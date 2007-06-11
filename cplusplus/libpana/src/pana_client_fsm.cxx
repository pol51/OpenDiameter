/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open PANA_: Open-source software for the PANA_ and                     */
/*                PANA_ related protocols                                 */
/*                                                                        */
/* Copyright (C) 2002-2007 Open PANA_ Project                             */
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

#include "pana_client_fsm.h"

PANA_ClientStateTable PANA_PacSession::m_StateTable;
typedef PANA_SessionRxStateFilter<PANA_Client, PANA_PacSession>
            PANA_ClientRxStateFilter;

PANA_ClientStateTable::PANA_ClientStateTable()
{
    PANA_PacEventVariable ev;

    //  ------------------------------
    //  State: INITIAL (Initial State)
    //  ------------------------------
    //
    //  Initialization Action:
    //
    // NONCE_SENT=Unset;
    // RTX_COUNTER=0;
    // RtxTimerStop();

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PaC-initiated Handshake) - - - - - - - - -
    // AUTH_USER                Tx:PCI();                  INITIAL
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_App(PANA_EV_APP_AUTH_USER);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PacOfflineExitActionAuthUser);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - -(PAA-initiated Handshake, not optimized) - - - - -
    // Rx:PAR[S] &&             Tx:PAN[S]();               WAIT_PAA
    // !PAR.exist_avp           EAP_Restart();
    // ("EAP-Payload")          SessionTimerReStart
    //                           (FAILED_SESS_TIMEOUT);
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagStart();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacOfflineExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - -(PAA-initiated Handshake, optimized) - - - - - -
    // Rx:PAR[S] &&             EAP_Restart();             INITIAL
    // PAR.exist_avp            TxEAP();
    // ("EAP-Payload") &&       SessionTimerReStart
    // eap_piggyback()            (FAILED_SESS_TIMEOUT);
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagStart();
    ev.AvpExist_EapPayload();
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PacOfflineExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - -(PAA-initiated Handshake, optimized) - - - - - -
    // Rx:PAR[S] &&             EAP_Restart();             WAIT_EAP_MSG
    // PAR.exist_avp            TxEAP();
    // ("EAP-Payload") &&       SessionTimerReStart
    // !eap_piggyback()            (FAILED_SESS_TIMEOUT);
    //                          TxPAN[S]();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagStart();
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacOfflineExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESPONSE             PAN.insert_avp             WAIT_PAA
    //                            ("EAP-Payload");
    //                          Tx:PAN[S]();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPAN);
    // Required to deal with eap piggyback flag enabled
    // during call to EapSendResponse(..) method
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPAN);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT         None();                    INITIAL
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PacExitActionTimeout);
    // Required to deal with eap piggyback flag enabled
    // during call to ScheduleEapResponse(..) method
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_INITIAL,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_INITIAL, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_INITIAL,
                               PANA_ST_INITIAL);
#endif

    // ---------------
    // State: WAIT_PAA
    // ---------------

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(PAR-PAN exchange) - - - - - - - -
    // Rx:PAR[] &&              RtxTimerStop();            WAIT_EAP_MSG
    // !eap_piggyback()         TxEAP();
    //                          EAP_RespTimerStart();
    //                          if (key_available())
    //                              PAN.insert_avp("AUTH");
    //                          if (NONCE_SENT==Unset) {
    //                              PAN.insert_avp("Nonce");
    //                              NONCE_SET=Set;
    //                          }
    //                          Tx:PAN[]();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAR[] &&              RtxTimerStop();            WAIT_EAP_MSG
    // eap_piggyback()          TxEAP();
    //                          EAP_RespTimerStart();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAN[]                 RtxTimerStop();            WAIT_PAA
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPaaExitActionRxPAN);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(EAP result) - - - - - - - - - - -
    // Rx:PAR[C] &&             TxEAP();                   WAIT_EAP_RESULT
    // PAR.RESULT_CODE==
    //  PANA_SUCCESS
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.AvpExist_EapPayload();
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT,
                       m_PacWaitPaaExitActionRxPARComplete);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAR[C] &&             if (PAR.exist_avp          WAIT_EAP_RESULT_
    // PAR.RESULT_CODE!=           ("EAP-Payload"))        CLOSE
    //  PANA_SUCCESS              TxEAP();
    //                          else
    //                            alt_reject();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.ResultCode(PANA_RCODE_AUTHORIZATION_REJECTED);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPARComplete);
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.ResultCode(PANA_RCODE_AUTHENTICATION_REJECTED);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPARComplete);

    /////////////////////////////////////////////////////////////////
    // - - - - - - (Reach maximum number of retransmission)- - - - -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PAA,
                               PANA_ST_WAIT_PAA);
