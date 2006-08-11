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


#ifndef __AAA_PEER_TABLE_H__
#define __AAA_PEER_TABLE_H__

#include "aaa_peer_fsm.h"

class DIAMETERBASEPROTOCOL_EXPORT AAA_PeerEntry : 
	public AAA_PeerStateMachine,
          AAA_MsgCollectorHandler
{
   public:
      AAA_PeerEntry(AAA_Task &task,
                    std::string &peername,
                    int peerport,
                    int tls_enabled,
                    int etime,
                    bool is_static) :
          AAA_PeerStateMachine(task),
          m_PeerInitiator(*this) {
          PeerData().m_Identity = peername;
          PeerData().m_Port = peerport;
          PeerData().m_Static = is_static;
          PeerData().m_Expiration = etime;
          PeerData().m_TLS = tls_enabled ? true : false;
          PeerData().m_DisconnectCause = AAA_DISCONNECT_DONTWANTTOTALK;
          AAA_StateMachineWithTimer<AAA_PeerStateMachine>::Start();
      }
      virtual ~AAA_PeerEntry() {
          AAA_PeerStateMachine::Stop();
      }
    
      void Start() throw (AAA_Error);
      void Stop(AAA_DISCONNECT_CAUSE cause);
    
      bool IsOpen() {
          return ((state == AAA_PEER_ST_I_OPEN) ||
                  (state == AAA_PEER_ST_R_OPEN)) ?
              true : false;
      }
      bool IsStatic() {
          return PeerData().m_Static;
      }

      // Internal use only    
      void IncommingConnectionRequest(std::auto_ptr<AAA_IO_Base> io,
                                      std::auto_ptr<DiameterMsg> cer);
      void ConnectionRequestAccepted(std::auto_ptr<AAA_IO_Base> io);
      void ConnectionRequestFailed();
      void Dump() {
          AAA_LOG(LM_INFO, "(%P|%t)                Peer : Host = %s, Port = %d, TLS = %d\n", 
                  PeerData().m_Identity.data(), 
                  PeerData().m_Port, 
                  PeerData().m_TLS);
      }
    
   protected:      
      void Message(std::auto_ptr<DiameterMsg> msg);
      void Error(COLLECTOR_ERROR error, std::string &io_name);

      class PeerInitiator : public AAA_TcpConnector,
                                   AAA_TlsConnector
      {
          public:
             PeerInitiator(AAA_PeerEntry &e) :
                 m_Entry(e) {
             }
             int Connect(std::string &host,
                         int port,
                         bool tls = false) {
                 return (tls) ?
                     AAA_TlsConnector::Open(host, port) :
                     AAA_TcpConnector::Open(host, port);
             }
             int Stop() {
                 AAA_TlsConnector::Close();
                 AAA_TcpConnector::Close();
                 return (0);
             }

          protected:
             int Success(AAA_IO_Base *io) {
                 std::auto_ptr<AAA_IO_Base> newIO(io);
                 m_Entry.ConnectionRequestAccepted(newIO);
                 return (0);
             }
             int Failed() {
                 m_Entry.ConnectionRequestFailed();
                 return (0);
             }
          
          private:
             AAA_PeerEntry &m_Entry;
      };

      friend class PeerInitiator;

   private:
      PeerInitiator m_PeerInitiator;
};

typedef std::list<AAA_PeerEntry*> AAA_PeerList;

class AAA_PeerTable : private AAA_PeerList
{
   public:
      AAA_PeerTable() : m_ExpirationTime(0) {
      }
      virtual ~AAA_PeerTable() {
      }
      bool Add(AAA_PeerEntry *e) {
          ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
          push_back(e);
          return true;
      }
      AAA_PeerEntry *Lookup(std::string &peername) {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          AAA_PeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              if ((*i)->Data().m_Identity == peername) {
                  return (*i);
              }
          }
          return (NULL);
      }
      AAA_PeerEntry *Remove(std::string &peername) {
          ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
          AAA_PeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              if ((*i)->Data().m_Identity == peername) {
                  erase(i);
                  return (*i);
              }
          }
          return (NULL);
      }
      AAA_PeerEntry *First() {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          return (! empty()) ? front() : NULL;
      }
      AAA_PeerEntry *Next(AAA_PeerEntry *e) {
          ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
          AAA_PeerList::iterator i;
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
              AAA_PeerEntry *e = front();
              pop_front();
              delete e;
          }
      }
      int &ExpirationTime() {
          return m_ExpirationTime;
      }
      void Dump() {
          AAA_LOG(LM_INFO, "(%P|%t) Dumping Peer Table\n");
          AAA_LOG(LM_INFO, "(%P|%t)      Expire Time %d\n", m_ExpirationTime);
          AAA_PeerList::iterator i;
          for (i = begin(); i != end(); i++) {
              (*i)->Dump();
          }
      }

   private:
      ACE_RW_Mutex m_Lock;
      int m_ExpirationTime;
};

typedef ACE_Singleton<AAA_PeerTable, ACE_Recursive_Thread_Mutex> AAA_PeerTable_S;
#define AAA_PEER_TABLE() AAA_PeerTable_S::instance()

#endif /* __AAA_PEER_TABLE_H__ */

