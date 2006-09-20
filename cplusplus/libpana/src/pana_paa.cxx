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
#include "pana_cookie.h"
#include "pana_sid_generator.h"
#include "pana_pmk_bootstrap.h"


PANA_Paa::PANA_Paa(PANA_SessionTxInterface &tp,
                   PANA_SessionTimerInterface &tm,
                   PANA_PaaEventInterface &ev) :
    PANA_Session(tp, tm, ev) 
{ 
    // resolve Pac device id
    PANA_DeviceIdContainer &localAddrs = m_TxChannel.GetLocalAddress();
    PANA_DeviceId *id = localAddrs.search(AAA_ADDR_FAMILY_802);
    if (id == NULL) {
       id = PANA_DeviceIdConverter::GetIpOnlyAddress(localAddrs,
                (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
       if (id) {
          PaaIpAddress().set_addr((void*)id->value.data(), 
              id->value.size(), 0);
       }
    }
    if (id) {
       PaaDeviceId() = *id;
    }
    else {
       throw (PANA_Exception(PANA_Exception::FAILED, 
                            "No device ID available"));
    }

    // ------------------------------
    // State: OFFLINE (Initial State)
    // ------------------------------
    //
    // Initialization Action:
    //
    // USE_COOKIE=Set|Unset;
    // EAP_PIGGYBACK=Set|Unset;
    // CARRY_LIFETIME=Set|Unset;
    // CARRY_DEVICE_ID=Set|Unset;
    // CARRY_NAP_INFO=Set|Unset;
    // CARRY_ISP_INFO=Set|Unset;
    // CARRY_PPAC=Set|Unset;
    // PROTECTION_CAP_IN_PSR=Set|Unset;
    // PROTECTION_CAP_IN_PBR=Set|Unset;
    // if (PROTECTION_CAP_IN_PBR=Unset)
    //   PROTECTION_CAP_IN_PSR=Unset;
    // else
    //   CARRY_DEVICE_ID=Set;
    // RTX_COUNTER=0;
    // FIRST_AUTH_EXCHG=Set;
    // STATELESS_HANDSHAKE=Set|Unset;
    // RtxTimerStop();
    Reset();

    // generate a new session id 
    pana_octetstring_t sid;
    PANA_SESSIONID_GENERATOR().Generate(sid);
    SessionId().assign(sid.data(), sid.size());

    // Local reset
    PreferedNAP().m_Id = PANA_CFG_PAA().m_NapInfo.m_Id;
    PreferedNAP().m_Name = PANA_CFG_PAA().m_NapInfo.m_Name;

    m_Flags.p = 0;
    m_Flags.i.CarryPcapInPSR = PANA_CARRY_PCAP_IN_PSR;
    m_Flags.i.CarryAlgoInPSR = PANA_CARRY_ALGO_IN_PSR;
    m_Flags.i.CarryPcapInPBR = (PANA_CFG_GENERAL().m_ProtectionCap >= 0) ?
                                  true : PANA_CARRY_PCAP_IN_PBR;
    if (! m_Flags.i.CarryPcapInPBR) {
        m_Flags.i.CarryPcapInPSR = false;
        m_Flags.i.CarryAlgoInPSR = false;
    }
    else {
        AuxVariables().CarryDeviceId() = true;
    }
}

void PANA_Paa::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_Pac.Set(PacDeviceId());
    args.m_Paa.Set(PaaDeviceId());

    if (SecurityAssociation().IsSet()) {
        args.m_Key.Set(SecurityAssociation().Get());
        if (PANA_CFG_GENERAL().m_WPASupport &&
            (PANA_CFG_PAA().m_EpIdList.size() > 0)) {
            PANA_DeviceIdIterator i;
            for (i = PANA_CFG_PAA().m_EpIdList.begin(); i != 
                 PANA_CFG_PAA().m_EpIdList.end(); i ++) {
                PANA_DeviceId *epId = (*i);
                PANA_PMKKey pmk(SecurityAssociation().Get(),
                                PacDeviceId().value,
                                epId->value);
                args.m_PMKKeyList().push_back(pmk.Key());
            }
            if (PANA_CFG_PAA().m_EpIdList.size() > 0) {
                args.m_PMKKeyList.IsSet() = true;
            }
        }
    }

    args.m_Lifetime.Set(SessionLifetime());
    args.m_ProtectionCapability.Set(ProtectionCapability());
    args.m_Ep.Set(&PANA_CFG_PAA().m_EpIdList);
    args.m_PreferedISP.Set(PreferedISP());
    args.m_PreferedNAP.Set(PreferedNAP());

    m_Event.Authorize(args);
}

void PANA_Paa::NotifyEapRestart()
{
    m_Event.EapStart();
}

void PANA_Paa::NotifyEapResponse(pana_octetstring_t &payload)
{
    AAAMessageBlock *block = AAAMessageBlock::Acquire(payload.size());
    if (block) {
        block->copy((char*)payload.data(), payload.size());
        block->wr_ptr(block->base());
        static_cast<PANA_PaaEventInterface&>
                     (m_Event).EapResponse(block, false));
        block->Release();
    }
}

