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

#ifndef __AAA_SESSION_H__
#define __AAA_SESSION_H__

#include "framework.h"
#include "aaa_data_defs.h"
#include "aaa_session_attributes.h"
#include "aaa_route_msg_router.h"
#include "aaa_garbage_collector.h"

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionIO :
    public AAA_JobData
{
    public:
        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<DiameterMsg> msg) = 0;

        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<DiameterMsg> msg) = 0;

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<DiameterMsg> msg) = 0;

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<DiameterMsg> msg) = 0;

        virtual ~DiameterSessionIO() {
	}
};

template<class ATTRIBUTE>
class DiameterSession :
    public DiameterSessionIO,
    public DiameterGarbageCollectorAttribute
{
    public:
        virtual ~DiameterSession() {
        }

        /// This function is used for setting Auth-Session-State 
        virtual void SetSessionTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
        }

        /// This function is used for setting the destination host
        virtual void SetDestinationHost
        (DiameterScholarAttribute<diameter_identity_t> &dHost)
        {
        }

        /// This function is used for setting the destination realm
        virtual void SetDestinationRealm
        (DiameterScholarAttribute<diameter_identity_t> &dRealm)
        {
        }

        /// This function is used for setting the Username attribute
        virtual void SetUsername
        (DiameterScholarAttribute<diameter_utf8string_t> &uname)
        {
        }

        /// This function is called when incomming request message is received
        virtual AAAReturnCode ReStart() {
            // For AAA clients, initialization can be
            // done here and a service specific request
            // message should be sent here. If successful
            // an AAA_ERR_SUCCESS must be returned else 
            // it is considered a failure. For AAA servers, 
            // the application can do initialization routines 
            // here and the return value of this handler is
            // ignored by the library.
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming request message is received
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // Incomming request messages are received here.
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming answer message is received
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming answer message is received
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when session has gone to open state
        virtual AAAReturnCode Success() {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called on session disconnection
        virtual AAAReturnCode Disconnect() {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when a stateless session is getting reclaimed
        virtual AAAReturnCode ReClaimSession() {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called on session timeout
        virtual AAAReturnCode SessionTimeout() {
            return (AAA_ERR_SUCCESS);
        }

        /// This fucntion is called internally to handle messages
        virtual AAAReturnCode RxDelivery(std::auto_ptr<DiameterMsg> msg) {
            // by default, discard msg
            return (AAA_ERR_SUCCESS);
	}

        /// This fucntion is called internally to route messages
        virtual AAAReturnCode TxDelivery(std::auto_ptr<DiameterMsg> msg);

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

        /// This function returns the current session attributes
        ATTRIBUTE &Attributes() {
            return m_Attributes;
        }

    protected:
        DiameterSession(diameter_unsigned32_t appId) {
           Reset();
           m_Attributes.ApplicationId() = appId;
	}

    protected:
        ATTRIBUTE m_Attributes;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterAuthSession :
    public DiameterSession<DiameterAuthSessionAttributes>
{
    public:
        virtual ~DiameterAuthSession() {
        }

        /// This function is used for setting Auth-Session-State 
        virtual void SetAuthLifetimeTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
        }

        /// This function is used for setting Auth-Session-State 
        virtual void SetAuthGracePeriodTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
        }

        /// This function is used for setting Class AVP contents in STR msg
        virtual void SetClassAvp
        (DiameterScholarAttribute<diameter_octetstring_t> &cls)
        {
        }

        /// This function called when an STA message contains a Class AVP
        virtual void ClassAvp
        (DiameterScholarAttribute<diameter_octetstring_t> &cls)
        {
        }

        /// This function is used for setting Auth-Session-State 
        virtual void SetAuthSessionState
        (DiameterScholarAttribute<diameter_unsigned32_t> &authState)
        {
        }

        /// This function is called when session has gone to open state
        virtual AAAReturnCode ReAuthenticate(diameter_unsigned32_t value) {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called on lifetime timeout
        virtual AAAReturnCode AuthorizationTimeout() {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called on rx of ASR
        virtual AAAReturnCode AbortSession() {
            return (AAA_ERR_SUCCESS);
        }

        virtual AAAReturnCode Reset() {
           m_Attributes.AuthSessionState().IsNegotiated() = false;
           return DiameterSession<DiameterAuthSessionAttributes>::Reset();
	}

    protected:
        DiameterAuthSession(diameter_unsigned32_t appId) : 
		DiameterSession<DiameterAuthSessionAttributes>(appId) {
           m_Attributes.SessionTimeout() = DIAMETER_CFG_AUTH_SESSION()->sessionTm;
           m_Attributes.AuthLifetime() = DIAMETER_CFG_AUTH_SESSION()->lifetimeTm;
           m_Attributes.AuthGrace() = DIAMETER_CFG_AUTH_SESSION()->graceTm;
           m_Attributes.AuthSessionState() = DIAMETER_CFG_AUTH_SESSION()->stateful;
	}
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterAcctSession :
    public DiameterSession<DiameterAcctSessionAttributes>
{
    public:
        virtual ~DiameterAcctSession() {
        }

        /// This function is used for setting realtime required AVP
        virtual void SetRealTimeRequired
        (DiameterScholarAttribute<diameter_enumerated_t> &rt)
        {
        }

        /// This function is used for setting acct interim interval
        virtual void SetInterimInterval
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
        }

        /// This function is used for setting RADIUS acct session id
        virtual void SetRadiusAcctSessionId
        (DiameterScholarAttribute<diameter_octetstring_t> &sid)
        {
        }

        /// This function is used for setting multi-session id
        virtual void SetMultiSessionId
        (DiameterScholarAttribute<diameter_utf8string_t> &sid)
        {
        }

        /// This function is called when record processing fails
        virtual AAAReturnCode Failed(int recNum) {
            return (AAA_ERR_SUCCESS);
        }

    protected:
        DiameterAcctSession(diameter_unsigned32_t appId) :
		DiameterSession<DiameterAcctSessionAttributes>(appId) {
           m_Attributes.SessionTimeout() = DIAMETER_CFG_ACCT_SESSION()->sessionTm;
           m_Attributes.InterimInterval() = DIAMETER_CFG_ACCT_SESSION()->recIntervalTm;
           m_Attributes.RealtimeRequired() = DIAMETER_CFG_ACCT_SESSION()->realtime;
           m_Attributes.RecordType() = DIAMETER_ACCT_RECTYPE_EVENT;
           m_Attributes.RecordNumber() = 0;
           m_Attributes.BackwardCompatibility() = false;
	}
};

#include "aaa_session.inl"

#endif



