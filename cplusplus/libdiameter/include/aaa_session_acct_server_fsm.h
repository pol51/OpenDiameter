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

#ifndef __AAA_SESSION_ACCT_SERVER_FSM_H__
#define __AAA_SESSION_ACCT_SERVER_FSM_H__

#include "aaa_session_acct_fsm.h"

///
/// REC_STORAGE MUST implement this class
/// This class provides callback functionality
/// to applications with regards to record
/// storage.
///
class DiameterServerAcctRecStorage
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
        virtual void UpdateAcctResponse(DiameterMsg &aca) = 0;

    protected:
        virtual ~DiameterServerAcctRecStorage() { }
};

///
/// REC_STORAGE specialization that provides user convertion
///
template <class T>
class DiameterServerAcctRecConverter
{
    public:
        virtual T *Convert(AAAAvpContainerList &avpList,
                           int recordType,
                           int recordNum) = 0;
        virtual void Output(T &record) = 0;
        virtual ~DiameterServerAcctRecConverter() { }
};

template <class T>
class DiameterServerAcctRecStorageWithConverter :
    public DiameterServerAcctRecStorage,
    public DiameterServerAcctRecConverter<T>
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

class DIAMETERBASEPROTOCOL_EXPORT DiameterAcctSessionServerStateMachine :
   public DiameterAcctSessionStateMachine
           <DiameterAcctSessionServerStateMachine>
{  
   public:
      DiameterAcctSessionServerStateMachine(AAA_Task &t,
                                        DiameterAcctSession &a,
                                        DiameterServerAcctRecStorage &s);
      virtual ~DiameterAcctSessionServerStateMachine() {
      }
      DiameterServerAcctRecStorage &RecStorage() {
        return m_RecStorage;
      }

      void RxACR(DiameterMsg &msg);
      void TxACA(diameter_unsigned32_t rcode);

   protected:
      DiameterServerAcctRecStorage &m_RecStorage;
};

class DiameterSessAcctServer_RxACR_Start : 
   public AAA_Action<DiameterAcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAcctSessionServerStateMachine &fsm) {
         std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
         fsm.CancelAllTimer();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.ScheduleTimer(DIAMETER_SESSION_ACCT_EV_SESSION_TOUT,
                           fsm.Attributes().SessionTimeout()(),
                           0, DIAMETER_TIMER_TYPE_SESSION);
         fsm.Session().Success();
      }
};

class DiameterSessAcctServer_RxACR_Stop : 
   public AAA_Action<DiameterAcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAcctSessionServerStateMachine &fsm) {
         std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
         fsm.CancelAllTimer();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.Session().Success();
         fsm.Session().Reset();
      }
};

class DiameterSessAcctServer_RxACR : 
   public AAA_Action<DiameterAcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAcctSessionServerStateMachine &fsm) {
         std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
         fsm.RecStorage().StoreRecord(msg->acl, 
                                      fsm.Attributes().RecordType()(),
                                      fsm.Attributes().RecordNumber()());
         fsm.TxACA(AAA_SUCCESS);
         fsm.Session().Success();
      }
};

class DiameterSessAcctServer_RxACR_OutofSpace : 
   public AAA_Action<DiameterAcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAcctSessionServerStateMachine &fsm) {
         fsm.TxACA(AAA_OUT_OF_SPACE);
         fsm.Session().Failed(fsm.Attributes().RecordNumber()());
         fsm.Session().Reset();
      }
};

class DiameterSessAcctServer_SessionTimeout : 
   public AAA_Action<DiameterAcctSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAcctSessionServerStateMachine &fsm) {
         fsm.CancelAllTimer();
         fsm.Session().SessionTimeout();
         fsm.Session().Reset();
      }
};

class DiameterSessAcctServerStateTable : 
   public AAA_StateTable<DiameterAcctSessionServerStateMachine>
{
   public:
      DiameterSessAcctServerStateTable() {

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Accounting start request       Send       Open
                     received, and successfully     accounting
                     processed.                     start
                                                    answer,
                                                    Start Ts
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_IDLE,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_START_OK,
                           DIAMETER_SESSION_ACCT_ST_OPEN,
                           m_acStart); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Accounting event request       Send       Idle
                     received, and successfully     accounting
                     processed.                     event
                                                    answer
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_IDLE,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_EV_OK,
                           DIAMETER_SESSION_ACCT_ST_IDLE,
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
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_IDLE,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_NO_BUF,
                           DIAMETER_SESSION_ACCT_ST_IDLE,
                           m_acRxACROutOfSpace); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle     Any                            None       Idle   
         */
        AddWildcardStateTableEntry(DIAMETER_SESSION_ACCT_ST_IDLE,
                                   DIAMETER_SESSION_ACCT_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Interim record received,       Send       Open
                     and successfully processed.    accounting
                                                    interim
                                                    answer,
                                                    Restart Ts
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_INT_OK,
                           DIAMETER_SESSION_ACCT_ST_OPEN,
                           m_acStart); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting stop request        Send       Idle
                     received, and successfully     accounting
                     processed                      stop answer,
                                                    Stop Ts
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_STOP_OK,
                           DIAMETER_SESSION_ACCT_ST_IDLE,
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
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_NO_BUF,
                           DIAMETER_SESSION_ACCT_ST_IDLE,
                           m_acRxACROutOfSpace); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Session supervision timer Ts   Stop Ts    Idle
                     expired
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_SESSION_TOUT,
                           DIAMETER_SESSION_ACCT_ST_IDLE,
                           m_acTimeout); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Accounting event request       Send       Open
                     received, and successfully     accounting
                     processed.                     event
                                                    answer
        */
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_EV_OK,
                           DIAMETER_SESSION_ACCT_ST_OPEN,
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
        AddStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                           DIAMETER_SESSION_ACCT_EV_RX_ACR_START_OK,
                           DIAMETER_SESSION_ACCT_ST_OPEN,
                           m_acStart); 
                           
        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open     Any                            None       Open   
         */
        AddWildcardStateTableEntry(DIAMETER_SESSION_ACCT_ST_OPEN,
                                   DIAMETER_SESSION_ACCT_ST_OPEN);

        InitialState(DIAMETER_SESSION_ACCT_ST_IDLE);
      }

      static DiameterSessAcctServerStateTable &Instance() {
        return m_AcctServerStateTable;
      }

   private:
       DiameterSessAcctServer_RxACR_Start               m_acStart;
       DiameterSessAcctServer_RxACR                     m_acRxACR;
       DiameterSessAcctServer_RxACR_OutofSpace          m_acRxACROutOfSpace;
       DiameterSessAcctServer_RxACR_Stop                m_acStop;
       DiameterSessAcctServer_SessionTimeout            m_acTimeout;

   private:
      static DiameterSessAcctServerStateTable m_AcctServerStateTable;
};

#endif /* __AAA_SESSION_ACCT_SERVER_FSM_H__ */

