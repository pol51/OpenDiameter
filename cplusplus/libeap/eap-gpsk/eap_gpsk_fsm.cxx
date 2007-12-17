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

#include "eap_gpsk_fsm.hxx"
#include "eap_gpsk_parser.hxx"
#include "eap_gpsk_crypto.hxx"
#include <openssl/rand.h>

/// Action class.
class EapPeerGpskAction : public AAA_Action<EapPeerGpskStateMachine>
{
 public:
  virtual void operator()(EapPeerGpskStateMachine&) {}
 protected:
  EapPeerGpskAction() {}
  virtual ~EapPeerGpskAction() {}
};

/// Action class.
class EapAuthGpskAction : public AAA_Action<EapAuthGpskStateMachine>
{
 public:
  virtual void operator()(EapAuthGpskStateMachine&) {}
 protected:
  EapAuthGpskAction() {}
  virtual ~EapAuthGpskAction() {}
};

/// State table used by EapPeerGpskStateMachine.
class EAP_ARCHIE_EXPORTS EapPeerGpskStateTable_S :
  public AAA_StateTable<EapPeerGpskStateMachine>
{
  friend class ACE_Singleton<EapPeerGpskStateTable_S,
			     ACE_Recursive_Thread_Mutex>;

private:
  /// defined as a leaf class

  class AcDoIntegrityCheckForGpsk1Msg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: Integrity check on GPSK1 message.\n");

      // Check the GPSK1-Request.
      EapRequestGpsk1 request;
      EapRequestGpsk1Parser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
         EAP_LOG(LM_ERROR, "PeerGpsk: Parse error on GPSK1.\n");
         msm.Event(EvSgInvalid);
         return;
      }

      // Input shared secret.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      msm.ServerID()  = request.IDServer();
      msm.ServerRAND()  = request.RANDServer();
      msm.CipherSuiteList() = request.CSuiteList();

      msm.Event(EvSgValid);
    }
  };

  class AcNotifyInvalid : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };










  class AcDoIntegrityCheckForRequestMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: Integrity check on request message.\n");

      // Check the Arhice-Request.
      EapRequestGpskRequest request;
      EapRequestGpskRequestParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Input shared secret.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      // Check if the shared secret validity.
      if (sharedSecret.size() != 64)
	{
	  EAP_LOG(LM_ERROR, "PeerGpsk: Invalid shared secret length.\n");
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

  class AcBuildResponse : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerGpsk: Building a response message.\n");

      AAAMessageBlock *msg = 
	    AAAMessageBlock::Acquire(8 + 32 + 256 + 40 + 516 + 12);

      ACE_OS::memset(msg->base(), 0, 8 + 32 + 256 + 40 + 516 + 12);

      EapResponseGpskResponse response;

      response.SessionID() = msm.SessionID();
      response.PeerID() = msm.PeerID() = msm.InputIdentity();

      // Compute nonceP.
      unsigned char nonceP[40];
      if (RAND_bytes(nonceP, 40) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthGpskStateTable: Failed to generate NonceP.\n");
	    return;
	  }
      response.NonceP() = msm.NonceP() 
	    = std::string((char*)nonceP, sizeof(nonceP));

      // Calculate Binding.  At this time, binding field is just a
      // place holder and never used.
      response.Binding() = msm.Binding();

      EapResponseGpskResponseParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
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
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
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

  class AcDoIntegrityCheckForConfirmMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: Identity checking on confirm message.\n");

      // Check the Arhice-Confirm.
      EapRequestGpskConfirm confirm;
      EapRequestGpskConfirmParser parser;
      parser.setAppData(&confirm);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Check session id.
      if (confirm.SessionID() != msm.SessionID())
	  {
	    EAP_LOG(LM_ERROR, "PeerGpsk: Session ID mismatch.");
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
	  EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2.\n");
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

  class AcNotifySuccess : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      // prepare the Response/Gpsk-Finish
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EapResponseGpskFinish finish;

      finish.SessionID() = msm.SessionID();
      finish.Mac3() = std::string(12, ' ');

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 32 + 12);
    
      ACE_OS::memset(msg->base(), 0, 8 + 32 + 12);

      EapResponseGpskFinishParser parser;
      parser.setAppData(&finish);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
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
	    EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
    
      ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
      EAP_LOG(LM_DEBUG, "PeerGpsk: Gpsk-Finish prepared.\n");

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;

      msm.KeyData() = msm.MK();

      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };














  enum {
    EvSgValid,
    EvSgInvalid,
  };
  enum state {
    StInitialize, 
    StProcessGpsk1, 
    StResponseSent, 
    StProcessConfirm, 
    StSuccess
  };

  AcDoIntegrityCheckForGpsk1Msg acDoIntegrityCheckForGpks1Msg;
  AcDoIntegrityCheckForConfirmMsg acDoIntegrityCheckForConfirmMsg;
  AcBuildResponse acBuildResponse;
  AcNotifySuccess acNotifySuccess;
  AcNotifyInvalid acNotifyInvalid;

  EapPeerGpskStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessGpsk1, acDoIntegrityCheckForGpks1Msg);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StProcessGpsk1, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessGpsk1, EvSgValid, 
		       StResponseSent, acBuildGpsk2);

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
  ~EapPeerGpskStateTable_S() {}
};

