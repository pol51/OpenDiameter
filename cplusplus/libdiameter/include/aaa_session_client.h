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

#ifndef __AAA_SESSION_CLIENT_H__
#define __AAA_SESSION_CLIENT_H__

#include "aaa_session_auth_client_fsm.h"
#include "aaa_session_acct_client_fsm.h"

///
/// This class provides all the functionality of
/// an Diameter client auth session. Applications
/// needs to create an instance of this object
/// or derived from this object to create a diameter
/// client session.
///
class DIAMETERBASEPROTOCOL_EXPORT DiameterClientAuthSession :
    public DiameterAuthSession 
{
    public:
        DiameterClientAuthSession(AAA_Task &task,
                                  diameter_unsigned32_t id) :
           DiameterAuthSession(id),
           m_Fsm(task, *this) {
        }
        virtual ~DiameterClientAuthSession(); 

        // This function initializes an AAA client session
        AAAReturnCode Begin(char *optionValue = 0);

        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<DiameterMsg> msg);

        /// This function terminates the AAA session
        AAAReturnCode End();

   protected:
        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called internally to handle messages
        virtual AAAReturnCode RxDelivery(std::auto_ptr<DiameterMsg> msg);

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

    private:
        DiameterAuthSessionClientStateMachine m_Fsm;
};

///
/// This class provides all the functionality of
/// an Diameter client acct session. The model
/// followed by OpenDiameter is to have a parent
/// accouting session and one or more accouting
/// sub-sessions. Each sub-session can generate
/// different record types. Applications are
/// also responsible for creating instance of 
/// these classes.
///
class DIAMETERBASEPROTOCOL_EXPORT DiameterClientAcctSession :
    public DiameterSessionIO
{
    public:
	typedef std::map<diameter_unsigned64_t, DiameterAcctSession*> 
                   AAA_SubSessionMap;
    public:
        DiameterClientAcctSession(AAA_Task &task,
                              diameter_unsigned32_t id,
                              char *optionalValue = 0);
        virtual ~DiameterClientAcctSession();
        AAA_Task &Task() {
           return m_Task;
	}
        diameter_unsigned32_t &ApplicationId() {
           return m_ApplicationId;
	}

        /// Registers an instance of a sub session
        AAAReturnCode RegisterSubSession(DiameterAcctSession &s);

        /// De-registers a sub session
        AAAReturnCode RemoveSubSession(diameter_unsigned64_t &id);

    private:
        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<DiameterMsg> msg);

    private:
        AAA_Task &m_Task;
        DiameterSessionId m_SessionId;
        diameter_unsigned32_t m_ApplicationId;
        diameter_unsigned64_t m_SubSessionId;
	AAA_SubSessionMap m_SubSessionMap;
};

///
/// The client accouting sub-session provides functionality
/// for record collection. It requires that a parent session
/// exits.
///
template<class REC_COLLECTOR>
class DiameterClientAcctSubSession :
    public DiameterAcctSession
{
    public:
        DiameterClientAcctSubSession(DiameterClientAcctSession &parent) :
	   DiameterAcctSession(parent.ApplicationId()),
           m_ParentSession(parent),
           m_Fsm(m_ParentSession.Task(), *this, m_RecCollector) {
           m_ParentSession.RegisterSubSession(*this);
           m_Fsm.Start();
        }
        virtual ~DiameterClientAcctSubSession() {
           m_ParentSession.RemoveSubSession
                  (Attributes().SubSessionId()());
           m_Fsm.Stop();
        }

        /// This function initializes an AAA recording session
        AAAReturnCode Begin(bool oneTime = false);

        /// This function terminates the AAA recording session
        AAAReturnCode End();

        /// Access function to the current collector
        REC_COLLECTOR &RecCollector() {
	   return m_RecCollector;
	}

        /// Access function to the parent session
        DiameterClientAcctSession &Parent() {
	   return m_ParentSession;
	}

   protected:
        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<DiameterMsg> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<DiameterMsg> msg);

    private:
        DiameterClientAcctSession &m_ParentSession;
        REC_COLLECTOR m_RecCollector;
        DiameterAcctSessionClientStateMachine m_Fsm;
};

#include "aaa_session_client.inl"

#endif

