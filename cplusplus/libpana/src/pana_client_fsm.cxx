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
    //  State: OFFLINE (Initial State)
    //  ------------------------------
    //
    //  Initialization Action:
    //
    // FIRST_AUTH_EXCHG=Set;
    // RTX_COUNTER=0;
    // RtxTimerStop();

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PaC-initiated Handshake) - - - - - - - - -
    // AUTH_USER                Tx:PCI();                  OFFLINE
    ev.Reset();
    ev.Event_App(PANA_EV_APP_AUTH_USER);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacOfflineExitActionAuthUser);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - -(PAA-initiated Handshake, not optimized) - - - - -
    // Rx:PAR &&                PAN.S_flag=Set;            WAIT_PAA
    // PAR.S_flag &&            Tx:PAN();
    // !PAR.exist_avp           EAP_Restart();
    // ("EAP-Payload") &&       SessionTimerStart
    // (!PAR.exist_avp            (FAILED_SESS_TIMEOUT);
    // ("Algorithm") ||
    // (PAR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported()))
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagStart();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacOfflineExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - -(PAA-initiated Handshake, optimized) - - - - - -
    // Rx:PAR &&                EAP_Restart();             OFFLINE
    // PAR.S_flag &&            TxEAP();
    // PAR.exist_avp            SessionTimerStart
    // ("EAP-Payload") &&         (FAILED_SESS_TIMEOUT);
    // (!PAR.exist_avp
    // ("Algorithm") ||
    // (PAR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported()))
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagStart();
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacOfflineExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESPONSE             PAN.insert_avp             WAIT_PAA
    //                            ("EAP-Payload");
    //                          PAN.S_flag=Set;
    //                          Tx:PAN();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPAN);
    // Required to deal with eap piggyback enabled flag enabled
    // during call to EapSendResponse(..) method
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPAN);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT ||      None();                    OFFLINE
    // EAP_INVALID_MSG

    /////////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG          None();                    OFFLINE
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT         None();                    OFFLINE
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacExitActionTimeout);
    // Required to deal with eap piggyback enabled flag enabled
    // during call to ScheduleEapResponse(..) method
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(PANA-Error-Message-Processing)- - - - - - - -
    // PROTOCOL_ERROR           if (key_available())       WAIT_PNA
    //                            PNR.insert_avp("AUTH");
    //                          PNR.insert_avp
    //                            ("Result-Code");
    //                          PNR.insert_avp
    //                            ("Failed-Message-Header");
    //                          PNR.E_flag=Set;
    //                          Tx:PNR();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_ERROR);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacExitActionTxPNRError);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OFFLINE,
                               PANA_ST_OFFLINE);
