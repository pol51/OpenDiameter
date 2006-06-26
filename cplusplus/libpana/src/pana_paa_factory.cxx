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

#include "pana_session.h"
#include "pana_paa_factory.h"
#include "pana_cookie.h"
#include "pana_config_manager.h"
#include "pana_sid_generator.h"

void PANA_PaaSessionFactory::Receive(PANA_Message &msg)
{
   PANA_DeviceId *ipId = PANA_DeviceIdConverter::GetIpOnlyAddress
                              (msg.srcDevices(),
                               (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
   if (ipId == NULL) {
      throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
             "Cannot determine the device ID of peer"));
   }

   try {
      AAA_Utf8AvpContainerWidget sidAvp(msg.avpList());
      diameter_utf8string_t *sid = sidAvp.GetAvp(PANA_AVPNAME_SESSIONID);
      if (sid) {
         PANA_PaaSession *session = PANA_SESSIONDB_SEARCH
                                       (const_cast<std::string&>(*sid));
         session->Receive(msg);
      }
      else {
         throw (PANA_Exception(PANA_Exception::ENTRY_NOT_FOUND,
                "No session-id present in message"));
      }
   }
   catch (PANA_Exception &e) {
      ACE_UNUSED_ARG(e);
      if (msg.flags().request) {
         throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                "factory received invalid request message"));
      }
      switch (msg.type()) {
         case PANA_MTYPE_PDI: 
             RxPDI(msg); 
             break;
         case PANA_MTYPE_PSA: 
             try {
                 // migrate to real db
                 PANA_PaaSession *s;
                 m_PendingDb.Remove(ipId->value, &s);
                 PANA_SESSIONDB_ADD(s->m_PAA.SessionId(), *s);

                 // notify session
                 PANA_PaaEventVariable ev;
                 ev.MsgType(PANA_EV_MTYPE_PSA);
                 s->m_PAA.AuxVariables().RxMsgQueue().Enqueue(&msg);
                 s->Notify(ev.Get());
                 break;
             }
             catch (PANA_SessionDb::DB_ERROR e) {                
                 ACE_UNUSED_ARG(e);
                 if (PANA_SessionDb::ENTRY_NOT_FOUND) {
                    StatelessRxPSA(msg); 
                    break;
                 }
             }
             // fall through
         default:
            throw (PANA_Exception(PANA_Exception::ENTRY_NOT_FOUND, 
                   "Session not found, discarding message"));
      }
   }
}

void PANA_PaaSessionFactory::PacFound(ACE_INET_Addr &addr) 
{
   if (PANA_CFG_PAA().m_UseCookie) {
       ///////////////////////////////////////////////////////////////
       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - - - - - - - - (Stateless discovery) - - - - - - - -
       // (Rx:PDI ||             if (SEPARATE==Set)         OFFLINE
       // PAC_FOUND) &&             PSR.S_flag=1;
       // USE_COOKIE==Set          PSR.insert_avp
       //                           ("Cookie");
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
       //
       StatelessTxPSR(addr);
   }
   else {
       ///////////////////////////////////////////////////////////////
       // - - - - - - - - - - - (Stateful discovery)- - - - - - - - -
       // (Rx:PDI ||             EAP_Restart();             WAIT_EAP_MSG_
       // PAC_FOUND) &&                                      IN_DISC
       // USE_COOKIE==Unset &&
       // EAP_PIGGYBACK==Set
       // 
       // (Rx:PDI ||             if (SEPARATE==Set)         STATEFUL_DISC
       //  PAC_FOUND) &&           PSR.S_flag=1;
       //  USE_COOKIE==Unset &&  if (CARRY_NAP_INFO==Set)
       //  PIGGYBACK==Unset        PSR.insert_avp
       //                           ("NAP-Information");
       //                        if (CARRY_ISP_INFO==Set)
       //                          PSR.insert_avp
       //                           ("ISP-Information");
       //                        if (CARRY_PPAC==Set)
       //                          PSR.insert_avp
       //                           ("Post-PANA-Address-
       //                             Configuration");
       //                        if (PROTECTION_CAP_IN_PSR
       //                            ==Set)
       //                           PSR.insert_avp
       //                            ("Protection-Cap.");
       //                        Tx:PSR();
       //                        RtxTimerStart();
       //
       PANA_PaaSession *session = Create();
       if (session == NULL) {
          throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                               "Failed to auth agent session"));
       }

       // add to pending db
       ACE_DEBUG((LM_INFO, "(%P|%t) New session created [stateful discovery]\n"));  
       diameter_address_t id;
       PANA_AddrConverter::ToAAAAddress(addr, id);
       m_PendingDb.Add(id.value, *session);

       // save address of PaC       
       session->m_PAA.PacIpAddress() = addr;
       PANA_DeviceIdConverter::PopulateFromAddr(addr, 
           session->m_PAA.PacDeviceId());

       // notify session
       PANA_PaaEventVariable ev;
       ev.Event_App(PANA_EV_APP_PAC_FOUND);
       if (PANA_CFG_GENERAL().m_EapPiggyback) {
           ev.EnableCfg_PiggyBack();
           session->Notify(ev.Get());
       }
       else {
           session->Notify(ev.Get());
       }
   }
}

