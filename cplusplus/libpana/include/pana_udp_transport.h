

/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
/*                                                                        */
/* This library is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU Lesser General Public License as         */
/* published by the Free Software Foundation; either version 2.1 of the   */
/* License, or (at your option) any later version.                        */
/*                                                                        */
/* This library is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      */
/* Lesser General Public License for more details.                        */
/*                                                                        */
/* You should have received a copy of the GNU Lesser General Public       */
/* License along with this library; if not, write to the Free Software    */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307    */
/* USA.                                                                   */
/*                                                                        */
/* In addition, when you copy and redistribute some or the entire part of */
/* the source code of this software with or without modification, you     */
/* MUST include this copyright notice in each copy.                       */
/*                                                                        */
/* If you make any changes that are appeared to be useful, please send    */
/* sources that include the changed part to                               */
/* diameter-developers@lists.sourceforge.net so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */

#ifndef __PANA_UDP_TRANSPORT_H__
#define __PANA_UDP_TRANSPORT_H__

#include <string>
#include "ace/SOCK_IO.h"
#include "ace/ace_wchar.h"
#include "ace/Handle_Set.h"
#include "ace/Log_Msg.h"
#include "pana_io.h"

// #define PANA_UDP_ADDR_DEBUG
#if defined(PANA_UDP_ADDR_DEBUG)
#define PANA_DUMP_TXADDR(x) { char localBuf[64]; \
                              (x).addr_to_string(localBuf, \
		                                 sizeof(localBuf)); \
                              printf("Send: %s\n", localBuf); } 
#define PANA_DUMP_RXADDR(x) { char localBuf[64]; \
                              (x).addr_to_string(localBuf, \
		                                 sizeof(localBuf)); \
                              printf("Recv: %s\n", localBuf); } 
#else
#define PANA_DUMP_TXADDR(x)
#define PANA_DUMP_RXADDR(x)
#endif

#if defined(HAVE_GETIFADDRS) && defined(HAVE_IFADDRS_H)
#include <ifaddrs.h>
#endif

#if defined(HAVE_NET_IF_DL_H)
#include <net/if_dl.h>
#endif

#if !defined(ACE_WIN32)
#include <net/if.h>
#endif

class PANA_IfName
{
    public:
        std::string &Ifname() {
            return m_Ifname;
        }
    
    private:
        std::string m_Ifname;
};

#if defined(ACE_WIN32)

#include "windows.h"
#include "Iphlpapi.h"
#include "ace/ace_wchar.h"

