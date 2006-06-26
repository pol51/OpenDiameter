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

#ifndef __AAA_SESSION_AUTH_CLIENT_FSM_H__
#define __AAA_SESSION_AUTH_CLIENT_FSM_H__

#include "aaa_session_auth_fsm.h"

class DIAMETERBASEPROTOCOL_EXPORT AAA_AuthSessionClientStateMachine :
   public AAA_AuthSessionStateMachine<AAA_AuthSessionClientStateMachine>
{  
   public:
      AAA_AuthSessionClientStateMachine(AAA_Task &t,
                                        AAA_AuthSession &a);
      virtual ~AAA_AuthSessionClientStateMachine() {
      }    

      void TxSTR(diameter_unsigned32_t cause);
      void TxASA(diameter_unsigned32_t rcode);
      void TxRAA(diameter_unsigned32_t rcode);
      void RxASR(AAAMessage &msg);
      void RxSTA(AAAMessage &msg);
      void RxRAR(AAAMessage &msg);
};

class AAA_SessAuthClient_TxSSAR : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          fsm.Session().TxDelivery(msg);
      }
};

class AAA_SessAuthClient_TxSSAR_Discard : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          AAA_LOG(LM_INFO, "(%P|%t) Message sent in invalid session state, discarding\n");
          AAA_MsgDump::Dump(*msg);
      }
};

class AAA_SessAuthClient_RxSSA : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          fsm.Session().RxDelivery(msg);
      }
};

class AAA_SessAuthClient_RxSSAA_Discard : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          AAA_LOG(LM_INFO, "(%P|%t) Message received in invalid session state, discarding\n");
          AAA_MsgDump::Dump(*msg);
      }
};

class AAA_SessAuthClient_RxSSAA_GrantAccess : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          if (fsm.Attributes().AuthSessionState()() == 
              AAA_SESSION_STATE_MAINTAINED) {
              fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
                  fsm.Attributes().SessionTimeout()(),
                  0, AAA_TIMER_TYPE_SESSION);
              fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
                  fsm.Attributes().AuthLifetime()() +
                  fsm.Attributes().AuthGrace()(),
                  0, AAA_TIMER_TYPE_AUTH);
          }
          else {
              fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                  fsm.Attributes().SessionTimeout()(),
                  0, AAA_TIMER_TYPE_SESSION);
          }
          AAA_LOG(LM_INFO, "(%P|%t) Client session in open state\n");
          AAA_LOG(LM_INFO, "(%P|%t) Session Timeout: %d\n", 
                            fsm.Attributes().SessionTimeout()());
          AAA_LOG(LM_INFO, "(%P|%t) Auth Lifetime  : %d\n",
                            fsm.Attributes().AuthLifetime()());
          AAA_LOG(LM_INFO, "(%P|%t) Grace Period   : %d\n",
                            fsm.Attributes().AuthGrace()());
          fsm.Session().Success();
      }
};

class AAA_SessAuthClient_RxRAR : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          diameter_unsigned32_t rcode = AAA_SUCCESS;
          if (fsm.Attributes().ReAuthRequestValue().IsSet()) {
              rcode = (diameter_unsigned32_t)fsm.Session().ReAuthenticate
                        (fsm.Attributes().ReAuthRequestValue()().reAuthType);             
	  }
          fsm.TxRAA(rcode);
      }
};

class AAA_SessAuthClient_TxSTR_NoSvc : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxSTR(AAA_REALM_NOT_SERVED);
      }
};

class AAA_SessAuthClient_TxSTR_ProcError : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxSTR(AAA_AVP_UNSUPPORTED);
      }
};

class AAA_SessAuthClient_TxSTR_Fail : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxSTR(AAA_AUTHORIZATION_REJECTED);
      }
};

class AAA_SessAuthClient_SessionTimeout : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Session().SessionTimeout();
          fsm.Session().Reset();
      }
};

