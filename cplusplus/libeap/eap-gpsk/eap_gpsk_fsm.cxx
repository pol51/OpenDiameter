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

  class AcBuildGpskFailMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: checking on GPSK-Fail message.\n");

      // Check the GPSK-Fail.
      EapGpskFail request;
      EapRequestGpskFailParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Fail received [%d].\n", request->FailureCode());

      AAAMessageBlock *rbuf = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(rbuf->base(), GPSK_MAX_PKT_SIZE);

      // Re-play the failure
      EapGpskFail response;
      response.FailureCode() = request->FailureCode();

      parser.setAppData(&response);
      parser.setRawData(rbuf);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Set the message to the session.
      ssm.SetTxMessage(rbuf);

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);

      // Continue to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcBuildGpskProtectedFailMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: checking on GPSK-Protected-Fail message.\n");

      // Check the GPSK-Fail.
      EapRequestGpskProtectedFail request;
      EapRequestGpskProtectedFailParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Save existing MAC
      std::string mac1 = request.MAC();
      request.MAC().resize(0);

      // Re-generate the MAC
      EapResponseGpskProtectedFailParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating Protected-Fail for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail received [%d].\n", request->FailureCode());

      AAAMessageBlock *rbuf = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(rbuf->base(), GPSK_MAX_PKT_SIZE);

      // Re-play the failure
      EapRequestGpskProtectedFail response;
      response.FailureCode() = request->FailureCode();

      parser.setAppData(&response);
      parser.setRawData(rbuf);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK-Protected-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Get the message stream and compute MAC
      std::string msg2Input(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msg2Input,
           msg2Input.size(), response.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK-Protected-Fail.\n");
        msm.Event(EvSgInvalid);
        return;
     }

      // Set the message to the session.
      ssm.SetTxMessage(rbuf);

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);

      // Continue to the next step.
      msm.Event(EvSgValid);
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

  class AcBuildGpsk2 : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerGpsk: Building a GPSK2 message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);

      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapResponseGpsk2 response;

      response.IDPeer() = msm.PeerID() = msm.InputIdentity();
      response.IDServer() = msm.ServerID();
      response.RANDServer() = msm.ServerRAND();
      response.CSuiteList() = msm.CipherSuiteList();
      response.CSuiteSelected() = msm.CipherSuite();

      // Compute RAND_Peer.
      unsigned char pRAND[32];
      if (RAND_bytes(pRAND, 32) == 0)
        {
          EAP_LOG(LM_ERROR, 
             "AuthGpskStateTable: Failed to generate peer RAND.\n");
          msm.Event(EvSgInvalid);
          return;
        }
      response.RANDPeer() = msm.PeerRAND()
         = std::string((char*)pRAND, sizeof(pRAND));

      EapResponseGpsk2Parser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK2.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), response.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Parse the message again to write the calculated MAC.
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK2.\n");
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

  class AcDoIntegrityCheckForGpsk3Msg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerGpsk: checking on GPSK3 message.\n");

      ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();

      AcBuildGpskFailMsg gpskFailAction;
      AcBuildGpskProtectedFailMsg gpskProtectedFailAction;

      // Read Op-Code to check for failures
      swtich (opCode)
        {
        case 3: break; // GPSK-3, continue
        case 5: gpskFailAction(msm); return;
        case 6: gpskProtectedFailAction(msm); return;
        default:
          EAP_LOG(LM_ERROR, "Invalid opCode %d (3 is expected).\n", opCode);
          msm.Event(EvSgInvalid);
          return;
        }

      // Check the GPSK3.
      EapRequestGpsk3 request;
      EapRequestGpsk3Parser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Check RAND values
      if (request.RANDPeer() != msm.PeerRAND()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid peer RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }
      if (request.RANDServer() != msm.ServerRAND()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid server RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Check selection
      if (msm.CipherSuite() != request.CSuiteSelected) {
         EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid selected CSuite value.\n");
         msm.Event(EvSgInvalid);
         return;
      }

      // Save the payload, TBD: Need to do something about the payload in the future
      msm.Payload() = request.PDPayload();

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Save existing MAC
      std::string mac1 = request.MAC();
      request.MAC().resize(0);

      // Re-generate the MAC
      EapResponseGpsk3Parser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK3 for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);

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

  class AcNotifyInvalid : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };

  class AcNotifyFail : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
    }
  };

  class AcBuildGpsk4 : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      // prepare the Response/Gpsk-Finish
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AcBuildGpskFailMsg gpskFailAction;
      AcBuildGpskProtectedFailMsg gpskProtectedFailAction;

      // Read Op-Code to check for failures
      swtich (opCode)
        {
        case 3: break; // GPSK-3, continue
        case 5: gpskFailAction(msm); return;
        case 6: gpskProtectedFailAction(msm); return;
        default:
          EAP_LOG(LM_ERROR, "Invalid opCode %d (3 is expected).\n", opCode);
          msm.Event(EvSgInvalid);
          return;
        }

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);

      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapResponseGpsk4 response;

      EapResponseGpsk4Parser parser;
      parser.setAppData(&response);
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
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), response.MAC());

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
      EAP_LOG(LM_DEBUG, "PeerGpsk: Gpsk4 prepared.\n");

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;

      msm.KeyData() = msm.MK();

      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
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

  AcDoIntegrityCheckForGpsk1Msg acDoIntegrityCheckForGpsk1Msg;
  AcDoIntegrityCheckForGpsk3Msg acDoIntegrityCheckForGpsk3Msg;
  AcBuildGpsk2 acBuildGpsk2;
  AcBuildGpsk4 acBuildGpsk4;
  AcNotifyInvalid acNotifyInvalid;
  AcNotifyFail acNotifyFail;

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
		       StGpsk2Sent, acNotifyFail);
    AddStateTableEntry(StProcessGpsk3, EvSgValid,
		       StSuccess, acBuildGpsk4);

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

  class AcDoIntegrityCheckForGpskFailMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Integrity check on GPSK-Fail message.\n");

      // Read Op-Code to check for failures
      swtich (opCode)
        {
        case 5:
        case 6: break;
        default:
          EAP_LOG(LM_ERROR, "Invalid opCode %d (3 is expected).\n", opCode);
          msm.Event(EvSgInvalid);
          return;
        }

      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcBuildGpskFailMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);

      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapGpskFail request;

      // Send the GPSK-Fail
      request.FailureCode() = msm.FailureCode();

      EAP_LOG(LM_ERROR, "AuthGpsk: Sending GPSK-Fail [%d].\n", msm.FailureCode());

      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Continue to the next step.
      msm.Event(EvSgValid);
    }
  };

  class AcBuildGpskProtectedFailMsg : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);

      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapGpskProtectedFail request;

      // Send the GPSK-Fail
      request.FailureCode() = msm.FailureCode();

      EAP_LOG(LM_ERROR, "AuthGpsk: Sending GPSK-Protected-Fail [%d].\n", msm.FailureCode());

      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "AuthGpsk: GPSK-Protected-Fail Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and compute MAC
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput,
           msgInput.size(), request.MAC());

      // Rewind the pointer.
      msg->wr_ptr(msg->base() + 4);

      // Write the message again.
      try { parser.parseAppToRaw(); }
        catch (...) {
          EAP_LOG(LM_ERROR, "PeerGpsk: Parse error generating GPSK-Protected-Fail.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Continue to the next step.
      msm.Event(EvSgValid);
  };

  class AcBuildGpsk1 : public EapAuthGpskAction
  {
    void operator()(EapAuthGpskStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "AuthGpsk: Building GPSK1 message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(GPSK_MAX_PKT_SIZE);
      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapRequestGpsk1 request;

      // Generate a RAND for server
      unsigned char randServer[32];
      if (RAND_bytes(randServer, sizeof(randServer)) == 0)
        {
          EAP_LOG(LM_ERROR,
                "AuthGpsk: Failed to generate a server RAND for GPSK1.\n");
          return;
        }
      msm.ServerRAND() = randServer;

      // Get a Server ID.
      request.IDServer() = msm.ServerID() = msm.InputIdentity();

      // CSuite list
      request.CSuiteList() = msm.CipherSuiteList();

      EapRequestGpsk1Parser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK1.\n");
          msm.Event(EvSgInvalid);
          return;
        }

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
      EapResponseGpsk2 response;
      EapResponseGpsk2Parser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK2 message.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Validate server RAND
      if (response.RANDServer() != msm.ServerRAND()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid server RAND values.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate CSuite list
      if (response.CSuiteList() != msm.CipherSuiteList()) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid Cipher Suite list.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate CSuite selected
      if (msm.CipherSuiteList().isPresent(response.CSuiteSelected())) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid selected Cipher Suite.\n");
          msm.Event(EvSgInvalid);
          return;
      }

      // Validate User
      if (msm.CheckPeerIdentity(response.IDPeer())) {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has invalid user ID.\n");
          msm.FailureCode() = EAP_GPSK_FAILURE_AUTHENTICATION_FAILURE;
          msm.Event(EvSgFail);
          return;
      }

      // Save attributes
      msm.PeerID() = response.IDPeer();
      msm.PeerRAND() = response.RANDPeer();
      msm.CipherSuite() = response.CSuiteSelected();
      msm.Payload() = response.PDPayload();

      // Save existing MAC
      std::string mac1 = response.MAC();
      response.MAC().resize(0);

      // Re-generate the MAC
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
        EAP_LOG(LM_ERROR, "PeerGpsk: Parse error when generating GPSK2 for MAC validation.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      // Obtain shared secret from the application.
      std::string& sharedSecret = msm.SharedSecret();

      // Get the message stream and generate MAC
      std::string mac2;
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2 for GPSK2.\n");
          msm.Event(EvSgFail);
          return;
        }

      // Is peer authorized
      if (msm.IsPeerAuthorized(response.IDPeer()))
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: GPSK3 has un-authorized user ID.\n");
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
      ACE_OS::memset(msg->base(), GPSK_MAX_PKT_SIZE);

      EapRequestGpsk3 request;

      // Generate a RAND for server
      unsigned char randServer[32];
      if (RAND_bytes(randServer, sizeof(randServer)) == 0)
        {
          EAP_LOG(LM_ERROR,
                "AuthGpsk: Failed to generate a server RAND for GPSK3.\n");
          return;
        }
      msm.ServerRAND() = randServer;

      // Get an Server ID.
      request.IDServer() = msm.ServerID() = msm.InputIdentity();

      // CSuite list
      request.CSuiteList() = msm.CipherSuiteList();

      EapRequestGpsk1Parser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GPSK3.\n");
          msm.Event(EvSgInvalid);
          return;
        }

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
      EapResponseGpsk4 response;
      EapResponseGpsk4Parser parser;
      parser.setAppData(&reponse);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch (...) {
          EAP_LOG(LM_ERROR, "AuthGpsk: Parse error for GSPK4.\n");
          msm.Event(EvSgInvalid);
          return;
        }

      // Save existing MAC
      std::string mac1 = response.MAC();
      response.MAC().resize(0);

      // Re-generate the MAC
      parser.setAppData(&response);
      parser.setRawData(msg);
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
      std::string msgInput(msg->base() + 4, msg->length());
      EapCryptoAES_CMAC_128 macCalculator;
      macCalculator(sharedSecret, msgInput, msgInput.size(), mac2);

      // do validity check.
      if (mac1 != mac2)
        {
          EAP_LOG(LM_ERROR, "PeerGpsk: Invalid MAC2 for GPSK4.\n");
          msm.Event(EvSgFail);
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

  class AcNotifySilentFail : public EapPeerGpskAction
  {
    void operator()(EapPeerGpskStateMachine &msm)
    {
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
  AcNotifySilentFail acNotifySilentFail;
  AcDoIntegrityCheckForGpsk2Msg acDoIntegrityCheckForGpsk2Msg;
  AcBuildGpskFailMsg acBuildGpskFailMsg;
  AcBuildGpskProtectedFailMsg acBuildGpskProtectedFailMsg;
  AcDoIntegrityCheckForGpskFailMsg acDoIntegrityCheckForGpskFailMsg;

  AcDoIntegrityCheckForResponseMsg acDoIntegrityCheckForResponseMsg;
  AcDoIntegrityCheckForFinishMsg acDoIntegrityCheckForFinishMsg;
  AcBuildConfirm acBuildConfirm;
  AcNotifySuccess acNotifySuccess;
  AcNotifyFailure acNotifyFailure;
  AcNotifyInvalid acNotifyInvalid;

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
		       StInitialize, acNotifyFailure);
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
    AddStateTableEntry(StGpskFailSent, EvSgInValid,
		       StInitialize, acNotifyFailure);

    AddStateTableEntry(StGpsk3Sent,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessGpsk4, acDoIntegrityCheckForGpsk4Msg);
    AddStateTableEntry(StGpsk3Sent, StGpsk3Sent, 0);

    AddStateTableEntry(StProcessGpsk4, EvSgInvalid,
		       StGpsk3Sent, acNotifySilentFail);
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
  (*this, *EapPeerGpskStateTable::instance(), s.Reactor(), s, "Gpsk(peer)")
{
} 

EapAuthGpskStateMachine::EapAuthGpskStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthGpskStateMachine>
  (*this, *EapAuthGpskStateTable::instance(), 
   s.Reactor(), s, "Gpsk(authenticator)")
{
} 
