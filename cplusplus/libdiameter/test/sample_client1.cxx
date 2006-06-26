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

#include <string>
#include "ace/OS.h"
#include "sample_client1.h"

///////////////////////////////////////////////////////////////////////////////////
/////////////      !!!!            WARNING              !!!!       ////////////////
/////////////      THE FOLLOWING ARE OLD COMPATIBILITY API's       ////////////////
/////////////     NEW APPLICATIONS SHOULD NOT USE THESE API's      ////////////////
///////////////////////////////////////////////////////////////////////////////////

/* session counter for indexing session holders */
int AAASampleClient::_sessionCount = 0;

/* simple synchronization tool */
ACE_Semaphore AAASampleClient::_semaphore(0);

/*
 * Session holder instance automatically
 * initializes the session client and
 * message handler
 */
AAASampleSessionHolder::AAASampleSessionHolder(AAAApplicationCore &appCore, 
                                               diameter_unsigned32_t appId) :
   _authSampleClient(appCore, appId),
   _authMsgHandler(_authSampleClient, 300),
   _acctSampleClient(appCore, appId),
   _acctMsgHandler(_acctSampleClient, AAASampleAccountingRequestMsg::ACR)
{
   // do nothing     
}

AAASampleAuthMessage::AAASampleAuthMessage(AAASampleClient &client, AAACommandCode code) :
    AAASessionMessageHandler((AAAApplicationCore&)client.GetApplicationCore(), code),
    session(client)
{
   /*
    * To facilitate simplicity, the registration of
    * the message handler to the session is done 
    * in the handlers constructor
    */
   session.RegisterMessageHandler(this);
}

AAASampleAuthMessage::~AAASampleAuthMessage()
{
   /*
    * Likewise, the registration removal is done
    * in the destructor
    */
   session.RemoveMessageHandler(this);
}

AAAReturnCode AAASampleAuthMessage::HandleMessage(AAAMessage &msg)
{
   /*
    * For test purposes, we leave error messages unhandled
    * for the moment.
    */
   if (msg.hdr.flags.e) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Error message received, unhandled at the moment\n"));
      return (AAA_ERR_SUCCESS);
   }

   /*
    * This is the sample state machine. For every message
    * received from the server we transition to another
    * state finally leading to termination. Details of
    * the message exchange is in the header files.
    *
    * Notice that we send a test auth only once after
    * receiving an answer to our first request. Then
    * the state transitions to termination state. On
    * termination state, the library will handle STR
    * processing sent by the server. After processing
    * the client will be notified by HandleDisconnect().
    */
   switch (session.state()) {
      case AAASampleClient::IDLE:
         if (! msg.hdr.flags.r) {

            ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: ^^^^^^^ Authenticated for Session # %d\n", session.sessionIndex()));

            session.Update(AAASession::EVENT_AUTH_SUCCESS);

            session.SendTestAuth();

            session.state(AAASampleClient::TERMINATING);
 	 }
	 break;

      case AAASampleClient::AUTHORIZED:
 	 // un-used for now
	 break;

      case AAASampleClient::TERMINATING:
	 if (! msg.hdr.flags.r) {
            ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Ready for termination  Session # %d\n", session.sessionIndex()));
         }
	 break;
   }
   return (AAA_ERR_SUCCESS);
}

AAASampleClient::AAASampleClient(AAAApplicationCore &appCore, 
                                 diameter_unsigned32_t appId) :
   AAAClientSession(appCore, appId)
{
   /*
    * A simple client session counter is introduced 
    * for referencing each session
    */

   _state = AAASampleClient::IDLE;

   AAASampleClient::_sessionCount ++;

   _sessionIndex = AAASampleClient::_sessionCount;
}

