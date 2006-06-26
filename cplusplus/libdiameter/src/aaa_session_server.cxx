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

#include "aaa_session_server.h"
#include "aaa_session_db.h"

AAA_ServerAuthSession::AAA_ServerAuthSession(AAA_Task &task,
                                             diameter_unsigned32_t id) :
    AAA_AuthSession(id),
    m_Fsm(task, *this) 
{
    // start the fsm
    if (ReStart() != AAA_ERR_SUCCESS) {
        delete this;
        return;
    }
    m_Fsm.Start();
}

AAAReturnCode AAA_ServerAuthSession::Send(std::auto_ptr<AAAMessage> msg)
{
    if (m_Fsm.IsRunning()) {
        AAA_Event ev = (msg->hdr.flags.r) ? AAA_SESSION_AUTH_EV_TX_SSAR : 
                                            AAA_SESSION_AUTH_EV_TX_SSAA;
        m_Fsm.Notify(ev, msg);
        return (AAA_ERR_SUCCESS);
    }
    return (AAA_ERR_FAILURE);
}

AAAReturnCode AAA_ServerAuthSession::ReAuth(diameter_unsigned32_t type)
{
    if (m_Fsm.IsRunning()) {
        AAA_ReAuthValue value = { 0, type };
        Attributes().ReAuthRequestValue() = value;
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_REAUTH);
        return (AAA_ERR_SUCCESS);
    }
    return (AAA_ERR_FAILURE);
}

AAAReturnCode AAA_ServerAuthSession::End()
{
    if (m_Fsm.IsRunning()) {
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_ABORT);
        return (AAA_ERR_SUCCESS);
    }
    return (AAA_ERR_FAILURE);
}

void AAA_ServerAuthSession::RxRequest(std::auto_ptr<AAAMessage> msg)
{
    // filter session id
    if (Attributes().SessionId().IsEmpty()) {
        AAA_SessionId sid;
        if (sid.Get(*msg)) {
            AAA_LOG(LM_DEBUG,"(%P|%t) *** Fatal, failed session id\n");
            return;
        }
        Attributes().SessionId() = sid;
        AAA_LOG(LM_DEBUG,"(%P|%t) New auth session\n");
        sid.Dump();
    }

    // base protocol request message filters
    if ((msg->hdr.code == AAA_MSGCODE_SESSIONTERMINATION) &&
        (Attributes().AuthSessionState() == AAA_SESSION_STATE_MAINTAINED)) {
        m_Fsm.RxSTR(*msg);
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_STR);
        return;
    }
    else if (msg->hdr.code == AAA_MSGCODE_ABORTSESSION) {
        AAA_LOG(LM_DEBUG,"(%P|%t) *** ASR received in server session, discarding\n");
        return;
    }

    AAA_IdentityAvpContainerWidget oHostAvp(msg->acl);
    AAA_IdentityAvpContainerWidget oRealmAvp(msg->acl);

    // filter out origin host
    diameter_identity_t *oHost = oHostAvp.GetAvp
                    (AAA_AVPNAME_ORIGINHOST);
    if (oHost && 
        (! m_Attributes.DestinationHost().IsSet()) && 
        (Attributes().AuthSessionState() == AAA_SESSION_STATE_MAINTAINED)) {
	   m_Attributes.DestinationHost().Set(*oHost);
    }

    // filter out origin realm
    diameter_identity_t *oRealm = oRealmAvp.GetAvp
                    (AAA_AVPNAME_ORIGINREALM);
    if (oRealm && 
        (! m_Attributes.DestinationRealm().IsSet()) && 
        (Attributes().AuthSessionState() == AAA_SESSION_STATE_MAINTAINED)) {
	   m_Attributes.DestinationRealm().Set(*oRealm);
    }

    // filter auth session state and negotiate
    if (! Attributes().AuthSessionState().IsNegotiated()) {
        AAA_EnumAvpContainerWidget sessionStateAvp(msg->acl);
        diameter_enumerated_t *state = sessionStateAvp.GetAvp
                (AAA_AVPNAME_AUTHSESSIONSTATE);
        if (state) {
            // session state policy negotiation is:
            if (Attributes().AuthSessionState()() != *state) {
                if (Attributes().AuthSessionState() == 
                    AAA_SESSION_STATE_MAINTAINED) {
                    // downgrade server to client (no state)
                    Attributes().AuthSessionState() =
                        AAA_SESSION_NO_STATE_MAINTAINED;
                    Attributes().AuthSessionState().Set(*state);
                }
            }
            Attributes().AuthSessionState().IsNegotiated() = true;
        }
        else {
            AAA_ScholarAttribute<diameter_unsigned32_t> authState;
            authState() = AAA_CFG_AUTH_SESSION()->stateful;
            SetAuthSessionState(authState);
            if (authState.IsSet()) {
               Attributes().AuthSessionState().Set(authState());
            }
            else {
               Attributes().AuthSessionState().Set(AAA_CFG_AUTH_SESSION()->stateful);
            }
        }
        AAA_LOG(LM_DEBUG,"(%P|%t) Negotiated session state: %d\n", 
                Attributes().AuthSessionState()());
    }

    // capture session timeout hint from client
    if (! Attributes().SessionTimeout().IsNegotiated()) {

        AAA_ScholarAttribute<diameter_unsigned32_t> sessTout;
        AAA_UInt32AvpContainerWidget timeoutAvp(msg->acl);
        diameter_unsigned32_t *tout = timeoutAvp.GetAvp
            (AAA_AVPNAME_SESSIONTIMEOUT);
        if (tout) {
            sessTout() = *tout;
	}
        else {
            sessTout() = AAA_CFG_AUTH_SESSION()->sessionTm;
	}

        SetSessionTimeout(sessTout);
        if (sessTout.IsSet()) {
            Attributes().SessionTimeout() = sessTout() - 10;
        }

        if (tout) {
            if (*tout < Attributes().SessionTimeout()()) {
                Attributes().SessionTimeout() = *tout;
                AAA_LOG(LM_INFO, "(%P|%t) Accepted client session timeout hint: %d\n", 
                        Attributes().SessionTimeout()());
            }
        }
    }

    // capture auth lifetime hint from client
    if (! Attributes().AuthLifetime().IsNegotiated()) {

        AAA_ScholarAttribute<diameter_unsigned32_t> authLifetime;
        AAA_UInt32AvpContainerWidget lifetimeAvp(msg->acl);
        diameter_unsigned32_t *tout = lifetimeAvp.GetAvp
            (AAA_AVPNAME_AUTHLIFETIME);
        if (tout) {
            authLifetime() = *tout;
	}
	else {
            authLifetime() = AAA_CFG_AUTH_SESSION()->lifetimeTm;
	}

        SetAuthLifetimeTimeout(authLifetime);
        if (authLifetime.IsSet()) {
            Attributes().AuthLifetime() = authLifetime();
        }

        if (tout) {
            if (*tout < Attributes().AuthLifetime()()) {
                Attributes().AuthLifetime() = *tout;
                AAA_LOG(LM_INFO, "(%P|%t) Accepted client auth lifetime hint: %d\n", 
                        Attributes().AuthLifetime()());
            }
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
    }

    // check for grace period
    if (! Attributes().AuthGrace().IsNegotiated()) {
        AAA_ScholarAttribute<diameter_unsigned32_t> authGrace;
        SetAuthGracePeriodTimeout(authGrace);
        if (authGrace.IsSet()) {
            Attributes().AuthGrace() = authGrace();
        }
    }

    m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_SSAR, msg);
}

