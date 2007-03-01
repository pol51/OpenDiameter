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

#include "pana_paa.h"
#include "pana_config_manager.h"
#include "pana_pmk_bootstrap.h"

PANA_Paa::PANA_Paa(PANA_SessionTxInterface &tp,
                   PANA_SessionTimerInterface &tm,
                   PANA_PaaEventInterface &ev) :
    PANA_Session(tp, tm, ev)
{
    //  ------------------------------
    //  State: OFFLINE (Initial State)
    //  ------------------------------
    //
    //  Initialization Action:
    //
    //    OPTIMIZED_HANDSHAKE=Set|Unset;
    //    CARRY_LIFETIME=Set|Unset;
    //    FIRST_AUTH_EXCHG=Set;
    //    RTX_PSR=Set|Unset;
    //    RTX_COUNTER=0;
    //    RtxTimerStop();

    Reset();

    // generate a new session id
    ACE_Time_Value tv = ACE_OS::gettimeofday();
    this->SessionId() = tv.sec() + tv.usec();
}

void PANA_Paa::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_PacAddress = this->PacAddress();
    args.m_PaaAddress = this->PaaAddress();

    if (SecurityAssociation().MSK().IsSet()) {
        args.m_Key.Set(SecurityAssociation().MSK().Get());
    }

    args.m_Lifetime.Set(SessionLifetime());
    m_Event.Authorize(args);
}

void PANA_Paa::NotifyEapRestart()
{
    m_Timer.CancelSession();
    m_Event.EapStart();
}

void PANA_Paa::NotifyEapResponse(pana_octetstring_t &payload)
{
    AAAMessageBlock *block = AAAMessageBlock::Acquire(payload.size());
    if (block) {
        block->copy((char*)payload.data(), payload.size());
        block->wr_ptr(block->base());
        static_cast<PANA_PaaEventInterface&>(m_Event).EapResponse(block);
        block->Release();
    }
}

void PANA_Paa::NotifyEapTimeout()
{
    TxPER(PANA_ERROR_UNABLE_TO_COMPLY);
    Disconnect(PANA_ERROR_UNABLE_TO_COMPLY);
}

void PANA_Paa::NotifyEapReAuth()
{
    m_Event.EapStart();
    m_Timer.CancelSession();
}

bool PANA_Paa::IsUserAuthorized()
{
    bool IsAuth = static_cast<PANA_PaaEventInterface&>
                              (m_Event).IsUserAuthorized();
    return (AuxVariables().Authorized() = IsAuth);
}

