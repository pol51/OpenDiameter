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

#include "aaa_session_db.h"
#include "aaa_session_msg_rx.h"

int DiameterSessionMsgRx::RxLocalMsgHandler::Request
  (std::auto_ptr<DiameterMsg> &msg, 
   DiameterPeerEntry *source, 
   DiameterPeerEntry *dest)
{
    // Note that request message needs to be 
    // maintained and will be delete by the
    // router when a corresponding answer 
    // is received

    DiameterSessionId sid;
    if (sid.Get(*msg) == AAA_ERR_SUCCESS) {
        AAA_JobData *data = NULL;
        if (DIAMETER_SESSION_DB().Lookup(sid, data)) {
            try {
               DiameterSessionIO *io = (DiameterSessionIO*)data;
               io->RxRequest(msg);
            }
            catch (...) {
            }
            return (0);
        }
        else if (m_SessionRx.m_SessionFactoryMap.Empty()) {
            m_SessionRx.RxUnknownSession(msg);
            return (0);
        }
        DiameterServerSessionFactory *factory = m_SessionRx.m_SessionFactoryMap.Lookup
            (msg->hdr.appId);
        if (factory) {
            DiameterSessionIO *newSession = factory->CreateInstance();
            if (newSession) {
                try {
                    newSession->RxRequest(msg);
                    DIAMETER_SESSION_DB().Add(sid, *newSession);
                }
                catch (DiameterBaseException &e) {
                    if (e.Code() == 
                        DiameterBaseException::MISSING_SESSION_ID) {
                        delete newSession;
                    }
                    AAA_LOG((LM_DEBUG,"(%P|%t) *** Processing error in new session ***\n"));
                }
                catch (...) {
                }
                return (0);
            }
            else {
                AAA_LOG((LM_DEBUG,"(%P|%t) *** Failed to create new session, discarding req msg ***\n"));
            }
        }
        else {
            AAA_LOG((LM_DEBUG,"(%P|%t) *** Un-supported application id, discarding req msg ***\n"));
        }
    }
    else {
        AAA_LOG((LM_DEBUG,"(%P|%t) *** Missing session id, discard req msg ***\n"));
    }
    DiameterMsgHeaderDump::Dump(*msg);
    return (-1);
}

int DiameterSessionMsgRx::RxLocalMsgHandler::Answer
  (std::auto_ptr<DiameterMsg> &msg, 
   DiameterPeerEntry *source, 
   DiameterPeerEntry *dest)
{
    // Note that answer message will be delete
    // when this function exits since they are
    // not stored by the router

    DiameterSessionId sid;
    if (sid.Get(*msg) == AAA_ERR_SUCCESS) {
        AAA_JobData *data = NULL;
        if (DIAMETER_SESSION_DB().Lookup(sid, data)) {
            try {
               DiameterSessionIO *io = (DiameterSessionIO*)data;
               io->RxAnswer(msg);
            }
            catch (...) {
            }
            return (0);
        }
        else {
            AAA_LOG((LM_DEBUG,"(%P|%t) *** Unknown session id, discard answer msg ***\n"));
        }
    }
    else {
        AAA_LOG((LM_DEBUG,"(%P|%t) *** Missing session id, discard answer msg ***\n"));
    }
    DiameterMsgHeaderDump::Dump(*msg);
    return (-1);
}

AAAReturnCode DiameterSessionMsgRx::RxUnknownSession
  (std::auto_ptr<DiameterMsg> msg)
{
    // special base protocol handling for unknown session id
    if (msg->hdr.code == DIAMETER_MSGCODE_ABORTSESSION) {
        TxASA(msg);
        return (AAA_ERR_SUCCESS);
    }
    AAA_LOG((LM_DEBUG,"(%P|%t) *** Unknown session, discarding msg***\n"));
    return (AAA_ERR_FAILURE);
}

