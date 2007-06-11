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
/* USA.                                                             f      */
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

#include "pana_paa_fsm.h"

PANA_PaaStateTable PANA_PaaSession::m_StateTable;
typedef PANA_SessionRxStateFilter<PANA_Paa, PANA_PaaSession>
            PANA_ServerRxStateFilter;

PANA_PaaStateTable::PANA_PaaStateTable()
{
    PANA_PaaEventVariable ev;

    /////////////////////////////////////////////////////////////////
    // ------------------------------
    // State: INITIAL (Initial State)
    // ------------------------------
    //
    // Initialization Action:
    //
    //    OPTIMIZED_INIT=Set|Unset;
    //    CARRY_LIFETIME=Set|Unset;
    //    NONCE_SENT=Unset;
    //    RTX_COUNTER=0;
    //    RtxTimerStop();
    //

    ///////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Optimized Handshake) - - - - - - - - - - -
    //  (Rx:PCI[] ||             if (OPTIMIZED_INIT == Set) INITIAL
    //    PAC_FOUND)                EAP_Restart();
    //                           else
    //                              Tx:PAR[S]();
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PAC_FOUND);
    ev.EnableCfg_OptimizedHandshake();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PaaExitActionPaaEapRestart);

    /////////////////////////////////////////////////////////////////
    //  EAP_REQUEST              PAR.insert_avp             INITIAL
    //                            ("EAP-Payload");
    //                            Tx:PAR[S]();
    //                            RtxTimerStart();
    //                            SessionTimerReStart
    //                              (FAILED_SESS_TIMEOUT);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PaaExitActionTxPARStart);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT              None                      INITIAL
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE              None                      INITIAL
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    //  Rx:PAN[S]                if (PAN.exist_avp          WAIT_EAP_MSG
    //                              ("EAP-Payload"))
    //                              TxEAP();
    //                            else {
    //                              EAP_Restart();
    //                              SessionTimerReStart
    //                                (FAILED_SESS_TIMEOUT);
    //                            }
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.FlagStart();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaExitActionRxPANStart);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_INITIAL,
                               PANA_ST_INITIAL);
#endif

    // -------------------
    // State: WAIT_EAP_MSG
    // -------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(Receiving EAP-Request)- - - - - - - - -
    //  EAP_REQUEST              if (key_available())       WAIT_PAN_OR_PAR
    //                              PAR.insert_avp("AUTH");
    //                            if (NONCE_SENT==Unset) {
    //                              PAR.insert_avp("Nonce");
    //                              NONCE_SENT=Set;
    //                            }
    //                            PAR.insert_avp
    //                            ("EAP-Payload");
    //                            Tx:PAR[]();
    //                            RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - -(Receiving EAP-Success/Failure) - - - - -
    //  EAP_FAILURE              PAR.insert_avp             WAIT_FAIL_PAN
    //                            ("EAP-Payload");
    //                            if (key_available())
    //                              PAR.insert_avp("AUTH");
    //                            PAR.RESULT_CODE =
    //                              PANA_AUTHENTICATION_REJECTED;
    //                            Tx:PAR[C]();
    //                            RtxTimerStart();
    //                            SessionTimerStop();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_FAIL_PAN,
                       m_PaaExitActionTxPARCompleteEapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PAR.insert_avp             WAIT_SUCC_PAN
    // Authorize()              ("EAP-Payload");
    //                          if (CARRY_LIFETIME==Set)
    //                            PAR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (new_key_available())
    //                            PAR.insert_avp
    //                            ("Key-Id");
    //                            PAR.insert_avp
    //                            ("Algorithm");
    //                          if (key_available())
    //                            PAR.insert_avp("AUTH");
    //                          PAR.RESULT_CODE =
    //                            PANA_SUCCESS;
    //                          Tx:PAR[C]();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_SUCC_PAN,
                       m_PaaExitActionTxPARCompleteEapSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PAR.insert_avp             WAIT_FAIL_PAN
    // !Authorize()             ("EAP-Payload");
    //                          if (new_key_available())
    //                            PAR.insert_avp
    //                            ("Key-Id");
    //                            PAR.insert_avp
    //                            ("Algorithm");
    //                          if (key_available())
    //                            PAR.insert_avp("AUTH");
    //                          PAR.RESULT_CODE =
    //                            PANA_AUTHORIZATION_REJECTED;
    //                          Tx:PAR[C]();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_FAIL_PAN,
                       m_PaaExitActionTxPARCompleteEapSuccessFail);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - (Receiving EAP-Timeout or invalid message) - - - - -
    // EAP_TIMEOUT ||           SessionTimerStop()          CLOSED
    // EAP_INVALID_MSG          Disconnect()
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG,
                               PANA_ST_WAIT_EAP_MSG);