#endif

    // -------------------
    // State: WAIT_EAP_MSG
    // -------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Return PAN/PAR) - - - - - - - - - - - - - -
    // EAP_RESPONSE &&          EAP_RespTimerStop()        WAIT_PAA
    // eap_piggyback()          PAN.insert_avp
    //                          ("EAP-Payload");
    //                          if (key_available())
    //                            PAN.insert_avp("AUTH");
    //                          if (NONCE_SENT==Unset) {
    //                            PAN.insert_avp("Nonce");
    //                            NONCE_SENT=Unset;
    //                          }
    //                          Tx:PAN[]();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgExitActionTxPAN);

    /////////////////////////////////////////////////////////////////
    // EAP_RESPONSE &&          EAP_RespTimerStop()        WAIT_PAA
    // !eap_piggyback()         PAR.insert_avp
    //                          ("EAP-Payload");
    //                          if (key_available())
    //                            PAR.insert_avp("AUTH");
    //                          Tx:PAR[]();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT &&      if (key_available())       WAIT_PAA
    // eap_piggyback()            PAN.insert_avp("AUTH");
    //                          Tx:PAN[]();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgExitActionTxPANTout);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE ||           SessionTimerStop();        CLOSED
    // EAP_INVALID_MSG          Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_CLOSED,
                       m_PacExitActionTimeout);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG,
                               PANA_ST_WAIT_EAP_MSG);
#endif

    // ----------------------
    // State: WAIT_EAP_RESULT
    // ----------------------

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS             if (PAR.exist_avp           OPEN
    //                            ("Key-Id"))
    //                            PAN.insert_avp("Key-Id");
    //                        Tx:PAN[C]();
    //                        Authorize();
    //                        SessionTimerReStart
    //                          (LIFETIME_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             Tx:PAN[C]();                CLOSED
    //                         SessionTimerStop();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_RESULT,
                               PANA_ST_WAIT_EAP_RESULT);
#endif

    // ----------------------------
    // State: WAIT_EAP_RESULT_CLOSE
    // ----------------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (EAP Result) - - - - - - - - - - - - -
    // EAP_SUCCESS ||          if (key_available())        CLOSED
    // EAP_FAILURE                PAN.insert_avp("AUTH");
    //                        if (EAP_SUCCESS &&
    //                            PAR.exist_avp("Key-Id"))
    //                           PAN.insert_avp("Key-Id");
    //                        Tx:PAN[C]()
    //                        SessionTimerStop();
    //                        Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapSuccessExitActionClose);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE,
                               PANA_ST_WAIT_EAP_RESULT_CLOSE);
#endif

    // -----------
    // State: OPEN
    // -----------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (liveness test initiated by PAA)- - - - - -
    // Rx:PNR[P]                if (key_available())       OPEN
    //                           PNA.insert_avp("AUTH");
    //                         Tx:PNA[P]();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagPing();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacOpenExitActionRxPNRPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (liveness test initiated by PaC)- - - - - -
    // PANA_PING                if (key_available())       WAIT_PNA
    //                           PNR.insert_avp("AUTH");
    //                          Tx:PNR[P]();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacOpenExitActionTxPNRPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PaC)- - - - - -
    // REAUTH                   if (key_available())       WAIT_PNA
    //                           PNR.insert_avp("AUTH");
    //                         NONCE_SENT=Unset;
    //                         Tx:PNR[A]();
    //                         RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacOpenExitActionTxPNRAuth);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // Rx:PAR[]                EAP_RespTimerStart();      WAIT_EAP_MSG
    //                         TxEAP();
    //                         if (key_available())
    //                            PAN.insert_avp("AUTH");
    //                         if (!eap_piggyback()) {
    //                             PAN.insert_avp("Nonce");
    //                             Tx:PAN[]();
    //                         }
    //                         else {
    //                            NONCE_SENT=Unset;
    //                         }
    //                         SessionTimerReStart
    //                            (FAILED_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacOpenExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Session termination initiated by PAA) - - - - - -
    // Rx:PTR[]                 if (key_available())       CLOSED
    //                            PTA.insert_avp("AUTH");
    //                          Tx:PTA[]();
    //                          SessionTimerStop();
    //                          Disconnect();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacOpenExitActionRxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Session termination initiated by PaC) - - - - - -
    // TERMINATE                if (key_available())       SESS_TERM
    //                            PTR.insert_avp("AUTH");
    //                          Tx:PTR[]();
    //                          RtxTimerStart();
    //                          SessionTimerStop();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PacOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OPEN,
                               PANA_ST_OPEN);
