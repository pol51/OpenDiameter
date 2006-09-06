
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

#ifndef __PANA_PAA_FSM_H__
#define __PANA_PAA_FSM_H__

#include "ace/Singleton.h"
#include "od_utl_rbtree_dbase.h"
#include "pana_exports.h"
#include "pana_fsm.h"
#include "pana_paa.h"

class PANA_PaaEventVariable
{
    private:
        typedef union {
            struct {
               ACE_UINT32 m_Type_Msg             : 5;

               ACE_UINT32 m_Event_Eap            : 4;
               ACE_UINT32 m_Event_App            : 4;

               ACE_UINT32 m_Cfg_UseCookie        : 1;
               ACE_UINT32 m_Cfg_PiggyBack        : 1;

               ACE_UINT32 m_Flag_Separate        : 1;

               ACE_UINT32 m_Result_FirstEap      : 2;

               ACE_UINT32 m_Do_AbortOnFirstEap   : 1;
               ACE_UINT32 m_Do_Separate          : 1;
               ACE_UINT32 m_Do_RetreiveSA        : 1;
               ACE_UINT32 m_Do_Authorize         : 1;
               ACE_UINT32 m_Do_Ping              : 1;
               ACE_UINT32 m_Do_RetryTimeout      : 1;
               ACE_UINT32 m_Do_ReTransmission    : 1;
               ACE_UINT32 m_Do_EapRespTimeout    : 1;
               ACE_UINT32 m_Do_Mobility          : 1;
               ACE_UINT32 m_Do_FatalError        : 1;
               ACE_UINT32 m_Do_Missing_DeviceId  : 1;
               ACE_UINT32 m_Do_SessTimeout       : 1;

               ACE_UINT32 m_AvpExist_EapPayload  : 1;
               
               ACE_UINT32 m_Reserved             : 1;
            } i;
            ACE_UINT32 p;
        } EventParams;

    public:
        PANA_PaaEventVariable() {
            m_Event.p = 0;
        }
        void MsgType(PANA_MSG_TYPE type) {
            m_Event.i.m_Type_Msg = type;
        }
        void Event_Eap(PANA_EAP_EVENT event) {
            m_Event.i.m_Event_Eap = event;
        }
        void Event_App(PANA_APP_EVENT event) {
            m_Event.i.m_Event_App = event;
        }
        void EnableCfg_UseCookie(bool set = true) {
            m_Event.i.m_Cfg_UseCookie = set;
        }
        void EnableCfg_PiggyBack(bool set = true) {
            m_Event.i.m_Cfg_PiggyBack = set;
        }
        void EnableFlag_Separate(bool set = true) {
            m_Event.i.m_Flag_Separate = set;
        }
        void Result_FirstEap(PANA_EAP_RESULT event) {
            m_Event.i.m_Result_FirstEap = event;
        }
        void Do_AbortOnFirstEap(bool set = true) {
            m_Event.i.m_Do_AbortOnFirstEap = set;
        }
        void Do_Separate(bool set = true) {
            m_Event.i.m_Do_Separate = set;
        }
        void Do_RetreiveSA(bool set = true) {
            m_Event.i.m_Do_RetreiveSA = set;
        }
        void Do_Authorize(bool set = true) {
            m_Event.i.m_Do_Authorize = set;
        }
        void Do_Ping(bool set = true) {
            m_Event.i.m_Do_Ping = set;
        }
        void Do_RetryTimeout(bool set = true) {
            m_Event.i.m_Do_RetryTimeout = set;
        }
        void Do_ReTransmission(bool set = true) {
            m_Event.i.m_Do_ReTransmission = set;
        }
        void Do_EapRespTimeout(bool set = true) {
            m_Event.i.m_Do_EapRespTimeout = set;
        }
        void Do_Mobility(bool set = true) {
            m_Event.i.m_Do_Mobility = set;
        }
        void Do_FatalError(bool set = true) {
            m_Event.i.m_Do_FatalError = set;
        }
        void Do_MissingId(bool set = true) {
            m_Event.i.m_Do_Missing_DeviceId = set;
        }
        void Do_SessTimeout(bool set = true) {
            m_Event.i.m_Do_SessTimeout = set;
        }
        void AvpExist_EapPayload(bool set = true) {
            m_Event.i.m_AvpExist_EapPayload = set;
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
            ACE_DEBUG((LM_DEBUG, "Event: "));
            if (m_Event.i.m_Type_Msg)
                ACE_DEBUG((LM_DEBUG, "Msg[%d] ", m_Event.i.m_Type_Msg));
            if (m_Event.i.m_Event_App)
                ACE_DEBUG((LM_DEBUG, "App[%d] ", m_Event.i.m_Event_App));
            if (m_Event.i.m_Event_Eap)
                ACE_DEBUG((LM_DEBUG, "Eap[%d] ", m_Event.i.m_Event_Eap));
            if (m_Event.i.m_Cfg_UseCookie)
                ACE_DEBUG((LM_DEBUG, "CookieCfg "));
            if (m_Event.i.m_Cfg_PiggyBack)
                ACE_DEBUG((LM_DEBUG, "EapPiggy "));
            if (m_Event.i.m_Flag_Separate)
                ACE_DEBUG((LM_DEBUG, "S-flag "));
            if (m_Event.i.m_Result_FirstEap)
                ACE_DEBUG((LM_DEBUG, "1stEAP[%d] ", m_Event.i.m_Result_FirstEap));
            if (m_Event.i.m_Do_AbortOnFirstEap)
                ACE_DEBUG((LM_DEBUG, "Abort1stEap "));
            if (m_Event.i.m_Do_Separate)
                ACE_DEBUG((LM_DEBUG, "DoSep "));
            if (m_Event.i.m_Do_RetreiveSA)
                ACE_DEBUG((LM_DEBUG, "DoSA "));
            if (m_Event.i.m_Do_Authorize)
                ACE_DEBUG((LM_DEBUG, "DoAuth "));
            if (m_Event.i.m_Do_Ping)
                ACE_DEBUG((LM_DEBUG, "DoPing "));
            if (m_Event.i.m_Do_RetryTimeout)
                ACE_DEBUG((LM_DEBUG, "RetryTout "));
            if (m_Event.i.m_Do_ReTransmission)
                ACE_DEBUG((LM_DEBUG, "Retran "));
            if (m_Event.i.m_Do_EapRespTimeout)
                ACE_DEBUG((LM_DEBUG, "EapRespTout "));
            if (m_Event.i.m_Do_Mobility)
                ACE_DEBUG((LM_DEBUG, "DoMobility "));
            if (m_Event.i.m_Do_FatalError)
                ACE_DEBUG((LM_DEBUG, "Fatal "));
            if (m_Event.i.m_Do_Missing_DeviceId)
                ACE_DEBUG((LM_DEBUG, "MissingDevId "));
            if (m_Event.i.m_Do_SessTimeout)
                ACE_DEBUG((LM_DEBUG, "SessTout "));
            if (m_Event.i.m_AvpExist_EapPayload)
                ACE_DEBUG((LM_DEBUG, "EapPayload "));
            ACE_DEBUG((LM_DEBUG, "\n"));
#endif
        }

