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

#ifndef __AAA_SESSION_ACCT_CLIENT_FSM_H__
#define __AAA_SESSION_ACCT_CLIENT_FSM_H__

#include "aaa_session.h"
#include "aaa_session_acct_fsm.h"

///
/// REC_COLLECTOR MUST implement this class
/// This class provides callback functionality
/// to applications with regards to record
/// collection.
///
class DIAMETERBASEPROTOCOL_EXPORT AAA_ClientAcctRecCollector
{
    public:
        /// Asks the client app if append record and other
        /// vendor specific AVP's in message list
        virtual void GenerateRecord(AAAAvpContainerList &avpList,
                                    int recordType,
                                    int recordNum) = 0;

	/// Checks the client app if there is stored records
        virtual bool IsLastRecordInStorage() = 0;

	/// Checks the client app if there is buffer space available
        virtual bool IsStorageSpaceAvailable() = 0;

        /// Asks the client app to store the
        /// record in the message list 
        virtual AAAReturnCode StoreLastRecord(int recordType) = 0;

        /// Asks the client app to delete the
        /// last stored record if any
        virtual AAAReturnCode DeleteLastRecord(int recordType) = 0;

        virtual ~AAA_ClientAcctRecCollector() { }
};

class DIAMETERBASEPROTOCOL_EXPORT AAA_AcctSessionClientStateMachine :
   public AAA_AcctSessionStateMachine<AAA_AcctSessionClientStateMachine>
{  
   public:
      AAA_AcctSessionClientStateMachine(AAA_Task &t,
                                        AAA_AcctSession &a,
                                        AAA_ClientAcctRecCollector &c);
      virtual ~AAA_AcctSessionClientStateMachine() {
      }    
      AAA_ClientAcctRecCollector &RecCollector() {
 	 return m_RecCollector;
      }

      void TxACR();
      void RxACA(DiameterMsg &msg);

   private:
      AAA_ClientAcctRecCollector &m_RecCollector;
};

class AAA_SessAcctClient_TxACR : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxACR();
      }
};

class AAA_SessAcctClient_InterimTimout : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Attributes().RecordType() = AAA_ACCT_RECTYPE_INTERIM;
          fsm.TxACR();
      }
};

class AAA_SessAcctClient_RxACA_Success : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          if (fsm.RecCollector().IsLastRecordInStorage()) {
	      fsm.RecCollector().DeleteLastRecord
                  (fsm.Attributes().RecordType()());
	  }
          else if ((fsm.Attributes().RecordType()() == AAA_ACCT_RECTYPE_START) || 
                   (fsm.Attributes().RecordType()() == AAA_ACCT_RECTYPE_INTERIM)) {
               fsm.CancelAllTimer();
               fsm.ScheduleTimer(AAA_SESSION_ACCT_EV_INT_EXPIRE,
                      fsm.Attributes().InterimInterval()(),
                      0, AAA_TIMER_TYPE_INTERVAL);
	  }
          fsm.Session().Success();
      }
};

class AAA_SessAcctClient_StartIntTimer : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.ScheduleTimer(AAA_SESSION_ACCT_EV_SESSION_TOUT,
                  fsm.Attributes().SessionTimeout()(),
                  0, AAA_TIMER_TYPE_SESSION);
      }
};

template <int REC_TYPE>
class AAA_SessAcctClient_StoreRecord : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
	  fsm.RecCollector().StoreLastRecord(REC_TYPE);
          if ((REC_TYPE == AAA_ACCT_RECTYPE_START) || 
              (REC_TYPE == AAA_ACCT_RECTYPE_INTERIM)) {
              fsm.ScheduleTimer(AAA_SESSION_ACCT_EV_INT_EXPIRE,
                      fsm.Attributes().InterimInterval()(),
                      0, AAA_TIMER_TYPE_INTERVAL);
	  }
      }
};

typedef AAA_SessAcctClient_StoreRecord<AAA_ACCT_RECTYPE_START>
             AAA_SessAcctClient_StoreStartRecord;
