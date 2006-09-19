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

    // ------------------------------
    // State: OFFLINE (Initial State)
    // ------------------------------

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // - - - - - - - - - - - - - (Stateful handshake)- - - - - - - - -
    // (Rx:PCI ||               EAP_Restart();             WAIT_EAP_MSG_
    //  PAC_FOUND) &&                                      IN_DISC
    //  USE_COOKIE==Unset &&
    //  PIGGYBACK==Set
    // 
    ev.Event_App(PANA_EV_APP_PAC_FOUND);
    ev.EnableCfg_PiggyBack();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG_IN_DISC, 
                       m_PaaExitActionPaaEapRestart);

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // - - - - - - - - - - - - - (Stateful handshake)- - - - - - - - -
    // (Rx:PCI ||               if (SEPARATE==Set)         STATEFUL_DISC
    //  PAC_FOUND) &&             PSR.S_flag=1;
    //  USE_COOKIE==Unset &&    if (CARRY_NAP_INFO==Set)
    //  PIGGYBACK==Unset          PSR.insert_avp
    //                             ("NAP-Information");
    //                          if (CARRY_ISP_INFO==Set)
    //                            PSR.insert_avp
    //                             ("ISP-Information");
    //                          if (CARRY_PPAC==Set)
    //                            PSR.insert_avp
    //                             ("Post-PANA-Address-
    //                               Configuration");
    //                          Tx:PSR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PAC_FOUND);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(), 
                       PANA_ST_STATEFUL_DISC, 
                       m_PaaExitActionTxPSR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - (PSA processing without mobility support) - - - -
    // Rx:PSA &&                if (SEPARATE==Set &&       WAIT_EAP_MSG
    // USE_COOKIE==Set &&           PSA.S_flag==0)
    // (!PSA.exist_avp            SEPARATE=Unset;
    // ("Session-Id") ||        EAP_Restart();
    // MOBILITY==Unset ||       if (SEPARATE==Set)
    // (MOBILITY==Set &&          NAP_AUTH=Set|Unset
    // !retrieve_pana_sa
    //  (PSA.SESSION_ID)))
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSA);
    ev.EnableCfg_UseCookie();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaExitActionRxPSA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - (PSA processing with mobility support)- - - - -
    // Rx:PSA &&                PBR.insert_avp("AUTH");     WAIT_SUCC_PBA
    // USE_COOKIE==Set &&       PBR.insert_avp("Key-Id");
    // PSA.exist_avp            if (CARRY_DEVICE_ID
    // ("Session-Id") &&           ==Set)
    // MOBILITY==Set &&           PBR.insert_avp
    // retrieve_pana_sa            ("Device-Id");   
    // (PSA.SESSION_ID)         if (PROTECTION_CAP==Set)      
    //                             PBR.insert_avp
    //                              ("Protection-Cap.");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSA);
    ev.Do_Mobility();
    ev.EnableCfg_UseCookie();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionRxPSA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OFFLINE, 
                               PANA_ST_OFFLINE);
#endif                            

    // ---------------------------
    // State: WAIT_EAP_MSG_IN_DISC
    // ---------------------------

    /////////////////////////////////////////////////////////////////
    // Exit Condition           Exit Action                Exist State
    // ------------------------+--------------------------+------------
    // - - - - - - - - - - - (Send PSR with EAP-Request) - - - - - - -
    // EAP_REQUEST            PSR.insert_avp             STATEFUL_DISC
    //                         ("EAP-Payload");
    //                        if (CARRY_NAP_INFO==Set)
    //                          PSR.insert_avp
    //                          ("NAP-Information");
    //                        if (CARRY_ISP_INFO==Set)
    //                          PSR.insert_avp
    //                          ("ISP-Information");
    //                        if (CARRY_PPAC==Set)
    //                          PSR.insert_avp
    //                          ("Post-PANA-Address-
    //                            Configuration");
    //                        Tx:PSR();
    //                        RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_DISC, ev.Get(), 
                       PANA_ST_STATEFUL_DISC, 
                       m_PaaExitActionTxPSR);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT              None();                    OFFLINE
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_DISC, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG_IN_DISC);
    
    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG_IN_DISC, 
                               PANA_ST_WAIT_EAP_MSG_IN_DISC);
