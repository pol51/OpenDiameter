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
class EAP_GPSK_EXPORTS EapPeerGpskStateTable_S :
  public AAA_StateTable<EapPeerGpskStateMachine>
{
  friend class ACE_Singleton<EapPeerGpskStateTable_S,
			     ACE_Recursive_Thread_Mutex>;
private:
  /// defined as a leaf class

  class ReplayGpskFail
  {
  public:
    void operator()(EapPeerGpskStateMachine &msm, EapGpskFail &gpsk)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      EAP_LOG(LM_DEBUG, "PeerGpsk: Replay GPSK-Fail message.\n");

      AAAMessageBlock *reply = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(reply->base(), 0, GPSK_MAX_PKT_SIZE);

      // Replay the failure
      EapGpskFailParser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(reply);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // resize buffer
      reply->size(reply->length());

      // Set the message to the session.
      ssm.SetTxMessage(reply);

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);

      // Continue to the next step.
      msm.Event(EvSgFail);
    }
  };

  class ReplayGpskProtectedFail
  {
  public:
    void operator()(EapPeerGpskStateMachine &msm, EapGpskProtectedFail &gpsk)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      EAP_LOG(LM_DEBUG, "PeerGpsk: Replaying GPSK-Protected-Fail message.\n");

      // Save existing MAC
      std::string mac1 = gpsk.MAC();
      gpsk.MAC().resize(0);

      AAAMessageBlock *reply = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(reply->base(), 0, GPSK_MAX_PKT_SIZE);

      // Re-generate the MAC
      EapGpskProtectedFailParser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(reply);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating Protected-Fail for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(reply->base() + 4 + 2, reply->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2 on GPSK-Protected-Fail.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Replay the failure
      gpsk.MAC() = mac2;
      reply->wr_ptr(reply->base());

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // resize buffer
      reply->size(reply->length());

      // Set the message to the session.
      ssm.SetTxMessage(reply);

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);

      // Continue to the next step.
      msm.Event(EvSgFail);
    }
  };

  class AcDoIntegrityCheckForGpsk1Msg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "PeerGpsk: Integrity check on GPSK1 message.\n");

      // Check the GPSK1-Request.
      EapGpsk1 gpsk;
      EapGpsk1Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
         EAP_LOG(LM_ERROR, "PeerGpsk: Parse error on GPSK1.\n");
         msm.Event(EvSgInvalid);
         return;
      }

      msm.ServerID()  = gpsk.IDServer();
      msm.ServerRAND()  = gpsk.RANDServer();
      msm.CipherSuiteList() = gpsk.CSuiteList();

      msm.Event(EvSgValid);
    }
  };

  class AcBuildGpsk2 : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      EAP_LOG(LM_DEBUG, "PeerGpsk: Building a GPSK2 message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      EapGpsk2 gpsk;
      gpsk.IDPeer() = msm.PeerID() = msm.InputIdentity();
      gpsk.IDServer() = msm.ServerID();
      gpsk.RANDServer() = msm.ServerRAND();
      gpsk.CSuiteList() = msm.CipherSuiteList();
      gpsk.CSuiteSelected() = msm.CipherSuite();

      // Compute RAND_Peer.
      unsigned char pRAND[32];
      if (RAND_bytes(pRAND, 32) == 0)
        {
          EAP_LOG(LM_ERROR, 
             "AuthGpskStateTable: Failed to generate peer RAND.\n");
          msm.Event(EvSgInvalid);
          return;
        }
      gpsk.RANDPeer() = msm.PeerRAND()
         = std::string((char*)pRAND, sizeof(pRAND));

      EapGpsk2Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK2.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret() = msm.InputSharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4 + 2, msg->length() - 4 - 2);

      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), gpsk.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK2.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcDoIntegrityCheckForGpsk3Msg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "PeerGpsk: Integrity check on GPSK3 message.\n");

      // Check the GPSK3.
      EapGpsk3 gpsk;
      EapGpsk3Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {

        EAP_LOG(LM_ERROR, "PeerGpsk: Msg is not GPSK3 ... checking for GPSK-Fail.\n");

        // Check the GPSK-Fail.
        EapGpskFail fgpsk;
        EapGpskFailParser fparser;
        fparser.setAppData(&fgpsk);
        fparser.setRawData(msg);
        try {
           fparser.parseRawToApp();
           EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Fail received [%d].\n", fgpsk.FailureCode());
           ReplayGpskFail replay;
           replay(msm, fgpsk);
        }
        catch (...) {

          EAP_LOG(LM_ERROR, "PeerGpsk: Msg is not GPSK-Fail ... checking for GPSK-Protected-Fail.\n");

          // Check the GPSK-Fail.
          EapGpskProtectedFail pfgpsk;
          EapGpskProtectedFailParser pfparser;
          pfparser.setAppData(&pfgpsk);
          pfparser.setRawData(msg);
          try {
             pfparser.parseRawToApp();
             EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail received [%d].\n", pfgpsk.FailureCode());
             ReplayGpskProtectedFail replay;
             replay(msm, pfgpsk);
          }
          catch (...) {

             EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail Parse error.\n");
             msm.Event(EvSgInvalid);
          }
        }
        return;
      }

      // Check RAND values
      if (gpsk.RANDPeer() != msm.PeerRAND()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid peer RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }
      if (gpsk.RANDServer() != msm.ServerRAND()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid server RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Check selection
      if (msm.CipherSuite() != gpsk.CSuiteSelected()) {
         EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid selected CSuite value.\n");
         msm.Event(EvSgInvalid);
         return;
      }

      // Save the payload, TBD: Need to do something about the payload in the future
      msm.Payload() = gpsk.PDPayload();

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Save existing MAC
      std::string mac1 = gpsk.MAC();
      gpsk.MAC().resize(0);

      // Re-generate the MAC
      AAAMessageBlock *raw = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(raw->base(), 0, GPSK_MAX_PKT_SIZE);
      parser.setAppData(&gpsk);
      parser.setRawData(raw);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK3 for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(raw->base() + 4 + 2, raw->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);
      raw->Release();

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Continue to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcBuildGpsk4 : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      // prepare the Response/Gpsk-Finish
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      EapGpsk4 gpsk;
      EapGpsk4Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "PeerGpsk: Parse error for GPSK4.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4 + 2, msg->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), gpsk.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Write the message again.
      try { parser.parseAppToRaw(); }
        catch (...) {
          EAP_LOG(LM_ERROR, "PeerGpsk: Parse error.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
      EAP_LOG(LM_DEBUG, "PeerGpsk: Gpsk4 prepared.\n");

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;

      msm.KeyData() = msm.MK();

      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcNotifyFailure : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
      ssm.Decision() = EapPeerSwitchStateMachine::FAIL;
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

  enum {
    EvSgValid,
    EvSgFail,
    EvSgInvalid,
  };
  enum state {
    StInitialize,
    StProcessGpsk1,
    StGpsk2Sent,
    StProcessGpsk3,
    StSuccess
  };

  AcBuildGpsk2 acBuildGpsk2;
  AcBuildGpsk4 acBuildGpsk4;
  AcNotifyInvalid acNotifyInvalid;
  AcNotifyFailure acNotifyFailure;
  AcDoIntegrityCheckForGpsk1Msg acDoIntegrityCheckForGpsk1Msg;
  AcDoIntegrityCheckForGpsk3Msg acDoIntegrityCheckForGpsk3Msg;

  EapPeerGpskStateTable_S()
  {
    AddStateTableEntry(StInitialize,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessGpsk1, acDoIntegrityCheckForGpsk1Msg);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StProcessGpsk1, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessGpsk1, EvSgValid,
		       StGpsk2Sent, acBuildGpsk2);

    AddStateTableEntry(StGpsk2Sent,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessGpsk3, acDoIntegrityCheckForGpsk3Msg);
    AddWildcardStateTableEntry(StGpsk2Sent, StGpsk2Sent);

    AddStateTableEntry(StProcessGpsk3, EvSgInvalid,
		       StGpsk2Sent, acNotifyInvalid);
    AddStateTableEntry(StProcessGpsk3, EvSgFail,
		       StInitialize, acNotifyFailure);
    AddStateTableEntry(StProcessGpsk3, EvSgValid,
		       StSuccess, acBuildGpsk4);

    AddWildcardStateTableEntry(StSuccess, StSuccess);

    InitialState(StInitialize);
  } // leaf class
  ~EapPeerGpskStateTable_S() {}
};

/// State table used by EapAuthGpskStateMachine.
class EAP_GPSK_EXPORTS EapAuthGpskStateTable_S :
  public AAA_StateTable<EapAuthGpskStateMachine>

{
  friend class ACE_Singleton<EapAuthGpskStateTable_S,
			     ACE_Recursive_Thread_Mutex>;

private:

  class AcDoIntegrityCheckForGpskFailMsg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "AuthGpsk: Integrity check on GPSK-Fail message.\n");

      // Check the GPSK-Fail.
      EapGpskFail gpsk1;
      EapGpskFailParser parser1;
      parser1.setAppData(&gpsk1);
      parser1.setRawData(msg);
      try {
        parser1.parseRawToApp();

        EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Fail received [%d].\n", gpsk1.FailureCode());
      }
      catch (...) {

        // Try GPSK-Protected-Fail.
        EapGpskProtectedFail gpsk2;
        EapGpskProtectedFailParser parser2;
        parser2.setAppData(&gpsk2);
        parser2.setRawData(msg);
        try {
          parser2.parseRawToApp();

          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Protected-Fail received [%d].\n", gpsk2.FailureCode());
        }
        catch (...) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail Parse error.\n");
          msm.Event(EvSgInvalid);
          return;
        }
      }

      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
      ssm.MethodState() = EapAuthSwitchStateMachine::END;
    }
  };

  class AcBuildGpskFailMsg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      // Send the GPSK-Fail
      EapGpskFail gpsk;
      gpsk.FailureCode() = msm.FailureCode();

      EAP_LOG(LM_ERROR, "AuthGpsk: Building GPSK-Fail [%d].\n", msm.FailureCode());

      EapGpskFailParser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcBuildGpskProtectedFailMsg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      // Send the GPSK-Fail
      EapGpskProtectedFail gpsk;
      gpsk.FailureCode() = msm.FailureCode();

      EAP_LOG(LM_ERROR, "AuthGpsk: Sending GPSK-Protected-Fail [%d].\n", msm.FailureCode());

      EapGpskProtectedFailParser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Protected-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4 + 2, msg->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), gpsk.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Write the message again.
      try { parser.parseAppToRaw(); }
        catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error generating GPSK-Protected-Fail.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcBuildGpsk1 : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      EAP_LOG(LM_DEBUG, "AuthGpsk: Building GPSK1 message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      // Generate a RAND for server
      unsigned char randServer[32];
      if (RAND_bytes(randServer, sizeof(randServer)) == 0)
        {
          EAP_LOG(LM_ERROR,
                "AuthGpsk: Failed to generate a server RAND for GPSK1.\n");
          return;
        }
      msm.ServerRAND().assign((const char*)randServer, sizeof(randServer));

      // Obtain shared secret from the application.
      msm.SharedSecret() = msm.InputSharedSecret();

      EapGpsk1 gpsk;
      gpsk.IDServer() = msm.ServerID() = msm.InputIdentity();
      gpsk.RANDServer() = msm.ServerRAND();
      gpsk.CSuiteList() = msm.CipherSuiteList();

      EapGpsk1Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK1.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcDoIntegrityCheckForGpsk2Msg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "AuthGpsk: Integrity check on GPSK2 message.\n");

      // Check the GPSK2 message.
      EapGpsk2 gpsk;
      EapGpsk2Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK2 message.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Validate server RAND
      if (gpsk.RANDServer() != msm.ServerRAND()) {
          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK2 has invalid server RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate CSuite list
      if (gpsk.CSuiteList() != msm.CipherSuiteList()) {
          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK2 has invalid Cipher Suite list.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate CSuite selected
      if (! msm.CipherSuiteList().isPresent(gpsk.CSuiteSelected())) {
          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK2 has invalid selected Cipher Suite.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate User
      if (! msm.ValidatePeerIdentity(gpsk.IDPeer())) {
          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK2 has invalid user ID.\n");
          msm.FailureCode() = EAP_GPSK_FAILURE_AUTHENTICATION_FAILURE;
          msm.Event(EvSgFail);
          return;
      }

      // Save attributes
      msm.PeerID() = gpsk.IDPeer();
      msm.PeerRAND() = gpsk.RANDPeer();
      msm.CipherSuite() = gpsk.CSuiteSelected();
      msm.Payload() = gpsk.PDPayload();

      // Save existing MAC
      std::string mac1 = gpsk.MAC();
      gpsk.MAC().resize(0);

      // Re-generate the MAC
      AAAMessageBlock *raw = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(raw->base(), 0, GPSK_MAX_PKT_SIZE);
      parser.setAppData(&gpsk);
      parser.setRawData(raw);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: Parse error when generating GPSK2 for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and generate MAC
      std::string mac2;
      EapCryptoAES_CMAC_128 macCalculator;
      std::string msgInput(raw->base() + 4 + 2, raw->length() - 4 - 2);

      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);
      raw->Release();

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "AuthGpsk: Invalid MAC2 for GPSK2.\n");
          msm.Event(EvSgFail);
          return;
        }

      // Is peer authorized
      if (! msm.IsPeerAuthorized(gpsk.IDPeer()))
        {
          EAP_LOG(LM_ERROR, "AuthGpsk: GPSK2 has un-authorized user ID.\n");
          msm.FailureCode() = EAP_GPSK_FAILURE_AUTHORIZATION_FAILURE;
          msm.Event(EvSgProtectedFail);
          return;
        }

      msm.Event(EvSgValid);
    }
  };

  class AcBuildGpsk3 : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Building GPSK3 message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), 0, GPSK_MAX_PKT_SIZE);

      // Get an Server ID.
      EapGpsk3 gpsk;
      gpsk.IDServer() = msm.ServerID();
      gpsk.RANDPeer() = msm.PeerRAND();
      gpsk.RANDServer() = msm.ServerRAND();
      gpsk.CSuiteSelected() = msm.CipherSuite();

      EapGpsk3Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK3.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4 + 2, msg->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), gpsk.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: Parse error when generating GPSK2.\n");
        msm.Event(EvSgInvalid);
        return;
     }

      // resize buffer
      msg->size(msg->length());

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcDoIntegrityCheckForGpsk4Msg : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();

      EAP_LOG(LM_DEBUG, "AuthGpsk: Do Identity Check on GPSK4 message.\n");

      // Check the GPSK4.
      EapGpsk4 gpsk;
      EapGpsk4Parser parser;
      parser.setAppData(&gpsk);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GSPK4.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Save existing MAC
      std::string mac1 = gpsk.MAC();
      gpsk.MAC().resize(0);

      // Re-generate the MAC
      AAAMessageBlock *raw = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(raw->base(), 0, GPSK_MAX_PKT_SIZE);
      parser.setAppData(&gpsk);
      parser.setRawData(raw);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK4 for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(raw->base() + 4 + 2, raw->length() - 4 - 2);
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);
      raw->Release();

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2 for GPSK4.\n");
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
    EvSgFail,
    EvSgProtectedFail,
    EvSgInvalid,
  };
  enum state {
    StInitialize,
    StGpsk1Sent,
    StProcessGpsk2,
    StGpskFailSent,
    StGpsk3Sent,
    StProcessGpsk4,
    StSuccess
  };

  AcBuildGpsk1 acBuildGpsk1;
  AcBuildGpsk3 acBuildGpsk3;
  AcNotifyInvalid acNotifyInvalid;
  AcNotifyFailure acNotifyFailure;
  AcNotifySuccess acNotifySuccess;
  AcDoIntegrityCheckForGpsk2Msg acDoIntegrityCheckForGpsk2Msg;
  AcDoIntegrityCheckForGpsk4Msg acDoIntegrityCheckForGpsk4Msg;
  AcBuildGpskFailMsg acBuildGpskFailMsg;
  AcDoIntegrityCheckForGpskFailMsg acDoIntegrityCheckForGpskFailMsg;
  AcBuildGpskProtectedFailMsg acBuildGpskProtectedFailMsg;

  EapAuthGpskStateTable_S()
  {
    AddStateTableEntry(StInitialize,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StGpsk1Sent, acBuildGpsk1);
    AddStateTableEntry(StInitialize, StInitialize, 0);

    AddStateTableEntry(StGpsk1Sent,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessGpsk2, acDoIntegrityCheckForGpsk2Msg);
    AddStateTableEntry(StGpsk1Sent, StGpsk1Sent, 0);

    AddStateTableEntry(StProcessGpsk2, EvSgInvalid,
		       StGpsk1Sent, acNotifyInvalid);
    AddStateTableEntry(StProcessGpsk2, EvSgFail,
		       StGpskFailSent, acBuildGpskFailMsg);
    AddStateTableEntry(StProcessGpsk2, EvSgProtectedFail,
		       StGpskFailSent, acBuildGpskProtectedFailMsg);
    AddStateTableEntry(StProcessGpsk2, EvSgValid,
		       StGpsk3Sent, acBuildGpsk3);

    AddStateTableEntry(StGpskFailSent,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StInitialize, acDoIntegrityCheckForGpskFailMsg);
    AddStateTableEntry(StGpskFailSent, EvSgValid,
		       StInitialize, acNotifyFailure);
    AddStateTableEntry(StGpskFailSent, EvSgInvalid,
		       StInitialize, acNotifyFailure);

    AddStateTableEntry(StGpsk3Sent,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessGpsk4, acDoIntegrityCheckForGpsk4Msg);
    AddStateTableEntry(StGpsk3Sent, StGpsk3Sent, 0);

    AddStateTableEntry(StProcessGpsk4, EvSgInvalid,
		       StGpsk3Sent, acNotifyInvalid);
    AddStateTableEntry(StProcessGpsk4, EvSgValid,
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
  (*this, *EapPeerGpskStateTable::instance(), s.Reactor(), s, (char *)"Gpsk(peer)")
{
} 

EapAuthGpskStateMachine::EapAuthGpskStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthGpskStateMachine>
  (*this, *EapAuthGpskStateTable::instance(),
   s.Reactor(), s, (char *)"Gpsk(authenticator)")
{
}