#endif

    // ----------------
    // State: WAIT_PNA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(re-authentication initiated by PaC) - - - - -
    // Rx:PNA[A]                RtxTimerStop();            WAIT_PAA
    //                          SessionTimerReStart
    //                            (FAILED_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagAuth();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPNAExitActionRxPNAAuth);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(liveness test initiated by PaC) - - - - - - -
    // Rx:PNA[P]                RtxTimerStop();            OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagPing();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitPNAExitActionRxPNAPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

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
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PNA,
                               PANA_ST_WAIT_PNA);
#endif

    // ----------------
    // State: SESS_TERM
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Session termination initiated by PaC) - - - - -
    // Rx:PTA[]                   Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTA);
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacSessTermExitActionRxPTA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Reach maximum number of retransmission)- - -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
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

class PANA_CsmRxPA : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPA(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_INITIAL,
                                           PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PacEventVariable ev;

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
      }
      virtual PANA_ClientRxStateFilter *clone() {
          return (new PANA_CsmRxPA(*this));
      }

   protected:
      virtual void HandleStart(PANA_Message &msg,
                               PANA_PacEventVariable &ev) {
          // first level validation
          if (! msg.flags().request) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                  "PAN recevied, invalid message"));
          }

          // consume the session identifier
          m_arg.SessionId() = msg.sessionId();

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save address of Paa
          m_arg.PaaAddress() = msg.srcAddress();
          m_arg.LastUsedChannel() = msg.destAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.MsgType(PANA_EV_MTYPE_PAR);
          ev.FlagStart();

          // check if piggyback is supported
          ev.EnableCfg_EapPiggyback(PANA_CFG_PAC().m_EapPiggyback ? 1 : 0);

          // verify algorithm if present and supported
          PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
          pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
          if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
              AAA_LOG((LM_INFO, "(%P|%t) Supplied algorithm [0x%x] is not supported, session will close\n",
                      *algo));
              throw (PANA_Exception(PANA_Exception::FAILED,
                      "PAR recevied, unsupported algorithm"));
          }

          PANA_StringAvpContainerWidget eapAvp(msg.avpList());
          if (eapAvp.GetAvp(PANA_AVPNAME_EAP)) {
              ev.AvpExist_EapPayload();
          }
      }
      virtual void HandleNormal(PANA_Message &msg,
                               PANA_PacEventVariable &ev) {
          // first level validation
          m_arg.RxValidateMsg(msg);

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PAR);
              m_arg.PaaAddress() = msg.srcAddress();
              ev.EnableCfg_EapPiggyback(PANA_CFG_PAC().m_EapPiggyback ? 1 : 0);
          }
          else {
              ev.MsgType(PANA_EV_MTYPE_PAN);
              PANA_StringAvpContainerWidget eapAvp(msg.avpList());
              pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
              if (payload) {
                  ev.AvpExist_EapPayload();
              }
          }
          m_arg.LastUsedChannel() = msg.destAddress();
      }
      virtual void HandleComplete(PANA_Message &msg,
                                  PANA_PacEventVariable &ev) {
          // first level validation
          if (! msg.flags().request) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PNA received, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg, true);

          // save address of Paa
          m_arg.PaaAddress() = msg.srcAddress();
          m_arg.LastUsedChannel() = msg.destAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.MsgType(PANA_EV_MTYPE_PAR);
          ev.FlagComplete();

          // resolve the eap event
          PANA_StringAvpContainerWidget eapAvp(msg.avpList());
          pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
          if (payload) {
              ev.AvpExist_EapPayload();
          }
          PANA_UInt32AvpContainerWidget rcodeAvp(msg.avpList());
          pana_unsigned32_t *rcode = rcodeAvp.GetAvp(PANA_AVPNAME_RESULTCODE);
          if (rcode && (ACE_NTOHL(*rcode) == PANA_RCODE_SUCCESS)) {
              // third level validation
              PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
              pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
              if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
                  ev.AlgorithmNotSupported();
              }
              ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
          }
          else {
              ev.ResultCode(PANA_RESULT_CODE_FAIL);
          }
      }
};

class PANA_CsmRxPN : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPN(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_PNA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PacEventVariable ev;

          // first level validation
          m_arg.RxValidateMsg(msg);

          if (msg.flags().start || msg.flags().complete) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PN recevied, invalid flag settings"));
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
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPN(*this));
      }

   protected:
      virtual void HandlePing(PANA_Message &msg,
                              PANA_PacEventVariable &ev) {
          // save address of Paa
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PNR);
              m_arg.PaaAddress() = msg.srcAddress();
          }
          else {
              ev.MsgType(PANA_EV_MTYPE_PNA);
          }
          m_arg.LastUsedChannel() = msg.destAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.FlagPing();
      }
};

class PANA_CsmRxPT : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPT(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_SESS_TERM };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         if (msg.flags().request) {
             m_arg.PaaAddress() = msg.srcAddress();
         }
         m_arg.LastUsedChannel() = msg.destAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(msg.flags().request ?
                    PANA_EV_MTYPE_PTR : PANA_EV_MTYPE_PTA);
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPT(*this));
      }
};