int DiameterSessionMsgRx::RxProxyMsgHandler::Request
  (std::auto_ptr<DiameterMsg> &msg, 
   DiameterPeerEntry *source, 
   DiameterPeerEntry *dest)
{
    // Note that request message needs to be 
    // maintained and will be delete by the
    // router when a corresponding answer 
    // is received
    AAA_ProxyHandler *handler = m_SessionRx.m_ProxyHandlerMap.Lookup
        (msg->hdr.appId);
    if (handler) {
        return (handler->RequestMsg(*msg) == AAA_ERR_SUCCESS) ? 0 : (-1);
    }
    else {
        AAA_LOG((LM_DEBUG,"(%P|%t) *** No proxy support for appId=%d, forwarding message ***\n",
                msg->hdr.appId));
    }
    DiameterMsgHeaderDump::Dump(*msg);
    return (-1);
}

int DiameterSessionMsgRx::RxProxyMsgHandler::Answer
  (std::auto_ptr<DiameterMsg> &msg, 
   DiameterPeerEntry *source, 
   DiameterPeerEntry *dest)
{
    // Note that answer message will be delete
    // when this function exits since they are
    // not stored by the router
    AAA_ProxyHandler *handler = m_SessionRx.m_ProxyHandlerMap.Lookup
        (msg->hdr.appId);
    if (handler) {
        handler->AnswerMsg(*msg);
        return (0);
    }
    return (-1);
}

int DiameterSessionMsgRx::RxErrorMsgHandler::LocalErrorHandling
  (std::auto_ptr<DiameterMsg> &msg, 
   DiameterPeerEntry *source, 
   DiameterPeerEntry *dest) 
{
    DiameterSessionId sid;
    if (sid.Get(*msg) == AAA_ERR_SUCCESS) {
        AAA_ProxyHandler *handler = NULL;
        AAA_JobData *data = NULL;
        if (DIAMETER_SESSION_DB().Lookup(sid, data)) {
            try {
               DiameterSessionIO *io = (DiameterSessionIO*)data;
               io->RxError(msg);
            }
            catch (...) {
            }
            return (0);
        }
        else if ((handler = m_SessionRx.m_ProxyHandlerMap.Lookup(msg->hdr.appId))) {
            handler->ErrorMsg(*msg);
            return (0);
        }
        else {
            AAA_LOG((LM_DEBUG,"(%P|%t) *** Error message with no local hanlder, resume routing ***\n"));
        }
    }
    else {
        AAA_LOG((LM_DEBUG,"(%P|%t) *** Error message with missing session id ***\n"));
    }
    AAA_LOG((LM_DEBUG, "(%P|%t) Discarding message\n"));
    return (-1);
}

void DiameterSessionMsgRx::TxASA(std::auto_ptr<DiameterMsg> &asr)
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

   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   ACE_OS::memset(&msg->hdr, 0, sizeof(msg->hdr));
   msg->hdr.ver = DIAMETER_PROTOCOL_VERSION;
   msg->hdr.length = 0;
   msg->hdr.flags.r = DIAMETER_FLAG_CLR;
   msg->hdr.flags.p = DIAMETER_FLAG_CLR;
   msg->hdr.flags.e = DIAMETER_FLAG_CLR;
   msg->hdr.code = DIAMETER_MSGCODE_ABORTSESSION;
   msg->hdr.appId = DIAMETER_BASE_APPLICATION_ID;

   // required
   DiameterSessionId sid;
   sid.Get(*asr);
   sid.Set(*msg);
   msg->hdr.hh = asr->hdr.hh;
   msg->hdr.ee = asr->hdr.ee;

   DiameterUInt32AvpWidget rcodeAvp(DIAMETER_AVPNAME_RESULTCODE);
   DiameterIdentityAvpWidget orHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget orRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
   DiameterUtf8AvpWidget orErrMsgAvp(DIAMETER_AVPNAME_ERRORMESSAGE);
   DiameterIdentityAvpWidget orErrHostAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);

   rcodeAvp.Get() = AAA_UNKNOWN_SESSION_ID;
   orHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
   orRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
   orErrMsgAvp.Get() = std::string("Unknown Session Id");
   orErrHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;

   msg->acl.add(rcodeAvp());
   msg->acl.add(orHostAvp());
   msg->acl.add(orRealmAvp());
   msg->acl.add(orErrMsgAvp());
   msg->acl.add(orErrHostAvp());

   DIAMETER_MSG_ROUTER()->AnswerMsg(msg, 0);
}