#endif                               

    // --------------------
    // State: STATEFUL_DISC
    // --------------------

    /////////////////////////////////////////////////////////////////
    // Exit Condition           Action                     Next-State
    // ------------------------+--------------------------+------------
    // - - - - - - - - - - - - - (Stateful handshake)- - - - - - - - -
    // Rx:PSA                   if (SEPARATE==Set &&       WAIT_EAP_MSG
    //                          PSA.S_flag==0)
    //                             SEPARATE=Unset;
    //                          if PSA.Exist("EAP-Payload")
    //                             TxEAP();
    //                          else {
    //                             if (SEPARATE==Set) 
    //                                 NAP_AUTH=Set|Unset
    //                             EAP_Restart();
    //                          }
    //                          RtxTimerStop();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSA);
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaExitActionRxPSA);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT              if (key_available())       WAIT_PEA
    //                            PER.insert_avp("AUTH");
    //                          Tx:PER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionTxPER);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
                       PANA_ST_STATEFUL_DISC, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_STATEFUL_DISC, ev.Get(), 
                       PANA_ST_STATEFUL_DISC, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_STATEFUL_DISC, 
                               PANA_ST_STATEFUL_DISC);
#endif

    // -------------------
    // State: WAIT_EAP_MSG
    // -------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(Receiving EAP-Request)- - - - - - - - -
    // EAP_REQUEST            if (key_available())       WAIT_PAN_OR_PAR
    //                          PAR.insert_avp("AUTH");
    //                        if (SEPARATE==Set) {
    //                          PAR.S_flag=1;
    //                          if (NAP_AUTH==Set)
    //                            PAR.N_flag=1;
    //                        }
    //                        Tx:PAR();
    //                        RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PAN_OR_PAR, 
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - -(Receiving EAP-Success/Failure single EAP)- - - - -
    // EAP_FAILURE &&           PBR.insert_avp            WAIT_FAIL_PBA
    // 1ST_EAP==Unset &&        ("EAP-Payload");
    // SEPARATE==Unset          if (key_available())
    //                           PBR.insert_avp("AUTH");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp            WAIT_SUCC_PBA
    // 1ST_EAP==Unset &&        ("EAP-Payload");
    // SEPARATE==Unset &&       if (CARRY_DEVICE_ID
    // Authorize()                 ==Set)
    //                          PBR.insert_avp
    //                          ("Device-Id");
    //                          if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                             ("Session-Lifetime");
    //                          if (PROTECTION_CAP==Set)
    //                            PBR.insert_avp
    //                              ("Protection-Cap.");
    //                          if (new_key_available())
    //                             PBR.insert_avp
    //                               ("Key-Id");
    //                          if (key_available())
    //                             PBR.insert_avp("AUTH");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapSuccess);

    /////////////////////////////////////////////////////////////////
    // - - - - - - -(Receiving EAP-Success/Failure for 1st EAP)- - - -
    // EAP_SUCCESS &&         PBR.insert_avp            WAIT_FAIL_PBA
    // 1ST_EAP==Unset &&        ("EAP-Payload");
    // SEPARATE==Unset &&     if (new_key_available())
    // !Authorize()             PBR.insert_avp
    //                          ("Key-Id");
    //                        if (key_available())
    //                          PBR.insert_avp("AUTH");
    //                        Tx:PBR();
    //                        RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapSuccessFail);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (key_available())       WAIT_PEA
    // 1ST_EAP==Unset &&          PER.insert_avp("AUTH");
    // SEPARATE==Unset          Tx:PER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionTxPER);

    /////////////////////////////////////////////////////////////////
    // - - - - - - -(Receiving EAP-Success/Failure for 1st EAP)- - - -
    // EAP_FAILURE &&           1ST_EAP=Failure            WAIT_PFEA
    // 1ST_EAP==Unset &&        PFER.insert_avp
    // SEPARATE==Set &&         ("EAP-Payload");
    // ABORT_ON_1ST_EAP_FAILURE if (key_available())
    // ==Unset                    PFER.insert_avp("AUTH");
    //                          PFER.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PFER.N_flag=1;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionTxPFEREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE &&           1ST_EAP=Failure            WAIT_FAIL_PFEA
    // 1ST_EAP==Unset &&        PFER.insert_avp
    // SEPARATE==Set &&         ("EAP-Payload");
    // ABORT_ON_1ST_EAP_FAILURE if (key_available())
    // ==Set                      PFER.insert_avp("AUTH");
    //                          PFER.S_flag=0;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    ev.Do_AbortOnFirstEap();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PFEA, 
                       m_PaaExitActionTxPFEREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&         1ST_EAP=Success            WAIT_PFEA
    // 1ST_EAP==Unset &&      PFER.insert_avp
    // SEPARATE==Set            ("EAP-Payload");
    //                        if (new_key_available())
    //                          PFER.insert_avp
    //                          ("Key-Id");
    //                        if (key_available())
    //                          PFER.insert_avp("AUTH");
    //                        PFER.S_flag=1;
    //                        if (NAP_AUTH)
    //                          PFER.N_flag=1;
    //                        Tx:PFER();
    //                        RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionTxPFEREapSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           1ST_EAP=Failure            WAIT_PFEA
    // 1ST_EAP==Unset &&        if (key_available())
    // SEPARATE==Set &&           PFER.insert_avp("AUTH");
    // ABORT_ON_1ST_EAP_FAILURE PFER.S_flag=1;
    // ==Unset                  if (NAP_AUTH)
    //                          PFER.N_flag=1;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionTxPFEREapTimeout);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           1ST_EAP=Failure            WAIT_FAIL_PFEA
    // 1ST_EAP==Unset &&        if (key_available())
    // SEPARATE==Set &&           PFER.insert_avp("AUTH");
    // ABORT_ON_1ST_EAP_FAILURE SEPARATE=Unset;
    // ==Set                    PFER.S_flag=0;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    ev.Do_AbortOnFirstEap();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PFEA, 
                       m_PaaExitActionTxPFEREapTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - - -(Receiving EAP-Success/Failure for 2nd EAP)- - - -
    // EAP_FAILURE &&           PBR.insert_avp             WAIT_FAIL_PBA
    // 1ST_EAP==Failure &&      ("EAP-Payload");
    // SEPARATE==Set            if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    // 
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_FAIL);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE &&           PBR.insert_avp             WAIT_SUCC_PBA
    // 1ST_EAP==Success &&      ("EAP-Payload");
    // SEPARATE==Set &&         if (CARRY_DEVICE_ID==Set)
    // Authorize()                PBR.insert_avp
    //                            ("Device-Id");
    //                          if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (PROTECTION_CAP_IN_PBR
    //                              ==Set)
    //                            PBR.insert_avp
    //                            ("Protection-Cap.");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapFailSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_FAILURE &&           PBR.insert_avp             WAIT_FAIL_PBA
    // 1ST_EAP==Success &&      ("EAP-Payload");
    // SEPARATE==Set &&         if (key_available())
    // !Authorize()               PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    // 
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_SUCC_PBA
    // 1ST_EAP==Success &&      ("EAP-Payload");
    // SEPARATE==Set &&         if (CARRY_DEVICE_ID==Set)
    // Authorize()                PBR.insert_avp
    //                            ("Device-Id");
    //                          if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (PROTECTION_CAP_IN_PBR
    //                              ==Set)
    //                            PBR.insert_avp
    //                            ("Protection-Cap.");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_FAIL_PBA
    // 1ST_EAP==Success &&      ("EAP-Payload");
    // SEPARATE==Set &&         if (new_key_available())
    // !Authorize()               PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapSuccessFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_SUCC_PBA
    // 1ST_EAP==Failure &&      ("EAP-Payload");
    // SEPARATE==Set &&         if (CARRY_DEVICE_ID==Set)
    // Authorize()                PBR.insert_avp
    //                            ("Device-Id");
    //                          if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (PROTECTION_CAP_IN_PBR
    //                              ==Set)
    //                            PBR.insert_avp
    //                            ("Protection-Cap.");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_FAIL);
    ev.Do_Separate();
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_FAIL_PBA
    // 1ST_EAP==Failure &&      ("EAP-Payload");  <--- victor: new_key_available()
    // SEPARATE==Set &&         if (key_available())
    // !Authorize()               PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    // 
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Result_FirstEap(PANA_RESULT_CODE_FAIL);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapSuccessFail);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (key_available())       WAIT_FAIL_PBA
    // 1ST_EAP==Failure &&        PBR.insert_avp("AUTH");
    // SEPARATE==Set            PBR.S_flag=1;
    //                         if (NAP_AUTH)
    //                           PBR.N_flag=1;
    //                         Tx:PBR();
    //                         RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_FAIL);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapTimeout);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (CARRY_DEVICE_ID==Set)  WAIT_SUCC_PBA
    // 1ST_EAP==Success &&        PBR.insert_avp
    // SEPARATE==Set &&           ("Device-Id");
    // Authorize()              if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (PROTECTION_CAP_IN_PBR
    //                              ==Set)
    //                            PBR.insert_avp
    //                            ("Protection-Cap.");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapTimeoutSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (key_available())       WAIT_FAIL_PBA
    // 1ST_EAP==Success &&        PBR.insert_avp("AUTH");
    // SEPARATE==Set &&         PBR.S_flag=1;
    // !Authorize()             if (NAP_AUTH)
    //                           PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

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
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_EAP_MSG, 
                               PANA_ST_WAIT_EAP_MSG);
