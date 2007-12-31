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
    SessionId() = tv.sec() + tv.usec();

    // use config file to set IP reconfiguration parameter
    IpReconfiguration() = (PANA_CFG_PAA().m_IpReconfig) ? true : false;

    // use config file to set PAA IP address
    char buf[32];
    sprintf(buf, "%s:%d", PANA_CFG_GENERAL().m_ListenAddress.data(),
            PANA_CFG_GENERAL().m_ListenPort);
    PaaAddress().string_to_addr(buf);
}

void PANA_Paa::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_PacAddress = PacAddress();
    args.m_PaaAddress = PaaAddress();
    args.m_IpReconfiguration = IpReconfiguration();

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
    Disconnect(PANA_TERMCAUSE_SESSION_TIMEOUT);
}

void PANA_Paa::NotifyEapReAuth()
{
    SecurityAssociation().PacNonce().Reset();
    SecurityAssociation().PaaNonce().Reset();

    m_Event.EapStart();
    m_Timer.CancelSession();
}

bool PANA_Paa::IsUserAuthorized()
{
    bool IsAuth = static_cast<PANA_PaaEventInterface&>
                              (m_Event).IsUserAuthorized();
    return (AuxVariables().Authorized() = IsAuth);
}

void PANA_Paa::TxPARStart()
{
   /*
    7.2.  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Request ::= < PANA-Header: 2, REQ[,STA][,COM] >
                          [ Nonce ]
                         *[ PRF-Algorithm ]
                         *[ Integrity-Algorithm ]
                          [ Result-Code ]
                          [ Session-Lifetime ]
                          [ Key-Id ]
                        * [ AVP ]
                      0*1 < AUTH >
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;
    msg->flags().start = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    msg->seq() = ++ LastTxSeqNum();

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

    // add integrity algorithm
    PANA_UInt32AvpWidget integrityAlgoAvp(PANA_AVPNAME_INTEGRITY_ALGO);
    integrityAlgoAvp.Get() = PANA_AUTH_HMAC_SHA1_160;
    msg->avpList().add(integrityAlgoAvp());

    // add prf algorithm
    PANA_UInt32AvpWidget prfAlgoAvp(PANA_AVPNAME_PRF_ALGO);
    prfAlgoAvp.Get() = PANA_PRF_HMAC_SHA1;
    msg->avpList().add(prfAlgoAvp());

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR-Start: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendReqMsg(msg);

    // Save the PAR-Start for AUTH generation (stateful mode)
    PANA_MsgByteStream byteConverter;
    PANA_MessageBuffer *buffer = byteConverter.Get(*msg);
    SecurityAssociation().PARStart().assign(buffer->base(), msg->length());
}

void PANA_Paa::RxPANStart()
{
   /*
    7.3.  PANA-Auth-Answer (PAN)

      The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
      PAA in response to a PANA-Auth-Request message.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Answer ::= < PANA-Header: 2 [,STA][,COM] >
                          [ Nonce ]
                          [ EAP-Payload ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN-Start: id=%u seq=%u\n",
            msg.sessionId(), msg.seq()));

    PANA_UInt32AvpContainerWidget integrityAlgoAvp(msg.avpList());
    pana_unsigned32_t *integrityAlgo = integrityAlgoAvp.GetAvp(PANA_AVPNAME_INTEGRITY_ALGO);
    if (integrityAlgo == NULL) {
        throw (PANA_Exception(PANA_Exception::MISSING_ALGORITHM,
               "No Integrity Algorithm present"));
    }
    else if (*integrityAlgo != PANA_AUTH_HMAC_SHA1_160) {
        throw (PANA_Exception(PANA_Exception::INVALID_ALGORITHM,
               "Integrity Algorithm not supported"));
    }

    PANA_UInt32AvpContainerWidget prfAlgoAvp(msg.avpList());
    pana_unsigned32_t *prfAlgo = prfAlgoAvp.GetAvp(PANA_AVPNAME_PRF_ALGO);
    if (prfAlgo == NULL) {
        throw (PANA_Exception(PANA_Exception::MISSING_PRF,
               "No PRF Algorithm present"));
    }
    else if (*prfAlgo != PANA_PRF_HMAC_SHA1) {
        throw (PANA_Exception(PANA_Exception::INVALID_PRF,
               "PRF Algorithm not supported"));
    }

    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapResponse(*payload);
    }
    else if (! PANA_CFG_PAA().m_OptimizedHandshake) {
        NotifyEapRestart();
    }

    // Save the PAN-Start for AUTH generation (any mode)
    PANA_MsgByteStream byteConverter;
    PANA_MessageBuffer *buffer = byteConverter.Get(msg);
    SecurityAssociation().PANStart().assign(buffer->base(), msg.length());
}

void PANA_Paa::TxPAR()
{
   /*
    7.2.  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Request ::= < PANA-Header: 2, REQ[,STA][,COM] >
                          [ EAP-Payload ]
                          [ Algorithm ]
                          [ Nonce ]
                          [ Result-Code ]
                          [ Session-Lifetime ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    msg->seq() = ++ LastTxSeqNum();

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

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::TxPARComplete(pana_unsigned32_t rcode,
                             EAP_EVENT ev)
{
   /*
    7.2.  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Request ::= < PANA-Header: 2, REQ[,STA][,COM] >
                          [ EAP-Payload ]
                          [ Algorithm ]
                          [ Nonce ]
                          [ Result-Code ]
                          [ Session-Lifetime ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;
    msg->flags().complete = true;

    // adjust serial num
    msg->seq() = ++ LastTxSeqNum();
    msg->sessionId() = this->SessionId();

    // add result-code
    PANA_UInt32AvpWidget rcodeAvp(PANA_AVPNAME_RESULTCODE);
    rcodeAvp.Get() = rcode;
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
            if ((SessionLifetime() > 0) && PANA_CFG_PAA().m_CarryLifetime) {
                // add session lifetime
                PANA_UInt32AvpWidget lifetimeAvp(PANA_AVPNAME_SESSIONLIFETIME);
                lifetimeAvp.Get() = SessionLifetime();
                msg->avpList().add(lifetimeAvp());
            }
            if (IpReconfiguration()) {
                // Set the IP reconfiguration flag if needed
                msg->flags().ipreconfig = true;
            }
        }

        // update the aaa key's
        pana_octetstring_t newKey;
        if (m_Event.IsKeyAvailable(newKey)) {
            SecurityAssociation().MSK().Set(newKey);
            SecurityAssociation().AddKeyIdAvp(*msg);
            SecurityAssociation().GenerateAuthKey();
        }
    }

    // add key from existing SA
    if (SecurityAssociation().Auth().IsSet()) {
        // add existing auth first before generating a new one
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR-Complete: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::RxPANComplete(bool success)
{
   /*
    7.3.  PANA-Auth-Answer (PAN)

      The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
      PAA in response to a PANA-Auth-Request message.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Answer ::= < PANA-Header: 2 [,STA][,COM] >
                          [ Nonce ]
                          [ EAP-Payload ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN-Complete: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();
    if (success) {
        if (SessionLifetime() > 0) {
            NotifyScheduleLifetime(SessionLifetime() - PANA_REAUTH_GRACE_PERIOD);
        }
        NotifyAuthorization();
    }
    else {
        Disconnect();
    }
}

void PANA_Paa::TxPAN()
{
   /*
    7.3.  PANA-Auth-Answer (PAN)

      The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
      PAA in response to a PANA-Auth-Request message.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Answer ::= < PANA-Header: 2 [,STA][,COM] >
                          [ Nonce ]
                          [ EAP-Payload ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAN;
    msg->seq() = LastRxSeqNum();
    msg->sessionId() = this->SessionId();

    // add SA if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::RxPAR()
{
   /*
    7.2.  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Request ::= < PANA-Header: 2, REQ[,STA][,COM] >
                          [ EAP-Payload ]
                          [ Algorithm ]
                          [ Nonce ]
                          [ Result-Code ]
                          [ Session-Lifetime ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
    */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));

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
    7.3.  PANA-Auth-Answer (PAN)

      The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
      PAA in response to a PANA-Auth-Request message.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Answer ::= < PANA-Header: 2 [,STA][,COM] >
                          [ Nonce ]
                          [ EAP-Payload ]
                          [ Key-Id ]
                        *  [ AVP ]
                      0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));

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

