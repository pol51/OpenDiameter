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

/* $Id: diameter_nasreq_client_session.hxx,v 1.3 2004/06/17 21:13:34 yohba Exp $ */
/* 
   diameter_nasreq_client_session.hxx
   Client Session definition for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created May 1, 2004.
*/

#ifndef __NASREQ_CLIENT_SESSION_H__
#define __NASREQ_CLIENT_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_nasreq_client_fsm.hxx"
#include "diameter_nasreq_parser.hxx"

class DiameterNasreqClientSession;

/// AA-Answer Message Handler
class DIAMETER_NASREQ_CLIENT_EXPORTS AA_AnswerHandler : 
    public AAASessionMessageHandler
{
 public:
  AA_AnswerHandler
  (AAAApplicationCore &appCore, DiameterNasreqClientSession &s) 
    : AAASessionMessageHandler(appCore, AA_CommandCode),
      session(s)
  {}
 private:
  AAAReturnCode HandleMessage (DiameterMsg &msg);
  DiameterNasreqClientSession &session;
};

/// Client Diameter NASREQ session.  This class is defined as multiple
/// inheritance, one from AAAClientSession (defined in Diameter API)
/// and the other from DiameterNasreqClientStateMachine.
class DIAMETER_NASREQ_CLIENT_EXPORTS DiameterNasreqClientSession : 
    public AAAClientSession, public DiameterNasreqClientStateMachine
{
 public:

  /// Constuctor.
  DiameterNasreqClientSession(AAAApplicationCore&, DiameterJobHandle &h);

  /// Destructor.
  ~DiameterNasreqClientSession() {}

  DiameterNasreqClientSession* Self() { return this; }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// incomming message events. The msg argument is pre-allocated by
  /// the library and is valid only within the context of this
  /// method. It is the responsibility of the derived class to
  /// override this function and capture the events if it is
  /// interested in it.
  AAAReturnCode HandleMessage(DiameterMsg &msg);

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session disconnect event. Disconnection occurs when a session is
  /// terminated or the peer connection is disconnection and unable to
  /// recover. It is the responsibility of the derived class to
  /// override this function and capture this events if it is
  /// interested in it.
  AAAReturnCode HandleDisconnect();

  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleSessionTimeout();
                                     
  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthLifetimeTimeout();

  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthGracePeriodTimeout();

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session timeout event. Timeout occurs when a session is idle
  /// (not re- authorization) for a specified amount of time. It is
  /// the responsibility of the derived class to override this
  /// function and capture this events if it is interested in it.
  AAAReturnCode HandleTimeout();

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session abort event. Abort occurs when the server decides to
  /// terminate the client session by sending an ASR. It is the
  /// responsibility of the derived class to override this function
  /// and capture this events if it is interested in it.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  void Start() throw (AAA_Error)
  {
    DiameterNasreqClientStateMachine::Start();
    AAAClientSession::Start();
  }

 protected:
 private:

  AA_AnswerHandler answerHandler;
};

#endif
