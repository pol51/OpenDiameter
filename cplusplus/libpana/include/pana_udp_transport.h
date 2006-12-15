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
        virtual ~PANA_IfAdapterInfo() {
            if (m_pAddresses) {
                ACE_OS::free(m_pAddresses);
                m_pAddresses = NULL;
            }
        }
        ACE_INT32 get_local_addr(PANA_AddressList &localAddresses) {
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
                        localAddresses.push_back(aceIpAddr);
                        pAddr = pAddr->Next;
                    }
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
        ACE_INT32 get_local_addr(PANA_AddressList &localAddresses) {
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
                       default:
                          continue;
                   }
                   localAddresses.push_back(addr);
                   rc = 0;
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
        ACE_INT32 get_local_addr(PANA_AddressList &localAddresses) {

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

               ACE_INET_Addr addr;
               switch (ifr->ifr_addr.sa_family) {
                   case AF_INET:
                      addr.set_addr(ifr->ifr_addr,
                                    sizeof(sockaddr_in), 0);
                      break;
                   case AF_INET6:
                      addr.set_addr(ifr->ifr_addr,
                                    sizeof(sockaddr_in6), 0);
                      break;
                   default:
                      continue;
               }
               localAddresses.push_back(addr);
               rc = 0;
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

class PANA_EXPORT PANA_Socket : public PANA_IO
{
    public:
        typedef enum {
            PANA_SOCKET_RECV_TIMEOUT = 5 // sec
        };

    public:
        virtual ACE_INT32 open(ACE_INET_Addr &listenAddr) {
            return m_Socket.open(SOCK_DGRAM,
                                 listenAddr.get_type(),
                                 IPPROTO_UDP, 0);
        }
        virtual ACE_INT32 recv(void *buf,
                               size_t n,
                               ACE_INET_Addr &srcAddr) {
            ACE_Handle_Set handle_set;
            handle_set.reset();
            handle_set.set_bit(m_Socket.get_handle());

            // Check the status of the current socket to make sure there's data
            // to recv (or time out).
            ACE_Time_Value tm(PANA_SOCKET_RECV_TIMEOUT, 0);
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
            sockaddr saddr;
            ACE_INT32 addrLen = 0;
            memset(&saddr, 0x0, sizeof(saddr));
            ACE_INT32 pktLen = ACE_OS::recvfrom(m_Socket.get_handle(),
                                                (char*)buf, n, 0,
                                                &saddr, &addrLen);
            if (pktLen > 0) {
                srcAddr.set((struct sockaddr_in*)&saddr, addrLen);
                PANA_DUMP_RXADDR(srcAddr);
            }
            return (pktLen);
        }
        virtual ACE_INT32 send(void *buf,
                               size_t n,
                               ACE_INET_Addr &destAddr) {
            PANA_DUMP_TXADDR(destAddr);
            return (ACE_OS::sendto(m_Socket.get_handle(),
                                   (char*)buf, n,
                                   0, (sockaddr*)destAddr.get_addr(),
                                   destAddr.get_size()));
        }
        virtual void close() {
            m_Socket.close();
        }

    protected:
        ACE_SOCK_IO m_Socket;
};

#endif /* __PANA_UDP_TRANSPORT_H__ */


