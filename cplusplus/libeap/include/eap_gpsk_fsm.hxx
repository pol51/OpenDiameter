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

/// Peer state machine for EAP EAP-Gpsk authentication method.
class EAP_GPSK_EXPORTS EapPeerGpskStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerGpskStateMachine>
{
  friend class EapMethodStateMachineCreator<EapPeerGpskStateMachine>;
  friend class EapPeerGpskStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    isDone=false;
    keyData.resize(0);
    masterKey.resize(0);
    history.resize(0);
    keyConfirmationKey.resize(0);
    keyEncryptionKey.resize(0);
    keyDerivationKey.resize(0);

    EapStateMachine<EapPeerGpskStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerGpskStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
*  virtual std::string& InputSharedSecret()=0;

  /// This pure virtual function is a callback used when a PeerID
  /// needs to be obtained.
  virtual std::string& InputIdentity()=0;

  /// This function is used for obtaining a reference to sharedSecret.
  std::string& SharedSecret() { return sharedSecret; }

  /// This function is used for obtaining a reference to keyConfirmationKey.
  std::string& KCK() 
  { 
    if (keyConfirmationKey.size() != 16)
      keyConfirmationKey.assign(sharedSecret, 0, 16);
    return keyConfirmationKey; 
  }

  /// This function is used for obtaining a reference to keyEncryptionKey.
  std::string& KEK() { 
    if (keyEncryptionKey.size() != 16)
      keyEncryptionKey.assign(sharedSecret, 16, 16);
    return keyEncryptionKey; 
  }

  /// This function is used for obtaining a reference to keyDerivationKey.
  std::string& KDK() { 
    if (keyDerivationKey.size() != 32)
      keyDerivationKey.assign(sharedSecret, 32, 32);
    return keyDerivationKey; 
  }

  /// This function is used for obtaining a reference to masterKey.
  std::string& MK() { 
    if (masterKey.size() != 32 && 
	this->PeerSwitchStateMachine().MethodState() 
	== EapPeerSwitchStateMachine::DONE)
      {
	// Compute the master key.
        std::string tmp(nonceA);
	tmp.append(nonceP);
	tmp.append("Gpsk session key");
	EapCryptoGpskPRF prf;
	prf(tmp, masterKey, KDK(), 32);
      }
    return masterKey; 
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

  /// This function is used for obtaining a reference to sessionID.
  std::string& SessionID() { return sessionID; }

  /// This function is used for obtaining a reference to binding.
  GpskBinding& Binding() { return binding; }

  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

protected:
  EapPeerGpskStateMachine(EapSwitchStateMachine &s);

  ~EapPeerGpskStateMachine() {} 

  /// Shared secret.
  std::string sharedSecret;

  /// Key-Confirmation Key.
  std::string keyConfirmationKey;

  /// Key-Encryption Key.
  std::string keyEncryptionKey;

  /// Key-Derivation Key.
  std::string keyDerivationKey;

  /// Master Key that is used for deriving an MSK.
  std::string masterKey;

  /// session id.
  std::string sessionID;

  /// peer id and server id.
*  std::string peerID, serverID;

  /// peer and server RAND
*  std::string peerRAND, serverRAND;

  /// cipher suite list
*  EapGpskCipherSuiteList cipherSuiteList;

  /// binding
  GpskBinding binding;

  /// Retains received messages with concatinating them.
  std::string history;
};

/// Authenticator state machine for EAP-Gpsk authentication method.
class EAP_GPSK_EXPORTS EapAuthGpskStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapAuthGpskStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthGpskStateMachine>;
  friend class EapAuthGpskStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    keyData.resize(0);
    masterKey.resize(0);
    history.resize(0);
    keyConfirmationKey.resize(0);
    keyEncryptionKey.resize(0);
    keyDerivationKey.resize(0);
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

  /// This function is used for obtaining a reference to sharedSecret.
  std::string& SharedSecret() { return sharedSecret; }

  /// This function is used for obtaining a reference to keyConfirmationKey.
  std::string& KCK() 
  { 
    if (keyConfirmationKey.size() != 16)
      keyConfirmationKey.assign(sharedSecret, 0, 16);
    return keyConfirmationKey; 
  }

  /// This function is used for obtaining a reference to keyEncryptionKey.
  std::string& KEK() { 
    if (keyEncryptionKey.size() != 16)
      keyEncryptionKey.assign(sharedSecret, 16, 16);
    return keyEncryptionKey; 
  }

  /// This function is used for obtaining a reference to keyDerivationKey.
  std::string& KDK() { 
    if (keyDerivationKey.size() != 32)
      keyDerivationKey.assign(sharedSecret, 32, 32);
    return keyDerivationKey; 
  }

  /// This function is used for obtaining a reference to masterKey.
  std::string& MK() { 
    if (masterKey.size() != 32 && IsDone())
      {
	// Compute the master key.
        std::string tmp(nonceA);
	tmp.append(nonceP);
	tmp.append("Gpsk session key");
	EapCryptoGpskPRF prf;
	prf(tmp, masterKey, KDK(), 32);
      }
    return masterKey; 
  }

  /// This function is used for obtaining a reference to sessionID.
  std::string& SessionID() { return sessionID; }

  /// This function is used for obtaining a reference to peerID.
  std::string& PeerID() { return peerID; }

  /// This function is used for obtaining a reference to authID.
  std::string& AuthID() { return authID; }

  /// This function is used for obtaining a reference to binding.
  GpskBinding& Binding() { return binding; }

  /// This function is used for obtaining a reference to nonceA;
  std::string& NonceA() { return nonceA; }

  /// This function is used for obtaining a reference to nonceP;
  std::string& NonceP() { return nonceP; }

  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

protected:
  EapAuthGpskStateMachine(EapSwitchStateMachine &s);

  ~EapAuthGpskStateMachine() {} 

  /// Shared secret.
  std::string sharedSecret;

  /// Key-Confirmation Key.
  std::string keyConfirmationKey;

  /// Key-Encryption Key.
  std::string keyEncryptionKey;

  /// Key-Derivation Key.
  std::string keyDerivationKey;

  /// Master Key that is used for deriving an MSK.
  std::string masterKey;

  /// session id.
  std::string sessionID;

  /// peer id and auth id.
  std::string peerID, authID;

  /// nonceP and nonceA and auth id.
  std::string nonceP, nonceA;

  /// binding
  GpskBinding binding;

  /// Retains received messages with concatinating them.
  std::string history;
};

#endif //__EAP_GPSK_FSM_HXX__