#endif

    // --------------------
    // State: WAIT_SUCC_PAN
    // --------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (PAN Processing)- - - - - - - - - - -
    // Rx:PAN[C]                RtxTimerStop();                    OPEN
    //                          SessionTimerReStart
    //                          (LIFETIME_SESS_TIMEOUT);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.FlagComplete();
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PAN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaExitActionRxPANCompleteSuccess);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PAN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PAN, ev.Get(),
                       PANA_ST_WAIT_SUCC_PAN,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_SUCC_PAN,
                               PANA_ST_WAIT_SUCC_PAN);
#endif

    // --------------------
    // State: WAIT_FAIL_PAN
    // --------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - (PAN Processing)- - - - - - - - - -
    // Rx:PAN[C]                RtxTimerStop();            CLOSED
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.FlagComplete();
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PAN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionRxPANCompleteFail);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PAN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PAN, ev.Get(),
                       PANA_ST_WAIT_FAIL_PAN,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_FAIL_PAN,
                               PANA_ST_WAIT_FAIL_PAN);
#endif

    // -----------
    // State: OPEN
    // -----------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (re-authentication initiated by PaC) - - - - - -
    // Rx:PNR[A]                if (key_available())       WAIT_EAP_MSG
    //                            PNA.insert_avp("AUTH");
    //                          NONCE_SENT=Unset;
    //                          EAP_Restart();
    //                          Tx:PNA[A]();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagAuth();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionRxPNRAuth);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // REAUTH ||                NONCE_SENT=Unset;           WAIT_EAP_MSG
    // REAUTH_TIMEOUT           EAP_Restart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionReAuth);
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionReAuth);

    /////////////////////////////////////////////////////////////////
    // - - (liveness test based on PNR-PnA exchange initiated by PAA)-
    //  PANA_PING                if (key_available())       WAIT_PNA_PING
    //                              PNR.insert_avp("AUTH");
    //                            Tx:PNR[P]();
    //                            RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PaaOpenExitActionTxPNRPing);

    /////////////////////////////////////////////////////////////////
    // - - (liveness test based on PPR-PPA exchange initiated by PaC)-
    // Rx:PNR[P]                if (key_available())       OPEN
    //                            PNA.insert_avp("AUTH");
    //                          Tx:PNA[P]();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagPing();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaOpenExitActionRxPNRPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PAA) - - - -
    // TERMINATE                if (key_available())       SESS_TERM
    //                            PTR.insert_avp("AUTH");
    //                          Tx:PTR[]();
    //                          SessionTimerStop();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PaaOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PaC) - - - -
    // Rx:PTR[]                 if (key_available())       CLOSED
    //                            PTA.insert_avp("AUTH");
    //                          Tx:PTA[]();
    //                          SessionTimerStop();
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaOpenExitActionRxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OPEN,
                               PANA_ST_OPEN);
#endif

    // --------------------
    // State: WAIT_PNA_PING
    // --------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PPA processing) - - - - - - - - - -
    // Rx:PNA[P]                RtxTimerStop();            OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagPing();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaWaitPaaExitActionRxPNAPing);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PNA,
                               PANA_ST_WAIT_PNA);