#endif

    // ---------------
    // State: WAIT_PAA
    // ---------------

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(PAR-PAN exchange) - - - - - - - -
    // Rx:PAR &&                RtxTimerStop();            WAIT_EAP_MSG
    // PAR.flg_clr() &&         TxEAP();
    // !eap_piggyback()         EAP_RespTimerStart();
    //                          if (key_available())
    //                              PAN.insert_avp("AUTH");
    //                          if (FIRST_AUTH_EXCHG==Set)
    //                              PAN.insert_avp("Nonce");
    //                          FIRST_AUTH_EXCHG=Unset;
    //                          Tx:PAN();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAR &&                RtxTimerStop();            WAIT_EAP_MSG
    // PAR.flg_clr() &&         TxEAP();
    // eap_piggyback()          EAP_RespTimerStart();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAN &&                RtxTimerStop();            WAIT_PAA
    // PAN.flg_clr() &&         OPTIMIZED_PAN=Unset;
    // !PAN.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPaaExitActionRxPAN);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAN &&                RtxTimerStop();            WAIT_EAP_MSG
    // PAN.flg_clr() &&         OPTIMIZED_PAN=Set;
    // PAN.exist_avp            TxEAP();
    // ("EAP-Payload")          EAP_RespTimerStart();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAN);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(EAP result) - - - - - - - - - - -
    // Rx:PAR &&                TxEAP();                   WAIT_EAP_RESULT
    // PAR.C_flag &&
    // PAR.RESULT_CODE==
    //  PANA_SUCCESS &&
    // (!PAR.exist_avp
    // ("Algorithm") ||
    // (PAR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())) &&
    // PAR.exist_avp
    // ("EAP-Payload")
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
    // Rx:PAR &&                TxEAP()                    WAIT_EAP_RESULT_
    // PAR.C_flag &&                                       CLOSE
    // PAR.RESULT_CODE==
    //  PANA_SUCCESS &&
    // (PAR.exist_avp
    // ("Algorithm") &&
    // !algorithm_supported()) &&
    // PAR.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.AvpExist_EapPayload();
    ev.AlgorithmNotSupported();
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPARComplete);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAR &&                TxEAP();                   WAIT_EAP_RESULT_
    // PAR.C_flag &&                                       CLOSE
    // PAR.RESULT_CODE!=
    //  PANA_SUCCESS &&
    // PAR.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.ResultCode(PANA_RESULT_CODE_FAIL);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPARComplete);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAR &&                alt_reject();              WAIT_EAP_RESULT_
    // PAR.C_flag &&                                       CLOSE
    // PAR.RESULT_CODE!=
    //  PANA_SUCCESS &&
    // !PAR.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.FlagComplete();
    ev.ResultCode(PANA_RESULT_CODE_FAIL);
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
    // - - - - - - - - -(PANA-Error-Message-Processing)- - - - - - - -
    // PROTOCOL_ERROR           if (key_available())       WAIT_PNA
    //                            PNR.insert_avp("AUTH");
    //                          PNR.insert_avp
    //                            ("Result-Code");
    //                          PNR.insert_avp
    //                            ("Failed-Message-Header");
    //                          PNR.E_flag=Set;
    //                          Tx:PNR();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_ERROR);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacExitActionTxPNRError);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacExitActionTxPNAError);

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
    //                          if (FIRST_AUTH_EXCHG==Set)
    //                            PAN.insert_avp("Nonce");
    //                          FIRST_AUTH_EXCHG=Unset;
    //                          Tx:PAN();
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
    //                          Tx:PAR();
    //                          RtxTimerStart();
    /////////////////////////////////////////////////////////////////
    // EAP_RESPONSE &&          EAP_RespTimerStop()        WAIT_PAA
    // OPTIMIZED_PAN            PAR.insert_avp
    //                          ("EAP-Payload");
    //                          if (key_available())
    //                            PAR.insert_avp("AUTH");
    //                          Tx:PAR();
    //                          OPTIMIZED_PAN=Unset;
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT &&      if (key_available())       WAIT_PAA
    // eap_piggyback()            PAN.insert_avp("AUTH");
    //                          Tx:PAN();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgExitActionTxPANTout);

    /////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT &&      None()                     WAIT_EAP_MSG
    // !eap_piggyback()
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG ||       None();                    WAIT_PAA
    // EAP_SUCCESS ||
    // EAP_FAILURE
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAA);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacExitActionTxPNAError);

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
    // EAP_SUCCESS &&          PAN.insert_avp("AUTH");     OPEN
    // PAR.exist_avp           PAN.insert_avp("Key-Id");
    // ("Key-Id")              PAN.C_flag=Set;
    //                         Tx:PAN();
    //                         Authorize();
    //                         SessionTimerStop();
    //                         SessionTimerStart
    //                           (LIFETIME_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        OPEN
    // !PAR.exist_avp            PAN.insert_avp("AUTH");
    // ("Key-Id")              PAN.C_flag=Set;
    //                         Tx:PAN();
    //                         Authorize();
    //                         SessionTimerStop();
    //                         SessionTimerStart
    //                           (LIFETIME_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             PAN.C_flag=Set;             CLOSED
    //                         Tx:PAN();
    //                         SessionTimerStop();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG         None();                     WAIT_PAA
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_WAIT_PAA);

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
    // EAP_SUCCESS &&          PAN.insert_avp("AUTH");     CLOSED
    // PAR.exist_avp           PAN.insert_avp("Key-Id");
    // ("Key-Id")              PAN.C_flag=Set;
    //                         Tx:PAR();
    //                         SessionTimerStop();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapSuccessExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        CLOSED
    // !PAR.exist_avp            PAN.insert_avp("AUTH");
    // ("Key-Id")              PAN.C_flag=Set;
    //                         Tx:PAN();
    //                         SessionTimerStop();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapSuccessExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             PAN.C_flag=Set;             CLOSED
    //                         Tx:PAN();
    //                         SessionTimerStop();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG         None();                     WAIT_PAA
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_WAIT_PAA);

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
    // Rx:PNR &&                if (key_available())       OPEN
    // PNR.P_flag                 PNA.insert_avp("AUTH");
    //                          PNA.P_flag=Set;
    //                          Tx:PNA();
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
    //                            PNR.insert_avp("AUTH");
    //                          PNR.P_flag=Set;
    //                          Tx:PNR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacOpenExitActionTxPNRPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PaC)- - - - - -
    // REAUTH                   if (key_available())       WAIT_PNA
    //                            PNR.insert_avp("AUTH");
    //                          FIRST_AUTH_EXCHG=Set;
    //                          PNR.A_flag=Set;
    //                          Tx:PNR();
    //                          RtxTimerStart();
    //                          SessionTimerStop();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacOpenExitActionTxPNRAuth);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // PAR.flg_clr() &&         TxEAP();
    // !eap_piggyback()         FIRST_AUTH_EXCHG=Set;
    //                          if (key_available())
    //                             PAN.insert_avp("AUTH");
    //                          Tx:PAN();
    //                          SessionTimerStop();
    //                          SessionTimerStart
    //                            (FAILED_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacOpenExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // PAR.flg_clr() &&         TxEAP();
    // eap_piggyback()          FIRST_AUTH_EXCHG=Set;
    //                          SessionTimerStop();
    //                          SessionTimerStart
    //                            (FAILED_SESS_TIMEOUT);
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacOpenExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Session termination initiated by PAA) - - - - - -
    // Rx:PTR                   if (key_available())       CLOSED
    //                            PTA.insert_avp("AUTH");
    //                          Tx:PTA();
    //                          SessionTimerStop();
    //                          Disconnect();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacOpenExitActionRxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Session termination initiated by PaC) - - - - - -
    // TERMINATE                if (key_available())       SESS_TERM
    //                            PTR.insert_avp("AUTH");
    //                          Tx:PTR();
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
    // - - - - - - - - -(PANA-Error-Message-Processing)- - - - - - - -
    // PROTOCOL_ERROR           if (key_available())       WAIT_PNA
    //                            PNR.insert_avp("AUTH");
    //                          PNR.insert_avp
    //                            ("Result-Code");
    //                          PNR.insert_avp
    //                            ("Failed-Message-Header");
    //                          PNR.E_flag=Set;
    //                          Tx:PNR();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_ERROR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacExitActionTxPNRError);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacExitActionTxPNAError);

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
    // Rx:PNA &&                RtxTimerStop();            WAIT_PAA
    // PNR.A_flag               SessionTimerStart
    //                            (FAILED_SESS_TIMEOUT);
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagAuth();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPNAExitActionRxPNAAuth);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - -(liveness test initiated by PaC) - - - - - - -
    // Rx:PNA &&                RtxTimerStop();            OPEN
    // PNR.P_flag
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagPing();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitPNAExitActionRxPNAPing);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(error processing) - - - - - - - - - - -
    // Rx:PNA &&                RtxTimerStop();            CLOSED
    // PNA.E_flag               SessionTimerStop();
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNA);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitPNAExitActionRxPNAError);

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

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_WAIT_PNA, ev.Get(),
                       PANA_ST_WAIT_PNA,
                       m_PacExitActionTxPNAError);

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
    // Rx:PTA                   Disconnect();              CLOSED
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

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PNR &&                PNA.insert_avp("AUTH");    CLOSED
    // PNR.E_flag &&            PNA.E_flag=Set;
    // fatal                    Tx:PNA();
    // (PNR.RESULT_CODE) &&     Disconnect();
    // PNR.exist_avp("AUTH") &&
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPNAError);

    /////////////////////////////////////////////////////////////////////
    // Rx:PNR &&                PNA.E_flag=Set;            (no change)
    // PNR.E_flag &&            Tx:PNA();
    // !fatal
    // (PNR.RESULT_CODE) ||
    // !PNR.exist_avp("AUTH") ||
    // !key_available()
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PNR);
    ev.FlagError();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PacExitActionTxPNAError);

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

    InitialState(PANA_ST_OFFLINE);
}

