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

#include "aaa_session_db.h"
#include "aaa_session_client.h"

AAA_ClientAuthSession::~AAA_ClientAuthSession()
{
    if (m_Fsm.IsRunning()) {
       Reset();
    } 
}

AAAReturnCode AAA_ClientAuthSession::Begin(char *optionValue)
{
    // re-negotiate the following values
    Attributes().SessionTimeout().IsNegotiated() = false;
    Attributes().AuthLifetime().IsNegotiated() = false;
    Attributes().AuthGrace().IsNegotiated() = false;

    // session lifetime values
    AAA_ScholarAttribute<diameter_unsigned32_t> sessTout;
    sessTout() = AAA_CFG_AUTH_SESSION()->sessionTm;
    SetSessionTimeout(sessTout);
    if (sessTout.IsSet()) {
        Attributes().SessionTimeout() = sessTout();
    }

    // auth lifetime values
    AAA_ScholarAttribute<diameter_unsigned32_t> authLifetime;
    authLifetime() = AAA_CFG_AUTH_SESSION()->lifetimeTm;
    SetAuthLifetimeTimeout(authLifetime);
    if (authLifetime.IsSet()) {
        Attributes().AuthLifetime() = authLifetime();
    }

    // sanity checks
    if (Attributes().AuthLifetime()() > 
        Attributes().SessionTimeout()()) {
        int holder = Attributes().SessionTimeout()() - 1;
        Attributes().AuthLifetime() = (holder >= 0) ? holder : holder + 1; 

        AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application sets authorization lifetime\n");
        AAA_LOG(LM_INFO, "(%P|%t)                 to be greater than session timeout, overriding to %d\n",
                Attributes().AuthLifetime()());
    }

    // grace period values
    AAA_ScholarAttribute<diameter_unsigned32_t> gracePeriod;
    gracePeriod() = AAA_CFG_AUTH_SESSION()->graceTm;
    SetAuthGracePeriodTimeout(gracePeriod);
    if (gracePeriod.IsSet()) {
        Attributes().AuthGrace() = gracePeriod();
    }

    // check for re-start
    if (! m_Fsm.IsRunning()) {
        // session state values
        AAA_ScholarAttribute<diameter_unsigned32_t> authState;
        authState() = AAA_CFG_AUTH_SESSION()->stateful;
        SetAuthSessionState(authState);
        if (authState.IsSet()) {
            Attributes().AuthSessionState() = authState();
        }

        // assign diameter id
        std::string &id = Attributes().SessionId().DiameterId();
        id = AAA_CFG_TRANSPORT()->identity + ".";
        id += AAA_CFG_TRANSPORT()->realm;
        if (optionValue) {
            Attributes().SessionId().OptionalValue() = optionValue;
        }

        // add to session database
        AAA_SESSION_DB().Add(Attributes().SessionId(), *this);
        Attributes().SessionId().Dump();

        // run the fsm
        m_Fsm.Start();
    }

    if (ReStart() == AAA_ERR_SUCCESS) {
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_REQUEST_ACCESS);
    }
    else {
        m_Fsm.Stop();
    }
    return (AAA_ERR_SUCCESS);
}

void AAA_ClientAuthSession::RxRequest(std::auto_ptr<AAAMessage> msg) 
{
    // base protocol request message filters
    if ((msg->hdr.code == AAA_MSGCODE_ABORTSESSION) &&
        (Attributes().AuthSessionState() == AAA_SESSION_STATE_MAINTAINED)) {
        m_Fsm.RxASR(*msg);
        if (AbortSession() == AAA_ERR_SUCCESS) {
            m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_ASR_OK);
        }
        else {
            m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_ASR_RETRY);
        }
    }
    else if (msg->hdr.code == AAA_MSGCODE_SESSIONTERMINATION) {
        AAA_LOG(LM_DEBUG,"(%P|%t) *** STR received in client session, discarding\n");
    }
    else if (msg->hdr.code == AAA_MSGCODE_REAUTH) {
        m_Fsm.RxRAR(*msg);
        AAA_UInt32AvpContainerWidget reAuthTypeAvp(msg->acl);
        diameter_unsigned32_t *reAuthType = reAuthTypeAvp.GetAvp(AAA_AVPNAME_REAUTHREQTYPE);
        if (reAuthType) {
           AAA_ReAuthValue value = { 0, *reAuthType };
           Attributes().ReAuthRequestValue() = value;
           m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_RAR);
        }
        else {
           AAA_LOG(LM_INFO, "(%P|%t) Re-Auth request received with no re-auth-type\n");
           Attributes().ReAuthRequestValue().Clear();
	}
    }
    else {
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_SSAR, msg);
    }
}

