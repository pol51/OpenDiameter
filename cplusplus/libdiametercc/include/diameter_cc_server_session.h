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
  File: diameter_cc_server_session.h
  Author: Amrit Kaur (kaur_amrit@hotmail.com)
*/

#ifndef __CC_SERVER_SESSION_H__
#define __CC_SERVER_SESSION_H__
#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_cc_server_fsm.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_application.h"

class DiameterCCClientSession;

/// CCR-Message Handler
class DIAMETER_CC_SERVER_EXPORTS CCR_Handler 
  : public AAASessionMessageHandler
{
 public:
  CCR_Handler(AAAApplicationCore &appCore, 
              DiameterCCServerSession &s) 
    : AAASessionMessageHandler(appCore, CC_CommandCode),
      session(s)
  {
  }
 private:
  AAAReturnCode HandleMessage (DiameterMsg &msg);
  DiameterCCServerSession &session;
};

/// Diameter CC Server session.  This class is defined as multiple
/// inheritance, one from AAAServerSession (defined in Diameter API)
/// and the other from DiameterCcServerStateMachine.
class DIAMETER_CC_SERVER_EXPORTS DiameterCCServerSession : 
    public AAAServerSession, public DiameterCCServerStateMachine
{
 public:

  /// Constuctor.
  DiameterCCServerSession
  (DiameterCCApplication &diameterCCApplication, 
   diameter_unsigned32_t appId=CCApplicationId);

  /// Destructor.
  ~DiameterCCServerSession() {}

  /// Returns the pointer to myself.

  DiameterCCServerSession* Self() { return this; }

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleMessage(DiameterMsg &msg);

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleDisconnect();

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleSessionTimeout();
                                     
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthLifetimeTimeout();
                                                                                
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthGracePeriodTimeout();

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  /// Reimplemented from AAAServerSession. This is a general timeout
  /// event handler.
  AAAReturnCode HandleTimeout();
  
  void Start() throw (AAA_Error);
  
  DiameterCCApplication& DiameterCCApp();

protected:
  DiameterCCApplication diameterCCApplication;

private:

  CCR_Handler requestHandler;
};

typedef DIAMETER_CC_SERVER_EXPORTS 
AAAServerSessionClassFactory<DiameterCCServerSession>
DiameterCCServerFactory;

#endif
