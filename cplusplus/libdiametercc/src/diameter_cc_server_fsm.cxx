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
// $Id: diameter_cc_server_fsm.cxx $

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

  class AcValidateEventRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Validating Event Request.\n"));
      if(sm.ValidateEventRequest())
        sm.Event(DiameterCCServerStateMachine::EvValidEventRequest);
      else
        sm.Event(DiameterCCServerStateMachine::EvInvalidEventRequest);
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

  class AcTerminationRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Processing Termination Request.\n"));
      if (sm.TerminationRequest())
        sm.Event(DiameterCCServerStateMachine::EvTerminationRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvTerminationRequestUnsuccessful);
    }
  };

  class AcEventRequest : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "(%P|%t) Event Request.\n"));

      if (sm.EventRequest())
        sm.Event(DiameterCCServerStateMachine::EvEventRequestSuccessful);
      else
        sm.Event(DiameterCCServerStateMachine::EvEventRequestUnsuccessful);
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

  class AcSuccessfulUpdateAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Update Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Update Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      sm.SendCCA(); 
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

      CCA_Data& ccaData = sm.CCA_DATA();
      sm.SendCCA(); 
    }
  };

  class AcSuccessfulEventAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Successful Event Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      sm.SendCCA(); 
    }
  };

  class AcUnsuccessfulEventAnswer : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Sending Unsuccessful Event Answer.\n"));

      CCA_Data& ccaData = sm.CCA_DATA();
      sm.SendCCA(); 
    }
  };


  class AcTccTimerExpired : public DiameterCCServerAction 
  {
    void operator()(DiameterCCServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
               "(%P|%t) Tcc Timer Expired.\n"));
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
  AcValidateEventRequest acValidateEventRequest;
  
  AcInitialRequest acInitialRequest;
  AcUnsuccessfulAnswer acUnsuccessfulAnswer;
  AcEventRequest acEventRequest;
  AcSuccessfulInitialAnswer acSuccessfulInitialAnswer;
  AcUnsuccessfulInitialAnswer acUnsuccessfulInitialAnswer;
  AcSuccessfulUpdateAnswer acSuccessfulUpdateAnswer;
  AcUnsuccessfulInitialAnswer acUnsuccessfulUpdateAnswer;
  AcSuccessfulTerminationAnswer acSuccessfulTerminationAnswer;
  AcUnsuccessfulTerminationAnswer acUnsuccessfulTerminationAnswer;
  AcSuccessfulEventAnswer acSuccessfulEventAnswer;
  AcUnsuccessfulEventAnswer acUnsuccessfulEventAnswer;

  AcValidateUpdateRequest acValidateUpdateRequest;
  AcValidateTerminationRequest acValidateTerminationRequest;
  AcTccTimerExpired acTccTimerExpired;

  AcTerminationRequest acTerminationRequest;
  AcUpdateRequest acUpdateRequest;

  // Defined as a leaf class
  DiameterCCServerStateTable_S() 
  {
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvInitialRequest,
                       StIdleValidateRequest, acValidateInitialRequest);
    AddStateTableEntry(StIdle, 
                       DiameterCCServerStateMachine::EvEventRequest, 
                       StIdleValidateRequest, acValidateEventRequest);
    AddWildcardStateTableEntry(StIdle, StTerminated);


    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidInitialRequest, 
                       StInitialRequest, acInitialRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidInitialRequest, 
                       StIdle, acUnsuccessfulInitialAnswer);
    AddStateTableEntry(StIdleValidateRequest, 
                       DiameterCCServerStateMachine::EvValidEventRequest, 
                       StEventRequest, acEventRequest);
    AddStateTableEntry(StIdleValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidEventRequest, 
                       StIdle, acUnsuccessfulEventAnswer);
    AddWildcardStateTableEntry(StIdleValidateRequest, StTerminated);


    AddStateTableEntry(StInitialRequest,
                       DiameterCCServerStateMachine::EvInitialRequestSuccessful, 
                       StOpen, acSuccessfulInitialAnswer);
    AddStateTableEntry(StInitialRequest,
                       DiameterCCServerStateMachine::EvInitialRequestUnsuccessful, 
                       StIdle, acUnsuccessfulInitialAnswer);
    AddWildcardStateTableEntry(StInitialRequest, StTerminated);

    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvEventRequestSuccessful, 
                       StIdle, acSuccessfulEventAnswer);
    AddStateTableEntry(StEventRequest,
                       DiameterCCServerStateMachine::EvEventRequestUnsuccessful, 
                       StIdle, acUnsuccessfulEventAnswer);
    AddWildcardStateTableEntry(StEventRequest, StTerminated);


    AddStateTableEntry(StOpen, 
                       DiameterCCServerStateMachine::EvUpdateRequest,
                       StOpenValidateRequest, acValidateUpdateRequest);
    AddStateTableEntry(StOpen,
                       DiameterCCServerStateMachine::EvTerminationRequest,
                       StOpenValidateRequest, acValidateTerminationRequest);
    AddStateTableEntry(StOpen,
                       DiameterCCServerStateMachine::EvTccExpired,
                       StIdle, acTccTimerExpired);
    AddWildcardStateTableEntry(StOpen, StTerminated);

    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvValidUpdateRequest,
                       StUpdateRequest, acUpdateRequest);
    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidUpdateRequest,
                       StIdle, acUnsuccessfulAnswer);
    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvValidTerminationRequest,
                       StTerminationRequest, acTerminationRequest);
    AddStateTableEntry(StOpenValidateRequest,
                       DiameterCCServerStateMachine::EvInvalidTerminationRequest,
                       StIdle, acUnsuccessfulAnswer);
    AddWildcardStateTableEntry(StOpenValidateRequest, StTerminated);

    AddStateTableEntry(StUpdateRequest, 
                       DiameterCCServerStateMachine::EvUpdateRequestSuccessful,
                       StOpen, acSuccessfulUpdateAnswer);
    AddStateTableEntry(StUpdateRequest, 
                       DiameterCCServerStateMachine::EvUpdateRequestUnsuccessful,
                       StIdle, acUnsuccessfulAnswer);
    AddWildcardStateTableEntry(StUpdateRequest, StTerminated);

    AddStateTableEntry(StTerminationRequest, 
                       DiameterCCServerStateMachine::EvTerminationRequestSuccessful,
                       StIdle, acSuccessfulTerminationAnswer);
    AddStateTableEntry(StTerminationRequest, 
                       DiameterCCServerStateMachine::EvTerminationRequestUnsuccessful,
                       StIdle, acUnsuccessfulAnswer);
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
(DiameterCCServerSession& s, DiameterCCJobHandle &h)
  : AAA_StateMachine<DiameterCCServerStateMachine>
  (*this, *DiameterCCServerStateTable::instance(), 
   "AAA_CC_SERVER"),
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

