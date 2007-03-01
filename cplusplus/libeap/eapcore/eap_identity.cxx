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
// $Id: eap_identity.cxx,v 1.35 2004/06/17 21:13:35 yohba Exp $

// eap_identity.cxx:  EAP Identity method state machine
// Written by Yoshihiro Ohba

#include <string>
#include <ace/Singleton.h>
#include <ace/OS_String.h>
#include <ace/Message_Block.h>
#include <ace/Thread.h>
#include "eap.hxx"
#include "eap_identity.hxx"
#include "eap_method_registrar.hxx"
#include "eap_parser.hxx"
#include "eap_log.hxx"

/// Action class.
class EapAuthIdentityAction : public AAA_Action<EapAuthIdentityStateMachine>
{
 public:
  virtual void operator()(EapAuthIdentityStateMachine&) {}
 protected:
  EapAuthIdentityAction() {}
  virtual ~EapAuthIdentityAction() {}
};

/// State table used by EapAuthIdentityStateMachine.
class EapAuthIdentityStateTable_S : 
  public AAA_StateTable<EapAuthIdentityStateMachine>

{
  friend class ACE_Singleton<EapAuthIdentityStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:

  class AcCheckPickUpInit : public EapAuthIdentityAction
  {
    void operator()(EapAuthIdentityStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      if (ssm.IsEapBackend() && !ssm.NeedInitialRequestToSend())
	msm.Event(EvSgPickUpInit);
      else
	msm.Event(EvSgNoPickUpInit);
    }
  };

  class AcDoIntegrityCheck  : public EapAuthIdentityAction
  {
    void operator()(EapAuthIdentityStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthIdentityStateTable: Do Identity Check.\n");

      // Dequeue the message from the processing queue.
      AAAMessageBlock *msg = ssm.GetRxMessage();

      // Check the Identity.
      EapIdentity id;
      EapResponseIdentityParser parser;
      parser.setAppData(&id);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) { 
	if (msm.NumberOfTrial()++ < msm.maxTrial) {
	  EAP_LOG(LM_DEBUG, 
		  "AuthIdentityStateTable: Re-sending Request/Identity.\n");
	  msm.Event(EvSgFailure_MoreTrial);
	}
	else {
	  msm.Event(EvSgFailure_NoMoreTrial);
	}
	return;
      }

      ssm.PeerIdentity() = id.Identity();

      EapAuthIdentityStateMachine::ProcessIdentityResult result = 
	msm.ProcessIdentity(ssm.PeerIdentity());

      if (result==EapAuthIdentityStateMachine::Success) {
	msm.Event(EvSgSuccess);
	return;
      } 
      else if (msm.NumberOfTrial()++ < msm.maxTrial) {
	EAP_LOG(LM_DEBUG, 
		"AuthIdentityStateTable: Re-sending Request/Identity.\n");
	msm.Event(EvSgFailure_MoreTrial);
      }
      else {
	msm.Event(EvSgFailure_NoMoreTrial);
      }
    }
  };

  class AcNotifySuccess : public EapAuthIdentityAction
  {
    void operator()(EapAuthIdentityStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyFailure : public EapAuthIdentityAction
  {
    void operator()(EapAuthIdentityStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
    }
  };

  class AcPrepareRequest : public EapAuthIdentityAction
  {
    void operator()(EapAuthIdentityStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5);
      EapRequest req(EapType(1));  

      // Use parser to set Type field.
      EapRequestParser parser;
      parser.setAppData(&req);
      parser.setRawData(msg);
      parser.parseAppToRaw();

      ssm.SetTxMessage(msg);
    
      EAP_LOG(LM_DEBUG, "AuthIdentityStateTable: Request Prepared.\n");
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  enum {
    EvSgSuccess,
    EvSgFailure_MoreTrial,
    EvSgFailure_NoMoreTrial,
    EvSgPickUpInit,
    EvSgNoPickUpInit,
  };

  enum state {
    StInitialize, 
    StCheckPickUpInit,
    StWaitResponse, 
    StProcessResponse,
    StSuccess,
    StFailure
  };

  AcCheckPickUpInit acCheckPickUpInit;
  AcDoIntegrityCheck acDoIntegrityCheck;
  AcNotifySuccess acNotifySuccess;
  AcNotifyFailure acNotifyFailure;
  AcPrepareRequest acPrepareRequest;

  // Defined as a leaf class
  EapAuthIdentityStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StCheckPickUpInit, acCheckPickUpInit);
    AddStateTableEntry(StCheckPickUpInit, EvSgPickUpInit,
		       StProcessResponse, acDoIntegrityCheck);
    AddStateTableEntry(StCheckPickUpInit, EvSgNoPickUpInit,
		       StWaitResponse, acPrepareRequest);
    AddStateTableEntry(StWaitResponse, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessResponse, acDoIntegrityCheck);
    AddStateTableEntry(StProcessResponse, EvSgSuccess, 
		       StSuccess, acNotifySuccess);
    AddStateTableEntry(StProcessResponse, EvSgFailure_MoreTrial, 
		       StWaitResponse, acPrepareRequest);
    AddStateTableEntry(StProcessResponse, EvSgFailure_NoMoreTrial, 
		       StFailure, acNotifyFailure);
    AddWildcardStateTableEntry(StSuccess, StSuccess);
    AddWildcardStateTableEntry(StFailure, StFailure);
    InitialState(StInitialize);
  }
  ~EapAuthIdentityStateTable_S() {}

};

typedef ACE_Singleton<EapAuthIdentityStateTable_S, ACE_Recursive_Thread_Mutex> 
EapAuthIdentityStateTable;

const ACE_UINT32 EapAuthIdentityStateMachine::maxTrial = 3;

EapAuthIdentityStateMachine::EapAuthIdentityStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s), 
    EapStateMachine<EapAuthIdentityStateMachine>
  (*this, *EapAuthIdentityStateTable::instance(), s.Reactor(), s, "identity"),
    nTrial(0) 
{} 

