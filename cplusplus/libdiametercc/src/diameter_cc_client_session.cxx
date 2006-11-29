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

#include "diameter_parser.h"
#include "diameter_cc_client_session.h"

DiameterCCClientSession::DiameterCCClientSession
(AAAApplicationCore &appCore, DiameterJobHandle &h)
  : AAAClientSession(appCore, CCApplicationId),
    DiameterCCClientStateMachine(*this, h),
    ccaHandler(CCA_Handler(appCore, *this))
{
  // Register the DEA message handler
  if (RegisterMessageHandler(&ccaHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG((LM_ERROR, "[%N] CCA Handler registration failed.\n"));
      throw -1; 
    }
}

AAAReturnCode
DiameterCCClientSession::HandleMessage(DiameterMsg &msg)
{
  AAA_LOG((LM_ERROR, "[%N] Unknown command.\n"));
  return AAA_ERR_UNKNOWN_CMD;

}

AAAReturnCode
DiameterCCClientSession::HandleDisconnect() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session termination event received.\n"));
  Notify(DiameterCCClientStateMachine::EvSgDisconnect);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCClientSession::HandleSessionTimeout() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
  Notify(DiameterCCClientStateMachine::EvSgSessionTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCClientSession::HandleAuthLifetimeTimeout()
{ 
  AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
  Notify(DiameterCCClientStateMachine::EvSgAuthLifetimeTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterCCClientSession::HandleTimeout() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
  Notify(DiameterCCClientStateMachine::EvSgTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
CCA_Handler::HandleMessage (DiameterMsg &msg)
{
  // Header flag check.
  if (msg.hdr.flags.r)
    {
      AAA_LOG((LM_ERROR, "[%N] Received CCR instead of CCA.\n"));
      return AAA_ERR_UNKNOWN_CMD;
    }

  // Parse the received message.
  CCA_Parser parser;
  parser.setAppData(&session.CCA_DATA());
  parser.setRawData(&msg);

  try {
    parser.parseRawToApp();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "[%N] Parsing error.\n"));
    return AAA_ERR_PARSING_ERROR;
  }

  if(session.CCA_DATA().CCRequestType.IsSet())
    {
      if (session.CCA_DATA().CCRequestType == CCR_TYPE_INITIAL_REQUEST)
        {
          session.Notify(DiameterCCClientStateMachine::EvInitialAnswer);
        }
      else if(session.CCA_DATA().CCRequestType == CCR_TYPE_UPDATE_REQUEST)
        {
            session.Notify(DiameterCCClientStateMachine::EvUpdateAnswer);
        }
      else if (session.CCA_DATA().CCRequestType == CCR_TYPE_TERMINATION_REQUEST)
        {
            session.Notify(DiameterCCClientStateMachine::EvTerminationAnswer);
        }
      else if (session.CCA_DATA().CCRequestType == CCR_TYPE_EVENT_REQUEST)
        {
            session.Notify(DiameterCCClientStateMachine::EvEventAnswer);
        }
    }
  return AAA_ERR_SUCCESS;
}

