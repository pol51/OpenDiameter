/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open PANA_: Open-source software for the PANA_ and               */
/*                PANA_ related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open PANA_ Project                          */
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

    // ------------------------------
    // State: OFFLINE (Initial State)
    // ------------------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(stateful handshake) - - - - - - - - - -
    // Rx:PSR &&                RtxTimerStop();            WAIT_EAP_MSG_
    // PSR.exist_avp            EAP_Restart();             IN_INIT
    // ("EAP-Payload") &&       TxEAP();
    // !PSR.exist_avp
    // ("Cookie") &&
    // (!PSR.exist_avp
    // ("Protection-Cap.") ||
    // (PSR.exist_avp
    // ("Protection-Cap.") &&
    // pcap_supported())) &&
    // (!PSR.exist_avp
    // ("Algorithm") ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())) &&
    // !PSR.L_flag
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG_IN_INIT,
                       m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////
    // Rx:PSR &&                RtxTimerStop();            WAIT_PAA
    // !PSR.exist_avp           if (choose_isp())
    // ("EAP-Payload") &&         PSA.insert_avp("ISP");
    // !PSR.exist_avp           PSA.insert_avp("Nonce");
    // ("Cookie") &&            Tx:PSA();
    // (!PSR.exist_avp          EAP_Restart();
    // ("Protection-Cap.") ||
    // (PSR.exist_avp
    // ("Protection-Cap.") &&
    // pcap_supported())) &&
    // (!PSR.exist_avp
    // ("Algorithm") ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())) &&
    // !PSR.L_flag
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(stateless handshake) - - - - - - - - - -
    // Rx:PSR &&                RtxTimerStop();            WAIT_PAA
    // !PSR.exist_avp           if (choose_isp())
    // ("EAP-Payload") &&         PSA.insert_avp("ISP");
    // PSR.exist_avp            PSA.insert_avp("Cookie");
    // ("Cookie") &&            Tx:PSA();
    // (!PSR.exist_avp          RtxTimerStart();
    // ("Protection-Cap.") ||   EAP_Restart();
    // (PSR.exist_avp           STATELESS_HANDSHAKE=Set;
    // ("Protection-Cap.") &&
    // pcap_supported())) &&
    // (!PSR.exist_avp
    // ("Algorithm") ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())) &&
    // PSR.L_flag
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    ev.Flag_StatelessHandshake();
    ev.AvpExist_Cookie();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                        PANA_ST_WAIT_PAA,
                        m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(un-supported PSR) - - - - - - - - - - -
    // Rx:PSR &&                None();                    OFFLINE
    // (PSR.exist_avp
    // ("Protection-Cap.") &&
    // !pcap_supported()) ||
    // (PSR.exist_avp
    // ("Algorithm") &&
    // algorithm_supported())
    //
    // This is handled in PANA_CsmRxPSR::HandleMessage(..)
    //
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Authentication trigger from application) - - -
    // AUTH_USER                Tx:PCI();                  OFFLINE
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_AUTH_USER);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PacOfflineExitActionAuthUser);

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // - - - - - - - - (PSR processing with mobility support)- - - - -
    // - The following state transitions are intended to be added    -
    // - to the OFFLINE state of the PaC base protocol state         -
    // - machine.                                                    -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Rx:PSR &&                RtxTimerStop();            WAIT_PAA
    // !PSR.exist_avp           PSA.insert_avp
    // ("EAP-Payload") &&       ("Session-Id");
    // MOBILITY==Set &&         PANA_SA_RESUMED=Set;
    // resume_pana_sa() &&      PSA.insert_avp("Cookie");
    // PSR.exist_avp            PSA.insert_avp("AUTH");
    // ("Cookie")               Tx:PSA();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    ev.Do_ResumeSession(); // ((MOBILIT==Set) && resume_pana_sa())
    ev.AvpExist_Cookie();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // - - - - - - - - (PSR processing with mobility support)- - - - -
    // - The following state transitions are intended to be added    -
    // - to the OFFLINE state of the PaC base protocol state         -
    // - machine.                                                    -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Rx:PSR &&                RtxTimerStop();            WAIT_PAA
    // !PSR.exist_avp           PSA.insert_avp
    // ("EAP-Payload") &&       ("Session-Id");
    // MOBILITY==Set &&         PSA.insert_avp("AUTH");
    // resume_pana_sa() &&      Tx:PSA();
    // !PSR.exist_avp           PANA_SA_RESUMED=Set;
    // ("Cookie")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSR);
    ev.Do_ResumeSession(); // ((MOBILIT==Set) && resume_pana_sa())
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(), 
                        PANA_ST_WAIT_PAA, 
                        m_PacOfflineExitActionRxPSR);

    /////////////////////////////////////////////////////////////////
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

    /////////////////////////////////////////////////////////////////
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

    /////////////////////////////////////////////////////////////////
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

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OFFLINE, 
                               PANA_ST_OFFLINE);
