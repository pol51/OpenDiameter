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
  File: diameter_cc_server_fsm.cxx
  Author: Amrit Kaur (kaur_amrit@hotmail.com)
*/

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_cc_server_session.h"
#include "diameter_cc_server_fsm.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_account.h"
#include "diameter_cc_application.h"

class DiameterCCServerAction 
  : public AAA_Action<DiameterCCServerStateMachine>
{
  virtual void operator()(DiameterCCServerStateMachine&)=0;
protected:
  DiameterCCServerAction() {}
  ~DiameterCCServerAction() {}
};

/// State table used by DiameterCCServerStateMachine.
class DiameterCCServerStateTable_S 
  : public AAA_StateTable<DiameterCCServerStateMachine>
{
  friend class 
  ACE_Singleton<DiameterCCServerStateTable_S, ACE_Recursive_Thread_Mutex>;

private:
  class AcValidateInitialRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Initial Request.\n"));

      if(sm.ValidateInitialRequest())
        {
          AAA_LOG((LM_DEBUG, "(%P|%t) \tValid Initial Request.\n"));
          sm.Event(DiameterCCServerStateMachine::EvValidInitialRequest);
        }
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidInitialRequest);
    }
  };

  class AcInitialRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Processing Initial Request.\n"));

      if (sm.InitialRequest())
        sm.Event(DiameterCCServerStateMachine::EvInitialRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvInitialRequestUnsuccessful);
    }
  };

  class AcSuccessfulInitialAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Initial Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();
      if(sm.InitialAnswer())
        {
          ccaData.CCRequestType = ccrData.CCRequestType;
          ccaData.CCRequestNumber = ccrData.CCRequestNumber;
          ccaData.ResultCode = AAA_SUCCESS;
          sm.SendCCA();
          sm.ScheduleTimer(DiameterCCServerStateMachine::EvTccExpired,
                           15,
                           0,
                           DiameterCCServerStateMachine::EvTccExpired); 
        }
    }
  };

  class AcUnsuccessfulInitialAnswer : public DiameterCCServerAction
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Initial Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      ccaData.ResultCode = 4012; //DIAMETER_CREDIT_LIMIT_REACHED
      sm.SendCCA(); 
    }
  };

  class AcValidateUpdateRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Update Request.\n"));
      if(sm.ValidateUpdateRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidUpdateRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidUpdateRequest);
    }
  };

  class AcUpdateRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Update Request.\n"));
      if (sm.UpdateRequest())
        sm.Event(DiameterCCServerStateMachine::EvUpdateRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvUpdateRequestUnsuccessful);
    }
  };

  class AcSuccessfulUpdateAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Update Answer.\n"));

      sm.SendCCA(); 
      sm.ScheduleTimer(DiameterCCServerStateMachine::EvTccExpired,
                       15,
                       0,
                       DiameterCCServerStateMachine::EvTccExpired); 
    }
  };

  class AcUnsuccessfulUpdateAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Update Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcValidateTerminationRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Termination Request.\n"));
      if(sm.ValidateTerminationRequest())
        {
          AAA_LOG((LM_DEBUG, "(%P|%t) \tValid Termination Request.\n"));
          sm.Event(DiameterCCServerStateMachine::EvValidTerminationRequest);
        }
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidTerminationRequest);
    }
  };

  class AcTerminationRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Processing Termination Request.\n"));
      if (sm.TerminationRequest())
        {
          sm.Event(DiameterCCServerStateMachine::EvTerminationRequestSuccessful);
          sm.CancelTimer(DiameterCCServerStateMachine::EvTccExpired);
        }
      else
        sm.Event(DiameterCCServerStateMachine::EvTerminationRequestUnsuccessful);
    }
  };

  class AcSuccessfulTerminationAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Termination Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();

      ccaData.CCRequestType = ccrData.CCRequestType;
      ccaData.CCRequestNumber = ccrData.CCRequestNumber;
      ccaData.ResultCode = AAA_SUCCESS;
      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulTerminationAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Termination Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcValidateDirectDebitingRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Direct Debiting Request.\n"));
      if(sm.ValidateDirectDebitingRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidDirectDebitingRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidDirectDebitingRequest);
    }
  };

  class AcDirectDebitingRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Direct Debiting Request.\n"));

      if (sm.DirectDebitingRequest())
        sm.Event(DiameterCCServerStateMachine::EvDirectDebitingRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvDirectDebitingRequestUnsuccessful);
    }
  };

  class AcSuccessfulDirectDebitingAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Direct Debiting Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();
            
      ccaData.CCRequestType = ccrData.CCRequestType;
      ccaData.CCRequestNumber = ccrData.CCRequestNumber;
      ccaData.ResultCode = AAA_SUCCESS;
      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulDirectDebitingAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Direct Debiting Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcValidateRefundAccountRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Refund Account Request.\n"));
      if(sm.ValidateRefundAccountRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidRefundAccountRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidRefundAccountRequest);
    }
  };

  class AcRefundAccountRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Refund Account Request.\n"));

      if (sm.RefundAccountRequest())
        sm.Event(DiameterCCServerStateMachine::EvRefundAccountRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvRefundAccountRequestUnsuccessful);
    }
  };

  class AcSuccessfulRefundAccountAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Refund Account Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();
      ccaData.CCRequestType = ccrData.CCRequestType;
      ccaData.CCRequestNumber = ccrData.CCRequestNumber;
      ccaData.ResultCode = AAA_SUCCESS;
      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulRefundAccountAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Refund Account Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcValidateCheckBalanceRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Check Balance Request.\n"));
      if(sm.ValidateCheckBalanceRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidCheckBalanceRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidCheckBalanceRequest);
    }
  };

  class AcCheckBalanceRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Check Balance Request.\n"));

      if (sm.CheckBalanceRequest())
        sm.Event(DiameterCCServerStateMachine::EvCheckBalanceRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvCheckBalanceRequestUnsuccessful);
    }
  };

  class AcSuccessfulCheckBalanceAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Check Balance Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();
      
      ccaData.CCRequestType = ccrData.CCRequestType;
      ccaData.CCRequestNumber = ccrData.CCRequestNumber;
      ccaData.CheckBalanceResult = 0; //ENOUGH_CREDIT
      ccaData.ResultCode = AAA_SUCCESS;
      sm.SendCCA(); 

    }
  };

  class AcUnsuccessfulCheckBalanceAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Check Balance Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      CCR_Data& ccrData = sm.CCR_DATA();

      ccaData.CCRequestType = ccrData.CCRequestType;
      ccaData.CCRequestNumber = ccrData.CCRequestNumber;
      ccaData.CheckBalanceResult = 1; //NO_CREDIT
      ccaData.ResultCode = AAA_SUCCESS;
      sm.SendCCA(); 
      
    }
  };

  class AcValidatePriceEnquiryRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Price Enquiry Request.\n"));
      if(sm.ValidatePriceEnquiryRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidPriceEnquiryRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidPriceEnquiryRequest);
    }
  };

  class AcPriceEnquiryRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Price Enquiry Request.\n"));

      if (sm.PriceEnquiryRequest())
        sm.Event(DiameterCCServerStateMachine::EvPriceEnquiryRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvPriceEnquiryRequestUnsuccessful);
    }
  };

  class AcSuccessfulPriceEnquiryAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Price Enquiry Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulPriceEnquiryAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Price Enquiry Answer.\n"));

      sm.SendCCA(); 
    }
  };

  class AcTccExpired : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Tcc Timer Expired.\n"));
      sm.CancelTimer(DiameterCCServerStateMachine::EvTccExpired);
    }
  };

  enum state {
    StIdle,
    StIdleValidateRequest,
    StInitialRequest,
    StEventRequest,
    StOpen,
    StOpenValidateRequest,
    StUpdateRequest,
    StTerminationRequest,
    StTerminated
  };


  AcValidateInitialRequest acValidateInitialRequest;
  AcInitialRequest acInitialRequest;
  AcSuccessfulInitialAnswer acSuccessfulInitialAnswer;
  AcUnsuccessfulInitialAnswer acUnsuccessfulInitialAnswer;

  AcValidateUpdateRequest acValidateUpdateRequest;
  AcUpdateRequest acUpdateRequest;
  AcSuccessfulUpdateAnswer acSuccessfulUpdateAnswer;
  AcUnsuccessfulUpdateAnswer acUnsuccessfulUpdateAnswer;

  AcValidateTerminationRequest acValidateTerminationRequest;
  AcTerminationRequest acTerminationRequest;
  AcSuccessfulTerminationAnswer acSuccessfulTerminationAnswer;
  AcUnsuccessfulTerminationAnswer acUnsuccessfulTerminationAnswer;

  AcValidateDirectDebitingRequest acValidateDirectDebitingRequest;
  AcDirectDebitingRequest acDirectDebitingRequest;
  AcSuccessfulDirectDebitingAnswer acSuccessfulDirectDebitingAnswer;
  AcUnsuccessfulDirectDebitingAnswer acUnsuccessfulDirectDebitingAnswer;

  AcValidateRefundAccountRequest acValidateRefundAccountRequest;
  AcRefundAccountRequest acRefundAccountRequest;
  AcSuccessfulRefundAccountAnswer acSuccessfulRefundAccountAnswer;
  AcUnsuccessfulRefundAccountAnswer acUnsuccessfulRefundAccountAnswer;

  AcValidateCheckBalanceRequest acValidateCheckBalanceRequest;
  AcCheckBalanceRequest acCheckBalanceRequest;
  AcSuccessfulCheckBalanceAnswer acSuccessfulCheckBalanceAnswer;
  AcUnsuccessfulCheckBalanceAnswer acUnsuccessfulCheckBalanceAnswer;

  AcValidatePriceEnquiryRequest acValidatePriceEnquiryRequest;
  AcPriceEnquiryRequest acPriceEnquiryRequest;
  AcSuccessfulPriceEnquiryAnswer acSuccessfulPriceEnquiryAnswer;
  AcUnsuccessfulPriceEnquiryAnswer acUnsuccessfulPriceEnquiryAnswer;

  AcTccExpired acTccExpired;


  // Defined as a leaf class
  DiameterCCServerStateTable_S() 
  {
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvInitialRequest,
                       StIdleValidateRequest, acValidateInitialRequest);
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvDirectDebitingRequest, 
                       StIdleValidateRequest, acValidateDirectDebitingRequest);
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvRefundAccountRequest, 
                       StIdleValidateRequest, acValidateRefundAccountRequest);
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvCheckBalanceRequest, 
                       StIdleValidateRequest, acValidateCheckBalanceRequest);
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvPriceEnquiryRequest, 
                       StIdleValidateRequest, acValidatePriceEnquiryRequest);
    AddWildcardStateTableEntry(StIdle, StTerminated);


    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidInitialRequest, 
                       StInitialRequest, acInitialRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidInitialRequest, 
                       StIdle, acUnsuccessfulInitialAnswer);

    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidDirectDebitingRequest, 
                       StEventRequest, acDirectDebitingRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidDirectDebitingRequest, 
                       StIdle, acUnsuccessfulDirectDebitingAnswer);

    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidRefundAccountRequest, 
                       StEventRequest, acRefundAccountRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidRefundAccountRequest, 
                       StIdle, acUnsuccessfulRefundAccountAnswer);

    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidCheckBalanceRequest, 
                       StEventRequest, acCheckBalanceRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidCheckBalanceRequest, 
                       StIdle, acUnsuccessfulCheckBalanceAnswer);

    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidPriceEnquiryRequest, 
                       StEventRequest, acPriceEnquiryRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidPriceEnquiryRequest, 
                       StIdle, acUnsuccessfulPriceEnquiryAnswer);
    AddWildcardStateTableEntry(StIdleValidateRequest, StTerminated);


    AddStateTableEntry(StInitialRequest,
                       DiameterCCServerStateMachine::EvInitialRequestSuccessful, 
                       StOpen, acSuccessfulInitialAnswer);
    AddStateTableEntry(StInitialRequest,
                       DiameterCCServerStateMachine::EvInitialRequestUnsuccessful, 
                       StIdle, acUnsuccessfulInitialAnswer);
    AddWildcardStateTableEntry(StInitialRequest, StTerminated);


    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvDirectDebitingRequestSuccessful, 
                       StIdle, acSuccessfulDirectDebitingAnswer);
    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvDirectDebitingRequestUnsuccessful, 
                       StIdle, acUnsuccessfulDirectDebitingAnswer);

    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvRefundAccountRequestSuccessful, 
                       StIdle, acSuccessfulRefundAccountAnswer);
    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvRefundAccountRequestUnsuccessful, 
                       StIdle, acUnsuccessfulRefundAccountAnswer);

    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvCheckBalanceRequestSuccessful, 
                       StIdle, acSuccessfulCheckBalanceAnswer);
    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvCheckBalanceRequestUnsuccessful, 
                       StIdle, acUnsuccessfulCheckBalanceAnswer);

    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvPriceEnquiryRequestSuccessful, 
                       StIdle, acSuccessfulPriceEnquiryAnswer);
    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvPriceEnquiryRequestUnsuccessful, 
                       StIdle, acUnsuccessfulPriceEnquiryAnswer);
    AddWildcardStateTableEntry(StEventRequest, StTerminated);


    AddStateTableEntry(StOpen, 
                       DiameterCCServerStateMachine::EvUpdateRequest,
                       StOpenValidateRequest, acValidateUpdateRequest);
    AddStateTableEntry(StOpen,
                       DiameterCCServerStateMachine::EvTerminationRequest,
                       StOpenValidateRequest, acValidateTerminationRequest);
    AddStateTableEntry(StOpen,
                       DiameterCCServerStateMachine::EvTccExpired,
                       StIdle, acTccExpired);
    AddWildcardStateTableEntry(StOpen, StTerminated);


    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvValidUpdateRequest,
                       StUpdateRequest, acUpdateRequest);
    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidUpdateRequest,
                       StIdle, acUnsuccessfulUpdateAnswer);

    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvValidTerminationRequest,
                       StTerminationRequest, acTerminationRequest);
    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidTerminationRequest,
                       StIdle, acUnsuccessfulTerminationAnswer);
    AddWildcardStateTableEntry(StOpenValidateRequest, StTerminated);


    AddStateTableEntry(StUpdateRequest, 
                       DiameterCCServerStateMachine::EvUpdateRequestSuccessful,
                       StOpen, acSuccessfulUpdateAnswer);
    AddStateTableEntry(StUpdateRequest, 
                       DiameterCCServerStateMachine::EvUpdateRequestUnsuccessful,
                       StIdle, acUnsuccessfulUpdateAnswer);
    AddWildcardStateTableEntry(StUpdateRequest, StTerminated);


    AddStateTableEntry(StTerminationRequest, 
                       DiameterCCServerStateMachine::EvTerminationRequestSuccessful,
                       StIdle, acSuccessfulTerminationAnswer);
    AddStateTableEntry(StTerminationRequest, 
                       DiameterCCServerStateMachine::EvTerminationRequestUnsuccessful,
                       StIdle, acUnsuccessfulTerminationAnswer);
    AddWildcardStateTableEntry(StTerminationRequest, StTerminated);


    AddWildcardStateTableEntry(StTerminated, StTerminated);


    InitialState(StIdle);
  }
  ~DiameterCCServerStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterCCServerStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterCCServerStateTable;

DiameterCCServerStateMachine::DiameterCCServerStateMachine
(DiameterCCServerSession& s, DiameterCCJobHandle &h, ACE_Reactor &reactor)
  : AAA_StateMachineWithTimer<DiameterCCServerStateMachine>
(*this, *DiameterCCServerStateTable::instance(), 
 reactor, "AAA_CC_SERVER"),
    session(s),
    handle(h)
{
}

void 
DiameterCCServerStateMachine::SendCCA(){
  DiameterMsg msg;

  ccaData.AuthApplicationId = CCApplicationId;

  CCA_Parser parser;
  parser.setAppData(&ccaData);
  parser.setRawData(&msg);

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "(%P|%t) Parsing error.\n"));
    return;
  }

  AAAMessageControl msgControl(Session().Self());
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG((LM_ERROR, "(%P|%t) Failed sending message.\n"));
  }
  else {
    AAA_LOG((LM_DEBUG, "(%P|%t) \tSent CC-Answer Message.\n"));
  }
}

