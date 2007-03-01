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
    //  - - - -(PAA-initiated Handshake, not optimized) - - - - - - - - -
    // Rx:PSR &&                Tx:PSA();                  WAIT_PAA
    // !PSR.exist_avp           EAP_Restart();
    // ("EAP-Payload") &&
    // (!PSR.exist_avp
    // ("Algorithm") ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported()))
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - -(PAA-initiated Handshake, optimized) - - - - - -
    // Rx:PSR &&                EAP_Restart();             OFFLINE
    // PSR.exist_avp            TxEAP();                   
    // ("EAP-Payload") &&
    // (!PSR.exist_avp
    // ("Algorithm") ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported()))
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////////
    // EAP_RESPONSE             PSA.insert_avp             WAIT_PAA
    //                            ("EAP-Payload")
    //                          Tx:PSA();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPSA);
    // Required to deal with eap piggyback enabled flag enabled
    // during call to EapSendResponse(..) method
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPSA);
                       
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

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PANA-Error-Message-Processing)- -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatalRx
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacExitActionTxPEA);

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
    // !eap_piggyback()         TxEAP();
    //                          EAP_RespTimerStart();
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
    // eap_piggyback()          TxEAP();
    //                          EAP_RespTimerStart();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PAN                   RtxTimerStop();            WAIT_PAA
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPaaExitActionRxPAN);

    /////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(EAP result) - - - - - - - - - - -
    // Rx:PBR &&                TxEAP();                   WAIT_EAP_RESULT
    // PBR.RESULT_CODE==
    //   PANA_SUCCESS &&
    // (!PBR.exist_avp
    // ("Algorithm") ||
    // (PBR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())) &&
    // PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.AvpExist_EapPayload();
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT,
                       m_PacWaitPaaExitActionRxPBR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PBR &&                TxEAP()                    WAIT_EAP_RESULT_
    // PBR.RESULT_CODE==                                   CLOSE
    //   PANA_SUCCESS &&
    // (PBR.exist_avp
    // ("Algorithm") &&
    // !algorithm_supported()) &&
    // PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.AvpExist_EapPayload();
    ev.AlgorithmNotSupported();
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPBR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PBR &&                TxEAP();                   WAIT_EAP_RESULT_
    // PBR.RESULT_CODE!=                                   CLOSE
    //   PANA_SUCCESS &&
    // PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.ResultCode(PANA_RESULT_CODE_FAIL);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPBR);

    /////////////////////////////////////////////////////////////////////
    // Rx:PBR &&                alt_reject();              WAIT_EAP_RESULT_
    // PBR.RESULT_CODE!=                                   CLOSE
    //   PANA_SUCCESS &&
    // !PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.ResultCode(PANA_RESULT_CODE_FAIL);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPBR);

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
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacExitActionTxPEA);

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
    // EAP_INVALID_MSG ||       Tx:PER();                  WAIT_PEA
    // EAP_SUCCESS ||           RtxTimerStart();
    // EAP_FAILURE
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PEA,
                       m_PacExitActionTxPER);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PEA,
                       m_PacExitActionTxPER);
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PEA,
                       m_PacExitActionTxPER);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacExitActionTxPEA);

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
    // EAP_SUCCESS &&          PBA.insert_avp("AUTH");     OPEN
    // PBR.exist_avp           PBA.insert_avp("Key-Id");
    // ("Key-Id")              Tx:PBA();
    //                         Authorize();
    //                         SessionTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        OPEN
    // !PBR.exist_avp            PBA.insert_avp("AUTH");
    // ("Key-Id")              Tx:PBA();
    //                         Authorize();
    //                         SessionTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             Tx:PBA();                   CLOSED
    //                         Disconnect();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG         None();                     WAIT_PAA
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
    // EAP_SUCCESS &&          PBA.insert_avp("AUTH");     CLOSED
    // PBR.exist_avp           PBA.insert_avp("Key-Id");
    // ("Key-Id")              Tx:PBA();
    //                         Disconnect();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapSuccessExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        CLOSED
    // !PBR.exist_avp            PBA.insert_avp("AUTH");
    // ("Key-Id")              Tx:PBA();
    //                         Disconnect();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapSuccessExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             Tx:PBA();                   CLOSED
    //                         Disconnect();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitEapFailExitActionClose);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG         None();                     WAIT_PAA
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
    // Rx:PPR                   if (key_available())       OPEN
    //                            PPA.insert_avp("AUTH");
    //                          Tx:PPA();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacOpenExitActionRxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (liveness test initiated by PaC)- - - - - -
    // PANA_PING                if (key_available())       WAIT_PPA
    //                            PPR.insert_avp("AUTH");
    //                          Tx:PPR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PPA,
                       m_PacOpenExitActionTxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PaC)- - - - - -
    // REAUTH                   if (key_available())       WAIT_PRA
    //                            PRR.insert_avp("AUTH");
    //                          FIRST_AUTH_EXCHG=Set;
    //                          Tx:PRR();
    //                          RtxTimerStart();
    //                          SessionTimerStop();
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PRA,
                       m_PacOpenExitActionTxPRR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // !eap_piggyback()         TxEAP();
    //                          FIRST_AUTH_EXCHG=Set;
    //                          if (key_available())
    //                             PAN.insert_avp("AUTH");
    //                          Tx:PAN();
    //                          SessionTimerStop();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacOpenExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // eap_piggyback()          TxEAP();
    //                          FIRST_AUTH_EXCHG=Set;
    //                          SessionTimerStop();
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
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PacOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - (PaC updates PAA with new IP or attributes)- - - - -
    // UPDATE                   if (key_available())       WAIT_PUA
    //                            PUR.insert_avp("AUTH");
    //                          Tx:PUR();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_App(PANA_EV_APP_UPDATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PUA,
                       m_PacOpenExitActionTxPUR);

    /////////////////////////////////////////////////////////////////
    //  - - - - - - - - - - - (Attribute update)  - - - - - - - - - - -
    // Rx:PUR                   If (key_avaialble())       OPEN
    //                            PUA.insert_avp("AUTH");
    //                          Tx:PUA();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacOpenExitActionRxPUR);

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
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OPEN,
                               PANA_ST_OPEN);