#endif

    // ----------------
    // State: WAIT_PFEA
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(PFEA Processing)- - - - - - - - - - - -
    // Rx:PFEA &&               RtxTimerStop();            WAIT_EAP_MSG
    // (1ST_EAP==Success ||     EAP_Restart();
    // (PFEA.S_flag==1 &&      if (NAP_AUTH==Set)
    //  1ST_EAP==Failure))         NAP_AUTH=Unset;
    //                         else
    //                             NAP_AUTH=Set;
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PFEA);
    ev.EnableFlag_Separate();  // (1ST_EAP==Success ||
                               // (PFEA.S_flag==1 &&
                               //  1ST_EAP==Failure))
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaWaitPFEAExitActionRxPFEASuccess);

    /////////////////////////////////////////////////////////////////
    // Rx:PFEA &&               RtxTimerStop();            CLOSED
    // PFEA.S_flag==0 &&        Disconnect();
    // 1ST_EAP==Failure
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PFEA);
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaWaitPFEAExitActionRxPFEAFail);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PFEA, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PFEA, 
                               PANA_ST_WAIT_PFEA);
#endif

    // ---------------------
    // State: WAIT_FAIL_PFEA
    // ---------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PFEA Processing)- - - - - - - - - -
    // Rx:PFEA                 RtxTimerStop();            CLOSED
    //                         Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PFEA);
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaWaitPFEAExitActionRxPFEAFail);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PFEA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PFEA, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_FAIL_PFEA, 
                               PANA_ST_WAIT_FAIL_PFEA);
