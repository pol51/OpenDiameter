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

/* 
   File: cc_client_test.cxx
   Author: Amrit Kaur (kaur_amrit@hotmail.com)   
*/

#include <vector>

#include <iostream>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/Atomic_Op_T.h>

#include "diameter_api.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_client_session.h"
#include "diameter_cc_application.h"

typedef AAA_JobHandle<AAA_GroupedJob> CCJobHandle;

class CCTask : public AAA_Task
{
 public:
  /// Constructor.
  CCTask() : AAA_Task(AAA_SCHED_FIFO, "CC") 
  {}

  /// Destructor.
  ~CCTask() {}
};

class CCClientSession : public DiameterCCClientSession
{
 public:
  CCClientSession(DiameterCCApplication & ccApp, CCJobHandle h) 
    : DiameterCCClientSession(ccApp, h)
  {}

  void Abort();

  void SignalContinue(DiameterCCAccount&);

  /// Reimplemented from the parent class.
  void SignalSuccess();

  /// Reimplemented from the parent class.
  void SignalFailure();

  /// Reimplemented from the parent class.
  void SignalReauthentication();

  /// Reimplemented from the parent class.
  void SignalDisconnect() {};

  void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &realm);

  void SetDestinationHost
  (DiameterScholarAttribute<diameter_utf8string_t> &host);

  void SendInitialRequest(subscriptionId_t& subscriptionId);

  bool InitialRequest();

  bool InitialAnswer();


  void SendTerminationRequest();

  bool TerminationRequest();


  void SendDirectDebitingRequest(subscriptionId_t& subscriptionId);

  bool DirectDebitingRequest();

  bool DirectDebitingAnswer();


  void SendCheckBalanceRequest(subscriptionId_t& subscriptionId);

  bool CheckBalanceRequest();

  bool CheckBalanceAnswer();


  void SendRefundAccountRequest(subscriptionId_t& subscriptionId);

  bool RefundAccountRequest();

  bool RefundAccountAnswer();

};

void 
CCClientSession::Abort()
  {
    std::cout << "Diameter CC client session aborted." << std::endl;
    CCClientSession::Stop();
    //JobData(Type2Type<DiameterCCClientApplication>()).Semaphore().release();
  }

void 
CCClientSession::SignalContinue
(DiameterCCAccount&)
{
}

void 
CCClientSession::SignalSuccess()
  {
    AAA_LOG((LM_DEBUG, "Client authentication successful.\n"));
   
    Stop();
    //JobData(Type2Type<DiameterCCClientApplication>()).Semaphore().release();
  }

void 
CCClientSession::SignalFailure()
{
  AAA_LOG((LM_DEBUG, "Client authentication failed.\n"));
  Abort();
}

void 
CCClientSession::SignalReauthentication()
{
  AAA_LOG((LM_DEBUG, "Client Re-authentication triggerred.\n"));
  Abort();
}

void
CCClientSession::SendInitialRequest(subscriptionId_t& subscriptionId)
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Notify Intial Request\n"));
  std::vector<subscriptionId_t> vec(1, subscriptionId);
  ccrData.SubscriptionId = vec;
  Notify(EvInitialRequest);
}

bool
CCClientSession::InitialRequest()
{
  ccrData.ServiceContextId = "opendiameter.org";
  
  ccrData.CCRequestType = CC_TYPE_INITIAL_REQUEST;

  ccrData.CCRequestNumber = 0;
  
  AAA_LOG((LM_DEBUG, "(%P|%t) \tRequested %d Service Units.\n",50 ));   
  unitValue_t unitValue (50, 0);
  ccMoney_t ccMoney(unitValue,840);
  requestedServiceUnit_t requestedServiceUnits(ccMoney);

  ccrData.RequestedServiceUnit = requestedServiceUnits;  

  DiameterCCClientStateMachine::InitialRequest();
  
  return true;
}