#endif

    // ----------------
    // State: WAIT_PRA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(re-authentication initiated by PaC) - - - - -
    // Rx:PRA                  RtxTimerStop();            WAIT_PAA
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PRA);
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPRAAExitActionRxPRA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
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
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
                       PANA_ST_WAIT_PRA,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PRA, ev.Get(),
                       PANA_ST_WAIT_PRA,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PRA,
                               PANA_ST_WAIT_PRA);
#endif

    // ----------------
    // State: WAIT_PPA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(liveness test initiated by PAA) - - - - - - -
    // Rx:PPA                   RtxTimerStop();            OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPA);
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitPPAExitActionRxPPA);

    /////////////////////////////////////////////////////////////////
    // SESS_TIMEOUT             Disconnect();              CLOSED
    //
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
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
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_WAIT_PPA,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_WAIT_PPA,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PPA,
                               PANA_ST_WAIT_PPA);
#endif

    // ----------------
    // State: WAIT_PUA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (PUA processing)- - - - - - - - - - -
    // Rx:PUA                   RtxTimerStop();            OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUA);
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PacWaitPUAExitActionRxPUA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
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
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_WAIT_PUA,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_WAIT_PUA,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PUA,
                               PANA_ST_WAIT_PUA);
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

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PANA-Error-Message-Processing)- - - - - - -
    // Rx:PER &&                PEA.insert_avp("AUTH");     CLOSED
    // fatal                    Tx:PEA();
    // (PER.RESULT_CODE) &&     Disconnect();
    // PER.exist_avp("AUTH") &&
    // key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    ev.Do_FatalError(); // fatal(PER.RESULT_CODE) &&
                        // PER.exist_avp("AUTH") &&
                        // key_available()
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_SESS_TERM,
                               PANA_ST_SESS_TERM);
#endif

    // ----------------
    // State: WAIT_PEA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PEA processing) - - - - - - - - - -
    // Rx:PEA                   RtxTimerStop();            CLOSED
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PEA);
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacWaitPEAExitActionRxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PacExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
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
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
                       PANA_ST_WAIT_PEA,
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PEA,
                               PANA_ST_WAIT_PEA);
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

class PANA_CsmRxPSR : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPSR(PANA_Client &c,
                    PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
         static PANA_ST validStates[] = { PANA_ST_OFFLINE };
         AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         if (! msg.flags().request) {
            throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                   "PSA recevied, invalid message"));
         }

         // consume the session identifier
         m_arg.SessionId() = msg.sessionId();

         // second level validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(PANA_EV_MTYPE_PSR);

         // verify algorithm if present and supported
         PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
         pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
         if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
             // - - - - - - - - - - (un-supported Algorithm) - - - - - - - - -
             // Rx:PSR &&                None();                    OFFLINE
             // (PSR.exist_avp
             // ("Algorithm") &&
             // !algorithm_supported())
             // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
             AAA_LOG((LM_INFO, "(%P|%t) Supplied algorithm [0x%x] is not supported, session will close\n",
                     *algo));
             throw (PANA_Exception(PANA_Exception::FAILED,
                    "PSA recevied, unsupported algorithm"));
         }

         PANA_StringAvpContainerWidget eapAvp(msg.avpList());
         if (eapAvp.GetAvp(PANA_AVPNAME_EAP)) {
             ev.AvpExist_EapPayload();
         }

         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      }
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPSR(*this));
      }
};

class PANA_CsmRxPA : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPA(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         if (msg.flags().request) {
            ev.MsgType(PANA_EV_MTYPE_PAR);
            ev.EnableCfg_EapPiggyback(PANA_CFG_PAC().m_EapPiggyback ? 1 : 0);
         }
         else {
            ev.MsgType(PANA_EV_MTYPE_PAN);
         }
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      }
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPA(*this));
      }
};

