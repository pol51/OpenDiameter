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

#ifndef __AAA_SESSION_FSM_H__
#define __AAA_SESSION_FSM_H__

#include "framework.h"
#include "diameter_parser.h"

template<class ARG, class DEBUG>
class DiameterSessionStateMachine : public AAA_StateMachineWithTimer<ARG>
{
   public:
      virtual ~DiameterSessionStateMachine() {
          AAA_StateMachineWithTimer<ARG>::Stop();
      }    
      AAA_State State() {
          return AAA_StateMachineWithTimer<ARG>::state;
      }
      bool IsRunning() {
          return AAA_StateMachine<ARG>::Running();
      }

   private:
      std::auto_ptr<DiameterMsg>  m_PendingMsg;
      ACE_Recursive_Thread_Mutex m_EventFsmMtx;

   public:
      std::auto_ptr<DiameterMsg> &PendingMsg() {
         return m_PendingMsg;
      }
      virtual void Notify(AAA_Event event) {
         AAA_ScopeLock<ACE_Recursive_Thread_Mutex> guard(m_EventFsmMtx);
         Process(event);
      }
      virtual void Notify(AAA_Event event,
                          std::auto_ptr<DiameterMsg> msg) {
         AAA_ScopeLock<ACE_Recursive_Thread_Mutex> guard(m_EventFsmMtx);
         m_PendingMsg = msg;
         Process(event);
      }
   protected:
      DiameterSessionStateMachine(AAA_Task &t,
                              AAA_StateTable<ARG> &table,
                              ARG &arg) :
          AAA_StateMachineWithTimer<ARG>(arg, table, *t.reactor()) {
      }
      virtual void Process(AAA_Event ev) {

         m_Debug.DumpEvent(AAA_StateMachineWithTimer<ARG>::state, ev);

         if (! AAA_StateMachineWithTimer<ARG>::Running()) {
             AAA_LOG((LM_ERROR, "(%P|%t) Session not running\n"));
             return;
         }

         try {
             AAA_StateMachineWithTimer<ARG>::Event(ev);
         }
         catch (DiameterBaseException &err) {
             AAA_LOG((LM_ERROR, "(%P|%t) FSM error[%d]: %s\n",
                        err.Code(), err.Description().c_str()));
         }
         catch (...) {
             AAA_LOG((LM_ERROR, "(%P|%t) Unknown exception in FSM\n"));
         }
      }
      virtual void Timeout(AAA_Event ev) {
         Notify(ev);
      }

   private:   
      DEBUG m_Debug;
};

#endif /* __AAA_SESSION_FSM_H__ */