#endif

    // ----------------------
    // State: WAIT_PAN_OR_PAR
    // ----------------------

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // Rx:PAR[]                 TxEAP();                   WAIT_EAP_MSG
    //                          if (key_available())
    //                            PAN.insert_avp("AUTH");
    //                          RtxTimerStop();
    //                          Tx:PAN[]();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaWaitPANExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - (Pass EAP Response to the EAP authenticator)- - - -
    // Rx:PAN[]                 TxEAP();                   WAIT_EAP_MSG
    // PAN.exist_avp            RtxTimerStop();
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaWaitPANExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PAN without an EAP response) - - - - - - -
    // Rx:PAN[] &&              RtxTimerStop();            WAIT_PAN_OR_PAR
    // !PAN.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaWaitPANExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(EAP retransmission) - - - - - - - - - -
    // EAP_REQUEST              if (key_available())       WAIT_PAN_OR_PAR
    //                            PAR.insert_avp("AUTH");
    //                          RtxTimerStop();
    //                          PAR.insert_avp("EAP-Payload");
    //                          Tx:PAR[]();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (EAP authentication timeout)- - - - - - - - -
    // EAP_FAILURE ||           RtxTimerStop();           CLOSED
    // EAP_TIMEOUT              SessionTimerStop();
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionEapTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR,
                               PANA_ST_WAIT_PAN_OR_PAR);
#endif

    // ----------------
    // State: SESS_TERM
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PTA processing) - - - - - - - - - -
    // Rx:PTA[                  RtxTimerStop();            CLOSED
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTA);
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaSessExitActionRxPTA);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_SESS_TERM,
                               PANA_ST_SESS_TERM);
#endif

    // -------------
    // State: CLOSED
    // -------------

    /////////////////////////////////////////////////////////////////
    // ANY                      None();                    CLOSED
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_CLOSED,
                               PANA_ST_CLOSED);
#endif

    InitialState(PANA_ST_INITIAL);
}

class PANA_PsmRxPCI : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPCI(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_INITIAL };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                    "PCI message with request flag set, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PCI);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      }
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPCI(*this));
      }
};

class PANA_PsmRxPA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPA(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_INITIAL,
                                           PANA_ST_WAIT_PAN_OR_PAR,
                                           PANA_ST_WAIT_SUCC_PAN,
                                           PANA_ST_WAIT_FAIL_PAN,
                                           PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PaaEventVariable ev;

          if (msg.flags().auth || msg.flags().ping) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PA recevied, invalid flag settings"));
          }
          else if (msg.flags().start) {
              HandleStart(msg, ev);
          }
          else if (msg.flags().complete) {
              HandleComplete(msg, ev);
          }
          else {
              HandleNormal(msg, ev);
          }

          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPA(*this));
      }

   protected:
      virtual void HandleStart(PANA_Message &msg,
                               PANA_PaaEventVariable &ev) {
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                    "PAR-Start request recevied, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve event
          ev.MsgType(PANA_EV_MTYPE_PAN);
          ev.FlagStart();
      }
      virtual void HandleComplete(PANA_Message &msg,
                                  PANA_PaaEventVariable &ev) {
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                    "PAR-Complete request message received, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve event
          ev.MsgType(PANA_EV_MTYPE_PAN);
          ev.FlagComplete();
      }
      virtual void HandleNormal(PANA_Message &msg,
                                PANA_PaaEventVariable &ev) {
          // first level validation
          m_arg.RxValidateMsg(msg);

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          if (! msg.flags().request) {

              ev.MsgType(PANA_EV_MTYPE_PAN);

              PANA_StringAvpContainerWidget eapAvp(msg.avpList());
              pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
              if (payload) {
                  ev.AvpExist_EapPayload();
              }
          }
          else {
              // save address of PaC
              m_arg.PacAddress() = msg.srcAddress();
              ev.MsgType(PANA_EV_MTYPE_PAR);
          }
      }
};