class PANA_CsmRxPBR : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPBR(PANA_Client &c, PANA_PacSession &s) :
          PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PAA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         if (! msg.flags().request) {
            throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                   "PBA received, invalid message"));
         }

         // second level validation
         m_arg.RxValidateMsg(msg, true);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(PANA_EV_MTYPE_PBR);

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
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPBR(*this));
      }
};

class PANA_CsmRxPP : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPP(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PPA,
                                           PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(msg.flags().request ?
                    PANA_EV_MTYPE_PPR : PANA_EV_MTYPE_PPA);
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() { 
         return (new PANA_CsmRxPP(*this)); 
      }
};

class PANA_CsmRxPU : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPU(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_PUA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // msg validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(msg.flags().request ?
                    PANA_EV_MTYPE_PUR : PANA_EV_MTYPE_PUA);
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPU(*this));
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
         m_arg.PaaAddress() = msg.srcAddress();

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

class PANA_CsmRxPRA : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPRA(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PRA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         if (msg.flags().request) {
            throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                   "PRR received, invalid message"));
         }

         // second level validation
         m_arg.RxValidateMsg(msg);

         // save address of Paa
         m_arg.PaaAddress() = msg.srcAddress();

         // save last received header
         m_arg.LastRxHeader() = msg;

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(PANA_EV_MTYPE_PRA);
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPRA(*this));
      }
};

class PANA_CsmRxPE : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPE(PANA_Client &c, PANA_PacSession &s) :
         PANA_ClientRxStateFilter(c, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN,
                                           PANA_ST_WAIT_EAP_MSG,
                                           PANA_ST_WAIT_PRA,
                                           PANA_ST_WAIT_PPA,
                                           PANA_ST_SESS_TERM,
                                           PANA_ST_WAIT_PUA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);

          // save address of Paa
          m_arg.PaaAddress() = msg.srcAddress();

          // resolve the event
          PANA_PacEventVariable ev;
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PER);
              if (m_arg.IsFatalError()) {
                  PANA_StringAvpContainerWidget authAvp(msg.avpList());
                  pana_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
                  if (auth && m_arg.SecurityAssociation().Auth().IsSet()) {
                      ev.Do_FatalError();
                  }
              }
              // tell the session
              m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
              m_arg.RxPER();
          }
          else {
              ev.MsgType(PANA_EV_MTYPE_PEA);
          }
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ClientRxStateFilter *clone() {
         return (new PANA_CsmRxPE(*this));
      }
};

PANA_PacSession::PANA_PacSession(PANA_Node &n,
                                 PANA_ClientEventInterface &eif) :
   PANA_StateMachine<PANA_Client, PANA_Channel>
            (m_PaC, m_StateTable, n, m_Channel),
   m_Node(n),
   m_Channel(n.Job(), "Pac UDP Channel"),
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

   // Listen to a specific port
   sprintf(strAddr, "%d", PANA_CFG_GENERAL().m_ListenPort);
   addr.string_to_addr(strAddr);
   m_Channel.Open(addr);
   m_Channel.RegisterHandler(msgHandler);

   InitializeMsgMaps();
   PANA_StateMachine<PANA_Client, PANA_Channel>::Start();
}

PANA_PacSession::~PANA_PacSession()
{ 
   PANA_StateMachine<PANA_Client, PANA_Channel>::Stop();
   FlushMsgMaps();
   m_Channel.Close();
   m_Channel.RemoveHandler();
}

void PANA_PacSession::InitializeMsgMaps()
{
   PANA_CsmRxPSR PSR(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PSR, PSR);

   PANA_CsmRxPA PA(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PAR, PA);

   PANA_CsmRxPBR PBR(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PBR, PBR);

   PANA_CsmRxPP PP(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PPR, PP);

   PANA_CsmRxPRA PRA(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PRR, PRA);

   PANA_CsmRxPU PU(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PUR, PU);

   PANA_CsmRxPT PT(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PTR, PT);

   PANA_CsmRxPE PE(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PER, PE);
}

void PANA_PacSession::FlushMsgMaps()
{
   m_MsgHandlers.Remove(PANA_MTYPE_PSR);
   m_MsgHandlers.Remove(PANA_MTYPE_PAR);
   m_MsgHandlers.Remove(PANA_MTYPE_PBR);
   m_MsgHandlers.Remove(PANA_MTYPE_PPR);
   m_MsgHandlers.Remove(PANA_MTYPE_PRA);
   m_MsgHandlers.Remove(PANA_MTYPE_PUA);
   m_MsgHandlers.Remove(PANA_MTYPE_PTR);
   m_MsgHandlers.Remove(PANA_MTYPE_PER);
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

void PANA_PacSession::Update(ACE_INET_Addr &addr)
{
   m_PaC.PacAddress() = addr;
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_UPDATE);
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