class PANA_EXPORT PANA_IfAdapterInfo : public PANA_IfName
{
    public:
        PANA_IfAdapterInfo() {
            m_pAddresses = (IP_ADAPTER_ADDRESSES*)ACE_OS::malloc
                                  (sizeof(IP_ADAPTER_ADDRESSES));
            ULONG outBufLen = 0;
            DWORD dwRetVal = 0;
#if defined(ACE_HAS_IPV6)
            ULONG Family = (bool)PANA_CFG_GENERAL().m_IPv6Enabled ? AF_INET6 : AF_INET;
#else
            ULONG Family = AF_INET;
#endif
            if (GetAdaptersAddresses(Family,
                                     0, 
                                     NULL, 
                                     m_pAddresses, 
                                     &outBufLen) == ERROR_BUFFER_OVERFLOW) {
                ACE_OS::free(m_pAddresses);
                m_pAddresses = (IP_ADAPTER_ADDRESSES*)ACE_OS::malloc(outBufLen);
            }
            if ((dwRetVal = GetAdaptersAddresses(Family, 
                                                 0, 
                                                 NULL, 
                                                 m_pAddresses, 
                                                 &outBufLen)) != NO_ERROR) {
                ACE_OS::free(m_pAddresses);
                m_pAddresses = NULL;
            }
        }
        ~PANA_IfAdapterInfo() {
            if (m_pAddresses) {
                ACE_OS::free(m_pAddresses);
                m_pAddresses = NULL;
            }
        }
        ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) {
            PIP_ADAPTER_ADDRESSES pCurrAddresses = m_pAddresses;
            while (pCurrAddresses) {
                ACE_Wide_To_Ascii charDesc(pCurrAddresses->Description);
                if (! ACE_OS::strncmp(Ifname().data(), 
                                      charDesc.char_rep(),
                                      Ifname().length())) {
                    PIP_ADAPTER_UNICAST_ADDRESS pAddr = pCurrAddresses->FirstUnicastAddress;
                    while (pAddr) {
                        ACE_INET_Addr aceIpAddr((sockaddr_in*)pAddr->Address.lpSockaddr,
                                                 pAddr->Address.iSockaddrLength);
                        PANA_DeviceId *devId = PANA_DeviceIdConverter::CreateFromAddr
                                                    (aceIpAddr);
                        if (devId) {
                            localDevices.push_back(devId);
                        }
                        pAddr = pAddr->Next;
                    }
                    PANA_DeviceId *hw = PANA_DeviceIdConverter::CreateFromLinkLayer
                                         (pCurrAddresses->PhysicalAddress, 
                                          pCurrAddresses->PhysicalAddressLength);
                    localDevices.push_back(hw);
                    break;
                }
                pCurrAddresses = pCurrAddresses->Next;
            }
            return (pCurrAddresses) ? (0) : (-1);
        }    
        ACE_UINT32 get_adapter_index(const char *ifname) {
            PIP_ADAPTER_ADDRESSES pCurrAddresses = m_pAddresses;
            while (pCurrAddresses) {
                ACE_Wide_To_Ascii charDesc(pCurrAddresses->Description);
                if (! ACE_OS::strncmp(Ifname().data(), 
                                      charDesc.char_rep(),
                                      Ifname().length())) {
#if defined(ACE_HAS_IPV6)
                    return PANA_CFG_GENERAL().m_IPv6Enabled ? 
                        pCurrAddresses->Ipv6IfIndex : 
                        pCurrAddresses->IfIndex;
#else
                    return pCurrAddresses->IfIndex;
#endif
                }
                pCurrAddresses = pCurrAddresses->Next;
            }
            return (-1);
        }

    private:
        PIP_ADAPTER_ADDRESSES m_pAddresses;
};

#elif defined(HAVE_GETIFADDRS) && defined(HAVE_IFADDRS_H)

class PANA_EXPORT PANA_IfAdapterInfo : public PANA_IfName
{
    public:
        ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) {
           int rc = (-1);
           struct ifaddrs *ifap, *ifn;
           if (getifaddrs(&ifap) == 0) {
               for (ifn = ifap; ifn != NULL; ifn = ifn->ifa_next) {
                   if (ACE_OS::strcmp(ifn->ifa_name, Ifname().data())) {
                       continue;
                   }
                   if ((ifn->ifa_addr == 0)) {
                       continue;

                   }
                   ACE_INET_Addr addr;
                   switch (ifn->ifa_addr->sa_family) {
                       case AF_INET: 
                          addr.set_addr(ifn->ifa_addr,
                                        sizeof(sockaddr_in), 0);
                          break;
                       case AF_INET6:
                          addr.set_addr(ifn->ifa_addr,
                                        sizeof(sockaddr_in6), 0);
                          break;
#if defined(AF_LINK)
                       case AF_LINK: {
                              struct sockaddr_dl *dl = (struct sockaddr_dl*)ifn->ifa_addr; 
                              PANA_DeviceId *hw = PANA_DeviceIdConverter::CreateFromLinkLayer
                                            (dl->sdl_data + dl->sdl_nlen, 
                                             dl->sdl_alen);
#endif
#if defined(AF_PACKET)
#include <linux/if_packet.h>
                       case AF_PACKET: {
                              struct sockaddr_ll *ll = (struct sockaddr_ll*)ifn->ifa_addr; 
                              PANA_DeviceId *hw = PANA_DeviceIdConverter::CreateFromLinkLayer
  	                                    (ll->sll_addr, ll->sll_halen); 
#endif
                              if (hw) {
                                  localDevices.push_back(hw);
                                  rc = 0;
                              }
                          }
                       default:
                          continue;
                   }
                   PANA_DeviceId *id = PANA_DeviceIdConverter::CreateFromAddr
                                          (addr);
                   if (id) {
                       localDevices.push_back(id);
                       rc = 0;
                   }
               }
               freeifaddrs(ifap);
           }
           else {
               AAA_LOG((LM_ERROR, "(%P|%t) Get address failure\n"));
           }
           return (rc);
        }
        ACE_UINT32 get_adapter_index(const char *ifname) {
           return if_nametoindex(ifname);
        }
};

