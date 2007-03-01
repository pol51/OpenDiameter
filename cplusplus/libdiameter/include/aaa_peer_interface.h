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

#ifndef __AAA_PEER_INTERFACE_H__
#define __AAA_PEER_INTERFACE_H__

#include "aaa_data_defs.h"
#include "aaa_peer_table.h"
#include "aaa_route_msg_router.h"

typedef enum {
    PEER_EVENT_CONN_NACK,
    PEER_EVENT_TIMEOUT_OR_NONCEA,
} PFSM_EV_ERR;

class DiameterPeerEventInterface
{
   public:
      virtual void Connected() = 0;
      virtual void Disconnected(int cause) = 0;
      virtual void Error(PFSM_EV_ERR err) = 0;
      virtual ~DiameterPeerEventInterface() {
      }

      // TBD: Can add more events here when needed
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterPeer :
   public DiameterPeerEntry
{
   public:
      DiameterPeer(AAA_Task &task,
               std::string &peername,
               int peerport,
               int use_sctp,
               int tls_enabled,
               int etime,
               bool is_static) :
          DiameterPeerEntry(task,
                        peername,
                        peerport,
                        use_sctp,
                        tls_enabled,
                        etime,
                        is_static),
          m_EventInterface(NULL) {
      }
      virtual void RegisterUserEventHandler
         (DiameterPeerEventInterface &handler) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_EventMtx);
         m_EventInterface = &handler;
      }
      virtual void RemoveUserEventHandler() {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_EventMtx);
         m_EventInterface = NULL;
      }

   protected:
      virtual void Connected() {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_EventMtx);
         if (m_EventInterface) {
             m_EventInterface->Connected();
         }
      }
      virtual void Error(int resultCode) {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_EventMtx);
         if (m_EventInterface) {
             m_EventInterface->Error
                ((resultCode == AAA_LIMITED_SUCCESS) ?
                   PEER_EVENT_TIMEOUT_OR_NONCEA :
                   PEER_EVENT_CONN_NACK);
         }
      }
      virtual void Disconnected(int cause) {

         DIAMETER_MSG_ROUTER()->ReTransmitEvent
            (static_cast<DiameterPeerEntry*>(this));

         ACE_Read_Guard<ACE_RW_Mutex> guard(m_EventMtx);
         if (m_EventInterface) {
             m_EventInterface->Disconnected(cause);
         }
      }

   private:
      ACE_RW_Mutex m_EventMtx;
      DiameterPeerEventInterface *m_EventInterface;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterPeerManager
{
   public:
      DiameterPeerManager(AAA_Task &t) :
         m_Task(t) {
      }
      bool Add(std::string &peername,
               int peerport,
               int use_sctp,
               int tls_enabled,
               int etime,
               bool is_static) {
         if (!is_static && (etime == 0)) {
             etime = DIAMETER_PEER_TABLE()->ExpirationTime();
         }
         DiameterPeer *p = new DiameterPeer(m_Task,
                                            peername,
                                            peerport,
                                            use_sctp,
                                            tls_enabled,
                                            etime,
                                            is_static);
         if (p) {
             DIAMETER_PEER_TABLE()->Add(p);
             return true;
         }
         return false;
      }
      bool Delete(std::string &peername) {
          DiameterPeerEntry *e = DIAMETER_PEER_TABLE()->Remove(peername);
          if (e) {
              e->Stop(AAA_DISCONNECT_DONTWANTTOTALK);
              delete e;
              return true;
          }
          return false;
      }
      DiameterPeer *Lookup(std::string &peername) {
          return static_cast<DiameterPeer*>(DIAMETER_PEER_TABLE()->Lookup(peername));
      }

   private:
      AAA_Task &m_Task;
};

class DiameterPeerConnector
{
   public:
      static inline void Start() {
          DiameterPeerEntry *e = DIAMETER_PEER_TABLE()->First();
          while (e) {
              e->Start();
              e = DIAMETER_PEER_TABLE()->Next(e);
          }
      }
      static inline int GetNumOpenPeers() {
          int count = 0;
          DiameterPeerEntry *e = DIAMETER_PEER_TABLE()->First();
          while (e) {
              count += (e->IsOpen()) ? 1 : 0;
              e = DIAMETER_PEER_TABLE()->Next(e);
          }
          return count;
      }
      static inline void Stop(DIAMETER_DISCONNECT_CAUSE cause) {
          DiameterPeerEntry *e = DIAMETER_PEER_TABLE()->First();
          while (e) {
              e->Stop(cause);
              e = DIAMETER_PEER_TABLE()->Next(e);
          }
      }
};

