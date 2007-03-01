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
}

void PANA_Client::NotifyEapRestart()
{
    m_Timer.CancelSession();
    m_Event.EapStart();
    m_SA.Reset();
}

void PANA_Client::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_PacAddress.Set(PacAddress());
    args.m_PaaAddress.Set(PaaAddress());

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

void PANA_Client::RxPSR()
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
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPSR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    // RtxTimerStop()
    m_Timer.CancelTxRetry();

    // start eap
    NotifyEapRestart();

    // PSR.exist_avp("EAP-Payload")
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
       NotifyEapRequest(*payload);
       m_Timer.ScheduleEapResponse();
    }
    else {
       TxPSA(false);
    }
}

void PANA_Client::TxPSA(bool eapOptimization)
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

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    if (eapOptimization) {
        // add eap payload
        PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
        PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
        eapAvp.Get().assign(guard()->base(), guard()->size());
        msg->avpList().add(eapAvp());
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPSA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
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

    AAA_LOG((LM_INFO, "(%P|%t) TxPCI: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::RxPAR()
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    // Stop any RtxTimerStop()
    m_Timer.CancelTxRetry();

    // update paa nonce
    PANA_StringAvpContainerWidget nonceAvp(msg.avpList());
    pana_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && ! SecurityAssociation().PaaNonce().IsSet()) {
        SecurityAssociation().PaaNonce().Set(*nonce);
    }

    // EAP piggyback check
    if (! PANA_CFG_PAC().m_EapPiggyback) {
        TxPAN(false);
    }

    // < EAP-Payload >
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
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
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();
    m_Timer.CancelSession();
}

void PANA_Client::TxPAR()
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

    // stop eap response timer
    m_Timer.CancelEapResponse();

    // eap payload
    PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
    PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
    eapAvp.Get().assign(guard()->base(), guard()->length());
    msg->avpList().add(eapAvp());

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::TxPAN(bool eapPiggyBack)
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

    // add pac nonce
    if (! SecurityAssociation().PacNonce().IsSet()) {
        SecurityAssociation().PacNonce().Generate();

        pana_octetstring_t &nonce = SecurityAssociation().PacNonce().Get();
        PANA_StringAvpWidget nonceAvp(PANA_AVPNAME_NONCE);
        nonceAvp.Get().assign(nonce.data(), nonce.size());
        msg->avpList().add(nonceAvp());
    }

    if (eapPiggyBack) {
       // stop eap response timer
       m_Timer.CancelEapResponse();

       // eap payload
       PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
       PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
       eapAvp.Get().assign(guard()->base(), guard()->length());
       msg->avpList().add(eapAvp());
    }

    // add SA if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::RxPBR()
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPBR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    // lookup result code
    PANA_UInt32AvpContainerWidget rcodeAvp(msg.avpList());
    pana_unsigned32_t *pRcode = rcodeAvp.GetAvp(PANA_AVPNAME_RESULTCODE);
    if (pRcode == NULL) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No Result-Code AVP in PBR message"));
    }
    pana_unsigned32_t rcode = ACE_NTOHL(*pRcode);

    // update session lifetime
    PANA_UInt32AvpContainerWidget slAvp(msg.avpList());
    pana_unsigned32_t *sl = slAvp.GetAvp(PANA_AVPNAME_SESSIONLIFETIME);
    if (sl) {
        SessionLifetime() = ACE_NTOHL(*sl);
    }

    // extract key id if any
    PANA_UInt32AvpContainerWidget keyIdAvp(msg.avpList());
    pana_unsigned32_t *pKeyId = rcodeAvp.GetAvp(PANA_AVPNAME_KEYID);
    if (pKeyId) {
        SecurityAssociation().MSK().Id() = ACE_NTOHL(*pKeyId);
    }

    // extract eap
    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapRequest(*payload);
    }
    else if (rcode != PANA_RCODE_SUCCESS) {
        m_Event.EapAltReject();
        Error(rcode);
    }
    else {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No EAP-Payload on PANA_SUCCESS result code"));
    }
}

void PANA_Client::TxPBA(bool authSuccess)
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
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PBA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    if (authSuccess) {
        pana_octetstring_t newKey;
        if (m_Event.IsKeyAvailable(newKey)) {
            SecurityAssociation().MSK().Set(newKey);
            SecurityAssociation().GenerateAuthKey(this->SessionId());
        }
    }

    // auth and key-id
    if (SecurityAssociation().MSK().IsSet()) {
        if (SecurityAssociation().MSK().Id() != 0) {
            SecurityAssociation().AddKeyIdAvp(*msg);
        }
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPBA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::TxPRR()
{
    /*
      7.6.  PANA-Reauth-Request (PRR)

         The PANA-Reauth-Request (PRR) message is sent by the PaC to the PAA
         to re-initiate EAP authentication.

         PANA-Reauth-Request ::= < PANA-Header: 4, REQ >
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PRR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // auth avp
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPRR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::RxPRA()
{
    /*
      7.7.  PANA-Reauth-Answer (PRA)

         The PANA-Reauth-Answer (PRA) message is sent by the PAA to the PaC in
         response to a PANA-Reauth-Request message.

         PANA-Reauth-Answer ::= < PANA-Header: 4 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPRA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();

    // re-start eap for new conversation
    NotifyEapRestart();
}

void PANA_Client::TxPrepareMessage(PANA_Message &msg)
{
    msg.srcAddress() = this->PacAddress();
    msg.destAddress() = this->PaaAddress();
}