/// State table used by EapAuthGpskStateMachine.
class EAP_ARCHIE_EXPORTS EapAuthGpskStateTable_S : 
  public AAA_StateTable<EapAuthGpskStateMachine>

{
  friend class ACE_Singleton<EapAuthGpskStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:

  class AcBuildRequest : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Building a request message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 256 + 32);
    
      ACE_OS::memset(msg->base(), 0, 8 + 256 + 32);

      EapRequestGpskRequest request;

      unsigned char sessionID[32];

      // Generate a session id.
      if (RAND_bytes(sessionID, sizeof sessionID) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthGpsk: Failed to generate a session id.\n");
	    return;
	  }

      request.SessionID() = msm.SessionID() 
	    = std::string((char*)sessionID, sizeof(sessionID));

      // Get an AuthID.
      request.AuthID() = msm.AuthID() = msm.InputIdentity();

      EapRequestGpskRequestParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Parse error.\n");
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

  class AcDoIntegrityCheckForResponseMsg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Integrity check on response message.\n");

      // Check the Arhice-Request.
      EapResponseGpskResponse response;
      EapResponseGpskResponseParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
      }	

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      // Check if the shared secret validity.
      if (sharedSecret.size() != 64)
	  {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Invalid shared secret length.\n");
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
	    EAP_LOG(LM_ERROR, "AuthGpsk: Invalid MAC1.\n");
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

  class AcBuildConfirm : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Building a confirm message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(8 + 32 + 40 + 516 + 12);

      ACE_OS::memset(msg->base(), 0, 8 + 32 + 40 + 516 + 12);

    
      // Compute nonceA.
      unsigned char nonceA[40];
      if (RAND_bytes(nonceA, 40) == 0)
	  {
	    EAP_LOG(LM_ERROR, 
		  "AuthGpskStateTable: Failed to generate NonceA.\n");
	    return;
	  }
      msm.NonceA() = std::string((char*)nonceA, sizeof(nonceA));

      EapRequestGpskConfirm confirm;

      confirm.SessionID() = msm.SessionID();
      confirm.NonceA() = msm.NonceA();
      confirm.Binding() = msm.Binding();

      // Construct a message with dummy MAC2.
      EapRequestGpskConfirmParser parser;
      parser.setAppData(&confirm);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Parse error.\n");
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
	    EAP_LOG(LM_ERROR, "AuthGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
      
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcDoIntegrityCheckForFinishMsg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Do Identity Check on confirm message.\n");

      // Check the Arhice-Confirm.
      EapResponseGpskFinish finish;
      EapResponseGpskFinishParser parser;
      parser.setAppData(&finish);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Check session id.
      if (finish.SessionID() != msm.SessionID())
	  {
	    EAP_LOG(LM_ERROR, "AuthGpsk: Session ID mismatch.\n");
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
	  EAP_LOG(LM_ERROR, "AuthGpsk: Invalid MAC3.\n");
	  msm.Event(EvSgInvalid);
	  return;
	}
      
      // Proceed to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcNotifySuccess : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      msm.KeyData() = msm.MK();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyFailure : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyInvalid : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
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

  EapAuthGpskStateTable_S() 
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
  ~EapAuthGpskStateTable_S() {}
};

typedef ACE_Singleton<EapPeerGpskStateTable_S, ACE_Recursive_Thread_Mutex>
EapPeerGpskStateTable;

typedef ACE_Singleton<EapAuthGpskStateTable_S, ACE_Recursive_Thread_Mutex> 
EapAuthGpskStateTable;

EapPeerGpskStateMachine::EapPeerGpskStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapPeerGpskStateMachine>
  (*this, *EapPeerGpskStateTable::instance(), s.Reactor(), s, "Gpsk(peer)")
{
  history.assign("");
} 

EapAuthGpskStateMachine::EapAuthGpskStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthGpskStateMachine>
  (*this, *EapAuthGpskStateTable::instance(), 
   s.Reactor(), s, "Gpsk(authenticator)")
{
  history.assign("");
} 
