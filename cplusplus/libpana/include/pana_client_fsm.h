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

#ifndef __PANA_CLIENT_FSM_H__
#define __PANA_CLIENT_FSM_H__

#include "framework.h"
#include "pana_fsm.h"
#include "pana_client.h"

class PANA_PacEventVariable
{
    private:
        typedef union {
            struct {
               ACE_UINT32 m_Type_Msg            : 5;

               ACE_UINT32 m_Flag_Start          : 1;
               ACE_UINT32 m_Flag_Complete       : 1;
               ACE_UINT32 m_Flag_Auth           : 1;
               ACE_UINT32 m_Flag_Ping           : 1;

               ACE_UINT32 m_Event_App           : 4;
               ACE_UINT32 m_Event_Eap           : 3;

               ACE_UINT32 m_Cfg_EapPiggyback    : 1; // dependent

               ACE_UINT32 m_Do_Ping             : 1;
               ACE_UINT32 m_Do_RetryTimeout     : 1;
               ACE_UINT32 m_Do_ReTransmission   : 1;
               ACE_UINT32 m_Do_SessTimeout      : 1;

               ACE_UINT32 m_ResultCode          : 2;

               ACE_UINT32 m_AvpExist_KeyId      : 1; // dependent
               ACE_UINT32 m_AvpExist_Auth       : 1;
               ACE_UINT32 m_AvpExist_EapPayload : 1;

               ACE_UINT32 m_Reserved            : 6;
            } i;
            ACE_UINT32 p;
        } EventParams;

    public:
        PANA_PacEventVariable() {
            m_Event.p = 0;
        }
        void MsgType(PANA_MSG_TYPE type) {
            m_Event.p = 0;
            m_Event.i.m_Type_Msg = type;
        }
        void FlagStart(bool set = true) {
            m_Event.i.m_Flag_Start = set;
        }
        void FlagComplete(bool set = true) {
            m_Event.i.m_Flag_Complete = set;
        }
        void FlagAuth(bool set = true) {
            m_Event.i.m_Flag_Auth = set;
        }
        void FlagPing(bool set = true) {
            m_Event.i.m_Flag_Ping = set;
        }
        void Event_App(PANA_APP_EVENT event) {
            m_Event.i.m_Event_App = event;
        }
        void EnableCfg_EapPiggyback(bool set = true) {
            m_Event.i.m_Cfg_EapPiggyback = set;
        }
        void Do_Ping(bool set = true) {
            m_Event.i.m_Do_Ping = set;
        }
        void Do_SessTimeout(bool set = true) {
            m_Event.i.m_Do_SessTimeout = set;
        }
        void Do_RetryTimeout(bool set = true) {
            m_Event.i.m_Do_RetryTimeout = set;
        }
        void Do_ReTransmission(bool set = true) {
            m_Event.i.m_Do_ReTransmission = set;
        }
        void ResultCode(PANA_RESULT_CODE event) {
            m_Event.i.m_ResultCode = event;
        }
        void AvpExist_KeyId(bool set = true) {
            m_Event.i.m_AvpExist_KeyId = set;
        }
        void AvpExist_Auth(bool set = true) {
            m_Event.i.m_AvpExist_Auth = set;
        }
        void AvpExist_EapPayload(bool set = true) {
            m_Event.i.m_AvpExist_EapPayload = set;
        }
        void Event_Eap(PANA_EAP_EVENT event) {
            m_Event.i.m_Event_Eap = event;
        }
        void Reset() {
            m_Event.p = 0;
        }
        ACE_UINT32 Get() {
            DumpEvent();
            return m_Event.p;
        }
        void DumpEvent() {
#if PANA_DEBUG
            AAA_LOG((LM_DEBUG, "Event: "));
            if (m_Event.i.m_Type_Msg)
                AAA_LOG((LM_DEBUG, "Msg[%d] ", m_Event.i.m_Type_Msg));
            if (m_Event.i.m_Flag_Start)
                AAA_LOG((LM_DEBUG, "Start flag "));
            if (m_Event.i.m_Flag_Complete)
                AAA_LOG((LM_DEBUG, "Complete flag "));
            if (m_Event.i.m_Flag_Auth)
                AAA_LOG((LM_DEBUG, "Auth flag "));
            if (m_Event.i.m_Flag_Ping)
                AAA_LOG((LM_DEBUG, "Ping flag "));
            if (m_Event.i.m_Event_App)
                AAA_LOG((LM_DEBUG, "App[%d] ", m_Event.i.m_Event_App));
            if (m_Event.i.m_Event_Eap)
                AAA_LOG((LM_DEBUG, "Eap Event[%d] ", m_Event.i.m_Event_Eap));
            if (m_Event.i.m_Cfg_EapPiggyback)
                AAA_LOG((LM_DEBUG, "EapPiggy "));
            if (m_Event.i.m_Do_Ping)
                AAA_LOG((LM_DEBUG, "DoPing "));
            if (m_Event.i.m_Do_RetryTimeout)
                AAA_LOG((LM_DEBUG, "RetryTout "));
            if (m_Event.i.m_Do_ReTransmission)
                AAA_LOG((LM_DEBUG, "Retran "));
            if (m_Event.i.m_Do_SessTimeout)
                AAA_LOG((LM_DEBUG, "SessTout "));
            if (m_Event.i.m_ResultCode)
                AAA_LOG((LM_DEBUG, "ResultCode[%d] ", m_Event.i.m_ResultCode));
            if (m_Event.i.m_AvpExist_KeyId)
                AAA_LOG((LM_DEBUG, "keyId "));
            if (m_Event.i.m_AvpExist_Auth)
                AAA_LOG((LM_DEBUG, "Auth "));
            if (m_Event.i.m_AvpExist_EapPayload)
                AAA_LOG((LM_DEBUG, "EapPayload "));
            AAA_LOG((LM_DEBUG, "\n"));
#endif
        }

