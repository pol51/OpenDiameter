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

#include "sample_server1.h"

///////////////////////////////////////////////////////////////////////////////////
/////////////      !!!!            WARNING              !!!!       ////////////////
/////////////      THE FOLLOWING ARE OLD COMPATIBILITY API's       ////////////////
/////////////     NEW APPLICATIONS SHOULD NOT USE THESE API's      ////////////////
///////////////////////////////////////////////////////////////////////////////////

/* session counter for counting authenitcation request */
int AAASampleServer::_sessionCount = 0;

/* abort flag */
bool AAASampleServer::_sendAbort = false;

/* simple synchronization tool */
ACE_Semaphore AAASampleServer::_semaphore(0);

AAASampleAuthAnswerMessage::AAASampleAuthAnswerMessage(AAASampleServer &server, AAACommandCode code) :
    AAASessionMessageHandler((AAAApplicationCore&)server.GetApplicationCore(), code),
    _session(server)
{
   /*
    * To facilitate simplicity, the registration of
    * the message handler to the session is done 
    * in the handlers constructor
    */
   _session.RegisterMessageHandler(this);
}

AAASampleAuthAnswerMessage::~AAASampleAuthAnswerMessage()
{
   /*
    * Likewise, the registration removal is done
    * in the destructor
    */
   _session.RemoveMessageHandler(this);
}

AAAReturnCode AAASampleAuthAnswerMessage::HandleMessage(AAAMessage &msg)
{
   /*
    * For test purposes, we leave error messages unhandled
    * for the moment.
    */
   if (msg.hdr.flags.e) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Error message received, unhandled at the moment\n"));
      return (AAA_ERR_SUCCESS);
   }

   /*
    * This is the sample state machine. For every message
    * received by this handler, it simply replys with
    * an answer message. This server functions much like
    * an echo server. However, if an abort flag is set
    * then it will send an ASR message upon receipt of
    * a request message.
    */
   switch (_session.state()) {
      case AAASampleServer::IDLE:

         ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: ^^^^^ New Auth Request\n"));
         _session.Update(AAASession::EVENT_AUTH_SUCCESS); 
         _session.state(AAASampleServer::AUTHORIZED);
         _session.SendTestAuthAnswer(msg);
         break;

      case AAASampleServer::AUTHORIZED:
      case AAASampleServer::TERMINATING:
      default:
         ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: ^^^^^ Responding for session # %d\n", 
                   _session.sessionIndex()));

         _session.SendTestAuthAnswer(msg);

         if (AAASampleServer::sendAbort()) {
	    _session.Abort();
	 }
	 break;
   }
   return (AAA_ERR_SUCCESS);
}

AAASampleServer::AAASampleServer(AAAApplicationCore &appCore, 
                                 diameter_unsigned32_t appId) :
   AAAServerSession(appCore, appId),
   _answerMsgHandler(AAASampleAuthAnswerMessage(*this, 300))
{
   /*
    * A simple servers session counter is maintained here
    * since the factory will create instances of this
    * class.
    *
    * Note also that we store references to message handler
    * objects as part of the session object. This is
    * convinient since the bindings between message handlers
    * and session is by being a member variables.
    */

   _state = AAASampleServer::IDLE;

   AAASampleServer::_sessionCount ++;

   _sessionIndex = AAASampleServer::_sessionCount;
}