void PANA_PaaSessionFactory::RxPDI(PANA_Message &msg)
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

   ACE_DEBUG((LM_INFO, "(%P|%t) RxPDI: S-flag %d, seq=%d\n",
             msg.flags().separate, msg.seq()));

   // validate sequence number
   if (msg.seq() != 0) {
      throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
             "Received PDI with non-zero seq num"));
   }
   // extract PaC IP address
   PANA_DeviceId *ipId = PANA_DeviceIdConverter::GetIpOnlyAddress
                              (msg.srcDevices(),
                               (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
   if (ipId == NULL) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
              "PDI has no IP address assigned to packet"));
   }

   AAA_StringAvpContainerWidget NotificationAvp(msg.avpList());
   diameter_octetstring_t *note = NotificationAvp.GetAvp
       (PANA_AVPNAME_NOTIFICATION);
   if (note) {
       ACE_DEBUG((LM_INFO, "(%P|%t) NOTIFICATION: %s\n",
           note->data()));
   }
    
   ACE_INET_Addr addr;
   PANA_DeviceIdConverter::FormatToAddr(*ipId, addr);
   addr.set_port_number(msg.srcPort());
   PacFound(addr);
   delete &msg;
}

void PANA_PaaSessionFactory::StatelessTxPSR(ACE_INET_Addr &addr)
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
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->seq() = PANA_SerialNumber::GenerateISN();
    msg->flags().request = true;
    msg->flags().separate = PANA_CFG_GENERAL().m_SeparateAuth;

    // add cookie
    diameter_octetstring_t cookie;
    diameter_address_t id;
    PANA_AddrConverter::ToAAAAddress(addr, id);
    PANA_COOKIE_GENERATE(id.value, cookie);
    AAA_StringAvpWidget cookieAvp(PANA_AVPNAME_COOKIE);
    cookieAvp.Get().assign(cookie.data(), cookie.size());
    msg->avpList().add(cookieAvp());

    // add ppac
    if (PANA_CFG_GENERAL().m_PPAC() > 0) {
        AAA_UInt32AvpWidget ppacAvp(PANA_AVPNAME_PPAC);
        ppacAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_PPAC());
        msg->avpList().add(ppacAvp());
    }

    // add protection capability
    if (m_Flags.i.CarryPcapInPSR) {
        AAA_UInt32AvpWidget pcapAvp(PANA_AVPNAME_PROTECTIONCAP);
        pcapAvp.Get() = ACE_HTONL(PANA_CFG_GENERAL().m_ProtectionCap);
        msg->avpList().add(pcapAvp());
    }
    
    // add nap information
    if (PANA_CFG_PAA().m_NapInfo.m_Id > 0) {
        AAA_GroupedAvpWidget napAvp(PANA_AVPNAME_NAPINFO);
        PANA_ProviderInfoTool infoTool;
        infoTool.Add(napAvp.Get(), PANA_CFG_PAA().m_NapInfo);
        msg->avpList().add(napAvp());
    }

    // add ISP information
    if (! PANA_CFG_PAA().m_IspInfo.empty()) {
        PANA_CfgProviderList::iterator i;
        AAA_GroupedAvpWidget ispAvp(PANA_AVPNAME_ISPINFO);
        for (i = PANA_CFG_PAA().m_IspInfo.begin();
             i != PANA_CFG_PAA().m_IspInfo.end();
             i++) {
            PANA_ProviderInfoTool infoTool;
            infoTool.Add(ispAvp.Get(), *(*i));
        }
        msg->avpList().add(ispAvp());
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

    PANA_DeviceId ipId;
    PANA_DeviceIdConverter::PopulateFromAddr(addr, ipId);
    msg->srcDevices().clone(ipId);
    msg->srcPort() = addr.get_port_number();

    ACE_DEBUG((LM_INFO, "(%P|%t) TxPSR: Stateless, S-flag=%d, seq=%d\n",
               msg->flags().separate, msg->seq()));

    m_UdpChannel.Send(msg);
}