void AAA_ClientAuthSession::RxAnswer(std::auto_ptr<AAAMessage> msg) 
{
    // base protocol answer message filters
    if (msg->hdr.code == AAA_MSGCODE_SESSIONTERMINATION) {
        if (Attributes().AuthSessionState() == 
            AAA_SESSION_STATE_MAINTAINED) {
            m_Fsm.RxSTA(*msg);
            m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_STA);
        }
        return;
    }
    else if (msg->hdr.code == AAA_MSGCODE_ABORTSESSION) {
        AAA_LOG(LM_DEBUG,"(%P|%t) *** ASA received in client session, discarding\n");
        return;
    }

    if (! Attributes().AuthSessionState().IsNegotiated()) {
        AAA_EnumAvpContainerWidget sessionStateAvp(msg->acl);
        diameter_enumerated_t *state = sessionStateAvp.GetAvp
            (AAA_AVPNAME_AUTHSESSIONSTATE);
        if (state) {
            Attributes().AuthSessionState().Set(*state);
            AAA_LOG(LM_INFO, "(%P|%t) Server dictated session state: %d\n", 
                    Attributes().AuthSessionState()());
        }
    }

    if (! Attributes().SessionTimeout().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget timeoutAvp(msg->acl);
        diameter_unsigned32_t *tout = timeoutAvp.GetAvp
            (AAA_AVPNAME_SESSIONTIMEOUT);
        if (tout) {
            Attributes().SessionTimeout().Set(*tout + 10);
            AAA_LOG(LM_INFO, "(%P|%t) Server dictated session timeout: %d\n", 
                    Attributes().SessionTimeout()());
        }
    }

    if (! Attributes().AuthLifetime().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget lifetimeAvp(msg->acl);
        diameter_unsigned32_t *tout = lifetimeAvp.GetAvp
            (AAA_AVPNAME_AUTHLIFETIME);
        if (tout) {
            Attributes().AuthLifetime().Set(*tout);
            AAA_LOG(LM_INFO, "(%P|%t) Server dictated auth lifetime: %d\n", 
                    Attributes().AuthLifetime()());
        }
    }

    if (! Attributes().AuthGrace().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget graceAvp(msg->acl);
        diameter_unsigned32_t *tout = graceAvp.GetAvp
            (AAA_AVPNAME_AUTHGRACE);
        if (tout) {
            Attributes().AuthGrace().Set(*tout);
            AAA_LOG(LM_INFO, "(%P|%t) Server dictated grace period: %d\n", 
                    Attributes().AuthGrace()());
        }
    }

    m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_SSAA, msg);
}

void AAA_ClientAuthSession::RxError(std::auto_ptr<AAAMessage> msg) 
{
    ErrorMsg(*msg);
}