void PANA_Paa::NotifyEapTimeout()
{
    TxPER(PANA_UNABLE_TO_COMPLY);
    Disconnect(PANA_UNABLE_TO_COMPLY);
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
    7.2  PANA-Start-Request (PSR)

     The PANA-Start-Request (PSR) message is sent by the PAA to the PaC to
     advertise availability of the PAA and start PANA authentication.  The
     PAA sets the sequence number to an initial random value.

      PANA-Start-Request ::= < PANA-Header: 2, REQ [,SEP] >
                             [ Nonce ]
                             [ Cookie ]
                             [ EAP-Payload ]
                             [ NAP-Information ]
                          *  [ ISP-Information ]
                             [ Algorithm ]
                             [ Protection-Capability]
                             [ PPAC ]
                             [ Notification ]
                          *  [ AVP ]
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->flags().request = true;
    msg->flags().stateless = false; // This TxPSR is for stateful handshake only

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // add paa nonce
    SecurityAssociation().PaaNonce().Generate();
    pana_octetstring_t &nonce = SecurityAssociation().PaaNonce().Get();
    PANA_StringAvpWidget nonceAvp(PANA_AVPNAME_NONCE);
    nonceAvp.Get().assign(nonce.data(), nonce.size());
    msg->avpList().add(nonceAvp());

    // add eap payload
    if (! AuxVariables().TxEapMessageQueue().Empty()) {
        PANA_MsgBlockGuard eapPkt(AuxVariables().TxEapMessageQueue().Dequeue());
        PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
        eapAvp.Get().assign(eapPkt()->base(), eapPkt()->size());
        msg->avpList().add(eapAvp());
    }

    // add ppac
    if (PANA_CFG_GENERAL().m_PPAC() > 0) {
        PANA_UInt32AvpWidget ppacAvp(PANA_AVPNAME_PPAC);
        ppacAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_PPAC());
        msg->avpList().add(ppacAvp());
    }

    // add protection capability
    if (SupportFlags().i.CarryPcapInPSR) {
        PANA_UInt32AvpWidget pcapAvp(PANA_AVPNAME_PROTECTIONCAP);
        pcapAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_ProtectionCap);
        msg->avpList().add(pcapAvp());
    }

    // add algorithm
    if (m_Flags.i.CarryAlgoInPSR) {
        PANA_UInt32AvpWidget algoAvp(PANA_AVPNAME_ALGORITHM);
        algoAvp.Get() = ACE_HTONL(PANA_AUTH_ALGORITHM());
        msg->avpList().add(algoAvp());
    }

    // add nap information
    if (PANA_CFG_PAA().m_NapInfo.m_Id > 0) {
        PANA_GroupedAvpWidget napAvp(PANA_AVPNAME_NAPINFO);
        PANA_ProviderInfoTool infoTool;
        infoTool.Add(napAvp.Get(), PreferedNAP());
        msg->avpList().add(napAvp());
    }

    // add ISP information
    if (! PANA_CFG_PAA().m_IspInfo.empty()) {
        PANA_CfgProviderList::iterator i;
        PANA_GroupedAvpWidget ispAvp(PANA_AVPNAME_ISPINFO);
        for (i = PANA_CFG_PAA().m_IspInfo.begin();
             i != PANA_CFG_PAA().m_IspInfo.end();
             i++) {
            PANA_ProviderInfoTool infoTool;
            infoTool.Add(ispAvp.Get(), *(*i));
        }
        msg->avpList().add(ispAvp());
    }

    // add notification if any
    AddNotification(*msg);

    AAA_LOG((LM_INFO, "(%P|%t) TxPSR: L-flag=%d seq=%d\n",
               msg->flags().stateless, msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::RxPSA()
{
    /*
     7.3  PANA-Start-Answer (PSA)

      The PANA-Start-Answer (PSA) message is sent by the PaC to the PAA in
      response to a PANA-Start-Request message.  This message completes the
      handshake to start PANA authentication.

      PANA-Start-Answer ::= < PANA-Header: 2 [,SEP] >
                            [ Nonce ]
                            [ Cookie ]
                            [ EAP-Payload ]
                            [ ISP-Information ]
                            [ Notification ]
                         *  [ AVP ]
      */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPSA: L-flag=%d seq=%d\n",
               msg->flags().stateless, msg->seq()));

    // update pac nonce
    PANA_StringAvpContainerWidget nonceAvp(msg.avpList());
    pana_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && 
        ! SecurityAssociation().PacNonce().IsSet()) {
        SecurityAssociation().PacNonce().Set(*nonce);
    }

    // process notification
    ProcessNotification(msg);

    m_Timer.CancelTxRetry();

    // update ISP info
    PANA_GroupedAvpContainerWidget ispAvp(msg.avpList());
    pana_grouped_t *isp = ispAvp.GetAvp(PANA_AVPNAME_ISPINFO);
    if (isp) {
        PANA_ProviderInfoTool infoTool;
        infoTool.Extract(*isp, PreferedISP());
        AAA_LOG((LM_INFO, "(%P|%t) ISP INFO: id=%d, name=%s\n",
                   PreferedISP().m_Id, PreferedISP().m_Name.data()));
    }

    PANA_Utf8AvpContainerWidget sidAvp(msg.avpList());
    pana_utf8string_t *sid = sidAvp.GetAvp(PANA_AVPNAME_SESSIONID);
    if (sid && PANA_CFG_GENERAL().m_MobilityEnabled) {
        // mobility support
        AuxVariables().SecAssociationResumed() = true;
        TxPBR(PANA_SUCCESS, EAP_SUCCESS);
    }
    else {
        PANA_StringAvpContainerWidget eapAvp(msg.avpList());
        pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
        if (payload) {
            NotifyEapResponse(*payload);
        }
        else {
            m_Event.EapStart();
        }
    }
}

