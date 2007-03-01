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

#ifndef __AAA_SESSION_ACCT_FSM_H__
#define __AAA_SESSION_ACCT_FSM_H__

#include "aaa_session.h"
#include "aaa_session_fsm.h"

typedef enum {
    DIAMETER_SESSION_ACCT_ST_IDLE,
    DIAMETER_SESSION_ACCT_ST_OPEN,
    DIAMETER_SESSION_ACCT_ST_PENDING_S,
    DIAMETER_SESSION_ACCT_ST_PENDING_I,
    DIAMETER_SESSION_ACCT_ST_PENDING_E,
    DIAMETER_SESSION_ACCT_ST_PENDING_B,
    DIAMETER_SESSION_ACCT_ST_PENDING_L,
} DIAMETER_SESSION_ACCT_ST;

typedef enum {
    DIAMETER_SESSION_ACCT_EV_REQUEST_ACCESS,
    DIAMETER_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS,
    DIAMETER_SESSION_ACCT_EV_REC_IN_STORAGE,
    DIAMETER_SESSION_ACCT_EV_RX_ACA_OK,
    DIAMETER_SESSION_ACCT_EV_FTS_NOT_DAG,
    DIAMETER_SESSION_ACCT_EV_FTS_AND_GAL,
    DIAMETER_SESSION_ACCT_EV_FTS_NOT_GAL,
    DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL,
    DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL,
    DIAMETER_SESSION_ACCT_EV_INT_EXPIRE,
    DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL,
    DIAMETER_SESSION_ACCT_EV_FTS,
    DIAMETER_SESSION_ACCT_EV_FTS_BUF,
    DIAMETER_SESSION_ACCT_EV_FTS_NO_BUF,
    DIAMETER_SESSION_ACCT_EV_RX_ACR_START_OK,
    DIAMETER_SESSION_ACCT_EV_RX_ACR_EV_OK,
    DIAMETER_SESSION_ACCT_EV_RX_ACR_INT_OK,
    DIAMETER_SESSION_ACCT_EV_RX_ACR_STOP_OK,
    DIAMETER_SESSION_ACCT_EV_RX_ACR_NO_BUF,
    DIAMETER_SESSION_ACCT_EV_SESSION_TOUT,
    DIAMETER_SESSION_ACCT_EV_STOP
} AAA_SESSION_EV_ACCT;

class DiameterSessionAcctFsmDebug
{
   public:
      void DumpEvent(AAA_State state, AAA_Event ev) {
#if AAA_SESSION_ACCT_DEBUG 
          static char *evStrTable[] = { "DIAMETER_SESSION_ACCT_EV_REQUEST_ACCESS",
                                        "DIAMETER_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS",
                                        "DIAMETER_SESSION_ACCT_EV_REC_IN_STORAGE",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACA_OK",
                                        "DIAMETER_SESSION_ACCT_EV_FTS_NOT_DAG",
                                        "DIAMETER_SESSION_ACCT_EV_FTS_AND_GAL",
                                        "DIAMETER_SESSION_ACCT_EV_FTS_NOT_GAL",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL",
                                        "DIAMETER_SESSION_ACCT_EV_INT_EXPIRE",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACA_FAIL",
                                        "DIAMETER_SESSION_ACCT_EV_FTS",
                                        "DIAMETER_SESSION_ACCT_EV_FTS_BUF",
                                        "DIAMETER_SESSION_ACCT_EV_FTS_NO_BUF",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACR_START_OK",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACR_EV_OK",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACR_INT_OK",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACR_STOP_OK",
                                        "DIAMETER_SESSION_ACCT_EV_RX_ACR_NO_BUF",
                                        "DIAMETER_SESSION_ACCT_EV_SESSION_TOUT",
                                        "DIAMETER_SESSION_ACCT_EV_STOP"
          };
          static char *stStrTable[] = { "DIAMETER_SESSION_ACCT_ST_IDLE",
                                        "DIAMETER_SESSION_ACCT_ST_OPEN",
                                        "DIAMETER_SESSION_ACCT_ST_PENDING_S",
                                        "DIAMETER_SESSION_ACCT_ST_PENDING_I",
                                        "DIAMETER_SESSION_ACCT_ST_PENDING_E",
                                        "DIAMETER_SESSION_ACCT_ST_PENDING_B",
                                        "DIAMETER_SESSION_ACCT_ST_PENDING_L"
          };
          AAA_LOG((LM_INFO, "(%P|%t) Acct session event [state=%s, event=%s]\n",
                  stStrTable[state], evStrTable[ev]));
#endif
      }
};

template<class ARG>
class DiameterAcctSessionStateMachine :
    public DiameterSessionStateMachine<ARG, DiameterSessionAcctFsmDebug>
{
   public:
      virtual ~DiameterAcctSessionStateMachine() {
      }    
      DiameterAcctSession &Session() {
          return m_Session;
      }
      DiameterAcctSessionAttributes &Attributes() {
          return m_Session.Attributes();
      }

   protected:
      DiameterAcctSessionStateMachine(AAA_Task &t,
                                  AAA_StateTable<ARG> &table,
                                  ARG &arg,
                                  DiameterAcctSession &s) :
          DiameterSessionStateMachine<ARG, DiameterSessionAcctFsmDebug>
           (t, table, arg),
          m_Session(s) {
      }

   protected:
      DiameterAcctSession &m_Session;
};

#endif /* __AAA_SESSION_ACCT_FSM_H__ */