#endif

    // --------------------
    // State: WAIT_SUCC_PBA
    // --------------------
    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (PBA Processing)- - - - - - - - - - -
    // Rx:PBA &&                SessionTimerStart();       OPEN
    // (CARRY_DEVICE_ID==Unset ||
    // (CARRY_DEVICE_ID==Set &&
    // PBA.exit_avp("Device-Id")))
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBA);
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PaaExitActionRxPBASuccess);

    /////////////////////////////////////////////////////////////////
    // Rx:PBA &&                 PER.RESULT_CODE=          WAIT_PEA
    // CARRY_DEVICE_ID==Set &&     PANA_MISSING_AVP
    // !PBA.exit_avp             Tx:PER();
    //  ("Device-Id")           RtxTimerStart();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBA);
    ev.Do_MissingId();  // CARRY_DEVICE_ID==Set &&
                        // !PBA.exit_avp
                        //  ("Device-Id")
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionTxPERMissingAvp);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_SUCC_PBA, 
                               PANA_ST_WAIT_SUCC_PBA);
#endif

    // --------------------
    // State: WAIT_FAIL_PBA
    // --------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - (PBA Processing)- - - - - - - - - -
    // Rx:PBA                   RtxTimerStop();            CLOSED
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBA);
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PBA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionRxPBAFail);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PBA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PBA, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PBA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_FAIL_PBA, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_FAIL_PBA, 
                               PANA_ST_WAIT_FAIL_PBA);