AAAReturnCode AAASampleClient::HandleMessage(AAAMessage &msg)
{
   /*
    * This is the default message handler. Any message
    * belonging to the session but does not have a 
    * registered handler will be passed to this method
    *
    * In this case, we treat any other message as an 
    * unknown. The application may send an unknown 
    * message error if it wishes.
    */

   ACE_ERROR((LM_ERROR, "(%P|%t) Client: Unknown message received\n"));
   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::HandleDisconnect()
{
   /*
    * This handler can be called when the transport
    * fails, when a session termination request is
    * received or when an application core is exiting.
    *
    * In the case of this sample, we are expecting
    * this function to get called on receipt of a
    * session termination request. In other cases,
    * the code here will work to.
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Ending session for session # %d\n",  _sessionIndex));

   End(); /* calling end will remove this session
		 * from the database and terminate it
		 * cleanly.
		 */

   /*
    * This is allows us to release the semaphore
    * on the main program once all the client
    * session has terminated.
    */
   AAASampleClient::_sessionCount --;

   if (AAASampleClient::_sessionCount == 0) {
      AAASampleClient::_semaphore.release();
   }

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::HandleSessionTimeout()
{
   /* 
    * Called on session timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Session timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::HandleAuthLifetimeTimeout()
{
   /* 
    * Called on auth lifetime timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Authorization lifetime timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::HandleAuthGracePeriodTimeout()
{
   /* 
    * Called on auth grace period timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Authorization grace period timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::HandleAbort()
{
   /*
    * Called when an abort session request is
    * received. We will never receive this because
    * we are acting as a client. The server session
    * may process this.
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Abort event for session # %d\n",  _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleClient::SendTestAuth()
{
   /*
    * Sample authorization request. This message
    * composition follows libdiamparser rules in
    * composing an AAAMessage. The test command
    * code we use is 300.
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Sending test auth message # %d\n",  _sessionIndex));

   AAAAvpContainerManager cm;
   AAAAvpContainerEntryManager em;

   AAAAvpContainer *c_sessId = cm.acquire("Session-Id");
   AAAAvpContainer *c_orhost = cm.acquire("Origin-Host");
   AAAAvpContainer *c_orrealm = cm.acquire("Origin-Realm");
   AAAAvpContainer *c_dhost = cm.acquire("Destination-Host");
   AAAAvpContainer *c_drealm = cm.acquire("Destination-Realm");
   AAAAvpContainer *c_authid = cm.acquire("Auth-Application-Id");
   AAAAvpContainer *c_reauth = cm.acquire("Re-Auth-Request-Type");
   AAAAvpContainerEntry *e;

   e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
   c_sessId->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orhost->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orrealm->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   diameter_identity_t &dhost = e->dataRef(Type2Type<diameter_identity_t>());
   c_dhost->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   diameter_identity_t &drealm = e->dataRef(Type2Type<diameter_identity_t>());
   c_drealm->add(e);
 
   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &authid = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_authid->add(e);
 
   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &reauth = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_reauth->add(e);
 
   AAAMessage authMsg;
   hdr_flag flag = {1,0,0,0,0};
   AAADiameterHeader h(1, 0, flag, 300, 10000, 0, 0);
   authMsg.hdr = h;

   authMsg.acl.add(c_sessId);
   authMsg.acl.add(c_orhost);
   authMsg.acl.add(c_orrealm);
   authMsg.acl.add(c_dhost);
   authMsg.acl.add(c_drealm);
   authMsg.acl.add(c_authid);
   authMsg.acl.add(c_reauth);

   dhost = _destHost;
   drealm = _destRealm;

   authid = 8; // SAMPLE
   reauth = 1;

   /*
    * The use of AAAMessageControl is shown here.
    * It is a utility class used in sending messages
    * via the local session object as well as setting
    * the result code.
    */
   AAAMessageControl msgControl(this);
   if (msgControl.Send(authMsg) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Failed sending message\n"));
      return (AAA_ERR_FAILURE);
   }
   else {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Sent Auth Message\n"));
   }

   return (AAA_ERR_SUCCESS);
}

AAASampleAccountingClient::AAASampleAccountingClient(AAAApplicationCore &appCore, 
                                                     diameter_unsigned32_t appId)
  : AAAAccountingClientSession(appCore, appId)
{
   rec_counter = 0;
}

AAASampleAccountingClient::~AAASampleAccountingClient()
{
   // do nothing
}

AAAReturnCode AAASampleAccountingClient::SendAcctMessage(RECTYPE type)
{
   /*
    * Sample accounting request. This message
    * composition follows libdiamparser rules in
    * composing an AAAMessage. The commandn code
    * for accounting request is AAASampleAccountingRequestMsg::ACR.
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Sending test acct message\n"));

   AAAAvpContainerManager cm;
   AAAAvpContainerEntryManager em;

   AAAAvpContainer *c_sessId = cm.acquire("Session-Id");
   AAAAvpContainer *c_orhost = cm.acquire("Origin-Host");
   AAAAvpContainer *c_orrealm = cm.acquire("Origin-Realm");
   AAAAvpContainer *c_drealm = cm.acquire("Destination-Realm");
   AAAAvpContainer *c_rectype = cm.acquire("Accounting-Record-Type");
   AAAAvpContainer *c_recnum = cm.acquire("Accounting-Record-Number");
   AAAAvpContainer *c_acctid = cm.acquire("Acct-Application-Id");
   AAAAvpContainer *c_user = cm.acquire("User-Name");
   AAAAvpContainer *c_subid = cm.acquire("Accounting-Sub-Session-Id");
   AAAAvpContainer *c_acctsid = cm.acquire("Accounting-Session-Id");
   AAAAvpContainer *c_acctmsid = cm.acquire("Acct-Multi-Session-Id");
   AAAAvpContainer *c_intvl = cm.acquire("Accounting-Interim-Interval");
   AAAAvpContainer *c_realtime = cm.acquire("Accounting-Realtime-Required");
   AAAAvpContainer *c_origin = cm.acquire("Origin-State-Id");

   AAAAvpContainerEntry *e;

   e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
   c_sessId->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orhost->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orrealm->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   diameter_identity_t &drealm = e->dataRef(Type2Type<diameter_identity_t>());
   c_drealm->add(e);
 
   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &rectyp = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_rectype->add(e);

   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &recnum = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_recnum->add(e);

   e = em.acquire(AAA_AVP_INTEGER32_TYPE);
   diameter_integer32_t &acctid = e->dataRef(Type2Type<diameter_integer32_t>());
   c_acctid->add(e);
 
   e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
   diameter_utf8string_t &user = e->dataRef(Type2Type<diameter_utf8string_t>());
   c_user->add(e);

   e = em.acquire(AAA_AVP_UINTEGER64_TYPE);
   diameter_unsigned64_t &subid = e->dataRef(Type2Type<diameter_unsigned64_t>());
   c_subid->add(e);
 
   e = em.acquire(AAA_AVP_STRING_TYPE);
   diameter_octetstring_t &acctsid = e->dataRef(Type2Type<diameter_octetstring_t>());
   c_acctsid->add(e);
 
   e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
   diameter_utf8string_t &acctmsid = e->dataRef(Type2Type<diameter_utf8string_t>());
   c_acctmsid->add(e);
 
   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &intvl = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_intvl->add(e);

   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &realtime = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_realtime->add(e);

   e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
   diameter_unsigned32_t &origin = e->dataRef(Type2Type<diameter_unsigned32_t>());
   c_origin->add(e);

   AAAMessage acctMsg;
   hdr_flag flag = {1,0,0,0,0};
   AAADiameterHeader h(1, 0, flag, AAASampleAccountingRequestMsg::ACR, 20000, 0, 0);
   acctMsg.hdr = h;

   acctMsg.acl.add(c_sessId);
   acctMsg.acl.add(c_orhost);
   acctMsg.acl.add(c_orrealm);
   acctMsg.acl.add(c_drealm);
   acctMsg.acl.add(c_rectype);
   acctMsg.acl.add(c_recnum);
   acctMsg.acl.add(c_acctid);
   acctMsg.acl.add(c_user);
   acctMsg.acl.add(c_subid);
   acctMsg.acl.add(c_acctsid);
   acctMsg.acl.add(c_acctmsid);
   acctMsg.acl.add(c_intvl);
   acctMsg.acl.add(c_realtime);
   acctMsg.acl.add(c_origin);

   drealm = _destRealm;
   rectyp = type; // event record only
   recnum = ++ rec_counter;
   acctid = 0; // sample only 
   user.assign("user@sample");
   subid = 0; // sample only
   acctsid.assign("samplesessid");
   acctmsid.assign("samplemsid");
   intvl = 30; // 30 sec interval for sampling
   realtime = 2; // grant and store
   origin = 0; // sample only

   /*
    * The use of AAAMessageControl is shown here.
    * It is a utility class used in sending messages
    * via the local session object as well as setting
    * the result code.
    */
   AAAMessageControl msgControl(this);
   if (msgControl.Send(acctMsg) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Failed sending message\n"));
      return (AAA_ERR_FAILURE);
   }
   else {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Sent Acct Message\n"));
   }

   return (AAA_ERR_SUCCESS);
}

/*!
 * Event handler for interim record events. This
 * method is invoked when interim record events
 * has been scheduled via SetInterimRecordInterval.
 *
 * In our example we use it to generate accounting 
 * request. The request are EVENT types only. For
 * duration type messages (START,INTERIN,STOP), you
 * need to extend this class to provide such
 * functionality.
 *
 * Note that you can use payload parameter to pass 
 * in user data.
 */
AAAReturnCode AAASampleAccountingClient::HandleInterimRecordEvent(RECTYPE type, 
                                                                  AAASessionPayload payload)
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Interim record event ... collecting data\n"));

  /* The interim handler is our queue
   * to collect accounting data and
   * send a request. In our sample
   * we simply send a request to and
   * mimic collection
   */
   return SendAcctMessage(type);
}

AAASampleAccountingRequestMsg::AAASampleAccountingRequestMsg(AAASampleAccountingClient &sess   , 
                                                             AAACommandCode code)
  : AAASessionMessageHandler((AAAApplicationCore&)sess.GetApplicationCore(), code),
    session(sess)
{
   /*
    * To facilitate simplicity, the registration of
    * the message handler to the session is done 
    * in the handlers constructor
    */
   session.RegisterMessageHandler(this);
}

AAASampleAccountingRequestMsg::~AAASampleAccountingRequestMsg()
{
   /*
    * Likewise, the registration removal is done
    * in the destructor
    */
   session.RemoveMessageHandler(this);
}

AAAReturnCode AAASampleAccountingRequestMsg::HandleMessage(AAAMessage &msg)
{
   /* This is the default handler for accounting messages.
    * In real implementations, applications MUST examine 
    * the contents of the answer message and act accordingly.
    * Applications may terminate in interim record generator
    * as a result.
    */
   if (msg.hdr.code == AAASampleAccountingRequestMsg::ACR && ! msg.hdr.flags.r) {
      /*
       * It is important to note that for test purposes, you have to
       * make sure that accounting interval will occurr while this
       * peer still has an active connection to the server. This
       * sample code DOES NOT check for this. Real application MUST
       * do so.
       */
      if (session.numRecords() == AAASampleAccountingRequestMsg::rec_threshold) {
         ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Accounting session ready for termination\n"));
      }
      else {

         ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Scheduling an accounting interim\n"));
         /* 
          * We set a 1 sec interval in scheduling an interim
          * handler event.
          */
         session.SetInterimRecordInterval(AAAAccountingClientSession::RECTYPE_EVENT, 
                                                0, // collect records immediately
                                                NULL);
      }
   }
   return (AAA_ERR_SUCCESS);
}

int main(int argc, char *argv[])
{
   /*
    * The main test program is straightforward. It
    * processes all arguments passed to it then 
    * creates the appropriate number of sessions.
    * It then waits till all sessions have terminated
    * completely before exiting.
    *
    * The number of sessions as well as the authorizing
    * host and realm is passed in as arguments. Also,
    * the configuration file is an argument.
    */

   if (argc != 5) {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Usage: aaa_sample_client [host] [realm] [num session] [config file]\n"));
      return (1);
   }

   int howManySessions = atoi(argv[3]);
   if (howManySessions <= 0) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Invalid number of sessions\n"));
      return (1);
   }

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Application starting\n"));

   AAA_Task myTask;
   try {
      myTask.Start(10);
   }
   catch (...) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Cannot start task\n"));
      return (1);
   }
   AAAApplicationCore myCore;
   if (myCore.Open(argv[4], myTask) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Application core open error\n"));
      return (1);
   }

   std::string destHost(argv[1]);
   std::string destRealm(argv[2]);

   holderList _holderList;

   // Primitive wait. Waiting for the client to
   // establish connection with the peers
   while (myCore.GetNumActivePeerConnections() == 0); 

   for (int i = 0; i < howManySessions; i++) {

      /*
       * This creates the session and sets 
       * application id to zero for this sample.
       *
       * Note that we are repsonsible for creating
       * instances of the client session. Hence,
       * we are able to make references of our
       * allocation and delete it afterwards.
       *
       * For accounting sessions, creation of
       * a new instance of the holder will
       * create a session id for the accounting
       * session.
       */
      AAASampleSessionHolder *holder = new AAASampleSessionHolder(myCore, 10000);

      if (holder) {

         ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Starting client # %d\n", i));

         holder->clientSession().destHost(destHost);

         holder->clientSession().destRealm(destRealm);

         holder->acctSession().destRealm(destRealm);

	 /*
	  * A call to start session will create the session
	  * id and add this session to the local database.
	  * On success, a session may be established with
	  * a server by sending an authorization request.
	  */
         holder->clientSession().Start();

	 /*
	  * Send the auth request
	  */
         holder->clientSession().SendTestAuth();

         /*
	  * Start accounting
	  */
	 holder->acctSession().SetInterimRecordInterval(AAAAccountingClientSession::RECTYPE_EVENT, 
                                                        0, // collect records immediately 
                                                        NULL);

	 /*
	  * Applications need to call Update on the session
	  * to notify the library wether the application
	  * specific authorization request was successful or
	  * not. This processing is application specific and
	  * can only be done at the application level. But since
	  * the result is required by the session state machine,
	  * an update call is necessary.
	  */
         holder->clientSession().Update(AAASession::EVENT_AUTH_REQUEST);

	 /* store our holders and cleanup later */
         _holderList.push_back(holder);
      }
   }

   /*
    * This is to make sure we wait for accounting
    * session to complete
    */
   ACE_OS::sleep(AAASampleAccountingRequestMsg::rec_threshold * 2);

   /*
    * this will block until all session are done.
    * i.e. thier HandleDisconnect() is called.
    */
   AAASampleClient::semaphore().acquire();

   /*
    * cleanup our allocations. we delete all
    * our client allocations here.
    */
   while (! _holderList.empty()) {
      AAASampleSessionHolder *holder = _holderList.front();
      _holderList.pop_front();
      delete holder;
   }

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Client: Exiting\n"));

   return (0);
}