#endif

    // ---------------------------
    // State: WAIT_EAP_MSG_IN_INIT
    // ---------------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - (Return PSA with EAP-Payload) - - - - - -
    // EAP_RESPONSE             PSA.insert_avp             WAIT_PAA
    //                             ("EAP-Payload")
    //                             if (choose_isp())
    //                             PSA.insert_avp("ISP");
    //                             PSA.insert_avp("Nonce");
    //                             Tx:PSA();
    //
    // EAP_RESP_TIMEOUT ||      None();                    OFFLINE
    // EAP_INVALID_MSG
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_INIT, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPSA);
    // Required to deal with eap piggyback enabled flag
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_INIT, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitEapMsgInExitActionTxPSA);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG          None();                    OFFLINE
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_INIT, ev.Get(),
                       PANA_ST_OFFLINE);

    /////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT         None();                    OFFLINE
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_INIT, ev.Get(),
                       PANA_ST_OFFLINE);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_INIT,
                               PANA_ST_WAIT_EAP_MSG_IN_INIT);
#endif

    // ---------------
    // State: WAIT_PAA
    // ---------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(PAR-PAN exchange) - - - - - - -
    // Rx:PAR &&                RtxTimerStop();         WAIT_EAP_MSG
    // !eap_piggyback()         TxEAP();
    //                          PANA_SA_RESUMED=Unset;
    //                          EAP_RespTimerStart();
    //                          if (key_available())
    //                            PAN.insert_avp("AUTH");
    //                          Tx:PAN();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // Rx:PAR &&                RtxTimerStop();         WAIT_EAP_MSG
    // eap_piggyback()          TxEAP();
    //                          PANA_SA_RESUMED=Unset;
    //                          EAP_RespTimerStart();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    ev.EnableCfg_EapPiggyback();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PacWaitPaaExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // Rx:PAN                   RtxTimerStop();         WAIT_PAA
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_PAA,
                       m_PacWaitPaaExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - -(EAP result) - - - - - - - - - - -
    // Rx:PBR &&                TxEAP();                   WAIT_EAP_RESULT
    // PBR.RESULT_CODE==        if (PBR.exist_avp
    //  PANA_SUCCESS &&            ("Device-Id"))
    // (!PBR.exist_avp             CARRY_DEVICE_ID=Set;
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

    /////////////////////////////////////////////////////////////////
    // Rx:PBR &&                TxEAP();                   WAIT_EAP_RESULT_
    // PBR.RESULT_CODE!=                                   CLOSE
    //   PANA_SUCCESS &&
    // PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.AvpExist_EapPayload();
    ev.ResultCode(PANA_RESULT_CODE_FAIL);
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT_CLOSE,
                       m_PacWaitPaaExitActionRxPBR);

    /////////////////////////////////////////////////////////////////
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
    // - - - - - - - - - - -(EAP result w/ mobility) - - - - - - - - -
    // - The following state transitions are intended to replace     -
    // - existing base protocol state transitions in the WAIT_PAA    -
    // - state. Original base protocol state transitions can be      -
    // - referenced by exit conditions that excludes PANA_SA_RESUMED -
    // - variable checks.                                            -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Rx:PBR &&                TxEAP();                   WAIT_EAP_RESULT
    // PBR.RESULT_CODE==        if (PBR.exist_avp
    //  PANA_SUCCESS &&           ("Device-Id"))
    // PANA_SA_RESUMED==Set &&    CARRY_DEVICE_ID=Set;
    // PBR.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    ev.Do_ResumeSession(); // PANA_SA_RESUMED==Set
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_WAIT_EAP_RESULT,
                       m_PacWaitPaaExitActionRxPBR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - (PBR processing with mobility support)- - - - -
    // - The following state transitions are intended to be added    -
    // - to the WAIT_PAA state of the PaC base protocol state        -
    // - machine.                                                    -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Rx:PBR &&                PBA.insert_avp("Key-Id");  OPEN
    // PBR.RESULT_CODE==        PBA.insert_avp("AUTH");
    //   PANA_SUCCESS &&        if (PBR.exist_avp
    // PANA_SA_RESUMED==Set &&     ("Device-Id"))
    // PBR.exist_avp            PBA.insert("Device-Id");
    // ("Key-Id") &&            Tx:PBA();
    // PBR.exist_avp            Authorize();
    // ("AUTH") &&              SessionTimerStart();
    // !PBR.exist_avp
    // ("EAP-Payload")
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBR);
    ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
    ev.Do_ResumeSession(); // PANA_SA_RESUMED==Set
    ev.AvpExist_KeyId();
    ev.AvpExist_Auth();
    AddStateTableEntry(PANA_ST_WAIT_PAA, ev.Get(),
                       PANA_ST_OPEN, 
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
    //                          if (FIRST_AUTH_EXCHG==Set &&
    //                              STATELESS_HANDSHAKE==Set)
    //                              PAN.insert_avp("Nonce");
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
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESPONSE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PAA, 
                       m_PacWaitEapMsgExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // EAP_RESP_TIMEOUT       if (key_available())       WAIT_EAP_MSG
    //                          PAN.insert_avp("AUTH");
    //                        Tx:PAN();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PacWaitEapMsgExitActionTxPANTout);

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
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG, 
                               PANA_ST_WAIT_EAP_MSG);
