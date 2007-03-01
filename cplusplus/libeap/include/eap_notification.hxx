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
// $Id: eap_notification.hxx,v 1.15 2004/06/17 21:13:36 yohba Exp $

// eap_notification.hxx:  Notification method state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_NOTIFICATION_HXX__
#define __EAP_NOTIFICATION_HXX__

/*  Notification Request/Response

    According to RFC2284bis, there are the following rules on
    processing Notification Request/Response messages.

    Section 2.1. Support for sequences:

      A Notification Response is only used as confirmation that the
      peer received the Notification Request, not that it has
      processed it, or displayed the message to the user. It cannot be
      assumed that the contents of the Notification Request or
      Response is available to another method.

      Given these considerations, the Success, Failure, Nak Response
      and Notification Request/Response messages MUST NOT used to
      carry data destined for delivery to other EAP methods.

    Section 4.2.1.  Processing of success and failure:

      In order to provide additional protection against tampering, EAP
      methods MAY support a MIC that covers some or all of the EAP
      packet, including headers. In addition, such a MIC MAY include
      coverage of previous Request and Response messages, so as to
      enable protection of other packets to that do not contain MICs,
      such as Identity Request/Response, Notification Request/Response
      and Nak Response.

    Section 5.2.  Notification:

      Description

         The Notification Type is optionally used to convey a
         displayable message from the authenticator to the peer. An
         authenticator MAY send a Notification Request to the peer at
         any time, The peer MUST respond to a Notification Request
         with a Notification Response; a Nak Response MUST NOT be
         sent.

         The peer SHOULD display this message to the user or log it if
         it cannot be displayed. The Notification Type is intended to
         provide an acknowledged notification of some imperative
         nature, but it is not an error indication, and therefore does
         not change the state of the peer. Examples include a password
         with an expiration time that is about to expire, an OTP
         sequence integer which is nearing 0, an authentication
         failure warning, etc. In most circumstances, Notification
         should not be required.

      Type

         2

      Type-Data

         The Type-Data field in the Request contains a displayable
         message greater than zero octets in length, containing UTF-8
         encoded ISO 10646 characters [RFC2279].  The length of the
         message is determined by Length field of the Request packet.
         The message MUST NOT be null terminated.  A Response MUST be
         sent in reply to the Request with a Type field of 2
         (Notification).  The Type-Data field of the Response is zero
         octets in length.  The Response should be sent immediately
         (independent of how the message is displayed or logged).
 */

#include "eap_fsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_peerfsm.hxx"
#include "eap_log.hxx"
#include "eap_method_registrar.hxx"

/// Authenticator application can directly use this class for sending
/// Notification Request and receiving Notification Response messages.
/// An application is not allowed to send Notification Request any
/// time.  Instead, a policy element needs to be inserted to the
/// policy tree, meaning that sending Notification Request message is
/// allowed between the end of a method and the beggining of another
/// method.
class EAP_EXPORTS EapAuthNotificationStateMachine :
  public EapMethodStateMachine,
  public EapStateMachine<EapAuthNotificationStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthNotificationStateMachine>;
  friend class EapAuthNotificationStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    EapStateMachine<EapAuthNotificationStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthNotificationStateMachine>::Notify(ev);
  }

  void Receive(AAAMessageBlock *b) {}
  EapAuthNotificationStateMachine(EapSwitchStateMachine &s);
  ~EapAuthNotificationStateMachine() {} 
private:

};

#endif // __EAP_NOTIFICATION_HXX__