class PANA_CsmRxPA : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPA(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_OFFLINE,
                                           PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PacEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PAR);

          if (msg.flags().auth ||
              msg.flags().ping ||
              msg.flags().error) {
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
          ev.FlagStart();

          // verify algorithm if present and supported
          PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
          pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
          if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
              // - - - - - - - - - - (un-supported Algorithm) - - - - - - - - -
              // Rx:PAR &&                None();                    OFFLINE
              // PAR.S_flag &&
              // (PAR.exist_avp
              // ("Algorithm") &&
              // !algorithm_supported())
              // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

          // save address of paa
          if (msg.flags().request) {
              m_arg.PaaAddress() = msg.srcAddress();
          }
          m_arg.LastUsedChannel() = msg.destAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PAR);
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
          static PANA_ST validStates[] = { PANA_ST_OFFLINE,
                                           PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN,
                                           PANA_ST_WAIT_PNA,
                                           PANA_ST_SESS_TERM };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {

          PANA_PacEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PNR);

          // first level validation
          m_arg.RxValidateMsg(msg);

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
          else if (msg.flags().error) {
              HandleError(msg, ev);
          }

          // post the event
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPN(*this));
      }

   protected:
      virtual void HandleAuth(PANA_Message &msg,
                              PANA_PacEventVariable &ev) {
          // second level validation
          if (msg.flags().request) {
              throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                     "PN received, invalid message"));
          }

          // save last received header
          m_arg.LastRxHeader() = msg;

          // save last used address
          m_arg.LastUsedChannel() = msg.destAddress();

          // resolve event
          ev.FlagAuth();
      }
      virtual void HandlePing(PANA_Message &msg,
                              PANA_PacEventVariable &ev) {
          // save address of Paa
          if (msg.flags().request) {
              m_arg.PaaAddress() = msg.srcAddress();
          }
          m_arg.LastUsedChannel() = msg.destAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          ev.FlagPing();
      }
      virtual void HandleError(PANA_Message &msg,
                               PANA_PacEventVariable &ev) {
          // save address of Paa
          if (msg.flags().request) {
              m_arg.PaaAddress() = msg.srcAddress();
          }
          m_arg.LastUsedChannel() = msg.destAddress();

          // resolve the event
          ev.FlagError();
          if (msg.flags().request) {
              if (m_arg.IsFatalError()) {
                  PANA_StringAvpContainerWidget authAvp(msg.avpList());
                  pana_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
                  if (auth && m_arg.SecurityAssociation().Auth().IsSet()) {
                      ev.Do_FatalError();
                  }
              }
              // tell the session
              m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
              m_arg.RxPNRError();
          }
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
   if (PANA_CFG_PAC().m_EapPiggyback &&
       ! m_PaC.AuxVariables().OptimizedPAN()) {
       ev.EnableCfg_EapPiggyback();
   }
   m_PaC.AuxVariables().OptimizedPAN() = false;
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

void PANA_PacSession::Error(pana_unsigned32_t error)
{
   PANA_PacEventVariable ev;
   m_PaC.LastProtocolError() = error;
   ev.Event_App(PANA_EV_APP_ERROR);
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
