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

/* Author   : Victor I. Fajardo
 * Synopsis : Server application test 
 */

#ifndef __SAMPLE_SERVER_H__
#define __SAMPLE_SERVER_H__

#include "diameter_api.h"
#include "ace/Synch.h"

/* forward declaration */
class AAASampleServer;

///////////////////////////////////////////////////////////////////////////////////
/////////////      !!!!            WARNING              !!!!       ////////////////
/////////////      THE FOLLOWING ARE OLD COMPATIBILITY API's       ////////////////
/////////////     NEW APPLICATIONS SHOULD NOT USE THESE API's      ////////////////
///////////////////////////////////////////////////////////////////////////////////

/*
 * This is a sample message handler object. An application
 * derives from a message handler if it wants to process
 * a specific message. Note that you can reuse this class
 * for any session wishing to process a specific message.
 *
 * You need to have more of these derived classes for 
 * each message you wish to handle. This gives you the
 * flexibility of coding handlers specific to a message.
 */
class AAASampleAuthAnswerMessage : public AAASessionMessageHandler {
    public:
       /*
        * The constructor forces us to pass in the session registering
        * this handler. This gives us access to the session once a
        * message has been receieved.
        */
       AAASampleAuthAnswerMessage(AAASampleServer &client, AAACommandCode code);

       ~AAASampleAuthAnswerMessage();

       /*
	* Actual message handler for this command code
	*/
       AAAReturnCode HandleMessage(AAAMessage &msg);

    protected:
       AAASampleServer &_session; /* reference to the original session */
};

/*
 * This is a sample application server. It derives
 * from AAAServerSession base class. Applications
 * interested in caputring events has to override
 * virtual handlers in this class.
 *
 * The sample server is a passive entity. Applications
 * has to define a class which overrides AAAServerSession
 * and performs the necessary functioning specific to
 * the application. Once the class is defined, the application
 * merely has to register the class via a class factory.
 * The application is not responsible for creating instances
 * of the classs it defined.
 *
 * Once an authentication request is received which has
 * a valid session id that this application supports,
 * the factory will create an instance of the sample
 * server session class. Then the request message will
 * be passed on to its default HandleMessage() method
 * where the application can examine it to determine
 * whether to accept the request or not. If the application
 * wishes to accept the request, it may call Accept()
 * on the message and the session will be valid.
 *
 * This sample server shows a simple state machine wherein
 * it accepts initial authentication request from the client.
 * The succeding request is treated as a signal to terminate
 * the session. Hence, if you look at the state machine, it
 * quickly transitions to termination on the 2nd request by
 * the client.
 */
class AAASampleServer : public AAAServerSession {
    public:
       /*
        * Sample state definitions for
        * application state machine
        */
       typedef enum {
         IDLE,
	 AUTHORIZED,
	 TERMINATING,
       } STATE;

    public:
       AAASampleServer(AAAApplicationCore &appCore, 
                       diameter_unsigned32_t appId);

       /*
	* For server session, this handler is invoked when
	* an incomming message belonging to this session
	* is not formally via RegisterMessageHandler().
	* This is a default catch all message handler.
	*
	* Note that if you do not wish to register a specific
	* message handler for a particular message, you may
	* capture that message here.
	*/
       AAAReturnCode HandleMessage(AAAMessage &msg);

       /*
	* On creation of a server session, the HandleDisconnect,
	* HandleSessionTimeout, HandleAuthLifetimeTimeout,
        * HandleAuthGracePeriodTimeout and HandleAbort methods 
        * available to the internal library if any of the events 
        * require thier invocation.
	*/

       AAAReturnCode HandleDisconnect();

       AAAReturnCode HandleSessionTimeout();

       AAAReturnCode HandleAuthLifetimeTimeout();

       AAAReturnCode HandleAuthGracePeriodTimeout();

       AAAReturnCode HandleAbort();

       /*
	* Test code to send a sample authentication
	* request answers. The command code is 300 
	* for the sample client message.
	*/
       AAAReturnCode SendTestAuthAnswer(AAAMessage &request);

