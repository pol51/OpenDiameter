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
    // State: OFFLINE (Initial State)
    // ------------------------------
    //
    // Initialization Action:
    //
    //   OPTIMIZED_HANDSHAKE=Set|Unset;
    //   CARRY_LIFETIME=Set|Unset;
    //   FIRST_AUTH_EXCHG=Set;
    //   RTX_PSR=Set|Unset;
    //   RTX_COUNTER=0;
    //   RtxTimerStop();
    //

    ///////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Optimized Handshake) - - - - - - - - - - -
    // (Rx:PCI ||               EAP_Restart();             OFFLINE
    //  PAC_FOUND) &&                                      
    // OPTIMIZED_HANDSHAKE==Set
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PAC_FOUND);
    ev.EnableCfg_OptimizedHandshake();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PaaExitActionPaaEapRestart);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - (Send PSR with EAP-Request) - - - - - - -
    // EAP_REQUEST              PSR.insert_avp             OFFLINE
    //                           ("EAP-Payload");          
    //                          if (AUTH_ALGORITHM_IN_PSR
    //                              ==Set)
    //                            PSR.insert_avp
    //                            ("Algorithm");
    //                          Tx:PSR();
    //                          if (RTX_PSR == Set)
    //                            RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PaaExitActionTxPSR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (EAP timeout) - - - - - - - - - - - -
    // EAP_TIMEOUT              if (RTX_PSR == Set)        OFFLINE
    //                            RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PaaExitActionTimeout);
                       
    ///////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Non-Optimized Handshake) - - - - - - - - -
    // (Rx:PCI ||               if (AUTH_ALGORITHM_IN_PSR  OFFLINE
    //  PAC_FOUND) &&               ==Set)                 
    // OPTIMIZED_HANDSHAKE==      PSR.insert_avp
    //  Unset &&                  ("Algorithm");
    // RTX_PSR=Set              Tx:PSR();
    //                       RtxTimerStart();
    //
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PAC_FOUND);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PaaExitActionTxPSR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Rx:PSA                   if (PSA.exist_avp          WAIT_EAP_MSG
    //                             ("EAP-Payload"))
    //                            TxEAP();
    //                          else
    //                            EAP_Restart();
    //                          RtxTimerStop();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PSA);
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaExitActionRxPSA);
                       
    /////////////////////////////////////////////////////////////////
    // RTX_TIMEOUT &&           Disconnect();              CLOSED
    // RTX_COUNTER>=
    // RTX_MAX_NUM
    //
    ev.Reset();
    ev.Do_RetryTimeout();
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
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
    AddStateTableEntry(PANA_ST_OFFLINE, ev.Get(),
                       PANA_ST_OFFLINE,
                       m_PaaExitActionRetransmission);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Catch all processing)- - - - - - - - - -
    //
#if !defined(PANA_DEBUG)
    AddWildcardStateTableEntry(PANA_ST_OFFLINE,
                               PANA_ST_OFFLINE);
