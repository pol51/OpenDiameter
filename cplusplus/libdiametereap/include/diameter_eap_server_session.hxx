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

/* $Id: diameter_eap_server_session.hxx,v 1.5 2004/06/17 19:10:42 yohba Exp $ */
/* 
   diameter_eap_server_session.hxx
   Server Session definition for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 16, 2003.
*/

#ifndef __EAP_SERVER_SESSION_H__
#define __EAP_SERVER_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_eap_server_fsm.hxx"
#include "diameter_eap_parser.hxx"

class DiameterEapClientSession;

/// DER Message Handler
class DIAMETER_EAP_SERVER_EXPORTS DER_Handler : public AAASessionMessageHandler
{
 public:
  DER_Handler(AAAApplicationCore &appCore, DiameterEapServerSession &s) 
    : AAASessionMessageHandler(appCore, EapCommandCode),
      session(s)
  {}
 private:
  AAAReturnCode HandleMessage (DiameterMsg &msg);
  DiameterEapServerSession &session;
};

/// Diameter EAP Server session.  This class is defined as multiple
/// inheritance, one from AAAServerSession (defined in Diameter API)
/// and the other from DiameterEapServerStateMachine.
class DIAMETER_EAP_SERVER_EXPORTS DiameterEapServerSession : 
    public AAAServerSession, public DiameterEapServerStateMachine
{
 public:

  /// Constuctor.
  DiameterEapServerSession
  (AAAApplicationCore &appCore, 
   diameter_unsigned32_t appId=EapApplicationId);

  /// Destructor.
  ~DiameterEapServerSession() {}

  /// Returns the pointer to myself.

  DiameterEapServerSession* Self() { return this; }

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

  void Start() throw (AAA_Error)
  {
    DiameterEapServerStateMachine::Start();
  }

 protected:
 private:

  DER_Handler requestHandler;
};

typedef DIAMETER_EAP_SERVER_EXPORTS 
AAAServerSessionClassFactory<DiameterEapServerSession>
DiameterEapServerFactory;

#endif