#endif

    // ----------------------
    // State: WAIT_EAP_RESULT
    // ----------------------


    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (EAP Result) - - - - - - - - - - - -
    // EAP_SUCCESS &&          PBA.insert_avp("AUTH");      OPEN
    // PBR.exist_avp           PBA.insert_avp("Key-Id");
    // ("Key-Id") &&           if (CARRY_DEVICE_ID)
    // ppac_available() &&       PBA.insert_avp
    // (!PBR.exist_avp           ("Device-Id");
    // ("Protection-           PBA.insert_avp("PPAC");
    //  Capability") ||        Tx:PBA();
    // (PBR.exist_avp          Authorize();
    // ("Protection-           SessionTimerStart();
    //  Capability") &&
    // pcap_supported()))
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        OPEN
    // !PBR.exist_avp            PBA.insert_avp("AUTH");
    // ("Key-Id") &&           if (CARRY_DEVICE_ID)
    // ppac_available() &&       PBA.insert_avp
    // (!PBR.exist_avp           ("Device-Id");
    // ("Protection-           PBA.insert_avp("PPAC");
    //   Capability") ||       Tx:PBA();
    // (PBR.exist_avp          Authorize();
    // ("Protection-           SessionTimerStart();
    //   Capability") &&
    // pcap_supported()))
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PacWaitEapResultExitActionEapOpen);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        WAIT_PEA
    // !ppac_available()         PER.insert_avp("AUTH");
    //                         PER.RESULT_CODE=
    //                           PANA_PPAC_CAPABILITY_
    //                             UNSUPPORTED
    //                         Tx:PER();
    //                         RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.NotSupported_Ppac();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PacWaitEapResultExitActionTxPERPpac);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        WAIT_PEA
    // (PBR.exist_avp            PER.insert_avp("AUTH");
    // ("Protection-           PER.RESULT_CODE=
    //   Capability") &&         PANA_PROTECTION_
    // !pcap_supported())          CAPABILITY_UNSUPPORTED
    //                         Tx:PER();
    //                         RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.NotSupported_Pcap();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PacWaitEapResultExitActionTxPERPcap);

    /////////////////////////////////////////////////////////////////
    // EAP_INVALID_MSG         None();                     WAIT_PAA
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_INVALID_MSG);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT, ev.Get(), 
                       PANA_ST_WAIT_PAA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_RESULT, 
                               PANA_ST_WAIT_EAP_RESULT);
