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


#ifndef __AAA_PEER_TABLE_H__
#define __AAA_PEER_TABLE_H__

#include "aaa_peer_fsm.h"

class DIAMETERBASEPROTOCOL_EXPORT DiameterPeerEntry :
	public DiameterPeerStateMachine,
          DiameterRxMsgCollectorHandler
{
   public:
      DiameterPeerEntry(AAA_Task &task,
                        std::string &peername,
                        int peerport,
                        int use_sctp,
                        int tls_enabled,
                        int etime,
                        bool is_static) :
          DiameterPeerStateMachine(task),
          m_PeerInitiator(*this) {
          PeerData().m_Identity = peername;
          PeerData().m_Port = peerport;
          PeerData().m_Static = is_static;
          PeerData().m_Expiration = etime;
          PeerData().m_TLS = tls_enabled ? true : false;
          PeerData().m_UseSctp = use_sctp ? true : false;
          PeerData().m_DisconnectCause = AAA_DISCONNECT_DONTWANTTOTALK;
          AAA_StateMachineWithTimer<DiameterPeerStateMachine>::Start();
      }
      virtual ~DiameterPeerEntry() {
          DiameterPeerStateMachine::Stop();
      }

      void Start() throw (AAA_Error);
      void Stop(DIAMETER_DISCONNECT_CAUSE cause);

      bool IsOpen() {
          return ((state == DIAMETER_PEER_ST_I_OPEN) ||
                  (state == DIAMETER_PEER_ST_R_OPEN)) ?
              true : false;
      }
      bool IsStatic() {
          return PeerData().m_Static;
      }

      // Internal use only    
      void IncommingConnectionRequest(std::auto_ptr<Diameter_IO_Base> io,
                                      std::auto_ptr<DiameterMsg> cer);
      void ConnectionRequestAccepted(std::auto_ptr<Diameter_IO_Base> io);
      void ConnectionRequestFailed();
      void Dump() {
          AAA_LOG((LM_INFO, "(%P|%t)                Peer : Host = %s, Port = %d, TLS = %d\n", 
                  PeerData().m_Identity.c_str(), 
                  PeerData().m_Port, 
                  PeerData().m_TLS));
      }

   protected:
      void Message(std::auto_ptr<DiameterMsg> msg);
      void Error(COLLECTOR_ERROR error, std::string &io_name);
      int SendErrorAnswer(std::auto_ptr<DiameterMsg> &msg);

      class PeerInitiator : public DiameterTcpConnector,
                                   DiameterSctpConnector
      {
          public:
             PeerInitiator(DiameterPeerEntry &e) :
                 m_Entry(e) {
             }
             int Connect(std::string &host,
                         int port,
                         bool useSctp) {
                 return (useSctp) ?
                     DiameterSctpConnector::Open(host, port) :
                     DiameterTcpConnector::Open(host, port);
             }
             int Stop() {
                 DiameterSctpConnector::Close();
                 DiameterTcpConnector::Close();
                 return (0);
             }

          protected:
             int Success(Diameter_IO_Base *io) {
                 std::auto_ptr<Diameter_IO_Base> newIO(io);
                 m_Entry.ConnectionRequestAccepted(newIO);
                 return (0);
             }
             int Failed() {
                 m_Entry.ConnectionRequestFailed();
                 return (0);
             }

          private:
             DiameterPeerEntry &m_Entry;
      };

      friend class PeerInitiator;

   private:
      PeerInitiator m_PeerInitiator;
};

typedef std::list<DiameterPeerEntry*> DiameterPeerList;

class DiameterPeerTable : private DiameterPeerList
{
   public:
      DiameterPeerTable() : m_ExpirationTime(0) {
      }
      virtual ~DiameterPeerTable() {
      }
      bool Add(DiameterPeerEntry *e) {
          ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
          push_back(e);
          return true;
      }
      DiameterPeerEntry *Lookup(std::string &peername) {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          DiameterPeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              //
              // Warning: This is a case in-sensitive lookup which may not
              //          be generally appropriate if we consider FQDN as
              //          a non ascii value.
              //
              // Deprecated:
              //  if ((*i)->Data().m_Identity == peername) {
              //
              if (! strcasecmp((*i)->Data().m_Identity.c_str(), peername.c_str())) {
                  return (*i);
              }
          }
          return (NULL);
      }
      DiameterPeerEntry *Remove(std::string &peername) {
          ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
          DiameterPeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              //
              // Warning: This is a case in-sensitive lookup which may not
              //          be generally appropriate if we consider FQDN as
              //          a non ascii value.
              //
              // Deprecated:
              //  if ((*i)->Data().m_Identity == peername) {
              //
              if (! strcasecmp((*i)->Data().m_Identity.c_str(), peername.c_str())) {
                  erase(i);
                  return (*i);
              }
          }
          return (NULL);
      }
      DiameterPeerEntry *First() {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          return (! empty()) ? front() : NULL;
      }
      DiameterPeerEntry *Next(DiameterPeerEntry *e) {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          DiameterPeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              if (e == (*i)) {
                  i ++;
                  if (i != end()) {
                      return (*i);
                  }
                  break;
              }
          }
          return (NULL);
      }
      void Clear() {
          ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
          while (! empty()) {
              DiameterPeerEntry *e = front();
              pop_front();
              delete e;
          }
      }
      int &ExpirationTime() {
          return m_ExpirationTime;
      }
      void Dump() {
          AAA_LOG((LM_INFO, "(%P|%t) Dumping Peer Table\n"));
          AAA_LOG((LM_INFO, "(%P|%t)      Expire Time %d\n", m_ExpirationTime));
          DiameterPeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              (*i)->Dump();
          }
      }

   private:
      ACE_RW_Mutex m_Lock;
      int m_ExpirationTime;
};

typedef ACE_Singleton<DiameterPeerTable, ACE_Recursive_Thread_Mutex> DiameterPeerTable_S;
#define DIAMETER_PEER_TABLE() DiameterPeerTable_S::instance()

#endif /* __AAA_PEER_TABLE_H__ */

