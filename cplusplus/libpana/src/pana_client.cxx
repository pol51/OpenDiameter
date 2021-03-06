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

#include "pana_client.h"
#include "pana_config_manager.h"
#include "pana_pmk_bootstrap.h"

PANA_Client::PANA_Client(PANA_SessionTxInterface &tp,
                         PANA_SessionTimerInterface &tm,
                         PANA_ClientEventInterface &ev) :
    PANA_Session(tp, tm, ev)
{
    // ------------------------------
    // State: OFFLINE (Initial State)
    // ------------------------------
    //
    // Initialization Action:
    //
    //   FIRST_AUTH_EXCHG=Set;
    //   RTX_COUNTER=0;
    //   RtxTimerStop();
    Reset();
    m_LastUsedChannel.set((u_short)0);
}

void PANA_Client::NotifyEapRestart()
{
    SecurityAssociation().PaaNonce().Reset();
    SecurityAssociation().PacNonce().Reset();

    m_Timer.CancelSession();
    m_Event.EapStart();
    m_SA.Reset();
}

void PANA_Client::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_PacAddress.Set(PacAddress());
    args.m_PaaAddress.Set(PaaAddress());
    args.m_IpReconfiguration = IpReconfiguration();

    if (SecurityAssociation().MSK().IsSet()) {
        args.m_Key.Set(SecurityAssociation().MSK().Get());
    }

    args.m_Lifetime.Set(SessionLifetime());

    m_Event.Authorize(args);
    AuxVariables().Authorized() = true; // PaC always authorized
}

void PANA_Client::NotifyEapRequest(pana_octetstring_t &payload)
{
    AAAMessageBlock *block = AAAMessageBlock::Acquire(payload.size());
    if (block) {
        block->copy((char*)payload.data(), payload.size());
        block->wr_ptr(block->base());
        block->rd_ptr(block->base());
        static_cast<PANA_ClientEventInterface&>(m_Event).EapRequest(block);
    }
}

void PANA_Client::RxPARStart()
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
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR-Start: id=%u seq=%u\n",
            msg.sessionId(), msg.seq()));

    // RtxTimerStop()
    m_Timer.CancelTxRetry();

    // start eap
    NotifyEapRestart();

    // start session timer to detect stalled sessions
    NotifyScheduleLifetime();

    // PSR.exist_avp("EAP-Payload")
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp((char *)PANA_AVPNAME_EAP);
    if (payload) {
       NotifyEapRequest(*payload);
       m_Timer.ScheduleEapResponse();
       if (! PANA_CFG_PAC().m_EapPiggyback) {
          TxPANStart(false);
       }
    }
    else {
       TxPANStart(false);
    }

    // Save the PAR-Start for AUTH generation
    PANA_MsgByteStream byteConverter;
    PANA_MessageBuffer *buffer = byteConverter.Get(msg);
    SecurityAssociation().PARStart().assign(buffer->base(), msg.length());
}

void PANA_Client::TxPANStart(bool eapOptimization)
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
    msg->flags().start = true;
    msg->seq() = LastRxSeqNum();
    msg->sessionId() = this->SessionId();

    if (eapOptimization) {
        // add eap payload
        PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
        PANA_StringAvpWidget eapAvp((char *)PANA_AVPNAME_EAP);
        eapAvp.Get().assign(guard()->base(), guard()->size());
        msg->avpList().add(eapAvp());
    }

    // add integrity algorithm
    PANA_UInt32AvpWidget integrityAlgoAvp((char *)PANA_AVPNAME_INTEGRITY_ALGO);
    integrityAlgoAvp.Get() = PANA_AUTH_HMAC_SHA1_160;
    msg->avpList().add(integrityAlgoAvp());

    // add prf algorithm
    PANA_UInt32AvpWidget prfAlgoAvp((char *)PANA_AVPNAME_PRF_ALGO);
    prfAlgoAvp.Get() = PANA_PRF_HMAC_SHA1;
    msg->avpList().add(prfAlgoAvp());

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN-Start: id=%u seq=%u\n",
            msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);

    // Save the PAN-Start for AUTH generation
    PANA_MsgByteStream byteConverter;
    PANA_MessageBuffer *buffer = byteConverter.Get(*msg);
    SecurityAssociation().PANStart().assign(buffer->base(), msg->length());
}