#endif

    // ----------------------------
    // State: WAIT_EAP_RESULT_CLOSE
    // ----------------------------

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          PBA.insert_avp("AUTH");      CLOSED
    // PBR.exist_avp           PBA.insert_avp("Key-Id");
    // ("Key-Id")              Tx:PBA();
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.AvpExist_KeyId();
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PacWaitEapResultCloseExitAction);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&          if (key_available())        CLOSED
    // !PBR.exist_avp            PBA.insert_avp("AUTH");
    // ("Key-Id")              Tx:PBA();
    //                         Disconnect();
    // 
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PacWaitEapResultCloseExitAction);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE             Tx:PBA();                   CLOSED
    //                         Disconnect();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_RESULT_CLOSE, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PacWaitEapResultCloseExitAction);

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
    // - - - - - - - - (liveness test based initiated by PAA)- - - - - -
    // Rx:PPR                  if (key_available())       OPEN
    //                            PPA.insert_avp("AUTH");
    //                          Tx:PPA();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PacOpenExitActionRxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (liveness test based initiated by PaC)- - - - - -
    // PANA_PING                if (key_available())       WAIT_PPA
    //                           PPR.insert_avp("AUTH");
    //                          Tx:PPR();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_PPA, 
                       m_PacOpenExitActionTxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PaC)- - - - - -
    // REAUTH                   if (key_available())       WAIT_PRAA
    //                            PRAR.insert_avp("AUTH");
    //                          Tx:PRAR();
    //                          RtxTimerStart();
    //                          SessionTimerStop();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_PRAA, 
                       m_PacOpenExitActionTxPRAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // !eap_piggyback()         TxEAP();
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
    // Rx:PAR &&                EAP_RespTimerStart();      WAIT_EAP_MSG
    // eap_piggyback()          TxEAP();
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
    //                          PTA.insert_avp("AUTH");
    //                            Tx:PTA();
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
    //                             PTR.insert_avp("AUTH");
    //                          Tx:PTR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_SESS_TERM, 
                       m_PacOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Address update) - - - - - - - - - - 
    // NOTIFY                   if (key_available())       WAIT_PUA
    //                            PUR.insert_avp("AUTH");
    //                          Tx:PUR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_NOTIFICATION);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_PUA, 
                       m_PacOpenExitActionTxPUR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - -(Notification update)- - - - - - - - - - 
    // Rx:PUR                   if (key_available())       OPEN
    //                            PUA.insert_avp("AUTH");
    //                          Tx:PUA();
    // 
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PacOpenExitActionRxPUR);

    /////////////////////////////////////////////////////////////////
    //     SESS_TIMEOUT             Disconnect();              CLOSED
    //
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
    // State: WAIT_PRAA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(liveness test initiated by PAA) - - - - - - -
    // Rx:PRAA                  RtxTimerStop();            WAIT_PAA
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PRAA);
    AddStateTableEntry(PANA_ST_WAIT_PRAA, ev.Get(), 
                       PANA_ST_WAIT_PAA, 
                       m_PacWaitPRAAExitActionRxPRAA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Retransmit();              (no change)
    // RTX_COUNTER<
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_ReTransmission();
    AddStateTableEntry(PANA_ST_WAIT_PRAA, ev.Get(), 
                       PANA_ST_WAIT_PRAA, 
                       m_PacExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Reach maximum number of retransmission)- -
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PRAA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PRAA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PRAA, ev.Get(), 
                       PANA_ST_WAIT_PRAA, 
                       m_PacExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PRAA, 
                               PANA_ST_WAIT_PRAA);