void PANA_Paa::RxPNRAuth()
{
    /*
      7.6.  PANA-Notification-Request (PNR)

        The PANA-Notification-Request (PNR) message is sent either by the PaC
        or the PAA for signaling re-authentication and errors, and performing
        liveness test.

        If 'A' or 'P' bit is set, the message MUST NOT carry Result-Code,
        Failed-Message-Header and Failed-AVP AVPs.

        If 'E' bit is set, the message MUST carry one Result-Code AVP and one
        Failed-Message-Header AVP and MAY carry one or more Failed AVPs.

        The message MUST have one of 'A', 'P' and 'E' bits exclusively set.

      PANA-Notification-Request ::= < PANA-Header: 4, REQ[,REA][,PIN][,ERR] >
                          [ Result-Code ]
                          [ Failed-Message-Header ]
                        *  [ Failed-AVP ]
                        *  [ AVP ]
                      0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPNR-Auth: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));

    SecurityAssociation().PacNonce().Reset();
    SecurityAssociation().PaaNonce().Reset();

    NotifyEapReAuth();
    TxPNAAuth();
}

void PANA_Paa::TxPNAAuth()
{
    /*
      7.7.  PANA-Notification-Answer (PNA)

        The PANA-Notification-Answer (PNA) message is sent by the PAA (PaC)
        to the PaC (PAA) in response to a PANA-Notification-Request from the
        PaC (PAA).

        The message MUST have one of 'A', 'P' and 'E' bits exclusively set.

        PANA-Notification-Answer ::= < PANA-Header: 4, REQ[,REA][,PIN][,ERR] >
                        *  [ AVP ]
                        0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PNA;
    msg->flags().auth = true;
    msg->seq() = LastRxSeqNum();
    msg->sessionId() = this->SessionId();

    // auth avp
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPNA-Auth: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::TxPrepareMessage(PANA_Message &msg)
{
    char buf[32];
    sprintf(buf, "%s:%d", PANA_CFG_GENERAL().m_ListenAddress.data(),
            PANA_CFG_GENERAL().m_ListenPort);
    msg.srcAddress().set(buf);
    msg.destAddress() = this->PacAddress();
}

