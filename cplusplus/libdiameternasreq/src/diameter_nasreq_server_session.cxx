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

/* $Id: diameter_nasreq_server_session.cxx,v 1.5 2004/06/17 21:13:34 yohba Exp $ */
/* 
   diameter_eap_server_session.cxx
   Server Session definition for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created May 1, 2004.
*/

#include "diameter_parser_api.h"
#include "diameter_nasreq_server_session.hxx"
#include "diameter_nasreq_server_fsm.hxx"
#include "diameter_nasreq_parser.hxx"

DiameterNasreqServerSession::DiameterNasreqServerSession
  (AAAApplicationCore &appCore, diameter_unsigned32_t appId)
  : AAAServerSession(appCore, appId),
    DiameterNasreqServerStateMachine(*this, appCore.GetTask().JobHandle()),
    requestHandler(AA_RequestHandler(appCore, *this))
{
  // Register the AA-Answer message handler
  if (RegisterMessageHandler(&requestHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG(LM_ERROR, "[%N] AA_AnswerHandler registration failed.\n");
      throw -1; // XXX
    }
}

AAAReturnCode
DiameterNasreqServerSession::HandleMessage(AAAMessage &msg)
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
DiameterNasreqServerSession::HandleDisconnect() 
{ 
  AAA_LOG(LM_ERROR, "[%N] Session termination event received.\n");
  Notify(DiameterNasreqServerStateMachine::EvSgDisconnect);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterNasreqServerSession::HandleSessionTimeout() 
{ 
  AAA_LOG(LM_ERROR, "[%N] Session timeout received.\n");
  Notify(DiameterNasreqServerStateMachine::EvSgSessionTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterNasreqServerSession::HandleAuthLifetimeTimeout()
{ 
  AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
  Notify(DiameterNasreqServerStateMachine::EvSgAuthLifetimeTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterNasreqServerSession::HandleAuthGracePeriodTimeout()
{ 
  AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
  Notify(DiameterNasreqServerStateMachine::EvSgAuthGracePeriodTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
DiameterNasreqServerSession::HandleTimeout() 
{ 
  AAA_LOG(LM_ERROR, "[%N] General timeout received.\n");
  Notify(DiameterNasreqServerStateMachine::EvSgTimeout);
  return AAA_ERR_SUCCESS; 
}

AAAReturnCode 
AA_RequestHandler::HandleMessage (AAAMessage &msg)
{
  // Header flag check.
  if (!msg.hdr.flags.r)
    {
      AAA_LOG(LM_ERROR, "[%N] Received AA-Answer instead of AA-Request.\n");
      return AAA_ERR_UNKNOWN_CMD;
    }

  // Parse the received message.
  AA_RequestParser parser;
  parser.setAppData(session.AA_Request().Self());
  parser.setRawData(&msg);

  try {
    parser.parseRawToApp();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return AAA_ERR_PARSING_ERROR;
  }

  session.Notify(DiameterNasreqServerStateMachine::EvRxAA_Request);
  return AAA_ERR_SUCCESS;
}

