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

/* $Id: diameter_eap_client_session.cxx,v 1.9 2004/06/17 19:11:00 yohba Exp $ */
/* 
   diameter_eap_client_session.cxx
   Client Session definition for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 4, 2003.
*/

#include "diameter_parser.h"
#include "diameter_eap_client_session.hxx"
#include "diameter_eap_client_fsm.hxx"
#include "diameter_eap_parser.hxx"

DiameterEapClientSession::DiameterEapClientSession
(AAAApplicationCore &appCore, DiameterJobHandle &h)
  : AAAClientSession(appCore, EapApplicationId),
    DiameterEapClientStateMachine(*this, h),
    answerHandler(DEA_Handler(appCore, *this))
{
  // Register the DEA message handler
  if (RegisterMessageHandler(&answerHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG((LM_ERROR, "[%N] DEA_Handler registration failed.\n"));
      throw -1; // XXX
    }
}

AAAReturnCode
DiameterEapClientSession::HandleMessage(DiameterMsg &msg)
{
  AAA_LOG((LM_ERROR, "[%N] Unknown command.\n"));
  return AAA_ERR_UNKNOWN_CMD;

}

AAAReturnCode
DiameterEapClientSession::HandleDisconnect() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session termination event received.\n"));
  Notify(DiameterEapClientStateMachine::EvSgDisconnect);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapClientSession::HandleSessionTimeout() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
  Notify(DiameterEapClientStateMachine::EvSgSessionTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapClientSession::HandleAuthLifetimeTimeout()
{ 
  AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
  Notify(DiameterEapClientStateMachine::EvSgAuthLifetimeTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapClientSession::HandleAuthGracePeriodTimeout()
{ 
  AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
  Notify(DiameterEapClientStateMachine::EvSgAuthGracePeriodTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapClientSession::HandleTimeout() 
{ 
  AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
  Notify(DiameterEapClientStateMachine::EvSgTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DEA_Handler::HandleMessage (DiameterMsg &msg)
{
  // Header flag check.
  if (msg.hdr.flags.r)
    {
      AAA_LOG((LM_ERROR, "[%N] Received DER instead of DEA.\n"));
      return AAA_ERR_UNKNOWN_CMD;
    }

  // Parse the received message.
  DEA_Parser parser;
  parser.setAppData(&session.DEA());
  parser.setRawData(&msg);

  try {
    parser.parseRawToApp();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "[%N] Parsing error.\n"));
    return AAA_ERR_PARSING_ERROR;
  }

  session.Notify(DiameterEapClientStateMachine::EvRxDEA);
  return AAA_ERR_SUCCESS;
}