    private:
        EventParams m_Event;
};

class PANA_EXPORT PANA_PaaStateTable : public AAA_StateTable<PANA_Paa>
{
   public:
      class PaaExitActionPaaEapRestart : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.NotifyEapRestart(); 
          }
      };
      class PaaExitActionTxPSR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPSR();
          }
      };
      class PaaExitActionRxPSA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPSA();
          }
      };
      class PaaExitActionEapTimeout : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.NotifyEapTimeout();
          }
      };
      class PaaExitActionTxPAR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPAR();
          }
      };
      class PaaExitActionTxPBREapSuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_SUCCESS, PANA_Paa::EAP_SUCCESS);
          }
      };
      class PaaExitActionTxPBREapSuccessFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_AUTHENTICATION_REJECTED, 
                      PANA_Paa::EAP_SUCCESS);
          }
      };
      class PaaExitActionTxPBREapFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_AUTHENTICATION_REJECTED, 
                      PANA_Paa::EAP_FAILURE);
          }
      };
      class PaaExitActionTxPBREapFailSuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_SUCCESS, 
                      PANA_Paa::EAP_FAILURE);
          }
      };
      class PaaExitActionTxPBREapTimeout : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_AUTHENTICATION_REJECTED, 
                      PANA_Paa::EAP_TIMEOUT);
          }
      };
      class PaaExitActionTxPBREapTimeoutSuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPBR(PANA_SUCCESS, PANA_Paa::EAP_TIMEOUT);
          }
      };
      class PaaExitActionTxPFEREapSuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPFER(PANA_SUCCESS, PANA_Paa::EAP_SUCCESS);
          }
      };
      class PaaExitActionTxPFEREapFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPFER(PANA_AUTHENTICATION_REJECTED,
                       PANA_Paa::EAP_FAILURE);
          }
      };
      class PaaExitActionTxPFEREapTimeout : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPFER(PANA_AUTHENTICATION_REJECTED,
                       PANA_Paa::EAP_TIMEOUT);
          }
      };
      class PaaExitActionTxPER : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPER(PANA_UNABLE_TO_COMPLY);
          }
      };
      class PaaExitActionTxPERMissingAvp : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPER(PANA_MISSING_AVP);
          }
      };
      class PaaExitActionTxPEA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPEA();
          }
      };
      class PaaExitActionRxPER : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPER();
          }
      };
      class PaaWaitPEAExitActionRxPEA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPEA(true);
          }
      };
      class PaaWaitPFEAExitActionRxPFEASuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPFEA(true);
          }
      };
      class PaaWaitPFEAExitActionRxPFEAFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPFEA(false);
          }
      };
      class PaaExitActionRxPBASuccess : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPBA(true);
          }
      };
      class PaaExitActionRxPBAFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPBA(false);
          }
      };
      class PaaOpenExitActionRxPPR : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.RxPPR(); 
           }
      };
      class PaaOpenExitActionRxPRAR : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.RxPRAR(); 
           }
      };
      class PaaOpenExitActionReAuth : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.NotifyEapReAuth();
           }
      };
      class PaaOpenExitActionTxPPR : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.TxPPR();
           }
      };
      class PaaOpenExitActionTxPTR : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.TxPTR(PANA_TERMCAUSE_ADMINISTRATIVE);
           }
      };
      class PaaOpenExitActionRxPTR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPTR();
          }
      };
      class PaaOpenExitActionTxPUR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPUR();
          }
      };
      class PaaWaitPUAExitActionRxPUA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPUA(); 
          }
      };
      class PaaOpenExitActionRxPUR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPUR();
          }
      };
      class PaaWaitPaaExitActionRxPPA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPPA();
          }
      };
      class PaaWaitPANExitActionRxPAN : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPAN();
          }
      };
      class PaaWaitPANExitActionRxPAR : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPAR();
          }
      };
      class PaaSessExitActionRxPTA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.Disconnect();
          }
      };
      class PaaExitActionRxPEA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPEA(false);
          }
      };
      class PaaExitActionTimeout : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.Disconnect(PANA_TERMCAUSE_SESSION_TIMEOUT);
          }
      };
      class PaaExitActionRetransmission : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p);
      };

      typedef AAA_NullAction<PANA_Paa> PaaNullAction;

      PANA_PaaStateTable();

   private:
      PaaExitActionPaaEapRestart              m_PaaExitActionPaaEapRestart;
      PaaExitActionTxPSR                      m_PaaExitActionTxPSR;
      PaaExitActionRxPSA                      m_PaaExitActionRxPSA;
      PaaExitActionEapTimeout                 m_PaaExitActionEapTimeout;
      PaaExitActionTxPAR                      m_PaaExitActionTxPAR;
      PaaExitActionTxPBREapSuccess            m_PaaExitActionTxPBREapSuccess;
      PaaExitActionTxPBREapFail               m_PaaExitActionTxPBREapFail;
      PaaExitActionTxPBREapFailSuccess        m_PaaExitActionTxPBREapFailSuccess;
      PaaExitActionTxPBREapTimeout            m_PaaExitActionTxPBREapTimeout;
      PaaExitActionTxPBREapTimeoutSuccess     m_PaaExitActionTxPBREapTimeoutSuccess;
      PaaExitActionTxPFEREapSuccess           m_PaaExitActionTxPFEREapSuccess;
      PaaExitActionTxPBREapSuccessFail        m_PaaExitActionTxPBREapSuccessFail;
      PaaExitActionTxPFEREapFail              m_PaaExitActionTxPFEREapFail;
      PaaExitActionTxPFEREapTimeout           m_PaaExitActionTxPFEREapTimeout;
      PaaExitActionTxPER                      m_PaaExitActionTxPER;
      PaaExitActionTxPERMissingAvp            m_PaaExitActionTxPERMissingAvp;
      PaaExitActionTxPEA                      m_PaaExitActionTxPEA;
      PaaExitActionRxPER                      m_PaaExitActionRxPER;
      PaaWaitPEAExitActionRxPEA               m_PaaWaitPEAExitActionRxPEA;
      PaaWaitPFEAExitActionRxPFEASuccess      m_PaaWaitPFEAExitActionRxPFEASuccess;
      PaaWaitPFEAExitActionRxPFEAFail         m_PaaWaitPFEAExitActionRxPFEAFail;
      PaaExitActionRxPBASuccess               m_PaaExitActionRxPBASuccess;
      PaaExitActionRxPBAFail                  m_PaaExitActionRxPBAFail;
      PaaOpenExitActionRxPPR                  m_PaaOpenExitActionRxPPR;
      PaaOpenExitActionRxPRAR                 m_PaaOpenExitActionRxPRAR;
      PaaOpenExitActionReAuth                 m_PaaOpenExitActionReAuth;
      PaaOpenExitActionTxPPR                  m_PaaOpenExitActionTxPPR;
      PaaOpenExitActionTxPTR                  m_PaaOpenExitActionTxPTR;
      PaaOpenExitActionRxPTR                  m_PaaOpenExitActionRxPTR;
      PaaOpenExitActionTxPUR                  m_PaaOpenExitActionTxPUR;
      PaaOpenExitActionRxPUR                  m_PaaOpenExitActionRxPUR;
      PaaWaitPUAExitActionRxPUA               m_PaaWaitPUAExitActionRxPUA;
      PaaWaitPaaExitActionRxPPA               m_PaaWaitPaaExitActionRxPPA;
      PaaWaitPANExitActionRxPAN               m_PaaWaitPANExitActionRxPAN;
      PaaWaitPANExitActionRxPAR               m_PaaWaitPANExitActionRxPAR;
      PaaSessExitActionRxPTA                  m_PaaSessExitActionRxPTA;
      PaaExitActionRxPEA                      m_PaaExitActionRxPEA;
      PaaExitActionTimeout                    m_PaaExitActionTimeout;
      PaaExitActionRetransmission             m_PaaExitActionRetransmission;
};

