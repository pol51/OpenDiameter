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

#ifndef __AAA_PEER_FSM_H__
#define __AAA_PEER_FSM_H__

#include "aaa_peer_data.h"

typedef enum {
  DIAMETER_PEER_ST_CLOSED,
  DIAMETER_PEER_ST_WAIT_CONN_ACK,
  DIAMETER_PEER_ST_WAIT_I_CEA,
  DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
  DIAMETER_PEER_ST_WAIT_RETURNS,
  DIAMETER_PEER_ST_I_OPEN,
  DIAMETER_PEER_ST_R_OPEN,
  DIAMETER_PEER_ST_CLOSING
} DIAMETER_PEER_ST;

typedef enum {
  DIAMETER_PEER_EV_START,  
  DIAMETER_PEER_EV_STOP,
  DIAMETER_PEER_EV_TIMEOUT,  
  DIAMETER_PEER_EV_CONN_RETRY,  
  DIAMETER_PEER_EV_R_CONN_CER,  
  DIAMETER_PEER_EV_I_RCV_CONN_ACK,  
  DIAMETER_PEER_EV_I_RCV_CONN_NACK,
  DIAMETER_PEER_EV_R_RCV_CEA,  
  DIAMETER_PEER_EV_I_RCV_CEA,  
  DIAMETER_PEER_EV_I_PEER_DISC,  
  DIAMETER_PEER_EV_R_PEER_DISC,  
  DIAMETER_PEER_EV_I_RCV_NON_CEA,  
  DIAMETER_PEER_EV_WIN_ELECTION,  
  DIAMETER_PEER_EV_SEND_MESSAGE,  
  DIAMETER_PEER_EV_R_RCV_MESSAGE,  
  DIAMETER_PEER_EV_I_RCV_MESSAGE,  
  DIAMETER_PEER_EV_R_RCV_DWR,  
  DIAMETER_PEER_EV_I_RCV_DWR,
  DIAMETER_PEER_EV_R_RCV_DWA,  
  DIAMETER_PEER_EV_I_RCV_DWA,
  DIAMETER_PEER_EV_R_RCV_DPR,  
  DIAMETER_PEER_EV_I_RCV_DPR,
  DIAMETER_PEER_EV_R_RCV_CER,
  DIAMETER_PEER_EV_I_RCV_CER,
  DIAMETER_PEER_EV_R_RCV_DPA,
  DIAMETER_PEER_EV_I_RCV_DPA,
  DIAMETER_PEER_EV_WATCHDOG,
} DIAMETER_PEER_EV;

class DiameterPeerFsmException
{
   public:
      typedef enum {
          ALLOC_FAILURE = 0,
          INVALID_ID_TYPE,
      } ERROR_CODE;
    
   public:
      DiameterPeerFsmException(int code, std::string &desc) :
        m_Code(code), m_Description(desc) {
      }
      DiameterPeerFsmException(int code, const char* desc) :
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

class DiameterPeerStateMachine;

class DiameterPeerR_ISendConnReq : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_AcceptSendCEA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendCER : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_ConnNack : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_Cleanup : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_ReConnect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_Accept : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_Error : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_ProcessCEA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_AcceptElect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_Disconnect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_DisconnectDPA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendCERElect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_SendCEA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_SendCEAOpen : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_DisconnectResp : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_DisconnectIOpen : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_Reject : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_DisconnectSendCEA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_SendMessage : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_Process : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerProcessDWRSendDWA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_ProcessDWA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendMessage : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendDPR : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_SendDPR : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendDPADisconnect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerR_SendDPADisconnect : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerI_SendCEA : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeer_Watchdog : public AAA_Action<DiameterPeerStateMachine> {
   public:
      virtual void operator()(DiameterPeerStateMachine &fsm);
};

class DiameterPeerStateTable : public AAA_StateTable<DiameterPeerStateMachine>
{
   public:
      DiameterPeerStateTable() {

        // ------------- DIAMETER_PEER_ST_CLOSED ----------------
        AddStateTableEntry(DIAMETER_PEER_ST_CLOSED,
                           DIAMETER_PEER_EV_START,
                           DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           m_acISendConnReq);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSED,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acRAcceptSendCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSED,
                           DIAMETER_PEER_EV_CONN_RETRY,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acRetry);

        AddWildcardStateTableEntry(DIAMETER_PEER_ST_CLOSED,
                                   DIAMETER_PEER_ST_CLOSED,
                                   m_acCleanup);

