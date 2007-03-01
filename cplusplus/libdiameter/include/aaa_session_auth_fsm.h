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

#ifndef __AAA_SESSION_AUTH_FSM_H__
#define __AAA_SESSION_AUTH_FSM_H__

#include "aaa_session.h"
#include "aaa_session_fsm.h"

typedef enum {
    DIAMETER_SESSION_AUTH_ST_IDLE,
    DIAMETER_SESSION_AUTH_ST_PENDING,
    DIAMETER_SESSION_AUTH_ST_OPEN,
    DIAMETER_SESSION_AUTH_ST_DISC,
} DIAMETER_SESSION_AUTH_ST;

typedef enum {
    DIAMETER_SESSION_AUTH_EV_REQUEST_ACCESS,
    DIAMETER_SESSION_AUTH_EV_RX_ASR,
    DIAMETER_SESSION_AUTH_EV_RX_ASA_OK,
    DIAMETER_SESSION_AUTH_EV_RX_ASA_FAIL,
    DIAMETER_SESSION_AUTH_EV_RX_ASR_USID,
    DIAMETER_SESSION_AUTH_EV_RX_ASR_OK,
    DIAMETER_SESSION_AUTH_EV_RX_ASR_RETRY,
    DIAMETER_SESSION_AUTH_EV_TX_ASR_FAIL,
    DIAMETER_SESSION_AUTH_EV_RX_STR,
    DIAMETER_SESSION_AUTH_EV_RX_STA,
    DIAMETER_SESSION_AUTH_EV_TX_SSAR,
    DIAMETER_SESSION_AUTH_EV_RX_SSAR,
    DIAMETER_SESSION_AUTH_EV_TX_SSAA,
    DIAMETER_SESSION_AUTH_EV_RX_SSAA,
    DIAMETER_SESSION_AUTH_EV_RX_RAR,
    DIAMETER_SESSION_AUTH_EV_RX_RAA,
    DIAMETER_SESSION_AUTH_EV_SSAR_OK,
    DIAMETER_SESSION_AUTH_EV_SSAR_FAIL,
    DIAMETER_SESSION_AUTH_EV_SSAA_OK,
    DIAMETER_SESSION_AUTH_EV_SSAA_NOSVC,
    DIAMETER_SESSION_AUTH_EV_SSAA_ERROR,
    DIAMETER_SESSION_AUTH_EV_SSAA_FAIL,
    DIAMETER_SESSION_AUTH_EV_REAUTH,
    DIAMETER_SESSION_AUTH_EV_LIFETIME_TOUT,
    DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_ST,
    DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_NOST,
    DIAMETER_SESSION_AUTH_EV_SESSION_RECLAIM,
    DIAMETER_SESSION_AUTH_EV_ABORT,
    DIAMETER_SESSION_AUTH_EV_STOP,
} DIAMETER_SESSION_EV_AUTH;

class DiameterSessionAuthFsmDebug 
{
   public:
      void DumpEvent(AAA_State state, AAA_Event ev) {
#if AAA_SESSION_AUTH_DEBUG 
          static char *evStrTable[] = { "DIAMETER_SESSION_AUTH_EV_REQUEST_ACCESS",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASR",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASA_OK",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASA_FAIL",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASR_USID",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASR_OK",
                                        "DIAMETER_SESSION_AUTH_EV_RX_ASR_RETRY",
                                        "DIAMETER_SESSION_AUTH_EV_TX_ASR_FAIL",
                                        "DIAMETER_SESSION_AUTH_EV_RX_STR",
                                        "DIAMETER_SESSION_AUTH_EV_RX_STA",
                                        "DIAMETER_SESSION_AUTH_EV_TX_SSAR",
                                        "DIAMETER_SESSION_AUTH_EV_RX_SSAR",
                                        "DIAMETER_SESSION_AUTH_EV_TX_SSAA",
                                        "DIAMETER_SESSION_AUTH_EV_RX_SSAA",
                                        "DIAMETER_SESSION_AUTH_EV_RX_RAR",
                                        "DIAMETER_SESSION_AUTH_EV_RX_RAA",
                                        "DIAMETER_SESSION_AUTH_EV_SSAR_OK",
                                        "DIAMETER_SESSION_AUTH_EV_SSAR_FAIL",
                                        "DIAMETER_SESSION_AUTH_EV_SSAA_OK",
                                        "DIAMETER_SESSION_AUTH_EV_SSAA_NOSVC",
                                        "DIAMETER_SESSION_AUTH_EV_SSAA_ERROR",
                                        "DIAMETER_SESSION_AUTH_EV_SSAA_FAIL",
                                        "DIAMETER_SESSION_AUTH_EV_REAUTH",
                                        "DIAMETER_SESSION_AUTH_EV_LIFETIME_TOUT",
                                        "DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_ST",
                                        "DIAMETER_SESSION_AUTH_EV_SESSION_TOUT_NOST",
                                        "DIAMETER_SESSION_AUTH_EV_SESSION_RECLAIM",
                                        "DIAMETER_SESSION_AUTH_EV_ABORT",
                                        "DIAMETER_SESSION_AUTH_EV_STOP"
          };
          static char *stStrTable[] = { "DIAMETER_SESSION_AUTH_ST_IDLE",
                                        "DIAMETER_SESSION_AUTH_ST_PENDING",
                                        "DIAMETER_SESSION_AUTH_ST_OPEN",
                                        "DIAMETER_SESSION_AUTH_ST_DISC"
          };
          AAA_LOG((LM_INFO, "(%P|%t) Auth session event [state=%s, event=%s]\n",
                    stStrTable[state], evStrTable[ev]));
#endif
      }
};

template<class ARG>
class DiameterAuthSessionStateMachine :
    public DiameterSessionStateMachine<ARG, DiameterSessionAuthFsmDebug>
{
   public:
      virtual ~DiameterAuthSessionStateMachine() {
      }    
      DiameterAuthSession &Session() {
          return m_Session;
      }
      DiameterAuthSessionAttributes &Attributes() {
          return m_Session.Attributes();
      }

   protected:
      DiameterAuthSessionStateMachine(AAA_Task &t,
                                  AAA_StateTable<ARG> &table,
                                  ARG &arg,
                                  DiameterAuthSession &s) :
          DiameterSessionStateMachine<ARG, DiameterSessionAuthFsmDebug>
           (t, table, arg),
          m_Session(s) {
      }

   protected:
      DiameterAuthSession &m_Session;
};

#endif /* __AAA_SESSION_AUTH_FSM_H__ */