void PANA_Client::TxPCI()
{
    /*
      7.1.  PANA-Client-Initiation (PCI)

         The PANA-Client-Initiation (PCI) message is used for PaC-initiated
         handshake.  The Sequence Number and Session Identifier fields in this
         message MUST be set to zero (0).

         PANA-Client-Initiation ::= < PANA-Header: 1 >
                          *  [ AVP ]
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PCI;
    msg->seq() = 0;
    this->SessionId() = 0;

    AAA_LOG((LM_INFO, "(%P|%t) TxPCI: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::RxPAR()
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));


    // Stop any RtxTimerStop()
    m_Timer.CancelTxRetry();

    // update paa nonce
    PANA_StringAvpContainerWidget nonceAvp(msg.avpList());
    pana_octetstring_t *nonce = nonceAvp.GetAvp((char *)PANA_AVPNAME_NONCE);
    if (nonce && ! SecurityAssociation().PaaNonce().IsSet()) {
        SecurityAssociation().PaaNonce().Set(*nonce);
    }

    // EAP piggyback check
    if (! PANA_CFG_PAC().m_EapPiggyback) {
        TxPAN(false);
    }

    // < EAP-Payload >
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp((char *)PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapRequest(*payload);

        // EAP response timeout should be less than retry
        m_Timer.ScheduleEapResponse();
    }
    else {
        throw (PANA_Exception(PANA_Exception::FAILED,
               "No EAP-Payload AVP in PAR message"));
    }
}

void PANA_Client::RxPAN()
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

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN: id=%u seq=%u\n",
            msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();
}

void PANA_Client::TxPAR()
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

    // stop eap response timer
    m_Timer.CancelEapResponse();

    // eap payload
    PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
    PANA_StringAvpWidget eapAvp((char *)PANA_AVPNAME_EAP);
    eapAvp.Get().assign(guard()->base(), guard()->length());
    msg->avpList().add(eapAvp());

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR: id=%u seq=%u\n",
            msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::TxPAN(bool eapPiggyBack)
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

    // add pac nonce
    if (! SecurityAssociation().PacNonce().IsSet()) {
        SecurityAssociation().PacNonce().Generate();

        pana_octetstring_t &nonce = SecurityAssociation().PacNonce().Get();
        PANA_StringAvpWidget nonceAvp((char *)PANA_AVPNAME_NONCE);
        nonceAvp.Get().assign(nonce.data(), nonce.size());
        msg->avpList().add(nonceAvp());
    }

    if (eapPiggyBack) {
       // stop eap response timer
       m_Timer.CancelEapResponse();

       // eap payload
       PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
       PANA_StringAvpWidget eapAvp((char *)PANA_AVPNAME_EAP);
       eapAvp.Get().assign(guard()->base(), guard()->length());
       msg->avpList().add(eapAvp());
    }

    // add SA if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN: id=%u seq=%u\n",
            msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::RxPARComplete()
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR-Complete: id=%u seq=%u\n",
             msg.sessionId(), msg.seq()));

    // lookup result code
    PANA_UInt32AvpContainerWidget rcodeAvp(msg.avpList());
    pana_unsigned32_t *pRcode = rcodeAvp.GetAvp((char *)PANA_AVPNAME_RESULTCODE);
    if (pRcode == NULL) {
        throw (PANA_Exception(PANA_Exception::FAILED,
               "No Result-Code AVP in PNR message"));
    }
    pana_unsigned32_t rcode = *pRcode;

    // set ip reconfiguration variable if any
    IpReconfiguration() = (msg.flags().ipreconfig) ? true : false;

    // extract eap
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp((char *)PANA_AVPNAME_EAP);
    if (payload) {
        // update session lifetime
        PANA_UInt32AvpContainerWidget slAvp(msg.avpList());
        pana_unsigned32_t *sl = slAvp.GetAvp((char *)PANA_AVPNAME_SESSIONLIFETIME);
        if (sl) {
            SessionLifetime() = *sl;
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t) Session lifetime is not annouced by the PAA, using default: [%d] sec\n",
                     SessionLifetime()));
        }

        // extract key id if any
        PANA_UInt32AvpContainerWidget keyIdAvp(msg.avpList());
        pana_unsigned32_t *pKeyId = rcodeAvp.GetAvp((char *)PANA_AVPNAME_KEYID);
        if (pKeyId) {
            SecurityAssociation().MSK().Id() = *pKeyId;
        }

        NotifyEapRequest(*payload);
    }
    else if (rcode != PANA_RCODE_SUCCESS) {
        m_Event.EapAltReject();
    }
    else {
        throw (PANA_Exception(PANA_Exception::FAILED,
               "No EAP-Payload on PANA_SUCCESS result code"));
    }
}

void PANA_Client::TxPANComplete(bool authSuccess)
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
    msg->flags().complete = true;
    msg->seq() = LastRxSeqNum();
    msg->sessionId() = this->SessionId();

    if (authSuccess) {
        pana_octetstring_t newKey;
        if (m_Event.IsKeyAvailable(newKey)) {
            SecurityAssociation().MSK().Set(newKey);
            SecurityAssociation().GenerateAuthKey();
        }
    }

    // auth and key-id
    if (SecurityAssociation().MSK().IsSet()) {
        if (SecurityAssociation().MSK().Id() != 0) {
            SecurityAssociation().AddKeyIdAvp(*msg);
        }
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN-Complete: id=%u seq=%u\n",
            msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::TxPNRAuth()
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

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PNR;
    msg->flags().request = true;
    msg->flags().auth = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    msg->seq() = ++ LastTxSeqNum();

    // auth avp
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    // cancel current session timer
    m_Timer.CancelSession();

    AAA_LOG((LM_INFO, "(%P|%t) TxPNR-Auth: id=%u seq=%u\n",
            msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::RxPNAAuth()
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPNA-Auth: id=%u seq=%u\n",
            msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();

    // re-start eap for new conversation
    NotifyEapRestart();
}

void PANA_Client::TxPrepareMessage(PANA_Message &msg)
{
    msg.destAddress() = this->PaaAddress();

    if (msg.flags().request || (msg.type() == PANA_MTYPE_PCI)) {
        // request message
        msg.destAddress().set_port_number(PANA_CFG_PAC().m_PaaPortNumber);
        msg.srcAddress() = this->PacAddress();
    }
    else {
        // answer message
        msg.srcAddress() = this->LastUsedChannel();
    }
}