        // ------------- DIAMETER_PEER_ST_WAIT_CONN_ACK ----------------
        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           DIAMETER_PEER_EV_I_RCV_CONN_ACK,
                           DIAMETER_PEER_ST_WAIT_I_CEA,
                           m_acISendCER);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           DIAMETER_PEER_EV_I_RCV_CONN_NACK,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acConnNack);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           m_acRAccept);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           DIAMETER_PEER_EV_TIMEOUT,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           DIAMETER_PEER_EV_STOP,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        // ------------- DIAMETER_PEER_ST_WAIT_CEA ----------------  
        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_I_RCV_CEA,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acProcessCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_WAIT_RETURNS,
                           m_acRAcceptElect);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_I_PEER_DISC,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_I_RCV_NON_CEA,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_TIMEOUT,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_I_CEA,
                           DIAMETER_PEER_EV_STOP,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        // ------------- DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT ----------------
        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           DIAMETER_PEER_EV_I_RCV_CONN_ACK,
                           DIAMETER_PEER_ST_WAIT_RETURNS,
                           m_acISendCERElect);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           DIAMETER_PEER_EV_I_RCV_CONN_NACK,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acRSendCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           DIAMETER_PEER_EV_R_PEER_DISC,
                           DIAMETER_PEER_ST_WAIT_CONN_ACK,
                           m_acRDisconnectResp);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           m_acRReject);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT,
                           DIAMETER_PEER_EV_TIMEOUT,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        // ------------- DIAMETER_PEER_ST_WAIT_RETURNS ----------------  
        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_WIN_ELECTION,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acIDisconnectSendCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_I_PEER_DISC,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acIDisconnectSendCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_I_RCV_CEA,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acRDisconnectIOpen);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_R_PEER_DISC,
                           DIAMETER_PEER_ST_WAIT_I_CEA,
                           m_acRDisconnectResp);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_WAIT_RETURNS,
                           m_acRReject);

        AddStateTableEntry(DIAMETER_PEER_ST_WAIT_RETURNS,
                           DIAMETER_PEER_EV_TIMEOUT,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        // ------------- DIAMETER_PEER_ST_R_OPEN ----------------  
        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_SEND_MESSAGE,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acRSendMessage);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_MESSAGE,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acProcess);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_DWR,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acProcessDWRSendDWA);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_DWA,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acProcessDWA);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acRReject);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_STOP,
                           DIAMETER_PEER_ST_CLOSING,
                           m_acRSendDPR);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_DPR,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acRSendDPADisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_PEER_DISC,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_CER,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acRSendCEAOpen);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_R_RCV_CEA,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acProcessCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_R_OPEN,
                           DIAMETER_PEER_EV_WATCHDOG,
                           DIAMETER_PEER_ST_R_OPEN,
                           m_acWatchdog);

        // ------------- DIAMETER_PEER_ST_I_OPEN ----------------  
        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_SEND_MESSAGE,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acISendMessage);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_MESSAGE,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acProcess);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_DWR,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acProcessDWRSendDWA);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_DWA,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acProcessDWA);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_R_CONN_CER,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acRReject);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_STOP,
                           DIAMETER_PEER_ST_CLOSING,
                           m_acISendDPR);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_DPR,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acISendDPADisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_PEER_DISC,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_CER,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acISendCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_I_RCV_CEA,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acProcessCEA);

        AddStateTableEntry(DIAMETER_PEER_ST_I_OPEN,
                           DIAMETER_PEER_EV_WATCHDOG,
                           DIAMETER_PEER_ST_I_OPEN,
                           m_acWatchdog);

        // ------------- DIAMETER_PEER_ST_CLOSING ----------------  
        AddStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                           DIAMETER_PEER_EV_I_RCV_DPA,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnectDPA);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                           DIAMETER_PEER_EV_R_RCV_DPA,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnectDPA);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                           DIAMETER_PEER_EV_TIMEOUT,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acError);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                           DIAMETER_PEER_EV_I_PEER_DISC,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnect);

        AddStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                           DIAMETER_PEER_EV_R_PEER_DISC,
                           DIAMETER_PEER_ST_CLOSED,
                           m_acDisconnect);

        AddWildcardStateTableEntry(DIAMETER_PEER_ST_CLOSING,
                                   DIAMETER_PEER_ST_CLOSING);

        InitialState(DIAMETER_PEER_ST_CLOSED);
      }

   private:
      DiameterPeerR_ISendConnReq       m_acISendConnReq;
      DiameterPeerR_AcceptSendCEA      m_acRAcceptSendCEA;
      DiameterPeerI_SendCER            m_acISendCER;
      DiameterPeer_ConnNack            m_acConnNack;
      DiameterPeer_Cleanup             m_acCleanup;
      DiameterPeer_ReConnect           m_acRetry;
      DiameterPeerR_Accept             m_acRAccept;
      DiameterPeer_Error               m_acError;
      DiameterPeer_ProcessCEA          m_acProcessCEA;
      DiameterPeerR_AcceptElect        m_acRAcceptElect;
      DiameterPeer_Disconnect          m_acDisconnect;
      DiameterPeer_DisconnectDPA       m_acDisconnectDPA;
      DiameterPeerI_SendCERElect       m_acISendCERElect;
      DiameterPeerR_SendCEA            m_acRSendCEA;
      DiameterPeerR_SendCEAOpen        m_acRSendCEAOpen;
      DiameterPeerR_DisconnectResp     m_acRDisconnectResp;
      DiameterPeerR_DisconnectIOpen    m_acRDisconnectIOpen;
      DiameterPeerR_Reject             m_acRReject;
      DiameterPeerI_DisconnectSendCEA  m_acIDisconnectSendCEA;
      DiameterPeerR_SendMessage        m_acRSendMessage;
      DiameterPeer_Process             m_acProcess;
      DiameterPeerProcessDWRSendDWA    m_acProcessDWRSendDWA;
      DiameterPeer_ProcessDWA          m_acProcessDWA;
      DiameterPeerI_SendDPR            m_acISendDPR;
      DiameterPeerR_SendDPR            m_acRSendDPR;
      DiameterPeerI_SendDPADisconnect  m_acISendDPADisconnect;
      DiameterPeerR_SendDPADisconnect  m_acRSendDPADisconnect;
      DiameterPeerI_SendMessage        m_acISendMessage;
      DiameterPeerI_SendCEA            m_acISendCEA;
      DiameterPeer_Watchdog            m_acWatchdog; 
};