#endif

    // -------------------
    // State: WAIT_EAP_MSG
    // -------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(Receiving EAP-Request)- - - - - - - - -
    // EAP_REQUEST              if (key_available())       WAIT_PAN_OR_PAR
    //                            PAR.insert_avp("AUTH");
    //                          if (FIRST_AUTH_EXCHG==Set)
    //                            PAR.insert_avp("Nonce");
    //                          FIRST_AUTH_EXCHG=Unset;
    //                          Tx:PAR();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - -(Receiving EAP-Success/Failure) - - - - -
    // EAP_FAILURE              PBR.insert_avp             WAIT_FAIL_PBA
    //                          ("EAP-Payload");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_FAILURE);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_FAIL_PBA,
                       m_PaaExitActionTxPBREapFail);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_SUCC_PBA
    // Authorize()              ("EAP-Payload");
    //                          if (CARRY_LIFETIME==Set)
    //                            PBR.insert_avp
    //                            ("Session-Lifetime");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                            PBR.insert_avp
    //                            ("Algorithm");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    //
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    ev.Do_Authorize();
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_SUCC_PBA,
                       m_PaaExitActionTxPBREapSuccess);

    /////////////////////////////////////////////////////////////////
    // EAP_SUCCESS &&           PBR.insert_avp             WAIT_FAIL_PBA
    // !Authorize()             ("EAP-Payload");
    //                          if (new_key_available())
    //                            PBR.insert_avp
    //                            ("Key-Id");
    //                            PBR.insert_avp
    //                            ("Algorithm");
    //                          if (key_available())
    //                            PBR.insert_avp("AUTH");
    //                          Tx:PBR();
    //                          RtxTimerStart();
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_SUCCESS);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_FAIL_PBA,
                       m_PaaExitActionTxPBREapSuccessFail);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (re-authentication initiated by PaC) - - - - - -
    // Rx:PRR                  if (key_available())       WAIT_EAP_MSG
    //                            PRA.insert_avp("AUTH");
    //                          EAP_Restart();
    //                          Tx:PRA();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PRR);
    AddStateTableEntry(PANA_ST_WAIT_EAP_MSG, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionRxPRR);

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

    // --------------------
    // State: WAIT_SUCC_PBA
    // --------------------

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (PBA Processing)- - - - - - - - - - -
    // Rx:PBA                   SessionTimerStart();       OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PBA);
    AddStateTableEntry(PANA_ST_WAIT_SUCC_PBA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaExitActionRxPBASuccess);

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
    // - - - - - - - - (re-authentication initiated by PaC) - - - - - -
    // Rx:PRR                  if (key_available())       WAIT_EAP_MSG
    //                            PRA.insert_avp("AUTH");
    //                          EAP_Restart();
    //                          Tx:PRA();
    //                          SessionTimerStop();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PRR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionRxPRR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (re-authentication initiated by PAA)- - - - - -
    // REAUTH                   EAP_Restart();             WAIT_EAP_MSG
    //                          SessionTimerStop();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_REAUTH);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaOpenExitActionReAuth);

    /////////////////////////////////////////////////////////////////
    // - - (liveness test based on PPR-PPA exchange initiated by PAA)-
    // PANA_PING                Tx:PPR();                  WAIT_PPA
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_PING);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PPA,
                       m_PaaOpenExitActionTxPPR);

    /////////////////////////////////////////////////////////////////
    // - - (liveness test based on PPR-PPA exchange initiated by PaC)-
    // Rx:PPR                   if (key_available())       OPEN
    //                            PPA.insert_avp("AUTH");
    //                          Tx:PPA();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaOpenExitActionRxPPR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PAA) - - - -
    // TERMINATE                if (key_available())       SESS_TERM
    //                            PTR.insert_avp("AUTH");
    //                          Tx:PTR();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_TERMINATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_SESS_TERM,
                       m_PaaOpenExitActionTxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (Session termination initated from PaC) - - - -
    // Rx:PTR                   if (key_available())       CLOSED
    //                            PTA.insert_avp("AUTH");
    //                          Tx:PTA();
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaOpenExitActionRxPTR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - (PAA updates the PAA with attributes)- - - - -
    // UPDATE                   if (key_available())       WAIT_PUA
    //                            PUR.insert_avp("AUTH");
    //                          Tx:PUR();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_App(PANA_EV_APP_UPDATE);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_WAIT_PUA,
                       m_PaaOpenExitActionTxPUR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - - (Attribute update)  - - - - - - - - -
    // Rx:PUR                   If (key_avaialble())       OPEN
    //                            PUA.insert_avp("AUTH");
    //                          Tx:PUA();
    //                          if (new_source_address())
    //                            update_popa();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUR);
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaOpenExitActionRxPUR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_OPEN, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

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
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PPA);
    AddStateTableEntry(PANA_ST_WAIT_PPA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaWaitPaaExitActionRxPPA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
    // - - - - - - (Pass EAP Response to the EAP authenticator)- - - -
    // Rx:PAN &&                TxEAP();                   WAIT_EAP_MSG
    // PAN.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    ev.AvpExist_EapPayload();
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaWaitPANExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - (PAN without an EAP response) - - - - - - -
    // Rx:PAN &&                RtxTimerStop();            WAIT_PAN_OR_PAR
    // !PAN.exist_avp
    // ("EAP-Payload")
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAN);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaWaitPANExitActionRxPAN);

    /////////////////////////////////////////////////////////////////
    // Rx:PAR                   TxEAP();                   WAIT_EAP_MSG
    //                          if (key_available())
    //                            PAN.insert_avp("AUTH");
    //                          RtxTimerStop();
    //                          Tx:PAN();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PAR);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_EAP_MSG,
                       m_PaaWaitPANExitActionRxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - -(EAP retransmission) - - - - - - - - - -
    // EAP_REQUEST              if (key_available())       WAIT_PAN_OR_PAR
    //                            PAR.insert_avp("AUTH");
    //                          Tx:PAR();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_REQUEST);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PAN_OR_PAR,
                       m_PaaExitActionTxPAR);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - (EAP authentication timeout)- - - - - - - - -
    // EAP_TIMEOUT              if (key_available())       WAIT_PEA
    //                            PER.insert_avp("AUTH");
    //                          Tx:PER();
    //                          RtxTimerStart();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Event_Eap(PANA_EV_EAP_TIMEOUT);
    AddStateTableEntry(PANA_ST_WAIT_PAN_OR_PAR, ev.Get(),
                       PANA_ST_WAIT_PEA,
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
    // Rx:PUA                   RtxTimerStop();            OPEN
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PUA);
    AddStateTableEntry(PANA_ST_WAIT_PUA, ev.Get(),
                       PANA_ST_OPEN,
                       m_PaaWaitPUAExitActionRxPUA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
    // - - - - - - - - - - - - - -(PTA processing) - - - - - - - - - -
    // Rx:PTA                   RtxTimerStop();            CLOSED
    //                          Disconnect();
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PTA);
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaSessExitActionRxPTA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_SESS_TERM, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

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
    //                       Disconnect();
    ev.Reset();
    ev.MsgType(PANA_EV_MTYPE_PEA);
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaWaitPEAExitActionRxPEA);

    /////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - -(Session timeout)- - - - - - - - - - -
    // SESS_TIMEOUT             Disconnect();              CLOSED
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    ev.Reset();
    ev.Do_SessTimeout();
    AddStateTableEntry(PANA_ST_WAIT_PEA, ev.Get(),
                       PANA_ST_CLOSED,
                       m_PaaExitActionTimeout);

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

