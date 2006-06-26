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

#ifndef __AAA_SESSION_ACCT_FSM_H__
#define __AAA_SESSION_ACCT_FSM_H__

#include "aaa_session.h"
#include "aaa_session_fsm.h"

#define AAA_SESSION_ACCT_DEBUG 0

typedef enum {
    AAA_SESSION_ACCT_ST_IDLE,
    AAA_SESSION_ACCT_ST_OPEN,
    AAA_SESSION_ACCT_ST_PENDING_S,
    AAA_SESSION_ACCT_ST_PENDING_I,
    AAA_SESSION_ACCT_ST_PENDING_E,
    AAA_SESSION_ACCT_ST_PENDING_B,
    AAA_SESSION_ACCT_ST_PENDING_L,
} AAA_SESSION_ACCT_ST;

typedef enum {
    AAA_SESSION_ACCT_EV_REQUEST_ACCESS,
    AAA_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS,
    AAA_SESSION_ACCT_EV_REC_IN_STORAGE,
    AAA_SESSION_ACCT_EV_RX_ACA_OK,
    AAA_SESSION_ACCT_EV_FTS_NOT_DAG,
    AAA_SESSION_ACCT_EV_FTS_AND_GAL,
    AAA_SESSION_ACCT_EV_FTS_NOT_GAL,
    AAA_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL,
    AAA_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL,
    AAA_SESSION_ACCT_EV_INT_EXPIRE,
    AAA_SESSION_ACCT_EV_RX_ACA_FAIL,
    AAA_SESSION_ACCT_EV_FTS,
    AAA_SESSION_ACCT_EV_FTS_BUF,
    AAA_SESSION_ACCT_EV_FTS_NO_BUF,
    AAA_SESSION_ACCT_EV_RX_ACR_START_OK,
    AAA_SESSION_ACCT_EV_RX_ACR_EV_OK,
    AAA_SESSION_ACCT_EV_RX_ACR_INT_OK,
    AAA_SESSION_ACCT_EV_RX_ACR_STOP_OK,
    AAA_SESSION_ACCT_EV_RX_ACR_NO_BUF,
    AAA_SESSION_ACCT_EV_SESSION_TOUT,
    AAA_SESSION_ACCT_EV_STOP
} AAA_SESSION_EV_ACCT;

class AAA_SessionAcctFsmDebug
{
   public:
      void DumpEvent(AAA_State state, AAA_Event ev) {
#if AAA_SESSION_ACCT_DEBUG 
          static char *evStrTable[] = { "AAA_SESSION_ACCT_EV_REQUEST_ACCESS",
                                        "AAA_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS",
                                        "AAA_SESSION_ACCT_EV_REC_IN_STORAGE",
                                        "AAA_SESSION_ACCT_EV_RX_ACA_OK",
                                        "AAA_SESSION_ACCT_EV_FTS_NOT_DAG",
                                        "AAA_SESSION_ACCT_EV_FTS_AND_GAL",
                                        "AAA_SESSION_ACCT_EV_FTS_NOT_GAL",
                                        "AAA_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL",
                                        "AAA_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL",
                                        "AAA_SESSION_ACCT_EV_INT_EXPIRE",
                                        "AAA_SESSION_ACCT_EV_RX_ACA_FAIL",
                                        "AAA_SESSION_ACCT_EV_FTS",
                                        "AAA_SESSION_ACCT_EV_FTS_BUF",
                                        "AAA_SESSION_ACCT_EV_FTS_NO_BUF",
                                        "AAA_SESSION_ACCT_EV_RX_ACR_START_OK",
                                        "AAA_SESSION_ACCT_EV_RX_ACR_EV_OK",
                                        "AAA_SESSION_ACCT_EV_RX_ACR_INT_OK",
                                        "AAA_SESSION_ACCT_EV_RX_ACR_STOP_OK",
                                        "AAA_SESSION_ACCT_EV_RX_ACR_NO_BUF",
                                        "AAA_SESSION_ACCT_EV_SESSION_TOUT",
                                        "AAA_SESSION_ACCT_EV_STOP"
          };
          static char *stStrTable[] = { "AAA_SESSION_ACCT_ST_IDLE",
                                        "AAA_SESSION_ACCT_ST_OPEN",
                                        "AAA_SESSION_ACCT_ST_PENDING_S",
                                        "AAA_SESSION_ACCT_ST_PENDING_I",
                                        "AAA_SESSION_ACCT_ST_PENDING_E",
                                        "AAA_SESSION_ACCT_ST_PENDING_B",
                                        "AAA_SESSION_ACCT_ST_PENDING_L"
          };
          AAA_LOG(LM_INFO, "(%P|%t) Acct session event [state=%s, event=%s]\n",
                  stStrTable[state], evStrTable[ev]);
#endif
      }
};

template<class ARG>
class AAA_AcctSessionStateMachine :
    public AAA_SessionStateMachine<ARG, AAA_SessionAcctFsmDebug>
{
   public:
      virtual ~AAA_AcctSessionStateMachine() {
      }    
      AAA_AcctSession &Session() {
          return m_Session;
      }
      AAA_AcctSessionAttributes &Attributes() {
          return m_Session.Attributes();
      }

   protected:
      AAA_AcctSessionStateMachine(AAA_Task &t,
                                  AAA_StateTable<ARG> &table,
                                  ARG &arg,
                                  AAA_AcctSession &s) :
          AAA_SessionStateMachine<ARG, AAA_SessionAcctFsmDebug>
           (t, table, arg),
          m_Session(s) {
      }

   protected:
      AAA_AcctSession &m_Session;
};

#endif /* __AAA_SESSION_ACCT_FSM_H__ */

