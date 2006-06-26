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


template <class REC_COLLECTOR>
AAA_AcctSessionServerStateMachine::AAA_AcctSessionServerStateMachine
(AAA_Task &t, AAA_AcctSession &s) :
     AAA_AcctSessionStateMachine<AAA_AcctSessionServerStateMachine>
        (t, AAA_SessAcctServerStateTable::Instance(), *this, s)
{
}

template <class REC_COLLECTOR>
void AAA_AcctSessionServerStateMachine<REC_COLLECTOR>::RxACR
(AAAMessage &msg)
{
   /*
   9.7.1.  Accounting-Request

   The Accounting-Request (ACR) command, indicated by the Command-Code
   field set to 271 and the Command Flags' 'R' bit set, is sent by a
   Diameter node, acting as a client, in order to exchange accounting
   information with a peer.

   One of Acct-Application-Id and Vendor-Specific-Application-Id AVPs
   MUST be present.  If the Vendor-Specific-Application-Id grouped AVP
   is present, it must have an Acct-Application-Id inside.

   The AVP listed below SHOULD include service specific accounting AVPs,
   as described in Section 9.3.

   Message Format

      <ACR> ::= < Diameter Header: 271, REQ, PXY >
                < Session-Id >
                { Origin-Host }
                { Origin-Realm }
                { Destination-Realm }
                { Accounting-Record-Type }
                { Accounting-Record-Number }
                [ Acct-Application-Id ]
                [ Vendor-Specific-Application-Id ]
                [ User-Name ]
                [ Accounting-Sub-Session-Id ]
                [ Acct-Session-Id ]
                [ Acct-Multi-Session-Id ]
                [ Acct-Interim-Interval ]
                [ Accounting-Realtime-Required ]
                [ Origin-State-Id ]
                [ Event-Timestamp ]
              * [ Proxy-Info ]
              * [ Route-Record ]
              * [ AVP ]
    */
    AAA_IdentityAvpContainerWidget oHostAvp(msg.acl);
    AAA_IdentityAvpContainerWidget oRealmAvp(msg.acl);
    AAA_Utf8AvpContainerWidget uNameAvp(msg.acl);
    AAA_EnumAvpContainerWidget acctRecTypeAvp(msg.acl);
    AAA_UInt32AvpContainerWidget acctRecNumAvp(msg.acl);
    AAA_UInt32AvpContainerWidget acctRealtimeAvp(msg.acl);

    diameter_identity_t *host = oHostAvp.GetAvp(AAA_AVPNAME_ORIGINHOST);
    diameter_identity_t *realm = oRealmAvp.GetAvp(AAA_AVPNAME_ORIGINREALM);
    diameter_utf8string_t *uname = uNameAvp.GetAvp(AAA_AVPNAME_USERNAME);
    diameter_unsigned32_t *realtime = acctRealtimeAvp.GetAvp(AAA_AVPNAME_ACCTREALTIME);
    diameter_enumerated_t *recType = acctRecTypeAvp.GetAvp(AAA_AVPNAME_ACCTREC_TYPE);
    diameter_unsigned32_t *recNum = acctRecNumAvp.GetAvp(AAA_AVPNAME_ACCTREC_NUM);

    AAA_LOG(LM_INFO, "(%P|%t) *** Accounting request received ***\n");
    Attributes().MsgIdRxMessage(msg);

    AAA_SessionId sid;
    sid.Get(msg);
    sid.Dump();
    if (host) {
        AAA_LOG(LM_INFO, "(%P|%t) From Host: %s\n", host->data());
    }
    if (realm) {
        AAA_LOG(LM_INFO, "(%P|%t) From Realm: %s\n", realm->data());
    }
    if (uname) {
        AAA_LOG(LM_INFO, "(%P|%t) From User: %s\n", uname->data());
    }
    if (recType) {
        AAA_LOG(LM_INFO, "(%P|%t) Rec Type : %d\n", *recType);
    }
    if (recNum) {
        AAA_LOG(LM_INFO, "(%P|%t) Rec Num  : %d\n", *recNum);
    }
    if (realtime) {
        AAA_LOG(LM_INFO, "(%P|%t) Realtime  : %d\n", *realtime);
    }
}

