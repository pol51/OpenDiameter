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
/* $Id: eap_archie.hxx,v 1.5 2004/06/17 21:13:36 yohba Exp $ */
/* 
   EAP Archie message format.

   Written by Yoshihiro Ohba
   
*/

/// \file 

#ifndef __EAP_ARCHIE_HXX__
#define __EAP_ARCHIE_HXX__

#include <ace/Basic_Types.h>
#include "eap.hxx"
#include "eap_log.hxx"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(EAP_ARCHIE_EXPORT)
       #define EAP_ARCHIE_EXPORTS __declspec(dllexport)
   #else
       #define EAP_ARCHIE_EXPORTS __declspec(dllimport)
   #endif
#else
   #define EAP_ARCHIE_EXPORTS
   #define EAP_ARCHIE_IMPORTS
#endif

/// \def EAP Request/Response Type code temporally assigned for
/// EAP-Archie.  This should be replaced with an IANA allocated value.
#define ARCHIE_METHOD_TYPE  100

/// EAP-Archie/Request-Archie message.
class EAP_ARCHIE_EXPORTS EapRequestArchie: public EapRequest
{
public:
  EapRequestArchie(ACE_Byte msgID) : 
    EapRequest(EapType(ARCHIE_METHOD_TYPE)), msgID(msgID) {}

  /// Use this function to obtain a reference to msgID.
  ACE_Byte& MsgID() { return msgID; }

  /// Enumerator 
  enum messageID {
    Request,    /// Enum for Archie-Request
    Response,   /// Enum for Archie-Response
    Confirm,    /// Enum for Archie-Confirm
    Finish      /// Enum for Archie-Finish
  };

protected:
  /// Archie message id
  ACE_Byte msgID;
};

/// EAP-Archie/Response-Archie message.
class EAP_ARCHIE_EXPORTS EapResponseArchie: public EapRequestArchie 
{
public:
  EapResponseArchie(ACE_Byte msgID) : EapRequestArchie(msgID) {}
};

/// EAP-Request/Archie-Request payload.  
class EAP_ARCHIE_EXPORTS EapRequestArchieRequest: public EapRequestArchie
{
public:
  /// Initialized with a specific message id (1).
  EapRequestArchieRequest() : EapRequestArchie(1) {}
			      
  /// Use this function to obtain a reference to authID.
  std::string& AuthID() { return authID; }
  
  /// Use this function to obtain a reference to sessionID.
  std::string& SessionID() { return sessionID; }
  
protected:

  /// The number of octets used in the AuthID field.  The value 0
  /// means the AuthID is the full 256 octets in length.
  ACE_Byte naiLength;

  /// The NAI o the EAP server.  The first naiLength octets of this
  /// field are non-zero, while any remaining octets of the authID
  /// field MUST be zero on transmit and ignored on receive.
  std::string authID;

  /// 32-octet session id.
  std::string sessionID;
};

/// Defines structure for "Binding" field that is used in several
/// EAP-Archie messages.
class EAP_ARCHIE_EXPORTS ArchieBinding
{
public:
  ArchieBinding() : bType(0), sLength(0), pLength(0),
		    addrS(std::string(256, '\0')),
		    addrP(std::string(256, '\0')) {}
  /// Binding type
  ACE_UINT16 bType;

  /// Octet length of the addS field.
  ACE_Byte sLength;

  /// Octet length of the addP field.
  ACE_Byte pLength;

  /// Server address.
  std::string addrS;

  /// Peer address.
  std::string addrP;
};

/// Archie-Response payload.  
class EAP_ARCHIE_EXPORTS EapResponseArchieResponse: public EapResponseArchie
{
public:
  /// Initialized with a specific message id (2).
  EapResponseArchieResponse() : EapResponseArchie(2) {}

  /// Use this function to obtain a reference to sessionID.
  std::string& SessionID() { return sessionID; }

  /// Use this function to obtain a reference to peerID.
  std::string& PeerID() { return peerID; }
  
  /// Use this function to obtain a reference to nanceP.
  std::string& NonceP() { return nonceP; }

  /// Use this function to obtain a reference to binding.
  ArchieBinding& Binding() { return binding; }

  /// Use this function to obtain a reference to mac1.
  std::string& Mac1() { return mac1; }
private:

  /// 32-octet session id.
  std::string sessionID;

  /// Peer identifier.
  std::string peerID;

  /// Nonce generated by the peer.
  std::string nonceP;

  /// Addressing information of peer and server.
  ArchieBinding binding;

  /// Message authentication code for this message.
  std::string mac1;
};

/// Archie-Confirm payload.  
class EAP_ARCHIE_EXPORTS EapRequestArchieConfirm: public EapRequestArchie
{
public:
  /// Initialized with a specific message ID (3).
  EapRequestArchieConfirm() : EapRequestArchie(3) {}

  /// Use this function to obtain a reference to sessionID.
  std::string& SessionID() { return sessionID; }

  /// Use this function to obtain a reference to nanceP.
  std::string& NonceA() { return nonceA; }

  /// Use this function to obtain a reference to binding.
  ArchieBinding& Binding() { return binding; }

  /// Use this function to obtain a reference to mac1.
  std::string& Mac2() { return mac2; }

private:

  /// 32-octet session id.
  std::string sessionID;

  /// Nonce generated by the server.
  std::string nonceA;

  /// Addressing information of peer and server.
  ArchieBinding binding;

  /// Message authentication code for this message.
  std::string mac2;
};

/// Archie-Finish payload.  
class EAP_ARCHIE_EXPORTS EapResponseArchieFinish: public EapResponseArchie
{
public:
  /// Initialized with a specific message id (4)
  EapResponseArchieFinish() : EapResponseArchie(4) {}

  /// Use this function to obtain a reference to sessionID.
  std::string& SessionID() { return sessionID; }

  /// Use this function to obtain a reference to mac1.
  std::string& Mac3() { return mac3; }

private:
  /// 32-octet session id.
  std::string sessionID;

  /// Message authentication code for this message.
  std::string mac3;
};

#endif // __EAP_ARCHIE_HXX__
