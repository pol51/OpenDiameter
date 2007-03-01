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

               ACE_UINT32 m_Event_App           : 4;
               ACE_UINT32 m_Event_Eap           : 3;

               ACE_UINT32 m_Cfg_EapPiggyback    : 1; // dependent

               ACE_UINT32 m_Do_Ping             : 1;
               ACE_UINT32 m_Do_RetryTimeout     : 1;
               ACE_UINT32 m_Do_ReTransmission   : 1;
               ACE_UINT32 m_Do_FatalError       : 1;
               ACE_UINT32 m_Do_SessTimeout      : 1;

               ACE_UINT32 m_ResultCode          : 2;

               ACE_UINT32 m_AvpExist_KeyId      : 1; // dependent
               ACE_UINT32 m_AvpExist_Auth       : 1;
               ACE_UINT32 m_AvpExist_EapPayload : 1;

               ACE_UINT32 m_AlgoNotSupported    : 1;

               ACE_UINT32 m_Reserved            : 8;
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
        void Do_FatalError(bool set = true) {
            m_Event.i.m_Do_FatalError = set;
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
        void AlgorithmNotSupported(bool set = true) {
            m_Event.i.m_AlgoNotSupported = set;
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
            if (m_Event.i.m_Event_App)
                AAA_LOG((LM_DEBUG, "App[%d] ", m_Event.i.m_Event_App));
            if (m_Event.i.m_Event_Eap)
                AAA_LOG((LM_DEBUG, "Eap Event[%d] ", m_Event.i.m_Event_Eap));
            if (m_Event.i.m_Cfg_EapPiggyback)
                AAA_LOG((LM_DEBUG, "EapPiggy "));
            if (m_Event.i.m_AlgoNotSupported)
                AAA_LOG((LM_DEBUG, "Algorithm "));
            if (m_Event.i.m_Do_Ping)
                AAA_LOG((LM_DEBUG, "DoPing "));
            if (m_Event.i.m_Do_RetryTimeout)
                AAA_LOG((LM_DEBUG, "RetryTout "));
            if (m_Event.i.m_Do_ReTransmission)
                AAA_LOG((LM_DEBUG, "Retran "));
            if (m_Event.i.m_Do_FatalError)
                AAA_LOG((LM_DEBUG, "Fatal "));
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

       class PacOfflineExitActionRxPSR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPSR();
           }
       };
       class PacOfflineExitActionAuthUser : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPCI();
           }
       };
       class PacWaitEapMsgInExitActionTxPSA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPSA(true);
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
       class PacWaitPaaExitActionRxPBR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPBR();
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
               if (PANA_CFG_PAC().m_EapPiggyback) {
                   c.TxPAN(false);
               }
               c.Timer().CancelEapResponse();
           }
       };
       class PacWaitEapResultExitActionEapOpen : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPBA(true);
               c.NotifyAuthorization();
               c.NotifyScheduleLifetime();
           }
       };
       class PacWaitEapSuccessExitActionClose : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPBA(true);
               c.Disconnect();
           }
       };
       class PacWaitEapFailExitActionClose : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPBA(false);
               c.Disconnect();
           }
       };
       class PacOpenExitActionRxPPR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPPR();
           }
       };
       class PacOpenExitActionTxPPR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPPR();
           }
       };
       class PacOpenExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.NotifyEapRestart();
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
       class PacOpenExitActionTxPUR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPUR();
           }
       };
       class PacOpenExitActionRxPUR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPUR();
           }
       };
       class PacOpenExitActionTxPRR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPRR();
           }
       };
       class PacWaitPRAAExitActionRxPRA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPRA();
           }
       };
       class PacWaitPPAExitActionRxPPA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPPA();
           }
       };
       class PacWaitPUAExitActionRxPUA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPUA();
           }
       };
       class PacSessTermExitActionRxPTA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPTA();
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
       class PacExitActionTxPEA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPEA();
           }
       };
       class PacExitActionTxPER : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.TxPER(PANA_ERROR_UNABLE_TO_COMPLY);
           }
       };
       class PacWaitPEAExitActionRxPEA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) {
               c.RxPEA();
           }
       };

   private:
       PacOfflineExitActionRxPSR              m_PacOfflineExitActionRxPSR;
       PacOfflineExitActionAuthUser           m_PacOfflineExitActionAuthUser;
       PacWaitEapMsgInExitActionTxPSA         m_PacWaitEapMsgInExitActionTxPSA;
       PacWaitPaaExitActionRxPAR              m_PacWaitPaaExitActionRxPAR;
       PacWaitPaaExitActionRxPAN              m_PacWaitPaaExitActionRxPAN;
       PacWaitPaaExitActionRxPBR              m_PacWaitPaaExitActionRxPBR;
       PacWaitEapMsgExitActionTxPAR           m_PacWaitEapMsgExitActionTxPAR;
       PacWaitEapMsgExitActionTxPAN           m_PacWaitEapMsgExitActionTxPAN;
       PacWaitEapMsgExitActionTxPANTout       m_PacWaitEapMsgExitActionTxPANTout;
       PacWaitEapResultExitActionEapOpen      m_PacWaitEapResultExitActionEapOpen;
       PacWaitEapSuccessExitActionClose       m_PacWaitEapSuccessExitActionClose;
       PacWaitEapFailExitActionClose          m_PacWaitEapFailExitActionClose;
       PacOpenExitActionRxPPR                 m_PacOpenExitActionRxPPR;
       PacOpenExitActionTxPPR                 m_PacOpenExitActionTxPPR;
       PacOpenExitActionRxPAR                 m_PacOpenExitActionRxPAR;
       PacOpenExitActionRxPTR                 m_PacOpenExitActionRxPTR;
       PacOpenExitActionTxPTR                 m_PacOpenExitActionTxPTR;
       PacOpenExitActionTxPUR                 m_PacOpenExitActionTxPUR;
       PacOpenExitActionRxPUR                 m_PacOpenExitActionRxPUR;
       PacOpenExitActionTxPRR                 m_PacOpenExitActionTxPRR;
       PacWaitPRAAExitActionRxPRA             m_PacWaitPRAAExitActionRxPRA;
       PacWaitPUAExitActionRxPUA              m_PacWaitPUAExitActionRxPUA;
       PacWaitPPAExitActionRxPPA              m_PacWaitPPAExitActionRxPPA;
       PacSessTermExitActionRxPTA             m_PacSessTermExitActionRxPTA;
       PacExitActionRetransmission            m_PacExitActionRetransmission;
       PacExitActionTimeout                   m_PacExitActionTimeout;
       PacExitActionTxPEA                     m_PacExitActionTxPEA;
       PacExitActionTxPER                     m_PacExitActionTxPER;
       PacWaitPEAExitActionRxPEA              m_PacWaitPEAExitActionRxPEA;
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
      virtual void Update(ACE_INET_Addr &addr);
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
      PANA_Channel m_Channel;
      FsmTimer<PANA_PacEventVariable, PANA_PacSession> m_Timer;

   private:
      // actual pana client
      PANA_Client m_PaC;
};

#endif // __PANA_CLIENT_FSM_H__