AAAReturnCode AAA_ClientAuthSession::RxDelivery(std::auto_ptr<AAAMessage> msg)
{
    typedef struct {
        AAAReturnCode m_Rc;
        AAA_Event m_AnsEvent;
        AAA_Event m_ReqEvent;
        bool m_StatefulOnly;
        bool m_ValidOnRequestMsg;
    } RxHandler;
    static RxHandler RxHandlerAry[] = {
        { AAA_ERR_INCOMPLETE, 0, 
          0, false, false },
        { AAA_ERR_SUCCESS, AAA_SESSION_AUTH_EV_SSAA_OK, 
          AAA_SESSION_AUTH_EV_SSAR_OK, false, true },
        { AAA_ERR_NOSERVICE, AAA_SESSION_AUTH_EV_SSAA_NOSVC, 
          0, true, false },
        { AAA_ERR_MSG_UNPROCESSED, AAA_SESSION_AUTH_EV_SSAA_ERROR, 
          0, true, false },
        { AAA_ERR_FAILURE, AAA_SESSION_AUTH_EV_SSAA_FAIL, 
          AAA_SESSION_AUTH_EV_SSAR_FAIL, false, true }
    };
    Attributes().MsgIdRxMessage(*msg);
    AAAReturnCode rc = (msg->hdr.flags.r) ? RequestMsg(*msg) : AnswerMsg(*msg);
    for (int i=0; i < sizeof(RxHandlerAry)/sizeof(RxHandler); i++) {
        if (RxHandlerAry[i].m_Rc == rc) {
            if (msg->hdr.flags.r) {
               if (! RxHandlerAry[i].m_ValidOnRequestMsg) {
                   AAA_LOG(LM_INFO, "(%P|%t) Invalid return value (INCOMPLETE, NOSERVICE or UNPROCESSED) in request msg\n");
                   break;
               }
               if (RxHandlerAry[i].m_ReqEvent != 0) {
                   m_Fsm.Notify(RxHandlerAry[i].m_ReqEvent);
               }
            }
            else {
               if ((RxHandlerAry[i].m_StatefulOnly) &&
                    (Attributes().AuthSessionState()() !=
                     AAA_SESSION_STATE_MAINTAINED)) {
                   AAA_LOG(LM_INFO, "(%P|%t) Invalid return value (NOSERVICE or UNPROCESSED) in stateless session\n");
                   break;
               }
               if (RxHandlerAry[i].m_AnsEvent != 0) {
                   m_Fsm.Notify(RxHandlerAry[i].m_AnsEvent);
               }
            }
            return (AAA_ERR_SUCCESS);
        }
    }
    return (AAA_ERR_FAILURE);
}