AAAReturnCode AAASampleServer::HandleMessage(AAAMessage &msg)
{
   /* 
    * After being created by the factory on receipt of a
    * request message, this default HandleMessage is called 
    * to process the request.
    *
    * Note that by being the default message handler. Any 
    * message belonging to the session but does not have a 
    * registered handler will also be passed to this method.
    *
    * In this case, we treat any other message as an 
    * unknown. The application may send an unknown 
    * message error if it wishes.
    */
   ACE_ERROR((LM_ERROR, "(%P|%t) Server: **** Unknown message received **** \n"));
   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::HandleDisconnect()
{
   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Ending session for session # %d\n",  
             _sessionIndex));

   /*
    * This handler can be called when the transport
    * fails, when a session termination request is
    * received or when an application core is exiting.
    *
    * In the case of this sample, we are expecting
    * this function to get called on events other
    * than the receipt of a termination request.
    */

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::HandleSessionTimeout()
{
   /* 
    * Called on session timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Session timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::HandleAuthLifetimeTimeout()
{
   /* 
    * Called on auth lifetime timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Authorization lifetime timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::HandleAuthGracePeriodTimeout()
{
   /* 
    * Called on auth grace period timeout
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Authorization grace period timeout event for session # %d\n",  
              _sessionIndex));

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::HandleAbort()
{
   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Abort event for session # %d\n",  
             _sessionIndex));

   /*
    * Called when an abort session request is
    * received. Internally, the library has
    * received an ASR from the client and hence
    * the session will be terminated.
    * This sample does not do anything upon
    * receipt of this event.
    */

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleServer::SendTestAuthAnswer(AAAMessage &request)
{
   /*
    * Sample authorization request answer. This message
    * composition follows libdiamparser rules in
    * composing an AAAMessage. 
    */

   AAAAvpContainerManager cm;
   AAAAvpContainerEntryManager em;

   AAAAvpContainer *c_sessId = cm.acquire("Session-Id");
   AAAAvpContainer *c_orhost = cm.acquire("Origin-Host");
   AAAAvpContainer *c_orrealm = cm.acquire("Origin-Realm");
   AAAAvpContainerEntry *e;

   e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
   c_sessId->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orhost->add(e);
 
   e = em.acquire(AAA_AVP_DIAMID_TYPE);
   c_orrealm->add(e);
 
   AAAMessage acaMsg;
   hdr_flag flag = {0,0,0,0,0};
   AAADiameterHeader h(1, 0, flag, request.hdr.code, 10000, 0, 0);
   acaMsg.hdr = h;

   acaMsg.acl.add(c_sessId);
   acaMsg.acl.add(c_orhost);
   acaMsg.acl.add(c_orrealm);

   /*
    * The use of AAAMessageControl is shown here.
    * It is a utility class used in sending messages
    * via the local session object as well as setting
    * the result code.
    */
   AAAMessageControl msgControl(this);
   msgControl.SetResultCode(acaMsg, request, AAA_SUCCESS);
   if (msgControl.Send(acaMsg) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Failed sending message\n"));
      return (AAA_ERR_FAILURE);
   }
   else {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Sent Auth Message\n"));
   }

   return (AAA_ERR_SUCCESS);
}

AAASampleAccountingServer::AAASampleAccountingServer(AAAApplicationCore &appCore, 
                                                     diameter_unsigned32_t appId) 
  : AAAAccountingServerSession(appCore, appId), xml(*this)
{
   // we assign the instance of our XML transformer
   // to the base class so it can invoke it when
   // needed
   SetTransformer(&xml);
}

AAASampleAccountingServer::~AAASampleAccountingServer()
{
   // do nothing
}

AAAReturnCode AAASampleAccountingServer::SendAcctAnswer(AAAMessage &request)
{
   /*
    * Sample accounting request. This message
    * composition follows libdiamparser rules in
    * composing an AAAMessage. The commandn code
    * for accounting request is AAASampleAccountingRequestMsg::ACR.
    */

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Sending test acct answer message\n"));

   AAAAvpContainerManager cm;
   AAAAvpContainerEntryManager em;

   AAAAvpContainer *c_sessId = cm.acquire("Session-Id");
   AAAAvpContainer *c_orhost = cm.acquire("Origin-Host");
   AAAAvpContainer *c_orrealm = cm.acquire("Origin-Realm");
   AAAAvpContainer *c_drealm = cm.acquire("Destination-Realm");
   AAAAvpContainer *c_rectype = cm.acquire("Accounting-Record-Type");
   AAAAvpContainer *c_recnum = cm.acquire("Accounting-Record-Number");
   AAAAvpContainer *c_acctid = cm.acquire("Accounting-Application-Id");
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
   hdr_flag flag = {0,0,0,0,0};
   AAADiameterHeader h(0, 0, flag, AAASampleAccountingServer::ACR, 20000, 0, 0);
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

   rectyp = 1; // event record only
   recnum = 0;
   acctid = 0; // sample only 
   user.assign("sample user");
   subid = 0; // sample only
   acctsid.assign("sample sid");
   acctmsid.assign("sample multi sid");
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
   msgControl.SetResultCode(acctMsg, request, AAA_SUCCESS);
   if (msgControl.Send(acctMsg) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Failed sending message\n"));
      return (AAA_ERR_FAILURE);
   }
   else {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Sent Acct Message\n"));
   }

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAASampleXMLTrans::OutputRecord(AAAMessage *originalMsg)
{
   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Accounting transformer successfully converted record\n"));

   if (record) {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Record Size: %d\n", record_size));
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Record: %s\n", (char*)record));

      /*
       * Note that it is the responsibility of the handler
       * to free the record holder when it is done
       * using it. In real applications, this section
       * MAY be used to store XML stream into databases.
       *
       * Note also that the default implementation of
       * the XML transformer OutputRecord method
       * automatically frees the record pointer.
       * Hence, you may invoke this method so it
       * handles record release for us.
       */
      AAAAccountingXMLRecTransformer::OutputRecord(originalMsg);
   }

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Sending answer back to client (test purpose only)\n"));
   return session.SendAcctAnswer(*originalMsg);
}

int main(int argc, char *argv[])
{
   /*
    * This sample server test program is straight forward.
    * After creating an instance of the application core
    * and passing it the configuration file, it registers
    * a class factory for handling requests with application
    * id 0. The the configuration filename is passed in as
    * an argument as well as the abort flag.
    * 
    * Note that a semaphore is used to mimic application
    * specific processing. On exit, the factory is removed
    * from the application core and the application cores
    * destructor will perform internal cleanup.
    *
    */

   if (argc != 3) {
      ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Usage: sample_server [wait|nowait] [config file]\n"));
      return (1);
   }

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Application starting\n"));

   AAA_Task myTask;
   try {
      myTask.Start(10);
   }
   catch (...) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Cannot start task\n"));
      return (1);
   }
   AAAApplicationCore myCore;
   if (myCore.Open(argv[2], myTask) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Application core open error\n"));
      return (1);
   }

   /* abort flag */
   std::string abort(argv[1]);
   AAASampleServer::sendAbort((abort == "wait") ? true : false);

   /* 
    * application id is for this sample
    */
   serverFactory authFactory(myCore, 10000);
   acctFactory acctFactory(myCore, 20000);
   
   /*
    * Register the factory to our application core.
    * All request with application id 0 will be
    * handled here.
    */
   myCore.RegisterServerSessionFactory(&authFactory);
   myCore.RegisterServerSessionFactory(&acctFactory);

   /*
    * This will block until all session are done.
    * In the case of this sample, this will block
    * indefinitely because we did not provide an
    * exit routine.
    */
   AAASampleServer::semaphore().acquire();

   myCore.RemoveServerSessionFactory(&authFactory);
   myCore.RemoveServerSessionFactory(&acctFactory);

   ACE_DEBUG((LM_DEBUG, "(%P|%t) Server: Exiting\n"));

   return (0);
}






