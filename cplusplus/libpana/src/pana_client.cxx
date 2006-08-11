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

#include "pana_client.h"
#include "pana_device_id.h"
#include "pana_config_manager.h"
#include "pana_pmk_bootstrap.h"
#if defined(PANA_MPA_SUPPORT)
#include "pana_pac_ep_key.h"
#endif

PANA_Client::PANA_Client(PANA_SessionTxInterface &tp,
                         PANA_SessionTimerInterface &tm,
                         PANA_ClientEventInterface &ev) :
    PANA_Session(tp, tm, ev)
{
    //
    // ------------------------------
    // State: OFFLINE (Initial State)
    // ------------------------------
    //
    // Initialization Action:
    //
    //   SEPARATE=Set|Unset;
    //   CARRY_DEVICE_ID=Unset;
    //   1ST_EAP=Unset;
    //   RtxTimerStop();
    //
    // Mobility Initialization Action:
    //
    //   MOBILITY=Set|Unset
    //   PANA_SA_RESUMED=Unset;
    //
    Reset();

    // Local reset
    AuxVariables().CarryDeviceId() = false;
    m_Flags.p = 0;
}

void PANA_Client::LoadLocalAddress()
{
    // resolve Pac device id
    PANA_DeviceIdContainer &localAddrs = m_TxChannel.GetLocalAddress();
    PANA_DeviceId *id = localAddrs.search(AAA_ADDR_FAMILY_802);
    if (id == NULL) {
       id = PANA_DeviceIdConverter::GetIpOnlyAddress(localAddrs,
                (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
       if (id) {
          PANA_AddrConverter::ToAce(*id, PacIpAddress());
       }
    }
    if (id) {
       PacDeviceId() = *id;
    }
    else {
       throw (PANA_Exception(PANA_Exception::FAILED, 
                            "No device ID available"));
    }
}

void PANA_Client::NotifyEapRestart()
{
    bool napAuth = AuxVariables().NapAuthentication();
    m_Event.EapStart(napAuth);
}

void PANA_Client::NotifyAuthorization()
{
    PANA_SessionEventInterface::PANA_AuthorizationArgs args;

    args.m_Pac.Set(PacDeviceId());
    args.m_Paa.Set(PaaDeviceId());

#if defined(PANA_MPA_SUPPORT)
    PANA_DeviceIdContainer &localAddrs = m_TxChannel.GetLocalAddress();
    PANA_DeviceId *id = localAddrs.search(AAA_ADDR_FAMILY_IPV4);
    if (id != NULL) {
       args.m_PacIPaddr.Set(*id);
    }
    args.m_PaaIPaddr.Set(PaaDeviceId());
#endif

    if (SecurityAssociation().IsSet()) {
        args.m_Key.Set(SecurityAssociation().Get());
        // TBD: key id removed from args
        if (PPAC().DhcpV6() && DhcpBootstrap().Enable()) {
            diameter_octetstring_t dhcpKey;
            DhcpBootstrap().DhcpKey(SecurityAssociation().Get(),
                                    dhcpKey);
            args.m_DhcpKey.Set(dhcpKey);
        }
        if (PANA_CFG_GENERAL().m_WPASupport &&
            (EpDeviceIds().size() > 0)) {
            PANA_DeviceIdIterator i;
            for (i = EpDeviceIds().begin(); i != 
                 EpDeviceIds().end(); i ++) {
                PANA_DeviceId *epId = (*i);
                PANA_PMKKey pmk(SecurityAssociation().Get(),
                                PacDeviceId().value,
                                epId->value);
                args.m_PMKKeyList().push_back(pmk.Key());
#if defined(PANA_MPA_SUPPORT)
		PANA_PAC_EP_Key pac_epkey(SecurityAssociation().Get(),
                                SecurityAssociation().AAAKey2().Id(),
                                SessionId(),epId->value);
		args.m_PSKKeyList().push_back(pac_epkey.Key());
#endif
            }
            if (EpDeviceIds().size() > 0) {
                args.m_PMKKeyList.IsSet() = true;
#if defined(PANA_MPA_SUPPORT)
		args.m_PSKKeyList.IsSet() = true;
#endif
            }
        }
    }

    args.m_Lifetime.Set(SessionLifetime());
    args.m_ProtectionCapability.Set(ProtectionCapability());
    args.m_Ep.Set(&EpDeviceIds());
    args.m_PreferedISP.Set(PreferedISP());
    args.m_PreferedNAP.Set(PreferedNAP());

    m_Event.Authorize(args);
    AuxVariables().Authorized() = true; // PaC always authorized
}

bool PANA_Client::IsSessionResumed()
{
    if (SecurityAssociation().IsSet()) {
        return static_cast<PANA_ClientEventInterface&>(m_Event).
            ResumeSession();
    }
    return (false);
}

void PANA_Client::NotifyEapRequest(diameter_octetstring_t &payload)
{
    AAAMessageBlock *block = AAAMessageBlock::Acquire(payload.size());
    if (block) {
        block->copy((char*)payload.data(), payload.size());
        block->wr_ptr(block->base());
        static_cast<PANA_ClientEventInterface&>
                    (m_Event).EapRequest(block,
                     AuxVariables().NapAuthentication());    
        block->Release();
    }
}

void PANA_Client::IspSelection(PANA_Message *psr)
{
    // extract ISP info if any
    PANA_CfgProviderList ispList;
    PANA_CfgProviderInfo *ispInfo;
    PANA_CfgProviderInfo *ispChoice = NULL;
    DiameterGroupedAvpContainerWidget ispAvp(psr->avpList());
    diameter_grouped_t *isp = ispAvp.GetAvp(PANA_AVPNAME_ISPINFO);
    if (isp) {
        for (int ndx=1; isp; ndx++) {
            ispInfo = new PANA_CfgProviderInfo;
            if (ispInfo) {
                PANA_ProviderInfoTool infoTool;
                infoTool.Extract(*isp, *ispInfo);
                ispList.push_back(ispInfo);
            }
            isp = ispAvp.GetAvp(PANA_AVPNAME_ISPINFO, ndx);
        }

        // ask user to choose
        static_cast<PANA_ClientEventInterface&>(m_Event).
               ChooseISP(ispList, ispChoice);
    }

    // if none chosen, select from config file
    if ((ispChoice == NULL) &&
        (PANA_CFG_PAC().m_IspInfo.m_Name.size() > 0)) {
        ispChoice = &PANA_CFG_PAC().m_IspInfo;
    }

    if (ispChoice) {
        PreferedISP().m_Name = ispChoice->m_Name;
        PreferedISP().m_Id = ispChoice->m_Id;

        ACE_DEBUG((LM_INFO, "(%P|%t) Selected ISP: [id=%d] %s\n",
                 PreferedISP().m_Id, PreferedISP().m_Name.data()));
    }

    // cleanup
    while (! ispList.empty()) {
        ispInfo = ispList.front();
        ispList.pop_front();
        delete ispInfo;
    }
}

void PANA_Client::RxPSR()
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
                             [ Protection-Capability]
                             [ PPAC ]
                             [ Notification ]
                          *  [ AVP ]
    */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    if (PANA_CFG_PAC().m_PaaIpAddress.size() == 0) {
        PANA_DeviceId *id = PANA_DeviceIdConverter::GetIpOnlyAddress
                                (msg.srcDevices(), 
                                (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
        if (id) {
            PaaIpAddress().set_address(id->value.data(), 
                                        id->value.size(), 
                                        (bool)PANA_CFG_GENERAL().m_IPv6Enabled ?
				        0 : 1);
            PaaIpAddress().set_port_number(msg.srcPort());
        }
        else {
            throw (PANA_Exception(PANA_Exception::FAILED, 
                                    "No PAA device ID gathered"));
        }
        PaaDeviceId() = *id;
    }
    else {
        std::string paaIpStr = PANA_CFG_PAC().m_PaaIpAddress + ":0";
        PaaIpAddress().string_to_addr(paaIpStr.data());
        PANA_AddrConverter::ToAAAAddress(PaaIpAddress(), PaaDeviceId());
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) RxPSR: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // update paa nonce
    DiameterStringAvpContainerWidget nonceAvp(msg.avpList());
    diameter_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && ! SecurityAssociation().PaaNonce().IsSet()) {
        SecurityAssociation().PaaNonce().Set(*nonce);
    }
    
    // process notification
    ProcessNotification(msg);

    // update nap information
    DiameterGroupedAvpContainerWidget napAvp(msg.avpList());
    diameter_grouped_t *nap = napAvp.GetAvp(PANA_AVPNAME_NAPINFO);
    if (nap) {
       PANA_ProviderInfoTool infoTool;
       infoTool.Extract(*nap, PreferedNAP());
    }

    // update current authentication mode
    AuxVariables().NapAuthentication() = msg.flags().nap;

    // RtxTimerStop()
    m_Timer.CancelTxRetry();

    // start eap
    if (! AuxVariables().SecAssociationResumed()) {
       NotifyEapRestart();
    }

    // PSR.exist_avp("EAP-Payload")
    DiameterStringAvpContainerWidget eapAvp(msg.avpList());
    diameter_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {    
       AuxVariables().SeparateAuthentication() = false;
       AuxVariables().NapAuthentication() = false;
       
       // make sure isp is choosen
       IspSelection(&msg);
       
       // send to eap
       NotifyEapRequest(*payload);
       return;
    }

    /*
     Protection-Capability and Post-PANA-Address-Configuration AVPs MAY be
     optionally included in the PANA-Start-Request in order to indicate
     required and available capabilities for the network access.  These
     AVPs MAY be used by the PaC for assessing the capability match even
     before the authentication takes place.  But these AVPs are provided
     during the insecure discovery phase, there are certain security risks
     involved in using the provided information.  See Section 9 for
     further discussion on this.

     ...
     ...

     In networks where lower-layers are not secured prior to running PANA,
     the capability discovery enabled through inclusion of
     Protection-Capability and Post-PANA-Address-Configuration AVPs in a
     PANA-Start-Request message is susceptible to spoofing leading to
     denial-of service attacks.  Therefore, usage of these AVPs during the
     discovery and initial handshake phase in such insecure networks is
     NOT RECOMMENDED.  The same AVPs are delivered via an
     integrity-protected PANA-Bind-Request upon successful authentication.
     */
     TxPSA(cleanup.get());
}

void PANA_Client::TxPSA(PANA_Message *psr)
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

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->seq() = LastRxSeqNum().Value();

    // add pac nonce
    if (SecurityAssociation().PaaNonce().IsSet() &&
        ! SecurityAssociation().PacNonce().IsSet()) {
        // generate nouce
        SecurityAssociation().PacNonce().Generate();
        
        diameter_octetstring_t &nonce = SecurityAssociation().PacNonce().Get();
        DiameterStringAvpWidget nonceAvp(PANA_AVPNAME_NONCE);
        nonceAvp.Get().assign(nonce.data(), nonce.size());
        msg->avpList().add(nonceAvp());
    }
    
    if (psr == NULL) {
    
        // add eap payload
        PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
        DiameterStringAvpWidget eapAvp(PANA_AVPNAME_EAP);
        eapAvp.Get().assign(guard()->base(), guard()->size());
        msg->avpList().add(eapAvp());

        // add prefered isp information if any
        if (PreferedISP().m_Name.size() > 0) {
            DiameterGroupedAvpWidget choosenIsp(PANA_AVPNAME_ISPINFO);
            PANA_ProviderInfoTool infoTool;
            infoTool.Add(choosenIsp.Get(), PreferedISP());
            msg->avpList().add(choosenIsp());
        }
       
        // add notification if any       
        AddNotification(*msg);

        ACE_DEBUG((LM_INFO, "(%P|%t) TxPSA: S-flag %d, N-flag=%d, seq=%d\n",
                   msg->flags().separate, msg->flags().nap, msg->seq()));

        SendAnsMsg(msg);
        return;
    }

    DiameterStringAvpContainerWidget cookieAvp(psr->avpList());
    diameter_octetstring_t *cookie = cookieAvp.GetAvp(PANA_AVPNAME_COOKIE);
    if (cookie) {
        // add cookie
        DiameterStringAvpWidget psaCookieAvp(PANA_AVPNAME_COOKIE);
        psaCookieAvp.Get().assign(cookie->data(), cookie->size());
        msg->avpList().add(psaCookieAvp());
    }

    if (AuxVariables().SecAssociationResumed()) {

        // add session id
        AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
        sessionIdAvp.Get() = SessionId();
        msg->avpList().add(sessionIdAvp());

        // add auth (make sure this is the last 
        // process in this message)
        SecurityAssociation().AddAuthAvp(*msg);

        // set aux variable
        AuxVariables().SeparateAuthentication() = false;
    }
    else {
       if (AuxVariables().SeparateAuthentication() &&
             psr->flags().separate) {
           // set s-flag in psa
           msg->flags().separate = true;
           SecurityAssociation().Type() = PANA_SecurityAssociation::DOUBLE;
       }
       else {
           AuxVariables().SeparateAuthentication() = false;
       }
       
       // ISP selection
       IspSelection(psr);

       // add prefered isp information
       DiameterGroupedAvpWidget choosenIsp(PANA_AVPNAME_ISPINFO);
       PANA_ProviderInfoTool infoTool;
       infoTool.Add(choosenIsp.Get(), PreferedISP());
       msg->avpList().add(choosenIsp());
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPSA: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    // add notification if any       
    AddNotification(*msg);

    SendReqMsg(msg, (cookie) ? true : false);
}

void PANA_Client::TxPDI()
{
    /*
      7.1  PANA-PAA-Discover (PDI)

        The PANA-PAA-Discover (PDI) message is used to discover the address
        of PAA(s).  The sequence number in this message is always set to zero
        (0).

        PANA-PAA-Discover ::= < PANA-Header: 1 >
                              [ Notification ]
                           *  [ AVP ]

    */

    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PDI;
    msg->seq() = 0;

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPDI: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    // add notification if any       
    AddNotification(*msg);

    SendReqMsg(msg);
}

void PANA_Client::RxPAR(bool eapReAuth)
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

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    ACE_DEBUG((LM_INFO, "(%P|%t) RxPAR: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // process notification
    ProcessNotification(msg);

    AuxVariables().NapAuthentication() = msg.flags().nap;
    AuxVariables().SecAssociationResumed() = false;
    if (eapReAuth) {
        AuxVariables().FirstEapResult() = 0;
        AuxVariables().SeparateAuthentication() = 
            PANA_CFG_GENERAL().m_SeparateAuth;
        m_Timer.CancelSession();
    } 
    else {
        // RtxTimerStop()
        m_Timer.CancelTxRetry();
    }

    // update paa nonce
    DiameterStringAvpContainerWidget nonceAvp(msg.avpList());
    diameter_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
    if (nonce && ! SecurityAssociation().PaaNonce().IsSet()) {
        SecurityAssociation().PaaNonce().Set(*nonce);
    }
    
    // PAR.exist_avp("EAP-Payload")
    DiameterStringAvpContainerWidget eapAvp(msg.avpList());
    diameter_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapRequest(*payload);
        
        // EAP response timeout should be less than retry 
        m_Timer.ScheduleEapResponse(PANA_CFG_GENERAL().m_RT.m_IRT/2);
    }
    else {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No EAP-Payload AVP in PAR message"));
    }
    
    // EAP piggyback check
    if (! PANA_CFG_GENERAL().m_EapPiggyback) {
        TxPAN(false);
    }    
}

void PANA_Client::RxPAN()
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
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;
    
    ACE_DEBUG((LM_INFO, "(%P|%t) RxPAN: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // process notification
    ProcessNotification(msg);

    // update current authentication mode
    AuxVariables().NapAuthentication() = msg.flags().nap;
    
    m_Timer.CancelTxRetry();
    m_Timer.CancelSession();
}
    
void PANA_Client::TxPAR()
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
    msg->flags().separate = AuxVariables().SeparateAuthentication();
    msg->flags().nap = AuxVariables().NapAuthentication();

    // add session id
    AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // stop eap response timer
    m_Timer.CancelEapResponse();
       
    // eap payload
    PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
    DiameterStringAvpWidget eapAvp(PANA_AVPNAME_EAP);
    eapAvp.Get().assign(guard()->base(), guard()->length());
    msg->avpList().add(eapAvp());

    // add notification if any       
    AddNotification(*msg);

    // auth avp if any
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPAR: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::TxPAN(bool eapPiggyBack)
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
    msg->flags().separate = AuxVariables().SeparateAuthentication();
    msg->flags().nap = AuxVariables().NapAuthentication();

    // add session id
    AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // add pac nonce
    if (! SecurityAssociation().PacNonce().IsSet()) {
        // generate nouce
        SecurityAssociation().PacNonce().Generate();
        
        diameter_octetstring_t &nonce = SecurityAssociation().PacNonce().Get();
        DiameterStringAvpWidget nonceAvp(PANA_AVPNAME_NONCE);
        nonceAvp.Get().assign(nonce.data(), nonce.size());
        msg->avpList().add(nonceAvp());
    }
    
    if (eapPiggyBack) {
       // stop eap response timer
       m_Timer.CancelEapResponse();
       
       // eap payload
       PANA_MsgBlockGuard guard(AuxVariables().TxEapMessageQueue().Dequeue());
       DiameterStringAvpWidget eapAvp(PANA_AVPNAME_EAP);
       eapAvp.Get().assign(guard()->base(), guard()->length());
       msg->avpList().add(eapAvp());
    }

    // add notification if any       
    AddNotification(*msg);

    // add SA if any
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddKeyIdAvp(*msg);
        SecurityAssociation().AddAuthAvp(*msg);
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPAN: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::RxPFER()
{
    /*
     7.16  PANA-FirstAuth-End-Request (PFER)

      The PANA-FirstAuth-End-Request (PFER) message is sent by the PAA to
      the PaC to signal the result of the first EAP authentication method
      when separate NAP and ISP authentication is performed.

       PANA-FirstAuth-End-Request ::= < PANA-Header: 9, REQ [,SEP] [,NAP] >
                                      < Session-Id >
                                      { Result-Code }
                                      [ EAP-Payload ]
                                      [ Key-Id ]
                                      [ Notification ]
                                   *  [ AVP ]
                                  0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    ACE_DEBUG((LM_INFO, "(%P|%t) RxPFER: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // process notification
    ProcessNotification(msg);

    // Reset flags
    m_Flags.p = 0;
    
    // sanity checks
    if (AuxVariables().FirstEapResult() != 0) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "PFER message received when 1st EAP result is already set"));
    }
    if (! AuxVariables().SeparateAuthentication()) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "PFER message received when separate auth is not supported"));
    }

    // update current authentication mode
    AuxVariables().NapAuthentication() = msg.flags().nap;

    // result code checks
    DiameterUInt32AvpContainerWidget rcodeAvp(msg.avpList());
    diameter_unsigned32_t *rcode = rcodeAvp.GetAvp(PANA_AVPNAME_RESULTCODE);
    if (rcode == NULL) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No Result-Code AVP in PFER message"));
    }
    AuxVariables().FirstEapResult() = ACE_NTOHL(*rcode);

    // eap paylaod checks
    DiameterStringAvpContainerWidget eapAvp(msg.avpList());
    diameter_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);
    if (payload) {
        NotifyEapRequest(*payload);
    }
    else {
        m_Event.EapAltReject();
        Error(ACE_NTOHL(*rcode));
        return;
    }

    // update key id
    DiameterUInt32AvpContainerWidget keyIdAvp(msg.avpList());
    diameter_unsigned32_t *keyId = keyIdAvp.GetAvp(PANA_AVPNAME_KEYID);
    if (keyId) {
        SecurityAssociation().UpdateKeyId1(ACE_NTOHL(*keyId));
    }
}

void PANA_Client::RxPBR()
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
                             [ Key-Id ]
                          *  [ Device-Id ]
                             [ Notification ]
                          *  [ AVP ]
                         0*1 < AUTH >
     */

    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    ACE_DEBUG((LM_INFO, "(%P|%t) RxPBR: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // process notification
    ProcessNotification(msg);

    // Reset flags
    m_Flags.p = 0;
    
    // lookup result code
    DiameterUInt32AvpContainerWidget rcodeAvp(msg.avpList());
    diameter_unsigned32_t *pRcode = rcodeAvp.GetAvp(PANA_AVPNAME_RESULTCODE);
    if (pRcode == NULL) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No Result-Code AVP in PBR message"));
    }
    diameter_unsigned32_t rcode = ACE_NTOHL(*pRcode);

    // update ep device id's
    bool epIdPresent = false;
    DiameterAddressAvpContainerWidget epIdAvp(msg.avpList());
    diameter_address_t *epId = epIdAvp.GetAvp(PANA_AVPNAME_DEVICEID);
    if (epId) {
        epIdPresent = true;
        for (int ndx=1; epId; ndx++) {
            PANA_DeviceId *newId = new PANA_DeviceId(*epId);
            EpDeviceIds().push_back(newId);
            epId = epIdAvp.GetAvp(PANA_AVPNAME_DEVICEID, ndx);
        }
    }

    // update session lifetime
    DiameterUInt32AvpContainerWidget slAvp(msg.avpList());
    diameter_unsigned32_t *sl = slAvp.GetAvp(PANA_AVPNAME_SESSIONLIFETIME);
    if (sl) {
        SessionLifetime() = ACE_NTOHL(*sl);
    }

    // update protection capability
    DiameterUInt32AvpContainerWidget pcapAvp(msg.avpList());
    diameter_unsigned32_t *pcap = pcapAvp.GetAvp(PANA_AVPNAME_PROTECTIONCAP);
    if (pcap && PANA_RCODE_SUCCESS(rcode)) {
        if (ACE_NTOHL(*pcap) != PANA_CFG_GENERAL().m_ProtectionCap) {
            ACE_DEBUG((LM_ERROR,  
                       "No matchig protection-capability ... session will close"));
            m_Flags.i.PcapNotSupported = true;
        }
        ProtectionCapability() = ACE_NTOHL(*pcap);
    }

    // update post pana address config
    DiameterUInt32AvpContainerWidget ppacAvp(msg.avpList());
    diameter_unsigned32_t *ppac = ppacAvp.GetAvp(PANA_AVPNAME_PPAC);
    if (ppac && PANA_RCODE_SUCCESS(rcode)) {
        if (! (PPAC().common(ACE_NTOHL(*ppac)))) {
            ACE_DEBUG((LM_ERROR,  
                       "No matching PPAC ... session will close"));
            m_Flags.i.PpacNotSupported = true;
        }
        PPAC().set(ACE_NTOHL(*ppac));
    }

    // update current authentication mode
    AuxVariables().NapAuthentication() = msg.flags().nap;

    // update dhcp bootstrapping
    if (PPAC().DhcpV6() && DhcpBootstrap().Enable()) {
        DhcpBootstrap().CheckPBR(msg);
    }

#if defined(PANA_MPA_SUPPORT)
    DiameterAddressAvpContainerWidget pacIpAvp(msg.avpList());
    diameter_address_t *pacIp = epIdAvp.GetAvp(PANA_AVPNAME_PACIP);
     
    if (pacIp) {
        PANA_DeviceIdContainer &localAddrs = m_TxChannel.GetLocalAddress();
        PANA_DeviceId *ptr_oldip =(localAddrs.search(AAA_ADDR_FAMILY_IPV4));
        PANA_DeviceId remote = PaaDeviceId();

 	if (ptr_oldip != NULL) {
            PANA_DeviceId ip = *pacIp;
            PANA_DeviceId oldip = *ptr_oldip;
            static_cast<PANA_ClientEventInterface&>(m_Event).PacIpAddress(ip, oldip, remote);
        }
    }
#endif

    // extract eap
    DiameterStringAvpContainerWidget eapAvp(msg.avpList());
    diameter_octetstring_t *payload = eapAvp.GetAvp(PANA_AVPNAME_EAP);

    if (AuxVariables().FirstEapResult() == 0) {
        if (AuxVariables().SeparateAuthentication() == false) {
            if (PANA_RCODE_SUCCESS(rcode)) {
                if (AuxVariables().SecAssociationResumed() == false) {
                    if (epIdPresent) {
                        AuxVariables().CarryDeviceId() = true;
                    }
                    if (payload) {
                        NotifyEapRequest(*payload);
                    }
                    else {
                        Error(rcode);
                    }
                }
                else {
                    // session resumption verification
                    DiameterStringAvpContainerWidget nonceAvp(msg.avpList());
                    diameter_octetstring_t *nonce = nonceAvp.GetAvp(PANA_AVPNAME_NONCE);
                    if (! nonce) {
                        throw (PANA_Exception(PANA_Exception::FAILED, 
                               "Session resumption failed, missing nonce in PBR"));
                    }
                    PANA_Nonce ncheck(*nonce);
                    if (! (ncheck == SecurityAssociation().PaaNonce())) {
                        throw (PANA_Exception(PANA_Exception::FAILED, 
                               "Session resumption failed, invalid PAA nonce"));
                    }
                    if (! SecurityAssociation().ValidateAuthAvp(msg)) {
                        throw (PANA_Exception(PANA_Exception::FAILED, 
                               "Session resumption failed, invalid AUTH received"));
                    }
                    DiameterUInt32AvpContainerWidget keyIdAvp(msg.avpList());
                    diameter_unsigned32_t *keyid = keyIdAvp.GetAvp(PANA_AVPNAME_KEYID);
                    if (! keyid) {
                        throw (PANA_Exception(PANA_Exception::FAILED, 
                               "Session resumption failed, missing keyid in PBR"));
                    }
                    if (ACE_NTOHL(*keyid) != SecurityAssociation().AAAKey1().Id()) {
                        throw (PANA_Exception(PANA_Exception::FAILED, 
                               "Session resumption failed, mis-match keyid"));
                    }

                    TxPBA(false); // TBD: resolve this
                    NotifyAuthorization();
                    NotifyScheduleLifetime();
                }
                return;
            }
        }
        else {
            throw (PANA_Exception(PANA_Exception::FAILED, 
                   "PBR received during separate auth w/o PFER"));
        }
    }
    
    // update device id
    if (PANA_RCODE_SUCCESS(rcode) && epIdPresent) {
        AuxVariables().CarryDeviceId() = true;
    }

    if (payload) {
        NotifyEapRequest(*payload);
    }
    else {
        m_Event.EapAltReject();
        if (! PANA_RCODE_SUCCESS(rcode)) {
            Error(rcode);
        }
        return;
    }

    if (PANA_RCODE_SUCCESS(rcode)) {
        // update verification flags
        m_Flags.i.BindSuccess = 1;
        
        // key id update
        DiameterUInt32AvpContainerWidget keyIdAvp(msg.avpList());
        diameter_unsigned32_t *keyId = keyIdAvp.GetAvp(PANA_AVPNAME_KEYID);
        if (keyId) {
            SecurityAssociation().UpdateKeyId2(ACE_NTOHL(*keyId));
        }
    }
    else {
        Error(rcode);
    }
}

void PANA_Client::TxPBA(bool close)
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
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PBA;
    msg->seq() = LastRxSeqNum().Value();
    msg->flags().separate = AuxVariables().SeparateAuthentication();
    msg->flags().nap = AuxVariables().NapAuthentication();

    // add session id
    AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    if (! close) {
       // add ppac
       DiameterUInt32AvpWidget ppacAvp(PANA_AVPNAME_PPAC);
       ppacAvp.Get() = ACE_HTONL(PPAC()());
       msg->avpList().add(ppacAvp());

       // add device-id
       if (AuxVariables().CarryDeviceId()) {
           DiameterAddressAvpWidget deviceAvp(PANA_AVPNAME_DEVICEID);
           deviceAvp.Get() = PacDeviceId()();
           msg->avpList().add(deviceAvp());
       }

       // update the aaa key's
       diameter_octetstring_t newKey;
       if (m_Event.IsKeyAvailable(newKey)) {
           if (AuxVariables().SeparateAuthentication() == false) {
               SecurityAssociation().UpdateAAAKey(newKey);
           }
           else {
               SecurityAssociation().UpdateAAAKey2(newKey);
           }
           SecurityAssociation().GenerateAuthKey(SessionId());
       }
    }

    // add notification if any       
    AddNotification(*msg);

    // auth and key-id
    if (SecurityAssociation().IsSet()) {
        // add Dhcp-AVP
        if (PPAC().DhcpV6() && DhcpBootstrap().Enable()) {
            DhcpBootstrap().AffixToPBA(*msg);
        }

        SecurityAssociation().AddKeyIdAvp(*msg);
        SecurityAssociation().AddAuthAvp(*msg);
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPBA: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::TxPFEA(bool closed)
{
    /*
      7.17  PANA-FirstAuth-End-Answer (PFEA)

       The PANA-FirstAuth-End-Answer (PFEA) message is sent by the PaC to
       the PAA in response to a PANA-FirstAuth-End-Request message.

        PANA-FirstAuth-End-Answer ::= < PANA-Header: 9, REQ [,SEP] [,NAP] >
                                      < Session-Id >
                                      [ Key-Id ]
                                      [ Notification ]
                                   *  [ AVP ]
                                  0*1 < AUTH >
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PFEA;
    msg->seq() = LastRxSeqNum().Value();
    if (closed) {
       msg->flags().separate = false;
       msg->flags().nap = false;
    }
    else {
       msg->flags().separate = true;
       msg->flags().nap = AuxVariables().NapAuthentication();
    }

    // add session id
    AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // update aaa key if any
    diameter_octetstring_t newKey;
    if (m_Event.IsKeyAvailable(newKey)) {
        SecurityAssociation().UpdateAAAKey1(newKey);
        SecurityAssociation().GenerateAuthKey(SessionId());
    }

    // add notification if any       
    AddNotification(*msg);

    // auth and key-id
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddKeyIdAvp(*msg);
        SecurityAssociation().AddAuthAvp(*msg);
    }

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPFEA: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Client::TxPRAR()
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
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PRAR;
    msg->flags().request = true;

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // reset values
    AuxVariables().FirstEapResult() = 0;
    AuxVariables().SecAssociationResumed() = false;
    AuxVariables().SeparateAuthentication() = 
            PANA_CFG_GENERAL().m_SeparateAuth;

    // add session id
    AAAUtf8AvpWidget sessionIdAvp(PANA_AVPNAME_SESSIONID);
    sessionIdAvp.Get() = SessionId();
    msg->avpList().add(sessionIdAvp());

    // add notification if any       
    AddNotification(*msg);

    // auth avp
    if (SecurityAssociation().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }
    
    // Cancel session timers
    m_Timer.CancelSession();

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPRAR: S-flag %d, N-flag=%d, seq=%d\n",
               msg->flags().separate, msg->flags().nap, msg->seq()));

    SendReqMsg(msg);
}

void PANA_Client::RxPRAA()
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
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    ACE_DEBUG((LM_INFO, "(%P|%t) RxPRAA: S-flag %d, N-flag=%d, seq=%d\n",
               msg.flags().separate, msg.flags().nap, msg.seq()));

    // process notification
    ProcessNotification(msg);

    m_Timer.CancelTxRetry();

    // re-start eap for new conversation
    NotifyEapRestart();
}

void PANA_Client::TxFormatAddress(PANA_Message &msg)
{
   PANA_CfgClient &c = PANA_CFG_PAC();

   // Proper destination IP address hierarchy is as follows:
   //   a. try the known Paa IP address from previous rx msg
   //   b. try locally config unicast addr
   //   c. try locally config mcast address if present
   //   d. undeliverable if not found
   if (PaaIpAddress().get_port_number() > 0) {
      PANA_DeviceId *ipId = PANA_DeviceIdConverter::CreateFromAddr
          (PaaIpAddress());
      if (ipId == NULL) {
         throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                              "Failed to allocate IP device id"));
      }
      msg.srcDevices().push_back(ipId);
      msg.srcPort() = PaaIpAddress().get_port_number();
   }
   else {
      // have to compute peer IP address
      std::string *destIp;

      msg.srcPort() = c.m_PaaPortNumber;
      if (c.m_PaaIpAddress.size() > 0) {
         destIp = &c.m_PaaIpAddress;
      }
      else if (c.m_PaaMcastAddress.size() > 0) {
         destIp = &c.m_PaaMcastAddress;
      }
      else {
         throw (PANA_Exception(PANA_Exception::FAILED, 
                              "Unable to configure a destination address"));
      }

      char buf[256];
      ACE_OS::sprintf(buf, "%s:%d", destIp->data(),
                      c.m_PaaPortNumber);
      ACE_INET_Addr paaAddr(buf);
      PANA_DeviceId *ipId = PANA_DeviceIdConverter::CreateFromAddr(paaAddr);
      if (ipId == NULL) {
         throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                              "Failed to allocate IP device id"));
      }
      msg.srcDevices().push_back(ipId);
   }
}