#else

#if defined(ACE_HAS_IPV6)
#error "You are compiling PANA with ACE_HAS_IPV6 enabled but your glibc does not support getifaddrs()"
#endif

#include <net/if_arp.h>

class PANA_EXPORT PANA_IfAdapterInfo : public PANA_IfName
{
    public:
        ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) {
            #define inaddrr(x)     (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
            #define IFRSIZE(sz)    ((int)((sz) * sizeof (struct ifreq)))

            int                ssize = 1, rc = (-1);
            struct ifreq       *ifr;
            struct ifreq       ifr_hw;
            struct ifconf      ifc;
            struct sockaddr_in *sa;

            ifc.ifc_len = IFRSIZE(ssize);
            ifc.ifc_req = NULL;

            ACE_SOCK_IO querySo;
            if (querySo.open(SOCK_DGRAM, PF_INET, IPPROTO_UDP, 0) < 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Interface query socket failed to open\n"));
                return rc;
            }

            do {

               ++ssize;

               /* realloc buffer size until no overflow occurs  */
               ifc.ifc_req = (struct ifreq*)realloc(ifc.ifc_req, IFRSIZE(ssize));
               if (ifc.ifc_req == NULL) {
                  ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) Allocation failure\n"), (-1));
               }

               ifc.ifc_len = IFRSIZE(ssize);

               /* attempt to retrieve IF conf */
               if (querySo.control(SIOCGIFCONF, &ifc)) {
                  ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) IOCTL error\n"), (-1));
               }
               
           } while  (IFRSIZE(ssize) <= ifc.ifc_len);

           ifr = ifc.ifc_req;
           for (;(char *) ifr < (char *) ifc.ifc_req + ifc.ifc_len; ++ifr) {

               if (ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data) {
                   continue;  /* duplicate, skip it */
               }

               sa = (sockaddr_in*)&ifr->ifr_addr;
               if (sa->sin_addr.s_addr == 0) {
                   continue; /* skip 0 address */
               }

               if (ACE_OS::strcmp(ifr->ifr_name, Ifname().data())) {
                   continue; /* skip this interface */
               }

               if ((ifr->ifr_addr.sa_family != AF_INET) &&
                   (ifr->ifr_addr.sa_family != AF_INET6)) {
                   continue; /* skip non IP interface */
               }

               PANA_DeviceId *ip = PANA_DeviceIdConverter::CreateFromAddr
                   (&ifr->ifr_addr, (ifr->ifr_addr.sa_family == AF_INET) ?
                         sizeof(sockaddr_in) : sizeof(sockaddr_in6));
               if (ip) {
                   localDevices.push_back(ip);
               }
                   
               ACE_OS::strcpy(ifr_hw.ifr_name, ifr->ifr_name);
               if (querySo.control(SIOCGIFHWADDR, &ifr_hw)) {
                  continue;  /* failed to get flags, skip it */
               }

               PANA_DeviceId *hw;
               switch (ifr_hw.ifr_hwaddr.sa_family) {
                   case ARPHRD_ETHER:
                       hw = PANA_DeviceIdConverter::CreateFromLinkLayer
                             (ifr_hw.ifr_hwaddr.sa_data, 6);
                       localDevices.push_back(hw);
                       break;
                   default:
                       // TBD: Add others here
                       delete hw;
                       break;
               }
               rc = 0;
               break;
           }

           if (ifc.ifc_req) {
               free(ifc.ifc_req);
           }

           return (rc);
        }    
       ACE_UINT32 get_adapter_index(const char *ifname) {
           return if_nametoindex(ifname);
        }
};

#endif