bool
CCClientSession::InitialAnswer()
{
  grantedServiceUnit_t& grantedServiceUnit = ccaData.GrantedServiceUnit();
  ccMoney_t& ccMoney = grantedServiceUnit.CCMoney();
  unitValue_t& unitValue = ccMoney.UnitValue();
  diameter_integer64_t& valueDigits = unitValue.ValueDigits();
 
  AAA_LOG((LM_DEBUG, "(%P|%t) \tGranted %d Service Units.\n",valueDigits ));

  DiameterCCClientStateMachine::InitialAnswer();

  SendTerminationRequest();
  return true;  

}

void
CCClientSession::SendTerminationRequest()
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Notify Termination Request\n"));
  Notify(EvTerminationRequest);
}

bool
CCClientSession::TerminationRequest()
{
  ccrData.ServiceContextId = "opendiameter.org";
  
  ccrData.CCRequestType = CC_TYPE_TERMINATION_REQUEST;

  ccrData.CCRequestNumber = 2;

  unitValue_t unitValue (20, 0);
  ccMoney_t ccMoney(unitValue,840); 
  std::vector<usedServiceUnit_t> usedServiceUnits(1, usedServiceUnit_t(ccMoney));
  AAA_LOG((LM_DEBUG, "(%P|%t) \tUsed %d Service Units.\n",unitValue.ValueDigits()));

  ccrData.UsedServiceUnit = usedServiceUnits;  
 
  DiameterCCClientSession::TerminationRequest(); 

  return true;
}

void
CCClientSession::SendRefundAccountRequest(subscriptionId_t& subscriptionId)
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Notify Refund Account Request\n"));
  std::vector<subscriptionId_t> vec(1, subscriptionId);
  ccrData.SubscriptionId = vec;
  Notify(EvRefundAccountRequest);
}

bool
CCClientSession::RefundAccountRequest()
{
  ccrData.ServiceContextId = "opendiameter.org";
  
  ccrData.CCRequestType = 4;
  ccrData.RequestedAction = CC_ACTION_REFUND_ACCOUNT;
  ccrData.CCRequestNumber = 0;

  AAA_LOG((LM_DEBUG, "(%P|%t) \tRefund %d Service Units to the Account.\n",40 ));   
  unitValue_t unitValue (40, 0);
  ccMoney_t ccMoney(unitValue,840);
  requestedServiceUnit_t requestedServiceUnits(ccMoney);

  ccrData.RequestedServiceUnit = requestedServiceUnits;  
 
  DiameterCCClientSession::RefundAccountRequest(); 

  return true;
}

bool
CCClientSession::RefundAccountAnswer()
{
  grantedServiceUnit_t& grantedServiceUnit = ccaData.GrantedServiceUnit();
  ccMoney_t& ccMoney = grantedServiceUnit.CCMoney();
  unitValue_t& unitValue = ccMoney.UnitValue();
  diameter_integer64_t& valueDigits = unitValue.ValueDigits();
 
  AAA_LOG((LM_DEBUG, "(%P|%t) \tRefunded %d Service Units.\n",valueDigits ));
  
  return true;  

}

void
CCClientSession::SendDirectDebitingRequest(subscriptionId_t& subscriptionId)
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Notify Direct Debiting Request\n"));
  std::vector<subscriptionId_t> vec(1, subscriptionId);
  ccrData.SubscriptionId = vec;
  Notify(EvDirectDebitingRequest);
}

bool
CCClientSession::DirectDebitingRequest()
{
  ccrData.ServiceContextId = "opendiameter.org";
  
  ccrData.CCRequestType = 4;
  ccrData.RequestedAction = CC_ACTION_DIRECT_DEBITING;
  ccrData.CCRequestNumber = 0;

  AAA_LOG((LM_DEBUG, "(%P|%t) \tDirect Debit %d Service Units.\n",20 ));   
  unitValue_t unitValue (20, 0);
  ccMoney_t ccMoney(unitValue,840);
  requestedServiceUnit_t requestedServiceUnits(ccMoney);

  ccrData.RequestedServiceUnit = requestedServiceUnits;  
 
  DiameterCCClientSession::DirectDebitingRequest(); 

  return true;
}

