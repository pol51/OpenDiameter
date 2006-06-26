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

#ifndef __AAA_PEER_FSM_H__
#define __AAA_PEER_FSM_H__

#include "aaa_peer_data.h"

#define AAA_FSM_EVENT_DEBUG 0

typedef enum {
  AAA_PEER_ST_CLOSED,
  AAA_PEER_ST_WAIT_CONN_ACK,
  AAA_PEER_ST_WAIT_I_CEA,
  AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
  AAA_PEER_ST_WAIT_RETURNS,
  AAA_PEER_ST_I_OPEN,
  AAA_PEER_ST_R_OPEN,
  AAA_PEER_ST_CLOSING
} AAA_PEER_ST;

typedef enum {
  AAA_PEER_EV_START,  
  AAA_PEER_EV_STOP,
  AAA_PEER_EV_TIMEOUT,  
  AAA_PEER_EV_CONN_RETRY,  
  AAA_PEER_EV_R_CONN_CER,  
  AAA_PEER_EV_I_RCV_CONN_ACK,  
  AAA_PEER_EV_I_RCV_CONN_NACK,
  AAA_PEER_EV_R_RCV_CEA,  
  AAA_PEER_EV_I_RCV_CEA,  
  AAA_PEER_EV_I_PEER_DISC,  
  AAA_PEER_EV_R_PEER_DISC,  
  AAA_PEER_EV_I_RCV_NON_CEA,  
  AAA_PEER_EV_WIN_ELECTION,  
  AAA_PEER_EV_SEND_MESSAGE,  
  AAA_PEER_EV_R_RCV_MESSAGE,  
  AAA_PEER_EV_I_RCV_MESSAGE,  
  AAA_PEER_EV_R_RCV_DWR,  
  AAA_PEER_EV_I_RCV_DWR,
  AAA_PEER_EV_R_RCV_DWA,  
  AAA_PEER_EV_I_RCV_DWA,
  AAA_PEER_EV_R_RCV_DPR,  
  AAA_PEER_EV_I_RCV_DPR,
  AAA_PEER_EV_R_RCV_CER,
  AAA_PEER_EV_I_RCV_CER,
  AAA_PEER_EV_R_RCV_DPA,
  AAA_PEER_EV_I_RCV_DPA,
  AAA_PEER_EV_WATCHDOG,
} AAA_PEER_EV;

class AAA_PeerFsmException
{
   public:
      typedef enum {
          ALLOC_FAILURE = 0,
          INVALID_ID_TYPE,
      } ERROR_CODE;
    
   public:
      AAA_PeerFsmException(int code, std::string &desc) :
        m_Code(code), m_Description(desc) {
      }
      AAA_PeerFsmException(int code, const char* desc) :
        m_Code(code), m_Description(desc) {
      }
      int &Code() {
          return m_Code;
      }
      std::string &Description() {
          return m_Description;
      }

   private:
      int m_Code;
      std::string m_Description;
};

class AAA_PeerStateMachine;

class AAA_PeerR_ISendConnReq : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_AcceptSendCEA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendCER : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_ConnNack : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Cleanup : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Retry : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_Accept : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Error : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_ProcessCEA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_AcceptElect : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Disconnect : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_DisconnectDPA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendCERElect : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_SendCEA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_SendCEAOpen : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_DisconnectResp : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_DisconnectIOpen : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_Reject : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_DisconnectSendCEA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_SendMessage : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Process : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerProcessDWRSendDWA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_ProcessDWA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendMessage : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendDPR : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_SendDPR : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendDPADisconnect : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerR_SendDPADisconnect : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerI_SendCEA : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_Peer_Watchdog : public AAA_Action<AAA_PeerStateMachine> {
   public:
      virtual void operator()(AAA_PeerStateMachine &fsm);
};