template<int PF>
class PANA_EXPORT PANA_Socket : public PANA_IO
{
    public:
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) {
            m_AdapterInfo.Ifname() = iface;
            m_BindAddr = listenAddr;
            return m_Socket.open(SOCK_DGRAM, PF, IPPROTO_UDP, 0);
        }
        virtual ACE_INT32 recv(void *buf,
                               size_t n,
                               ACE_UINT32 &srcPort,
                               PANA_DeviceIdContainer &srcDevices) {
            ACE_Handle_Set handle_set;
            handle_set.reset();
            handle_set.set_bit(m_Socket.get_handle());
 
            // Check the status of the current socket to make sure there's data
            // to recv (or time out).
            ACE_Time_Value tm(5, 0);
            int selectWidth;
#if defined (ACE_WIN64) || defined(ACE_WIN32)
            // This arg is ignored on Windows and causes pointer truncation
            // warnings on 64-bit compiles.
            selectWidth = 0;
#else
            selectWidth = (int)m_Socket.get_handle () + 1;
#endif /* ACE_WIN64 */
            switch (ACE_OS::select (selectWidth,
                                    handle_set,
                                    0, 0, &tm)) {
                case -1: return -1;
                case 0: errno = ETIME; return -1;
                default: break;
            }

            // Read the data
            ACE_INET_Addr srcAddr;
            sockaddr *saddr = (sockaddr *) srcAddr.get_addr ();
            ACE_INT32 addrLen = srcAddr.get_size ();
            ACE_INT32 pktLen = ACE_OS::recvfrom(m_Socket.get_handle(),
                                                (char*)buf, n, 0,
                                                saddr, &addrLen);
            if (pktLen > 0) {                
                srcAddr.set_size(addrLen);
                srcPort = srcAddr.get_port_number();
                PANA_DUMP_RXADDR(srcAddr);
                PANA_DeviceId *ip = PANA_DeviceIdConverter::CreateFromAddr(srcAddr);
                srcDevices.push_back(ip);
            }
            return (pktLen);
        }
        virtual ACE_INT32 send(void *buf,
                               size_t n,
                               ACE_UINT32 destPort,
                               PANA_DeviceIdContainer &destDevices) {
            PANA_DeviceId *id = PANA_DeviceIdConverter::
                                GetIpOnlyAddress(destDevices,
                                                 (PF == PF_INET6));
            if (id) {
                ACE_INET_Addr addr;
                PANA_DeviceIdConverter::FormatToAddr(*id, addr);
                addr.set_port_number(destPort);
		PANA_DUMP_TXADDR(addr);
                return (ACE_OS::sendto(m_Socket.get_handle(),
                                       (char*)buf, n,
                                       0, (sockaddr*)addr.get_addr(),
                                       addr.get_size()));
            }
            return (-1);
        }
        virtual ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) {
            return m_AdapterInfo.get_local_addr(localDevices);
        }
        virtual void close() {
            m_Socket.close();
        }

    protected:
        ACE_SOCK_IO m_Socket;
        ACE_INET_Addr m_BindAddr;
        PANA_IfAdapterInfo m_AdapterInfo;
};

template<int PF>    
class PANA_EXPORT PANA_BoundedSocket : public PANA_Socket<PF>
{
    public:
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) {
            if (PANA_Socket<PF>::open(iface, listenAddr) >= 0) {
                if (ACE_OS::bind(PANA_Socket<PF>::m_Socket.get_handle(),
                                 (sockaddr*)listenAddr.get_addr(),
                                 listenAddr.get_size()) == 0) {
                    return 0;
                }
                AAA_LOG((LM_ERROR, "(%P|%t) UDP bind failure\n"));
                PANA_Socket<PF>::close();
            }
            return -1;
        }
};
    
#if defined(ACE_HAS_IPV6)

