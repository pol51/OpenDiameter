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
// $Id: eap_md5.hxx,v 1.13 2004/06/17 21:13:36 yohba Exp $

// eap_identity.hxx:  Identity method state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_MD5_HXX__
#define __EAP_MD5_HXX__

#include <ace/Basic_Types.h>
#include <openssl/md5.h>
#include "eap.hxx"
#include "eap_fsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_peerfsm.hxx"

/* MD5-Challenge Request/Response description.

In section 5.4 of RFC2284bis:

"5.4 MD5-Challenge                                                                                
   Description
                                                                                
      The MD5-Challenge Type is analogous to the PPP CHAP protocol
      [RFC1994] (with MD5 as the specified algorithm). The Request
      contains a "challenge" message to the peer.  A Response MUST be
      sent in reply to the Request.  The Response MAY be either of Type
      4 (MD5-Challenge), Nak (Type 3) or Expanded Nak (Type 254). The
      Nak reply indicates the peer's desired authentication Type(s).
      EAP peer and EAP server implementations MUST support the
      MD5-Challenge mechanism.  An authenticator that supports only
      pass-through MUST allow communication with a backend
      authentication server that is capable of supporting MD5-Challenge,
      although the EAP authenticator implementation need not support
      MD5-Challenge itself.  However, if the EAP authenticator can be
      configured to authenticate peers locally (e.g., not operate in
      pass-through), then the requirement for support of the
      MD5-Challenge mechanism applies.
                                                                                
      Note that the use of the Identifier field in the MD5-Challenge
      Type is different from that described in [RFC1994].  EAP allows
      for retransmission of MD5-Challenge Request packets while
      [RFC1994] states that both the Identifier and Challenge fields
      MUST change each time a Challenge (the CHAP equivalent of the
      MD5-Challenge Request packet) is sent.
                                                                                
      Note: [RFC1994] treats the shared secret as an octet string, and
      does not specify how it is entered into the system (or if it is
      handled by the user at all). EAP MD5-Challenge implementations MAY
      support entering passphrases with non-ASCII characters.  See
      Section 5 for instructions how the input should be processed and
      encoded into octets.

                                                                                
   Type
                                                                                
      4
                                                                                
   Type-Data
                                                                                
      The contents of the Type-Data  field is summarized below.  For
      reference on the use of these fields see the PPP Challenge
      Handshake Authentication Protocol [RFC1994].
                                                                                
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Value-Size   |  Value ...
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Name ...
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
"


In section 5 of RFC2284bis:

   "EAP methods MAY support authentication based on shared secrets. If
   the shared secret is a passphrase entered by the user,
   implementations MAY support entering passphrases with non-ASCII
   characters.  In this case, the input should be processed using an
   appropriate stringprep [RFC3454] profile, and encoded in octets using
   UTF-8 encoding [RFC2279].  A preliminary version of a possible
   stringprep profile is described in [SASLPREP]."
                                                                                
In section 2.3 of RFC1994:

"2.3.  Design Requirements

   The CHAP algorithm requires that the length of the secret MUST be at
   least 1 octet.  The secret SHOULD be at least as large and
   unguessable as a well-chosen password.  It is preferred that the
   secret be at least the length of the hash value for the hashing
   algorithm chosen (16 octets for MD5).  This is to ensure a
   sufficiently large range for the secret to provide protection against
   exhaustive search attacks.

   The one-way hash algorithm is chosen such that it is computationally
   infeasible to determine the secret from the known challenge and
   response values.

   Each challenge value SHOULD be unique, since repetition of a
   challenge value in conjunction with the same secret would permit an
   attacker to reply with a previously intercepted response.  Since it
   is expected that the same secret MAY be used to authenticate with
   servers in disparate geographic regions, the challenge SHOULD exhibit
   global and temporal uniqueness.

   Each challenge value SHOULD also be unpredictable, least an attacker
   trick a peer into responding to a predicted future challenge, and
   then use the response to masquerade as that peer to an authenticator.

   Although protocols such as CHAP are incapable of protecting against
   realtime active wiretapping attacks, generation of unique
   unpredictable challenges can protect against a wide range of active
   attacks.

   A discussion of sources of uniqueness and probability of divergence
   is included in the Magic-Number Configuration Option [1]."


In section 4 of RFC1994:

"4.  Packet Format

   Exactly one Challenge-Handshake Authentication Protocol packet is
   encapsulated in the Information field of a PPP Data Link Layer frame
   where the protocol field indicates type hex c223 (Challenge-Handshake
   Authentication Protocol).  A summary of the CHAP packet format is
   shown below.  The fields are transmitted from left to right.

   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Data ...
   +-+-+-+-+

   Code

      The Code field is one octet and identifies the type of CHAP
      packet.  CHAP Codes are assigned as follows:

         1       Challenge
         2       Response
         3       Success
         4       Failure

   Identifier

      The Identifier field is one octet and aids in matching challenges,
      responses and replies.

   Length

      The Length field is two octets and indicates the length of the
      CHAP packet including the Code, Identifier, Length and Data
      fields.  Octets outside the range of the Length field should be
      treated as Data Link Layer padding and should be ignored on
      reception.

   Data

      The Data field is zero or more octets.  The format of the Data
      field is determined by the Code field.


4.1.  Challenge and Response

   Description

      The Challenge packet is used to begin the Challenge-Handshake
      Authentication Protocol.  The authenticator MUST transmit a CHAP
      packet with the Code field set to 1 (Challenge).  Additional
      Challenge packets MUST be sent until a valid Response packet is
      received, or an optional retry counter expires.

      A Challenge packet MAY also be transmitted at any time during the
      Network-Layer Protocol phase to ensure that the connection has not
      been altered.

      The peer SHOULD expect Challenge packets during the Authentication
      phase and the Network-Layer Protocol phase.  Whenever a Challenge
      packet is received, the peer MUST transmit a CHAP packet with the
      Code field set to 2 (Response).

      Whenever a Response packet is received, the authenticator compares
      the Response Value with its own calculation of the expected value.
      Based on this comparison, the authenticator MUST send a Success or
      Failure packet (described below).

         Implementation Notes: Because the Success might be lost, the
         authenticator MUST allow repeated Response packets during the
         Network-Layer Protocol phase after completing the
         Authentication phase.  To prevent discovery of alternative
         Names and Secrets, any Response packets received having the
         current Challenge Identifier MUST return the same reply Code
         previously returned for that specific Challenge (the message
         portion MAY be different).  Any Response packets received
         during any other phase MUST be silently discarded.

         When the Failure is lost, and the authenticator terminates the
         link, the LCP Terminate-Request and Terminate-Ack provide an
         alternative indication that authentication failed.


   A summary of the Challenge and Response packet format is shown below.
   The fields are transmitted from left to right.

   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Value-Size   |  Value ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Name ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Code

      1 for Challenge;

      2 for Response.

   Identifier

      The Identifier field is one octet.  The Identifier field MUST be
      changed each time a Challenge is sent.

      The Response Identifier MUST be copied from the Identifier field
      of the Challenge which caused the Response.

   Value-Size

      This field is one octet and indicates the length of the Value
      field.

   Value

      The Value field is one or more octets.  The most significant octet
      is transmitted first.

      The Challenge Value is a variable stream of octets.  The
      importance of the uniqueness of the Challenge Value and its
      relationship to the secret is described above.  The Challenge
      Value MUST be changed each time a Challenge is sent.  The length
      of the Challenge Value depends upon the method used to generate
      the octets, and is independent of the hash algorithm used.

      The Response Value is the one-way hash calculated over a stream of
      octets consisting of the Identifier, followed by (concatenated
      with) the "secret", followed by (concatenated with) the Challenge
      Value.  The length of the Response Value depends upon the hash
      algorithm used (16 octets for MD5).


   Name

      The Name field is one or more octets representing the
      identification of the system transmitting the packet.  There are
      no limitations on the content of this field.  For example, it MAY
      contain ASCII character strings or globally unique identifiers in
      ASN.1 syntax.  The Name should not be NUL or CR/LF terminated.
      The size is determined from the Length field."

*/