class PANA_PsmRxPSA : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPSA(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OFFLINE };
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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PSA);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);

          m_Session.Notify(ev.Get());
      }
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPSA(*this));
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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PBA);

          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPBA(*this));
      }
};

class PANA_PsmRxPRR : public PANA_ServerRxStateFilter
{
   public:
      PANA_PsmRxPRR(PANA_Paa &a, PANA_PaaSession &s) :
          PANA_ServerRxStateFilter(a, s) {
          static PANA_ST validStates[] = { PANA_ST_OPEN,
                                           PANA_ST_WAIT_EAP_MSG };
          AllowedStates(validStates, sizeof(validStates)/sizeof(PANA_ST));
      }
      virtual void HandleMessage(PANA_Message &msg) {
          // first level validation
          if (! msg.flags().request) {
             throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                    "PRA message with request flag set, invalid message"));
          }

          // second level validation
          m_arg.RxValidateMsg(msg);

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          ev.MsgType(PANA_EV_MTYPE_PRR);
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_Session.Notify(ev.Get());
      };
      virtual PANA_ServerRxStateFilter *clone() {
          return (new PANA_PsmRxPRR(*this));
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
          m_arg.PacAddress() = msg.srcAddress();

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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

          // resolve the event
          PANA_PaaEventVariable ev;
          if (msg.flags().request) {
              if (m_arg.PacAddress() != msg.srcAddress()) {
                  AAA_LOG((LM_INFO, "(%P|%t) New IP address detected for Pac ... updating\n"));
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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // save last received header
          m_arg.LastRxHeader() = msg;

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
          static PANA_ST validStates[] = { PANA_ST_WAIT_SUCC_PBA,
                                           PANA_ST_WAIT_PEA,
                                           PANA_ST_WAIT_PAN_OR_PAR,
                                           PANA_ST_WAIT_FAIL_PBA,
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

          // save address of PaC
          m_arg.PacAddress() = msg.srcAddress();

          // tell the session
          m_arg.AuxVariables().RxMsgQueue().Enqueue(&msg);
          m_arg.RxPER();

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

   PANA_PsmRxPA PA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PAR, PA);

   PANA_PsmRxPBA PBA(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PBA, PBA);

   PANA_PsmRxPRR PRR(m_PAA, *this);
   m_MsgHandlers.Register(PANA_MTYPE_PRR, PRR);

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
   m_MsgHandlers.Remove(PANA_MTYPE_PAN);
   m_MsgHandlers.Remove(PANA_MTYPE_PBA);
   m_MsgHandlers.Remove(PANA_MTYPE_PRR);
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
    Notify(ev.Get());
 }

void PANA_PaaSession::EapReAuthenticate()
{
    PANA_PaaEventVariable ev;
    ev.Event_App(PANA_EV_APP_REAUTH);
    Notify(ev.Get());
}

void PANA_PaaSession::Update(ACE_INET_Addr &addr)
{
   m_PAA.PacAddress() = addr;
   PANA_PaaEventVariable ev;
   ev.Event_App(PANA_EV_APP_UPDATE);
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
