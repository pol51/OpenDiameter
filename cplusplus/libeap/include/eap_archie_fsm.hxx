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
// $Id: eap_archie_fsm.hxx,v 1.6 2004/06/17 21:13:36 yohba Exp $

// EAP-Archie state machines.
// Written by Yoshihiro Ohba (yohba@tari.toshiba.com)

#ifndef __EAP_ARCHIE_FSM_HXX__
#define __EAP_ARCHIE_FSM_HXX__

#include <ace/Reactor.h>
#include <iostream>
#include "eap_archie.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_method_registrar.hxx"
#include "eap_archie_crypto.hxx"

/// Peer state machine for EAP EAP-Archie authentication method.
class EAP_ARCHIE_EXPORTS EapPeerArchieStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerArchieStateMachine>
{
  friend class EapMethodStateMachineCreator<EapPeerArchieStateMachine>;
  friend class EapPeerArchieStateTable_S;
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

    EapStateMachine<EapPeerArchieStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerArchieStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
  virtual std::string& InputSharedSecret()=0;

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
	tmp.append("Archie session key");
	EapCryptoArchiePRF prf;
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
  ArchieBinding& Binding() { return binding; }

  /// This function is used for obtaining a reference to nonceA;
  std::string& NonceA() { return nonceA; }

  /// This function is used for obtaining a reference to nonceP;
  std::string& NonceP() { return nonceP; }

  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

protected:
  EapPeerArchieStateMachine(EapSwitchStateMachine &s);

  ~EapPeerArchieStateMachine() {} 

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
  ArchieBinding binding;

  /// Retains received messages with concatinating them.
  std::string history;
};

/// Authenticator state machine for EAP-Archie authentication method.
class EAP_ARCHIE_EXPORTS EapAuthArchieStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapAuthArchieStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthArchieStateMachine>;
  friend class EapAuthArchieStateTable_S;
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
    EapStateMachine<EapAuthArchieStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthArchieStateMachine>::Notify(ev);
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
	tmp.append("Archie session key");
	EapCryptoArchiePRF prf;
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
  ArchieBinding& Binding() { return binding; }

  /// This function is used for obtaining a reference to nonceA;
  std::string& NonceA() { return nonceA; }

  /// This function is used for obtaining a reference to nonceP;
  std::string& NonceP() { return nonceP; }

  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

protected:
  EapAuthArchieStateMachine(EapSwitchStateMachine &s);

  ~EapAuthArchieStateMachine() {} 

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
  ArchieBinding binding;

  /// Retains received messages with concatinating them.
  std::string history;
};

#endif //__EAP_ARCHIE_FSM_HXX__