template<bool BIND, bool MCAST, bool JOIN>
class PANA_EXPORT PANA_UdpIPv6 : public PANA_Socket<PF_INET6>
{
    public:
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) {
            m_AdapterInfo.Ifname() = iface;
            m_BindAddr = listenAddr;

            struct addrinfo hints, *res, *ressave;
            memset(&hints, 0, sizeof(struct addrinfo));
            /*
              AI_PASSIVE flag: the resulting address is used to bind
              to a socket for accepting incoming connections.
              So, when the hostname==NULL, getaddrinfo function will
              return one entry per allowed protocol family containing
              the unspecified address for that family.
            */
            hints.ai_flags    = AI_PASSIVE;
            hints.ai_family   = AF_INET6;
            hints.ai_socktype = SOCK_DGRAM;

            char service[32];
            ACE_OS::sprintf(service, "%d", 
                            listenAddr.get_port_number());
            if (getaddrinfo(NULL, service, 
                            &hints, &res) != 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Address info failure\n"));
                return -1;
            }
            ressave=res;

            /*
              Try open socket with each address getaddrinfo returned,
              until getting a valid listening socket.
             */
            try {
                while (res) {
                    if (m_Socket.open(res->ai_socktype,
                                      res->ai_family,
                                      res->ai_protocol,
                                      0) < 0) {
                        res = res->ai_next;
                        continue;
                    }
                    if (BIND && ACE_OS::bind(m_Socket.get_handle(), 
                                     res->ai_addr, 
                                     (int)res->ai_addrlen) < 0) {
                        AAA_LOG((LM_ERROR, "(%P|%t) UDP bind failure\n"));
                        throw (1);
                    }
                    sockaddr_in6 *in6 = (sockaddr_in6*)listenAddr.get_addr();
                    if (MCAST && IN6_IS_ADDR_MULTICAST(&in6->sin6_addr) &&
                        McastSetup(listenAddr) < 0) {
                        throw (1);
                    }
                    freeaddrinfo(ressave);
                    return 0;
                }
            } catch (...) {
            }
            close();
            freeaddrinfo(ressave);
            return -1;
        }
        virtual ACE_INT32 send(void *buf,
                               size_t n,
                               ACE_UINT32 destPort,
                               PANA_DeviceIdContainer &destDevices) {
            PANA_DeviceId *id = PANA_DeviceIdConverter::
                                GetIpOnlyAddress(destDevices, true);
            if (id) {
                sockaddr_in6 daddr;
                ACE_OS::memset(&daddr, 0, sizeof(daddr));
                daddr.sin6_family = AF_INET6;
                daddr.sin6_port = ACE_HTONS(destPort);
                ACE_OS::memcpy(&daddr.sin6_addr, 
                               id->value.data(),
                               id->value.size());
#if defined(ACE_WIN32)
                if (IN6_IS_ADDR_LINKLOCAL(&daddr.sin6_addr)) {
#else
                if (IN6_IS_ADDR_LINKLOCAL(&daddr.sin6_addr) ||
                    IN6_IS_ADDR_MULTICAST(&daddr.sin6_addr)) {
#endif
                    daddr.sin6_scope_id = m_AdapterInfo.get_adapter_index
                                   (          m_AdapterInfo.Ifname().data());
                    if (daddr.sin6_scope_id == 0) {
                        AAA_LOG((LM_ERROR, "(%P|%t) IF index failed to resolve: %s\n", 
                        m_AdapterInfo.Ifname().data()));
                        return -1;
                    }
                }
#if defined(PANA_UDP_ADDR_DEBUG)
                ACE_INET_Addr aceAddr;
                PANA_DeviceIdConverter::FormatToAddr(*id, aceAddr);
                PANA_DUMP_TXADDR(aceAddr);
#endif
                return (ACE_OS::sendto(m_Socket.get_handle(),
                                       (char*)buf, n, 
                                       0, (sockaddr*)&daddr,
                                       sizeof(sockaddr_in6)));
            }
            return (-1);
        }

    private:
        int McastSetup(ACE_INET_Addr &mcastAddr) {
            int hops = 8;
            if (m_Socket.set_option(IPPROTO_IPV6, 
                                    IPV6_MULTICAST_HOPS, 
                                    &hops, 
                                    sizeof(hops)) < 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Multicast hops option failed\n"));
                return -1;
            }
            int ifindex = m_AdapterInfo.get_adapter_index
                (m_AdapterInfo.Ifname().data());
            if (ifindex == 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Invalid ifname: %s\n",
                           m_AdapterInfo.Ifname().data()));
                return -1;
            }
            if (m_Socket.set_option(IPPROTO_IPV6, 
                                    IPV6_MULTICAST_IF,
                                    &ifindex, 
                                    sizeof(ifindex)) < 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Multicast IF option failed\n"));
                return -1; 
            }
            if (JOIN) {
                struct ipv6_mreq mreq;
                sockaddr_in6 *sin = (sockaddr_in6*)mcastAddr.get_addr();
                ACE_OS::memcpy(&mreq.ipv6mr_multiaddr,
                               &sin->sin6_addr, sizeof(in6_addr));
                mreq.ipv6mr_interface = ifindex;
                if (m_Socket.set_option(IPPROTO_IPV6, 
                                        IPV6_JOIN_GROUP,
                                        (char *)&mreq, 
                                        sizeof(mreq)) < 0) {
                    AAA_LOG((LM_ERROR, "(%P|%t) Multicast join failed: %s\n",
                              strerror(errno)));
                    return -1;
                }
            }
            return (0);
        }
};

