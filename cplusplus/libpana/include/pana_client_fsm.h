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
               ACE_UINT32 m_Ver_Flag            : 1;
               ACE_UINT32 m_Type_Msg            : 5;
               ACE_UINT32 m_Event_App           : 4;

               ACE_UINT32 m_Flag_Separate       : 1;
               ACE_UINT32 m_Flag_SAResumed      : 1;

               ACE_UINT32 m_Cfg_Separate        : 1;
               ACE_UINT32 m_Cfg_AbortOnFirstEap : 1;
               ACE_UINT32 m_Cfg_EapPiggyback    : 1; // dependent

               ACE_UINT32 m_Do_Separate         : 1; // dependent
               ACE_UINT32 m_Do_ResumeSession    : 1;
               ACE_UINT32 m_Do_Ping             : 1;
               ACE_UINT32 m_Do_RetryTimeout     : 1;
               ACE_UINT32 m_Do_ReTransmission   : 1;
               ACE_UINT32 m_Do_FatalError       : 1;
               ACE_UINT32 m_Do_SessTimeout      : 1;

               ACE_UINT32 m_Result_FirstEap     : 2;
               ACE_UINT32 m_Result_Eap          : 2;

               ACE_UINT32 m_AvpExist_KeyId      : 1; // dependent
               ACE_UINT32 m_AvpExist_Auth       : 1;
               ACE_UINT32 m_AvpExist_Cookie     : 1;
               ACE_UINT32 m_AvpExist_EapPayload : 1;
               
               ACE_UINT32 m_Reserved            : 1;
            } i;
            ACE_UINT32 p;
        } EventParams01;
        
        typedef union {
            struct {
               ACE_UINT32 m_Ver_Flag            :  1;               
               ACE_UINT32 m_Event_Eap           :  4;
               
               ACE_UINT32 m_NotSupported_Pcap   :  1;
               ACE_UINT32 m_NotSupported_Ppac   :  1;
               
               ACE_UINT32 m_Cfg_EapPiggyback    :  1;
               
               ACE_UINT32 m_AvpExist_KeyId      :  1;
               
               ACE_UINT32 m_Do_Separate         :  1;
               
               ACE_UINT32 m_Reserved            : 22;
            } i;
            ACE_UINT32 p;
        } EventParams02;
        
        typedef union {
            struct {
               ACE_UINT32 m_Ver_Flag   :  1;
               ACE_UINT32 m_Data       : 31;
            } i;
            ACE_UINT32 p;
            EventParams01 e01;
            EventParams02 e02;            
        } EventParams;

    public:
        PANA_PacEventVariable() {
            m_Event.p = 0;
        }
        void MsgType(PANA_MSG_TYPE type) {
            m_Event.p = 0;
            m_Event.e01.i.m_Type_Msg = type;
        }
        void Event_App(PANA_APP_EVENT event) {
            m_Event.e01.i.m_Event_App = event;
        }
        void EnableFlag_Separate(bool set = true) {
            m_Event.e01.i.m_Flag_Separate = set;
        }
        void EnableCfg_Separate(bool set = true) {
            m_Event.e01.i.m_Cfg_Separate = set;
        }
        void EnableCfg_AbortOnFirstEap(bool set = true) {
            m_Event.e01.i.m_Cfg_AbortOnFirstEap = set;
        }
        void EnableCfg_EapPiggyback(bool set = true) {
            if (! m_Event.i.m_Ver_Flag) {
                m_Event.e01.i.m_Cfg_EapPiggyback = set;
            }
            else {
                m_Event.e02.i.m_Cfg_EapPiggyback = set;
            }
        }
        void Do_Separate(bool set = true) {
            if (! m_Event.i.m_Ver_Flag) {
                m_Event.e01.i.m_Do_Separate = set;
            }
            else {
                m_Event.e02.i.m_Do_Separate = set;
            }
        }
        void Do_ResumeSession(bool set = true) {
            m_Event.e01.i.m_Do_ResumeSession = set;
        }
        void Do_Ping(bool set = true) {
            m_Event.e01.i.m_Do_Ping = set;
        }
        void Do_SessTimeout(bool set = true) {
            m_Event.e01.i.m_Do_SessTimeout = set;
        }
        void Do_RetryTimeout(bool set = true) {
            m_Event.e01.i.m_Do_RetryTimeout = set;
        }
        void Do_ReTransmission(bool set = true) {
            m_Event.e01.i.m_Do_ReTransmission = set;
        }
        void Do_FatalError(bool set = true) {
            m_Event.e01.i.m_Do_FatalError = set;
        }
        void Result_FirstEap(PANA_EAP_RESULT event) {
            m_Event.e01.i.m_Result_FirstEap = event;
        }
        void Result_Eap(PANA_EAP_RESULT event) {
            m_Event.e01.i.m_Result_Eap = event;
        }
        void AvpExist_KeyId(bool set = true) {
            if (! m_Event.i.m_Ver_Flag) {
                m_Event.e01.i.m_AvpExist_KeyId = set;
            }
            else {
                m_Event.e02.i.m_AvpExist_KeyId = set;
            }
        }
        void AvpExist_Auth(bool set = true) {
            m_Event.e01.i.m_AvpExist_Auth = set;
        }
        void AvpExist_Cookie(bool set = true) {
            m_Event.e01.i.m_AvpExist_Cookie = set;
        }
        void AvpExist_EapPayload(bool set = true) {
            m_Event.e01.i.m_AvpExist_EapPayload = set;
        }
        void Event_Eap(PANA_EAP_EVENT event) {
            m_Event.i.m_Ver_Flag = 1;
            m_Event.e02.i.m_Event_Eap = event;
        }
        void NotSupported_Pcap(bool set = true) {
            m_Event.e02.i.m_NotSupported_Pcap = set;
        }
        void NotSupported_Ppac(bool set = true) {
            m_Event.e02.i.m_NotSupported_Ppac = set;
        }
        void Reset() {
            m_Event.p = 0;
        }
        ACE_UINT32 Get() {
            DumpEvent();
            return m_Event.p;
        }
        void DumpEvent() {
#if defined(PANA_DEBUG)
            if (m_Event.i.m_Ver_Flag) DumpParam02(); else DumpParam01();
#endif
        }

    private:
        void DumpParam01() {
            AAA_LOG((LM_DEBUG, "Event01: "));
            if (m_Event.e01.i.m_Type_Msg)
                AAA_LOG((LM_DEBUG, "Msg[%d] ", m_Event.e01.i.m_Type_Msg));
            if (m_Event.e01.i.m_Event_App)
                AAA_LOG((LM_DEBUG, "App[%d] ", m_Event.e01.i.m_Event_App));
            if (m_Event.e01.i.m_Flag_Separate)
                AAA_LOG((LM_DEBUG, "S-flag "));
            if (m_Event.e01.i.m_Flag_SAResumed) 
                AAA_LOG((LM_DEBUG, "SA resumed "));
            if (m_Event.e01.i.m_Cfg_Separate)
                AAA_LOG((LM_DEBUG, "SepCfg "));
            if (m_Event.e01.i.m_Cfg_AbortOnFirstEap)
                AAA_LOG((LM_DEBUG, "Abort1stEap "));
            if (m_Event.e01.i.m_Cfg_EapPiggyback)
                AAA_LOG((LM_DEBUG, "EapPiggy "));
            if (m_Event.e01.i.m_Do_Separate)
                AAA_LOG((LM_DEBUG, "DoSep "));
            if (m_Event.e01.i.m_Do_ResumeSession)
                AAA_LOG((LM_DEBUG, "DoResumeSA "));
            if (m_Event.e01.i.m_Do_Ping)
                AAA_LOG((LM_DEBUG, "DoPing "));
            if (m_Event.e01.i.m_Do_RetryTimeout)
                AAA_LOG((LM_DEBUG, "RetryTout "));
            if (m_Event.e01.i.m_Do_ReTransmission)
                AAA_LOG((LM_DEBUG, "Retran "));
            if (m_Event.e01.i.m_Do_FatalError)
                AAA_LOG((LM_DEBUG, "Fatal "));
            if (m_Event.e01.i.m_Do_SessTimeout)
                AAA_LOG((LM_DEBUG, "SessTout "));
            if (m_Event.e01.i.m_Result_FirstEap)
                AAA_LOG((LM_DEBUG, "1stEAP[%d] ", m_Event.e01.i.m_Result_FirstEap));
            if (m_Event.e01.i.m_Result_Eap)
                AAA_LOG((LM_DEBUG, "EAP[%d] ", m_Event.e01.i.m_Result_Eap));
            if (m_Event.e01.i.m_AvpExist_KeyId)
                AAA_LOG((LM_DEBUG, "keyId "));
            if (m_Event.e01.i.m_AvpExist_Auth)
                AAA_LOG((LM_DEBUG, "Auth "));
            if (m_Event.e01.i.m_AvpExist_Cookie)
                AAA_LOG((LM_DEBUG, "Cookie "));
            if (m_Event.e01.i.m_AvpExist_EapPayload)
                AAA_LOG((LM_DEBUG, "EapPayload "));
            AAA_LOG((LM_DEBUG, "\n"));
        }
        void DumpParam02() {
            AAA_LOG((LM_DEBUG, "Event02: "));
            if (m_Event.e02.i.m_Event_Eap)
                AAA_LOG((LM_DEBUG, "Eap[%d] ", m_Event.e02.i.m_Event_Eap));
            if (m_Event.e02.i.m_NotSupported_Pcap)
                AAA_LOG((LM_DEBUG, "NoPcap "));
            if (m_Event.e02.i.m_NotSupported_Ppac)
                AAA_LOG((LM_DEBUG, "NoPpac "));
            if (m_Event.e02.i.m_Cfg_EapPiggyback)
                AAA_LOG((LM_DEBUG, "EapPiggy "));
            if (m_Event.e02.i.m_AvpExist_KeyId)
                AAA_LOG((LM_DEBUG, "keyId "));
            if (m_Event.e02.i.m_Do_Separate)
                AAA_LOG((LM_DEBUG, "DoSep "));
            AAA_LOG((LM_DEBUG, "\n"));
        }
        EventParams m_Event;
};

