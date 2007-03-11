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

/* $Id: diameter_cc_server_session.cxx $ */

#include "diameter_parser.h"
#include "diameter_cc_server_session.h"
#include "diameter_cc_server_fsm.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_application.h"

DiameterCCServerSession::DiameterCCServerSession
  (DiameterCCApplication &ccApplication, diameter_unsigned32_t appId)
    : AAAServerSession(ccApplication, appId),
      DiameterCCServerStateMachine(*this, 
                                   ccApplication.GetTask().JobHandle(),
                                   *(ccApplication.GetTask().reactor())),
      diameterCCApplication(ccApplication),
      requestHandler(CCR_Handler(ccApplication, *this))

{
  // Register the AA-Answer message handler
  if (RegisterMessageHandler(&requestHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG((LM_ERROR, "(%P|%t) CCR_Handler registration failed.\n"));
      throw -1; 
    }
  ACE_DEBUG((LM_INFO, "(%P|%t) CCR_Handler registration succeeded.\n"));
}

AAAReturnCode
DiameterCCServerSession::HandleMessage(DiameterMsg &msg)
{
  AAA_LOG((LM_ERROR, "(%P|%t) Unknown command.\n"));
  return AAA_ERR_UNKNOWN_CMD;
}

AAAReturnCode
DiameterCCServerSession::HandleDisconnect() 
{ 
  AAA_LOG((LM_ERROR, "(%P|%t) Session termination event received.\n"));
  Notify(DiameterCCServerStateMachine::EvSgDisconnect);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCServerSession::HandleSessionTimeout() 
{ 
  AAA_LOG((LM_ERROR, "(%P|%t) Session timeout received.\n"));
  Notify(DiameterCCServerStateMachine::EvSgSessionTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCServerSession::HandleAuthLifetimeTimeout()
{ 
  AAA_LOG((LM_ERROR, "(%P|%t) Timeout received.\n"));
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCServerSession::HandleAuthGracePeriodTimeout()
{ 
  AAA_LOG((LM_ERROR, "(%P|%t) Timeout received.\n"));
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCServerSession::HandleTimeout() 
{ 
  AAA_LOG((LM_ERROR, "(%P|%t) General timeout received.\n"));
  Notify(DiameterCCServerStateMachine::EvSgTimeout);
  return AAA_ERR_SUCCESS; 
}

void DiameterCCServerSession::Start() throw (AAA_Error)
{
  DiameterCCServerStateMachine::Start();
}

DiameterCCApplication& DiameterCCServerSession:: DiameterCCApp()
{
  return diameterCCApplication;
}

AAAReturnCode 
CCR_Handler::HandleMessage (DiameterMsg &msg)
{
  // Header flag check.
  if (!msg.hdr.flags.r)
    {
      AAA_LOG((LM_ERROR, "(%P|%t) Received CCA instead of CCR.\n"));
      return AAA_ERR_UNKNOWN_CMD;
    }

  // Parse the received message.
  CCR_Parser parser;
  parser.setAppData(session.CCR_DATA().Self());
  parser.setRawData(&msg);

  try {
    parser.parseRawToApp();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "(%P|%t) Parsing error.\n"));
    return AAA_ERR_PARSING_ERROR;
  }
  
  if(session.CCR_DATA().CCRequestType.IsSet())
    {
      if (session.CCR_DATA().CCRequestType == CC_TYPE_INITIAL_REQUEST)
        {
          session.Notify(DiameterCCServerStateMachine::EvInitialRequest);
        }
      else if(session.CCR_DATA().CCRequestType == CC_TYPE_UPDATE_REQUEST)
        {
            session.Notify(DiameterCCServerStateMachine::EvUpdateRequest);
        }
      else if (session.CCR_DATA().CCRequestType == CC_TYPE_TERMINATION_REQUEST)
        {
            session.Notify(DiameterCCServerStateMachine::EvTerminationRequest);
        }
      else if (session.CCR_DATA().CCRequestType == CC_TYPE_EVENT_REQUEST)
        {
          if (session.CCR_DATA().RequestedAction == CC_ACTION_DIRECT_DEBITING)
            session.Notify(DiameterCCServerStateMachine::EvDirectDebitingRequest);
          else if (session.CCR_DATA().RequestedAction == CC_ACTION_REFUND_ACCOUNT)
            session.Notify(DiameterCCServerStateMachine::EvRefundAccountRequest);
          else if (session.CCR_DATA().RequestedAction == CC_ACTION_CHECK_BALANCE)
            session.Notify(DiameterCCServerStateMachine::EvCheckBalanceRequest);
          else if (session.CCR_DATA().RequestedAction == CC_ACTION_PRICE_ENQUIRY)
            session.Notify(DiameterCCServerStateMachine::EvPriceEnquiryRequest);
        }
    }

  return AAA_ERR_SUCCESS;
}
