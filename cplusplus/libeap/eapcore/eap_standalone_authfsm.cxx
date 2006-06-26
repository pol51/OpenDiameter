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
// $Id: eap_standalone_authfsm.cxx,v 1.16 2004/08/10 16:41:06 vfajardo Exp $

// eap_standalone_authfsm.cxx:  StandAlone Authenticator state machine
// Written by Yoshihiro Ohba

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include "eap.hxx"
#include "eap_authfsm.hxx"
#include "eap_policy.hxx"
#include "eap_log.hxx"
#include "eap_parser.hxx"

/// Action class for EAP. 
class EapStandAloneAuthSwitchAction : 
  public AAA_Action<EapStandAloneAuthSwitchStateMachine>
{
 public:
  virtual void operator()(EapStandAloneAuthSwitchStateMachine&) {}
 protected:
  EapStandAloneAuthSwitchAction() {}
  virtual ~EapStandAloneAuthSwitchAction() {}
};

/// State table used by EapAuthSwitchStateMachine.
class EapStandAloneAuthSwitchStateTable_S 
  : public AAA_StateTable<EapStandAloneAuthSwitchStateMachine>
{
  friend class ACE_Singleton<EapStandAloneAuthSwitchStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;
private:
  class AcDisable : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      sm.Event(EapAuthSwitchStateMachine::EvSgPortEnabled);
    }
  };

  class AcInitialize : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      sm.CurrentIdentifier() = 0;  // Initilize the identifier
      sm.Event(EvUCT);
    }
  };

  class AcSendSuccess : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(4);

      EAP_LOG(LM_DEBUG, "Standalone: Composing Success message.\n");

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

  class AcSendFailure : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(4);
      EAP_LOG(LM_DEBUG, "Standalone: Composing Failure message.\n");

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

  class AcMethodResponse : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EapMethodStateMachine &msm = sm.MethodStateMachine();
      if (msm.IsDone())
	{
	  if (msm.KeyAvailable())
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

  class AcBuildRequest : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
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

  class AcSendRequest : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
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
	      "Standalone: Request sent and timer started.\n");
      if (sm.RetransmissionEnabled())
	sm.ScheduleTimer(EvTo, sm.RetransmissionInterval());

      sm.Notify(EvUCT);

      // Send the messsage.
      //      cb->Send(msg);
      sm.Send(msg);
    }
  };

  class AcDoIntegrityCheck : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Standalone: Integrity Check.\n");
      sm.MethodStateMachine().Notify
	(EapMethodStateMachine::EvSgIntegrityCheck);
    }
  };

  class AcProposeMethod : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EapType type = sm.Policy().CurrentMethod();

      if (!type.IsVSE())
	EAP_LOG(LM_DEBUG, "Standalone: Trying a legacy method.\n");
      else
	EAP_LOG(LM_DEBUG, "Standalone: Trying an extended method.\n");

      sm.DeleteMethodStateMachine();
      sm.CurrentMethod() = type;
      sm.CreateMethodStateMachine(type, Authenticator);
      sm.MethodStateMachine().Start();

      if (type == EapType(1) || type == EapType(1)) // Identity or Notification
	sm.MethodState() = EapAuthSwitchStateMachine::CONT;
      else
	sm.MethodState() = EapAuthSwitchStateMachine::PROPOSED;

      // Notify method
      sm.MethodStateMachine().Notify
	(EapMethodStateMachine::EvSgIntegrityCheck);
    }
  };

  class AcDoPolicyCheck : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EapAuthSwitchStateMachine::EapAuthDecision decision = sm.Decision();
      if (decision == EapAuthSwitchStateMachine::DecisionSuccess)
	sm.Event(EvSgPolicySat);
      else if (decision == EapAuthSwitchStateMachine::DecisionFailure)
	sm.Event(EvSgPolicyNotSat_EndSess);	
      else // continue
	sm.Event(EvElse);
    }
  };

  class AcDiscard : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Standalone: Message Discarded.\n");
      sm.DiscardCount()++;
      sm.Event(EvUCT);
    }
  };


  class AcDiscard2 : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
      EAP_LOG(LM_DEBUG, "Standalone: Message discarded without parsing.\n");
      sm.DiscardCount()++;
      AAAMessageBlock *msg;
      EapMessageQueue &q = sm.RxQueue();
      // Dequeue a message from receiving queue.
      q.dequeue_head((ACE_Message_Block*&)msg);

      // Release it.
      msg->release();
    }
  };

  class AcResetMethod : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {  
      AAAMessageBlock *msg=sm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "Standalone: Reset method due to receiving Nak.\n");
      // Parse the nak
      EapNakParser parser;
      EapNak nak;
      parser.setDictData(NULL);
      parser.setAppData(&nak);
      parser.setRawData(msg);
      parser.parseRawToApp();

      sm.DeleteRxMessage();

      // Update policy based on type list.
      EAP_LOG(LM_DEBUG, "Standalone: Update policy on Nak.\n");
      sm.Policy().Update(nak.TypeList());

      sm.Event(EvUCT);
    }
  };

  class AcRetransmit : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
    {
  
      if (sm.RetransmissionCount() < sm.MaxRetransmissionCount())
	{
	  EAP_LOG(LM_DEBUG, "Standalone: Retransmitting Request.\n");
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
	  EAP_LOG(LM_ERROR, "Standalone: Reach max retransmission count.\n");
	  sm.CancelTimer();
	  sm.Event(EvSgMaxRetransmission);
	}
    }
  };

  class AcReceiveMsg : public EapStandAloneAuthSwitchAction
  {
    void operator()(EapStandAloneAuthSwitchStateMachine &sm)
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
      if (resp.GetType() == EapType(3))
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
	  if (resp.GetType() != type)
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
    StReceived
  };

  EapStandAloneAuthSwitchStateTable_S()
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
    AddWildcardStateTableEntry(StIntegrityCheck, StIntegrityCheck);

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

    InitialState(StBegin);
  }
  ~EapStandAloneAuthSwitchStateTable_S() {}
};

typedef ACE_Singleton<EapStandAloneAuthSwitchStateTable_S, 
		      ACE_Recursive_Thread_Mutex> 
EapStandAloneAuthSwitchStateTable;

EapStandAloneAuthSwitchStateMachine::EapStandAloneAuthSwitchStateMachine
(ACE_Reactor &r, EapJobHandle &h) :
  EapAuthSwitchStateMachine(r, h),
  EapStateMachine<EapStandAloneAuthSwitchStateMachine>
  (*this, *EapStandAloneAuthSwitchStateTable::instance(), 
   r, *this, "standalone")
{}

EapStandAloneAuthSwitchStateMachine::~EapStandAloneAuthSwitchStateMachine() {}

