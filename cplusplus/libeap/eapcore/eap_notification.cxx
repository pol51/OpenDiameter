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
// $Id: eap_notification.cxx,v 1.31 2004/06/17 21:13:35 yohba Exp $

// eap_notification.cxx:  EAP Identity method state machine
// Written by Yoshihiro Ohba

#include <string>
#include <ace/Singleton.h>
#include <ace/OS_String.h>
#include <ace/Message_Block.h>
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_notification.hxx"
#include "eap_method_registrar.hxx"
#include "eap_parser.hxx"
#include "eap_log.hxx"

/// Action class.
class EapAuthNotificationAction : 
  public AAA_Action<EapAuthNotificationStateMachine>
{
 public:
  virtual void operator()(EapAuthNotificationStateMachine&) {}
 protected:
  EapAuthNotificationAction() {}
  virtual ~EapAuthNotificationAction() {}
};

/// State table used by EapAuthNotificationStateMachine.
class EapAuthNotificationStateTable_S :
  public AAA_StateTable<EapAuthNotificationStateMachine>
{
  friend class ACE_Singleton
  <EapAuthNotificationStateTable_S, ACE_Recursive_Thread_Mutex>;

private:

  class AcDoIntegrityCheck : public EapAuthNotificationAction
  {
    void operator()(EapAuthNotificationStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "Notification: Do Identity Check.\n");

      // Check the payload.
      EapNotification notification;
      EapRequestNotificationParser parser;
      parser.setAppData(&notification);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); } 
      catch (...) { 
	msm.Event(EvSgFailure);
	return;
      }
      msm.Event(EvSgSuccess);
    }
  };

  class AcNotifySuccess : public EapAuthNotificationAction
  {
    void operator()(EapAuthNotificationStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      // release the outstanding request.
      ssm.SetTxMessage(0);
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyFailure : public EapAuthNotificationAction
  {
    void operator()(EapAuthNotificationStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      // Release the outstanding request.
      ssm.SetTxMessage(0);
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
    }
  };

  class AcPrepareRequest : public EapAuthNotificationAction
  {
    void operator()(EapAuthNotificationStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      EapNotification req;
    
      req.Notification() = ssm.NotificationString();
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5 + req.Notification().size());

      // Use parser to set Type field.
      EapRequestNotificationParser parser;
      parser.setAppData(&req);
      parser.setRawData(msg);
      try {
	parser.parseAppToRaw();
      }
      catch (...) {
	EAP_LOG(LM_ERROR, "Notification: Request parse error.\n");
	ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
	return;
      }

      ssm.SetTxMessage(msg);
    
      EAP_LOG(LM_DEBUG, "Notification: Request Prepared.\n");
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  enum {
    EvSgSuccess,
    EvSgFailure
  };

  enum state {
    StInitialize, 
    StWaitResponse, 
    StProcessResponse,
    StSuccess,
    StFailure
  };

  AcDoIntegrityCheck acDoIntegrityCheck;
  AcNotifySuccess acNotifySuccess;
  AcNotifyFailure acNotifyFailure;
  AcPrepareRequest acPrepareRequest;

  // Defined as leaf class
  EapAuthNotificationStateTable_S() 
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
  ~EapAuthNotificationStateTable_S() {}
};

typedef ACE_Singleton<EapAuthNotificationStateTable_S, 
		      ACE_Recursive_Thread_Mutex> 
EapAuthNotificationStateTable;

EapAuthNotificationStateMachine::EapAuthNotificationStateMachine
(EapSwitchStateMachine &s) :
  EapMethodStateMachine(s),
  EapStateMachine<EapAuthNotificationStateMachine>
  (*this, *EapAuthNotificationStateTable::instance(), s.Reactor(), 
   s, "notification") 
{}  