#endif

    // -----------
    // State: OPEN
    // -----------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - (re-authentication initiated by PaC) - -
    // Rx:PRAR            if (key_available())       WAIT_EAP_MSG
    //                       PRAA.insert_avp("AUTH");
    //                    EAP_Restart();
    //                    1ST_EAP=Unset;
    //                    NAP_AUTH=Set|Unset;
    //                    Tx:PRAA();
    //                    SessionTimerStop();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PRAR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaOpenExitActionRxPRAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - (re-authentication initiated by PAA) - - - - - -
    // REAUTH                 EAP_Restart();         WAIT_EAP_MSG
    //                        1ST_EAP=Unset;
    //                        NAP_AUTH=Set|Unset;
    //                        SessionTimerStop();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaOpenExitActionReAuth);

    /////////////////////////////////////////////////////////////////
    // - - (liveness test based on PPR-PPA exchange initiated by PAA)-
    // PANA_PING          Tx:PPR();                  WAIT_PPA
    //                    RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_PAA,
                       m_PaaOpenExitActionTxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (liveness test based initiated by PAA)- - - - - -
    // Rx:PPR             if (key_available())       OPEN
    //                       PPA.insert_avp("AUTH");
    //                    Tx:PPA();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PaaOpenExitActionRxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PAA) - - - -
    // TERMINATE               if (key_available())       SESS_TERM
    //                           PTR.insert_avp("AUTH");
    //                         Tx:PTR();
    //                         RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_SESS_TERM, 
                       m_PaaOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PaC) - - - -
    // Rx:PTR                 if (key_available())       CLOSED
    //                          PTA.insert_avp("AUTH");
    //                        Tx:PTA();
    //                        Disconnect();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaOpenExitActionRxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - -(Notification message) - - - - - - - - - -
    // NOTIFY                 if (key_available())       WAIT_PUA
    //                           PUR.insert_avp("AUTH");
    //                        Tx:PUR();
    //                        RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_NOTIFICATION);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_WAIT_PUA, 
                       m_PaaOpenExitActionTxPUR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - -(Notification/Address update)  - - - - - - - - 
    // Rx:PUR                   If (key_avaialble())       OPEN
    //                            PUA.insert_avp("AUTH");
    //                          Tx:PUA();
    //                          if (new_source_address())
    //                            update_popa();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PaaOpenExitActionRxPUR);

    /////////////////////////////////////////////////////////////////
    // SESS_TIMEOUT             Disconnect();              CLOSED
    //
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

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
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OPEN, 
                               PANA_ST_OPEN);
#endif                               

    // ---------------
    // State: WAIT_PPA
    // ---------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PPA processing) - - - - - - - - - -
    // Rx:PPA                   RtxTimerStop();            OPEN
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPA);
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(), 
                       PANA_ST_OPEN, 
                       m_PaaWaitPaaExitActionRxPPA);

    /////////////////////////////////////////////////////////////////
    // SESS_TIMEOUT             Disconnect();              CLOSED
    //
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(), 
                       PANA_ST_WAIT_PPA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

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
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PPA, 
                               PANA_ST_WAIT_PPA);
