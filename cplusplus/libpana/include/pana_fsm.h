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

#ifndef __PANA_FSM_H__
#define __PANA_FSM_H__

#include "framework.h"
#include "pana_node.h"
#include "pana_session.h"
#include "pana_channel.h"

// #define PANA_FSM_DEBUG 1

typedef enum {
  PANA_ST_OFFLINE = 1,
  PANA_ST_WAIT_PAA,
  PANA_ST_WAIT_SUCC_PBA,
  PANA_ST_WAIT_FAIL_PBA,
  PANA_ST_WAIT_EAP_MSG,
  PANA_ST_WAIT_EAP_RESULT,
  PANA_ST_WAIT_EAP_RESULT_CLOSE,
  PANA_ST_OPEN,
  PANA_ST_WAIT_PRA,
  PANA_ST_WAIT_PAN_OR_PAR,
  PANA_ST_WAIT_PPA,
  PANA_ST_WAIT_PUA,
  PANA_ST_WAIT_PEA,
  PANA_ST_SESS_TERM,
  PANA_ST_CLOSED
} PANA_ST;

typedef enum {
  PANA_EV_MTYPE_PCI = 1,
  PANA_EV_MTYPE_PSR,
  PANA_EV_MTYPE_PSA,
  PANA_EV_MTYPE_PAR,
  PANA_EV_MTYPE_PAN,
  PANA_EV_MTYPE_PRR,
  PANA_EV_MTYPE_PRA,
  PANA_EV_MTYPE_PBR,
  PANA_EV_MTYPE_PBA,
  PANA_EV_MTYPE_PPR,
  PANA_EV_MTYPE_PPA,
  PANA_EV_MTYPE_PTR,
  PANA_EV_MTYPE_PTA,
  PANA_EV_MTYPE_PER,
  PANA_EV_MTYPE_PEA,
  PANA_EV_MTYPE_PUR,
  PANA_EV_MTYPE_PUA
} PANA_MSG_TYPE;

typedef enum {
  PANA_EV_EAP_SUCCESS = 1,
  PANA_EV_EAP_FAILURE,
  PANA_EV_EAP_REQUEST,
  PANA_EV_EAP_RESPONSE,
  PANA_EV_EAP_INVALID_MSG,
  PANA_EV_EAP_TIMEOUT,
  PANA_EV_EAP_RESP_TIMEOUT,
} PANA_EAP_EVENT;

typedef enum {
  PANA_EV_APP_START = 1,
  PANA_EV_APP_PAC_FOUND,
  PANA_EV_APP_REAUTH,
  PANA_EV_APP_TERMINATE,
  PANA_EV_APP_AUTH_USER,
  PANA_EV_APP_UPDATE,
  PANA_EV_APP_PING
} PANA_APP_EVENT;

typedef enum {
  PANA_RESULT_CODE_UNSET = 1,
  PANA_RESULT_CODE_FAIL,
  PANA_RESULT_CODE_SUCCESS
} PANA_RESULT_CODE;

template<class EVENT_VAR, class FSM>
class FsmTimer : public PANA_SessionTimerInterface {
   public:
      FsmTimer(FSM &fsm) :
          m_fsm(fsm) {
      }
      void Schedule(PANA_TID tid, ACE_UINT32 sec) {
          EVENT_VAR ev;
          switch (tid) {
             case PANA_TID_RETRY:
                 ev.Do_ReTransmission();
                 break;
             case PANA_TID_SESSION:
                 ev.Do_SessTimeout();
                 break;
             case PANA_TID_EAP_RESP:
                 if (PANA_CFG_PAC().m_EapPiggyback) {
                     ev.EnableCfg_EapPiggyback();
                 }
                 ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
                 break;
          }
          m_fsm.ScheduleTimer(ev.Get(), sec, 0, ev.Get());
      }
      void Cancel(PANA_TID tid) {
          EVENT_VAR ev;
          switch (tid) {
             case PANA_TID_RETRY:
                 ev.Do_ReTransmission();
                 break;
             case PANA_TID_SESSION:
                 ev.Do_SessTimeout();
                 break;
             case PANA_TID_EAP_RESP:
                 if (PANA_CFG_PAC().m_EapPiggyback) {
                     ev.EnableCfg_EapPiggyback();
                 }
                 ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
                 break;
          }
          m_fsm.CancelTimer(ev.Get());
      }
      FSM &Fms() {
          return m_fsm;
      }
   private:
      FSM &m_fsm;
};