void AAA_ServerAuthSession::RxAnswer(std::auto_ptr<AAAMessage> msg)
{
    // base protocol answer message filters
    if ((msg->hdr.code == AAA_MSGCODE_ABORTSESSION) &&
        (Attributes().AuthSessionState() == AAA_SESSION_STATE_MAINTAINED)) {
        if (m_Fsm.ASRSent()) {
            m_Fsm.RxASA(*msg);
            AAA_MsgResultCode rcode(*msg);
            if (rcode.InterpretedResultCode() == 
                AAA_MsgResultCode::RCODE_SUCCESS) {
                m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_ASA_OK);
	    }
            else {
                m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_ASA_FAIL);
	    }
        }
        else {
            AAA_LOG(LM_DEBUG,"(%P|%t) *** ASA received with no ASR sent, discarding\n");
        }
    }
    else if (msg->hdr.code == AAA_MSGCODE_SESSIONTERMINATION) {
        AAA_LOG(LM_DEBUG,"(%P|%t) *** STA received in server session, discarding\n");
    }
    else if (msg->hdr.code == AAA_MSGCODE_REAUTH) {
        m_Fsm.RxRAA(*msg);
        AAA_UInt32AvpContainerWidget rcodeAvp(msg->acl);
        diameter_unsigned32_t *rcode = rcodeAvp.GetAvp(AAA_AVPNAME_RESULTCODE);
        if (rcode) {
            AAA_ReAuthValue value = { *rcode, 0 };
            Attributes().ReAuthRequestValue() = value;
            m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_RAA);
	}
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Re-Auth answer received with no result-code\n");
            Attributes().ReAuthRequestValue().Clear();
	}
    }
    else {
        m_Fsm.Notify(AAA_SESSION_AUTH_EV_RX_SSAA, msg);
    }
}

void AAA_ServerAuthSession::RxError(std::auto_ptr<AAAMessage> msg)
{
    ErrorMsg(*msg);
}

