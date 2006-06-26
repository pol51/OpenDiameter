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

#ifndef __SAMPLE_CLIENT_H__
#define __SAMPLE_CLIENT_H__

///////////////////////////////////////////////////////////////////////////////////
/////////////      !!!!            WARNING              !!!!       ////////////////
/////////////      THE FOLLOWING ARE OLD COMPATIBILITY API's       ////////////////
/////////////     NEW APPLICATIONS SHOULD NOT USE THESE API's      ////////////////
///////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"

/*
 * This is a sample application client. It derives
 * from AAAClientSession base class. Applications
 * interested in caputring events has to override
 * virtual handlers in this class.
 *
 * The sample client attempts to establish a session
 * with a sample server. The sample server is designated
 * by destination host and realm and passed in as a
 * parameter to this program. The initial request sent
 * out by this client is processed by the server and
 * an answer is sent back. Once the answer is received
 * another test message is sent to test normal traffic.
 * If the second request message is recieved properly
 * by the server, an answer is sent back and the server
 * prepares to abort the session. Once the client receives
 * the answer it sends a termination request to the server.
 * Once a complete cycle of abort and termination answers
 * are sent by the client and server respectively, the
 * session is terminated on both sides.
 */
class AAASampleClient : public AAAClientSession {
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
       AAASampleClient(AAAApplicationCore &appCore, 
                       diameter_unsigned32_t appId);

       /*
	* For client session, this handler is invoked when
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
	* request. The command code is 300 for the
	* sample test message.
	*/
       AAAReturnCode SendTestAuth();

       /*
	* Session counter and session index is a way
	* to track how many client this sample application
	* will create. These are mostly for sample purposes
	* and does not in anyway relate to a real application
	*/

       inline static int sessionCount() { return AAASampleClient::_sessionCount; }

       inline static void sessionCount(int count) { AAASampleClient::_sessionCount = count; }

       inline int sessionIndex() { return _sessionIndex; }

       /*
	* Allows this session to keep track of the host and
	* realm it's connected to
	*/

       inline std::string &destHost() { return _destHost; }

       inline void destHost(std::string &host) { _destHost = host; }

       inline std::string &destRealm() { return _destRealm; }

       inline void destRealm(std::string &realm) { _destRealm = realm; }

       /*
	* State access functions
	*/

       inline void state(AAASampleClient::STATE s) { _state = s; }

       inline AAASampleClient::STATE state() { return _state; }

       /*
	* This semaphore allows the sample application to wait
	* till all sessions are completed before allowing the
	* application to exit cleanly. This is for controlled
	* test purposes only and does not reflect any real
	* applications.
	*/
       inline static ACE_Semaphore &semaphore() { return AAASampleClient::_semaphore; }

    protected:
       std::string _destHost; /* session destination host */

       std::string _destRealm; /* session destination realm */

       int _sessionIndex; /* session local index */

       static int _sessionCount; /* number of sessions in this application */

       AAASampleClient::STATE _state; /* application specific state */

       static ACE_Semaphore _semaphore; /* simple sync solution */
};

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
class AAASampleAuthMessage : public AAASessionMessageHandler {
    public:
       /*
        * The constructor forces us to pass in the session registering
        * this handler. This gives us access to the session once a
        * message has been receieved.
        */
       AAASampleAuthMessage(AAASampleClient &client, AAACommandCode code);

       ~AAASampleAuthMessage();

       /*
	* Actual message handler for code
	*/
       AAAReturnCode HandleMessage(AAAMessage &msg);

    protected:
       AAASampleClient &session; /* reference to the original authentication session */
};

/*
 * This is a sample accounting session client. An application
 * derives from this class to add accounting functionality
 * to an application. Note that accounting sessions mimic
 * authentication sessions. The only differences is the
 * addition of the interim record handler in the session
 * class.
 */
class AAASampleAccountingClient : public AAAAccountingClientSession
{
   public:
      AAASampleAccountingClient(AAAApplicationCore &appCore, 
                                diameter_unsigned32_t appId);

      ~AAASampleAccountingClient();

       /*
	* Auxillary method to generate an accounting
	* request message. The message is a general
	* accounting message.
	*/
       AAAReturnCode SendAcctMessage(RECTYPE type);

       /*!
        * Event handler for interim record events. This
        * method is invoked when interim record events
        * has been scheduled via SetInterimRecordInterval
        */
       AAAReturnCode HandleInterimRecordEvent(RECTYPE type, 
                                              AAASessionPayload payload);

       inline void destRealm(std::string &realm) { _destRealm = realm; }

       inline int numRecords() { return rec_counter; }

   protected:
       std::string _destRealm; /* session destination realm */

       int rec_counter; /* session record counter */
};

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
class AAASampleAccountingRequestMsg : public AAASessionMessageHandler
{
    public:
       typedef enum {
 	  rec_threshold = 1  // this defines an arbitrary number of records
                             // that we are willing to process. This is for
	                     // testing purposes only. You may modify this
                             // number to suite your needs
       };

       typedef enum {
 	  ACR = 271 // Accounting request command code value
       };

    public:
       AAASampleAccountingRequestMsg(AAASampleAccountingClient &session, 
                                     AAACommandCode code);

       ~AAASampleAccountingRequestMsg();

       /* 
	* This handler method is invoked when an incomming
	* message matches the registration of this event
	* handler. In this example, we use the session 
	* object itself as the registered event handler.
	*/
       AAAReturnCode HandleMessage(AAAMessage &msg);

    protected:
       AAASampleAccountingClient &session; /* reference to the original session */
};

/*
 * This class is for convience only. Since this
 * application creates more than one session, we
 * created a simple class which holds both instances
 * of the message handlers and the session themselves.
 */
class AAASampleSessionHolder
{
   public:
      AAASampleSessionHolder(AAAApplicationCore &appCore, 
                             diameter_unsigned32_t appId);

      /*
       * Access functions to client session and
       * message handler instance
       */

      AAASampleAuthMessage &authMsgHandler() { return _authMsgHandler; }

      AAASampleClient &clientSession() { return _authSampleClient; }

      AAASampleAccountingClient &acctSession() { return _acctSampleClient; }

      AAASampleAccountingRequestMsg &acctMsgHandler() { return _acctMsgHandler; }

   protected:
      AAASampleClient _authSampleClient; /* instance of sample client */

      AAASampleAuthMessage _authMsgHandler; /* instance of message handler,
					     * if you want to handle more
					     * command codes, you may create
					     * more instances of the handler.
					     * One for each command code
                                             */

      AAASampleAccountingClient _acctSampleClient; /* instance of an authentication client */

      AAASampleAccountingRequestMsg _acctMsgHandler;  /* instance of message handler,
					               * if you want to handle more
					               * command codes, you may create
					               * more instances of the handler.
					               * One for each command code
                                                       */
};

/*
 * simple list database of session holders
 */
typedef std::list<AAASampleSessionHolder*> holderList;

#endif /* __SAMPLE_CLIENT_H__ */