typedef AAA_SessAcctClient_StoreRecord<AAA_ACCT_RECTYPE_STOP>
             AAA_SessAcctClient_StoreStopRecord;
typedef AAA_SessAcctClient_StoreRecord<AAA_ACCT_RECTYPE_INTERIM>
             AAA_SessAcctClient_StoreInterimRecord;
typedef AAA_SessAcctClient_StoreRecord<AAA_ACCT_RECTYPE_EVENT>
             AAA_SessAcctClient_StoreEventRecord;

class AAA_SessAcctClient_DeleteRecord : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
	  fsm.RecCollector().DeleteLastRecord
             (fsm.Attributes().RecordType()());
      }
};

class AAA_SessAcctClient_Disconnect : 
   public AAA_Action<AAA_AcctSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Session().Reset();
      }
};

class AAA_SessAcctClientStateTable : 
   public AAA_StateTable<AAA_AcctSessionClientStateMachine>
{
   public:
      AAA_SessAcctClientStateTable() {
        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Client or device requests      Send       PendingS
                     access                         accounting
                                                    start req.
         */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_REQUEST_ACCESS,
                           AAA_SESSION_ACCT_ST_PENDING_S,
                           m_acTxACR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Client or device requests      Send       PendingE
                     a one-time service             accounting
                                                    event req
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS,
                           AAA_SESSION_ACCT_ST_PENDING_E,
                           m_acTxACR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Records in storage             Send       PendingB
                                                    record
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_REC_IN_STORAGE,
                           AAA_SESSION_ACCT_ST_PENDING_B,
                           m_acTxACR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle     Any                            None       Idle
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                                   AAA_SESSION_ACCT_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Successful accounting                     Open
                     start answer received
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_RX_ACA_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acRxACASuccess);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Failure to send and buffer     Store      Open
                     space available and realtime   Start
                     not equal to DELIVER_AND_GRANT Record
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_FTS_NOT_DAG,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStoreStartRecord);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Failure to send and no buffer             Open
                     space available and realtime
                     equal to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_FTS_AND_GAL,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStartIntTimer);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Failure to send and no buffer  Disconnect Idle
                     space available and realtime   user/dev
                     not equal to
                     GRANT_AND_LOSE
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_FTS_NOT_GAL,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Failed accounting start answer            Open
                     received and realtime equal
                     to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStartIntTimer);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  Failed accounting start answer Disconnect Idle
                     received and realtime not      user/dev
                     equal to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingS  User service terminated        Store      PendingS
                                                    stop
                                                    record
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                           AAA_SESSION_ACCT_EV_STOP,
                           AAA_SESSION_ACCT_ST_PENDING_S,
                           m_acStoreStopRecord);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            PendingS Any                            None       PendingS
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_S,
                                   AAA_SESSION_ACCT_ST_PENDING_S);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Interim interval elapses       Send       PendingI
                                                    accounting
                                                    interim
                                                    record
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_INT_EXPIRE,
                           AAA_SESSION_ACCT_ST_PENDING_I,
                           m_acInterimTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      User service terminated        Send       PendingL
                                                    accounting
                                                    stop   
                                                    record
         */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_STOP,
                           AAA_SESSION_ACCT_ST_PENDING_L,
                           m_acTxACR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Any                            None       Open
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                                   AAA_SESSION_ACCT_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Successful accounting                     Open
                     interim answer received
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_RX_ACA_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acRxACASuccess);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Failure to send and buffer     Store      Open
                     space available and realtime   Interim
                     not equal to DELIVER_AND_GRANT Record
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_FTS_NOT_DAG,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStoreInterimRecord);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Failure to send and no buffer             Open
                     space available and realtime
                     equal to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_FTS_AND_GAL,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStartIntTimer);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Failure to send and no buffer  Disconnect Idle
                     space available and realtime   user/dev
                     not equal to
                     GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_FTS_NOT_GAL,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Failed accounting interim answer            Open
                     received and realtime equal
                     to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStartIntTimer);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Failed accounting interim answer Disconnect Idle
                     received and realtime not        user/dev
                     equal to GRANT_AND_LOSE
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  User service terminated        Store      PendingI
                                                    stop
                                                    record
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                           AAA_SESSION_ACCT_EV_STOP,
                           AAA_SESSION_ACCT_ST_PENDING_I,
                           m_acStoreStopRecord);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingI  Any                            None       PendingI
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_I,
                                   AAA_SESSION_ACCT_ST_PENDING_I);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingE  Successful accounting                     Idle
                     event answer received
	 */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_E,
                           AAA_SESSION_ACCT_EV_RX_ACA_OK,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACASuccess);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingE  Failure to send and buffer     Store      Idle
                     space available                event
                                                    record
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_E,
                           AAA_SESSION_ACCT_EV_FTS_BUF,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acStoreEventRecord);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingE  Failure to send and no buffer             Idle
                     space available
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_E,
                           AAA_SESSION_ACCT_EV_FTS_NO_BUF,
                           AAA_SESSION_ACCT_ST_IDLE);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingE  Failed accounting event answer            Idle
                     received
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_E,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL,
                           AAA_SESSION_ACCT_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingE  Any                            None       PendingE
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_E,
                                   AAA_SESSION_ACCT_ST_PENDING_E);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingB  Successful accounting answer   Delete     Idle
                     received                       record
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_B,
                           AAA_SESSION_ACCT_EV_RX_ACA_OK,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACASuccess);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingB  Failure to send                           Idle
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_B,
                           AAA_SESSION_ACCT_EV_FTS,
                           AAA_SESSION_ACCT_ST_IDLE);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingB  Failed accounting answer       Delete     Idle
                     received                       record
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_B,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acDeleteRecord);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingB  Any                            None       PendingB
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_B,
                                   AAA_SESSION_ACCT_ST_PENDING_B);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingL  Successful accounting                     Idle
                     stop answer received
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_L,
                           AAA_SESSION_ACCT_EV_RX_ACA_OK,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACASuccess);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingL  Failure to send and buffer     Store      Idle
                     space available                stop
                                                    record
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_L,
                           AAA_SESSION_ACCT_EV_FTS_BUF,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acStoreStopRecord);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingL  Failure to send and no buffer             Idle
                     space available
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_L,
                           AAA_SESSION_ACCT_EV_FTS_NO_BUF,
                           AAA_SESSION_ACCT_ST_IDLE);

	/*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingL  Failed accounting stop answer             Idle
                     received
	*/
        AddStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_L,
                           AAA_SESSION_ACCT_EV_RX_ACA_FAIL,
                           AAA_SESSION_ACCT_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           PendingL  Any                            None       PendingL
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_PENDING_L,
                                   AAA_SESSION_ACCT_ST_PENDING_L);

        InitialState(AAA_SESSION_ACCT_ST_IDLE); 
      }

      static AAA_SessAcctClientStateTable &Instance() {
        return m_AcctClientStateTable;
      }

   private:
      AAA_SessAcctClient_TxACR               m_acTxACR;
      AAA_SessAcctClient_RxACA_Success       m_acRxACASuccess;
      AAA_SessAcctClient_InterimTimout       m_acInterimTimeout;
      AAA_SessAcctClient_StoreStartRecord    m_acStoreStartRecord;
      AAA_SessAcctClient_StoreStopRecord     m_acStoreStopRecord;
      AAA_SessAcctClient_StoreInterimRecord  m_acStoreInterimRecord;
      AAA_SessAcctClient_StoreEventRecord    m_acStoreEventRecord;
      AAA_SessAcctClient_StartIntTimer       m_acStartIntTimer;
      AAA_SessAcctClient_DeleteRecord        m_acDeleteRecord;
      AAA_SessAcctClient_Disconnect          m_acDisconnect;

   private:
      static AAA_SessAcctClientStateTable m_AcctClientStateTable;
};

#endif /* __AAA_SESSION_ACCT_CLIENT_FSM_H__ */

