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

#ifndef __EAP_GPSK_FSM_HXX__
#define __EAP_GPSK_FSM_HXX__

#include <ace/Reactor.h>
#include <iostream>
#include "eap_gpsk.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_method_registrar.hxx"
#include "eap_gpsk_crypto.hxx"

/// EAP-Gpsk key management
class EAP_GPSK_EXPORT EapGpskNodeAttributes
{
public:

  /// This function is used for obtaining a reference to sharedSecret.
  std::string& SharedSecret() { return sharedSecret; }

  /// This function is used for obtaining a reference to masterKey.
  std::string& MK() {
    if (mk.size() == 0)
      {
        // Input string.
        std::string input(peerRAND);
        input.append(peerID);
        input.append(serverRAND);
        input.append(serverID);

        // PSK[0..KS-1].
        std::string psk = std::string(sharedSecret.data(), cipherSuite.KeySize());
        std::string Y(psk);

        // PL || PSK || CSuite_Sel || inputString.
        char pl[2];
        *(ACE_UINT16*)pl = ACE_HTONS(cphierSuite.KeySize());
        std::string Z(pl, sizeof(pl));
        Z.append(sharedSecret);
        Z.append(cipherSuite.toString());
        Z.append(input);

        // MK = GKDF-16 (PSK[0..15], PL || PSK || CSuite_Sel || inputString)
        EapCryptoAES_CMAC_128_GKDF gkdf;
        gkdf(Y, Z, mk, 16);
      }
    return mk;
  }

  /// This function is used for obtaining a reference to MSK.
  std::string& MSK() {
    if ((msk.size() == 0) && (mk.size() != 0))
      {
        // MSK = GKDF-160 (MK, inputString)[0..63]
        std::string output = GKDF160();
        msk.assign(output.data(), 8);
      }
    return msk;
  }

  /// This function is used for obtaining a reference to EMSK.
  std::string& EMSK() {
    if ((emsk.size() == 0) && (mk.size() != 0))
      {
        // EMSK = GKDF-160 (MK, inputString)[64..127]
        std::string output = GKDF160();
        emsk.assign(output.data() + 8, 8);
      }
    return msk;
  }

  /// This function is used for obtaining a reference to EMSK.
  std::string& SK() {
    if ((sk.size() == 0) && (mk.size() != 0))
      {
        // SK = GKDF-160 (MK, inputString)[128..143]
        std::string output = GKDF160();
        sk.assign(output.data() + 16, 2);
      }
    return msk;
  }

  /// This function is used for obtaining a reference to EMSK.
  std::string& PK() {
    if ((pk.size() == 0) && (mk.size() != 0))
      {
        // PK = GKDF-160 (MK, inputString)[144..159]
        std::string output = GKDF160();
        pk.assign(output.data() + 18, 2);
      }
    return msk;
  }

  /// This function is used for obtaining a reference to peerID.
  std::string& PeerID() { return peerID; }

  /// This function is used for obtaining a reference to authID.
  std::string& ServerID() { return serverID; }

  /// This function is used for obtaining a reference to peerRAND;
  std::string& PeerRAND() { return peerRAND; }

  /// This function is used for obtaining a reference to serverRAND;
  std::string& ServerRAND() { return serverRAND; }

  /// This function is used for obtaining a reference to serverRAND;
  EapGpskCipherSuiteList &CipherSuiteList() { return cipherSuiteList; }

  /// This function is used for obtaining a reference to cipherSuite;
  EapGpskCipherSuite &CipherSuite() { return cipherSuite; }

  /// This function is used for obtaining a reference to payload;
  std::string& Payload() { return payload; }

  void Initiliaze() {
     sharedSecret.resize(0);
     mk.resize(0);
     msk.resize(0);
     emsk.resize(0);
     sk.resize(0);
     pk.resize(0);
     peerID.resize(0);
     serverID.resize(0);
     peerRAND.resize(0);
     serverRAND.resize(0);
  }

protected:

  /// Shared secret.
  std::string sharedSecret;

  /// Master Key that is used for deriving an MSK.
  std::string mk;

  /// Master Session Key
  std::string msk;

  /// Extended-Master Session Key
  std::string emsk;

  /// Session Key
  std::string sk;

  /// Protected data key
  std::string pk;

  /// peer id and server id.
  std::string peerID, serverID;

  /// peer and server RAND
  std::string peerRAND, serverRAND;

  /// cipher suite list
  EapGpskCipherSuiteList cipherSuiteList;

  /// selected cipher suite
  EapGpskCipherSuite cipherSuite; // only AES is supported for now

  /// Retains payload carried in the message
  std::string payload;

private:

  /// This function is used for generated derived keys
  std::string GKDF160() {
    std::string output;
    if (mk.size() != 0)
      {
        // Input string.
        std::string input(peerRAND);
        input.append(peerID);
        input.append(serverRAND);
        input.append(serverID);

        // MSK = GKDF-160 (MK, inputString)
        EapCryptoAES_CMAC_128_GKDF gkdf;
        gkdf(mk, input, output, 20);
      }
    return output;
  }
};

/// Peer state machine for EAP EAP-Gpsk authentication method.
class EAP_GPSK_EXPORTS EapPeerGpskStateMachine
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerGpskStateMachine>,
     public EapGpskNodeAttributes
{
  friend class EapMethodStateMachineCreator<EapPeerGpskStateMachine>;
  friend class EapPeerGpskStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    isDone=false;
    Initialize();
    history.resize(0);

    EapStateMachine<EapPeerGpskStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerGpskStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
  virtual std::string& InputSharedSecret()=0;

  /// This pure virtual function is a callback used when a PeerID
  /// needs to be obtained.
  virtual std::string& InputIdentity()=0;

protected:
  EapPeerGpskStateMachine(EapSwitchStateMachine &s);

  ~EapPeerGpskStateMachine() {}
};

/// Authenticator state machine for EAP-Gpsk authentication method.
class EAP_GPSK_EXPORTS EapAuthGpskStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapAuthGpskStateMachine>,
     public EapGpskNodeAttributes
{
  friend class EapMethodStateMachineCreator<EapAuthGpskStateMachine>;
  friend class EapAuthGpskStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    history.resize(0);
    Initiliaze();
    EapStateMachine<EapAuthGpskStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthGpskStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
  virtual std::string& InputSharedSecret()=0;

  /// This pure virtual function is a callback used when an AuthID
  /// needs to be obtained.
  virtual std::string& InputIdentity()=0;

  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

protected:
  EapAuthGpskStateMachine(EapSwitchStateMachine &s);

  ~EapAuthGpskStateMachine() {}

  /// Retains received messages with concatinating them.
  std::string history;
};

#endif //__EAP_GPSK_FSM_HXX__