       /*
	* State access functions
	*/

       inline void state(AAASampleServer::STATE s) { _state = s; }

       inline AAASampleServer::STATE state() { return _state; }

       /*
	* This semaphore allows the sample application to wait
	* till all sessions are completed before allowing the
	* application to exit cleanly. This is for controlled
	* test purposes only and does not reflect any real
	* applications.
	*/
       inline static ACE_Semaphore &semaphore() { return AAASampleServer::_semaphore; }

       /*
	* session index is a way to track how many client 
	* has attempted an authentication request.
	*/
       inline int sessionIndex() { return _sessionIndex; }

       /*
	* This flag is passed in as parameter to this application.
	* It tells the application whether to send an abort message
	* once the last message from the client is received.
	*/
       inline static bool sendAbort() { return AAASampleServer::_sendAbort; }

       inline static void sendAbort(bool send) { AAASampleServer::_sendAbort = send; }

    protected:
       static bool _sendAbort; /* abort flag */

       std::string _destHost; /* client hostname */

       std::string _destRealm; /* client realm */

       int _sessionIndex; /* current index */

       static int _sessionCount; /* session counter */

       AAASampleServer::STATE _state; /* application specific state */

       static ACE_Semaphore _semaphore; /* simple sync solution */

       AAASampleAuthAnswerMessage _answerMsgHandler; /* convinient to store it here */
};

/*
 * This is the sample authentication server factory definition.
 * When registered, this factory will create instances
 * of AAASampleServer when an incomming request message
 * not matching any session and containing an application
 * id of this factory is received.
 */
typedef AAAServerSessionClassFactory<AAASampleServer> serverFactory;

class AAASampleAccountingServer;

/*
 * We inherit the default transformer so we can
 * over-ride the output methods. In this method,
 * we can provide application specific storage
 * mechanism.
 */
class AAASampleXMLTrans : public AAAAccountingXMLRecTransformer
{
    public:
        AAASampleXMLTrans(AAASampleAccountingServer &server) : session(server) { }

        /*!
         * We provide our own storage mechanism here
         */
        AAAReturnCode OutputRecord(AAAMessage *originalMsg);

    protected:
        AAASampleAccountingServer &session;
};

/*
 * This is a sample accounting server. It derives
 * from AAAServerAccountingSession base class. 
 * Applications interested in providing accounting
 * record storage or forwarding records to an
 * accounting server MUST implement an accounting
 * server session.
 *
 * As with authentication, the accounting server
 * is a passive entity. All internal functionings
 * are similar to authentication sessions. It even
 * uses the same session database. The biggest
 * difference would be the alternative use of 
 * record transformers. These objects allows the
 * transformation of AAAMessage to an application
 * specific message type. Open diameter currently
 * provides a default transformer that converts
 * AAAMessage to a streamed XML format. An application
 * may implement it's own transformer as it wishes.
 * This sample code uses the default transfomer.
 *
 * In this sample code, since we are using a
 * transformer, we DO NOT need to assign a
 * message handler. We can let the transformer
 * be the message handler for each incomming
 * message for this session.
 */
class AAASampleAccountingServer : public AAAAccountingServerSession {
    public:
       typedef enum {
 	  ACR = 271 // Accounting request command code value
       };

    public:
       AAASampleAccountingServer(AAAApplicationCore &appCore, 
                                 diameter_unsigned32_t appId);

       ~AAASampleAccountingServer();

       /*
	* Test code to send a sample authentication
	* request answers. The command code is 271
	* for the sample client message.
	*/
       AAAReturnCode SendAcctAnswer(AAAMessage &request);

    private:
       AAASampleXMLTrans xml; /* Default transformer */
};

/*
 * This is the sample accounting server factory definition.
 * When registered, this factory will create instances
 * of AAASampleServer when an incomming request message
 * not matching any session and containing an application
 * id of this factory is received.
 */
typedef AAAServerSessionClassFactory<AAASampleAccountingServer> acctFactory;

#endif /* __SAMPLE_SERVER_H__ */







