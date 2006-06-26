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
// $Id: eap_archie_fsm.cxx,v 1.15 2004/06/17 21:13:34 yohba Exp $

// EAP-Archie state machines.
// Written by Yoshihiro Ohba (yohba@tari.toshiba.com)

#include "eap_archie_fsm.hxx"
#include "eap_archie_parser.hxx"
#include "eap_archie_crypto.hxx"
#include <openssl/rand.h>

/// Action class.
class EapPeerArchieAction : public AAA_Action<EapPeerArchieStateMachine>
{
 public:
  virtual void operator()(EapPeerArchieStateMachine&) {}
 protected:
  EapPeerArchieAction() {}
  virtual ~EapPeerArchieAction() {}
};

/// Action class.
class EapAuthArchieAction : public AAA_Action<EapAuthArchieStateMachine>
{
 public:
  virtual void operator()(EapAuthArchieStateMachine&) {}
 protected:
  EapAuthArchieAction() {}
  virtual ~EapAuthArchieAction() {}
};

/// State table used by EapPeerArchieStateMachine.
class EAP_ARCHIE_EXPORTS EapPeerArchieStateTable_S : 
  public AAA_StateTable<EapPeerArchieStateMachine>
{
  friend class ACE_Singleton<EapPeerArchieStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:
  /// defined as a leaf class

  class AcDoIntegrityCheckForRequestMsg : public EapPeerArchieAction
  {
    void operator()(EapPeerArchieStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerArchie: Integrity check on request message.\n");

      // Check the Arhice-Request.
      EapRequestArchieRequest request;
      EapRequestArchieRequestParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Input shared secret.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      // Check if the shared secret validity.
      if (sharedSecret.size() != 64)
	{
	  EAP_LOG(LM_ERROR, "PeerArchie: Invalid shared secret length.\n");
	  msm.Event(EvSgInvalid);
	  return;
	}

      msm.AuthID()  = request.AuthID();
      msm.SessionID()  = request.SessionID();

      // Retain the message.
      msm.History().append(msg->base() + 4, 4 + 256);  // exclude SessionID

      msm.Event(EvSgValid);
    }
  };

  class AcBuildResponse : public EapPeerArchieAction
  {
    void operator()(EapPeerArchieStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerArchie: Building a response message.\n");

      AAAMessageBlock *msg = 
	    AAAMessageBlock::Acquire(8 + 32 + 256 + 40 + 516 + 12);

      ACE_OS::memset(msg->base(), 0, 8 + 32 + 256 + 40 + 516 + 12);

      EapResponseArchieResponse response;

      response.SessionID() = msm.SessionID();
      response.PeerID() = msm.PeerID() = msm.InputIdentity();

      // Compute nonceP.
      unsigned char nonceP[40];
      if (RAND_bytes(nonceP, 40) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthArchieStateTable: Failed to generate NonceP.\n");
	    return;
	  }
      response.NonceP() = msm.NonceP() 
	    = std::string((char*)nonceP, sizeof(nonceP));

      // Calculate Binding.  At this time, binding field is just a
      // place holder and never used.
      response.Binding() = msm.Binding();

      EapResponseArchieResponseParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Update the history.
      msm.History().append(msg->base() + 4, 4+32+256+40+516);

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      EapCryptoAES_CBC_MAC macCalculator;

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);

      // Calculate MAC1.
      macCalculator(msm.History(), response.Mac1(), KCK, 
		    EapCryptoAES_CBC_MAC::MAC_Length96);
      
      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
      
      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcDoIntegrityCheckForConfirmMsg : public EapPeerArchieAction
  {
    void operator()(EapPeerArchieStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerArchie: Identity checking on confirm message.\n");

      // Check the Arhice-Confirm.
      EapRequestArchieConfirm confirm;
      EapRequestArchieConfirmParser parser;
      parser.setAppData(&confirm);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Check session id.
      if (confirm.SessionID() != msm.SessionID())
	  {
	    EAP_LOG(LM_ERROR, "PeerArchie: Session ID mismatch.");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);

      // Obtain history of received message and construct MAC input.
      std::string input = msm.History();
      input.append(msg->base() + 4, 4+32+40+516);
      std::string mac2;

      // Compute MAC2 and do validity check.
      EapCryptoAES_CBC_MAC macCalculator;
      macCalculator(input, mac2, KCK, EapCryptoAES_CBC_MAC::MAC_Length96);
      if (mac2 != confirm.Mac2())
	{
	  EAP_LOG(LM_ERROR, "PeerArchie: Invalid MAC2.\n");
	  msm.Event(EvSgInvalid);
	  return;
	}

      // If validity check is successful, store the received fields
      // and update the history data.
      msm.NonceA()  = confirm.NonceA();
      msm.Binding()  = confirm.Binding();
      msm.History() = input;

      // Continue to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcNotifySuccess : public EapPeerArchieAction
  {
    void operator()(EapPeerArchieStateMachine &msm)
    {
      // prepare the Response/Archie-Finish
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EapResponseArchieFinish finish;

      finish.SessionID() = msm.SessionID();
      finish.Mac3() = std::string(12, ' ');

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 32 + 12);
    
      ACE_OS::memset(msg->base(), 0, 8 + 32 + 12);

      EapResponseArchieFinishParser parser;
      parser.setAppData(&finish);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Calculate MAC3.

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      EapCryptoAES_CBC_MAC macCalculator;

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);

      std::string input(msg->base()+4, 4+32);

      macCalculator(input, finish.Mac3(), KCK, 
		    EapCryptoAES_CBC_MAC::MAC_Length96);
      
      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Write the message again.
      try { parser.parseAppToRaw(); }
	  catch (...) {
	    EAP_LOG(LM_ERROR, "PeerArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
    
      ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
      EAP_LOG(LM_DEBUG, "PeerArchie: Archie-Finish prepared.\n");

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;

      msm.KeyData() = msm.MK();

      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcNotifyInvalid : public EapPeerArchieAction
  {
    void operator()(EapPeerArchieStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };

  enum {
    EvSgValid,
    EvSgInvalid,
  };
  enum state {
    StInitialize, 
    StProcessRequest, 
    StResponseSent, 
    StProcessConfirm, 
    StSuccess
  };

  AcDoIntegrityCheckForRequestMsg acDoIntegrityCheckForRequestMsg;
  AcDoIntegrityCheckForConfirmMsg acDoIntegrityCheckForConfirmMsg;
  AcBuildResponse acBuildResponse;
  AcNotifySuccess acNotifySuccess;
  AcNotifyInvalid acNotifyInvalid;

  EapPeerArchieStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessRequest, acDoIntegrityCheckForRequestMsg);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StProcessRequest, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessRequest, EvSgValid, 
		       StResponseSent, acBuildResponse);

    AddStateTableEntry(StResponseSent, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessConfirm, acDoIntegrityCheckForConfirmMsg);
    AddWildcardStateTableEntry(StResponseSent, StResponseSent);

    AddStateTableEntry(StProcessConfirm, EvSgInvalid,
		       StResponseSent, acNotifyInvalid);
    AddStateTableEntry(StProcessConfirm, EvSgValid, 
		       StSuccess, acNotifySuccess);

    AddWildcardStateTableEntry(StSuccess, StSuccess);

    InitialState(StInitialize);
  } // leaf class
  ~EapPeerArchieStateTable_S() {}
};

/// State table used by EapAuthArchieStateMachine.
class EAP_ARCHIE_EXPORTS EapAuthArchieStateTable_S : 
  public AAA_StateTable<EapAuthArchieStateMachine>

{
  friend class ACE_Singleton<EapAuthArchieStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:

  class AcBuildRequest : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthArchie: Building a request message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 256 + 32);
    
      ACE_OS::memset(msg->base(), 0, 8 + 256 + 32);

      EapRequestArchieRequest request;

      unsigned char sessionID[32];

      // Generate a session id.
      if (RAND_bytes(sessionID, sizeof sessionID) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthArchie: Failed to generate a session id.\n");
	    return;
	  }

      request.SessionID() = msm.SessionID() 
	    = std::string((char*)sessionID, sizeof(sessionID));

      // Get an AuthID.
      request.AuthID() = msm.AuthID() = msm.InputIdentity();

      EapRequestArchieRequestParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      msm.History().append(msg->base() + 4, 4+256);

      // Set the message to the session.
      ssm.SetTxMessage(msg);
      
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcDoIntegrityCheckForResponseMsg : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "AuthArchie: Integrity check on response message.\n");

      // Check the Arhice-Request.
      EapResponseArchieResponse response;
      EapResponseArchieResponseParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
	    EAP_LOG(LM_ERROR, "AuthArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
      }	

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      // Check if the shared secret validity.
      if (sharedSecret.size() != 64)
	  {
	    EAP_LOG(LM_ERROR, "AuthArchie: Invalid shared secret length.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Compute MAC1 and do validity check.

      EapCryptoAES_CBC_MAC macCalculator;

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);
      std::string input = msm.History();
      input.append(msg->base() + 4, 4+32+256+40+516);
      std::string mac1;

      macCalculator(input, mac1, KCK, EapCryptoAES_CBC_MAC::MAC_Length96);

      if (mac1 != response.Mac1())
	  {
	    EAP_LOG(LM_ERROR, "AuthArchie: Invalid MAC1.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // If validity check is successful, store the received fields
      // and update the history data.

      msm.SessionID() = response.SessionID();
      msm.PeerID() = response.PeerID();
      msm.NonceP() = response.NonceP();
      msm.Binding() = response.Binding();

      msm.History() = input;

      msm.Event(EvSgValid);
    }
  };

  class AcBuildConfirm : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthArchie: Building a confirm message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 32 + 40 + 516 + 12);

      ACE_OS::memset(msg->base(), 0, 8 + 32 + 40 + 516 + 12);

    
      // Compute nonceA.
      unsigned char nonceA[40];
      if (RAND_bytes(nonceA, 40) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthArchieStateTable: Failed to generate NonceA.\n");
	    return;
	  }
      msm.NonceA() = std::string((char*)nonceA, sizeof(nonceA));

      EapRequestArchieConfirm confirm;

      confirm.SessionID() = msm.SessionID();
      confirm.NonceA() = msm.NonceA();
      confirm.Binding() = msm.Binding();

      // Construct a message with dummy MAC2.
      EapRequestArchieConfirmParser parser;
      parser.setAppData(&confirm);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Update the history.
      msm.History().append(msg->base() + 4, 4+32+40+516);

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);

      // Calculate MAC2.
      EapCryptoAES_CBC_MAC macCalculator;
      macCalculator(msm.History(), confirm.Mac2(), KCK, 
		    EapCryptoAES_CBC_MAC::MAC_Length96);
      
      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Write the message again.
      try { parser.parseAppToRaw(); }
	  catch (...) {
	    EAP_LOG(LM_ERROR, "AuthArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
      
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcDoIntegrityCheckForFinishMsg : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "AuthArchie: Do Identity Check on confirm message.\n");

      // Check the Arhice-Confirm.
      EapResponseArchieFinish finish;
      EapResponseArchieFinishParser parser;
      parser.setAppData(&finish);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthArchie: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Check session id.
      if (finish.SessionID() != msm.SessionID())
	  {
	    EAP_LOG(LM_ERROR, "AuthArchie: Session ID mismatch.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Obtain Key Confirmation Key from the shared secret.
      std::string KCK(sharedSecret.data(), 16);

      // Obtain the history of received messsages and construct MAC input.
      std::string input(msg->base() + 4, 4+32);

      // Compute MAC3 and do validity check.
      EapCryptoAES_CBC_MAC macCalculator;
      std::string mac3;
      macCalculator(input, mac3, KCK, EapCryptoAES_CBC_MAC::MAC_Length96);
      if (mac3 != finish.Mac3())
	{
	  EAP_LOG(LM_ERROR, "AuthArchie: Invalid MAC3.\n");
	  msm.Event(EvSgInvalid);
	  return;
	}
      
      // Proceed to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcNotifySuccess : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      msm.KeyData() = msm.MK();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyFailure : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyInvalid : public EapAuthArchieAction
  {
    void operator()(EapAuthArchieStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
    }
  };

  enum {
    EvSgValid,
    EvSgInvalid,
  };
  enum state {
    StInitialize, 
    StRequestSent, 
    StProcessResponse,
    StConfirmSent, 
    StProcessFinish,
    StSuccess
  };

  AcDoIntegrityCheckForResponseMsg acDoIntegrityCheckForResponseMsg;
  AcDoIntegrityCheckForFinishMsg acDoIntegrityCheckForFinishMsg;
  AcBuildRequest acBuildRequest;
  AcBuildConfirm acBuildConfirm;
  AcNotifySuccess acNotifySuccess;
  AcNotifyFailure acNotifyFailure;
  AcNotifyInvalid acNotifyInvalid;

  EapAuthArchieStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StRequestSent, acBuildRequest);
    AddStateTableEntry(StInitialize, StInitialize, 0);

    AddStateTableEntry(StRequestSent, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessResponse, acDoIntegrityCheckForResponseMsg);
    AddStateTableEntry(StRequestSent, StRequestSent, 0);

    AddStateTableEntry(StProcessResponse, EvSgInvalid,
		       StInitialize, acNotifyFailure);
    AddStateTableEntry(StProcessResponse, EvSgValid, 
		       StConfirmSent, acBuildConfirm);

    AddStateTableEntry(StConfirmSent, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessFinish, acDoIntegrityCheckForFinishMsg);
    AddStateTableEntry(StConfirmSent, StConfirmSent, 0);

    AddStateTableEntry(StProcessFinish, EvSgInvalid,
		       StConfirmSent, acNotifyFailure);
    AddStateTableEntry(StProcessFinish, EvSgValid, 
		       StSuccess, acNotifySuccess);

    AddStateTableEntry(StSuccess, StSuccess, 0);

    InitialState(StInitialize);
  } // leaf class
  ~EapAuthArchieStateTable_S() {}
};

typedef ACE_Singleton<EapPeerArchieStateTable_S, ACE_Recursive_Thread_Mutex>
EapPeerArchieStateTable;

typedef ACE_Singleton<EapAuthArchieStateTable_S, ACE_Recursive_Thread_Mutex> 
EapAuthArchieStateTable;

EapPeerArchieStateMachine::EapPeerArchieStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapPeerArchieStateMachine>
  (*this, *EapPeerArchieStateTable::instance(), s.Reactor(), s, "Archie(peer)")
{
  history.assign("");
} 

EapAuthArchieStateMachine::EapAuthArchieStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthArchieStateMachine>
  (*this, *EapAuthArchieStateTable::instance(), 
   s.Reactor(), s, "Archie(authenticator)")
{
  history.assign("");
} 