#endif

    // ----------------------
    // State: WAIT_PAN_OR_PAR
    // ----------------------

    /////////////////////////////////////////////////////////////////
    // ------------------------+--------------------------+------------
    // - - - - - - (Pass EAP Resposne to the EAP authenticator)- - - -
    // Rx:PAN &&                TxEAP();                   WAIT_EAP_MSG
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
    // Rx:PAR                 TxEAP();                   WAIT_EAP_MSG
    //                        if (key_available())
    //                           PAN.insert_avp("AUTH");
    //                        if (SEPARATE==Set) {
    //                           PAN.S_flag=1;
    //                           if (NAP_AUTH==Set)
    //                             PAN.N_flag=1;
    //                        }
    //                        RtxTimerStop();
    //                        Tx:PAN();
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_EAP_MSG, 
                       m_PaaWaitPANExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PAN without an EAP response) - - - - - - 
    // Rx:PAN &&                RtxTimerStop();          WAIT_PAN_OR_PAR
    // !PAN.exist_avp
    // ("EAP-Payload")
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_PAN_OR_PAR, 
                       m_PaaWaitPANExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(EAP retransmission) - - - - - - - - -
    // EAP_REQUEST              if (key_available())       WAIT_PAN_OR_PAR
    //                            PAR.insert_avp("AUTH");
    //                          if (SEPARATE==Set) {
    //                            PAR.S_flag=1;
    //                          if (NAP_AUTH==Set)
    //                            PAR.N_flag=1;
    //                          }
    //                          Tx:PAR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_PAN_OR_PAR, 
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - -(EAP authentication timeout)- - - - - - - - 
    // EAP_TIMEOUT &&           if (key_available())       WAIT_PEA
    // 1ST_EAP==Unset &&          PER.insert_avp("AUTH");
    // SEPARATE==Unset          Tx:PER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionTxPER);

    /////////////////////////////////////////////////////////////////
    // - - - - - -(EAP authentication timeout for 1st EAP)- - - - - 
    // EAP_TIMEOUT &&           1ST_EAP=Failure            WAIT_PFEA
    // 1ST_EAP==Unset &&        if (key_available())
    // SEPARATE==Set &&           PFER.insert_avp("AUTH");
    // ABORT_ON_1ST_EAP_FAILURE PFER.S_flag=1;
    // ==Unset                  if (NAP_AUTH)
    //                            PFER.N_flag=1;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_PFEA, 
                       m_PaaExitActionTxPFEREapTimeout);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           1ST_EAP=Failure          WAIT_FAIL_PFEA
    // 1ST_EAP==Unset &&        if (key_available())
    // SEPARATE==Set &&           PFER.insert_avp("AUTH");
    // ABORT_ON_1ST_EAP_FAILURE SEPARATE=Unset;
    // ==Set                    PFER.S_flag=0;
    //                          Tx:PFER();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_UNSET);
    ev.Do_Separate();
    ev.Do_AbortOnFirstEap();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PFEA, 
                       m_PaaExitActionTxPFEREapTimeout);

    /////////////////////////////////////////////////////////////////
    // - - - - - -(EAP authentication timeout for 2nd EAP)- - - - - 
    // EAP_TIMEOUT &&           if (key_available())       WAIT_FAIL_PBA
    // 1ST_EAP==Failure &&        PBR.insert_avp("AUTH");
    // SEPARATE==Set            PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_FAIL);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapTimeout);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (CARRY_DEVICE_ID==Set)  WAIT_SUCC_PBA
    // 1ST_EAP==Success &&        PBR.insert_avp
    // SEPARATE==Set &&           ("Device-Id");
    // Authorize()              if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (PROTECTION_CAP_IN_PBR
    //                              ==Set)
    //                            PBR.insert_avp
    //                            ("Protection-Cap.");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          PBR.S_flag=1;
    //                          if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    // 
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_SUCC_PBA, 
                       m_PaaExitActionTxPBREapTimeoutSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_TIMEOUT &&           if (key_available())       WAIT_FAIL_PBA
    // 1ST_EAP==Success &&        PBR.insert_avp("AUTH");
    // SEPARATE==Set &&         PBR.S_flag=1;
    // !Authorize()             if (NAP_AUTH)
    //                            PBR.N_flag=1;
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    ev.Result_FirstEap(PANA_RESULT_CODE_SUCCESS);
    ev.Do_Separate();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_FAIL_PBA, 
                       m_PaaExitActionTxPBREapTimeout);

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
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(), 
                       PANA_ST_WAIT_PAN_OR_PAR, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, 
                               PANA_ST_WAIT_PAN_OR_PAR);
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
                       m_PaaWaitPUAExitActionRxPUA);

    /////////////////////////////////////////////////////////////////
    // SESS_TIMEOUT            Disconnect();              CLOSED
    //
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTimeout);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(), 
                       PANA_ST_WAIT_PUA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

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
                       m_PaaExitActionTxPEA);

    // ----------------
    // State: SESS_TERM
    // ----------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - -(PTA processing) - - - - - - - - - -
    // Rx:PTA                   RtxTimerStop();            CLOSED
    //                          Disconnect();
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
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

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
                       m_PaaExitActionTxPEA);

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
                       m_PaaWaitPEAExitActionRxPEA);

    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(), 
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
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionRetransmission);

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
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(), 
                       PANA_ST_CLOSED, 
                       m_PaaExitActionTxPEA);

    /////////////////////////////////////////////////////////////////
    // Rx:PER &&                Tx:PEA();                  (no change)
    // !fatal
    // (PER.RESULT_CODE) ||
    // !PER.exist_avp("AUTH") ||
    // !key_available()
    //
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PER);
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(), 
                       PANA_ST_WAIT_PEA, 
                       m_PaaExitActionTxPEA);

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