    private:
        EventParams m_Event;
};

class PANA_EXPORT PANA_ClientStateTable :
    public AAA_StateTable<PANA_Client>
{
   public:
       PANA_ClientStateTable();

       class PacOfflineExitActionAuthUser : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPCI();
           }
       };
       class PacOfflineExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPARStart();
           }
       };
       class PacWaitEapMsgInExitActionTxPAN : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPANStart(true);
           }
       };
       class PacWaitPaaExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPAR();
           }
       };
       class PacWaitPaaExitActionRxPAN : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPAN();
           }
       };
       class PacWaitPaaExitActionRxPARComplete : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPARComplete();
           }
       };
       class PacWaitEapMsgExitActionTxPAN : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPAN(true);
           }
       };
       class PacWaitEapMsgExitActionTxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPAR();
           }
       };
       class PacWaitEapMsgExitActionTxPANTout : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPAN(false);
               c.Timer().CancelEapResponse();
           }
       };
       class PacWaitEapResultExitActionEapOpen : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPANComplete(true);
               c.NotifyAuthorization();
               c.NotifyScheduleLifetime();
           }
       };
       class PacWaitEapFailExitActionClose : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPANComplete(false);
               c.Disconnect();
           }
       };
       class PacWaitEapSuccessExitActionClose : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPANComplete(true);
               c.Disconnect();
           }
       };
       class PacOpenExitActionRxPNRPing : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPNRPing();
           }
       };
       class PacOpenExitActionTxPNRPing : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPNRPing();
           }
       };
       class PacOpenExitActionTxPNRAuth : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPNRAuth();
           }
       };
       class PacOpenExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.NotifyEapRestart();
               c.NotifyScheduleLifetime();
               c.RxPAR();
           }
       };
       class PacOpenExitActionRxPTR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPTR();
           }
       };
       class PacOpenExitActionTxPTR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPTR(PANA_TERMCAUSE_LOGOUT);
           }
       };
       class PacWaitPNAExitActionRxPNAAuth : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPNAAuth();
           }
       };
       class PacWaitPNAExitActionRxPNAPing : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPNAPing();
           }
       };
       class PacExitActionTimeout : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.Disconnect(PANA_TERMCAUSE_SESSION_TIMEOUT);
           }
       };
       class PacExitActionRetransmission : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c);
       };
       class PacSessTermExitActionRxPTA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPTA();
           }
       };

   private:
       PacOfflineExitActionRxPAR              m_PacOfflineExitActionRxPAR;
       PacWaitEapMsgInExitActionTxPAN         m_PacWaitEapMsgInExitActionTxPAN;
       PacWaitPaaExitActionRxPAR              m_PacWaitPaaExitActionRxPAR;
       PacWaitPaaExitActionRxPAN              m_PacWaitPaaExitActionRxPAN;
       PacWaitPaaExitActionRxPARComplete      m_PacWaitPaaExitActionRxPARComplete;
       PacWaitEapMsgExitActionTxPAR           m_PacWaitEapMsgExitActionTxPAR;
       PacWaitEapMsgExitActionTxPAN           m_PacWaitEapMsgExitActionTxPAN;
       PacWaitEapMsgExitActionTxPANTout       m_PacWaitEapMsgExitActionTxPANTout;
       PacWaitEapResultExitActionEapOpen      m_PacWaitEapResultExitActionEapOpen;
       PacWaitEapFailExitActionClose          m_PacWaitEapFailExitActionClose;
       PacWaitEapSuccessExitActionClose       m_PacWaitEapSuccessExitActionClose;
       PacOpenExitActionRxPNRPing             m_PacOpenExitActionRxPNRPing;
       PacOpenExitActionTxPNRPing             m_PacOpenExitActionTxPNRPing;
       PacOpenExitActionTxPNRAuth             m_PacOpenExitActionTxPNRAuth;
       PacOpenExitActionRxPAR                 m_PacOpenExitActionRxPAR;
       PacOpenExitActionRxPTR                 m_PacOpenExitActionRxPTR;
       PacOpenExitActionTxPTR                 m_PacOpenExitActionTxPTR;
       PacWaitPNAExitActionRxPNAAuth          m_PacWaitPNAExitActionRxPNAAuth;
       PacWaitPNAExitActionRxPNAPing          m_PacWaitPNAExitActionRxPNAPing;
       PacExitActionTimeout                   m_PacExitActionTimeout;
       PacExitActionRetransmission            m_PacExitActionRetransmission;
       PacOfflineExitActionAuthUser           m_PacOfflineExitActionAuthUser;
       PacSessTermExitActionRxPTA             m_PacSessTermExitActionRxPTA;
};

class PANA_EXPORT PANA_PacSession : public
      PANA_StateMachine<PANA_Client, PANA_Channel>
{
   public:
      PANA_PacSession(PANA_Node &n,
                      PANA_ClientEventInterface &eif);
      virtual ~PANA_PacSession();

      virtual void Start() throw (AAA_Error);
      virtual void EapSendResponse(AAAMessageBlock *response);
      virtual void EapInvalidMessage();
      virtual void EapSuccess();
      virtual void EapTimeout();
      virtual void EapFailure();
      virtual void ReAuthenticate();
      virtual void Ping();
      virtual void Stop();

   private:
      virtual void InitializeMsgMaps();
      virtual void FlushMsgMaps();

   private:
      // actual statemachine table
      static PANA_ClientStateTable m_StateTable;

   private:
      // auxillary members
      PANA_Node &m_Node;
      PANA_Channel m_PanaChannel;
      PANA_Channel m_PacChannel;
      FsmTimer<PANA_PacEventVariable, PANA_PacSession> m_Timer;

   private:
      // actual pana client
      PANA_Client m_PaC;
};

#endif // __PANA_CLIENT_FSM_H__
