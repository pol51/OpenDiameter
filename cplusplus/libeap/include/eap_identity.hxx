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
// $Id: eap_identity.hxx,v 1.16 2004/07/25 00:50:27 vfajardo Exp $

// eap_identity.hxx:  Identity method state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_IDENTITY_HXX__
#define __EAP_IDENTITY_HXX__

#include <ace/Basic_Types.h>
#include "eap.hxx"
#include "eap_fsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_peerfsm.hxx"

/// This class implements Identity method at authenticator side.
class EAP_EXPORTS EapAuthIdentityStateMachine : 
  public EapMethodStateMachine,
  public EapStateMachine<EapAuthIdentityStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthIdentityStateMachine>;
  friend class EapAuthIdentityStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    EapStateMachine<EapAuthIdentityStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthIdentityStateMachine>::Notify(ev);
  }

  void Receive(AAAMessageBlock *b) {}
  /// Maximum number of retry for sending Request/Identity.
  static const ACE_UINT32 maxTrial;

  /// Use this function to obtain the current number of trial for this method.
  ACE_UINT32& NumberOfTrial() { return nTrial; }

  /// Used by ProcessIdentity() as return values.
  enum ProcessIdentityResult {
    Success,
    Failure
  };

  /// This pure virtual function is a callback used when an
  /// identity of the peer is received.
  virtual ProcessIdentityResult ProcessIdentity(std::string& identity)=0;

protected:
  EapAuthIdentityStateMachine(EapSwitchStateMachine &s);
  ~EapAuthIdentityStateMachine() {} 

  ACE_UINT32 nTrial;
};

#endif // __EAP_IDENTITY_HXX__
