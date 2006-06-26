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

#ifndef __AAA_SESSION_SERVER_H__
#define __AAA_SESSION_SERVER_H__

#include "aaa_session.h"
#include "aaa_session_auth_server_fsm.h"
#include "aaa_session_acct_server_fsm.h"
#include "aaa_session_garbage_collector.h"

///
/// This class provides all the functionality of
/// an Diameter client auth session. Applications
/// needs to use the server session factory which
/// generates instance of this class.
///
class DIAMETERBASEPROTOCOL_EXPORT AAA_ServerAuthSession : 
    public AAA_AuthSession 
{
    public:
        AAA_ServerAuthSession(AAA_Task &task,
                              diameter_unsigned32_t id);
        virtual ~AAA_ServerAuthSession() {
           m_Fsm.Stop();
        }

        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<AAAMessage> msg);

        /// Initiates a server side Re-Authentication
        AAAReturnCode ReAuth(diameter_unsigned32_t type);

        /// This function terminates the AAA session
        AAAReturnCode End();

    protected:
        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called internally to handle messages
        virtual AAAReturnCode RxDelivery(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called internally to route messages
        virtual AAAReturnCode TxDelivery(std::auto_ptr<AAAMessage> msg);

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

    private:
        AAA_AuthSessionServerStateMachine m_Fsm;
};

///
/// This class provides all the functionality of
/// an Diameter client acct session. Applications
/// needs to use the server session factory which
/// generates instance of this class. Note that
/// this requires a record storage class that 
/// interfaces with the applications storage
/// schemes.
///
template<class REC_STORAGE>
class AAA_ServerAcctSession : 
    public AAA_AcctSession
{
    public:
        AAA_ServerAcctSession(AAA_Task &task,
                              diameter_unsigned32_t id,
                              bool stateful = false);
        virtual ~AAA_ServerAcctSession() {
           m_Fsm.Stop();
        }

        REC_STORAGE &RecStorage() { 
           return m_RecStorage; 
        }

    protected:
        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxRequest(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxAnswer(std::auto_ptr<AAAMessage> msg);

        /// This fucntion is called by the internal message rx
        virtual void RxError(std::auto_ptr<AAAMessage> msg);

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

    private:
        bool m_Stateful;
        REC_STORAGE m_RecStorage;
        AAA_AcctSessionServerStateMachine m_Fsm;
};

///
/// Internal garbage collector definitions
///

typedef AAA_SessionGarbageCollectorSingleton<AAA_AuthSession>
             AAA_ServerAuthSessionGC;
typedef AAA_SessionGarbageCollectorSingleton<AAA_AcctSession>
             AAA_ServerAcctSessionGC;

typedef ACE_Singleton<AAA_ServerAuthSessionGC, 
                      ACE_Recursive_Thread_Mutex> 
                      AAA_ServerAuthSessionGC_S;
#define AAA_AUTH_SESSION_GC_ROOT() (AAA_ServerAuthSessionGC_S::instance())
#define AAA_AUTH_SESSION_GC() (AAA_ServerAuthSessionGC_S::instance()->Instance()) 

typedef ACE_Singleton<AAA_ServerAcctSessionGC, 
                      ACE_Recursive_Thread_Mutex> 
                      AAA_ServerAcctSessionGC_S;
#define AAA_ACCT_SESSION_GC_ROOT() (AAA_ServerAcctSessionGC_S::instance())
#define AAA_ACCT_SESSION_GC() (AAA_ServerAcctSessionGC_S::instance()->Instance()) 

#include "aaa_session_server.inl"

#endif

