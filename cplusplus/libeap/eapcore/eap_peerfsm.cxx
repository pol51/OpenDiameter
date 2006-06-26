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
// $Id: eap_peerfsm.cxx,v 1.51 2004/08/10 16:41:06 vfajardo Exp $
//
// eap_peerfsm.cxx:  Peer state machine
// Written by Yoshihiro Ohba

#include <ace/Singleton.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>
#include <ace/Synch.h>
#include <ace/OS.h>
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_log.hxx"
#include "eap_parser.hxx"

int
InputIdentityMethodRequest::call()
{
  return futureResult.set(stateMachine->InputIdentity());
}

/// Action class for EAP. 
class EapPeerAction : public AAA_Action<EapPeerSwitchStateMachine>
{
 public:
  virtual void operator()(EapPeerSwitchStateMachine&) {}
 protected:
  EapPeerAction() {}
  virtual ~EapPeerAction() {}
};

/// State table used by EapPeerSwitchStateMachine.
class EapPeerSwitchStateTable_S : 
  public AAA_StateTable<EapPeerSwitchStateMachine>
{
  friend class ACE_Singleton<EapPeerSwitchStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:
  class AcInitialize : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      // allowNotification=TRUE;
      sm.NotificationAllowed() = true;
      sm.MethodState() = EapPeerSwitchStateMachine::NONE;
      sm.Event(EvUCT);
    }
  };
  class AcRetransmit : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Peer:  Detect duplicate request.\n");
      SubAcSendResp(sm);
      sm.Event(EvUCT);
    }
  };

  class AcCheckTimeoutCondition : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      if (sm.Policy().IsSatisfied() && 
	  sm.Decision() == EapPeerSwitchStateMachine::COND_SUCC)
	sm.Event(EvSgSuccess);
      else
	sm.Event(EvSgFailure);
    }
  };

  class AcSendSuccess : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      sm.CancelTimer();
      EAP_LOG(LM_DEBUG, "Peer:  Success.\n");
      if (sm.KeyData().size()>0)
	sm.KeyAvailable() = true;
      sm.Success();
    }
  };

  class AcSendFailure : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      sm.CancelTimer();
      EAP_LOG(LM_DEBUG, "Peer:  Failure.\n");
      sm.Failure();
    }
  };

  class AcBuildResp : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = sm.GetTxMessage();

      EAP_LOG(LM_DEBUG, "Peer:  Sent Response.\n");
      // Compose the PDU.
      EapHeaderParser headerParser;
      EapHeader header;
      header.code = Response;
      header.identifier = sm.CurrentIdentifier();
      header.length = msg->length();
      headerParser.setAppData(&header);
      headerParser.setRawData(msg);
      headerParser.parseAppToRaw();

      // Set the wr_ptr to the tail of the message.
      msg->wr_ptr(msg->base()+header.length);

      sm.LastIdValidity() = true;

      // Set the received identifier to the currentId
      sm.LastIdentifier() = sm.CurrentIdentifier();

      EAP_LOG(LM_DEBUG, "checking key availability.\n");
      if (sm.MethodState() == EapPeerSwitchStateMachine::NONE ||
	  sm.MethodState() == EapPeerSwitchStateMachine::INIT)
	goto next;

      // Check the availability of a key.
      if (sm.MethodStateMachine().KeyAvailable())
	{
	  EAP_LOG(LM_DEBUG, "key available.\n");
	  sm.KeyData() = sm.MethodStateMachine().KeyData();
	}

    next:
      // Send it.
      SubAcSendResp(sm);
    }
  };

  class AcDoIntegrityCheck : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Peer:  Do Integrity Check.\n");
      sm.MethodStateMachine().Notify
	(EapMethodStateMachine::EvSgIntegrityCheck);
    }
  };

  class AcDoPolicyCheck : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Peer:  Do Policy Check.\n");
      EapType &type = sm.ReqMethod();
      bool allowMethod;

      if (type == EapType(1))
	// This is Request/Identity.  If this is the first method, it
	// accepts the request.
	{
	  if (sm.CurrentMethod() == EapType(0))
	    allowMethod = true;
	  else
	    allowMethod = false;
	}
      else
	{
	  allowMethod = sm.Policy().Allow(type);
	}

      if (allowMethod) {
	// Set the current method type
	sm.CurrentMethod() = type;

	// Switch to new method
	EAP_LOG(LM_DEBUG, "Peer:  Creating Method.\n");
	// Delete the old method (if any)
	sm.DeleteMethodStateMachine();

	// Create a new method with a specific type
	sm.CreateMethodStateMachine(type, Peer);

	// Change the method state to INIT.
	sm.MethodState() = EapPeerSwitchStateMachine::INIT;

	// Start the method
	sm.MethodStateMachine().Start();

	// Enter the method
	sm.Event(EvSgEnterMethod);
      } 
      else {
	sm.Event(EvSgNoEnterMethod);
      }
    }
  };

  class AcDiscard : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Peer:  Message Discarded.\n");
      sm.DiscardCount()++;
      sm.Event(EvUCT);
    }
  };

  class AcBuildNak : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      // Create a type list from the current policy.
      EapType type = sm.CurrentMethod();
      EapNak nak;
      EapTypeList &tList = nak.TypeList();
      sm.Policy().MakeTypeList(tList, type.IsVSE());

      // Calculate NAK type and payload size.
      int payloadLength = tList.size();
      EapNakDict::NakType nakType = EapNakDict::LegacyNak;

      if (tList.IsVSE())
	{
	  payloadLength *= 8;
	  nakType = EapNakDict::ExtendedNak;
	}

      // Create NAK.
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(4 + payloadLength); 

      // Use parser to set Type field.
      EapNakParser payloadParser;
      payloadParser.setDictData(NULL);
      payloadParser.setAppData(&nak);
      payloadParser.setRawData(msg);
      EapNakDict dict(nakType);
      payloadParser.setDictData(&dict);
      payloadParser.parseAppToRaw();

      // Set the message to the session.
      sm.SetTxMessage(msg);

      EAP_LOG(LM_DEBUG, "Peer:  Sent Nak.\n");

      // Compose the PDU.
      EapHeaderParser headerParser;
      EapHeader header;
      header.code = Response;
      header.identifier = sm.CurrentIdentifier();
      header.length = msg->length();
      headerParser.setAppData(&header);
      headerParser.setRawData(msg);
      headerParser.parseAppToRaw();
    
      // Set the wr_ptr to the tail of the message.
      msg->wr_ptr(msg->base()+header.length);

      sm.LastIdValidity() = true;

      // Send it.
      SubAcSendResp(sm);
    }
  };

  class AcParseNotification : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = sm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "Peer: Parsing Notification request.\n");
      EapRequestNotificationParser parser;
      // Check the Notification.
      EapNotification notification;
      parser.setRawData(msg);
      parser.setAppData(&notification);
      try { parser.parseRawToApp(); }
      catch (...)
	{
	  // When an identity request contains non-UTF8 string, silently
	  // discard it as if it were not received.
	  EAP_LOG(LM_ERROR, "Peer: Notification parsing failed.\n");
	  sm.Event(EvElse);
	  return;
	}

      // Output notification to the application.  
      sm.Notification(sm.NotificationString());
      sm.Event(EvSgNotificationOutputFinished);
    }
  };

  class AcBuildNotification : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5);
      EapResponse response(EapType(2)); // Notification

      // Use parser to set Type field.
      EapResponseParser payloadParser;
      payloadParser.setAppData(&response);
      payloadParser.setRawData(msg);
      payloadParser.parseAppToRaw();

      // Set the message to the session.
      sm.SetTxMessage(msg);

      sm.Event(EvUCT);
    }
  };

  class AcParseIdentity : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = sm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "Peer: Parsing Identity request.\n");

      // Check the Identity.
      EapIdentity id;
      EapRequestIdentityParser parser;
      parser.setAppData(&id);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...)
	{
	  // When an identity request contains non-UTF8 string, silently
	  // discard it as if it were not received.
	  EAP_LOG(LM_ERROR, "Peer: Identity request parsing failed.\n");
	  sm.Event(EvElse);
	  return;
	}

      sm.AuthenticatorIdentity() = id.Identity();

      // Get identity from the application.  Since calling
      // InputIdenity() typically involves in user input, a new task
      // is created for exectuting InputIdenity().

      sm.InputIdentityTask().open();

      sm.FutureIdentity() = sm.InputIdentityTask().InputIdentity();

      if (sm.FutureIdentity().ready())
	{
	  sm.FutureIdentity().get(sm.PeerIdentity());
	  sm.InputIdentityTask().suspend();
	  sm.Notify(EvSgUserInputFinished);
	}
      else
	{
	  // Immediate check for the first time.
	  sm.ScheduleTimer(EvToCheckIdentity, 0, 100000, 
			   sm.InputIdentityTimerType());
	  return;
	}
    }
  };

  class AcBuildIdentity : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Peer: Building Identity response.\n");

      // Prepare the Response
      EapIdentity id;
      std::string& idString = id.Identity() = sm.PeerIdentity();
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5 + idString.size());
    
      EapResponseIdentityParser parser;
      parser.setAppData(&id);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...)
	{
	  EAP_LOG(LM_ERROR, "Peer: Identity data is not UTF8 formatted.\n");
	  sm.Event(EvElse);
	  return;
	}

      // Set the message to the session.
      sm.SetTxMessage(msg);
    
      // Set the last id validity.
      sm.LastIdValidity() = true;

      sm.Event(EvUCT);
    }
  };

  class AcStartTimer : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      sm.ScheduleTimer(EvTo, sm.AuthPeriod());
      EAP_LOG(LM_DEBUG, "Peer:  Timer Started.\n");
    }
  };

  class AcCheckRxQueue : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      EapMessageQueue &q = sm.RxQueue();
      // if rxQueue is active, generate message reception event.
      if (q.message_count()>0)
	sm.Event(EapPeerSwitchStateMachine::EvRxMsg);
      // otherwise, wait external event 
    }
  };

  class AcReceiveMsg : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.RxQueue();

      // dequeue a message from receiving queue.
      q.dequeue_head((ACE_Message_Block*&)msg);
      sm.SetRxMessage(msg);

      msg->rd_ptr(msg->base());

      // parse the message
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp(); 

      ACE_Byte code = header.code;
      ACE_Byte id = header.identifier;

      // Check code
      if (code != Request && code != Success && code != Failure)
	{
	  // Invalid code.  Delete the message.
	  sm.Event(EvElse);
	  return;
	}

      // rxReq
      if (code == Request)
	{
	  // Check identifier
	  if (sm.ReceivedFirstRequest() && id == sm.LastIdentifier())
	    {
	      // We are on the "rxReq && reqId == lastId" branch.
	      // This is a duplicate request.  If there is a valid
	      // request for which a response was already returned,
	      // send the copy of the response.  Otherwise, discard the
	      // request.

	      if (sm.LastIdValidity())
		{
		  sm.Event(EvSgRetransmit);
		}
	      else
		{
		  sm.Event(EvElse);
		}
	    }
	  else
	    {
	      // Set the received identifier to the currentId
	      sm.CurrentIdentifier() = id;

	      if (!sm.ReceivedFirstRequest())
		sm.ReceivedFirstRequest()=true;

	      SubAcParseRequest(sm);
	    }
	}
      else if (code == Success)
	{
	  EAP_LOG(LM_DEBUG, "Peer: Success received.\n");
	  SubAcCheckSuccessCondition(sm);
	}
      else // if (code == Failure)
	{
	  EAP_LOG(LM_DEBUG, "Peer: Failure received.\n");
	  SubAcCheckFailureCondition(sm);
	}
    }
  };

  class AcCheckIdentity : public EapPeerAction
  {
    void operator()(EapPeerSwitchStateMachine &sm)
    {
      sm.CancelTimer(sm.InputIdentityTimerType());

      if (sm.FutureIdentity().ready())
	{
	  sm.FutureIdentity().get(sm.PeerIdentity());
	  sm.InputIdentityTask().suspend();
	  sm.Notify(EvSgUserInputFinished);
	  sm.CancelTimer(sm.InputIdentityTimerType());
	}
      else
	{
	  sm.ScheduleTimer(EvToCheckIdentity, 0, 100000, 
			   sm.InputIdentityTimerType());
	}
    }
  };

  /// Internal event definition
  enum event {
    EvUCT=100,
    EvSgNewMethod,
    EvSgContMethod,
    EvSgEnterMethod,
    EvSgNoEnterMethod,
    EvSgSuccess,
    EvSgFailure,
    EvSgRetransmit,
    EvSgUserInputFinished,
    EvSgNotificationOutputFinished, 
    EvRxRequest,
    EvRxSuccess,
    EvRxFailure,
    EvRxNotification,
    EvRxIdentity,
    EvElse,
    EvTo,
    EvToCheckIdentity,
  };

  /// Internal state definition
  enum state {
    StDisabled,
    StInitialize,
    StIdle,
    StReceived,
    StMethod,
    StGetMethod,
    StDiscard,
    StRetransmit,
    StSendResponse,
    StIdentity,
    StNotification,
    StSuccess,
    StFailure
  };

  AcInitialize acInitialize;
  AcRetransmit acRetransmit;
  AcCheckTimeoutCondition acCheckTimeoutCondition;
  AcSendSuccess acSendSuccess;
  AcSendFailure acSendFailure;
  AcBuildResp acBuildResp;
  AcDoIntegrityCheck acDoIntegrityCheck;
  AcDoPolicyCheck acDoPolicyCheck;
  AcDiscard acDiscard;
  AcBuildNak acBuildNak;
  AcParseNotification acParseNotification;
  AcBuildNotification acBuildNotification;
  AcParseIdentity acParseIdentity;
  AcBuildIdentity acBuildIdentity;
  AcStartTimer acStartTimer;
  AcCheckRxQueue acCheckRxQueue;
  AcReceiveMsg acReceiveMsg;
  AcCheckIdentity acCheckIdentity;

  EapPeerSwitchStateTable_S()
  {
    AddStateTableEntry(StDisabled, 
		       EapPeerSwitchStateMachine::EvSgPortEnabled,  
		       StInitialize, acInitialize);

    AddStateTableEntry(StInitialize, EvUCT,          
		       StIdle, acStartTimer);

    AddStateTableEntry(StIdle, EapPeerSwitchStateMachine::EvRxMsg,
		       StReceived, acReceiveMsg);
    AddStateTableEntry(StIdle, EvTo,                 
		       StIdle, acCheckTimeoutCondition);
    AddStateTableEntry(StIdle, EvSgSuccess,
		       StSuccess, acSendSuccess);
    AddStateTableEntry(StIdle, EvSgFailure,
		       StFailure, acSendFailure);

    AddStateTableEntry(StReceived, EvSgSuccess,
		       StSuccess, acSendSuccess);
    AddStateTableEntry(StReceived, EvSgFailure,
		       StFailure, acSendFailure);
    AddStateTableEntry(StReceived, EvSgRetransmit,          
		       StRetransmit, acRetransmit);
    AddStateTableEntry(StReceived, EvSgContMethod,           
		       StMethod, acDoIntegrityCheck);
    AddStateTableEntry(StReceived, EvSgNewMethod,            
		       StGetMethod, acDoPolicyCheck);
    AddStateTableEntry(StReceived, EvRxNotification,
		       StNotification, acParseNotification);
    AddStateTableEntry(StReceived, EvRxIdentity,
		       StIdentity, acParseIdentity);
    AddStateTableEntry(StReceived, EvElse, 
		       StDiscard, acDiscard);

    AddStateTableEntry(StIdentity, EvSgUserInputFinished,
		       StIdentity, acBuildIdentity);
    AddStateTableEntry(StIdentity, EvUCT, 
		       StSendResponse, acBuildResp);
    // The following two entries are needed because identity
    // processing is asynchronously done.
    AddStateTableEntry(StIdentity, EvTo,                     
		       StFailure, acSendFailure);
    AddStateTableEntry(StIdentity, EvToCheckIdentity,
		       StIdentity, acCheckIdentity);
    AddWildcardStateTableEntry(StIdentity, StIdentity);

    AddStateTableEntry(StNotification, EvSgNotificationOutputFinished, 
		       StNotification, acBuildNotification);
    AddStateTableEntry(StNotification, EvUCT, 
		       StSendResponse, acBuildResp);
    // The following two entries are needed because identity
    // processing is asynchronously done.
    AddStateTableEntry(StNotification, EvTo,                     
		       StFailure, acSendFailure);
    AddStateTableEntry(StNotification, EvElse, 
		       StDiscard, acDiscard);
    AddWildcardStateTableEntry(StNotification, StIdentity);

    AddStateTableEntry(StGetMethod, EvSgEnterMethod,      
		       StMethod, acDoIntegrityCheck);
    AddStateTableEntry(StGetMethod, EvSgNoEnterMethod,    
		       StSendResponse, acBuildNak);
    AddWildcardStateTableEntry(StGetMethod, StGetMethod);

    AddStateTableEntry(StMethod, EapPeerSwitchStateMachine::EvSgValidReq,
		       StSendResponse, acBuildResp);
    AddStateTableEntry(StMethod, EapPeerSwitchStateMachine::EvSgInvalidReq,
		       StDiscard, acDiscard);
    AddStateTableEntry(StMethod, EvSgFailure,
		       StFailure, acSendFailure);

    // The following two entries are needed because method
    // processing is asynchronously done.
    AddStateTableEntry(StMethod, EvTo,                     
		       StMethod, acCheckTimeoutCondition);
    AddStateTableEntry(StMethod, EvSgSuccess,
		       StSuccess, acSendSuccess);
    AddWildcardStateTableEntry(StMethod, StMethod);

    AddStateTableEntry(StDiscard, EvUCT, 
		       StIdle, acCheckRxQueue);

    AddStateTableEntry(StSuccess, EapPeerSwitchStateMachine::EvRxMsg,
		       StSuccess, acReceiveMsg);
    AddWildcardStateTableEntry(StSuccess, StSuccess);
    AddStateTableEntry(StFailure, EapPeerSwitchStateMachine::EvRxMsg,
		       StFailure, acReceiveMsg);
    AddWildcardStateTableEntry(StFailure, StFailure);

    AddStateTableEntry(StRetransmit, EvUCT, StSendResponse);

    AddStateTableEntry(StSendResponse, EvUCT,
		       StIdle, acCheckRxQueue);

    InitialState(StDisabled);
  };

  static void SubAcCheckSuccessCondition(EapPeerSwitchStateMachine &sm)
  {
    if (sm.Decision() != EapPeerSwitchStateMachine::FAIL)
      {
	sm.Event(EvSgSuccess);
      }
    else if (sm.MethodState() != EapPeerSwitchStateMachine::CONT)
      {
	sm.Event(EvSgFailure);
      }
    else
      {
	sm.Event(EvElse);
      }
  }

  static void SubAcCheckFailureCondition(EapPeerSwitchStateMachine &sm)
  {
    if (sm.MethodState() != EapPeerSwitchStateMachine::CONT && 
	sm.Decision() != EapPeerSwitchStateMachine::UNCOND_SUCC)
      {
	sm.Event(EvSgFailure);
      }
    else
      {
	sm.Event(EvElse);
      }
  }

  static void SubAcParseRequest(EapPeerSwitchStateMachine &sm)
  {
    EAP_LOG(LM_DEBUG, "Peer:  Parse Request.\n");
    AAAMessageBlock *msg=sm.rxMessage;

    // parse the request
    EapRequestParser parser;
    EapRequest req;
    parser.setAppData(&req);
    parser.setRawData(msg);
    parser.parseRawToApp();

    EapType type = sm.reqMethod = req.GetType();

    if (type == EapType(2))  // Notification
      {
	if (!sm.NotificationAllowed())
	  // Discard the notification if the peer is not allowed to
	  // process the notification.
	  {
	    sm.Event(EvElse);
	  }
	else
	  {
	    // Set the current method type
	    sm.CurrentMethod() = type;

	    // Extract the notification string.
	    std::string notifStr(msg->rd_ptr(), msg->size()-
				 (msg->rd_ptr()-msg->base()));

	    // Execute the notification callback.
	    sm.Notification(notifStr);

	    // Generate an event to send Response/Notification.
	    sm.Event(EvRxNotification);
	  }
	   
      } 
    else if (sm.CurrentMethod() == EapType(0) && type == EapType(1))
      {

	// This is an Identity request.
	sm.Event(EvRxIdentity);
      }
    else if (sm.CurrentMethod() == EapType(0))
      {
	EAP_LOG(LM_DEBUG, "Peer:  New Method.\n");
	sm.Event(EvSgNewMethod);
      }
    else if (type == sm.CurrentMethod() && 
	     sm.MethodState() != EapPeerSwitchStateMachine::DONE)

      {
	sm.Event(EvSgContMethod);
      }
    else 
      {
	sm.Event(EvElse);
      }
  }

  static void SubAcSendResp(EapPeerSwitchStateMachine &sm)
  {
    AAAMessageBlock *msg = sm.GetTxMessage();
    
    // Call Send callback function.
    sm.Notify(EvUCT);
    sm.Send(msg);
  }
};