AAAReturnCode AAA_ClientAuthSession::Send(std::auto_ptr<AAAMessage> msg) 
{
    if (! m_Fsm.IsRunning()) {
        return (AAA_ERR_FAILURE);
    }
    if ((m_Fsm.State() == AAA_SESSION_AUTH_ST_IDLE) &&
        (! Attributes().AuthSessionState().IsNegotiated())) {
         // send the session state hint
         AAA_EnumAvpContainerWidget sessionStateAvp(msg->acl);
         sessionStateAvp.AddAvp
             (AAA_AVPNAME_AUTHSESSIONSTATE) =
         Attributes().AuthSessionState()();
    }

    // send hint for session timeout
    if (! Attributes().SessionTimeout().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget sessToutAvp(msg->acl);
        diameter_unsigned32_t *tout = sessToutAvp.GetAvp
                (AAA_AVPNAME_SESSIONTIMEOUT);
        if (! tout) {
            sessToutAvp.AddAvp(AAA_AVPNAME_SESSIONTIMEOUT) =
                    Attributes().SessionTimeout()();
        }
        else if (Attributes().SessionTimeout()() < *tout) {
            AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application session timeout\n");
            AAA_LOG(LM_INFO, "(%P|%t)                 greater than configuration or callback, overriding\n");
            *tout = Attributes().SessionTimeout()();
        }
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Using applications session timeout settings\n");
            Attributes().SessionTimeout() = *tout;
	}
    }

    // send hint for authorization lifetime
    if (! Attributes().AuthLifetime().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget authLifetimeAvp(msg->acl);
        diameter_unsigned32_t *tout = authLifetimeAvp.GetAvp
                (AAA_AVPNAME_AUTHLIFETIME);
        if (! tout) {
           authLifetimeAvp.AddAvp(AAA_AVPNAME_AUTHLIFETIME) =
                    Attributes().AuthLifetime()();
        }
        else if (Attributes().AuthLifetime()() < *tout) {
           AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application authorization lifetime\n");
           AAA_LOG(LM_INFO, "(%P|%t)                 greater than configuration or callback, overriding\n");
            *tout = Attributes().AuthLifetime()();
        }
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Using applications auth lifetime settings\n");
            Attributes().AuthLifetime() = *tout;
	}
    }

    m_Fsm.Notify(AAA_SESSION_AUTH_EV_TX_SSAR, msg);
    return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAA_ClientAuthSession::End()
{
    if (m_Fsm.IsRunning()) {
        AAA_SESSION_EV_AUTH ev = (Attributes().AuthSessionState()() == 
                                  AAA_SESSION_STATE_MAINTAINED) ? 
                                  AAA_SESSION_AUTH_EV_ABORT :
                                  AAA_SESSION_AUTH_EV_STOP;
        m_Fsm.Notify(ev);
    }
    return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAA_ClientAuthSession::Reset()
{
    m_Fsm.Stop();
    AAA_AuthSession::Reset();
    AAA_SESSION_DB().Remove(Attributes().SessionId());
    return (AAA_ERR_SUCCESS);
}

AAA_ClientAcctSession::AAA_ClientAcctSession
(AAA_Task &task, diameter_unsigned32_t id, 
 char *optionalValue) :
    m_Task(task),
    m_ApplicationId(id),
    m_SubSessionId(0)
{
    // assign diameter id
    std::string &sid = m_SessionId.DiameterId();
    sid = AAA_CFG_TRANSPORT()->identity + ".";
    sid += AAA_CFG_TRANSPORT()->realm;
    if (optionalValue) {
        m_SessionId.OptionalValue() = optionalValue;
    }

    // add to session database
    AAA_SESSION_DB().Add(m_SessionId, *this);
    m_SessionId.Dump();
}

AAA_ClientAcctSession::~AAA_ClientAcctSession() 
{
    // remove from session id
    AAA_SESSION_DB().Remove(m_SessionId);
}
        
AAAReturnCode AAA_ClientAcctSession::RegisterSubSession
(AAA_AcctSession &s)
{
    if (m_SubSessionMap.find(s.Attributes().SubSessionId()()) ==
        m_SubSessionMap.end()) {
        s.Attributes().SessionId() = m_SessionId;
        s.Attributes().SubSessionId() = ++ m_SubSessionId;
        m_SubSessionMap.insert
	    (std::pair<diameter_unsigned64_t, AAA_AcctSession*>
	     (m_SubSessionId, &s));
        return (AAA_ERR_SUCCESS);
    }
    return (AAA_ERR_FAILURE);
}

AAAReturnCode AAA_ClientAcctSession::RemoveSubSession
(diameter_unsigned64_t &id)
{
    AAA_SubSessionMap::iterator i = m_SubSessionMap.find(id);
    if (i != m_SubSessionMap.end()) {
        m_SubSessionMap.erase(i);
        return (AAA_ERR_SUCCESS);
    }
    return (AAA_ERR_FAILURE);
}

AAAReturnCode AAA_ClientAcctSession::Send
(std::auto_ptr<AAAMessage> msg)
{
    // stub, should not be used
    return (AAA_ERR_FAILURE);
}

void AAA_ClientAcctSession::RxRequest
(std::auto_ptr<AAAMessage> msg)
{  
    AAA_UInt64AvpContainerWidget subSessionIdAvp(msg->acl);
    diameter_unsigned64_t *subSid = subSessionIdAvp.GetAvp
                 (AAA_AVPNAME_ACCTSUBSID);
    if (subSid) {
        AAA_SubSessionMap::iterator i = m_SubSessionMap.find(*subSid);
        if (i != m_SubSessionMap.end()) {
            i->second->RxRequest(msg);
            return;
        }
    }
    AAA_LOG(LM_INFO, "(%P|%t) WARNING: Sub session id not found\n");
}

void AAA_ClientAcctSession::RxAnswer
(std::auto_ptr<AAAMessage> msg)
{
    AAA_UInt64AvpContainerWidget subSessionIdAvp(msg->acl);
    diameter_unsigned64_t *subSid = subSessionIdAvp.GetAvp
                 (AAA_AVPNAME_ACCTSUBSID);
    if (subSid) {
        AAA_SubSessionMap::iterator i = m_SubSessionMap.find(*subSid);
        if (i != m_SubSessionMap.end()) {
            i->second->RxAnswer(msg);
            return;
        }
    }
    AAA_LOG(LM_INFO, "(%P|%t) WARNING: Sub session id not found\n");
}

void AAA_ClientAcctSession::RxError
(std::auto_ptr<AAAMessage> msg)
{
    AAA_UInt64AvpContainerWidget subSessionIdAvp(msg->acl);
    diameter_unsigned64_t *subSid = subSessionIdAvp.GetAvp
                 (AAA_AVPNAME_ACCTSUBSID);
    if (subSid) {
        AAA_SubSessionMap::iterator i = m_SubSessionMap.find(*subSid);
        if (i != m_SubSessionMap.end()) {
            i->second->RxError(msg);
            return;
        }
    }
    AAA_LOG(LM_INFO, "(%P|%t) WARNING: Sub session id not found\n");
}












