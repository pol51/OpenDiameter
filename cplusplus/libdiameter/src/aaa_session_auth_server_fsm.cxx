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

#include "aaa_session_auth_server_fsm.h"

static AAA_SessAuthServerStateTable AuthServerStateTable_S;

DiameterAuthSessionServerStateMachine::DiameterAuthSessionServerStateMachine
(AAA_Task &t, DiameterAuthSession &a) :
     DiameterAuthSessionStateMachine<DiameterAuthSessionServerStateMachine>
         (t, AuthServerStateTable_S, *this, a),
     m_ASRSent(false)
{
}

void DiameterAuthSessionServerStateMachine::RxSTR(DiameterMsg &msg)
{
   /*
        8.4.1.  Session-Termination-Request

        The Session-Termination-Request (STR), indicated by the Command-Code
        set to 275 and the Command Flags' 'R' bit set, is sent by the access
        device to inform the Diameter Server that an authenticated and/or
        authorized session is being terminated.

        Message Format

            <STR> ::= < Diameter Header: 275, REQ, PXY >
                        < Session-Id >
                        { Origin-Host }
                        { Origin-Realm }
                        { Destination-Realm }
                        { Auth-Application-Id }
                        { Termination-Cause }
                        [ User-Name ]
                        [ Destination-Host ]
                      * [ Class ]
                        [ Origin-State-Id ]
                      * [ Proxy-Info ]
                      * [ Route-Record ]
                      * [ AVP ]
   */
    DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
    DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);
    DiameterUInt32AvpContainerWidget acctAppIdAvp(msg.acl);
    DiameterUInt32AvpContainerWidget termCauseAvp(msg.acl);
    DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
    DiameterStringAvpContainerWidget classAvp(msg.acl);

    diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
    diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
    diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);
    diameter_unsigned32_t *acctAppId = acctAppIdAvp.GetAvp(DIAMETER_AVPNAME_ACCTAPPID);
    diameter_unsigned32_t *termCause = termCauseAvp.GetAvp(DIAMETER_AVPNAME_TERMINATION);
    diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
    diameter_octetstring_t *cls = classAvp.GetAvp(DIAMETER_AVPNAME_CLASS);

    AAA_LOG((LM_INFO, "(%P|%t) *** Session termination request received ***\n"));
    Attributes().MsgIdRxMessage(msg);

    DiameterSessionId sid;
    sid.Get(msg);
    sid.Dump();
    if (host) {
        AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
    }
    if (realm) {
        AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
    }
    if (uname) {
        AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
    }
    if (termCause) {
        AAA_LOG((LM_INFO, "(%P|%t) Termination Cause: %d\n", *termCause));
    }
    if (authAppId) {
        AAA_LOG((LM_INFO, "(%P|%t) Auth Application Id: %d\n", *authAppId));
    }
    if (acctAppId) {
        AAA_LOG((LM_INFO, "(%P|%t) Acct Application Id: %d\n", *acctAppId));
    }
    if (cls) {
        DiameterScholarAttribute<diameter_octetstring_t> schClass(*cls);
        m_Session.ClassAvp(schClass);
    }
}

