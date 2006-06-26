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

#ifndef __AAA_SESSION_ACCT_SERVER_FSM_H__
#define __AAA_SESSION_ACCT_SERVER_FSM_H__

#include "aaa_session_acct_fsm.h"

///
/// REC_STORAGE MUST implement this class
/// This class provides callback functionality
/// to applications with regards to record
/// storage.
///
class AAA_ServerAcctRecStorage
{
    public:
	/// Checks the server app if there is enough storage space
        virtual bool IsSpaceAvailableOnDevice() = 0;

        /// Asks the server app to store record
        virtual void StoreRecord(AAAAvpContainerList &avpList,
                                 int recordType,
                                 int recordNum) = 0;

        /// Asks the server app to update the ACA message  
        /// before it is sent to the client session
        virtual void UpdateAcctResponse(AAAMessage &aca) = 0;

    protected:
        virtual ~AAA_ServerAcctRecStorage() { }
};

///
/// REC_STORAGE specialization that provides user convertion
///
template <class T>
class AAA_ServerAcctRecConverter
{
    public:
        virtual T *Convert(AAAAvpContainerList &avpList,
                           int recordType,
                           int recordNum) = 0;
        virtual void Output(T &record) = 0;
        virtual ~AAA_ServerAcctRecConverter() { }
};

template <class T>
class AAA_ServerAcctRecStorageWithConverter :
    public AAA_ServerAcctRecStorage,
    public AAA_ServerAcctRecConverter<T>
{
    public:
        /// Asks the server app to store record
        virtual void StoreRecord(AAAAvpContainerList &avpList,
                                 int recordType,
                                 int recordNum) {
           T *userRec = this->Convert(avpList, recordType, recordNum);
           if (userRec) {
               this->Output(*userRec);
	   }
	}
};

class DIAMETERBASEPROTOCOL_EXPORT AAA_AcctSessionServerStateMachine :
   public AAA_AcctSessionStateMachine
           <AAA_AcctSessionServerStateMachine>
{  
   public:
      AAA_AcctSessionServerStateMachine(AAA_Task &t,
                                        AAA_AcctSession &a,
                                        AAA_ServerAcctRecStorage &s);
      virtual ~AAA_AcctSessionServerStateMachine() {
      }
      AAA_ServerAcctRecStorage &RecStorage() {
        return m_RecStorage;
      }

      void RxACR(AAAMessage &msg);
      void TxACA(diameter_unsigned32_t rcode);

   protected:
      AAA_ServerAcctRecStorage &m_RecStorage;
};

class AAA_SessAcctServer_RxACR_Start : 
   public AAA_Action<AAA_AcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionServerStateMachine &fsm) {
         std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
         fsm.CancelAllTimer();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.ScheduleTimer(AAA_SESSION_ACCT_EV_SESSION_TOUT,
                           fsm.Attributes().SessionTimeout()(),
                           0, AAA_TIMER_TYPE_SESSION);
         fsm.Session().Success();
      }
};

class AAA_SessAcctServer_RxACR_Stop : 
   public AAA_Action<AAA_AcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionServerStateMachine &fsm) {
         std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
         fsm.CancelAllTimer();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.Session().Success();
         fsm.Session().Reset();
      }
};

class AAA_SessAcctServer_RxACR : 
   public AAA_Action<AAA_AcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionServerStateMachine &fsm) {
         std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.Session().Success();
      }
};

class AAA_SessAcctServer_RxACR_OutofSpace : 
   public AAA_Action<AAA_AcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionServerStateMachine &fsm) {
         fsm.TxACA(AAA_OUT_OF_SPACE);
         fsm.Session().Failed(fsm.Attributes().RecordNumber()());
         fsm.Session().Reset();
      }
};

class AAA_SessAcctServer_SessionTimeout : 
   public AAA_Action<AAA_AcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AcctSessionServerStateMachine &fsm) {
         fsm.CancelAllTimer();
         fsm.Session().SessionTimeout();
         fsm.Session().Reset();
      }
};

class AAA_SessAcctServerStateTable : 
   public AAA_StateTable<AAA_AcctSessionServerStateMachine>
{
   public:
      AAA_SessAcctServerStateTable() {

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Accounting start request       Send       Open
                     received, and successfully     accounting
                     processed.                     start
                                                    answer,
                                                    Start Ts
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_RX_ACR_START_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStart); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Accounting event request       Send       Idle
                     received, and successfully     accounting
                     processed.                     event
                                                    answer
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_RX_ACR_EV_OK,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACR); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Accounting request received,   Send       Idle
                     no space left to store         accounting
                     records                        answer,
                                                    Result-Code
                                                    = OUT_OF_
                                                    SPACE
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_IDLE,
                           AAA_SESSION_ACCT_EV_RX_ACR_NO_BUF,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACROutOfSpace); 

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
           Open      Interim record received,       Send       Open
                     and successfully processed.    accounting
                                                    interim
                                                    answer,
                                                    Restart Ts
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_RX_ACR_INT_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStart); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting stop request        Send       Idle
                     received, and successfully     accounting
                     processed                      stop answer,
                                                    Stop Ts
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_RX_ACR_STOP_OK,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acStop); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting request received,   Send       Idle
                     no space left to store         accounting
                     records                        answer,
                                                    Result-Code
                                                    = OUT_OF_
                                                    SPACE,
                                                    Stop Ts
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_RX_ACR_NO_BUF,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acRxACROutOfSpace); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Session supervision timer Ts   Stop Ts    Idle
                     expired
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_SESSION_TOUT,
                           AAA_SESSION_ACCT_ST_IDLE,
                           m_acTimeout); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting event request       Send       Open
                     received, and successfully     accounting
                     processed.                     event
                                                    answer
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_RX_ACR_EV_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acRxACR); 

        /* 
           ---- This is to support single session on server side -------
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting start request       Send       Open
                     received, and successfully     accounting
                     processed.                     start
                                                    answer,
                                                    Start Ts
        */
        AddStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                           AAA_SESSION_ACCT_EV_RX_ACR_START_OK,
                           AAA_SESSION_ACCT_ST_OPEN,
                           m_acStart); 
                           
        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open     Any                            None       Open   
         */
        AddWildcardStateTableEntry(AAA_SESSION_ACCT_ST_OPEN,
                                   AAA_SESSION_ACCT_ST_OPEN);

        InitialState(AAA_SESSION_ACCT_ST_IDLE);
      }

      static AAA_SessAcctServerStateTable &Instance() {
        return m_AcctServerStateTable;
      }

   private:
       AAA_SessAcctServer_RxACR_Start               m_acStart;
       AAA_SessAcctServer_RxACR                     m_acRxACR;
       AAA_SessAcctServer_RxACR_OutofSpace          m_acRxACROutOfSpace;
       AAA_SessAcctServer_RxACR_Stop                m_acStop;
       AAA_SessAcctServer_SessionTimeout            m_acTimeout;

   private:
      static AAA_SessAcctServerStateTable m_AcctServerStateTable;
};

#endif /* __AAA_SESSION_ACCT_SERVER_FSM_H__ */