void PANA_PaaSessionFactory::StatelessRxPSA(PANA_Message &msg)
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

   ACE_DEBUG((LM_INFO, "(%P|%t) RxPSA: Stateless, S-flag %d, seq=%d\n",
                       msg.flags().separate, msg.seq()));

   PANA_DeviceId *ipId = PANA_DeviceIdConverter::GetIpOnlyAddress
                              (msg.srcDevices(),
                               (bool)PANA_CFG_GENERAL().m_IPv6Enabled);
   if (ipId == NULL) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
              "Cannot determine the device ID of peer"));
   }

   //////////////////////////////////////////////////////////////////
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // - - - - - - - (PSA processing without mobility support) - - - -
   // Rx:PSA &&                if (SEPARATE==Set &&       WAIT_EAP_MSG
   // USE_COOKIE==Set &&           PSA.S_flag==0)
   // (!PSA.exist_avp            SEPARATE=Unset;
   //   ("Session-Id") ||      EAP_Restart();
   //  MOBILITY==Unset ||
   //  (MOBILITY==Set &&
   //  !retrieve_pana_sa
   //    (PSA.SESSION_ID)))

   // first level validation
   if (! ValidateCookie(msg)) {
      throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                           "PSA received with invalid cookie"));
   }

   PANA_PaaSession *session = Create();
   if (session == NULL) {
      throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                           "Failed to auth agent session"));
   }
   ACE_DEBUG((LM_INFO, "(%P|%t) New session created [stateless discovery]\n"));  

   // save address of PaC       
   session->m_PAA.PacDeviceId() = *ipId;
   PANA_DeviceIdConverter::FormatToAddr(*ipId, 
       session->m_PAA.PacIpAddress());
   session->m_PAA.PacIpAddress().set_port_number(msg.srcPort());

   // save initial seq number
   session->m_PAA.LastTxSeqNum() = msg.seq();

   // add to session database permanently
   PANA_SESSIONDB_ADD(session->SessionId(), *session);
   session->Receive(msg);
}

bool PANA_PaaSessionFactory::ValidateCookie(PANA_Message &msg)
{
   AAA_StringAvpContainerWidget cookieAvp(msg.avpList());
   diameter_octetstring_t *cookie = cookieAvp.GetAvp(PANA_AVPNAME_COOKIE);
   if (cookie) {
       PANA_DeviceId *ipId = PANA_DeviceIdConverter::GetIpOnlyAddress
           (msg.srcDevices(), (bool)PANA_CFG_GENERAL().m_IPv6Enabled);

       ACE_DEBUG((LM_INFO, "(%P|%t) Validating cookie\n"));

       return PANA_COOKIE_VERIFY(ipId->value, *cookie);
   }
   return (true);
}