void PANA_Paa::TxPSR()
{
   /*
      7.2.  PANA-Start-Request (PSR)

         The PANA-Start-Request (PSR) message is sent by the PAA to the PaC to
         start PANA authentication.  The PAA sets the Sequence Number field to
         an initial random value and sets the Session Identifier field to a
         newly assigned value.

         PANA-Start-Request ::= < PANA-Header: 2, REQ >
                             [ EAP-Payload ]
                             [ Algorithm ]
                          *  [ AVP ]
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // add eap payload
    if (PANA_CFG_PAA().m_OptimizedHandshake) {
        if (AuxVariables().TxEapMessageQueue().Empty()) {
            throw (PANA_Exception(PANA_Exception::MISSING_EAP_PAYLOAD, 
                   "No EAP payload generated on optimized handshake"));
        }

        PANA_MsgBlockGuard eapPkt(AuxVariables().TxEapMessageQueue().Dequeue());
        PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
        eapAvp.Get().assign(eapPkt()->base(), eapPkt()->size());
        msg->avpList().add(eapAvp());
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPSR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg, PANA_CFG_PAA().m_RetryPSR);
}

void PANA_Paa::RxPSA()
{
   /*
     7.3.  PANA-Start-Answer (PSA)

        The PANA-Start-Answer (PSA) message is sent by the PaC to the PAA in
        response to a PANA-Start-Request message.  This message completes the
        handshake to start PANA authentication.

        PANA-Start-Answer ::= < PANA-Header: 2 >
                            [ EAP-Payload ]
                         *  [ AVP ]
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPSA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    if ((PANA_CFG_PAA().m_OptimizedHandshake == 0) &&
        (PANA_CFG_PAA().m_RetryPSR == 0)) {
        // Rx:PSA in OFFLINE state
        NotifyEapRestart();
    }
    else {
        // Rx:PSA in WAIT_PAC_IN_INIT state
        if (PANA_CFG_PAA().m_RetryPSR) {
            m_Timer.CancelTxRetry();
        }

        PANA_StringAvpContainerWidget eapAvp(msg.avpList());
        pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
        if (payload) {
            NotifyEapResponse(*payload);
        }
        else {
            NotifyEapRestart();
        }
    }
}

void PANA_Paa::TxPAR()
{
    /*
      7.4.  PANA-Auth-Request (PAR)

         The PANA-Auth-Request (PAR) message is either sent by the PAA or the
         PaC.  Its main task is to carry an EAP-Payload AVP.

         PANA-Auth-Request ::= < PANA-Header: 3, REQ >
                             < EAP-Payload >
                             [ Nonce ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // add eap payload
    if (AuxVariables().TxEapMessageQueue().Empty()) {
        throw (PANA_Exception(PANA_Exception::MISSING_EAP_PAYLOAD, 
               "No EAP payload on TxPAR"));
    }

    PANA_MsgBlockGuard eapPkt(AuxVariables().TxEapMessageQueue().Dequeue());
    PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
    eapAvp.Get().assign(eapPkt()->base(), eapPkt()->size());
    msg->avpList().add(eapAvp());

    // add paa nonce
    if (! SecurityAssociation().PaaNonce().IsSet()) {
        // generate nouce
        SecurityAssociation().PaaNonce().Generate();

        pana_octetstring_t &nonce = SecurityAssociation().PaaNonce().Get();
        PANA_StringAvpWidget nonceAvp(PANA_AVPNAME_NONCE);
        nonceAvp.Get().assign(nonce.data(), nonce.size());
        msg->avpList().add(nonceAvp());
    }

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::TxPBR(pana_unsigned32_t rcode,
                     EAP_EVENT ev)
{
    /*
      7.8.  PANA-Bind-Request (PBR)

         The PANA-Bind-Request (PBR) message is sent by the PAA to the PaC to
         deliver the result of PANA authentication.

         PANA-Bind-Request ::= < PANA-Header: 5, REQ >
                             { Result-Code }
                             [ EAP-Payload ]
                             [ Session-Lifetime ]
                             [ Key-Id ]
                             [ Algorithm ]
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PBR;
    msg->flags().request = true;

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // add result-code
    PANA_UInt32AvpWidget rcodeAvp(PANA_AVPNAME_RESULTCODE);
    rcodeAvp.Get() = ACE_HTONL(rcode);
    msg->avpList().add(rcodeAvp());

    PANA_MsgBlockGuard eapPkt(AuxVariables().TxEapMessageQueue().Dequeue());
    if (eapPkt() && (ev != EAP_TIMEOUT)) {
        // add eap payload
        PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
        eapAvp.Get().assign(eapPkt()->base(), eapPkt()->size());
        msg->avpList().add(eapAvp());
    }

    if (ev == EAP_SUCCESS) {

        if (AuxVariables().Authorized()) {
            if (SessionLifetime() > 0) {
                // add session lifetime
                PANA_UInt32AvpWidget lifetimeAvp(PANA_AVPNAME_SESSIONLIFETIME);
                lifetimeAvp.Get() = ACE_HTONL(SessionLifetime());
                msg->avpList().add(lifetimeAvp());
            }
        }

        // update the aaa key's
        pana_octetstring_t newKey;
        if (m_Event.IsKeyAvailable(newKey)) {
            SecurityAssociation().MSK().Set(newKey);
            if (! AuxVariables().AlgorithmIsSet()) {
                // add algorithm
                // TBD: need to make sure algo value is ok
                PANA_UInt32AvpWidget algoAvp(PANA_AVPNAME_ALGORITHM);
                algoAvp.Get() = ACE_HTONL(PANA_AUTH_ALGORITHM());
                msg->avpList().add(algoAvp());
                AuxVariables().AlgorithmIsSet() = true;
            }
            SecurityAssociation().AddKeyIdAvp(*msg);
            SecurityAssociation().GenerateAuthKey(this->SessionId());
        }
    }

    // add key from existing SA
    if (SecurityAssociation().Auth().IsSet()) {
        // add existing auth first before generating a new one
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPBR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::RxPBA(bool success)
{
    /*
      7.9.  PANA-Bind-Answer (PBA)

         The PANA-Bind-Answer (PBA) message is sent by the PaC to the PAA in
         response to a PANA-Bind-Request message.

         PANA-Bind-Answer ::= < PANA-Header: 5 >
                             [ Key-Id ]
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPBA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();
    if (success) {
        NotifyScheduleLifetime();
        NotifyAuthorization();
    }
    else {
        Disconnect();
    }
}

void PANA_Paa::TxPAN()
{
    /*
      7.5.  PANA-Auth-Answer (PAN)

         The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
         PAA in response to a PANA-Auth-Request message.  It MAY carry an
         EAP-Payload AVP.

         PANA-Auth-Answer ::= < PANA-Header: 3 >
                             [ Nonce ]
                             [ EAP-Payload ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAN;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // add SA if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::RxPAR()
{
    /*
      7.4.  PANA-Auth-Request (PAR)

         The PANA-Auth-Request (PAR) message is either sent by the PAA or the
         PaC.  Its main task is to carry an EAP-Payload AVP.

         PANA-Auth-Request ::= < PANA-Header: 3, REQ >
                             < EAP-Payload >
                             [ Nonce ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapResponse(*payload);
    }

    m_Timer.CancelTxRetry();
    TxPAN();
}

void PANA_Paa::RxPAN()
{
    /*
      7.5.  PANA-Auth-Answer (PAN)

         The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
         PAA in response to a PANA-Auth-Request message.  It MAY carry an
         EAP-Payload AVP.

         PANA-Auth-Answer ::= < PANA-Header: 3 >
                             [ Nonce ]
                             [ EAP-Payload ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();

    // update pac nonce
    PANA_StringAvpContainerWidget nonceAvp(msg.avpList());
    pana_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && ! SecurityAssociation().PacNonce().IsSet()) {
        SecurityAssociation().PacNonce().Set(*nonce);
    }

    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapResponse(*payload);
    }
}

void PANA_Paa::RxPRR()
{
    /*
      7.6.  PANA-Reauth-Request (PRR)

         The PANA-Reauth-Request (PRR) message is sent by the PaC to the PAA
         to re-initiate EAP authentication.

         PANA-Reauth-Request ::= < PANA-Header: 4, REQ >
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPRR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    NotifyEapReAuth();
    TxPRA();
}

void PANA_Paa::TxPRA()
{
    /*
      7.7.  PANA-Reauth-Answer (PRA)

         The PANA-Reauth-Answer (PRA) message is sent by the PAA to the PaC in
         response to a PANA-Reauth-Request message.

         PANA-Reauth-Answer ::= < PANA-Header: 4 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PRA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // auth avp
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPRA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::TxPrepareMessage(PANA_Message &msg)
{
    msg.srcAddress() = this->PaaAddress();
    msg.destAddress() = this->PacAddress();
}