template<class ARG, class CHANNEL>
class PANA_EXPORT PANA_StateMachine : 
   public AAA_StateMachineWithTimer<ARG>, AAA_Job
{
   private:
      class FsmTxChannel : public PANA_SessionTxInterface {
         public:
            FsmTxChannel(CHANNEL &ch) :
               m_Channel(ch) {
            }
            virtual void Send(boost::shared_ptr<PANA_Message> msg) {
               m_Channel.Send(msg);
            }
         private:
            CHANNEL &m_Channel;
      };

   public:

      virtual void InitializeMsgMaps() = 0;
      virtual void FlushMsgMaps() = 0;

      virtual void Notify(AAA_Event event) {
         if (! AAA_StateMachineWithTimer<ARG>::Running()) {
            return;
         }
         m_EventQueue.Enqueue(event);
         Schedule(this);
      }
      virtual void Receive(PANA_Message &msg) {
         m_RxMsgQueue.Enqueue(&msg);
         Schedule(this);
      }
      virtual void Error(int err) { 
         Stop(); 
      }
      virtual void Stop() {
         AAA_StateMachineWithTimer<ARG>::Stop(); 
         AAA_StateMachineWithTimer<ARG>::CancelAllTimer(); 
         while (m_GroupedJob.Job().BacklogSize()) {
             ACE_Time_Value tv(0, 100);
             ACE_OS::sleep(tv);
         }
         m_GroupedJob.Job().Flush(); 
      }
      virtual void Abort() {
         Stop();
         AAA_StateMachineWithTimer<ARG>::actionArg.Reset();
      }
      AAA_GroupedJob &Job() {
          return m_GroupedJob.Job();
      }
      void Wait() {
          AAA_MutexScopeLock lock(m_FsmLock);
      }
      PANA_ST CurrentState() {
          return PANA_ST(AAA_StateMachineWithTimer<ARG>::state);
      }

   protected:
      PANA_StateMachine(ARG &arg, 
                        AAA_StateTable<ARG> &table, 
                        PANA_Node &node,
                        CHANNEL &udp) :
	 AAA_StateMachineWithTimer<ARG>(arg, table, *node.Task().reactor(), "PANA"),
         m_GroupedJob(AAA_GroupedJob::Create(node.Job(), (AAA_JobData*)this)),
         m_TxChannel(udp) {
      }
      virtual ~PANA_StateMachine() { }

      virtual int Serve() {

         AAA_MutexScopeLock lock(m_FsmLock);

         if (! m_RxMsgQueue.IsEmpty()) {
             PANA_Message *msg = m_RxMsgQueue.Dequeue();
             m_MsgHandlers.Receive(*msg);
         }

         if (! m_EventQueue.IsEmpty()) {
             AAA_Event ev = m_EventQueue.Dequeue();
             try {
#if PANA_FSM_DEBUG
                 int prevState = AAA_StateMachineWithTimer<ARG>::state;
                 AAA_LOG((LM_DEBUG, "(%P|%t) Event: %d occurring\n", ev));
#endif
                 AAA_StateMachineWithTimer<ARG>::Event(ev);
#if PANA_FSM_DEBUG
                 AAA_LOG((LM_DEBUG, "(%P|%t) From state: %s to %s\n",
                            StrState(prevState),
                            StrState(AAA_StateMachineWithTimer<ARG>::state)));
#endif
             }
             catch (PANA_Exception &e) {
                 AAA_LOG((LM_ERROR, "(%P|%t) Error[%d]: %s\n",
                            e.code(), e.description().data()));
                 Stop();
             }
             catch (...) {
                 AAA_LOG((LM_ERROR, "(%P|%t) Unknown error during FSM\n"));
                 Stop();
             }
         }
         return (0);
      }
      virtual int Schedule(AAA_Job* job, size_t backlogSize = 1) {
         return m_GroupedJob->Schedule(job, backlogSize);
      }
      virtual void Timeout(AAA_Event ev) {
         Notify(ev);
      }

   protected:
      AAA_ProtectedQueue<AAA_Event> m_EventQueue;
      AAA_ProtectedQueue<PANA_Message*> m_RxMsgQueue;
      AAA_JobHandle<AAA_GroupedJob> m_GroupedJob;
      PANA_SessionRxInterfaceTable<ARG> m_MsgHandlers;
      FsmTxChannel m_TxChannel;
      ACE_Mutex m_FsmLock;

   private:
       const char *StrState(int state) {
           static char *str[] = { "OFFLINE",
                                  "WAIT_EAP_MSG_IN_INIT",
                                  "WAIT_PAC_IN_INIT",
                                  "WAIT_PAA",
                                  "WAIT_SUCC_PBA",
                                  "WAIT_FAIL_PBA",
                                  "WAIT_EAP_MSG",
                                  "WAIT_EAP_RESULT",
                                  "WAIT_EAP_RESULT_CLOSE",
                                  "OPEN",
                                  "WAIT_PRA",
                                  "WAIT_PAN_OR_PAR",
                                  "WAIT_PPA",
                                  "WAIT_PUA",
                                  "WAIT_PEA",
                                  "SESS_TERM",
                                  "CLOSED" };
           return str[state - 1];
       }
};

template<class ARG, class SESSION>
class PANA_SessionRxStateFilter : public PANA_SessionRxInterface<ARG>
{
   public:
      PANA_SessionRxStateFilter(ARG &arg, SESSION &s,
                                PANA_ST *states = NULL,
                                int count = 0) :
          PANA_SessionRxInterface<ARG>(arg),
          m_Session(s),
          m_AllowedStates(states),
          m_Count(count) {
      }
      virtual void operator()(PANA_Message &msg) {
          if (Allowed()) {
              HandleMessage(msg);
          }
          else {
              delete &msg;
          }
      }
      virtual void HandleMessage(PANA_Message &msg) = 0;

   protected:
      bool Allowed() {
          for (int i=0; i<m_Count; i++) {
              if (m_AllowedStates[i] == m_Session.CurrentState()) {
                  return true;
              }
          }
          return (m_Count == 0) ? true : false;
      }
      virtual void AllowedStates(PANA_ST *states,
                                 int count) {
          m_AllowedStates = states;
          m_Count = count;
      }

   protected:
      SESSION &m_Session;

   private:
      PANA_ST *m_AllowedStates;
      int m_Count;
};

#endif // __PANA_FSM_H__
