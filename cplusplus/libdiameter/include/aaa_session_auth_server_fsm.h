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

#ifndef __AAA_SESSION_AUTH_SERVER_FSM_H__
#define __AAA_SESSION_AUTH_SERVER_FSM_H__

#include "aaa_session_auth_fsm.h"

class DIAMETERBASEPROTOCOL_EXPORT AAA_AuthSessionServerStateMachine :
    public AAA_AuthSessionStateMachine<AAA_AuthSessionServerStateMachine>
{  
   public:
   public:
      AAA_AuthSessionServerStateMachine(AAA_Task &t,
                                        AAA_AuthSession &a);
      virtual ~AAA_AuthSessionServerStateMachine() {
      }

      void TxASR();
      void TxSTA(diameter_unsigned32_t rcode);
      void TxRAR(diameter_unsigned32_t rcode);
      void RxSTR(AAAMessage &msg);
      void RxASA(AAAMessage &msg);
      void RxRAA(AAAMessage &msg);

      bool &ASRSent() {
          return m_ASRSent;
      }

   protected:
       bool m_ASRSent;
};

class AAA_SessAuthServer_RxSSAR_Discard : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          AAA_LOG(LM_INFO, "(%P|%t) Message sent in invalid session state, discarding\n");
          AAA_MsgDump::Dump(*msg);
      }
};

class AAA_SessAuthServer_RxSSA : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          fsm.Session().RxDelivery(msg);
      }
};

class AAA_SessAuthServer_TxSSAA_Discard : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          AAA_LOG(LM_INFO, "(%P|%t) Message received in invalid session state, discarding\n");
          AAA_MsgDump::Dump(*msg);
      }
};

class AAA_SessAuthServer_TxSSA : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          std::auto_ptr<AAAMessage> msg = fsm.PendingMsg();
          fsm.Session().TxDelivery(msg);
          
          if (fsm.Attributes().AuthSessionState()() != 
              AAA_SESSION_STATE_MAINTAINED) {
              fsm.CancelAllTimer();
              fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                  fsm.Attributes().SessionTimeout()() +
                  AAA_AUTH_SESSION_GRACE_PERIOD,
                  0, AAA_TIMER_TYPE_SESSION);
                  
              AAA_LOG(LM_INFO, "(%P|%t) Server session in stateless mode, extending access time\n");
              AAA_LOG(LM_INFO, "(%P|%t) Session Timeout: %d\n", 
                            fsm.Attributes().SessionTimeout()());
              AAA_LOG(LM_INFO, "(%P|%t) Auth Lifetime  : %d\n",
                            fsm.Attributes().AuthLifetime()());
              AAA_LOG(LM_INFO, "(%P|%t) Grace Period   : %d\n",
                            fsm.Attributes().AuthGrace()());
              fsm.Session().Success();
          }
      }
};

class AAA_SessAuthServer_TxSSAA_GrantAccess : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
              fsm.Attributes().SessionTimeout()() +
              AAA_AUTH_SESSION_GRACE_PERIOD,
              0, AAA_TIMER_TYPE_SESSION);
          fsm.ScheduleTimer(AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
              fsm.Attributes().AuthLifetime()() +
              fsm.Attributes().AuthGrace()() +
              AAA_AUTH_SESSION_GRACE_PERIOD,
              0, AAA_TIMER_TYPE_AUTH);
              
          AAA_LOG(LM_INFO, "(%P|%t) Server session in open state\n");
          AAA_LOG(LM_INFO, "(%P|%t) Session Timeout: %d\n", 
                            fsm.Attributes().SessionTimeout()());
          AAA_LOG(LM_INFO, "(%P|%t) Auth Lifetime  : %d\n",
                            fsm.Attributes().AuthLifetime()());
          AAA_LOG(LM_INFO, "(%P|%t) Grace Period   : %d\n",
                            fsm.Attributes().AuthGrace()());
          fsm.Session().Success();
      }
};

class AAA_SessAuthServer_TxASR : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxASR();
      }
};

class AAA_SessAuthServer_TxRAR : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          if (fsm.Attributes().ReAuthRequestValue().IsSet()) {
              fsm.TxRAR
                (fsm.Attributes().ReAuthRequestValue()().reAuthType);  
	  }
      }
};

class AAA_SessAuthServer_RxRAA : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          if (fsm.Attributes().ReAuthRequestValue().IsSet()) {
              fsm.Session().ReAuthenticate
                 (fsm.Attributes().ReAuthRequestValue()().resultCode);        
	  }
      }
};

class AAA_SessAuthServer_TxSTACleanup : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTA(AAA_AUTHORIZATION_REJECTED);
          fsm.Session().Disconnect();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServer_TxSTACleanup_Idle : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTA(AAA_AUTHORIZATION_REJECTED);
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServer_RxASA : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
	  fsm.CancelTimer(AAA_TIMER_TYPE_ASR);
          fsm.Session().AbortSession();
      }
};

