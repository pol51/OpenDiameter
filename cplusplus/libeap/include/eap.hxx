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
/* $Id: eap.hxx,v 1.10 2004/06/17 21:13:36 yohba Exp $ */
/* 
   EAP header file.
   EAP application programs and programs for each of EAP mechanisms 
   MUST include this file.

   Written by Yoshihiro Ohba
   
*/

#ifndef __EAP_HXX__
#define __EAP_HXX__

/* Structure for EAP Finite State Machine */

#include <ace/Basic_Types.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Queue.h>
#include <ace/Synch.h>
#include <list>
#include "aaa_parser_api.h"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(EAP_EXPORT)
       #define EAP_EXPORTS __declspec(dllexport)
   #else
       #define EAP_EXPORTS __declspec(dllimport)
   #endif
#else
   #define EAP_EXPORTS
   #define EAP_IMPORTS
#endif

/// Thread-safe queue class used for storing EAP messages.
typedef ACE_Message_Queue<ACE_MT_SYNCH> EapMessageQueue;

/// EAP codes
enum EapCode {
  Request = 1,
  Response,
  Success,
  Failure
};

/// Integrated EAP type with a support for vendor-specific types.
class EapType
{
 public:
  EapType() {}

  /// This constractor is used for legacy Request/Response
  EapType(ACE_Byte type) : 
    type(type), vendorId(0), vendorType(0) {}

  /// This constractor is used for extended Request/Response
  EapType(ACE_UINT16 vid, ACE_UINT16 vty) : 
    type(254), vendorId(vid), vendorType(vty) {}

  /// This operator is used for equality check for given two EapType
  /// values.
  bool operator==(EapType ty) 
    {
      if ((ty.type == type) && 
	  (ty.vendorId == vendorId) && 
	  (ty.vendorType == vendorType))
	return true;
      return false;
    }

  /// This operator is used for inequality check for given two EapType
  /// values.
  bool operator!=(EapType ty) 
    {
      return !(*this == ty);
    }

  /// This function is used for checking whether this eap type has
  /// a vendor-specific extention or not.
  inline bool IsVSE() { return (type==254 && (*this != EapType(0,0))); }
  ACE_Byte  type;
  ACE_UINT16 vendorId;
  ACE_UINT16 vendorType;
};

/// EAP roles
enum EapRole {
  Peer,
  Authenticator
};

/// EAP header definition.
class EapHeader
{
 public:
  EapHeader() : code(0), identifier(0), length(0) {}
  ACE_Byte code;
  ACE_Byte identifier;
  ACE_UINT16 length;
};

const ACE_UINT16 EapMaxPduLength=ACE_UINT16_MAX;

/// EAP payload definition.  This is the base class for EAP
/// request/response.
class EapPayload {};

/// Eap Request payload
class EapRequest : public EapPayload
{
public:
  EapRequest() {}
  EapRequest(EapType ty) : type(ty) {}
  ~EapRequest() {}
  inline EapType& GetType() { return type; }
  inline void SetType(EapType ty) { type=ty; }
  inline bool IsVSE() { return isVSE; }
  inline void SetVSE() { isVSE=true; }
protected:
  /// Type field of EAP Request/Response
  EapType  type;

  /// indicates whether this contains vendor-specific extention.
  bool isVSE;
};

/// Eap Response payload.
class EapResponse: public EapRequest 
{
 public:
  EapResponse() : EapRequest() {}
  EapResponse(EapType ty) : EapRequest(ty) {}
  ~EapResponse() {}
};

/// This class is used for storing EAP types for Nak.
class EapTypeList : public std::list<EapType>
{
public:

  /// This function returns true if the specified type is found in the
  /// type list.
  bool Search(EapType type) 
  {
    EapTypeList::iterator i;
    for (i=begin(); i!=end(); i++)
      {
	if (*i == type)
	  return true;
      }
    return false;
  }

  /// This function returns true if typeL contains an extended type.
  bool IsVSE() 
  {
    EapTypeList::iterator i;
    for (i=begin(); i!=end(); i++)
      {
	if ((*i).IsVSE())
	  return true;
      }
    return false;
  }
};

/// Eap Nak type payload
class EapNak: public EapResponse
{
public:
  EapNak() : EapResponse(EapType(3)) {}
  EapTypeList& TypeList() { return typeList; }
private:
  EapTypeList typeList;
};

/// Idenity type payload.  
class EapIdentity: public EapRequest
{
public:
  /// Initialized with a specific EAP Type (1).
  EapIdentity() : EapRequest(EapType(1)) {}

  /// This function is used for getting identity data.
  std::string& Identity() { return identity; }

private:
  /// The Identity Type-Data.  It is described in RFC2284bis that
  /// "This field MAY contain a displayable message in the Request,
  /// containing UTF-8 encoded ISO 10646 characters [RFC2279].  Where
  /// the Request contains a null, only the portion of the field prior
  /// to the null is displayed.  If the Identity is unknown, the
  /// Identity Response field should be zero bytes in length.  The
  /// Identity Response field MUST NOT be null terminated.  In all
  /// cases, the length of the Type-Data field is derived from the
  /// Length field of the Request/Response packet".
  std::string identity;
};

/// Notification type payload.  The notification parser has the
/// functionality of detecting null-data which is not allowed in
/// RFC2284bis.  See \ref notification "Notification handling rule"
/// for detailed information.
class EapNotification: public EapRequest
{
public:
  /// Initialized with a specific EAP Type (2).
  EapNotification() : EapRequest(EapType(2)) {}

  /// This function is used for getting identity data.
  std::string& Notification() { return notification; }

private:
  /// The Notification Type-Data.  See \ref notification "Notification
  /// handling rule" for detailed information.
  std::string notification;
};

/// MD5-Challenge type payload.  See \ref md5-challange "MD5-Challange
/// Request/Response description" for detailed information.
/// information.
class EapMD5Challenge: public EapRequest
{
public:
  /// Initialized with a specific EAP Type (4).
  EapMD5Challenge() : EapRequest(EapType(4)) {}

  /// This function is used for getting the value data.
  std::string& Value() { return value; }

  /// This function is used for setting the value data.
  void Value(std::string& value) { this->value = value; }

  /// This function is used for getting name data.
  std::string& Name() { return name; }

  /// This function is used for setting name data.
  void Name(std::string& name) { this->name = name; }

private:
  /// The MD5-Challenge Type-Data.  It can be used for storing either
  /// challenge or response.
  std::string value;
  std::string name;
};

#endif