class AAA_PeerStateTable : public AAA_StateTable<AAA_PeerStateMachine>
{
   public:
      AAA_PeerStateTable() {

        // ------------- AAA_PEER_ST_CLOSED ----------------  
        AddStateTableEntry(AAA_PEER_ST_CLOSED,
                           AAA_PEER_EV_START,
                           AAA_PEER_ST_WAIT_CONN_ACK,
                           m_acISendConnReq); 

        AddStateTableEntry(AAA_PEER_ST_CLOSED,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_R_OPEN,
                           m_acRAcceptSendCEA);         

        AddStateTableEntry(AAA_PEER_ST_CLOSED,
                           AAA_PEER_EV_CONN_RETRY,
                           AAA_PEER_ST_CLOSED,
                           m_acRetry); 

        AddWildcardStateTableEntry(AAA_PEER_ST_CLOSED,
                                   AAA_PEER_ST_CLOSED,
                                   m_acCleanup);        
        
        // ------------- AAA_PEER_ST_WAIT_CONN_ACK ----------------  
        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK,
                           AAA_PEER_EV_I_RCV_CONN_ACK,
                           AAA_PEER_ST_WAIT_I_CEA,
                           m_acISendCER);               

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK,
                           AAA_PEER_EV_I_RCV_CONN_NACK,
                           AAA_PEER_ST_CLOSED,
                           m_acConnNack);                

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           m_acRAccept);                

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK,
                           AAA_PEER_EV_TIMEOUT,
                           AAA_PEER_ST_CLOSED,
                           m_acError);                  

        // ------------- AAA_PEER_ST_WAIT_CEA ----------------  
        AddStateTableEntry(AAA_PEER_ST_WAIT_I_CEA,
                           AAA_PEER_EV_I_RCV_CEA,
                           AAA_PEER_ST_I_OPEN,
                           m_acProcessCEA);             

        AddStateTableEntry(AAA_PEER_ST_WAIT_I_CEA,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_WAIT_RETURNS,
                           m_acRAcceptElect);           

        AddStateTableEntry(AAA_PEER_ST_WAIT_I_CEA,
                           AAA_PEER_EV_I_PEER_DISC,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnect);           

        AddStateTableEntry(AAA_PEER_ST_WAIT_I_CEA,
                           AAA_PEER_EV_I_RCV_NON_CEA,  
                           AAA_PEER_ST_CLOSED,
                           m_acError);

        AddStateTableEntry(AAA_PEER_ST_WAIT_I_CEA,
                           AAA_PEER_EV_TIMEOUT,
                           AAA_PEER_ST_CLOSED,         
                           m_acError);

        // ------------- AAA_PEER_ST_WAIT_CONN_ACK_ELECT ----------------  
        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           AAA_PEER_EV_I_RCV_CONN_ACK,
                           AAA_PEER_ST_WAIT_RETURNS,
                           m_acISendCERElect);

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           AAA_PEER_EV_I_RCV_CONN_NACK,
                           AAA_PEER_ST_R_OPEN,
                           m_acRSendCEA);

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           AAA_PEER_EV_R_PEER_DISC,
                           AAA_PEER_ST_WAIT_CONN_ACK,
                           m_acRDisconnectResp);

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           m_acRReject);

        AddStateTableEntry(AAA_PEER_ST_WAIT_CONN_ACK_ELECT,
                           AAA_PEER_EV_TIMEOUT,
                           AAA_PEER_ST_CLOSED,
                           m_acError);

        // ------------- AAA_PEER_ST_WAIT_RETURNS ----------------  
        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_WIN_ELECTION,
                           AAA_PEER_ST_R_OPEN,
                           m_acIDisconnectSendCEA);  

        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_I_PEER_DISC,
                           AAA_PEER_ST_R_OPEN,
                           m_acIDisconnectSendCEA);  

        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_I_RCV_CEA,
                           AAA_PEER_ST_I_OPEN,
                           m_acRDisconnectIOpen);

        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_R_PEER_DISC,
                           AAA_PEER_ST_WAIT_I_CEA,
                           m_acRDisconnectResp);         

        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_WAIT_RETURNS,
                           m_acRReject);

        AddStateTableEntry(AAA_PEER_ST_WAIT_RETURNS,
                           AAA_PEER_EV_TIMEOUT,
                           AAA_PEER_ST_CLOSED,
                           m_acError);              

        // ------------- AAA_PEER_ST_R_OPEN ----------------  
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_SEND_MESSAGE,
                           AAA_PEER_ST_R_OPEN,
                           m_acRSendMessage);

        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_MESSAGE,
                           AAA_PEER_ST_R_OPEN,
                           m_acProcess);

        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_DWR,
                           AAA_PEER_ST_R_OPEN,
                           m_acProcessDWRSendDWA);

        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_DWA,
                           AAA_PEER_ST_R_OPEN,
                           m_acProcessDWA);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_R_OPEN,
                           m_acRReject);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_STOP,
                           AAA_PEER_ST_CLOSING,
                           m_acRSendDPR);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_DPR,
                           AAA_PEER_ST_CLOSED,
                           m_acRSendDPADisconnect);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_PEER_DISC,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnect);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_CER,
                           AAA_PEER_ST_R_OPEN,
                           m_acRSendCEAOpen);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_R_RCV_CEA,
                           AAA_PEER_ST_R_OPEN,
                           m_acProcessCEA);
        
        AddStateTableEntry(AAA_PEER_ST_R_OPEN,
                           AAA_PEER_EV_WATCHDOG,
                           AAA_PEER_ST_R_OPEN,
                           m_acWatchdog);
        
        // ------------- AAA_PEER_ST_I_OPEN ----------------  
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_SEND_MESSAGE,
                           AAA_PEER_ST_I_OPEN,
                           m_acISendMessage);

        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_MESSAGE,
                           AAA_PEER_ST_I_OPEN,
                           m_acProcess);

        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_DWR,
                           AAA_PEER_ST_I_OPEN,
                           m_acProcessDWRSendDWA);

        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_DWA,
                           AAA_PEER_ST_I_OPEN,
                           m_acProcessDWA);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_R_CONN_CER,
                           AAA_PEER_ST_I_OPEN,
                           m_acRReject);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_STOP,
                           AAA_PEER_ST_CLOSING,
                           m_acISendDPR);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_DPR,
                           AAA_PEER_ST_CLOSED,
                           m_acISendDPADisconnect);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_PEER_DISC,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnect);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_CER,
                           AAA_PEER_ST_I_OPEN,
                           m_acISendCEA);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_I_RCV_CEA,
                           AAA_PEER_ST_I_OPEN,
                           m_acProcessCEA);
        
        AddStateTableEntry(AAA_PEER_ST_I_OPEN,
                           AAA_PEER_EV_WATCHDOG,
                           AAA_PEER_ST_I_OPEN,
                           m_acWatchdog);
        
        // ------------- AAA_PEER_ST_CLOSING ----------------  
        AddStateTableEntry(AAA_PEER_ST_CLOSING,
                           AAA_PEER_EV_I_RCV_DPA,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnectDPA);
        
        AddStateTableEntry(AAA_PEER_ST_CLOSING,
                           AAA_PEER_EV_R_RCV_DPA,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnectDPA);
        
        AddStateTableEntry(AAA_PEER_ST_CLOSING,
                           AAA_PEER_EV_TIMEOUT,
                           AAA_PEER_ST_CLOSED,
                           m_acError);
        
        AddStateTableEntry(AAA_PEER_ST_CLOSING,
                           AAA_PEER_EV_I_PEER_DISC,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnect);
        
        AddStateTableEntry(AAA_PEER_ST_CLOSING,
                           AAA_PEER_EV_R_PEER_DISC,
                           AAA_PEER_ST_CLOSED,
                           m_acDisconnect);
        
        AddWildcardStateTableEntry(AAA_PEER_ST_CLOSING,
                                   AAA_PEER_ST_CLOSING);
        
        InitialState(AAA_PEER_ST_CLOSED);
      }

   private:
      AAA_PeerR_ISendConnReq       m_acISendConnReq;
      AAA_PeerR_AcceptSendCEA      m_acRAcceptSendCEA;
      AAA_PeerI_SendCER            m_acISendCER;
      AAA_Peer_ConnNack            m_acConnNack;
      AAA_Peer_Cleanup             m_acCleanup;
      AAA_Peer_Retry               m_acRetry;
      AAA_PeerR_Accept             m_acRAccept;
      AAA_Peer_Error               m_acError;
      AAA_Peer_ProcessCEA          m_acProcessCEA;
      AAA_PeerR_AcceptElect        m_acRAcceptElect;
      AAA_Peer_Disconnect          m_acDisconnect;
      AAA_Peer_DisconnectDPA       m_acDisconnectDPA;
      AAA_PeerI_SendCERElect       m_acISendCERElect;
      AAA_PeerR_SendCEA            m_acRSendCEA;
      AAA_PeerR_SendCEAOpen        m_acRSendCEAOpen;
      AAA_PeerR_DisconnectResp     m_acRDisconnectResp;
      AAA_PeerR_DisconnectIOpen    m_acRDisconnectIOpen;
      AAA_PeerR_Reject             m_acRReject;
      AAA_PeerI_DisconnectSendCEA  m_acIDisconnectSendCEA;
      AAA_PeerR_SendMessage        m_acRSendMessage;
      AAA_Peer_Process             m_acProcess;
      AAA_PeerProcessDWRSendDWA    m_acProcessDWRSendDWA;
      AAA_Peer_ProcessDWA          m_acProcessDWA;
      AAA_PeerI_SendDPR            m_acISendDPR;
      AAA_PeerR_SendDPR            m_acRSendDPR;
      AAA_PeerI_SendDPADisconnect  m_acISendDPADisconnect;
      AAA_PeerR_SendDPADisconnect  m_acRSendDPADisconnect;
      AAA_PeerI_SendMessage        m_acISendMessage;
      AAA_PeerI_SendCEA            m_acISendCEA;
      AAA_Peer_Watchdog            m_acWatchdog; 
};