void PANA_Paa::TxPAR()
{
    /*
     7.4  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.  Its main task is to carry an EAP-Payload AVP.

       PANA-Auth-Request ::= < PANA-Header: 3, REQ [,SEP] [,NAP] >
                             < Session-Id >
                             < EAP-Payload >
                             [ Nonce ]
                             [ Notification ]
                          *  [ AVP ]
                          0*1 < AUTH >
     */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // add session id
    PANA_Utf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

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

    // add notification if any
    AddNotification(*msg);

    // auth avp if any
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR: L-flag=%d seq=%d\n",
               msg->flags().stateless, msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::TxPBR(pana_unsigned32_t rcode,
                     EAP_EVENT ev)
{
    /*
     7.8  PANA-Bind-Request (PBR)

       The PANA-Bind-Request (PBR) message is sent by the PAA to the PaC to
       deliver the result of PANA authentication.

       PANA-Bind-Request ::= < PANA-Header: 5, REQ [,SEP] [,NAP] >
                             < Session-Id >
                             { Result-Code }
                             [ PPAC ]
                             [ EAP-Payload ]
                             [ Session-Lifetime ]
                             [ Protection-Capability ]
                             [ Algorith ]
                             [ Key-Id ]
                          *  [ Device-Id ]
                             [ Notification ]
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

    // add session id
    PANA_Utf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // add result-code
    PANA_UInt32AvpWidget rcodeAvp(PANA_AVPNAME_RESULTCODE);
    rcodeAvp.Get() = ACE_HTONL(rcode);
    msg->avpList().add(rcodeAvp());

    // add ppac
    if (PANA_CFG_GENERAL().m_PPAC() > 0) {
        PANA_UInt32AvpWidget ppacAvp(PANA_AVPNAME_PPAC);
        ppacAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_PPAC());
        msg->avpList().add(ppacAvp());
    }

    bool AddBindingAvp = false;

    if (AuxVariables().SecAssociationResumed()) {
        AddBindingAvp = true;
    }
    else {
        PANA_MsgBlockGuard eapPkt(AuxVariables().TxEapMessageQueue().Dequeue());
        if (eapPkt() && (ev != EAP_TIMEOUT)) {
            // add eap payload
            PANA_StringAvpWidget eapAvp(PANA_AVPNAME_EAP);
            eapAvp.Get().assign(eapPkt()->base(), eapPkt()->size());
            msg->avpList().add(eapAvp());
        }

        if (ev == EAP_SUCCESS) {
	    AddBindingAvp = AuxVariables().Authorized() ? true : false;
        }
    }

    if (AddBindingAvp) {
        if (SessionLifetime() > 0) {
            // add session lifetime
            PANA_UInt32AvpWidget lifetimeAvp(PANA_AVPNAME_SESSIONLIFETIME);
            lifetimeAvp.Get() = ACE_HTONL(SessionLifetime());
            msg->avpList().add(lifetimeAvp());
        }

        // add protection capability
        if (SupportFlags().i.CarryPcapInPBR) {
            PANA_UInt32AvpWidget pcapAvp(PANA_AVPNAME_PROTECTIONCAP);
            pcapAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_ProtectionCap);
            msg->avpList().add(pcapAvp());
        }

        // add EP device id
        if (AuxVariables().CarryDeviceId()) {
            PANA_DeviceIdIterator i = PANA_CFG_PAA().m_EpIdList.begin();
            PANA_AddressAvpWidget deviceAvp(PANA_AVPNAME_DEVICEID);
            for (; i != PANA_CFG_PAA().m_EpIdList.end(); i++) {
                deviceAvp.Get() = **i;
            }
            msg->avpList().add(deviceAvp());
        }
    }

    // update the aaa key's
    if ((ev == EAP_SUCCESS) && (! AuxVariables().SecAssociationResumed())) {
        pana_octetstring_t newKey;
        if (m_Event.IsKeyAvailable(newKey)) {
            SecurityAssociation().UpdateAAAKey(newKey);
            if (! AuxVariables().AlgorithmIsSet()) {
                // add algorithm
                // TBD: need to make sure algo value is ok
                PANA_UInt32AvpWidget algoAvp(PANA_AVPNAME_ALGORITHM);
                algoAvp.Get() = ACE_HTONL(PANA_AUTH_ALGORITHM());
                msg->avpList().add(algoAvp());
                AuxVariables().AlgorithmIsSet() = true;
            }
            SecurityAssociation().AddKeyIdAvp(*msg);
            SecurityAssociation().GenerateAuthKey(SessionId());
        }
    }

    // add notification if any
    AddNotification(*msg);

    // add key from existing SA
    if (SecurityAssociation().IsSet()) {
        // key id avp if any
        if (AuxVariables().SecAssociationResumed()) {
            SecurityAssociation().AddKeyIdAvp(*msg);
        }

        // add existing auth first before generating a new one
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPBR: seq=%d\n", msg->seq()));

    SendReqMsg(msg);
}

void PANA_Paa::RxPBA(bool success)
{
    /*
      7.9  PANA-Bind-Answer (PBA)

        The PANA-Bind-Answer (PBA) message is sent by the PaC to the PAA in
        response to a PANA-Bind-Request message.

        PANA-Bind-Answer ::= < PANA-Header: 5 [,SEP] [,NAP] >
                             < Session-Id >
                             [ PPAC ]
                             [ Device-Id ]
                             [ Key-Id ]
                             [ Notification ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPBA: seq=%d\n", msg.seq()));

    // process notification
    ProcessNotification(msg);

    if (success) {

        m_Timer.CancelTxRetry();

        // save the device id
        PANA_AddressAvpContainerWidget pacIdAvp(msg.avpList());
        pana_address_t *pacId = pacIdAvp.GetAvp(PANA_AVPNAME_DEVICEID);
        if (pacId) {
            PacDeviceId() = *pacId;
        }

        NotifyScheduleLifetime(PANA_CFG_PAA().m_GracePeriod);
        NotifyAuthorization();
    }
    else {
        Disconnect();
    }
}

void PANA_Paa::TxPAN()
{
    /*
     7.5  PANA-Auth-Answer (PAN)

       THe PANA-Auth-Answer (PAN) message is sent by either the PaC or the
       PAA in response to a PANA-Auth-Request message.  It MAY carry an EAP-
       Payload AVP.

        PANA-Auth-Answer ::= < PANA-Header: 3 [,SEP] [,NAP] >
                             < Session-Id >
                             [ Nonce ]
                             [ EAP-Payload ]
                             [ Notification ]
                          *  [ AVP ]
                         0*1 < AUTH >
    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAN;
    msg->seq() = LastRxSeqNum().Value();

    // add session id
    PANA_Utf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // add notification if any
    AddNotification(*msg);

    // add SA if any
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPAN: L-flag %d, seq=%d\n",
               msg->flags().stateless, msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::RxPAR()
{
    /*
     7.4  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.  Its main task is to carry an EAP-Payload AVP.

       PANA-Auth-Request ::= < PANA-Header: 3, REQ [,SEP] [,NAP] >
                             < Session-Id >
                             < EAP-Payload >
                             [ Nonce ]
                             [ Notification ]
                          *  [ AVP ]
                          0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAR: L-flag %d, seq=%d\n",
               msg->flags().stateless, msg->seq()));

    // process notification
    ProcessNotification(msg);

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
     7.5  PANA-Auth-Answer (PAN)

       THe PANA-Auth-Answer (PAN) message is sent by either the PaC or the
       PAA in response to a PANA-Auth-Request message.  It MAY carry an EAP-
       Payload AVP.

        PANA-Auth-Answer ::= < PANA-Header: 3 [,SEP] [,NAP] >
                             < Session-Id >
                             [ Nonce ]
                             [ EAP-Payload ]
                             [ Notification ]
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPAN: L-flag %d, seq=%d\n",
               msg->flags().stateless, msg->seq()));

    // process notification
    ProcessNotification(msg);

    m_Timer.CancelTxRetry();

    // update pac nonce
    PANA_StringAvpContainerWidget nonceAvp(msg.avpList());
    pana_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && 
        ! SecurityAssociation().PacNonce().IsSet()) {
        SecurityAssociation().PacNonce().Set(*nonce);
    }

    PANA_StringAvpContainerWidget eapAvp(msg.avpList());
    pana_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapResponse(*payload);
    }
}

void PANA_Paa::RxPRAR()
{
    /*
      7.6  PANA-Reauth-Request (PRAR)

       The PANA-Reauth-Request (PRAR) message is sent by the PaC to the PAA
       to re-initiate EAP authentication.

        PANA-Reauth-Request ::= < PANA-Header: 4, REQ >
                                < Session-Id >
                                [ Notification ]
                             *  [ AVP ]
                            0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPRAR: seq=%d\n", msg.seq()));

    // process notification
    ProcessNotification(msg);

    NotifyEapReAuth();
    TxPRAA();
}

void PANA_Paa::TxPRAA()
{
    /*
      7.7  PANA-Reauth-Answer (PRAA)

       The PANA-Reauth-Answer (PRAA) message is sent by the PAA to the PaC
       in response to a PANA-Reauth-Request message.

        PANA-Reauth-Answer ::= < PANA-Header: 4 >
                               < Session-Id >
                               [ Notification ]
                            *  [ AVP ]
                           0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PRAA;
    msg->seq() = LastRxSeqNum().Value();

    // add session id
    PANA_Utf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // add notification if any       
    AddNotification(*msg);

    // auth avp
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPRAA: seq=%d\n", msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Paa::TxFormatAddress(PANA_Message &msg)
{
    msg.srcPort() = PacIpAddress().get_port_number();
    PANA_DeviceId *ipId = PANA_DeviceIdConverter::CreateFromAddr
        (PacIpAddress());
    if (ipId == NULL) {
        throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                    "Failed to allocate IP device id"));
    }
    msg.srcDevices().push_back(ipId);
}