#endif

    // ----------------
    // State: WAIT_PPA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(liveness test initiated by PAA) - - - - - - -
    // Rx:PPA                   None();                    OPEN
    //
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
    // Rx:PUA                  RtxTimerStop();            OPEN
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUA);
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PacWaitPUAExitActionRxPUA);

    /////////////////////////////////////////////////////////////////
    // SESS_TIMEOUT             Disconnect();              CLOSED
    //
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
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTA);
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PacSessTermExitActionRxPTA);

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
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PEA);
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PacWaitPEAExitActionRxPEA);

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

         // second level validation
         m_arg.RxValidateMsg(msg);

         // verify algorithm if present and supported
         PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
         pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
         if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
             AAA_LOG((LM_INFO, "(%P|%t) Supplied algorithm [0x%x] is not supported, session will close\n",
                     *algo));
             throw (PANA_Exception(PANA_Exception::FAILED,
                    "PSA recevied, unsupported algorithm"));
         }

         // verify if protection capability is present and supported
         PANA_UInt32AvpContainerWidget pcapAvp(msg.avpList());
         pana_unsigned32_t *pcap = pcapAvp.GetAvp(PANA_AVPNAME_PROTECTIONCAP);
         if (pcap) {
             if (ACE_NTOHL(*pcap) != PANA_CFG_GENERAL().m_ProtectionCap)) {
                 AAA_LOG((LM_INFO, "(%P|%t) Supplied protection-capability [0x%x] is not supported, session will close\n",
                         *pcap));
                 throw (PANA_Exception(PANA_Exception::FAILED,
                         "PSA recevied, unsupported protection-capability"));
             }
         }

         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(PANA_EV_MTYPE_PSR);
         if (msg.Flags().stateless) {
             ev.Flag_StatelessHandshake();
         }

         PANA_StringAvpContainerWidget eapAvp(msg.avpList());
         if (eapAvp.GetAvp(PANA_AVPNAME_EAP)) {
             ev.AvpExist_EapPayload();
             if (msg.Flags().stateless) {
                 AAA_LOG((LM_INFO, "(%P|%t) PSA advertizing stateless message with EAP-payload\n"));
                 throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                         "PSA recevied, invalid message"));
             }
         }
         else {
             PANA_StringAvpContainerWidget cookieAvp(msg.avpList());
             if (cookieAvp.GetAvp(PANA_AVPNAME_COOKIE)) {
                 if (!msg.Flags().stateless) {
                     AAA_LOG((LM_INFO, "(%P|%t) PSA advertizing stateful message with cookie present\n"));
                     throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                             "PSA recevied, invalid message"));
                 }
                 ev.AvpExist_Cookie();
             }
             // ((MOBILIT==Set) && resume_pana_sa())
             if (PANA_CFG_GENERAL().m_MobilityEnabled) {
                 if (m_arg.IsSessionResumed()) {
                     m_arg.AuxVariables().SecAssociationResumed() = true;
                     ev.Do_ResumeSession();
                 }
                 else {
                     m_arg.AuxVariables().SecAssociationResumed() = false;
                 }
             }
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
         // update session-id 
         if (msg.flags().request && (m_arg.SessionId().size() == 0)) {
             PANA_Utf8AvpContainerWidget sidAvp(msg.avpList());
             pana_utf8string_t *sid = sidAvp.GetAvp(PANA_AVPNAME_SESSIONID);
             if (sid == NULL) {
                 throw (PANA_Exception(PANA_Exception::FAILED, 
                        "Session id missing from PAR"));
             }
             m_arg.SessionId() = *sid;
         }
         // first level validation
         m_arg.RxValidateMsg(msg);
         // resolve the event
         PANA_PacEventVariable ev;
         if (msg.flags().request) {
            ev.MsgType(PANA_EV_MTYPE_PAR);
            ev.EnableCfg_EapPiggyback(PANA_CFG_GENERAL().m_EapPiggyback ? 1 : 0);
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

         // third level validation
         PANA_UInt32AvpContainerWidget algoAvp(msg.avpList());
         pana_unsigned32_t *algo = algoAvp.GetAvp(PANA_AVPNAME_ALGORITHM);
         if (algo && (ACE_NTOHL(*algo) != PANA_AUTH_ALGORITHM())) {
            throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                   "PBR received with no matching algorithm"));
         }

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
         if (rcode && (PANA_RCODE_SUCCESS(ACE_NTOHL(*rcode)))) {
            ev.ResultCode(PANA_RESULT_CODE_SUCCESS);
         }
         else {
            ev.ResultCode(PANA_RESULT_CODE_FAIL);
         }
         if (m_arg.AuxVariables().SecAssociationResumed()) {
            ev.Do_ResumeSession();
            PANA_UInt32AvpContainerWidget keyIdAvp(msg.avpList());
            pana_unsigned32_t *keyId = keyIdAvp.GetAvp(PANA_AVPNAME_KEYID);
            if (keyId) {
                ev.AvpExist_KeyId();
            }
            PANA_StringAvpContainerWidget authAvp(msg.avpList());
            pana_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
            if (auth) {
                ev.AvpExist_Auth();
            }
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

class PANA_CsmRxPRAA : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPRAA(PANA_Client &c, PANA_PacSession &s) : 
         PANA_ClientRxStateFilter(c, s) { 
          static PANA_ST validStates[] = { PANA_ST_WAIT_PRAA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
         // first level validation
         if (msg.flags().request) {
            throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                   "PRAR received, invalid message"));
         }
         // second level validation
         m_arg.RxValidateMsg(msg);
         // resolve the event
         PANA_PacEventVariable ev;
         ev.MsgType(PANA_EV_MTYPE_PRAA);
         m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
         m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ClientRxStateFilter *clone() { 
         return (new PANA_CsmRxPRAA(*this)); 
      }
};

