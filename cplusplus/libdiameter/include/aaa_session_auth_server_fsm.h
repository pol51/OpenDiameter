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

#ifndef __AAA_SESSION_AUTH_SERVER_FSM_H__
#define __AAA_SESSION_AUTH_SERVER_FSM_H__

#include "aaa_session_auth_fsm.h"

class DIAMETERBASEPROTOCOL_EXPORT DiameterAuthSessionServerStateMachine :
    public DiameterAuthSessionStateMachine<DiameterAuthSessionServerStateMachine>
{  
   public:
   public:
      DiameterAuthSessionServerStateMachine(AAA_Task &t,
                                        DiameterAuthSession &a);
      virtual ~DiameterAuthSessionServerStateMachine() {
      }

      void TxASR();
      void TxSTA(diameter_unsigned32_t rcode);
      void TxRAR(diameter_unsigned32_t rcode);
      void RxSTR(DiameterMsg &msg);
      void RxASA(DiameterMsg &msg);
      void RxRAA(DiameterMsg &msg);

      bool &ASRSent() {
          return m_ASRSent;
      }

   protected:
       bool m_ASRSent;
};

class DiameterSessAuthServer_RxSSAR_Discard : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
          AAA_LOG((LM_INFO, "(%P|%t) Message sent in invalid session state, discarding\n"));
          DiameterMsgHeaderDump::Dump(*msg);
      }
};

class DiameterSessAuthServer_RxSSA : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
          fsm.Session().RxDelivery(msg);
      }
};

class DiameterSessAuthServer_TxSSAA_Discard : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
          AAA_LOG((LM_INFO, "(%P|%t) Message received in invalid session state, discarding\n"));
          DiameterMsgHeaderDump::Dump(*msg);
      }
};

class DiameterSessAuthServer_TxSSA : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          std::auto_ptr<DiameterMsg> msg = fsm.PendingMsg();
          fsm.Session().TxDelivery(msg);

          if (fsm.Attributes().AuthSessionState()() !=
              DIAMETER_SESSION_STATE_MAINTAINED) {
              fsm.CancelAllTimer();

              AAA_LOG((LM_INFO, "(%P|%t) Server session in stateless mode, extending access time\n"));

              if (fsm.Attributes().SessionTimeout()() > 0) {
                  fsm.ScheduleTimer(DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                                    fsm.Attributes().SessionTimeout()() +
                                    AAA_AUTH_SESSION_GRACE_PERIOD,
                                    0, DIAMETER_TIMER_TYPE_SESSION);

                  AAA_LOG((LM_INFO, "(%P|%t) Session Timeout: %d\n", 
                                  fsm.Attributes().SessionTimeout()()));
                  AAA_LOG((LM_INFO, "(%P|%t) Auth Lifetime  : %d\n",
                                  fsm.Attributes().AuthLifetime()()));
                  AAA_LOG((LM_INFO, "(%P|%t) Grace Period   : %d\n",
                                  fsm.Attributes().AuthGrace()()));
              }
              else {
                  fsm.ScheduleTimer(DIAMETER_SESSION_AUTH_EV_SESSION_RECLAIM,
                                    AAA_AUTH_SESSION_RECLAMATION_PERIOD,
                                    0, DIAMETER_TIMER_TYPE_SESSION);

                  AAA_LOG((LM_INFO, "(%P|%t) Reclamation Timeout: %d\n",
                                  AAA_AUTH_SESSION_RECLAMATION_PERIOD));
              }

              fsm.Session().Success();
          }
      }
};

class DiameterSessAuthServer_TxSSAA_GrantAccess : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.ScheduleTimer(DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_ST,
              fsm.Attributes().SessionTimeout()() +
              AAA_AUTH_SESSION_GRACE_PERIOD,
              0, DIAMETER_TIMER_TYPE_SESSION);
          fsm.ScheduleTimer(DIAMETER_SESSION_AUTH_EV_LIFETIME_TOUT,
              fsm.Attributes().AuthLifetime()() +
              fsm.Attributes().AuthGrace()() +
              AAA_AUTH_SESSION_GRACE_PERIOD,
              0, DIAMETER_TIMER_TYPE_AUTH);
              
          AAA_LOG((LM_INFO, "(%P|%t) Server session in open state\n"));
          AAA_LOG((LM_INFO, "(%P|%t) Session Timeout: %d\n", 
                            fsm.Attributes().SessionTimeout()()));
          AAA_LOG((LM_INFO, "(%P|%t) Auth Lifetime  : %d\n",
                            fsm.Attributes().AuthLifetime()()));
          AAA_LOG((LM_INFO, "(%P|%t) Grace Period   : %d\n",
                            fsm.Attributes().AuthGrace()()));
          fsm.Session().Success();
      }
};