class DiameterPeerStateMachine :
    public AAA_StateMachineWithTimer<DiameterPeerStateMachine>,
           AAA_Job
{
   public:
      DiameterPeerData &Data() {
         return m_Data;
      }
      AAA_GroupedJob &Job() {
          return m_GroupedJob;
      }
      virtual int Send(std::auto_ptr<DiameterMsg> &msg, bool consume = true) {
          ///  If using ASYNC SEND
          ///  EnqueueSendMsg(msg);
          ///  Notify(DIAMETER_PEER_EV_SEND_MESSAGE);
          if (! AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
             AAA_LOG((LM_INFO, "(%P|%t) Cannot send message, statemachine not running\n"));
             return (-1);
          }
          switch (state) {
              case DIAMETER_PEER_ST_I_OPEN:
                  return m_TxMsgCollector.Send(msg, m_Data.m_IOInitiator.get(), consume);
              case DIAMETER_PEER_ST_R_OPEN:
                  return m_TxMsgCollector.Send(msg, m_Data.m_IOResponder.get(), consume);
              default:
                  AAA_LOG((LM_INFO, "(%P|%t) Discarding msg to send, peer state is not open\n"));
                  break;
          }
          return (0);
      }

   private: // Event Queue

      typedef struct {
         AAA_Event m_Event;
         std::auto_ptr<DiameterMsg> m_Msg;
         std::auto_ptr<Diameter_IO_Base> m_IO;
      } DiameterPeerEventParam;

      AAA_ProtectedPtrQueue<DiameterPeerEventParam> m_EventQueue;
      std::auto_ptr<DiameterPeerEventParam> m_CurrentPeerEventParam;

   public:

      virtual void Notify(AAA_Event event) {
         if (AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
             std::auto_ptr<DiameterPeerEventParam> e(new DiameterPeerEventParam);
             e->m_Event = event;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG((LM_INFO, "(%P|%t) Event not processed, statemachine not running\n"));
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<DiameterMsg> msg) {
         if (AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
             std::auto_ptr<DiameterPeerEventParam> e(new DiameterPeerEventParam);
             e->m_Event = event;
             e->m_Msg = msg;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG((LM_INFO, "(%P|%t) Event not processed, statemachine not running\n"));
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<Diameter_IO_Base> io) {
         if (AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
             std::auto_ptr<DiameterPeerEventParam> e(new DiameterPeerEventParam);
             e->m_Event = event;
             e->m_IO = io;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG((LM_INFO, "(%P|%t) Event not processed, statemachine not running\n"));
         }
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<DiameterMsg> msg,
                          std::auto_ptr<Diameter_IO_Base> io) {
         if (AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
             std::auto_ptr<DiameterPeerEventParam> e(new DiameterPeerEventParam);
             e->m_Event = event;
             e->m_Msg = msg;
             e->m_IO = io;
             m_EventQueue.Enqueue(e);
             Schedule(this);
         }
         else {
             AAA_LOG((LM_INFO, "(%P|%t) Event not processed, statemachine not running\n"));
         }
      }

   protected: // Constructor/Destructor

      DiameterPeerStateMachine(AAA_Task &t) :
          AAA_StateMachineWithTimer<DiameterPeerStateMachine>
             (*this, m_StateTable, *t.reactor()),
             m_CleanupSignal(m_CleanupMutex),
             m_GroupedJob(t.Job()) {
          m_ReconnectAttempt = 0;
          m_TxMsgCollector.Start();
      }
      virtual ~DiameterPeerStateMachine() {
          if (state != DIAMETER_PEER_ST_CLOSED) {
              m_CleanupSignal.wait();
          }
          AAA_StateMachine<DiameterPeerStateMachine>::Stop();
          m_TxMsgCollector.Stop();
      }

   protected: // Notifications
      virtual void Connected() {
      }
      virtual void Error(int resultCode) {
      }
      virtual void Disconnected(int cause) {
      }

   protected: // Job servicing

      ACE_Mutex m_EventFsmMtx;
      virtual int Serve() {
         AAA_MutexScopeLock guard(m_EventFsmMtx);
         m_CurrentPeerEventParam = m_EventQueue.Dequeue();
         DumpEvent(m_CurrentPeerEventParam->m_Event, "Pre-Event");
         try {
             AAA_StateMachineWithTimer
                 <DiameterPeerStateMachine>::Event
                        (m_CurrentPeerEventParam->m_Event);
         }
         catch (DiameterPeerFsmException &err) {
             AAA_LOG((LM_ERROR, "(%P|%t) FSM error[%d]: %s\n",
                        err.Code(), err.Description().c_str()));
         }
         catch (...) {
             AAA_LOG((LM_ERROR, "(%P|%t) Unknown exception in FSM\n"));
         }
         DumpEvent(m_CurrentPeerEventParam->m_Event, "Post-Event");
         m_CurrentPeerEventParam.reset();
         return (0);
      }
      virtual int Schedule(AAA_Job* job, size_t backlogSize = 1) {
         if (! AAA_StateMachineWithTimer
             <DiameterPeerStateMachine>::Running()) {
            return (-1);
         }
         return m_GroupedJob.Schedule(job, backlogSize);
      }
      virtual void Timeout(AAA_Event ev) {
         Notify(ev);
      }
      DiameterPeerData &PeerData() {
         return m_Data;
      }

   protected: // Capabilities exchange

      virtual void SendCER();
      virtual void SendCEA(diameter_unsigned32_t rcode,
                           std::string &message);

      void AssembleCE(DiameterMsg &msg,
                      bool request = true);
      void DisassembleCE(DiameterMsg &msg);
      bool ValidatePeer(diameter_unsigned32_t &rcode,
                        std::string &message);

   protected: // Watchdog

      virtual void SendDWR();
      virtual void SendDWA(diameter_unsigned32_t rcode,
                           std::string &message);

      void AssembleDW(DiameterMsg &msg,
                      bool request = true);
      void DisassembleDW(DiameterMsg &msg);

   protected: // Disconnection

      virtual void SendDPR(bool initiator);
      virtual void SendDPA(bool initiator,
                           diameter_unsigned32_t rcode);
      void AssembleDP(DiameterMsg &msg,
                      bool request = true);
      void DisassembleDP(DiameterMsg &msg);

   protected: // Message Id's

      void MsgIdTxMessage(DiameterMsg &msg);
      bool MsgIdRxMessage(DiameterMsg &msg);

   protected:  // Re-connection logic

      int m_ReconnectAttempt;

      void DoReConnect();
      void StopReConnect();

   protected: // Update of peer SCTP addresses

      int TransportProtocolInUse();

   protected: // Message transmission thread
      DiameterTxMsgCollector m_TxMsgCollector;

   protected: // Auxillary

      void Elect();

      typedef enum {
          CLEANUP_IO_I = 0x00000001,
          CLEANUP_IO_R = 0x00000002,
          CLEANUP_FSM  = 0x00000004,
          CLEANUP_ALL  = 0xffffffff
      } CLEANUP_FLG;

      ACE_Mutex m_CleanupMutex;
      ACE_Condition<ACE_Mutex> m_CleanupSignal;

      virtual void Cleanup(unsigned int flags = CLEANUP_ALL);

   protected:

      void DumpPeerCapabilities();
      void DumpEvent(AAA_Event ev, char *prefix) {
#if AAA_FSM_EVENT_DEBUG
          static char *evStrTable[] = { "DIAMETER_PEER_EV_START",  
                                        "DIAMETER_PEER_EV_STOP",
                                        "DIAMETER_PEER_EV_TIMEOUT",  
                                        "DIAMETER_PEER_EV_CONN_RETRY",  
                                        "DIAMETER_PEER_EV_R_CONN_CER",  
                                        "DIAMETER_PEER_EV_I_RCV_CONN_ACK",  
                                        "DIAMETER_PEER_EV_I_RCV_CONN_NACK",
                                        "DIAMETER_PEER_EV_R_RCV_CEA",  
                                        "DIAMETER_PEER_EV_I_RCV_CEA",  
                                        "DIAMETER_PEER_EV_I_PEER_DISC",  
                                        "DIAMETER_PEER_EV_R_PEER_DISC",  
                                        "DIAMETER_PEER_EV_I_RCV_NON_CEA",  
                                        "DIAMETER_PEER_EV_WIN_ELECTION",  
                                        "DIAMETER_PEER_EV_SEND_MESSAGE",  
                                        "DIAMETER_PEER_EV_R_RCV_MESSAGE",  
                                        "DIAMETER_PEER_EV_I_RCV_MESSAGE",  
                                        "DIAMETER_PEER_EV_R_RCV_DWR",  
                                        "DIAMETER_PEER_EV_I_RCV_DWR",
                                        "DIAMETER_PEER_EV_R_RCV_DWA",  
                                        "DIAMETER_PEER_EV_I_RCV_DWA",
                                        "DIAMETER_PEER_EV_R_RCV_DPR",  
                                        "DIAMETER_PEER_EV_I_RCV_DPR",
                                        "DIAMETER_PEER_EV_R_RCV_CER",
                                        "DIAMETER_PEER_EV_I_RCV_CER",
                                        "DIAMETER_PEER_EV_R_RCV_DPA",
                                        "DIAMETER_PEER_EV_I_RCV_DPA",
                                        "DIAMETER_PEER_EV_WATCHDOG"
          };
          static char *stStrTable[] = { "DIAMETER_PEER_ST_CLOSED",
                                        "DIAMETER_PEER_ST_WAIT_CONN_ACK",
                                        "DIAMETER_PEER_ST_WAIT_I_CEA",
                                        "DIAMETER_PEER_ST_WAIT_CONN_ACK_ELECT",
                                        "DIAMETER_PEER_ST_WAIT_RETURNS",
                                        "DIAMETER_PEER_ST_I_OPEN",
                                        "DIAMETER_PEER_ST_R_OPEN",
                                        "DIAMETER_PEER_ST_CLOSING"
          };
          AAA_LOG((LM_INFO, "(%P|%t) FSM EVENT DEBUG [state=%s, event=%s]: %s\n",
                    stStrTable[state], evStrTable[ev], prefix));
#endif // AAA_FSM_EVENT_DEBUG
      }

    public:
      friend class DiameterPeerR_AcceptSendCEA;
      friend class DiameterPeerI_SendCER;
      friend class DiameterPeer_Cleanup;
      friend class DiameterPeer_ConnNack;
      friend class DiameterPeerR_Accept;
      friend class DiameterPeer_Error;
      friend class DiameterPeer_ReConnect;
      friend class DiameterPeer_ProcessCEA;
      friend class DiameterPeerR_AcceptElect;
      friend class DiameterPeer_Disconnect;
      friend class DiameterPeer_DisconnectDPA;
      friend class DiameterPeerI_SendCERElect;
      friend class DiameterPeerR_SendCEA;
      friend class DiameterPeerR_SendCEAOpen;
      friend class DiameterPeerR_DisconnectResp;
      friend class DiameterPeerR_DisconnectIOpen;
      friend class DiameterPeerR_Reject;
      friend class DiameterPeerI_DisconnectSendCEA;
      friend class DiameterPeerR_SendMessage;
      friend class DiameterPeer_Process;
      friend class DiameterPeerProcessDWRSendDWA;
      friend class DiameterPeer_ProcessDWA;
      friend class DiameterPeerI_SendDPR;
      friend class DiameterPeerR_SendDPR;
      friend class DiameterPeerI_SendDPADisconnect;
      friend class DiameterPeerR_SendDPADisconnect;
      friend class DiameterPeerI_SendMessage;
      friend class DiameterPeerI_SendCEA;
      friend class DiameterPeer_Watchdog;

   private:

      AAA_GroupedJob &m_GroupedJob;
      DiameterPeerData m_Data;
      static DiameterPeerStateTable m_StateTable;
};

#endif /* __AAA_PEER_FSM_H__ */

