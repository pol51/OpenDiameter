
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

               ACE_UINT32 m_Cfg_OptimizedHshk    : 1;

               ACE_UINT32 m_Do_Authorize         : 1;
               ACE_UINT32 m_Do_Ping              : 1;
               ACE_UINT32 m_Do_RetryTimeout      : 1;
               ACE_UINT32 m_Do_ReTransmission    : 1;
               ACE_UINT32 m_Do_EapRespTimeout    : 1;
               ACE_UINT32 m_Do_FatalError        : 1;
               ACE_UINT32 m_Do_SessTimeout       : 1;

               ACE_UINT32 m_AvpExist_EapPayload  : 1;

               ACE_UINT32 m_Reserved             : 10;
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
        void EnableCfg_OptimizedHandshake(bool set = true) {
            m_Event.i.m_Cfg_OptimizedHshk = set;
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
        void Do_FatalError(bool set = true) {
            m_Event.i.m_Do_FatalError = set;
        }
        void Do_SessTimeout(bool set = true) {
            m_Event.i.m_Do_SessTimeout = set;
        }
        void AvpExist_EapPayload(bool set = true) {
            m_Event.i.m_AvpExist_EapPayload = set;
        }
        void EnableCfg_EapPiggyback(bool set = true) {
            // un-used, just to satisfy template requirements
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
                AAA_LOG((LM_DEBUG, "Eap[%d] ", m_Event.i.m_Event_Eap));
            if (m_Event.i.m_Cfg_OptimizedHshk)
                AAA_LOG((LM_DEBUG, "EapOptimized "));
            if (m_Event.i.m_Do_Authorize)
                AAA_LOG((LM_DEBUG, "DoAuth "));
            if (m_Event.i.m_Do_Ping)
                AAA_LOG((LM_DEBUG, "DoPing "));
            if (m_Event.i.m_Do_RetryTimeout)
                AAA_LOG((LM_DEBUG, "RetryTout "));
            if (m_Event.i.m_Do_ReTransmission)
                AAA_LOG((LM_DEBUG, "Retran "));
            if (m_Event.i.m_Do_EapRespTimeout)
                AAA_LOG((LM_DEBUG, "EapRespTout "));
            if (m_Event.i.m_Do_FatalError)
                AAA_LOG((LM_DEBUG, "Fatal "));
            if (m_Event.i.m_Do_SessTimeout)
                AAA_LOG((LM_DEBUG, "SessTout "));
            if (m_Event.i.m_AvpExist_EapPayload)
                AAA_LOG((LM_DEBUG, "EapPayload "));
            AAA_LOG((LM_DEBUG, "\n"));
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
              p.TxPBR(PANA_RCODE_SUCCESS, PANA_Paa::EAP_SUCCESS);
          }
      };
      class PaaExitActionTxPBREapSuccessFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) {
              p.TxPBR(PANA_RCODE_AUTHENTICATION_REJECTED,
                      PANA_Paa::EAP_SUCCESS);
          }
      };
      class PaaExitActionTxPBREapFail : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) {
              p.TxPBR(PANA_RCODE_AUTHENTICATION_REJECTED,
                      PANA_Paa::EAP_FAILURE);
          }
      };
      class PaaExitActionTxPBREapTimeout : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) {
              p.TxPBR(PANA_RCODE_AUTHENTICATION_REJECTED, 
                      PANA_Paa::EAP_TIMEOUT);
          }
      };
      class PaaExitActionTxPER : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPER(PANA_ERROR_UNABLE_TO_COMPLY);
          }
      };
      class PaaExitActionTxPEA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.TxPEA();
          }
      };
      class PaaWaitPEAExitActionRxPEA : public AAA_Action<PANA_Paa> {
          virtual void operator()(PANA_Paa &p) { 
              p.RxPEA();
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
      class PaaOpenExitActionRxPRR : public AAA_Action<PANA_Paa> {
           virtual void operator()(PANA_Paa &p) { 
               p.RxPRR();
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
      PaaExitActionTxPBREapTimeout            m_PaaExitActionTxPBREapTimeout;
      PaaExitActionTxPBREapSuccessFail        m_PaaExitActionTxPBREapSuccessFail;
      PaaExitActionTxPER                      m_PaaExitActionTxPER;
      PaaExitActionTxPEA                      m_PaaExitActionTxPEA;
      PaaWaitPEAExitActionRxPEA               m_PaaWaitPEAExitActionRxPEA;
      PaaExitActionRxPBASuccess               m_PaaExitActionRxPBASuccess;
      PaaExitActionRxPBAFail                  m_PaaExitActionRxPBAFail;
      PaaOpenExitActionRxPPR                  m_PaaOpenExitActionRxPPR;
      PaaOpenExitActionRxPRR                  m_PaaOpenExitActionRxPRR;
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
      PaaExitActionTimeout                    m_PaaExitActionTimeout;
      PaaExitActionRetransmission             m_PaaExitActionRetransmission;
};

class PANA_EXPORT PANA_PaaSessionChannel
{
   public:
      PANA_PaaSessionChannel(PANA_Node &n) :
          m_Node(n),
          m_Channel(n.Job(), "PAA Channel") {
          ACE_INET_Addr paaAddr(PANA_CFG_GENERAL().m_ListenPort);
          m_Channel.Open(paaAddr);
      }
      virtual ~PANA_PaaSessionChannel() {
          m_Channel.Close();
      }
      PANA_Node &Node() {
          return m_Node;
      }
      PANA_Channel &Channel() {
          return m_Channel;
      }
      void RegisterHandler(OD_Utl_CbFunction1<PANA_Message&> &h) {
          m_Channel.RegisterHandler(h);
      }
      void RemoveHandler() {
          m_Channel.RemoveHandler();
      }

   protected:
      PANA_Node &m_Node;
      PANA_Channel m_Channel;
};

class PANA_PaaSessionFactory;
class PANA_EXPORT PANA_PaaSession : public
             PANA_StateMachine<PANA_Paa, PANA_Channel>
{
   private:
      friend class PANA_PaaSessionFactory;

   public:
      PANA_PaaSession(PANA_PaaSessionChannel &ch,
                      PANA_PaaEventInterface &eif) :
         PANA_StateMachine<PANA_Paa, PANA_Channel>
                  (m_PAA, m_StateTable, ch.Node(), ch.Channel()),
         m_Timer(*this),
         m_PAA(m_TxChannel, m_Timer, eif) {
         InitializeMsgMaps();
         PANA_StateMachine<PANA_Paa, PANA_Channel>::Start(); 
      }
      virtual ~PANA_PaaSession() { 
         PANA_StateMachine<PANA_Paa, PANA_Channel>::Stop(); 
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
      virtual void Update(ACE_INET_Addr &addr);
      virtual void Ping();
      virtual void Stop();

      ACE_UINT32 &SessionId() {
         return m_PAA.SessionId();
      }
      ACE_INET_Addr &PacAddress() {
         return m_PAA.PacAddress();
      }

   private:
      void InitializeMsgMaps();
      void FlushMsgMaps();

      static PANA_PaaStateTable m_StateTable;
      FsmTimer<PANA_PaaEventVariable, PANA_PaaSession> m_Timer;
      PANA_Paa m_PAA;
};

typedef OD_Utl_DbaseTree<ACE_UINT32, PANA_PaaSession> PANA_SessionDb;
typedef ACE_Singleton<PANA_SessionDb, ACE_Recursive_Thread_Mutex> PANA_SessionDb_S;

#define PANA_SESSIONDB()           PANA_SessionDb_S::instance()
#define PANA_SESSIONDB_ADD(x, y)   PANA_SessionDb_S::instance()->Add((x), (y))
#define PANA_SESSIONDB_DEL(x, y)   PANA_SessionDb_S::instance()->Remove((x), (y))
#define PANA_SESSIONDB_SEARCH(x)   PANA_SessionDb_S::instance()->Search((x))

#endif /* __PANA_PAA_FSM_H__ */