class DiameterSessAuthServer_TxASR : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxASR();
      }
};

class DiameterSessAuthServer_TxRAR : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          if (fsm.Attributes().ReAuthRequestValue().IsSet()) {
              fsm.TxRAR
                (fsm.Attributes().ReAuthRequestValue()().reAuthType);
	  }
      }
};

class DiameterSessAuthServer_RxRAA : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          if (fsm.Attributes().ReAuthRequestValue().IsSet()) {
              fsm.Session().ReAuthenticate
                 (fsm.Attributes().ReAuthRequestValue()().resultCode);
	  }
      }
};

class DiameterSessAuthServer_TxSTACleanup : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTA(AAA_AUTHORIZATION_REJECTED);
          fsm.Session().Disconnect();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class DiameterSessAuthServer_TxSTACleanup_Idle : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.TxSTA(AAA_AUTHORIZATION_REJECTED);
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class DiameterSessAuthServer_RxASA : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
	  fsm.CancelTimer(DIAMETER_TIMER_TYPE_ASR);
          fsm.Session().AbortSession();
      }
};

class DiameterSessAuthServer_AuthorizationTimeout : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.Session().AuthorizationTimeout();
          fsm.CancelAllTimer();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class DiameterSessAuthServer_SessionTimeout : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.Session().SessionTimeout();
          fsm.CancelAllTimer();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class DiameterSessAuthServer_SessionReclaim :
   public AAA_Action<DiameterAuthSessionServerStateMachine>
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          if (fsm.Session().ReClaimSession() == AAA_ERR_SUCCESS) {
              fsm.Session().Disconnect();
              fsm.ASRSent() = false;
              fsm.Session().Reset();
          }
          else {
              AAA_LOG((LM_INFO, "(%P|%t) Scheduling next reclamation Timeout: %d\n",
                                  AAA_AUTH_SESSION_RECLAMATION_PERIOD));

              fsm.ScheduleTimer(DIAMETER_SESSION_AUTH_EV_SESSION_RECLAIM,
                                AAA_AUTH_SESSION_RECLAMATION_PERIOD,
                                0, DIAMETER_TIMER_TYPE_SESSION);
          }
      }
};

class DiameterSessAuthServer_Cleanup : 
   public AAA_Action<DiameterAuthSessionServerStateMachine> 
{
   public:
      virtual void operator()(DiameterAuthSessionServerStateMachine &fsm) {
          fsm.CancelAllTimer();
          fsm.Session().AbortSession();
          fsm.ASRSent() = false;
          fsm.Session().Reset();
      }
};