typedef ACE_Singleton<EapPeerSwitchStateTable_S, ACE_Recursive_Thread_Mutex> 
EapPeerSwitchStateTable;

const ACE_UINT16 EapPeerSwitchStateMachine::defaultAuthPeriod=10;

EapPeerSwitchStateMachine::EapPeerSwitchStateMachine
(ACE_Reactor &r, EapJobHandle &h) 
  :  EapSwitchStateMachine(r, h),
     EapStateMachine<EapPeerSwitchStateMachine>
  (*this, *EapPeerSwitchStateTable::instance(), r, *this, "peer"),
     authPeriod(defaultAuthPeriod),
     receivedFirstRequest(false), 
     inputIdentityTimerType(AAA_TimerTypeAllocator::instance()->Allocate())
{
  inputIdentityTask.Set(this);
}

void
EapPeerSwitchStateMachine::Start() throw(AAA_Error)
{
  // Set the current policy element to point to the initial policy
  // element.
  policy.CurrentPolicyElement(policy.InitialPolicyElement());

  // Delete the last executed method if any.
  DeleteMethodStateMachine();

  EapStateMachine<EapPeerSwitchStateMachine>::Start();
  lastIdValidity = false;
  currentIdentifier = 0;
  currentMethod = EapType(0);
  keyAvailable = false;
  keyData.resize(0);
  Notify(EvSgPortEnabled);
}

void
EapPeerSwitchStateMachine::Receive(AAAMessageBlock* msg)
{
  // Enqueue the received message with increasing reference counter
  // and generate a notification event.  Since rxQueue is thread-safe,
  // enqueue_tail does not need session mutex lock.

  rxQueue.enqueue_tail(AAAMessageBlock::Acquire(msg));
  Notify(EapPeerSwitchStateMachine::EvRxMsg);
}

