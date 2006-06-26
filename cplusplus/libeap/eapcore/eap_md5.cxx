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
// $Id: eap_md5.cxx,v 1.26 2004/06/17 21:13:35 yohba Exp $

// eap_md5.cxx:  EAP MD5 authentication method state machine
// Written by Yoshihiro Ohba

#include <string>
#include <ace/Singleton.h>
#include <ace/OS_String.h>
#include <ace/Message_Block.h>
#include <ace/Thread.h>
#include <openssl/rand.h>
#include "eap.hxx"
#include "eap_identity.hxx"
#include "eap_method_registrar.hxx"
#include "eap_parser.hxx"
#include "eap_log.hxx"
#include "eap_md5.hxx"

/// Action class.
class EapPeerMD5Action : public AAA_Action<EapPeerMD5ChallengeStateMachine>
{
 public:
  virtual void operator()(EapPeerMD5ChallengeStateMachine&) {}
 protected:
  EapPeerMD5Action() {}
  virtual ~EapPeerMD5Action() {}
};

/// Action class.
class EapAuthMD5Action : public AAA_Action<EapAuthMD5ChallengeStateMachine>
{
 public:
  virtual void operator()(EapAuthMD5ChallengeStateMachine&) {}
 protected:
  EapAuthMD5Action() {}
  virtual ~EapAuthMD5Action() {}
};

/// State table used by EapPeerMD5ChallengeStateMachine.
class EapPeerMD5ChallengeStateTable_S : 
  public AAA_StateTable<EapPeerMD5ChallengeStateMachine>

{
  friend class ACE_Singleton<EapPeerMD5ChallengeStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;

private:
  /// defined as a leaf class

  class AcDoIntegrityCheck : public EapPeerMD5Action
  {
    void operator()(EapPeerMD5ChallengeStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerMDChallenge: Do Identity Check.\n");

      // Check the MD5-Challange.
      EapMD5Challenge md5Challenge;
      EapRequestMD5ChallengeParser parser;
      parser.setAppData(&md5Challenge);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...)
	{
	  EAP_LOG(LM_ERROR, "EapPeerMD5Challenge: Parse error.\n");
	  msm.Event(EvSgReject);
	  return;
	}

      msm.InputPassphrase();
      std::string& passphrase = msm.Passphrase();

      // Check if the passphrase is empty.
      if (passphrase.empty())
	{
	  EAP_LOG(LM_ERROR, "EapPeerMD5Challenge: Passphrase is not set.\n");
	  msm.Event(EvSgReject);
	  return;
	}

      std::string& challenge = md5Challenge.Value();
      std::string& name = md5Challenge.Name();  

      // Set the received parameters to the state machine.
      msm.PeerName() = name;

      // Begin of response computation.

      // Get the Identifier.

      ACE_Byte id = ((EapPeerSwitchStateMachine&)ssm).CurrentIdentifier();
      EAP_LOG(LM_DEBUG, "EapPeerMD5Challenge: Identifier = %d.\n", id);

      // Compose a raw response concatinating Identifier, passphrase
      // (i.e., a shared secret) and the challenge value.
      std::string rawResponse((const char*)&id, sizeof(id));
      rawResponse.append(passphrase);
      rawResponse.append(challenge);

      // Initialize the response.
      std::string md5Response(MD5_DIGEST_LENGTH, '\0');

      // Do MD5.
      MD5((const unsigned char*)rawResponse.data(), 
	  rawResponse.size(), (unsigned char*)md5Response.data());

      // End of response computation.

      // Set the computed response.
      msm.Value() = md5Response;
                                                                                
      msm.Event(EvSgAccept);
    }
  };

  class AcNotifySuccess : public EapPeerMD5Action
  {
    void operator()(EapPeerMD5ChallengeStateMachine &msm)
    {
      // prepare the Response
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EapMD5Challenge md5Challenge;

      md5Challenge.Name() = msm.MyName();
      md5Challenge.Value() = msm.Value();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire
	(5 + md5Challenge.Value().size() + 1 + md5Challenge.Name().size());
    
      EapResponseMD5ChallengeParser parser;
      parser.setAppData(&md5Challenge);
      parser.setRawData(msg);
      parser.parseAppToRaw();

      // Set the message to the session.
      ssm.SetTxMessage(msg);
    
      ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
      EAP_LOG(LM_DEBUG, "PeerMD5ChallengeStateTable: Response Prepared.\n");
      msm.IsDone() = true;
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcNotifyInvalid : public EapPeerMD5Action
  {
    void operator()(EapPeerMD5ChallengeStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      msm.IsDone() = true;
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };

  enum {
    EvSgAccept,
    EvSgReject,
  };
  enum state {
    StInitialize, 
    StProcessRequest, 
    StSuccess
  };

  AcDoIntegrityCheck acDoIntegrityCheck;
  AcNotifySuccess acNotifySuccess;
  AcNotifyInvalid acNotifyInvalid;

  EapPeerMD5ChallengeStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessRequest, acDoIntegrityCheck);
    AddStateTableEntry(StProcessRequest, EvSgReject,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessRequest, EvSgAccept, 
		       StSuccess, acNotifySuccess);
    AddWildcardStateTableEntry(StSuccess, StSuccess);

    InitialState(StInitialize);
  } // leaf class
  ~EapPeerMD5ChallengeStateTable_S() {}
};

