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
// $Id: eap_authfsm.cxx,v 1.34 2004/06/17 21:13:35 yohba Exp $

// eap_authfsm.cxx:  Authenticator state machine
// Written by Yoshihiro Ohba

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include "eap.hxx"
#include "eap_authfsm.hxx"
#include "eap_policy.hxx"
#include "eap_log.hxx"
#include "eap_parser.hxx"

const ACE_UINT16 EapAuthSwitchStateMachine::defaultMaxRetransmissionCount=3;
const ACE_UINT16 EapAuthSwitchStateMachine::defaultRetransmissionInterval=3;

void
EapAuthSwitchStateMachine::Receive(AAAMessageBlock* msg)
{
  // Enqueue the received message with increasing reference counter
  // and generate a notification event.

  AAAMessageBlock*& msgR=msg;
  rxQueue.enqueue_tail(AAAMessageBlock::Acquire(msgR));
  Notify(EapAuthSwitchStateMachine::EvRxMsg);
}

EapAuthSwitchStateMachine::EapAuthDecision
EapAuthSwitchStateMachine::Decision()
{
  EapType type;

  if (policy.IsSatisfied())
    return DecisionSuccess;
    
  try {
    type = policy.CurrentMethod();
  } 
  catch (EapPolicy::PolicyError e) {  // No method to try.
    if (e == EapPolicy::NoCurrentMethod)
      return DecisionFailure;
  }

  return DecisionContinue;
}

EapAuthSwitchStateMachine::EapAuthMethodState&
EapAuthSwitchStateMachine::MethodState()
{
  return methodState;
}