template <class REC_COLLECTOR>
void AAA_AcctSessionServerStateMachine<REC_COLLECTOR>::TxACA
(diameter_unsigned32_t rcode)
{
   /*
   9.7.2.  Accounting-Answer

   The Accounting-Answer (ACA) command, indicated by the Command-Code
   field set to 271 and the Command Flags' 'R' bit cleared, is used to
   acknowledge an Accounting-Request command.  The Accounting-Answer
   command contains the same Session-Id and includes the usage AVPs only
   if CMS is in use when sending this command.  Note that the inclusion
   of the usage AVPs when CMS is not being used leads to unnecessarily
   large answer messages, and can not be used as a server's proof of the
   receipt of these AVPs in an end-to-end fashion.  If the Accounting-
   Request was protected by end-to-end security, then the corresponding
   ACA message MUST be protected by end-to-end security.

   Only the target Diameter Server, known as the home Diameter Server,
   SHOULD respond with the Accounting-Answer command.

   One of Acct-Application-Id and Vendor-Specific-Application-Id AVPs
   MUST be present.  If the Vendor-Specific-Application-Id grouped AVP
   is present, it must have an Acct-Application-Id inside.

   The AVP listed below SHOULD include service specific accounting AVPs,
   as described in Section 9.3.

   Message Format

      <ACA> ::= < Diameter Header: 271, PXY >
                < Session-Id >
                { Result-Code }
                { Origin-Host }
                { Origin-Realm }
                { Accounting-Record-Type }
                { Accounting-Record-Number }
                [ Acct-Application-Id ]
                [ Vendor-Specific-Application-Id ]
                [ User-Name ]
                [ Accounting-Sub-Session-Id ]
                [ Acct-Session-Id ]
                [ Acct-Multi-Session-Id ]
                [ Error-Reporting-Host ]
                [ Acct-Interim-Interval ]
                [ Accounting-Realtime-Required ]
                [ Origin-State-Id ]
                [ Event-Timestamp ]
              * [ Proxy-Info ]
              * [ AVP ]
   */

   std::auto_ptr<AAAMessage> msg(new AAAMessage);
   ACE_OS::memset(&msg->hdr, 0, sizeof(msg->hdr));
   msg->hdr.ver = AAA_PROTOCOL_VERSION;
   msg->hdr.length = 0;
   msg->hdr.flags.r = AAA_FLG_CLR;
   msg->hdr.flags.p = AAA_FLG_CLR;
   msg->hdr.flags.e = AAA_FLG_CLR;
   msg->hdr.code = AAA_MSGCODE_ACCOUNTING;
   msg->hdr.appId = AAA_BASE_APPLICATION_ID;

   // required
   Attributes().SessionId().Set(*msg);

   AAA_UInt32AvpWidget rcodeAvp(AAA_AVPNAME_RESULTCODE);
   AAA_IdentityAvpWidget orHostAvp(AAA_AVPNAME_ORIGINHOST);
   AAA_IdentityAvpWidget orRealmAvp(AAA_AVPNAME_ORIGINREALM);
   AAA_EnumAvpWidget acctRecTypeAvp(AAA_AVPNAME_ACCTREC_TYPE);
   AAA_UInt32AvpWidget acctRecNumAvp(AAA_AVPNAME_ACCTREC_NUM);
   AAA_UInt32AvpWidget acctIdAvp(AAA_AVPNAME_ACCTAPPID);
   AAA_UInt64AvpWidget acctSubIdAvp(AAA_AVPNAME_ACCTSUBSID);
   AAA_IdentityAvpWidget errrHostAvp(AAA_AVPNAME_ERRORREPORTINGHOST);
   AAA_UInt32AvpWidget orStateId(AAA_AVPNAME_ORIGINSTATEID);
   AAA_EnumAvpWidget realtimeAvp(AAA_AVPNAME_ACCTREALTIME);
   AAA_UInt32AvpWidget intervalAvp(AAA_AVPNAME_ACCTINTERVAL);

   rcodeAvp.Get() = rcode;
   orHostAvp.Get() = AAA_CFG_TRANSPORT()->identity;
   orRealmAvp.Get() = AAA_CFG_TRANSPORT()->realm;
   acctRecTypeAvp.Get() = Attributes().RecordType()();
   acctRecNumAvp.Get() = Attributes().RecordNumber()();
   acctIdAvp.Get() = Attributes().ApplicationId();
   acctSubIdAvp.Get() = Attributes().SubSessionId()();
   errHoststAvp.Get() = AAA_CFG_TRANSPORT()->identity;
   realtimeAvp.Get() = Attributes().RealtimeRequired()();
   intervalAvp.Get() = Attributes().InterimInterval()();

   msg->acl.add(rcodeAvp());
   msg->acl.add(orHostAvp());
   msg->acl.add(orRealmAvp());
   msg->acl.add(acctRecTypeAvp());
   msg->acl.add(acctRecNumAvp());
   msg->acl.add(acctIdAvp());
   msg->acl.add(acctSubIdAvp());
   msg->acl.add(realtimeAvp());
   msg->acl.add(intervalAvp());

   // optional avps
   if (Attributes().RadiusAcctSessionId().IsSet()) {
       AAA_StringAvpWidget radiusIdAvp(AAA_AVPNAME_ACCTSID);
       radiusIdAvp.Get() = Attributes().RadiusAcctSessionId()();
       msg->acl.add(radiusIdAvp());
   }

   if (Attributes().MultiSessionId().IsSet()) {
       AAA_Utf8AvpWidget multiIdAvp(AAA_AVPNAME_ACCTMULTISID);
       multiIdAvp.Get() = Attributes().MultiSessionId()();
       msg->acl.add(multiIdAvp());
   }

   if (Attributes().Username().IsSet()) {
       AAA_Utf8AvpWidget unameAvp(AAA_AVPNAME_USERNAME);
       unameAvp.Get() = Attributes().Username()();
       msg->acl.add(unameAvp());
   }

   orStateId.Get() = AAA_CFG_RUNTIME()->originStateId;
   msg->acl.add(orStateId());

   m_RecStorage.UpdateAcctResponse(*msg);

   m_Session.TxDelivery(msg);
}


