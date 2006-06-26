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
// $Id: eap_passthrough_authfsm.cxx,v 1.22 2004/09/29 15:42:43 yohba Exp $

// eap_passthrough_authfsm.cxx:  Passthrough Authenticator state machine
// Written by Yoshihiro Ohba

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include "eap.hxx"
#include "eap_authfsm.hxx"
#include "eap_policy.hxx"
#include "eap_log.hxx"
#include "eap_parser.hxx"

/// Action class for EAP. 
class EapPassThroughAuthSwitchAction : 
  public AAA_Action<EapPassThroughAuthSwitchStateMachine>
{
 public:
  virtual void operator()(EapPassThroughAuthSwitchStateMachine&) {}
 protected:
  EapPassThroughAuthSwitchAction() {}
  virtual ~EapPassThroughAuthSwitchAction() {}
};

/// State table used by EapAuthSwitchStateMachine.
class EapPassThroughAuthSwitchStateTable_S 
  : public AAA_StateTable<EapPassThroughAuthSwitchStateMachine>

{
  friend class ACE_Singleton<EapPassThroughAuthSwitchStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;
private:
  class AcDisable : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      sm.Event(EapAuthSwitchStateMachine::EvSgPortEnabled);
    }
  };

  class AcInitialize : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      sm.CurrentIdentifier() = 0;  // Initilize the identifier
      sm.Event(EvUCT);
    }
  };

  class AcSendSuccess : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(4);

      EAP_LOG(LM_DEBUG, "Passthrough: Composing Success message.\n");

      // Compose the PDU.
      EapHeaderParser parser;
      EapHeader header;
      header.code = Success;
      header.identifier = sm.CurrentIdentifier();
      header.length = 4;
      parser.setAppData(&header);
      parser.setRawData(msg);
      parser.parseAppToRaw();

      // Stop retransmission timer and reset retransmission counter.
      sm.CancelTimer();
      sm.RetransmissionCount()=0;

      // Make the key available. (not specified in the state machine document.)
      if (sm.KeyData().size()>0)
	sm.KeyAvailable() = true;
	
      // Call Success callback function.
      sm.Success(msg);
      msg->release();
    }
  };

  class AcSendFailure : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(4);
      EAP_LOG(LM_DEBUG, "Passthrough: Composing Failure message.\n");

      // Compose the PDU.
      EapHeaderParser parser;
      EapHeader header;
      header.code = Failure;
      header.identifier = sm.CurrentIdentifier();
      header.length = 4;
      parser.setAppData(&header);
      parser.setRawData(msg);
      parser.parseAppToRaw();

      // Stop retransmission timer and reset retransmission counter.
      sm.CancelTimer();
      sm.RetransmissionCount()=0;

      // Call Failure callback function.
      sm.Failure(msg);
      msg->release();
    }
  };

  class AcMethodResponse : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EapMethodStateMachine &msm = sm.MethodStateMachine();
      if (msm.IsDone())
	{
	  sm.KeyData() = msm.KeyData();
	  sm.MethodState() = EapAuthSwitchStateMachine::END;
	  sm.Event(EapAuthSwitchStateMachine::EvSgEndMethod);
	}
      else
	{
	  sm.MethodState() = EapAuthSwitchStateMachine::CONT;
	  sm.Event(EvElse);
	}
    }
  };

  class AcBuildRequest : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      // Calculate the next Identifier for the new request.
      ACE_Byte id = sm.CurrentIdentifier() = 
	sm.MethodStateMachine().GetNextIdentifier(sm.CurrentIdentifier());

      AAAMessageBlock *msg = sm.GetTxMessage();

      // Compose the PDU.
      EapHeaderParser headerParser;
      EapHeader header;
      header.code = Request;
      header.identifier = id;
      header.length = msg->length();
      headerParser.setAppData(&header);
      headerParser.setRawData(msg);
      headerParser.parseAppToRaw();

      // Set the wr_ptr to the tail of the message.
      msg->wr_ptr(msg->base()+header.length);

      sm.Event(EvUCT);
    }
  };

  class AcSendRequest : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      // Stop retransmission timer and reset retransmission counter.
      sm.CancelTimer();
      sm.RetransmissionCount()=0;

      AAAMessageBlock *msg = sm.GetTxMessage();

      // Call Send callback function.
      // Increment the retransmission count.
      sm.RetransmissionCount()++;

      // Schedule retransmssion timer
      EAP_LOG(LM_DEBUG, 
	      "Passthrough: Request sent and timer started.\n");
      if (sm.RetransmissionEnabled())
	sm.ScheduleTimer(EvTo, sm.RetransmissionInterval());

      sm.Notify(EvUCT);

      // Send the messsage.
      sm.Send(msg);
    }
  };

  class AcDoIntegrityCheck : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Passthrough: Integrity Check.\n");
      sm.MethodStateMachine().Notify
	(EapMethodStateMachine::EvSgIntegrityCheck);
    }
  };

  class AcProposeMethod : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EapType type = sm.Policy().CurrentMethod();

      if (!type.IsVSE())
	EAP_LOG(LM_DEBUG, "PassThrough: Trying a legacy method.\n");
      else
	EAP_LOG(LM_DEBUG, "PassThrough: Trying an extended method.\n");

      sm.DeleteMethodStateMachine();
      sm.CurrentMethod() = type;
      sm.CreateMethodStateMachine(type, Authenticator);
      sm.MethodStateMachine().Start();

      if (type == EapType(1) || type == EapType(2)) // Identity or Notification
	sm.MethodState() = EapAuthSwitchStateMachine::CONT;
      else
	sm.MethodState() = EapAuthSwitchStateMachine::PROPOSED;

      // Notify method
      sm.MethodStateMachine().Notify
	(EapMethodStateMachine::EvSgIntegrityCheck);
    }
  };

  class AcDoPolicyCheck : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EapAuthSwitchStateMachine::EapAuthDecision decision = sm.Decision();
      if (decision == EapAuthSwitchStateMachine::DecisionSuccess)
	sm.Event(EvSgPolicySat);
      else if (decision == EapAuthSwitchStateMachine::DecisionFailure)
	sm.Event(EvSgPolicyNotSat_EndSess);	
      else if (decision == EapAuthSwitchStateMachine::DecisionPassthrough)
	sm.Event(EvSgPassThrough);	
      else // continue
	sm.Event(EvElse);
    }
  };

  class AcDiscard : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Passthrough: Message Discarded.\n");
      sm.DiscardCount()++;
      sm.Event(EvUCT);
    }
  };


  class AcDiscard2 : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Passthrough: Message discarded without parsing.\n");
      sm.DiscardCount()++;
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.RxQueue();
      // Dequeue a message from receiving queue.
      q.dequeue_head((ACE_Message_Block*&)msg);

      // Release it.
      msg->release();
      sm.Event(EvUCT);
    }
  };

  class AcDiscard3 : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Passthrough: Message Discarded.\n");
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.AAARxQueue();

      // Dequeue a message from receiving queue.
      q.dequeue_head((ACE_Message_Block*&)msg);
      msg->release();
    }
  };

  class AcResetMethod : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {  
      AAAMessageBlock *msg=sm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "Passthrough: Reset method due to receiving Nak.\n");
      // Parse the nak
      EapNakParser parser;
      EapNak nak;
      parser.setDictData(NULL);
      parser.setAppData(&nak);
      parser.setRawData(msg);
      parser.parseRawToApp();

      sm.DeleteRxMessage();

      // Update policy based on type list.
      EAP_LOG(LM_DEBUG, "Passthrough: Update policy on Nak.\n");
      sm.Policy().Update(nak.TypeList());

      sm.Event(EvUCT);
    }
  };

  class AcRetransmit : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
        if (sm.RetransmissionCount() < sm.MaxRetransmissionCount())
	{
	  EAP_LOG(LM_DEBUG, "Passthrough: Retransmitting Request.\n");
	  // Get the message to retransmit.
	  AAAMessageBlock *msg = sm.GetTxMessage();

	  // Call Send callback function.
	  // Increment the retransmission count
	  sm.RetransmissionCount()++;

	  sm.Notify(EvElse);

	  // Execute the callback function for sending the messsage
	  //	  cb->Send(msg);
	  sm.Send(msg);

	}
      else
	{
	  EAP_LOG(LM_ERROR, "AuthSwitch: Reach max retransmission count.\n");
	  sm.CancelTimer();
	  sm.Event(EvSgMaxRetransmission);
	}
    }
  };

  class AcReceiveMsg : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;
      // Dequeue a message from receiving queue.
      sm.RxQueue().dequeue_head((ACE_Message_Block*&)msg);

      // Set the read pointer to the message head.
      msg->rd_ptr(msg->base());

      // Set msg to rxMessage.
      sm.SetRxMessage(msg);

      // Parse the header
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp();

      // Check code
      unsigned code = header.code;
      unsigned id = header.identifier;

      // Check code & identifier
      if ((code != Response) || (id != sm.CurrentIdentifier()))
	{
	  sm.DeleteRxMessage();
	  sm.Notify(EvElse);
	  return;
	}

      // Parse the response
      EapResponseParser parser;
      EapResponse resp;
      parser.setAppData(&resp);
      parser.setRawData(msg);
      parser.parseRawToApp();

      EapType type = sm.CurrentMethod();

      // Check response type.  If this is a Nak message and the current
      // method is not a pass-through method, then generate a Nak
      // reception event.
      if (resp.GetType() == EapType(3) && 
	  sm.Decision() != EapAuthSwitchStateMachine::DecisionPassthrough)
	{
	  if (sm.MethodState() == EapAuthSwitchStateMachine::PROPOSED)
	    {
	      sm.Event(EvRxNak);
	    }
	  else
	    {
	      sm.DeleteRxMessage();
	      sm.Event(EvElse);
	    }
	}
      else // Normal method
	{
	  if (resp.GetType() != type && 
	      sm.Decision() != EapAuthSwitchStateMachine::DecisionPassthrough)
	    {
	      sm.DeleteRxMessage();
	      sm.Event(EvElse);
	    }
	  else
	    {
	      sm.Event(EvRxMethodResp);
	    }
	}
    }
  };

  class AcReceiveMsg2 : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;

      // Dequeue a message from receiving queue.
      sm.RxQueue().dequeue_head((ACE_Message_Block*&)msg);

      // Set the read pointer to the message head.
      msg->rd_ptr(msg->base());

      // Set msg to rxMessage.
      sm.SetRxMessage(msg);

      // Parse the header
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp();

      // Check code
      unsigned code = header.code;
      unsigned id = header.identifier;

      // Check code & identifier
      if ((code != Response) || (id != sm.CurrentIdentifier()))
	{
	  sm.DeleteRxMessage();
	  sm.Notify(EvElse);
	  return;
	}

      sm.Event(EvRxMethodResp);
    }
  };

  class AcPickUpInitCheck : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      if (sm.GetRxMessage()==0)
	sm.Event(EvSgNoPickUpInit);
      else
	sm.Event(EvSgPickUpInit);
    }
  };

  class AcAaaRequest : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      sm.Event(EvUCT);
    }
  };

  class AcDoIntegrityCheck2 : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg=sm.GetRxMessage();

      // If msg is empty, the system has been just started.  So call
      // ForwardResponse with null argument and let the application do
      // an appropriate action (e.g, send AAA message to the EAP
      // server).  Otherwise, there is another method that has been
      // completed immediately before entering passthrough mode, with
      // receiving the first Response from the peer.  In this case, just
      // pick up the Response and forward to the backend.

      EAP_LOG(LM_DEBUG, "Passthough: Integrity check.\n");

      // Call the callback to forward the response.
      sm.ForwardResponse(msg);
      // stay in this state (wait external event).
    }
  };

  class AcAaaContinue : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.AAARxQueue();

      if (q.is_empty())
	{
	  sm.DeleteTxMessage();
	  EAP_LOG(LM_ERROR, 
		  "Passthrough: AAA message does not contain EAP-Request.\n");
	  sm.Abort();
	  return;
	}

      q.dequeue_head((ACE_Message_Block*&)msg);

      // Parse the message and determine the event to be passed.
      msg->rd_ptr(msg->base());

      // Parse the message
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp(); 

      unsigned code = header.code;

      // Check code
      if (code != Request)
	{
	  sm.Event(EvSgDiscard);
	  return;
	}

      // Set the received identifier as the current identifier.
      sm.CurrentIdentifier() = header.identifier;

      // Set the pass-through transmission message.
      sm.SetTxMessage(msg);

      if (code != Request)
	{
	  EAP_LOG(LM_ERROR, 
		  "Passthrough: AAA message does not contain EAP-Request.\n");
	  sm.Abort();
	  return;
	}

      // XXX: If we need to modify Identifier field, this is the place
      // to do it.
      sm.Event(EvRxAaaEapReq);
    }
  };

  class AcAaaSuccess : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.AAARxQueue();

      if (q.is_empty())
	{
	  sm.DeleteTxMessage();
	  sm.Event(EvSgAaaSuccess);
	  return;
	}
      q.dequeue_head((ACE_Message_Block*&)msg);

      // Parse the message and determine the event to be passed.
      msg->rd_ptr(msg->base());

      // Parse the message
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp(); 

      unsigned code = header.code;

      // Check code
      if (code != Success)
	{
	  sm.Event(EvSgDiscard);
	  return;
	}

      // Set the received identifier as the current identifier.
      sm.CurrentIdentifier() = header.identifier;

      // Set the pass-through transmission message.
      sm.SetTxMessage(msg);
      sm.Event(EvSgAaaSuccess);
    }
  };

  class AcAaaFailure : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.AAARxQueue();

      if (q.is_empty())
	{
	  sm.DeleteTxMessage();
	  sm.Event(EvSgAaaFailure);
	  return;
	}
      q.dequeue_head((ACE_Message_Block*&)msg);

      // Parse the message and determine the event to be passed.
      msg->rd_ptr(msg->base());

      // Parse the message
      EapHeaderParser hp;
      EapHeader header;
      hp.setAppData(&header);
      hp.setRawData(msg);
      hp.parseRawToApp(); 

      unsigned code = header.code;

      // Check code
      if (code != Failure)
	{
	  sm.Event(EvSgDiscard);
	  return;
	}

      // Set the received identifier as the current identifier.
      sm.CurrentIdentifier() = header.identifier;

      // Set the pass-through transmission message.
      sm.SetTxMessage(msg);
      sm.Event(EvSgAaaFailure);
    }
  };

  class AcPrepareRequest : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      sm.Event(EvUCT);
    }
  };

  class AcCompletePassThroughWithFailure 
    : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = sm.GetTxMessage();

      EAP_LOG(LM_DEBUG, "Passthrough: EAP authentication failed.\n");

      // Stop retransmission timer and reset retransmission counter.
      sm.CancelTimer();
      sm.RetransmissionCount()=0;

      // Call Failure callback function.
      if (msg)
	{
	  sm.Failure(msg);
	  // release the outstanding request.
	  sm.DeleteTxMessage();
	}
      else
	sm.Failure();
    }
  };

  class AcCompletePassThroughWithSuccess 
    : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = sm.GetTxMessage();

      EAP_LOG(LM_DEBUG, "Passthrough: EAP authentication succeeded.\n");

      // Stop retransmission timer and reset retransmission counter.
      sm.CancelTimer();
      sm.RetransmissionCount()=0;

      // Make the key available. (not specified in the state machine document.)
      if (sm.KeyData().size()>0)
	sm.KeyAvailable() = true;
	
      // Call Failure callback function.
      if (msg)
	{
	  sm.Success(msg);
	  // release the outstanding request.
	  sm.DeleteTxMessage();
	}
      else
	sm.Success();
    }
  };

  class AcReturnToSwitch : public EapPassThroughAuthSwitchAction
  {
    void operator()(EapPassThroughAuthSwitchStateMachine &sm)
    {
      sm.Event(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  AcDisable acDisable;
  AcInitialize acInitialize;
  AcProposeMethod acProposeMethod;
  AcBuildRequest acBuildRequest;
  AcMethodResponse acMethodResponse;
  AcSendSuccess acSendSuccess;
  AcSendFailure acSendFailure;
  AcSendRequest acSendRequest;
  AcDoIntegrityCheck acDoIntegrityCheck;
  AcDoPolicyCheck acDoPolicyCheck;
  AcDiscard acDiscard;
  AcDiscard2 acDiscard2;
  AcResetMethod acResetMethod;
  AcRetransmit acRetransmit;
  AcReceiveMsg acReceiveMsg;

  AcPickUpInitCheck acPickUpInitCheck;
  AcAaaRequest acAaaRequest;
  AcDoIntegrityCheck2 acDoIntegrityCheck2;
  AcCompletePassThroughWithSuccess acCompletePassThroughWithSuccess;
  AcCompletePassThroughWithFailure acCompletePassThroughWithFailure;
  AcPrepareRequest acPrepareRequest;
  AcAaaContinue acAaaContinue;
  AcAaaSuccess acAaaSuccess;
  AcAaaFailure acAaaFailure;
  AcReturnToSwitch acReturnToSwitch;
  AcDiscard3 acDiscard3;
  AcReceiveMsg2 acReceiveMsg2;

  enum event {
    EvUCT,
    EvSgPolicySat,
    EvSgPolicyNotSat_EndSess,
    EvSgPolicyNotSat_ContSess,
    EvRxMethodResp,
    EvRxNak,
    EvTo,
    EvSgMaxRetransmission,
    EvElse,
    EvSgPassThrough,
    EvSgPickUpInit,
    EvSgNoPickUpInit,
    EvRxAaaEapReq,
    EvSgAaaSuccess,
    EvSgAaaFailure,
    EvSgDiscard,
  };

  enum state {
    StBegin,
    StDisabled,
    StInitialize,
    StSelectAction,
    StProposeMethod,
    StIdle,
    StIntegrityCheck,
    StMethodRequest,
    StMethodResponse,

    StSendRequest,
    StNak,
    StRetransmit,
    StDiscard,
    StSuccess,
    StFailure,
    StTimeoutFailure,
    StReceived,

    StInitializePassThrough,
    StAaaRequest,
    StAaaIdle,
    StAaaResponse,
    StIdle2, 
    StReceived2,
    StSuccess2,
    StFailure2,
    StSendRequest2,
    StDiscard2,
    StRetransmit2,
    StTimeoutFailure2,
  };

  EapPassThroughAuthSwitchStateTable_S()
  {

    AddStateTableEntry(StBegin, EapAuthSwitchStateMachine::EvSgRestart,
		       StDisabled, acDisable);
    AddStateTableEntry(StDisabled, 
		       EapAuthSwitchStateMachine::EvSgPortEnabled,  
		       StInitialize, acInitialize);
    AddStateTableEntry(StInitialize, EvUCT,
		       StSelectAction, acDoPolicyCheck);

    AddStateTableEntry(StSelectAction, EvSgPolicySat,
		       StSuccess, acSendSuccess);
    AddStateTableEntry(StSelectAction, EvSgPolicyNotSat_EndSess,
		       StFailure, acSendFailure);
    AddStateTableEntry(StSelectAction, EvElse, 
		       StProposeMethod, acProposeMethod);
    AddStateTableEntry(StSelectAction, EvSgPassThrough, 
		       StInitializePassThrough, acPickUpInitCheck);
    AddStateTableEntry(StProposeMethod, 
		       EapAuthSwitchStateMachine::EvSgValidResp,
		       StMethodRequest, acBuildRequest);

    AddStateTableEntry(StMethodRequest, EvUCT, 
		       StSendRequest, acSendRequest);

    AddStateTableEntry(StIntegrityCheck, 
		       EapAuthSwitchStateMachine::EvSgValidResp,
		       StMethodResponse, acMethodResponse);
    AddStateTableEntry(StIntegrityCheck, 
		       EapAuthSwitchStateMachine::EvSgInvalidResp,
		       StDiscard, acDiscard);
    // This state table entry is needed for queueing incoming requests.
    AddStateTableEntry(StIntegrityCheck, EapAuthSwitchStateMachine::EvRxMsg,
		       StIntegrityCheck, acDiscard2);


    AddStateTableEntry(StMethodResponse, 
		       EapAuthSwitchStateMachine::EvSgEndMethod,
		       StSelectAction, acDoPolicyCheck);
    AddStateTableEntry(StMethodResponse, EvElse,
		       StMethodRequest, acBuildRequest);

    AddStateTableEntry(StSendRequest, EvUCT, StIdle);

    AddStateTableEntry(StIdle, EvTo, StRetransmit, acRetransmit);
    AddStateTableEntry(StIdle, EapAuthSwitchStateMachine::EvRxMsg,
		       StReceived, acReceiveMsg);

    AddStateTableEntry(StReceived, EvRxMethodResp,
		       StIntegrityCheck, acDoIntegrityCheck);
    AddStateTableEntry(StReceived, EvRxNak,
		       StNak, acResetMethod);
    AddStateTableEntry(StReceived, EvElse, 
		       StDiscard, acDiscard);

    AddStateTableEntry(StRetransmit, EvSgMaxRetransmission, StTimeoutFailure);
    AddStateTableEntry(StRetransmit, EvElse, StIdle);

    AddStateTableEntry(StNak, EvUCT, StSelectAction, acDoPolicyCheck);

    AddStateTableEntry(StDiscard, EvUCT, StIdle);

    AddStateTableEntry(StSuccess, EapAuthSwitchStateMachine::EvRxMsg,
		       StSuccess, acDiscard2);
    AddWildcardStateTableEntry(StSuccess, StSuccess);
    AddStateTableEntry(StFailure, EapAuthSwitchStateMachine::EvRxMsg,
		       StFailure, acDiscard2);
    AddWildcardStateTableEntry(StFailure, StFailure);

    // Passthrough diagram
    AddStateTableEntry(StInitializePassThrough, EvSgPickUpInit,
		       StAaaRequest, acAaaRequest);
    AddStateTableEntry(StInitializePassThrough, EvSgNoPickUpInit,
		       StAaaIdle, acDoIntegrityCheck2);

    AddStateTableEntry(StAaaRequest, EvUCT, 
		       StAaaIdle, acDoIntegrityCheck2);

    AddStateTableEntry(StAaaIdle, EvRxAaaEapReq,
		       StAaaResponse, acPrepareRequest);
    AddStateTableEntry(StAaaIdle, EvSgAaaSuccess,
		       StSuccess2, acCompletePassThroughWithSuccess);
    AddStateTableEntry(StAaaIdle, EvSgAaaFailure, 
		       StFailure2, acCompletePassThroughWithFailure);
    AddStateTableEntry(StAaaIdle, EapAuthSwitchStateMachine::EvSgAaaContinue,
		       StAaaIdle, acAaaContinue);
    AddStateTableEntry(StAaaIdle, EapAuthSwitchStateMachine::EvSgAaaSuccess,
		       StAaaIdle, acAaaSuccess);
    AddStateTableEntry(StAaaIdle, EapAuthSwitchStateMachine::EvSgAaaFailure,
		       StAaaIdle, acAaaFailure);
    AddStateTableEntry(StAaaIdle, EvSgDiscard,
		       StDiscard2, acDiscard3);
    AddWildcardStateTableEntry(StAaaIdle, StAaaIdle);

    AddStateTableEntry(StDiscard2, EvUCT, StIdle2);

    AddStateTableEntry(StReceived2, EvRxMethodResp,
		       StAaaRequest, acAaaRequest);
    AddStateTableEntry(StReceived2, EvElse, 
		       StDiscard2, acDiscard);

    AddStateTableEntry(StIdle2, EapAuthSwitchStateMachine::EvRxMsg,
		       StReceived2, acReceiveMsg2);
    AddStateTableEntry(StIdle2, EapAuthSwitchStateMachine::EvSgAaaContinue,
		       StIdle2, acDiscard3);
    AddStateTableEntry(StIdle2, EvTo, StRetransmit2, acRetransmit);
    AddWildcardStateTableEntry(StIdle2, StIdle2);

    AddStateTableEntry(StAaaResponse, EvUCT, StSendRequest2, acSendRequest);
    AddStateTableEntry(StSendRequest2, EvUCT, StIdle2);

    AddStateTableEntry(StSuccess2, EapAuthSwitchStateMachine::EvSgAaaContinue,
		       StSuccess2, acDiscard3);
    AddWildcardStateTableEntry(StSuccess2, StSuccess2);
    AddStateTableEntry(StFailure2, EapAuthSwitchStateMachine::EvSgAaaContinue,
		       StFailure2, acDiscard3);
    AddWildcardStateTableEntry(StFailure2, StFailure2);

    AddStateTableEntry(StRetransmit2, EvSgMaxRetransmission, 
		       StTimeoutFailure2);
    AddStateTableEntry(StRetransmit2, EvElse, StIdle2);

    InitialState(StBegin);
  }
  ~EapPassThroughAuthSwitchStateTable_S() {}
};

typedef ACE_Singleton<EapPassThroughAuthSwitchStateTable_S, 
		      ACE_Recursive_Thread_Mutex> 
EapPassThroughAuthSwitchStateTable;

EapPassThroughAuthSwitchStateMachine::EapPassThroughAuthSwitchStateMachine
(ACE_Reactor &r, EapJobHandle &h) :
  EapAuthSwitchStateMachine(r, h),
  EapStateMachine<EapPassThroughAuthSwitchStateMachine>
  (*this, *EapPassThroughAuthSwitchStateTable::instance(), r, 
   *this, "passthrough")
{}

EapPassThroughAuthSwitchStateMachine::~EapPassThroughAuthSwitchStateMachine() 
{}

EapAuthSwitchStateMachine::EapAuthDecision
EapPassThroughAuthSwitchStateMachine::Decision()
{
  EapType type;

  try {
    type = policy.CurrentMethod();
  } 
  catch (EapPolicy::PolicyError e) {  // No method to try.
    if (e == EapPolicy::NoCurrentMethod)
      return DecisionPassthrough;
  }
  return DecisionContinue;
}