class AAA_SessAuthServer_AuthorizationTimeout : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.Session().AuthorizationTimeout();
          fsm.CancelAllTimer();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServer_SessionTimeout : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.Session().SessionTimeout();
          fsm.CancelAllTimer();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServer_Cleanup : 
   public AAA_Action<AAA_AuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(AAA_AuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Session().AbortSession();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServerStateTable : 
   public AAA_StateTable<AAA_AuthSessionServerStateMachine>
{
   public:
      AAA_SessAuthServerStateTable() {

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Service-specific authorization Send       Open
                      request received, and          successful
                      user is authorized             serv.
                                                     specific answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_SSAR_OK,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acGrantAccess); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Service-specific authorization Send       Idle
                      request received, and          failed serv.
                      user is not authorized         specific answer,
                                                     Cleanup
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_SSAR_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_ASA_OK,
                           AAA_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_ASA_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Any       STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_STR,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanupIdle);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Send       Idle
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_TX_SSAA,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acTxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Accept     Idle
                     request received               service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_RX_SSAR,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server for stateless
                      session.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Home server wants to           Cleanup    Idle
                      terminate service                      
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_IDLE,
                           AAA_SESSION_AUTH_EV_ABORT,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

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
            Open      Service-specific authorization Send       Open
                      request received, and user     successful
                      is authorized                  serv. specific
                                                     answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAR_OK,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acGrantAccess);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Service-specific authorization Send       Idle
                      request received, and user     failed serv.
                      is not authorized              specific
                                                     answer,
                                                     Cleanup
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAR_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Home server wants to send      Send       Open
                      Service-specific authorization service
                      request                        specific
                                                     auth req
        */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_TX_SSAR,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acTxSSA);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Service-specific authorization Provide    Open
                      answer received                Service
        */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_SSAA,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Successful Service-specific    Provide    Open
                      authorization answer received  Service
        */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAA_OK,
                           AAA_SESSION_AUTH_ST_OPEN);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Failed Service-specific        Cleanup    Idle
                      authorization answer received
        */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SSAA_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Home server wants to           Send ASR   Discon
                      terminate the service
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_ABORT,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxASR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acAuthTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Session-Timeout expires on     Cleanup    Idle
                      home server
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_ASA_OK,
                           AAA_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_ASA_FAIL,
                           AAA_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_STR,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Send       Open
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_TX_SSAA,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acTxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Accept     Open
                     request received               service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_SSAR,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Server initiated re-auth       Send       Open
                                                    Re-Auth
             
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_REAUTH,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acTxRAR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Re-auth answer received        process    Open
             
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_OPEN,
                           AAA_SESSION_AUTH_EV_RX_RAA,
                           AAA_SESSION_AUTH_ST_OPEN,
                           m_acRxRAA);

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
            Discon    Failure to send ASR            Wait,      Discon
                                                     resend ASR
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_TX_ASR_FAIL,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxASR);       

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    ASR successfully sent and      Wait STR   Discon
                      ASA Received with Result-Code
                         == SUCCESS
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_ASA_OK,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acRxASA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    ASR successfully sent and      Cleanup    Idle
                      ASA Received with Result-Code
                        != SUCCESS
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_ASA_FAIL,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_STR,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanup);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server.
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_LIFETIME_TOUT,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acAuthTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Session-Timeout expires on     Cleanup    Idle
                      home server
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           AAA_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon    Service-specific authorization Discard    Discon
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_TX_SSAA,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acTxSSAADiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon   Service-specific authorization  Discard   Discon
                    request received                service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(AAA_SESSION_AUTH_ST_DISC,
                           AAA_SESSION_AUTH_EV_RX_SSAR,
                           AAA_SESSION_AUTH_ST_DISC,
                           m_acRxSSARDiscard);

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
       AAA_SessAuthServer_RxSSAR_Discard               m_acRxSSARDiscard;
       AAA_SessAuthServer_TxSSAA_Discard               m_acTxSSAADiscard;
       AAA_SessAuthServer_RxSSA                        m_acRxSSA;
       AAA_SessAuthServer_TxSSA                        m_acTxSSA;
       AAA_SessAuthServer_TxRAR                        m_acTxRAR;
       AAA_SessAuthServer_TxSSAA_GrantAccess           m_acGrantAccess;
       AAA_SessAuthServer_TxASR                        m_acTxASR;
       AAA_SessAuthServer_RxRAA                        m_acRxRAA;
       AAA_SessAuthServer_RxASA                        m_acRxASA;
       AAA_SessAuthServer_TxSTACleanup                 m_acTxSTACleanup;
       AAA_SessAuthServer_TxSTACleanup_Idle            m_acTxSTACleanupIdle;
       AAA_SessAuthServer_AuthorizationTimeout         m_acAuthTimeout;
       AAA_SessAuthServer_SessionTimeout               m_acSessionTimeout;
       AAA_SessAuthServer_Cleanup                      m_acCleanup;
};

#endif /* __AAA_SESSION_AUTH_SERVER_FSM_H__ */

