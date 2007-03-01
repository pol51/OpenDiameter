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

#ifndef __AAA_SESSION_SERVER_H__
#define __AAA_SESSION_SERVER_H__

#include "aaa_session.h"
#include "aaa_session_auth_server_fsm.h"
#include "aaa_session_acct_server_fsm.h"

///
/// This class provides all the functionality of
/// an Diameter client auth session. Applications
/// needs to use the server session factory which
/// generates instance of this class.
///
class DIAMETERBASEPROTOCOL_EXPORT DiameterServerAuthSession :
    public DiameterAuthSession 
{
    public:
        DiameterServerAuthSession(AAA_Task &task,
                              diameter_unsigned32_t id);
        virtual ~DiameterServerAuthSession() {
           m_Fsm.Stop();
        }

        /// This fucntion sends a message to the peer session
        virtual AAAReturnCode Send(std::auto_ptr<DiameterMsg> msg);

        /// Initiates a server side Re-Authentication
        AAAReturnCode ReAuth(diameter_unsigned32_t type);

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

        /// This fucntion is called internally to route messages
        virtual AAAReturnCode TxDelivery(std::auto_ptr<DiameterMsg> msg);

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

    private:
        DiameterAuthSessionServerStateMachine m_Fsm;
};

///
/// This class provides all the functionality of
/// an Diameter client acct session. Applications
/// needs to use the server session factory which
/// generates instance of this class. Note that
/// this requires a record storage class DiameterServerAcctSession 
/// interfaces with the applications storage
/// schemes.
///
template<class REC_STORAGE>
class DiameterServerAcctSession : 
    public DiameterAcctSession
{
    public:
        DiameterServerAcctSession(AAA_Task &task,
                              diameter_unsigned32_t id,
                              bool stateful = false);
        virtual ~DiameterServerAcctSession() {
           m_Fsm.Stop();
        }

        REC_STORAGE &RecStorage() { 
           return m_RecStorage; 
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

        /// This function resets the current session attributes to default
        virtual AAAReturnCode Reset();

    private:
        bool m_Stateful;
        REC_STORAGE m_RecStorage;
        DiameterAcctSessionServerStateMachine m_Fsm;
};

///
/// Internal garbage collector definitions
///

typedef DiameterGarbageCollectorSingleton<DiameterAuthSession>
             DiameterServerAuthSessionGC;
typedef DiameterGarbageCollectorSingleton<DiameterAcctSession>
             DiameterServerAcctSessionGC;

typedef ACE_Singleton<DiameterServerAuthSessionGC,
                      ACE_Recursive_Thread_Mutex> 
                      DiameterServerAuthSessionGC_S;
#define DIAMETER_AUTH_SESSION_GC_ROOT() (DiameterServerAuthSessionGC_S::instance())
#define DIAMETER_AUTH_SESSION_GC() (DiameterServerAuthSessionGC_S::instance()->Instance()) 

typedef ACE_Singleton<DiameterServerAcctSessionGC, 
                      ACE_Recursive_Thread_Mutex> 
                      DiameterServerAcctSessionGC_S;
#define DIAMETER_ACCT_SESSION_GC_ROOT() (DiameterServerAcctSessionGC_S::instance())
#define DIAMETER_ACCT_SESSION_GC() (DiameterServerAcctSessionGC_S::instance()->Instance()) 

#include "aaa_session_server.inl"

#endif

