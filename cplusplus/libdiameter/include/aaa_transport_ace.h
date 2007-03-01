/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
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

#ifndef __AAA_TRANSPORT_ACE_H__
#define __AAA_TRANSPORT_ACE_H__

#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_SEQPACK_Acceptor.h"
#include "ace/SOCK_SEQPACK_Connector.h"
#include "ace/Signal.h"
#include "ace/Handle_Set.h"
#include "aaa_transport_interface.h"

// interface implemented by transport
// specific IO (lower layer). Assumes
// lower layer is connection oriented
template<class ACE_ACCEPTOR,
         class ACE_CONNECTOR,
         class ACE_STREAM,
         class ACE_ADDRESS,
         int IP_PROTOCOL>
class Diameter_ACE_Transport : public DiameterTransportInterface<ACE_ADDRESS>
{
   public:
      typedef enum {
          ACCEPTOR_TIMEOUT = 10   // 10 sec
      };

   public:
      Diameter_ACE_Transport() : m_PendingStream(0) {
      }
      virtual ~Diameter_ACE_Transport() {
         if (m_PendingStream) {
             ResetStream((DiameterTransportInterface<ACE_ADDRESS>*&)m_PendingStream);
         }
      }
      int Open() {
         return 0;
      }
      int Connect(std::string &hostname, int port) {
         if (! m_PendingStream) {
             m_PendingStream = new Diameter_ACE_Transport
                  <ACE_ACCEPTOR, ACE_CONNECTOR, ACE_STREAM, ACE_ADDRESS, IP_PROTOCOL>;
             ACE_Time_Value tm(0, 0);
             ACE_ADDRESS dest(port, hostname.c_str());
             int rc = m_Connector.connect(m_PendingStream->Stream(),
                                          dest, &tm, ACE_Addr::sap_any, true);
             return AceAsynchResults(rc);
         }
         return (-1);
      }
      int Complete(DiameterTransportInterface<ACE_ADDRESS> *&iface) {
         iface = 0; 
         if (m_PendingStream) {
            int rc = m_Connector.complete(m_PendingStream->Stream());
            if (rc == 0) {
                return HandOverStream(iface, 
                                (DiameterTransportInterface<ACE_ADDRESS>*&)m_PendingStream);
            }
            else {
                return ((AceAsynchResults(rc) == 0) && (errno != ETIMEDOUT)) ? 0 :
                        ResetStream((DiameterTransportInterface<ACE_ADDRESS>*&)
                                    m_PendingStream);
            }
         }
         return (-1);
      }
      int Listen(int port, ACE_ADDRESS localAddr) {
         if (! m_PendingStream) {
             m_PendingStream = new Diameter_ACE_Transport
                 <ACE_ACCEPTOR, ACE_CONNECTOR, ACE_STREAM, ACE_ADDRESS, IP_PROTOCOL>;
             return AceAsynchResults(m_Acceptor.open(localAddr, true, AddressFamilyToUse(),
                                                     ACE_DEFAULT_BACKLOG, IP_PROTOCOL));
         }
         return (-1);
      }
      virtual int Accept(DiameterTransportInterface<ACE_ADDRESS> *&iface) {
         iface = 0;
         if (m_PendingStream) {
            ACE_Time_Value wait(ACCEPTOR_TIMEOUT, 0);
            int rc = m_Acceptor.accept(m_PendingStream->Stream(), 0, &wait);
            if (rc == 0) {
                HandOverStream(iface, (DiameterTransportInterface<ACE_ADDRESS>*&)
                               m_PendingStream);
                m_PendingStream = new Diameter_ACE_Transport
                     <ACE_ACCEPTOR, ACE_CONNECTOR, ACE_STREAM, ACE_ADDRESS, IP_PROTOCOL>;
            }
            else if (AceAsynchResults(rc) < 0) {
                ResetStream((DiameterTransportInterface<ACE_ADDRESS>*&)
                            m_PendingStream);
                return (0);
            }
            return AceAsynchResults((rc == 0) ? 1 : rc);
         }
         return (-1);
      }
      int Send(void *data, size_t length) {
         return AceIOResults(m_Stream.send(data, length));
      }
      int Receive(void *data, size_t length, int timeout = 0) {
         if (timeout > 0) {
             ACE_Time_Value tm(0, timeout);
             return AceIOResults(m_Stream.recv(data, length, &tm));
         }
         return AceIOResults(m_Stream.recv(data, length));
      }
      int Close() {
         // close auxillary sockets
         m_Acceptor.close();

         // close data stream
         if (!AceAsynchResults(m_Stream.close_writer()) &&
             !AceAsynchResults(m_Stream.close_reader())) {
             return (0);
         }
         return AceAsynchResults(m_Stream.close());
      }
      ACE_STREAM &Stream() {
         return m_Stream;
      }
      int TransportProtocolInUse() {
         return IP_PROTOCOL;
      }