class PANA_PacSession;
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
               c.TxPDI(); 
           }
       };
       class PacWaitEapMsgInExitActionTxPSA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPSA(NULL); 
           }
       };
       class PacWaitPaaExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPAR(false); 
           }
       };
       class PacWaitPaaExitActionRxPAN : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPAN(); 
           }
       };
       class PacWaitPaaExitActionRxPFER : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPFER(); 
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
               c.TxPAN(false);
           }
       };
       class PacWaitEapResultExitActionEapOpen : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPBA(false); 
               c.NotifyAuthorization(); 
               c.NotifyScheduleLifetime();
           }
       };
       class PacWaitEapResultCloseExitAction : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPBA(true); 
               c.Disconnect(); 
           }
       };
       class PacWaitEapResultExitActionTxPERPpac : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPER(PANA_PPAC_CAPABILITY_UNSUPPORTED);
           }
       };
       class PacWaitEapResultExitActionTxPERPcap : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPER(PANA_PROTECTION_CAPABILITY_UNSUPPORTED);
           }
       };
       class PacWait1stEapResultExitAction : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPFEA(false); 
               c.NotifyEapRestart();
           }
       };
       class PacWait1stEapResultCloseExitAction : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPFEA(true); 
               c.Disconnect(); 
           }
       };       
       class PacWait1stEapResultRestartExitAction : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.NotifyEapRestart();
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
       class PacOpenExitActionEapReAuth : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPDI(); 
           }
       };
       class PacOpenExitActionRxPAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPAR(true); 
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
       class PacOpenExitActionTxPRAR : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.TxPRAR(); 
           }
       };
       class PacWaitPRAAExitActionRxPRAA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPRAA(); 
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
       class PacWaitPEAExitActionRxPEA : public AAA_Action<PANA_Client> {
           virtual void operator()(PANA_Client &c) { 
               c.RxPEA(true);
           }
       };

   private:
       PacOfflineExitActionRxPSR              m_PacOfflineExitActionRxPSR;
       PacOfflineExitActionAuthUser           m_PacOfflineExitActionAuthUser;
       PacWaitEapMsgInExitActionTxPSA         m_PacWaitEapMsgInExitActionTxPSA;
       PacWaitPaaExitActionRxPAR              m_PacWaitPaaExitActionRxPAR;
       PacWaitPaaExitActionRxPAN              m_PacWaitPaaExitActionRxPAN;
       PacWaitPaaExitActionRxPFER             m_PacWaitPaaExitActionRxPFER;
       PacWaitPaaExitActionRxPBR              m_PacWaitPaaExitActionRxPBR;
       PacWaitEapMsgExitActionTxPAR           m_PacWaitEapMsgExitActionTxPAR;
       PacWaitEapMsgExitActionTxPAN           m_PacWaitEapMsgExitActionTxPAN;
       PacWaitEapMsgExitActionTxPANTout       m_PacWaitEapMsgExitActionTxPANTout;
       PacWaitEapResultExitActionEapOpen      m_PacWaitEapResultExitActionEapOpen;
       PacWaitEapResultCloseExitAction        m_PacWaitEapResultCloseExitAction;
       PacWaitEapResultExitActionTxPERPpac    m_PacWaitEapResultExitActionTxPERPpac;
       PacWaitEapResultExitActionTxPERPcap    m_PacWaitEapResultExitActionTxPERPcap;
       PacWait1stEapResultExitAction          m_PacWait1stEapResultExitAction;
       PacWait1stEapResultCloseExitAction     m_PacWait1stEapResultCloseExitAction;
       PacWait1stEapResultRestartExitAction   m_PacWait1stEapResultRestartExitAction;
       PacOpenExitActionRxPPR                 m_PacOpenExitActionRxPPR;
       PacOpenExitActionTxPPR                 m_PacOpenExitActionTxPPR;
       PacOpenExitActionEapReAuth             m_PacOpenExitActionEapReAuth;
       PacOpenExitActionRxPAR                 m_PacOpenExitActionRxPAR;
       PacOpenExitActionRxPTR                 m_PacOpenExitActionRxPTR;
       PacOpenExitActionTxPTR                 m_PacOpenExitActionTxPTR;
       PacOpenExitActionTxPUR                 m_PacOpenExitActionTxPUR;
       PacOpenExitActionRxPUR                 m_PacOpenExitActionRxPUR;
       PacOpenExitActionTxPRAR                m_PacOpenExitActionTxPRAR;
       PacWaitPRAAExitActionRxPRAA            m_PacWaitPRAAExitActionRxPRAA;
       PacWaitPUAExitActionRxPUA              m_PacWaitPUAExitActionRxPUA;
       PacWaitPPAExitActionRxPPA              m_PacWaitPPAExitActionRxPPA;
       PacSessTermExitActionRxPTA             m_PacSessTermExitActionRxPTA;
       PacExitActionRetransmission            m_PacExitActionRetransmission;
       PacExitActionTimeout                   m_PacExitActionTimeout;
       PacExitActionTxPEA                     m_PacExitActionTxPEA;
       PacWaitPEAExitActionRxPEA              m_PacWaitPEAExitActionRxPEA;
};
class PANA_EXPORT PANA_PacSession : public
      PANA_StateMachine<PANA_Client, PANA_SenderChannel>
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
      virtual void EapReAuthenticate();
      virtual void Update(ACE_INET_Addr &addr, std::string &msg);
      virtual void SendNotification(std::string &msg);
      virtual void Ping();
      virtual void Stop();

      PANA_CfgProviderInfo &PreferedISP() {
         return m_PaC.PreferedISP();
      }
      PANA_CfgProviderInfo &PreferedNAP() {
         return m_PaC.PreferedNAP();
      }
      virtual PANA_DeviceId &PacDeviceId() {
         return m_PaC.PacDeviceId();
      }
      virtual PANA_DeviceId &PaaDeviceId() {
         return m_PaC.PaaDeviceId();
      }
      bool &ResumePreviousSession() {
         return m_PaC.AuxVariables().SecAssociationResumed();
      }
      std::string &CurrentInterface() {
         return PANA_CFG_GENERAL().m_Interface;
      }

   private:
      virtual void InitializeMsgMaps();
      virtual void FlushMsgMaps();
      virtual void GetBindAddress(ACE_INET_Addr &addr);

      static PANA_ClientStateTable m_StateTable;
      PANA_Node &m_Node;
      PANA_SenderChannel m_Udp;
      FsmTimer<PANA_PacEventVariable, PANA_PacSession> m_Timer;
      PANA_Client m_PaC;
};

#endif // __PANA_CLIENT_FSM_H__