class PANA_PsmRxPN : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPN(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_PNA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PaaEventVariable ev;

          if (msg.flags().start ||
              msg.flags().complete) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PN recevied, invalid flag settings"));
          }
          else if (msg.flags().auth) {
              HandleAuth(msg, ev);
          }
          else if (msg.flags().ping) {
              HandlePing(msg, ev);
          }
          else {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PN recevied with empty flags"));
          }

          // post the event
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPN(*this));
      }

   protected:
      virtual void HandleAuth(PANA_Message &msg,
                               PANA_PaaEventVariable &ev) {
          // first level validation
          if (! msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                    "PNR-Auth answer message received, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.MsgType(PANA_EV_MTYPE_PNR);
          ev.FlagAuth();
      }
      virtual void HandlePing(PANA_Message &msg,
                              PANA_PaaEventVariable &ev) {
          // first level validation
          m_arg.RxValidateMsg(msg);

          // save address of PaC
          if (msg.flags().request) {
              m_arg.PacAddress() = msg.srcAddress();
              ev.MsgType(PANA_EV_MTYPE_PNR);
          }
          else {
              ev.MsgType(PANA_EV_MTYPE_PNA);
          }

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.FlagPing();
      }
};

class PANA_PsmRxPT : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPT(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_SESS_TERM };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);

          // save address of PaC
          if (msg.flags().request) {
             m_arg.PacAddress() = msg.srcAddress();
          }

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(msg.flags().request ?
                     PANA_EV_MTYPE_PTR : PANA_EV_MTYPE_PTA);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPT(*this));
      }
};

void PANA_PaaSession::InitializeMsgMaps()
{
   PANA_PsmRxPCI PCI(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PCI, PCI);

   PANA_PsmRxPA PA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PAR, PA);

   PANA_PsmRxPN PN(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PNR, PN);

   PANA_PsmRxPT PT(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PTR, PT);
}

void PANA_PaaSession::FlushMsgMaps()
{
   m_MsgHandlers.Remove(PANA_MTYPE_PCI);
   m_MsgHandlers.Remove(PANA_MTYPE_PAR);
   m_MsgHandlers.Remove(PANA_MTYPE_PTR);
   m_MsgHandlers.Remove(PANA_MTYPE_PNR);
}

void PANA_PaaSession::EapSendRequest(AAAMessageBlock *req)
{
   PANA_MsgBlockGuard eapPkt(req, true);
   m_PAA.AuxVariables().TxEapMessageQueue().Enqueue(eapPkt.Release());
   PANA_PaaEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_REQUEST);
   Notify(ev.Get());
}

void PANA_PaaSession::EapInvalidMessage()
{
   PANA_PaaEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
   Notify(ev.Get());
}

void PANA_PaaSession::EapSuccess(AAAMessageBlock *req)
{
    if (req) {
        PANA_MsgBlockGuard eapPkt(req, true);
        m_PAA.AuxVariables().TxEapMessageQueue().Enqueue(eapPkt.Release());
    }
    PANA_PaaEventVariable ev;
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    if (m_PAA.IsUserAuthorized()) {
        ev.Do_Authorize();
    }
    Notify(ev.Get());
}

void PANA_PaaSession::EapFailure(AAAMessageBlock *req)
{
    if (req) {
        PANA_MsgBlockGuard eapPkt(req, true);
        m_PAA.AuxVariables().TxEapMessageQueue().Enqueue(eapPkt.Release());
    }
    PANA_PaaEventVariable ev;
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    Notify(ev.Get());
}

void PANA_PaaSession::EapTimeout()
{
    PANA_PaaEventVariable ev;
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    Notify(ev.Get());
 }

void PANA_PaaSession::EapReAuthenticate()
{
    PANA_PaaEventVariable ev;
    ev.Event_App(PANA_EV_APP_REAUTH);
    Notify(ev.Get());
}

void PANA_PaaSession::Ping()
{
   PANA_PaaEventVariable ev;
   ev.Event_App(PANA_EV_APP_PING);
   Notify(ev.Get());
}

void PANA_PaaSession::Stop()
{
   PANA_PaaEventVariable ev;
   ev.Event_App(PANA_EV_APP_TERMINATE);
   Notify(ev.Get());
}

void PANA_PaaStateTable::PaaExitActionRetransmission::
    operator()(PANA_Paa &p)
{
   if (! p.TxLastReqMsg()) {
       PANA_PaaEventVariable ev;
       ev.Do_RetryTimeout();
       FsmTimer<PANA_PaaEventVariable, PANA_PaaSession> &tm =
       static_cast< FsmTimer<PANA_PaaEventVariable, PANA_PaaSession>& >
                  (p.Timer());
       tm.Fms().Notify(ev.Get());
   }
}