/// Peer state machine for EAP MD5-Challenge authentication method.
class EAP_EXPORTS EapPeerMD5ChallengeStateMachine 
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerMD5ChallengeStateMachine>
{
  friend class EapMethodStateMachineCreator<EapPeerMD5ChallengeStateMachine>;
  friend class EapPeerMD5ChallengeStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    isDone = false;
    EapStateMachine<EapPeerMD5ChallengeStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerMD5ChallengeStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a passpharase
  /// needs to be obtained.
  virtual void InputPassphrase()=0;

  /// This function is used for getting the passphrase data.
  std::string& Passphrase() { return passphrase; }

  /// This function is used for getting the value data.
  std::string& Value() { return value; }

  /// This function is used for getting myName data.
  std::string& MyName() { return myName; }

  /// This function is used for getting peerName data.
  std::string& PeerName() { return peerName; }

protected:
  EapPeerMD5ChallengeStateMachine(EapSwitchStateMachine &s);
  ~EapPeerMD5ChallengeStateMachine() {} 

private:
  /// Shared secret.
  std::string passphrase;

  /// My name and peer name.
  std::string myName, peerName;

  /// The value data to send.
  std::string value;
};

/// Authenticator state machine for EAP MD5-Challenge method.
class EAP_EXPORTS EapAuthMD5ChallengeStateMachine 
  : public EapMethodStateMachine,
    public EapStateMachine<EapAuthMD5ChallengeStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthMD5ChallengeStateMachine>;
  friend class EapAuthMD5ChallengeStateTable_S;  
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
    EapStateMachine<EapAuthMD5ChallengeStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthMD5ChallengeStateMachine>::Notify(ev);
  }

  void Receive(AAAMessageBlock *b) {}

  /// This pure virtual function is a callback used when a passpharase
  /// needs to be obtained.
  virtual void InputPassphrase()=0;

  /// This function is used for getting the passphrase data.
  std::string& Passphrase() { return passphrase; }

  /// This function is used for getting the value data.
  std::string& Value() { return value; }

  /// This function is used for setting the value data.
  //  void Value(std::string& str) { value=str; }

  /// This function is used for getting myName data.
  std::string& MyName() { return myName; }

  /// This function is used for getting peerName data.
  std::string& PeerName() { return peerName; }

protected:
  EapAuthMD5ChallengeStateMachine(EapSwitchStateMachine &s);
  ~EapAuthMD5ChallengeStateMachine() {} 

private:
  /// Shared secret.
  std::string passphrase;

  /// My name and peer name.
  std::string myName, peerName;

  /// The value data to send.
  std::string value;
};

#endif // __EAP_MD5_HXX__