class DiameterPeerAcceptor : public DiameterTcpAcceptor,
                                    DiameterSctpAcceptor
{
   public:
      void Start(int ports[DIAMETER_PEER_TTYPE_MAX]) {
          if (ports[DIAMETER_PEER_TTYPE_TCP] > 0) {
#ifdef ACE_HAS_IPV6
              ACE_INET_Addr hostIdentity(ports[DIAMETER_PEER_TTYPE_TCP], DIAMETER_CFG_TRANSPORT()->identity.c_str(),
                                         (DIAMETER_CFG_TRANSPORT()->use_ipv6) ? AF_INET6 : AF_INET);
#else /* ! ACE_HAS_IPV6 */
              ACE_INET_Addr hostIdentity(ports[DIAMETER_PEER_TTYPE_TCP], DIAMETER_CFG_TRANSPORT()->identity.c_str(), AF_INET);
#endif /* ! ACE_HAS_IPV6 */
              AAA_LOG((LM_ERROR, "(%P|%t) TCP Acceptor Listening at %d, binding to %s \n",
                      ports[DIAMETER_PEER_TTYPE_TCP], DIAMETER_CFG_TRANSPORT()->identity.c_str()));
              DiameterTcpAcceptor::Open(ports[DIAMETER_PEER_TTYPE_TCP], hostIdentity);
          }
          if (ports[DIAMETER_PEER_TTYPE_SCTP] > 0) {
              AAA_LOG((LM_ERROR, "(%P|%t) SCTP Acceptor Listening at %d, binding to ", ports[DIAMETER_PEER_TTYPE_SCTP]));

              ACE_Multihomed_INET_Addr hostAddresses;
              char **name = new char*[DIAMETER_CFG_TRANSPORT()->advertised_hostname.size()];
              std::list<std::string>::iterator i = DIAMETER_CFG_TRANSPORT()->advertised_hostname.begin();
              for (int y = 0; i != DIAMETER_CFG_TRANSPORT()->advertised_hostname.end(); i++, y++) {
                  name[y] = (char*)(*i).c_str();
                  AAA_LOG((LM_ERROR, "%s ", name[y]));
              }
              AAA_LOG((LM_ERROR, "\n"));
              hostAddresses.set(ports[DIAMETER_PEER_TTYPE_SCTP], DIAMETER_CFG_TRANSPORT()->identity.c_str(), 1,
#ifdef ACE_HAS_IPV6
                                (DIAMETER_CFG_TRANSPORT()->use_ipv6) ? AF_INET6 : AF_INET,
#else /* ! ACE_HAS_IPV6 */
                                 AF_INET,
#endif /* ! ACE_HAS_IPV6 */
                                 (const char**)name, DIAMETER_CFG_TRANSPORT()->advertised_hostname.size());
              DiameterSctpAcceptor::Open(ports[DIAMETER_PEER_TTYPE_SCTP], hostAddresses);
              delete[] name;
          }
      }
      void Stop() {
          DiameterTcpAcceptor::Close();
          DiameterSctpAcceptor::Close();
      }

   protected:

    /*
        5.6.1.  Incoming connections

           When a connection request is received from a Diameter peer, it is
           not, in the general case, possible to know the identity of that peer
           until a CER is received from it.  This is because host and port
           determine the identity of a Diameter peer; and the source port of an
           incoming connection is arbitrary.  Upon receipt of CER, the identity
           of the connecting peer can be uniquely determined from Origin-Host.

           For this reason, a Diameter peer must employ logic separate from the
           state machine to receive connection requests, accept them, and await
           CER.  Once CER arrives on a new connection, the Origin-Host that
           identifies the peer is used to locate the state machine associated
           with that peer, and the new connection and CER are passed to the
           state machine as an R-Conn-CER event.

           The logic that handles incoming connections SHOULD close and discard
           the connection if any message other than CER arrives, or if an
           implementation-defined timeout occurs prior to receipt of CER.

           Because handling of incoming connections up to and including receipt
           of CER requires logic, separate from that of any individual state
           machine associated with a particular peer, it is described separately
           in this section rather than in the state machine above.
    */

      class PendingResponder :
         public DiameterRxMsgCollectorHandler {
            public:
               PendingResponder(DiameterPeerAcceptor &a,
                                std::auto_ptr<Diameter_IO_Base> io) :
                   m_IO(io), m_Acceptor(a) {
                   DiameterRxMsgCollector *h = reinterpret_cast<DiameterRxMsgCollector*>
                       (m_IO->Handler());
                   h->RegisterHandler(*this);
               }
               virtual ~PendingResponder() {
               }
               void Message(std::auto_ptr<DiameterMsg> msg) {

                   DiameterMsgQuery query(*msg);
                   if (query.IsCapabilities() && query.IsRequest()) {
                       DiameterIdentityAvpContainerWidget c_orhost(msg->acl);
                       diameter_identity_t *ohost = c_orhost.GetAvp
                           (DIAMETER_AVPNAME_ORIGINHOST);
                       if (ohost) {
                         DiameterPeerEntry *e = DIAMETER_PEER_TABLE()->Lookup(*ohost);
                         if (e) {
                            std::auto_ptr<PendingResponder> guard(this);
                            m_Acceptor.RemoveFromPendingList(*this);
                            e->IncommingConnectionRequest(m_IO, msg);
                            return;
                         }
                      }
                      /*
                         CERs received from unknown peers MAY be silently
                         discarded, or a CEA MAY be issued with the Result-Code
                         AVP set to DIAMETER_UNKNOWN_PEER. In both cases, the
                         transport connection is closed.  If the local policy
                         permits receiving CERs from unknown hosts, a successful
                         CEA MAY be returned.  If a CER from an unknown peer is
                         answered with a successful CEA, the lifetime of the peer
                         entry is equal to the lifetime of the transport connection.
                         In case of a transport failure, all the pending transactions
                         destined to the unknown peer can be discarded.
                      */
                   }
                   Error(DiameterRxMsgCollectorHandler::INVALID_MSG,
                         m_IO->Name());
               }
               void Error(COLLECTOR_ERROR error, std::string &io_name) {
                   if (error == DiameterRxMsgCollectorHandler::PARSING_ERROR) {
                       return; // not considered fatal for now
                   }
                   AAA_LOG((LM_ERROR,
                             "(%P|%t) %s peer failed establishing state [%d], closing\n",
                              io_name.c_str(), error));
                   std::auto_ptr<PendingResponder> guard(this);
                   m_Acceptor.RemoveFromPendingList(*this);

                   m_IO->Close();
                   DIAMETER_IO_GC().ScheduleForDeletion(m_IO);
               }
               int SendErrorAnswer(std::auto_ptr<DiameterMsg> &msg) {
                   AAA_LOG((LM_ERROR,
                             "(%P|%t) Peer message maybe malformed, ignoring during CER/CEA exchange\n"));
                   return (0);
               }

            private:
               std::auto_ptr<Diameter_IO_Base> m_IO;
               DiameterPeerAcceptor &m_Acceptor;
      };

      friend class PendingResponder;

      int Success(Diameter_IO_Base *io) {
          std::auto_ptr<Diameter_IO_Base> newIO(io);
          PendingResponder *r = new PendingResponder(*this, newIO);
          if (r) {
              AAA_TokenScopeLock guard(m_ResponderToken);
              m_PendingResponders.push_back(r);
          }
          return (0);
      }
      int Failed() {
          Stop();
          return (0);
      }

   protected:
      void RemoveFromPendingList(PendingResponder &r) {
         AAA_TokenScopeLock guard(m_ResponderToken);
         std::list<PendingResponder*>::iterator i;
         for (i = m_PendingResponders.begin();
              i != m_PendingResponders.end();
              i++) {
             if ((*i) == &r) {
                 m_PendingResponders.erase(i);
                 break;
             }
         }
      }

   private:
      std::list<PendingResponder*> m_PendingResponders;
      ACE_Token m_ResponderToken;
};

#endif 
