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

/* $Id: diameter_nasreq_server_session.hxx,v 1.3 2004/06/17 21:13:34 yohba Exp $ */
/* 
   diameter_nasreq_server_session.hxx
   Server Session definition for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created April 26, 2004.
*/

#ifndef __NASREQ_SERVER_SESSION_H__
#define __NASREQ_SERVER_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_nasreq_server_fsm.hxx"
#include "diameter_nasreq_parser.hxx"

class DiameterNasreqClientSession;

/// AA-Request Message Handler
class DIAMETER_NASREQ_SERVER_EXPORTS AA_RequestHandler 
  : public AAASessionMessageHandler
{
 public:
  AA_RequestHandler(AAAApplicationCore &appCore, 
		    DiameterNasreqServerSession &s) 
    : AAASessionMessageHandler(appCore, AA_CommandCode),
      session(s)
  {}
 private:
  AAAReturnCode HandleMessage (DiameterMsg &msg);
  DiameterNasreqServerSession &session;
};

/// Diameter NASREQ Server session.  This class is defined as multiple
/// inheritance, one from AAAServerSession (defined in Diameter API)
/// and the other from DiameterNasreqServerStateMachine.
class DIAMETER_NASREQ_SERVER_EXPORTS DiameterNasreqServerSession : 
    public AAAServerSession, public DiameterNasreqServerStateMachine
{
 public:

  /// Constuctor.
  DiameterNasreqServerSession
  (AAAApplicationCore &appCore, 
   diameter_unsigned32_t appId=NasreqApplicationId);

  /// Destructor.
  ~DiameterNasreqServerSession() {}

  /// Returns the pointer to myself.

  DiameterNasreqServerSession* Self() { return this; }

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
    DiameterNasreqServerStateMachine::Start();
  }

 protected:
 private:

  AA_RequestHandler requestHandler;
};

typedef DIAMETER_NASREQ_SERVER_EXPORTS 
AAAServerSessionClassFactory<DiameterNasreqServerSession>
DiameterNasreqServerFactory;

#endif