typedef PANA_UdpIPv6<false, true, true> PANA_McastSender;
typedef PANA_UdpIPv6<true, true, true>  PANA_McastListener;
#if defined(ACE_WIN32)
typedef PANA_UdpIPv6<false, true, false> PANA_UdpBoundedSender;
#else
typedef PANA_UdpIPv6<false, true, true> PANA_UdpBoundedSender;
#endif

#else

class PANA_EXPORT PANA_McastSender : public PANA_BoundedSocket<PF_INET>
{
    public:
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) {
            if (PANA_Socket<PF_INET>::open(iface, listenAddr) >= 0) {
                return McastIFSetup();
            }
            return -1;
        }

    private:
        int McastIFSetup() {
            PANA_DeviceIdContainer localDevices;
            m_AdapterInfo.get_local_addr(localDevices);
            PANA_DeviceId *id = PANA_DeviceIdConverter::
                                GetIpOnlyAddress(localDevices);
            if (id) {
                ACE_INET_Addr ifAddr;
                PANA_DeviceIdConverter::FormatToAddr(*id, ifAddr);

                ip_mreq mcastAddr;
                mcastAddr.imr_interface.s_addr =
                    htonl(ifAddr.get_ip_address());

                if (m_Socket.set_option(IPPROTO_IP, 
                                        IP_MULTICAST_IF,
                                        &mcastAddr.imr_interface.s_addr,
                                        sizeof (struct in_addr)) >= 0) {
                    return 0; 
                }
            }
            AAA_LOG((LM_ERROR, "(%P|%t) Multicast IF option failed\n"));
            close();
            return -1;
        }
};

class PANA_EXPORT PANA_McastListener : public PANA_BoundedSocket<PF_INET>
{
    public:
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &mcastAddr) {
            ACE_INET_Addr listenAddr(mcastAddr.get_port_number());
            if (PANA_BoundedSocket<PF_INET>::open(iface, listenAddr) >= 0) {

                struct ip_mreq mreq;
                memset(&mreq, 0, sizeof(mreq));

                PANA_DeviceIdContainer localDevices;
                m_AdapterInfo.get_local_addr(localDevices);
                PANA_DeviceId *id = PANA_DeviceIdConverter::
                                    GetIpOnlyAddress(localDevices);
                if (id) {
                    ACE_INET_Addr ifAddr;
                    PANA_DeviceIdConverter::FormatToAddr(*id, ifAddr);
                    mreq.imr_interface.s_addr =
                        htonl(ifAddr.get_ip_address());
                }
                else {
                    mreq.imr_interface.s_addr = ACE_HTONS(INADDR_ANY); 
                }

                mreq.imr_multiaddr.s_addr = htonl(mcastAddr.get_ip_address());
                if (m_Socket.set_option(IPPROTO_IP, 
                                        IP_ADD_MEMBERSHIP,
                                        (char *)&mreq, 
                                        sizeof(mreq)) >= 0) {
                    return (0);
                }
                printf("Error: %s\n", strerror(errno));
                AAA_LOG((LM_ERROR, "(%P|%t) Multicast add membership failed\n"));
                close();
            }
            return -1;
        }
};

typedef PANA_McastSender PANA_UdpBoundedSender;

#endif // ACE_HAS_IPV6

#endif /* __PANA_UDP_TRANSPORT_H__ */