class PANA_PsmRxPCI : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPCI(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OFFLINE };
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

class PANA_PsmRxPSA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPSA(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OFFLINE,
                                           PANA_ST_STATEFUL_DISC };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) { 
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                    "PSR recevied, invalid message"));
          }
          // second level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PSA);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);

          PANA_Utf8AvpContainerWidget sidAvp(msg.avpList());
          pana_utf8string_t *sid = sidAvp.GetAvp(PANA_AVPNAME_SESSIONID);
          if (sid && PANA_CFG_GENERAL().m_MobilityEnabled) {
              ev.Do_Mobility();
          }
          if (PANA_CFG_PAA().m_UseCookie) {
              ev.EnableCfg_UseCookie();
          }
          m_Session.Notify(ev.Get()); 
      }
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPSA(*this)); 
      }
};

class PANA_PsmRxPFEA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPFEA(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PFEA,
                                           PANA_ST_WAIT_FAIL_PFEA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) { 
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                    "PFER message with request flag set, invalid message"));
          }
          // second level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PFEA);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);

          if (PANA_RCODE_SUCCESS(m_arg.AuxVariables().FirstEapResult()) ||
              (! PANA_RCODE_SUCCESS(m_arg.AuxVariables().FirstEapResult()) &&
                 msg.flags().separate)) {
              ev.EnableFlag_Separate();
          }
          m_Session.Notify(ev.Get()); 
      }
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPFEA(*this)); 
      }
};

class PANA_PsmRxPBA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPBA(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_SUCC_PBA,
                                           PANA_ST_WAIT_FAIL_PBA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          if (msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                    "PBR message with request flag set, invalid message"));
          }
          // second level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PBA);
          // checked for presence of device id
          PANA_AddressAvpContainerWidget pacIdAvp(msg.avpList());
          pana_address_t *id = pacIdAvp.GetAvp(PANA_AVPNAME_DEVICEID);
          if (m_arg.AuxVariables().CarryDeviceId()) {
             if (! id) {
                ev.Do_MissingId();
             }
             else {
                bool match = false;
                PANA_DeviceIdIterator i = PANA_CFG_PAA().m_EpIdList.begin();
                for (; i != PANA_CFG_PAA().m_EpIdList.end(); i++) {
                   if ((*i)->type == id->type) {
                      match = true;
                      break;
                   }
                }
                if (! match) {
                   ev.Do_MissingId();
                }
             }
          }
          
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPBA(*this)); 
      }
};

class PANA_PsmRxPRAR : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPRAR(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          if (! msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                    "PRAA message with request flag set, invalid message"));
          }
          // second level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PRAR);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPRAR(*this)); 
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

class PANA_PsmRxPU : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPU(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_PUA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // msg validation
          m_arg.RxValidateMsg(msg);
          
          // resolve the event
          PANA_PaaEventVariable ev;

          // IP address update check
          if (msg.flags().request) {
              ACE_INET_Addr newAddr;
              PANA_DeviceId *ipId = PANA_DeviceIdConverter::GetIpOnlyAddress
                                          (msg.srcDevices(),
                                           (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
              if (ipId == NULL) {
                  throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                         "Cannot determine the device ID of peer"));
              }          

              PANA_DeviceIdConverter::FormatToAddr(*ipId, newAddr);
              newAddr.set_port_number(msg.srcPort());
              // save address of PaC       
              if (m_arg.PacIpAddress() != newAddr) {
                  AAA_LOG((LM_INFO, "(%P|%t) New IP address detected for Pac ... updating\n"));
                  m_arg.PacIpAddress() = newAddr;
                  m_arg.PacDeviceId() = *ipId;
              }

	      ev.MsgType(PANA_EV_MTYPE_PUR);
          }
          else {
	      ev.MsgType(PANA_EV_MTYPE_PUA);
	  }
          
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPU(*this)); 
      }
};

class PANA_PsmRxPP : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPP(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_PPA };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(msg.flags().request ? 
                     PANA_EV_MTYPE_PPR : PANA_EV_MTYPE_PPA);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPP(*this)); 
      }
};