/// State table used by EapAuthMD5ChallengeStateMachine.
class EapAuthMD5ChallengeStateTable_S : 
  public AAA_StateTable<EapAuthMD5ChallengeStateMachine>
{
  friend class ACE_Singleton<EapAuthMD5ChallengeStateTable_S, 
			     ACE_Recursive_Thread_Mutex>;
private:
  // Defined as a leaf class

  class AcDoIntegrityCheck : public EapAuthMD5Action
  {
    void operator()(EapAuthMD5ChallengeStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      EAP_LOG(LM_DEBUG, "AuthMD5ChallengeStateTable: Do Identity Check.\n");

      // Dequeue the message from the processing queue.
      AAAMessageBlock *msg = ssm.GetRxMessage();

      // Check the MD5-Challenge.
      EapMD5Challenge md5Challenge;
      EapResponseMD5ChallengeParser parser;
      parser.setAppData(&md5Challenge);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) { 
	EAP_LOG(LM_DEBUG, "AuthIdentity: Re-sending Request/Identity.\n");
	msm.Event(EvSgFailure);
	return;
      }

      // Set the received parameter to the state machine.
      msm.PeerName() = md5Challenge.Name();

      msm.InputPassphrase();
      std::string& passphrase = msm.Passphrase();

      // Check if the passphrase is empty.
      if (passphrase.empty())
	{
	  EAP_LOG(LM_ERROR, "EapAuthMD5Challenge: Passphrase is not set.\n");
	  msm.Event(EvSgFailure);
	  return;
	}

      std::string& challenge = msm.Value();

      // Begin of response computation.

      // Get the Identifier.
      ACE_Byte id = ssm.CurrentIdentifier();
      EAP_LOG(LM_DEBUG, "EapAuthMD5Challenge: Identifier = %d.\n", id);

      // Compose a raw response concatinating Identifier, passphrase
      // (i.e., a shared secret) and the challenge value.
      std::string rawResponse((const char*)&id, sizeof(id));
      rawResponse.append(passphrase);
      rawResponse.append(challenge);

      // Initialize the response.
      std::string md5Response(MD5_DIGEST_LENGTH, '\0');

      // Do MD5.
      MD5((const unsigned char*)rawResponse.data(), 
	  rawResponse.size(), (unsigned char*)md5Response.data());

      // End of response computation.

      // Check the computed value with the expected value.

      if (md5Response == md5Challenge.Value())
	{
	  msm.Event(EvSgSuccess);
	  return;
	} 
      else {
	EAP_LOG(LM_DEBUG, "AuthMD5ChallengeStateTable: Invalid response.\n");
	msm.Event(EvSgFailure);
      }
    }
  };

  class AcNotifySuccess : public EapAuthMD5Action
  {
    void operator()(EapAuthMD5ChallengeStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyFailure : public EapAuthMD5Action
  {
    void operator()(EapAuthMD5ChallengeStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcPrepareRequest : public EapAuthMD5Action
  {
    void operator()(EapAuthMD5ChallengeStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      std::string md5ChallengeString(MD5_DIGEST_LENGTH, '\0');

      // Calculate the challenge.
      if (RAND_bytes((unsigned char*)md5ChallengeString.data(), 
		     MD5_DIGEST_LENGTH) == 0)
	{
	  EAP_LOG(LM_ERROR, 
		  "AuthMD5Challenge: Failed to calculate a challenge.\n");
	  return;
	}

      EapMD5Challenge md5Challenge;
      msm.Value() = md5ChallengeString;

      md5Challenge.Value() = md5ChallengeString;
      md5Challenge.Name() = msm.MyName();
      
      AAAMessageBlock *msg = AAAMessageBlock::Acquire
	(5 + md5Challenge.Value().size() + 1 + md5Challenge.Name().size());

      // Use parser to set Type field.
      EapRequestMD5ChallengeParser parser;
      parser.setAppData(&md5Challenge);
      parser.setRawData(msg);
      parser.parseAppToRaw();

      ssm.SetTxMessage(msg);
    
      EAP_LOG(LM_DEBUG, "AuthMD5ChallengeST: Request Prepared.\n");
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  enum {
    EvSgSuccess,
    EvSgFailure
  };

  AcDoIntegrityCheck acDoIntegrityCheck;
  AcNotifySuccess acNotifySuccess;
  AcNotifyFailure acNotifyFailure;
  AcPrepareRequest acPrepareRequest;

  enum state {
    StInitialize, 
    StWaitResponse, 
    StProcessResponse,
    StSuccess,
    StFailure
  };

  EapAuthMD5ChallengeStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StWaitResponse, acPrepareRequest);
    AddStateTableEntry(StWaitResponse, 
		       EapMethodStateMachine::EvSgIntegrityCheck, 
		       StProcessResponse, acDoIntegrityCheck);
    AddStateTableEntry(StProcessResponse, EvSgSuccess, 
		       StSuccess, acNotifySuccess);
    AddStateTableEntry(StProcessResponse, EvSgFailure,
		       StFailure, acNotifyFailure);
    AddWildcardStateTableEntry(StSuccess, StSuccess);
    AddWildcardStateTableEntry(StFailure, StFailure);
    InitialState(StInitialize);
  }
  ~EapAuthMD5ChallengeStateTable_S() {}
};

typedef ACE_Singleton<EapPeerMD5ChallengeStateTable_S, 
		      ACE_Recursive_Thread_Mutex>
EapPeerMD5ChallengeStateTable;

typedef ACE_Singleton<EapAuthMD5ChallengeStateTable_S, 
		      ACE_Recursive_Thread_Mutex> 
EapAuthMD5ChallengeStateTable;

EapPeerMD5ChallengeStateMachine::EapPeerMD5ChallengeStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapPeerMD5ChallengeStateMachine>
  (*this, *EapPeerMD5ChallengeStateTable::instance(), s.Reactor(), 
   s, "MD5(peer)")
{} 

EapAuthMD5ChallengeStateMachine::EapAuthMD5ChallengeStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthMD5ChallengeStateMachine>
  (*this, *EapAuthMD5ChallengeStateTable::instance(), 
   s.Reactor(), s, "MD5(authenticator)")
{} 