bool
CCClientSession::DirectDebitingAnswer()
{
  grantedServiceUnit_t& grantedServiceUnit = ccaData.GrantedServiceUnit();
  ccMoney_t& ccMoney = grantedServiceUnit.CCMoney();
  unitValue_t& unitValue = ccMoney.UnitValue();
  diameter_integer64_t& valueDigits = unitValue.ValueDigits();
 
  AAA_LOG((LM_DEBUG, "(%P|%t) \tDebited %d Service Units from Account.\n",valueDigits ));
  
  return true;  

}

void
CCClientSession::SendCheckBalanceRequest(subscriptionId_t& subscriptionId)
{
  ACE_DEBUG((LM_DEBUG, "(%P|%t) Notify Check Balance Request\n"));
  std::vector<subscriptionId_t> vec(1, subscriptionId);
  ccrData.SubscriptionId = vec;
  Notify(EvCheckBalanceRequest);
}

bool
CCClientSession::CheckBalanceRequest()
{
  ccrData.ServiceContextId = "opendiameter.org";
  
  ccrData.CCRequestType = 4;
  ccrData.RequestedAction = CC_ACTION_CHECK_BALANCE;
  ccrData.CCRequestNumber = 0;
  
  AAA_LOG((LM_DEBUG, "(%P|%t) \t Check Balance for %d Service Units.\n",50 ));   
  unitValue_t unitValue (50, 0);
  ccMoney_t ccMoney(unitValue,840);
  requestedServiceUnit_t requestedServiceUnits(ccMoney);

  ccrData.RequestedServiceUnit = requestedServiceUnits;  
 
  DiameterCCClientSession::CheckBalanceRequest(); 

  return true;
}

bool
CCClientSession::CheckBalanceAnswer()
{
  diameter_unsigned32_t& checkBalanceResult = ccaData.CheckBalanceResult();
  if(checkBalanceResult == 0)
  AAA_LOG((LM_DEBUG, "(%P|%t) \tEnough Credit.\n"));
  else if(checkBalanceResult == 1)
  AAA_LOG((LM_DEBUG, "(%P|%t) \tNo Credit.\n"));

  return true;  
}

void 
CCClientSession::SetDestinationHost
(DiameterScholarAttribute<diameter_identity_t> &dHost)
{
  dHost.Set(std::string("localhost"));
}

void
CCClientSession::SetDestinationRealm
(DiameterScholarAttribute<diameter_utf8string_t> &realm)
{
  realm.Set(std::string("localdomain"));  
}

class DiameterCCClientApplication : public AAA_JobData
{
 public:
  DiameterCCClientApplication(CCTask &task, DiameterCCApplication& diameterCCApplication,
                              ACE_Semaphore &sem)
    : handle(CCJobHandle
             (AAA_GroupedJob::Create(diameterCCApplication.GetTask().Job(), this, "DiameterCCClientApplication"))),
      ccClientSession(boost::shared_ptr<CCClientSession>
                      (new CCClientSession(diameterCCApplication, handle))),
      semaphore(sem)
  {
    semaphore.acquire();
  }
  ~DiameterCCClientApplication() {}

  /// if algorithm is 0 use PAP, otherwise use CHAP.
  void Start()
  { 
    ccClientSession->Start();     
  }
  void SendInitialRequest(subscriptionId_t& subscriptionId)
  {
    ccClientSession->SendInitialRequest(subscriptionId);
  }

  void SendDirectDebitingRequest(subscriptionId_t& subscriptionId)
  {
    ccClientSession->SendDirectDebitingRequest(subscriptionId);
  }

  void SendRefundAccountRequest(subscriptionId_t& subscriptionId)
  {
    ccClientSession->SendRefundAccountRequest(subscriptionId);
  }

  void SendCheckBalanceRequest(subscriptionId_t& subscriptionId)
  {
    ccClientSession->SendCheckBalanceRequest(subscriptionId);
  }
  
  CCClientSession& getCCClientSession() { return *ccClientSession; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  CCJobHandle handle;
  boost::shared_ptr<CCClientSession> ccClientSession;
  ACE_Semaphore &semaphore;
};

class CCInitializer
{
 public:
  CCInitializer(CCTask &t, DiameterCCApplication &ccApplication) 
    : task(t), diameterCCApplication(ccApplication)
  {
    Start();
  }