class AAA_PeerStateMachine :
    public AAA_StateMachineWithTimer<AAA_PeerStateMachine>,
           AAA_Job
{
   public:
      AAA_PeerData &Data() { 
         return m_Data; 
      }          
      AAA_GroupedJob &Job() {
          return *m_GroupedJob.get();
      }    
      virtual int Send(std::auto_ptr<AAAMessage> &msg) {
          ///  If using ASYNC SEND
          ///  EnqueueSendMsg(msg);
          ///  Notify(AAA_PEER_EV_SEND_MESSAGE);
          if (! AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
             AAA_LOG(LM_INFO, "(%P|%t) Cannot send message, statemachine not running\n");
             return (-1);
          }
          switch (state) {
              case AAA_PEER_ST_I_OPEN:
                  return RawSend(msg, m_Data.m_IOInitiator.get());
              case AAA_PEER_ST_R_OPEN:
                  return RawSend(msg, m_Data.m_IOResponder.get());
              default:
                  AAA_LOG(LM_INFO, "(%P|%t) Discarding msg to send, peer state is not open\n");
                  break;
          }
          return (0);
      }      

   private: // Event Queue

      typedef struct {
         AAA_Event m_Event;
         std::auto_ptr<AAAMessage> m_Msg;
         std::auto_ptr<AAA_IO_Base> m_IO;
      } AAA_PeerEventParam;

      AAA_ProtectedPtrQueue<AAA_PeerEventParam> m_EventQueue;
      std::auto_ptr<AAA_PeerEventParam> m_CurrentPeerEventParam;

   public:

      virtual void Notify(AAA_Event event) {
         if (AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
             std::auto_ptr<AAA_PeerEventParam> e(new AAA_PeerEventParam);
             e->m_Event = event;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG(LM_INFO, "(%P|%t) Event not processed, statemachine not running\n");
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<AAAMessage> msg) {
         if (AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
             std::auto_ptr<AAA_PeerEventParam> e(new AAA_PeerEventParam);
             e->m_Event = event;
             e->m_Msg = msg;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG(LM_INFO, "(%P|%t) Event not processed, statemachine not running\n");
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<AAA_IO_Base> io) {
         if (AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
             std::auto_ptr<AAA_PeerEventParam> e(new AAA_PeerEventParam);
             e->m_Event = event;
             e->m_IO = io;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG(LM_INFO, "(%P|%t) Event not processed, statemachine not running\n");
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<AAAMessage> msg,
                          std::auto_ptr<AAA_IO_Base> io) {
         if (AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
             std::auto_ptr<AAA_PeerEventParam> e(new AAA_PeerEventParam);
             e->m_Event = event;
             e->m_Msg = msg;
             e->m_IO = io;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG(LM_INFO, "(%P|%t) Event not processed, statemachine not running\n");
         }
      }
      
   protected: // Constructor/Destructor
 
      AAA_PeerStateMachine(AAA_Task &t) :
          AAA_StateMachineWithTimer<AAA_PeerStateMachine>
             (*this, m_StateTable, *t.reactor()),
             m_GroupedJob(&t.Job()) {
      }
      virtual ~AAA_PeerStateMachine() {
          WaitOnCleanup();
          AAA_StateMachine<AAA_PeerStateMachine>::Stop();
      }    

   protected: // Notifications
      virtual void PeerFsmConnected() {
      }
      virtual void PeerFsmError(int resultCode) {
      }
      virtual void PeerFsmDisconnected(int cause) {
      }
      
   protected: // Job servicing

      ACE_Mutex m_EventFsmMtx;
      virtual int Serve() {
         AAA_MutexScopeLock guard(m_EventFsmMtx);
         m_CurrentPeerEventParam = m_EventQueue.Dequeue();
         DumpEvent(m_CurrentPeerEventParam->m_Event, "Pre-Event");
         try {
             AAA_StateMachineWithTimer
                 <AAA_PeerStateMachine>::Event
                        (m_CurrentPeerEventParam->m_Event);
         }
         catch (AAA_PeerFsmException &err) {
             AAA_LOG(LM_ERROR, "(%P|%t) FSM error[%d]: %s\n",
                        err.Code(), err.Description().data());
         }
         catch (...) {
             AAA_LOG(LM_ERROR, "(%P|%t) Unknown exception in FSM\n");
         }
         DumpEvent(m_CurrentPeerEventParam->m_Event, "Post-Event");
         m_CurrentPeerEventParam.reset();
         return (0);
      }
      virtual int Schedule(AAA_Job* job, size_t backlogSize = 1) {
         if (! AAA_StateMachineWithTimer
             <AAA_PeerStateMachine>::Running()) {
            return (-1);
         }
         return m_GroupedJob->Schedule(job, backlogSize);
      }
      virtual void Timeout(AAA_Event ev) {
         Notify(ev);
      }
      AAA_PeerData &PeerData() {
         return m_Data;
      }
      int RawSend(std::auto_ptr<AAAMessage> &msg, AAA_IO_Base *io);    

   protected: // Capabilities exchange

      virtual void SendCER();
      virtual void SendCEA(diameter_unsigned32_t rcode,
                           std::string &message);
    
      void AssembleCE(AAAMessage &msg,
                      bool request = true);
      void DisassembleCE(AAAMessage &msg);
      bool ValidatePeer(diameter_unsigned32_t &rcode,
                        std::string &message);

   protected: // Watchdog

      virtual void SendDWR();
      virtual void SendDWA(diameter_unsigned32_t rcode,
                           std::string &message);
    
      void AssembleDW(AAAMessage &msg,
                      bool request = true);
      void DisassembleDW(AAAMessage &msg);
    
   protected: // Disconnection

      virtual void SendDPR(bool initiator);
      virtual void SendDPA(bool initiator,
                           diameter_unsigned32_t rcode,
                           std::string &message);
      void AssembleDP(AAAMessage &msg,
                      bool request = true);
      void DisassembleDP(AAAMessage &msg);

   protected: // Message Id's

      void MsgIdTxMessage(AAAMessage &msg);
      bool MsgIdRxMessage(AAAMessage &msg);
   
   protected: // Auxillary

      void Elect();
    
      typedef enum {
          CLEANUP_IO_I = 0x00000001,
          CLEANUP_IO_R = 0x00000002,
          CLEANUP_FSM  = 0x00000004,
          CLEANUP_ALL  = 0xffffffff
      } CLEANUP_FLG;    
      virtual void Cleanup(unsigned int flags = CLEANUP_ALL);

      bool m_CleanupEvent;
      void WaitOnCleanup() {
         m_CleanupEvent = (state != AAA_PEER_ST_CLOSED) ? 
                          false : true;
         do {
             ACE_Time_Value tv(0, 100000);
             ACE_OS::sleep(tv);
         } while (! m_CleanupEvent);
      }
    
   protected:
    
      void DumpPeerCapabilities();
      void DumpEvent(AAA_Event ev, char *prefix) {
#if AAA_FSM_EVENT_DEBUG
          static char *evStrTable[] = { "AAA_PEER_EV_START",  
                                        "AAA_PEER_EV_STOP",
                                        "AAA_PEER_EV_TIMEOUT",  
                                        "AAA_PEER_EV_CONN_RETRY",  
                                        "AAA_PEER_EV_R_CONN_CER",  
                                        "AAA_PEER_EV_I_RCV_CONN_ACK",  
                                        "AAA_PEER_EV_I_RCV_CONN_NACK",
                                        "AAA_PEER_EV_R_RCV_CEA",  
                                        "AAA_PEER_EV_I_RCV_CEA",  
                                        "AAA_PEER_EV_I_PEER_DISC",  
                                        "AAA_PEER_EV_R_PEER_DISC",  
                                        "AAA_PEER_EV_I_RCV_NON_CEA",  
                                        "AAA_PEER_EV_WIN_ELECTION",  
                                        "AAA_PEER_EV_SEND_MESSAGE",  
                                        "AAA_PEER_EV_R_RCV_MESSAGE",  
                                        "AAA_PEER_EV_I_RCV_MESSAGE",  
                                        "AAA_PEER_EV_R_RCV_DWR",  
                                        "AAA_PEER_EV_I_RCV_DWR",
                                        "AAA_PEER_EV_R_RCV_DWA",  
                                        "AAA_PEER_EV_I_RCV_DWA",
                                        "AAA_PEER_EV_R_RCV_DPR",  
                                        "AAA_PEER_EV_I_RCV_DPR",
                                        "AAA_PEER_EV_R_RCV_CER",
                                        "AAA_PEER_EV_I_RCV_CER",
                                        "AAA_PEER_EV_R_RCV_DPA",
                                        "AAA_PEER_EV_I_RCV_DPA",
                                        "AAA_PEER_EV_WATCHDOG"
          };
          static char *stStrTable[] = { "AAA_PEER_ST_CLOSED",
                                        "AAA_PEER_ST_WAIT_CONN_ACK",
                                        "AAA_PEER_ST_WAIT_I_CEA",
                                        "AAA_PEER_ST_WAIT_CONN_ACK_ELECT",
                                        "AAA_PEER_ST_WAIT_RETURNS",
                                        "AAA_PEER_ST_I_OPEN",
                                        "AAA_PEER_ST_R_OPEN",
                                        "AAA_PEER_ST_CLOSING"
          };
          AAA_LOG(LM_INFO, "(%P|%t) FSM EVENT DEBUG [state=%s, event=%s]: %s\n",
                    stStrTable[state], evStrTable[ev], prefix);
#endif // AAA_FSM_EVENT_DEBUG
      }

    public:
      friend class AAA_PeerR_AcceptSendCEA;
      friend class AAA_PeerI_SendCER;
      friend class AAA_Peer_Cleanup;
      friend class AAA_Peer_ConnNack;
      friend class AAA_PeerR_Accept;
      friend class AAA_Peer_Error;
      friend class AAA_Peer_ProcessCEA;
      friend class AAA_PeerR_AcceptElect;
      friend class AAA_Peer_Disconnect;
      friend class AAA_Peer_DisconnectDPA;
      friend class AAA_PeerI_SendCERElect;
      friend class AAA_PeerR_SendCEA;
      friend class AAA_PeerR_SendCEAOpen;
      friend class AAA_PeerR_DisconnectResp;
      friend class AAA_PeerR_DisconnectIOpen;
      friend class AAA_PeerR_Reject;
      friend class AAA_PeerI_DisconnectSendCEA;
      friend class AAA_PeerR_SendMessage;
      friend class AAA_Peer_Process;
      friend class AAA_PeerProcessDWRSendDWA;
      friend class AAA_Peer_ProcessDWA;
      friend class AAA_PeerI_SendDPR;
      friend class AAA_PeerR_SendDPR;
      friend class AAA_PeerI_SendDPADisconnect;
      friend class AAA_PeerR_SendDPADisconnect;
      friend class AAA_PeerI_SendMessage;
      friend class AAA_PeerI_SendCEA;
      friend class AAA_Peer_Watchdog;
    
   private:
    
      AAA_JobHandle<AAA_GroupedJob> m_GroupedJob;    
    
      AAA_PeerData m_Data;
      static AAA_PeerStateTable m_StateTable;
};

#endif /* __AAA_PEER_FSM_H__ */