AAAReturnCode AAA_ServerAuthSession::TxDelivery(std::auto_ptr<AAAMessage> msg)
{
    // filter auth session state and negotiate
    AAA_EnumAvpContainerWidget sessionStateAvp(msg->acl);
    diameter_enumerated_t *state = sessionStateAvp.GetAvp
            (AAA_AVPNAME_AUTHSESSIONSTATE);
    if (! state) {
        sessionStateAvp.AddAvp(AAA_AVPNAME_AUTHSESSIONSTATE) =
                Attributes().AuthSessionState()();
    }
    else if (Attributes().AuthSessionState()() != *state) {
        AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application sending auth state\n");
        AAA_LOG(LM_INFO, "(%P|%t)                 not matching base protocol, overriding\n");
        *state = Attributes().AuthSessionState()();
    }

    // dictate the session timeout
    if (! Attributes().SessionTimeout().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget timeoutAvp(msg->acl);
        diameter_unsigned32_t *tout = timeoutAvp.GetAvp
            (AAA_AVPNAME_SESSIONTIMEOUT);
        if (! tout) {
            timeoutAvp.AddAvp(AAA_AVPNAME_SESSIONTIMEOUT) =
                    Attributes().SessionTimeout()();
        }
        else if (Attributes().SessionTimeout()() < *tout) {
            AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application sending session timeout\n");
            AAA_LOG(LM_INFO, "(%P|%t)                 greater than configuration or callback, overriding\n");
            *tout = Attributes().SessionTimeout()();
        }
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Using applications session timeout settings\n");
            Attributes().SessionTimeout() = *tout;
	}
        Attributes().SessionTimeout().IsNegotiated() = true;
    }

    // dictate the auth lifetime
    if (! Attributes().AuthLifetime().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget timeoutAvp(msg->acl);
        diameter_unsigned32_t *tout = timeoutAvp.GetAvp
            (AAA_AVPNAME_AUTHLIFETIME);
        if (! tout) {
            timeoutAvp.AddAvp(AAA_AVPNAME_AUTHLIFETIME) =
                    Attributes().AuthLifetime()();
        }
        else if (Attributes().AuthLifetime()() < *tout) {
            AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application sending auth lifetime \n");
            AAA_LOG(LM_INFO, "(%P|%t)                 greater than configuration or callback, overriding\n"); 
            *tout = Attributes().AuthLifetime()();
        }
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Using applications auth lifetime settings\n");
            Attributes().AuthLifetime() = *tout;
	}
        Attributes().AuthLifetime().IsNegotiated() = true;
    }

    // dictate the grace period
    if (! Attributes().AuthGrace().IsNegotiated()) {
        AAA_UInt32AvpContainerWidget timeoutAvp(msg->acl);
        diameter_unsigned32_t *tout = timeoutAvp.GetAvp
            (AAA_AVPNAME_AUTHGRACE);
        if (! tout) {
            timeoutAvp.AddAvp(AAA_AVPNAME_AUTHGRACE) =
                    Attributes().AuthGrace()();
        }
        else if (Attributes().AuthGrace()() < *tout) {
            AAA_LOG(LM_INFO, "(%P|%t) !!! WARNING !!! application sending grace period \n");
            AAA_LOG(LM_INFO, "(%P|%t)                 greater than configuration or callback, overriding\n");
            *tout = Attributes().AuthGrace()();
        }
        else {
            AAA_LOG(LM_INFO, "(%P|%t) Using applications auth grace period settings\n");
            Attributes().AuthGrace() = *tout;
	}
        Attributes().AuthGrace().IsNegotiated() = true;
    }

    return AAA_AuthSession::TxDelivery(msg);
}

AAAReturnCode AAA_ServerAuthSession::RxDelivery(std::auto_ptr<AAAMessage> msg)
{
    Attributes().MsgIdRxMessage(*msg);
    AAAReturnCode rc = (msg->hdr.flags.r) ? RequestMsg(*msg) : AnswerMsg(*msg);
    if (Attributes().AuthSessionState()() == AAA_SESSION_STATE_MAINTAINED) {
        AAA_Event ev = 0; 
        if (rc == AAA_ERR_SUCCESS) {
            ev = (msg->hdr.flags.r) ? AAA_SESSION_AUTH_EV_SSAR_OK :
                                      AAA_SESSION_AUTH_EV_SSAA_OK;
        }
        else if (rc != AAA_ERR_INCOMPLETE) {
            ev = (msg->hdr.flags.r) ? AAA_SESSION_AUTH_EV_SSAR_FAIL :
                                      AAA_SESSION_AUTH_EV_SSAA_FAIL;
        }
        m_Fsm.Notify(ev);
    }
    return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAA_ServerAuthSession::Reset()
{
    AAA_AuthSession::Reset();
    AAA_SESSION_DB().Remove(Attributes().SessionId());
    m_Fsm.Stop();

    // WARNING!!!: schedule this object for destruction
    AAA_AUTH_SESSION_GC().ScheduleForDeletion(*this);
    return (AAA_ERR_SUCCESS);
}