   private:
      // template streams and factories
      ACE_STREAM     m_Stream;
      ACE_ACCEPTOR   m_Acceptor;
      ACE_CONNECTOR  m_Connector;

      // pending stream
      Diameter_ACE_Transport<ACE_ACCEPTOR,
                             ACE_CONNECTOR,
                             ACE_STREAM,
                             ACE_ADDRESS,
                             IP_PROTOCOL>*
                     m_PendingStream;

      int inline AceAsynchResults(int rc) {
         if (rc < 0) {
             if ((errno == EWOULDBLOCK) || 
                 (errno == ETIME) ||
                 (errno == ETIMEDOUT) ||
                 (errno == EAGAIN)) { 
                return (0);
             }
             AAA_LOG((LM_ERROR, "(%P|%t) Async Transport Setup Reports: %s\n",
                        strerror(errno)));
         }
         return (rc);
      }
      int inline AceIOResults(int rc) {
         if (rc < 0) {
             if ((errno == EWOULDBLOCK) || 
                 (errno == ETIME) ||
                 (errno == EAGAIN)) { 
                return (0);
             }
             AAA_LOG((LM_ERROR, "(%P|%t) Async IO Reports: %s\n",
                        strerror(errno)));
             rc = (-1);
         }
         else if (rc == 0) {
             AAA_LOG((LM_ERROR, "(%P|%t) Async IO, peer has closed\n"));
             rc = (-1);
         }
         return (rc);
      }
      int inline ResetStream(DiameterTransportInterface<ACE_ADDRESS> *&stream) {
         delete stream;
         stream = NULL;
         return (-1);
      }
      int inline HandOverStream(DiameterTransportInterface<ACE_ADDRESS> *&dest,
                                DiameterTransportInterface<ACE_ADDRESS> *&src) {
         dest = src;
         src = NULL;
         return (1);
      }
      int inline AddressFamilyToUse() {
#ifdef ACE_HAS_IPV6
         return (DIAMETER_CFG_TRANSPORT()->use_ipv6) ? AF_INET6 : AF_INET;
#else /* ! ACE_HAS_IPV6 */
         return AF_INET;
#endif /* ! ACE_HAS_IPV6 */
      }
};

class Diameter_ACE_TransportAddress :
    public DiameterTransportAddress<ACE_INET_Addr>
{
   public:
      virtual int GetLocalAddresses(size_t &count,
                                    ACE_INET_Addr *&addrs) {
         return ACE::get_ip_interfaces
             (count, addrs);
      }
      virtual void *GetAddressPtr(ACE_INET_Addr &addr) {
#if defined (ACE_HAS_IPV6)
         if (addr.get_type() == AF_INET6) {
            sockaddr_in6 *in = (sockaddr_in6*)addr.get_addr();
            return in->sin6_addr.s6_addr;
         }
         else if (addr.get_type() == AF_INET) { 
#else
         if (addr.get_type() == AF_INET) { 
#endif
            sockaddr_in  *in = (sockaddr_in*)addr.get_addr();
            return &(in->sin_addr.s_addr);
         }
         return (NULL);
      }
      virtual int GetAddressSize(ACE_INET_Addr &addr) {
#if defined (ACE_HAS_IPV6)
          if (addr.get_type() == AF_INET6) {
             return sizeof(struct in6_addr);
          }
         else if (addr.get_type() == AF_INET) { 
#else
         if (addr.get_type() == AF_INET) { 
#endif
             return sizeof(struct in_addr);
         }
         return 0;
      }
};

class Diameter_IO_SigMask : public ACE_Event_Handler
{
    public:
       Diameter_IO_SigMask() {
          m_SigRegistrar.register_handler(SIGPIPE, this);
       }
       virtual int handle_signal(int signo,
                                 siginfo_t * = 0, 
                                 ucontext_t * = 0) {
           AAA_LOG((LM_ERROR, 
              "(%P|%t) SIGPIPE received, closing connection\n"));
#ifndef WIN32
           errno = EPIPE;
#endif
           return 0;
       }
   private:
       ACE_Sig_Handler m_SigRegistrar;
};

#endif 