  ~CCInitializer() 
  {
    Stop();
  }

 private:

  void Start()
  {
    InitTask();
    InitCCApplication();
  }

  void Stop()
  {
    task.Stop();
  }

  void InitTask()
  {
      try {
         task.Start(10);
      }
      catch (...) {
         ACE_ERROR((LM_ERROR, "(%P|%t) Server: Cannot start task\n"));
         exit(1);
      }
  }
  void InitCCApplication()
  {
    ACE_DEBUG((LM_DEBUG, "(%P|%t) Application starting\n"));
    if (diameterCCApplication.Open("config/client.local.xml",
                                   task) != AAA_ERR_SUCCESS)
      {
        AAA_LOG((LM_ERROR, "(%P|%t) Can't open configuraiton file."));
        exit(1);
      }
    subscriptionId_t subscriptionId;
    unitValue_t unitValue (0, 0);
    ccMoney_t ccMoney(unitValue,840);
    requestedServiceUnit_t balanceunits(ccMoney);

    subscriptionId = subscriptionId_t(0,"1"); //END_USER_E164
    AAA_LOG((LM_DEBUG, 
             "(%P|%t) Account for Subscription Id 1 setup for Continue for Credit Control Failure Handling.\n")); 
    diameterCCApplication.addAccount(subscriptionId, 
                                     balanceunits,
                                     CREDIT_CONTROL_FAILURE_HANDLING_CONTINUE);

    subscriptionId =  subscriptionId_t(0,"2"); //END_USER_E164
    AAA_LOG((LM_DEBUG, 
             "(%P|%t) Account for Subscription Id 2 setup for Terminate for Credit Control Failure Handling.\n"));
    diameterCCApplication.addAccount(subscriptionId, 
                                     balanceunits,
                                     CREDIT_CONTROL_FAILURE_HANDLING_TERMINATE);

    subscriptionId =  subscriptionId_t(0,"3"); //END_USER_E164
    AAA_LOG((LM_DEBUG, 
             "(%P|%t) Account for Subscription Id 3 setup for Continue for Credit Control Failure Handling.\n"));
    diameterCCApplication.addAccount(subscriptionId, 
                                     balanceunits,
                                     CREDIT_CONTROL_FAILURE_HANDLING_CONTINUE);
   
    
  }

  CCTask &task;
  DiameterCCApplication &diameterCCApplication;
};

int
main(int argc, char *argv[])
{
  CCTask task;
  DiameterCCApplication diameterCCApplication;
  CCInitializer initializer(task, diameterCCApplication);

  ACE_Semaphore semaphore;

  std::auto_ptr<DiameterCCClientApplication> clientApp
    = std::auto_ptr<DiameterCCClientApplication>
	(new DiameterCCClientApplication(task, diameterCCApplication, semaphore));
  clientApp->Start();
  do {
    ACE_DEBUG((LM_INFO, "(%P|%t) Waiting till this AAA has connectivity\n"));
    ACE_OS::sleep(1);
  } while (diameterCCApplication.GetNumActivePeerConnections() == 0);

  subscriptionId_t subscriptionId;
  subscriptionId = subscriptionId_t(0,"1"); //END_USER_E164

  clientApp->SendInitialRequest(subscriptionId);
  sleep (13);
  clientApp->SendDirectDebitingRequest(subscriptionId);
  sleep (10);
  clientApp->SendRefundAccountRequest(subscriptionId);
  sleep (10);
  clientApp->SendCheckBalanceRequest(subscriptionId);
  sleep (10);

  subscriptionId = subscriptionId_t(0,"2"); //END_USER_E164
  clientApp->SendInitialRequest(subscriptionId);

  sleep (28);
  subscriptionId = subscriptionId_t(0,"3"); //END_USER_E164
  clientApp->SendInitialRequest(subscriptionId);

  semaphore.acquire();

  task.Stop();
  return 0;
}