class PANA_CsmRxPE : public PANA_ClientRxStateFilter
{
   public:
      PANA_CsmRxPE(PANA_Client &c, PANA_PacSession &s) : 
         PANA_ClientRxStateFilter(c, s) { 
          static PANA_ST validStates[] = { PANA_ST_OFFLINE,
                                           PANA_ST_WAIT_PAA,
                                           PANA_ST_OPEN,
                                           PANA_ST_WAIT_EAP_MSG,
                                           PANA_ST_WAIT_PRAA,
                                           PANA_ST_WAIT_PPA,
                                           PANA_ST_SESS_TERM,
                                           PANA_ST_WAIT_PUA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          // resolve the event
          PANA_PacEventVariable ev;
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PER);
              if (m_arg.IsFatalError()) {
                  PANA_StringAvpContainerWidget authAvp(msg.avpList());
                  pana_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
                  if (auth && m_arg.SecurityAssociation().IsSet()) {
                      ev.Do_FatalError();
                  }
              }
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
   PANA_StateMachine<PANA_Client, PANA_SenderChannel>
            (m_PaC, m_StateTable, n, m_Udp),
   m_Node(n),
   m_Udp(n.Job(), "Pac UDP Channel"),
   m_Timer(*this),
   m_PaC(m_TxChannel, m_Timer, eif) 
{          
   OD_Utl_SCSIAdapter1<PANA_PacSession, 
                   void(PANA_PacSession::*)(PANA_Message&),
                   PANA_Message&> 
               msgHandler(*this, &PANA_PacSession::Receive);
   
   ACE_INET_Addr addr;
   GetBindAddress(addr);
    
   m_Udp.Open(addr);
   m_Udp.RegisterHandler(msgHandler);
   m_PaC.LoadLocalAddress();
   
   InitializeMsgMaps();
   PANA_StateMachine<PANA_Client, PANA_SenderChannel>::Start(); 
}

PANA_PacSession::~PANA_PacSession() 
{ 
   PANA_StateMachine<PANA_Client, PANA_SenderChannel>::Stop(); 
   FlushMsgMaps(); 
   m_Udp.Close();
   m_Udp.RemoveHandler();
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

   PANA_CsmRxPRAA PRAA(m_PaC, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PRAR, PRAA);

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
   m_MsgHandlers.Remove(PANA_MTYPE_PRAA);
   m_MsgHandlers.Remove(PANA_MTYPE_PUA);
   m_MsgHandlers.Remove(PANA_MTYPE_PTR);
   m_MsgHandlers.Remove(PANA_MTYPE_PER);
}

void PANA_PacSession::GetBindAddress(ACE_INET_Addr &addr)
{
   if (PANA_CFG_GENERAL().m_IPv6Enabled) {
#if defined(ACE_HAS_IPV6)
       PANA_CfgClient &c = PANA_CFG_PAC();
       if (c.m_PaaMcastAddress.size() > 0) {
           char addrBuf[64];
           ACE_OS::sprintf(addrBuf, "%s:%d",
                           c.m_PaaMcastAddress.data(),
                           PANA_CFG_GENERAL().m_ListenPort);
           addr.set(addrBuf);
       }
       else {
           addr.set_type(AF_INET6);
       }
       addr.set_port_number(PANA_CFG_GENERAL().m_ListenPort);
#else
       throw PANA_Exception(PANA_Exception::FAILED,
                           "No IPv6 support in this system");
#endif
   }
   else {
       addr.set(PANA_CFG_GENERAL().m_ListenPort);
   }
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
   if (PANA_CFG_GENERAL().m_EapPiggyback) {
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
   if (m_PaC.SupportFlags().i.PcapNotSupported) {
       ev.NotSupported_Pcap();
       Notify(ev.Get());
       return;
   }
   if (m_PaC.SupportFlags().i.PpacNotSupported) {
       ev.NotSupported_Ppac();
       Notify(ev.Get());
       return;
   }
   if (m_PaC.SecurityAssociation().IsSet()) {
       ev.AvpExist_KeyId();
   }
   Notify(ev.Get());
}

void PANA_PacSession::EapTimeout()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_RESP_TIMEOUT);
   Notify(ev.Get());
}

void PANA_PacSession::EapFailure()
{
   PANA_PacEventVariable ev;
   ev.Event_Eap(PANA_EV_EAP_FAILURE);
   if (m_PaC.SupportFlags().i.PcapNotSupported) {
       ev.NotSupported_Pcap();
       Notify(ev.Get());
       return;
   }
   if (m_PaC.SecurityAssociation().IsSet()) {
       ev.AvpExist_KeyId();
   }
   Notify(ev.Get());
}

void PANA_PacSession::EapReAuthenticate()
{
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_REAUTH);
   Notify(ev.Get());
}

void PANA_PacSession::Update(ACE_INET_Addr &addr, std::string &msg)
{
   m_PaC.LastTxNotification() = msg;
   m_PaC.PacIpAddress() = addr;
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_NOTIFICATION);
   Notify(ev.Get());
}

void PANA_PacSession::SendNotification(std::string &msg)
{
   m_PaC.LastTxNotification() = msg;
   PANA_PacEventVariable ev;
   ev.Event_App(PANA_EV_APP_NOTIFICATION);
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

void PANA_ClientStateTable::PacExitActionRetransmission::
   operator()(PANA_Client &c)
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