class AAA_SessAuthServerStateTable : 
   public AAA_StateTable<DiameterAuthSessionServerStateMachine>
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
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_SSAR_OK,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acGrantAccess); 

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Service-specific authorization Send       Idle
                      request received, and          failed serv.
                      user is not authorized         specific answer,
                                                     Cleanup
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_SSAR_FAIL,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_OK,
                           DIAMETER_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_FAIL,
                           DIAMETER_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Any       STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_RX_STR,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanupIdle);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Send       Idle
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_TX_SSAA,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acTxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Idle      Service-specific authorization Accept     Idle
                     request received               service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_RX_SSAR,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server for stateless
                      session.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_NOST,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server for stateless
                      session.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_SESSION_RECLAIM,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acSessionReclaim);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle      Home server wants to           Cleanup    Idle
                      terminate service                      
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                           DIAMETER_SESSION_AUTH_EV_ABORT,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Idle     Any                            None       Idle
         */
        AddWildcardStateTableEntry(DIAMETER_SESSION_AUTH_ST_IDLE,
                                   DIAMETER_SESSION_AUTH_ST_IDLE);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Service-specific authorization Send       Open
                      request received, and user     successful
                      is authorized                  serv. specific
                                                     answer
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_SSAR_OK,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
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
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_SSAR_FAIL,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Home server wants to send      Send       Open
                      Service-specific authorization service
                      request                        specific
                                                     auth req
        */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_TX_SSAR,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acTxSSA);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Service-specific authorization Provide    Open
                      answer received                Service
        */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_SSAA,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Successful Service-specific    Provide    Open
                      authorization answer received  Service
        */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_SSAA_OK,
                           DIAMETER_SESSION_AUTH_ST_OPEN);

       /*
           State     Event                          Action     New State
           -------------------------------------------------------------
 [un-offic] Open      Failed Service-specific        Cleanup    Idle
                      authorization answer received
        */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_SSAA_FAIL,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Home server wants to           Send ASR   Discon
                      terminate the service
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_ABORT,
                           DIAMETER_SESSION_AUTH_ST_DISC,
                           m_acTxASR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_LIFETIME_TOUT,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acAuthTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      Session-Timeout expires on     Cleanup    Idle
                      home server
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_OK,
                           DIAMETER_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Not       ASA Received                   None       No Change.
            Discon
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_FAIL,
                           DIAMETER_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open      STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_STR,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Send       Open
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_TX_SSAA,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acTxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Service-specific authorization Accept     Open
                     request received               service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_SSAR,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acRxSSA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Server initiated re-auth       Send       Open
                                                    Re-Auth
             
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_REAUTH,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acTxRAR);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Open      Re-auth answer received        process    Open
             
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                           DIAMETER_SESSION_AUTH_EV_RX_RAA,
                           DIAMETER_SESSION_AUTH_ST_OPEN,
                           m_acRxRAA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Open     Any                            None       Open
         */
        AddWildcardStateTableEntry(DIAMETER_SESSION_AUTH_ST_OPEN,
                                   DIAMETER_SESSION_AUTH_ST_OPEN);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Failure to send ASR            Wait,      Discon
                                                     resend ASR
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_TX_ASR_FAIL,
                           DIAMETER_SESSION_AUTH_ST_DISC,
                           m_acTxASR);       

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    ASR successfully sent and      Wait STR   Discon
                      ASA Received with Result-Code
                         == SUCCESS
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_OK,
                           DIAMETER_SESSION_AUTH_ST_DISC,
                           m_acRxASA);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    ASR successfully sent and      Cleanup    Idle
                      ASA Received with Result-Code
                        != SUCCESS
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_RX_ASA_FAIL,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acCleanup);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    STR Received                   Send STA,  Idle
                                                     Cleanup.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_RX_STR,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acTxSTACleanup);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Authorization-Lifetime (and    Cleanup    Idle
                      Auth-Grace-Period) expires
                      on home server.
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_LIFETIME_TOUT,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acAuthTimeout);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon    Session-Timeout expires on     Cleanup    Idle
                      home server
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_ST,
                           DIAMETER_SESSION_AUTH_ST_IDLE,
                           m_acSessionTimeout);         

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon    Service-specific authorization Discard    Discon
                     answer sent                    service
                                                    specific
                                                    auth answer
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_TX_SSAA,
                           DIAMETER_SESSION_AUTH_ST_DISC,
                           m_acTxSSAADiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
           Discon   Service-specific authorization  Discard   Discon
                    request received                service
                                                    specific
                                                    auth req
         */
        AddStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                           DIAMETER_SESSION_AUTH_EV_RX_SSAR,
                           DIAMETER_SESSION_AUTH_ST_DISC,
                           m_acRxSSARDiscard);

        /*
           State     Event                          Action     New State
           -------------------------------------------------------------
            Discon   Any                            None       Discon
         */
        AddWildcardStateTableEntry(DIAMETER_SESSION_AUTH_ST_DISC,
                                   DIAMETER_SESSION_AUTH_ST_DISC);

        InitialState(DIAMETER_SESSION_AUTH_ST_IDLE);
      }


   private:
       DiameterSessAuthServer_RxSSAR_Discard               m_acRxSSARDiscard;
       DiameterSessAuthServer_TxSSAA_Discard               m_acTxSSAADiscard;
       DiameterSessAuthServer_RxSSA                        m_acRxSSA;
       DiameterSessAuthServer_TxSSA                        m_acTxSSA;
       DiameterSessAuthServer_TxRAR                        m_acTxRAR;
       DiameterSessAuthServer_TxSSAA_GrantAccess           m_acGrantAccess;
       DiameterSessAuthServer_TxASR                        m_acTxASR;
       DiameterSessAuthServer_RxRAA                        m_acRxRAA;
       DiameterSessAuthServer_RxASA                        m_acRxASA;
       DiameterSessAuthServer_TxSTACleanup                 m_acTxSTACleanup;
       DiameterSessAuthServer_TxSTACleanup_Idle            m_acTxSTACleanupIdle;
       DiameterSessAuthServer_AuthorizationTimeout         m_acAuthTimeout;
       DiameterSessAuthServer_SessionTimeout               m_acSessionTimeout;
       DiameterSessAuthServer_SessionReclaim               m_acSessionReclaim;
       DiameterSessAuthServer_Cleanup                      m_acCleanup;
};

#endif /* __AAA_SESSION_AUTH_SERVER_FSM_H__ */

