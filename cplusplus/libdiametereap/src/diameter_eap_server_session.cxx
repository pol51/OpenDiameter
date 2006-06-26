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

/* $Id: diameter_eap_server_session.cxx,v 1.11 2004/06/17 19:11:00 yohba Exp $ */
/* 
   diameter_eap_server_session.cxx
   Server Session definition for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 16, 2003.
*/

#include "diameter_parser_api.h"
#include "diameter_eap_server_session.hxx"
#include "diameter_eap_server_fsm.hxx"
#include "diameter_eap_parser.hxx"

DiameterEapServerSession::DiameterEapServerSession
  (AAAApplicationCore &appCore, diameter_unsigned32_t appId)
  : AAAServerSession(appCore, appId),
    DiameterEapServerStateMachine(*this, appCore.GetTask().JobHandle()),
    requestHandler(DER_Handler(appCore, *this))
{
  // Register the DEA message handler
  if (RegisterMessageHandler(&requestHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG(LM_ERROR, "[%N] DER_Handler registration failed.\n");
      throw -1; // XXX
    }
}

AAAReturnCode
DiameterEapServerSession::HandleMessage(AAAMessage &msg)
{
  // Header flag check.
  if (!msg.hdr.flags.r)
    {
      AAA_LOG(LM_ERROR, "[%N] Received DEA instead of DER.\n");
      return AAA_ERR_UNKNOWN_CMD;
    }

  AAA_LOG(LM_ERROR, "[%N] Unknown command.\n");
  return AAA_ERR_UNKNOWN_CMD;
}

AAAReturnCode
DiameterEapServerSession::HandleDisconnect() 
{ 
  AAA_LOG(LM_ERROR, "[%N] Session termination event received.\n");
  Notify(DiameterEapServerStateMachine::EvSgDisconnect);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapServerSession::HandleSessionTimeout() 
{ 
  AAA_LOG(LM_ERROR, "[%N] Session timeout received.\n");
  Notify(DiameterEapServerStateMachine::EvSgSessionTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapServerSession::HandleAuthLifetimeTimeout()
{ 
  AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
  Notify(DiameterEapServerStateMachine::EvSgAuthLifetimeTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapServerSession::HandleAuthGracePeriodTimeout()
{ 
  AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
  Notify(DiameterEapServerStateMachine::EvSgAuthGracePeriodTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterEapServerSession::HandleTimeout() 
{ 
  AAA_LOG(LM_ERROR, "[%N] General timeout received.\n");
  Notify(DiameterEapServerStateMachine::EvSgTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DER_Handler::HandleMessage (AAAMessage &msg)
{
  // Header flag check.
  if (!msg.hdr.flags.r)
    {
      AAA_LOG(LM_ERROR, "[%N] Received DEA instead of DER.\n");
      return AAA_ERR_UNKNOWN_CMD;
    }

  // Parse the received message.
  DER_Parser parser;
  parser.setAppData(&session.DER());
  parser.setRawData(&msg);

  try {
    parser.parseRawToApp();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return AAA_ERR_PARSING_ERROR;
  }

  session.Notify(DiameterEapServerStateMachine::EvRxDER);
  return AAA_ERR_SUCCESS;
}

