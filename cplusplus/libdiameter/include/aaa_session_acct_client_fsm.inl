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

template <class REC_COLLECTOR>
DiameterAcctSessionClientStateMachine::DiameterAcctSessionClientStateMachine
(AAA_Task &t, DiameterAcctSession &s) :
     DiameterAcctSessionStateMachine<DiameterAcctSessionClientStateMachine>
        (t, AAA_SessAcctClientStateTable::Instance(), *this, s)
{
}

template <class REC_COLLECTOR>
void DiameterAcctSessionClientStateMachine<REC_COLLECTOR>::TxACR()
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
   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   ACE_OS::memset(&msg->hdr, 0, sizeof(msg->hdr));
   msg->hdr.ver = DIAMETER_PROTOCOL_VERSION;
   msg->hdr.length = 0;
   msg->hdr.flags.r = DIAMETER_FLAG_SET;
   msg->hdr.flags.p = DIAMETER_FLAG_CLR;
   msg->hdr.flags.e = DIAMETER_FLAG_CLR;
   msg->hdr.code = DIAMETER_MSGCODE_ACCOUNTING;
   msg->hdr.appId = DIAMETER_BASE_APPLICATION_ID;

   // required
   Attributes().SessionId().Set(*msg);

   DiameterIdentityAvpWidget orHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget orRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
   DiameterIdentityAvpWidget destRealmAvp(DIAMETER_AVPNAME_DESTREALM);
   DiameterEnumAvpWidget acctRecTypeAvp(DIAMETER_AVPNAME_ACCTREC_TYPE);
   DiameterUInt32AvpWidget acctRecNumAvp(DIAMETER_AVPNAME_ACCTREC_NUM);
   DiameterUInt32AvpWidget acctIdAvp(DIAMETER_AVPNAME_ACCTAPPID);
   DiameterUInt64AvpWidget acctSubIdAvp(DIAMETER_AVPNAME_ACCTSUBSID);
   DiameterUInt32AvpWidget orStateId(DIAMETER_AVPNAME_ORIGINSTATEID);
   DiameterEnumAvpWidget realtimeAvp(DIAMETER_AVPNAME_ACCTREALTIME);
   DiameterUInt32AvpWidget intervalAvp(DIAMETER_AVPNAME_ACCTINTERVAL);

   orHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
   orRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
   destRealmAvp.Get() = Attributes().DestinationRealm()();
   acctRecTypeAvp.Get() = Attributes().RecordType()();
   acctRecNumAvp.Get() = ++ Attributes().RecordNumber()();
   acctIdAvp.Get() = Attributes().ApplicationId();
   acctSubIdAvp.Get() = Attributes().SubSessionId()();
   realtimeAvp.Get() = Attributes().RealtimeRequired()();
   intervalAvp.Get() = Attributes().InterimInterval()();

   msg->acl.add(orHostAvp());
   msg->acl.add(orRealmAvp());
   msg->acl.add(destRealmAvp());
   msg->acl.add(acctRecTypeAvp());
   msg->acl.add(acctRecNumAvp());
   msg->acl.add(acctIdAvp());
   msg->acl.add(acctSubIdAvp());
   msg->acl.add(realtimeAvp());
   msg->acl.add(intervalAvp());

   // optional avps
   if (Attributes().RadiusAcctSessionId().IsSet()) {
       DiameterStringAvpWidget radiusIdAvp(DIAMETER_AVPNAME_ACCTSID);
       radiusIdAvp.Get() = Attributes().RadiusAcctSessionId()();
       msg->acl.add(radiusIdAvp());
   }

   if (Attributes().MultiSessionId().IsSet()) {
       DiameterUtf8AvpWidget multiIdAvp(DIAMETER_AVPNAME_ACCTMULTISID);
       multiIdAvp.Get() = Attributes().MultiSessionId()();
       msg->acl.add(multiIdAvp());
   }

   if (Attributes().Username().IsSet()) {
       DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
       unameAvp.Get() = Attributes().Username()();
       msg->acl.add(unameAvp());
   }

   if (Attributes().DestinationHost().IsSet()) {
       DiameterIdentityAvpWidget dhostAvp(DIAMETER_AVPNAME_DESTHOST);
       dhostAvp.Get() = Attributes().DestinationHost()();
       msg->acl.add(dhostAvp());
   }

   orStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;
   msg->acl.add(orStateId());

   m_RecCollector().GenerateRecord(msg->acl,
                  Attributes().RecordType());

   m_Session.TxDelivery(msg);
}

template <class REC_COLLECTOR>
void DiameterAcctSessionClientStateMachine<REC_COLLECTOR>::RxACA()
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
    DiameterUInt32AvpContainerWidget rcodeAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
    DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
    DiameterEnumAvpContainerWidget acctRecTypeAvp(msg.acl);
    DiameterUInt32AvpContainerWidget acctRecNumAvp(msg.acl);
    DiameterUInt32AvpContainerWidget acctRealtimeAvp(msg.acl);
    DiameterUtf8AvpContainerWidget errMsgAvp(msg.acl);
    DiameterIdentityAvpContainerWidget errHostAvp(msg.acl);

    diameter_unsigned32_t *rcode = rcodeAvp.GetAvp(DIAMETER_AVPNAME_RESULTCODE);
    diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
    diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
    diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
    diameter_enumerated_t *recType = acctRecTypeAvp.GetAvp(DIAMETER_AVPNAME_ACCTREC_TYPE);
    diameter_unsigned32_t *recNum = acctRecNumAvp.GetAvp(DIAMETER_AVPNAME_ACCTREC_NUM);
    diameter_unsigned32_t *realtime = acctRealtimeAvp.GetAvp(DIAMETER_AVPNAME_ACCTREALTIME);
    diameter_utf8string_t *errMsg = errMsgAvp.GetAvp(DIAMETER_AVPNAME_ERRORMESSAGE);
    diameter_identity_t *errHost = errHostAvp.GetAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);

    AAA_LOG((LM_INFO, "(%P|%t) *** Accounting answer received ***\n"));
    Attributes().MsgIdRxMessage(msg);

    DiameterSessionId sid;
    sid.Get(msg);
    sid.Dump();
    if (host) {
        AAA_LOG((LM_INFO, "(%P|%t) From Host : %s\n", host->c_str()));
    }
    if (realm) {
        AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
    }
    if (uname) {
        AAA_LOG((LM_INFO, "(%P|%t) From User : %s\n", uname->c_str()));
    }
    if (recType) {
        AAA_LOG((LM_INFO, "(%P|%t) Rec Type  : %d\n", *recType));
    }
    if (recNum) {
        AAA_LOG((LM_INFO, "(%P|%t) Rec Num   : %d\n", *recNum));
    }
    if (realtime) {
        AAA_LOG((LM_INFO, "(%P|%t) Realtime  : %d\n", *realtime));
    }
    if (rcode) {
        AAA_LOG((LM_INFO, "(%P|%t) Result-Code: %d\n", *rcode));
    }
    if (errMsg) {
        if (errHost) {
            AAA_LOG((LM_INFO, "(%P|%t) Message from [%s]: %s\n", 
                errHost->c_str(), errMsg->c_str()));
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t) Message: %s\n", 
                errMsg->c_str()));
        }
    }
}