class PANA_EXPORT PANA_PaaSessionChannel
{
   public:
      PANA_PaaSessionChannel(PANA_Node &n) :
          m_Node(n),
          m_UdpChannel(n.Job(), "PAA Channel") {
          char strAddr[256];
          ACE_OS::sprintf(strAddr, "%s:%d",
                          PANA_CFG_PAA().m_McastAddress.data(),
                          PANA_CFG_GENERAL().m_ListenPort);
          ACE_INET_Addr mcastAddr(strAddr);
          m_UdpChannel.Open(mcastAddr);
      }
      ~PANA_PaaSessionChannel() {
          m_UdpChannel.Close();
      }
      PANA_Node &Node() {
          return m_Node;
      }
      PANA_ListenerChannel &UdpChannel() {
          return m_UdpChannel;
      }
      void RegisterHandler(OD_Utl_CbFunction1<PANA_Message&> &h) {
          m_UdpChannel.RegisterHandler(h);
      }
      void RemoveHandler() {
          m_UdpChannel.RemoveHandler();
      }
          
   protected:
      PANA_Node &m_Node;
      PANA_ListenerChannel m_UdpChannel;
};

class PANA_PaaSessionFactory;
class PANA_EXPORT PANA_PaaSession : public
             PANA_StateMachine<PANA_Paa, PANA_ListenerChannel>
{
   private:
      friend class PANA_PaaSessionFactory;

   public:
      PANA_PaaSession(PANA_PaaSessionChannel &ch,
                      PANA_PaaEventInterface &eif) :
         PANA_StateMachine<PANA_Paa, PANA_ListenerChannel>
                  (m_PAA, m_StateTable, ch.Node(), ch.UdpChannel()),
         m_Timer(*this),
         m_PAA(m_TxChannel, m_Timer, eif) {
         InitializeMsgMaps();
         PANA_StateMachine<PANA_Paa, PANA_ListenerChannel>::Start(); 
      }
      virtual ~PANA_PaaSession() { 
         PANA_StateMachine<PANA_Paa, PANA_ListenerChannel>::Stop(); 
         FlushMsgMaps(); 
      }
      virtual void Start() throw (AAA_Error) { 
      }
      virtual void EapSendRequest(AAAMessageBlock *req);
      virtual void EapInvalidMessage();
      virtual void EapSuccess(AAAMessageBlock *req = 0);
      virtual void EapFailure(AAAMessageBlock *req = 0);
      virtual void EapTimeout();
      virtual void EapReAuthenticate();
      virtual void Update(ACE_INET_Addr &addr, std::string &msg);
      virtual void SendNotification(std::string &msg);
      virtual void Ping();
      virtual void Stop();

      PANA_CfgProviderInfo &PreferedISP() {
         return m_PAA.PreferedISP();
      }
      PANA_CfgProviderInfo &PreferedNAP() {
         return m_PAA.PreferedNAP();
      }
      std::string &SessionId() {
         return m_PAA.SessionId();
      }
      PANA_DeviceId &PeerDeviceId() {
         return m_PAA.PacDeviceId();
      }
   private:
      void InitializeMsgMaps();
      void FlushMsgMaps();

      static PANA_PaaStateTable m_StateTable;
      FsmTimer<PANA_PaaEventVariable, PANA_PaaSession> m_Timer;
      PANA_Paa m_PAA;
};

typedef OD_Utl_DbaseTree<std::string, PANA_PaaSession> PANA_SessionDb;
typedef ACE_Singleton<PANA_SessionDb, ACE_Recursive_Thread_Mutex> PANA_SessionDb_S;

#define PANA_SESSIONDB()           PANA_SessionDb_S::instance()
#define PANA_SESSIONDB_ADD(x, y)   PANA_SessionDb_S::instance()->Add((x), (y))
#define PANA_SESSIONDB_DEL(x, y)   PANA_SessionDb_S::instance()->Remove((x), (y))
#define PANA_SESSIONDB_SEARCH(x)   PANA_SessionDb_S::instance()->Search((x))

#endif /* __PANA_PAA_FSM_H__ */