void DiameterAuthSessionServerStateMachine::RxASA(DiameterMsg &msg)
{
   /*
        8.5.2.  Abort-Session-Answer

        The Abort-Session-Answer (ASA), indicated by the Command-Code set to
        274 and the message flags' 'R' bit clear, is sent in response to the
        ASR.  The Result-Code AVP MUST be present, and indicates the
        disposition of the request.

        If the session identified by Session-Id in the ASR was successfully
        terminated, Result-Code is set to DIAMETER_SUCCESS.  If the session
        is not currently active, Result-Code is set to
        DIAMETER_UNKNOWN_SESSION_ID.  If the access device does not stop the
        session for any other reason, Result-Code is set to
        DIAMETER_UNABLE_TO_COMPLY.

        Message Format

            <ASA>  ::= < Diameter Header: 274, PXY >
                        < Session-Id >
                        { Result-Code }
                        { Origin-Host }
                        { Origin-Realm }
                        [ User-Name ]
                        [ Origin-State-Id ]
                        [ Error-Message ]
                        [ Error-Reporting-Host ]
                        * [ Failed-AVP ]
                        * [ Redirect-Host ]
                        [ Redirect-Host-Usage ]
                        [ Redirect-Max-Cache-Time ]
                        * [ Proxy-Info ]
                        * [ AVP ]
    */
    DiameterUInt32AvpContainerWidget rcodeAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
    DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
    DiameterUtf8AvpContainerWidget errMsgAvp(msg.acl);
    DiameterIdentityAvpContainerWidget errHostAvp(msg.acl);

    diameter_unsigned32_t *rcode = rcodeAvp.GetAvp(DIAMETER_AVPNAME_RESULTCODE);
    diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
    diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
    diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
    diameter_utf8string_t *errMsg = errMsgAvp.GetAvp(DIAMETER_AVPNAME_ERRORMESSAGE);
    diameter_identity_t *errHost = errHostAvp.GetAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);

    AAA_LOG((LM_INFO, "(%P|%t) *** Abort session answer received ***\n"));
    Attributes().MsgIdRxMessage(msg);

    DiameterSessionId sid;
    sid.Get(msg);
    sid.Dump();
    if (host) {
        AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
    }
    if (realm) {
        AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
    }
    if (uname) {
        AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
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

void DiameterAuthSessionServerStateMachine::TxSTA(diameter_unsigned32_t rcode)
{
    /*
        8.4.2.  Session-Termination-Answer

        The Session-Termination-Answer (STA), indicated by the Command-Code
        set to 275 and the message flags' 'R' bit clear, is sent by the
        Diameter Server to acknowledge the notification that the session has
        been terminated.  The Result-Code AVP MUST be present, and MAY
        contain an indication that an error occurred while servicing the STR.

        Upon sending or receipt of the STA, the Diameter Server MUST release
        all resources for the session indicated by the Session-Id AVP.  Any
        intermediate server in the Proxy-Chain MAY also release any
        resources, if necessary.

        Message Format

            <STA>  ::= < Diameter Header: 275, PXY >
                        < Session-Id >
                        { Result-Code }
                        { Origin-Host }
                        { Origin-Realm }
                        [ User-Name ]
                      * [ Class ]
                        [ Error-Message ]
                        [ Error-Reporting-Host ]
                      * [ Failed-AVP ]
                        [ Origin-State-Id ]
                      * [ Redirect-Host ]
                        [ Redirect-Host-Usage ]
                                            ^
                        [ Redirect-Max-Cache-Time ]
                      * [ Proxy-Info ]
                      * [ AVP ]
     */
    std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
    ACE_OS::memset(&msg->hdr, 0, sizeof(msg->hdr));
    msg->hdr.ver = DIAMETER_PROTOCOL_VERSION;
    msg->hdr.length = 0;
    msg->hdr.flags.r = DIAMETER_FLAG_CLR;
    msg->hdr.flags.p = DIAMETER_FLAG_CLR;
    msg->hdr.flags.e = DIAMETER_FLAG_CLR;
    msg->hdr.code = DIAMETER_MSGCODE_SESSIONTERMINATION;
    msg->hdr.appId = DIAMETER_BASE_APPLICATION_ID;

    // required
    Attributes().SessionId().Set(*msg);

    DiameterUInt32AvpWidget rcodeAvp(DIAMETER_AVPNAME_RESULTCODE);
    DiameterIdentityAvpWidget orHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
    DiameterIdentityAvpWidget orRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
    DiameterUInt32AvpWidget orStateId(DIAMETER_AVPNAME_ORIGINSTATEID);

    rcodeAvp.Get() = rcode;
    orHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
    orRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;

    msg->acl.add(rcodeAvp());
    msg->acl.add(orHostAvp());
    msg->acl.add(orRealmAvp());

    // optional avps
    if (Attributes().Username().IsSet()) {
        DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
        unameAvp.Get() = Attributes().Username()();
        msg->acl.add(unameAvp());
    }

    DiameterScholarAttribute<diameter_octetstring_t> cls;
    m_Session.SetClassAvp(cls);
    if (cls.IsSet()) {
        DiameterStringAvpWidget classAvp(DIAMETER_AVPNAME_CLASS);
        classAvp.Get() = cls();
        msg->acl.add(classAvp());
    }

    orStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;
    msg->acl.add(orStateId());

    // TBD: Add more AVP's here if needed

    m_Session.TxDelivery(msg);
}

void DiameterAuthSessionServerStateMachine::TxASR()
{
    /*
        8.5.1.  Abort-Session-Request

        The Abort-Session-Request (ASR), indicated by the Command-Code set to
        274 and the message flags' 'R' bit set, may be sent by any server to
        the access device that is providing session service, to request that
        the session identified by the Session-Id be stopped.

        Message Format

            <ASR>  ::= < Diameter Header: 274, REQ, PXY >
                        < Session-Id >
                        { Origin-Host }
                        { Origin-Realm }
                        { Destination-Realm }
                        { Destination-Host }
                        { Auth-Application-Id }
                        [ User-Name ]
                        [ Origin-State-Id ]
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
    msg->hdr.code = DIAMETER_MSGCODE_ABORTSESSION;
    msg->hdr.appId = DIAMETER_BASE_APPLICATION_ID;

    // required
    Attributes().SessionId().Set(*msg);

    DiameterIdentityAvpWidget orHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
    DiameterIdentityAvpWidget orRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
    DiameterIdentityAvpWidget destHostAvp(DIAMETER_AVPNAME_DESTHOST);
    DiameterIdentityAvpWidget destRealmAvp(DIAMETER_AVPNAME_DESTREALM);
    DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
    DiameterUInt32AvpWidget orStateId(DIAMETER_AVPNAME_ORIGINSTATEID);

    orHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
    orRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
    destRealmAvp.Get() = Attributes().DestinationRealm()();
    destHostAvp.Get() = Attributes().DestinationHost()();
    authIdAvp.Get() = Attributes().ApplicationId();

    msg->acl.add(orHostAvp());
    msg->acl.add(orRealmAvp());
    msg->acl.add(destHostAvp());
    msg->acl.add(destRealmAvp());
    msg->acl.add(authIdAvp());

    // optional avps
    if (Attributes().Username().IsSet()) {
        DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
        unameAvp.Get() = Attributes().Username()();
        msg->acl.add(unameAvp());
    }

    orStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;
    msg->acl.add(orStateId());

    // TBD: Add more AVP's here if needed

    if (m_Session.TxDelivery(msg) != AAA_ERR_SUCCESS) {
        ScheduleTimer(DIAMETER_SESSION_AUTH_EV_TX_ASR_FAIL,
                      DIAMETER_CFG_AUTH_SESSION()->abortRetryTm,
                      0, DIAMETER_TIMER_TYPE_ASR);
        return;
    }
    m_ASRSent = true;
}

void DiameterAuthSessionServerStateMachine::TxRAR(diameter_unsigned32_t reAuthType)
{
   /*
      8.3.1.  Re-Auth-Request

    The Re-Auth-Request (RAR), indicated by the Command-Code set to 258
    and the message flags' 'R' bit set, may be sent by any server to the
    access device that is providing session service, to request that the
    user be re-authenticated and/or re-authorized.

    Message Format

       <RAR>  ::= < Diameter Header: 258, REQ, PXY >
                  < Session-Id >
                  { Origin-Host }
                  { Origin-Realm }
                  { Destination-Realm }
                  { Destination-Host }
                  { Auth-Application-Id }
                  { Re-Auth-Request-Type }
                  [ User-Name ]
                  [ Origin-State-Id ]
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
    msg->hdr.code = DIAMETER_MSGCODE_REAUTH;
    msg->hdr.appId = DIAMETER_BASE_APPLICATION_ID;

    // required
    Attributes().SessionId().Set(*msg);

    DiameterIdentityAvpWidget orHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
    DiameterIdentityAvpWidget orRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
    DiameterIdentityAvpWidget destHostAvp(DIAMETER_AVPNAME_DESTHOST);
    DiameterIdentityAvpWidget destRealmAvp(DIAMETER_AVPNAME_DESTREALM);
    DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
    DiameterUInt32AvpWidget reAuthTypeAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);
    DiameterUtf8AvpWidget uNameAvp(DIAMETER_AVPNAME_USERNAME);
    DiameterUInt32AvpWidget orStateId(DIAMETER_AVPNAME_ORIGINSTATEID);

    orHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
    orRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
    destRealmAvp.Get() = Attributes().DestinationRealm()();
    destHostAvp.Get() = Attributes().DestinationHost()();
    authIdAvp.Get() = Attributes().ApplicationId();
    reAuthTypeAvp.Get() = reAuthType;

    msg->acl.add(orHostAvp());
    msg->acl.add(orRealmAvp());
    msg->acl.add(destHostAvp());
    msg->acl.add(destRealmAvp());
    msg->acl.add(authIdAvp());
    msg->acl.add(reAuthTypeAvp());

    // optional avps
    if (Attributes().Username().IsSet()) {
        DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
        unameAvp.Get() = Attributes().Username()();
        msg->acl.add(unameAvp());
    }

    orStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;
    msg->acl.add(orStateId());

    // TBD: Add more AVP's here if needed

    m_Session.TxDelivery(msg);
}

void DiameterAuthSessionServerStateMachine::RxRAA(DiameterMsg &msg)
{
    /*
      8.3.2.  Re-Auth-Answer

      The Re-Auth-Answer (RAA), indicated by the Command-Code set to 258
      and the message flags' 'R' bit clear, is sent in response to the RAR.
      The Result-Code AVP MUST be present, and indicates the disposition of
      the request.

      A successful RAA message MUST be followed by an application-specific
      authentication and/or authorization message.

      Message Format

      <RAA>  ::= < Diameter Header: 258, PXY >
                 < Session-Id >
                 { Result-Code }
                 { Origin-Host }
                 { Origin-Realm }
                 [ User-Name ]
                 [ Origin-State-Id ]
                 [ Error-Message ]
                 [ Error-Reporting-Host ]
               * [ Failed-AVP ]
               * [ Redirect-Host ]
                 [ Redirect-Host-Usage ]
                 [ Redirect-Host-Cache-Time ]
               * [ Proxy-Info ]
               * [ AVP ]
     */
    DiameterUInt32AvpContainerWidget rcodeAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
    DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
    DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
    DiameterUtf8AvpContainerWidget errMsgAvp(msg.acl);
    DiameterIdentityAvpContainerWidget errHostAvp(msg.acl);

    diameter_unsigned32_t *rcode = rcodeAvp.GetAvp(DIAMETER_AVPNAME_RESULTCODE);
    diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
    diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
    diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
    diameter_utf8string_t *errMsg = errMsgAvp.GetAvp(DIAMETER_AVPNAME_ERRORMESSAGE);
    diameter_identity_t *errHost = errHostAvp.GetAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);

    AAA_LOG((LM_INFO, "(%P|%t) *** Re-Auth answer received ***\n"));
    Attributes().MsgIdRxMessage(msg);

    DiameterSessionId sid;
    sid.Get(msg);
    sid.Dump();
    if (host) {
        AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
    }
    if (realm) {
        AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
    }
    if (uname) {
        AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
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