class PANA_PsmRxPA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPA(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_WAIT_PAN_OR_PAR };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(msg.flags().request ?
                     PANA_EV_MTYPE_PAR : PANA_EV_MTYPE_PAN);
          if (! msg.flags().request) {
              PANA_StringAvpContainerWidget eapAvp(msg.avpList());
              pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
              if (payload) {
                  ev.AvpExist_EapPayload();
              }
          }
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPA(*this)); 
      }
};

class PANA_PsmRxPE : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPE(PANA_Paa &a, PANA_PaaSession &s) : 
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_STATEFUL_DISC,
                                           PANA_ST_WAIT_SUCC_PBA,
                                           PANA_ST_WAIT_PEA,
                                           PANA_ST_WAIT_PAN_OR_PAR,
                                           PANA_ST_WAIT_FAIL_PBA,
                                           PANA_ST_WAIT_PFEA,
                                           PANA_ST_WAIT_FAIL_PFEA,
                                           PANA_ST_WAIT_PEA,
                                           PANA_ST_OPEN,
                                           PANA_ST_WAIT_EAP_MSG,
                                           PANA_ST_WAIT_PPA,
                                           PANA_ST_WAIT_PAN_OR_PAR,
                                           PANA_ST_SESS_TERM,
                                           PANA_ST_CLOSED };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          m_arg.RxValidateMsg(msg);
          // resolve the event
          PANA_PaaEventVariable ev;
          if (msg.flags().request) {
              ev.MsgType(PANA_EV_MTYPE_PER);
              if (m_arg.IsFatalError()) {
                  ev.Do_FatalError();
              }
          }
          else {
              ev.MsgType(PANA_EV_MTYPE_PEA);
          }
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get()); 
      };
      virtual PANA_ServerRxStateFilter *clone() { 
          return (new PANA_PsmRxPE(*this)); 
      }
};

void PANA_PaaSession::InitializeMsgMaps()
{
   PANA_PsmRxPCI PCI(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PCI, PCI);

   PANA_PsmRxPSA PSA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PSA, PSA);

   PANA_PsmRxPFEA PFEA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PFEA, PFEA);

   PANA_PsmRxPA PA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PAR, PA);

   PANA_PsmRxPBA PBA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PBA, PBA);

   PANA_PsmRxPRAR PRAR(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PRAR, PRAR);

   PANA_PsmRxPT PT(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PTR, PT);

   PANA_PsmRxPU PU(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PUR, PU);

   PANA_PsmRxPP PP(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PPR, PP);

   PANA_PsmRxPE PE(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PER, PE);
}

void PANA_PaaSession::FlushMsgMaps()
{
   m_MsgHandlers.Remove(PANA_MTYPE_PCI);
   m_MsgHandlers.Remove(PANA_MTYPE_PSA);
   m_MsgHandlers.Remove(PANA_MTYPE_PFEA);
   m_MsgHandlers.Remove(PANA_MTYPE_PAN);
   m_MsgHandlers.Remove(PANA_MTYPE_PBA);
   m_MsgHandlers.Remove(PANA_MTYPE_PRAR);
   m_MsgHandlers.Remove(PANA_MTYPE_PTR);
   m_MsgHandlers.Remove(PANA_MTYPE_PUR);
   m_MsgHandlers.Remove(PANA_MTYPE_PPA);
   m_MsgHandlers.Remove(PANA_MTYPE_PAN);
   m_MsgHandlers.Remove(PANA_MTYPE_PER);
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
    if ((state == PANA_ST_WAIT_EAP_MSG_IN_DISC) ||
        (state == PANA_ST_STATEFUL_DISC)) {
        return;
    }
    Notify(ev.Get());
 }

void PANA_PaaSession::EapReAuthenticate() 
{
    PANA_PaaEventVariable ev;
    ev.Event_App(PANA_EV_APP_REAUTH);
    Notify(ev.Get());
}

void PANA_PaaSession::Update(ACE_INET_Addr &addr, std::string &msg)
{
   m_PAA.LastTxNotification() = msg;
   m_PAA.PacIpAddress() = addr;
   PANA_PaaEventVariable ev;
   ev.Event_App(PANA_EV_APP_NOTIFICATION);
   Notify(ev.Get());
}

void PANA_PaaSession::SendNotification(std::string &msg)
{
    PANA_PaaEventVariable ev;
    m_PAA.LastTxNotification() = msg;
    ev.Event_App(PANA_EV_APP_NOTIFICATION);
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