PANA_PacSession::PANA_PacSession(PANA_Node &n,
                                 PANA_ClientEventInterface &eif) :
   PANA_StateMachine<PANA_Client, PANA_Channel>
            (m_PaC, m_StateTable, n, m_PacChannel),
   m_Node(n),
   m_PanaChannel(n.Job(), "Pac Well known UDP Channel"),
   m_PacChannel(n.Job(), "Pac Specific UDP Channel"),
   m_Timer(*this),
   m_PaC(m_TxChannel, m_Timer, eif)
{
   OD_Utl_SCSIAdapter1<PANA_PacSession,
                   void(PANA_PacSession::*)(PANA_Message&),
                   PANA_Message&>
               msgHandler(*this, &PANA_PacSession::Receive);

   ACE_INET_Addr addr;
   char strAddr[64];

   // Setup the known PAA address
   sprintf(strAddr, "%s:%d", PANA_CFG_PAC().m_PaaIpAddress.data(),
           PANA_CFG_PAC().m_PaaPortNumber);
   addr.string_to_addr(strAddr);
   m_PaC.PaaAddress() = addr;

   // Listen to a well known port
   addr.set((u_short)PANA_CFG_GENERAL().m_ListenPort);
   sprintf(strAddr, "%d", PANA_CFG_GENERAL().m_ListenPort);
   addr.string_to_addr(strAddr);
   m_PanaChannel.Open(addr);
   m_PanaChannel.RegisterHandler(msgHandler);

   // Listen to a PaC specific port - some pseudo random value
   m_PaC.PacAddress().set((u_short)(PANA_CFG_GENERAL().m_ListenPort + ((int)this / 1000)));
   m_PacChannel.Open(m_PaC.PacAddress());
   m_PacChannel.RegisterHandler(msgHandler);

   InitializeMsgMaps();
   PANA_StateMachine<PANA_Client, PANA_Channel>::Start();
}

PANA_PacSession::~PANA_PacSession()
{
   PANA_StateMachine<PANA_Client, PANA_Channel>::Stop();
   FlushMsgMaps();
   m_PanaChannel.Close();
   m_PanaChannel.RemoveHandler();
   m_PacChannel.Close();
   m_PacChannel.RemoveHandler();
}

void PANA_PacSession::InitializeMsgMaps()
{
   PANA_CsmRxPA PA(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PAR, PA);

   PANA_CsmRxPN PN(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PNR, PN);

   PANA_CsmRxPT PT(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PTR, PT);
}

void PANA_PacSession::FlushMsgMaps()
{
   m_MsgHandlers.Remove(PANA_MTYPE_PAR);
   m_MsgHandlers.Remove(PANA_MTYPE_PNR);
   m_MsgHandlers.Remove(PANA_MTYPE_PTR);
}

void PANA_PacSession::Start() throw (AAA_Error)
{
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_AUTH_USER);
   Notify(ev.Get());
}

void PANA_PacSession::EapSendResponse(AAAMessageBlock *response)
{
   PANA_MsgBlockGuard guard(response, true);
   m_PaC.AuxVariables().TxEapMessageQueue().Enqueue(guard.Release());
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_RESPONSE);
   if (PANA_CFG_PAC().m_EapPiggyback) {
       ev.EnableCfg_EapPiggyback();
   }
   Notify(ev.Get());
}

void PANA_PacSession::EapInvalidMessage()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
   Notify(ev.Get());
}

void PANA_PacSession::EapSuccess()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_SUCCESS);
   if (m_PaC.SecurityAssociation().MSK().Id() > 0) {
       ev.AvpExist_KeyId();
   }
   Notify(ev.Get());
}

void PANA_PacSession::EapTimeout()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
   Notify(ev.Get());
}

void PANA_PacSession::EapFailure()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_FAILURE);
   Notify(ev.Get());
}

void PANA_PacSession::ReAuthenticate()
{
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_REAUTH);
   Notify(ev.Get());
}

void PANA_PacSession::Ping()
{
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_PING);
   Notify(ev.Get());
}

void PANA_PacSession::Stop()
{
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_TERMINATE);
   Notify(ev.Get());
}

void PANA_ClientStateTable::PacExitActionRetransmission::operator ( )(PANA_Client &c)
{
    if (! c.TxLastReqMsg()) {
       PANA_PacEventVariable ev;
       ev.Do_RetryTimeout();
       FsmTimer<PANA_PacEventVariable, PANA_PacSession> &tm =
                static_cast< FsmTimer<PANA_PacEventVariable, PANA_PacSession>& >
                          (c.Timer());
       tm.Fms().Notify(ev.Get());
    }
}