class AAA_SessAuthClient_SessionTimeoutState : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTR(AAA_AUTHORIZATION_REJECTED);
          fsm.Session().SessionTimeout();
      }
};

class AAA_SessAuthClient_AuthTimeout : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTR(AAA_AUTHORIZATION_REJECTED);
          fsm.Session().AuthorizationTimeout();
      }
};

class AAA_SessAuthClient_UserAbort : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTR(AAA_AUTHORIZATION_REJECTED);
      }
};

class AAA_SessAuthClient_TxASA_Success : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxASA(AAA_SUCCESS);
      }
};

class AAA_SessAuthClient_TxASA_TxSTR_Success : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxASA(AAA_SUCCESS);
          fsm.TxSTR(AAA_SUCCESS);
      }
};

class AAA_SessAuthClient_TxASA_Fail : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.TxASA(AAA_UNABLE_TO_COMPLY);
      }
};

class AAA_SessAuthClient_Disconnect : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.Session().Disconnect();
          fsm.CancelAllTimer();
          fsm.Session().Reset();
      }
};

class AAA_SessAuthClient_Cleanup : 
   public AAA_Action<AAA_AuthSessionClientStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionClientStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Session().AbortSession();
          fsm.Session().Reset();
      }
};

class AAA_SessAuthClientStateTable : 
   public AAA_StateTable<AAA_AuthSessionClientStateMachine>
{
   public:
      AAA_SessAuthClientStateTable() {

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Client or Device Requests      Send       Pending
                     access                         service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_REQUEST_ACCESS,
                           AAA_SESSION_AUTH_ST_PENDING);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      ASR Received                   Send ASA   Idle
                      for unknown session            with
                                                     Result-Code
                                                     = UNKNOWN_
                                                     SESSION_ID
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_ASR_USID,
                           AAA_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Discard      Idle
                     req sent                       service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_TX_SSAR,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acTxSSARDiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Discard      Idle
                     answer received                service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_SSAA,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acRxSSAADiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle     Any                            None       Idle
         */
        AddWildcardStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                                   AAA_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Successful Service-specific    Grant      Open
                      authorization answer           Access
                      received with Auth-Session-
                      State set to
                      NO_STATE_MAINTAINED

           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Successful Service-specific    Grant      Open
                      authorization answer           Access
                      received with default
                      Auth-Session-State value
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_SSAA_OK,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acGrantAccess);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Successful Service-specific    Sent STR   Discon
                      authorization answer received
                      but service not provided
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_SSAA_NOSVC,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxSTRNoSvc);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Error processing successful    Sent STR   Discon
                      Service-specific authorization
                      answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_SSAA_ERROR,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxSTRProcError);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Failed Service-specific        Cleanup    Idle
                      authorization answer received
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_SSAA_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Pending   Service-specific authorization Send       Pending
                     req sent                       service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_TX_SSAR,
                           AAA_SESSION_AUTH_ST_PENDING,
                           m_acTxSSAR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Pending   Service-specific authorization Accept       Pending
                     answer received                service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_RX_SSAA,
                           AAA_SESSION_AUTH_ST_PENDING,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending   Service to user is terminated  Discon.    Idle
                                                     user/device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                           AAA_SESSION_AUTH_EV_STOP,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Pending  Any                            None       Pending
         */
        AddWildcardStateTableEntry(AAA_SESSION_AUTH_ST_PENDING,
                                   AAA_SESSION_AUTH_ST_PENDING);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      User or client device          Send       Open
                      requests access to service     service
                                                     specific
                                                     auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_REQUEST_ACCESS,
                           AAA_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Re-auth request received       Send       Open
                                                     re-auth
                                                     answer  
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_RAR,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxRAR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Successful Service-specific    Provide    Open
                      authorization answer received  Service
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAA_OK,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acGrantAccess);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Service-specific authorization Process    Open
                      requests received              service
                                                     specific
                                                                
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_SSAR,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Service-specific authorization Send       Open
                      requests received and user     service
                      or client device can comply    specific
                                                     auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAR_OK,
                           AAA_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Service-specific authorization Send       Idle
                      requests received and user     failed serv.
                      or client device cannot comply specific
                                                     answer,
                                                     Cleanup
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAR_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Failed Service-specific        Discon.    Idle
                      authorization answer           user/device
                      received.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAA_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Session-Timeout Expires on     Send STR   Discon
                      Access Device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acSessionTimeoutState);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Session-Timeout Expires on     Discon.    Idle
                      Access Device                  user/device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      ASR Received,                  Send ASA   Discon
                      client will comply with        with
                      request to end the session     Result-Code
                                                      = SUCCESS,
                                                      Send STR.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_ASR_OK,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxASATxSTRSuccess);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      ASR Received,                  Send ASA   Open
                      client will not comply with    with
                      request to end the session     Result-Code
                                                      != SUCCESS
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_ASR_RETRY,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acTxASAFail);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Authorization-Lifetime +       Send STR   Discon
                      Auth-Grace-Period expires on
                      access device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acAuthTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Service to user is terminated  Discon.    Idle
                                                     user/device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_STOP,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Session being terminated by    Send STR   Discon
                      user                                       
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_ABORT,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acUserAbort);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Send       Open
                     req sent                       service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_TX_SSAR,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acTxSSAR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Accept       Open
                     answer received                service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_SSAA,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open     Any                            None       Open
         */
        AddWildcardStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                                   AAA_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    ASR Received                   Send ASA   Discon
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_ASR,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxASASuccess);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    STA Received                   Discon.    Idle
                                                     user/device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_STA,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Session-Timeout Expires on     Cleanup    Idle
                      Access Device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Authorization-Lifetime +       Cleanup    Idle
                      Auth-Grace-Period expires on
                      access device
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acDisconnect);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon    Service-specific authorization Discard      Discon
                     req sent                       service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_TX_SSAR,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxSSARDiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon    Service-specific authorization Discard      Discon
                     answer received                service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_SSAA,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acRxSSAADiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon   Any                            None       Discon
         */
        AddWildcardStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                                   AAA_SESSION_AUTH_ST_DISC);

        InitialState(AAA_SESSION_AUTH_ST_IDLE);
      }

   private:
      AAA_SessAuthClient_TxSSAR               m_acTxSSAR;
      AAA_SessAuthClient_RxSSA                m_acRxSSA;
      AAA_SessAuthClient_TxSSAR_Discard       m_acTxSSARDiscard;
      AAA_SessAuthClient_RxSSAA_Discard       m_acRxSSAADiscard;
      AAA_SessAuthClient_RxSSAA_GrantAccess   m_acGrantAccess;
      AAA_SessAuthClient_RxRAR                m_acRxRAR;
      AAA_SessAuthClient_TxSTR_NoSvc          m_acTxSTRNoSvc;
      AAA_SessAuthClient_TxSTR_ProcError      m_acTxSTRProcError;
      AAA_SessAuthClient_TxSTR_Fail           m_acTxSTRFail;
      AAA_SessAuthClient_SessionTimeout       m_acSessionTimeout;
      AAA_SessAuthClient_SessionTimeoutState  m_acSessionTimeoutState;
      AAA_SessAuthClient_AuthTimeout          m_acAuthTimeout;
      AAA_SessAuthClient_UserAbort            m_acUserAbort;
      AAA_SessAuthClient_TxASA_Success        m_acTxASASuccess;
      AAA_SessAuthClient_TxASA_TxSTR_Success  m_acTxASATxSTRSuccess;
      AAA_SessAuthClient_TxASA_Fail           m_acTxASAFail;
      AAA_SessAuthClient_Cleanup              m_acCleanup;
      AAA_SessAuthClient_Disconnect           m_acDisconnect;
};

#endif /* __AAA_SESSION_AUTH_CLIENT_FSM_H__ */

