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
#ifndef  __EAP_PARSER_H__
#define  __EAP_PARSER_H__

#include "aaa_parser_api.h"
#include "eap.hxx"
#include "eap_log.hxx"

/// A parser to parse EAP header (i.e., Code, Identifier and Length fields).
typedef AAAParser<AAAMessageBlock*, EapHeader*> EapHeaderParser;

/// Use this function to convert raw EAP header data to
/// application-specific EAP header data.  As a result of calling this
/// function, the read pointer of the message block points to
/// the end of EAP header.
template<> inline void
EapHeaderParser::parseRawToApp()
{
  EapHeader *header = getAppData();
  AAAMessageBlock *buffer = getRawData();
  // Set the read pointer to the head of the buffer.
  buffer->rd_ptr(buffer->base());

  // read the code
  header->code = *buffer->rd_ptr(); buffer->rd_ptr(1);

  // read the identifier
  header->identifier = *buffer->rd_ptr(); buffer->rd_ptr(1);

  // read the length
  header->length = ntohs(*((ACE_UINT16*)buffer->rd_ptr())); 
  buffer->rd_ptr(sizeof(ACE_UINT16));

  // make sure buffer length is in sequence with header length
  buffer->wr_ptr(buffer->base() + header->length);
}

/// Use this function to convert 
/// application-specific EAP header data to raw EAP header data.  
/// As a result of calling this function, the write pointer
/// of the message block points to the end of EAP header.
template<> inline void
EapHeaderParser::parseAppToRaw()
{
  EapHeader *header = getAppData();
  AAAMessageBlock *buffer = getRawData();
  // Set the write pointer to the head of the buffer.
  buffer->wr_ptr(buffer->base());

  // write the code
  *buffer->wr_ptr() = header->code; buffer->wr_ptr(1);

  // write the identifier
  *buffer->wr_ptr() = header->identifier; buffer->wr_ptr(1);

  // write the length
  *((ACE_UINT16*)buffer->wr_ptr())
    = htons(header->length); buffer->wr_ptr(sizeof(ACE_UINT16));
}

/// A parser to parse EAP header (i.e., Code, Identifier and Length fields).
typedef AAAParser<AAAMessageBlock*, EapRequest*> EapRequestParser;

/// Use this function to convert raw EAP request payload data to
/// application-specific payload data.  As a result of calling this
/// function, the read pointer of the message block points to
/// the end of payload.
template<> inline void
EapRequestParser::parseRawToApp()
{
  EapPayload *payload = getAppData();
  AAAMessageBlock *buffer = getRawData();
  EapRequest *request = (EapRequest*)payload;

  // Set the read pointer to the head of the payload.
  buffer->rd_ptr(buffer->base() + 4);

  // Read the type.
  ACE_Byte type = *buffer->rd_ptr(); 
  ACE_UINT16 vendorId;
  ACE_UINT16 vendorType;
  if (type == 254) // Vendor Specific Extension
    {
      vendorId=((ACE_UINT16)*buffer->rd_ptr() && 0x00ffffff);
      buffer->rd_ptr(4);
      vendorId = ntohs(vendorId);
      vendorType=(ACE_UINT16)*buffer->rd_ptr();
      vendorType = ntohs(vendorType);
      request->SetType(EapType(vendorId, vendorType));  
    }
  else
    {
      buffer->rd_ptr(1);
      request->SetType(EapType(type));
    }
}

/// Use this function to convert 
/// application-specific payload data to raw EAP request payload
/// data.  As a result of calling this function, the write pointer
/// of the message block points to the end of payload so that the
/// parser can calculate the PDU length by using
/// ACE_Message_Block::wr_ptr() - ACE_Message_Block::base().
template<> inline void
EapRequestParser::parseAppToRaw()
{
  //EapPayload *payload = getAppData();
  AAAMessageBlock *buffer = getRawData();
  EapRequest *request = getAppData();

  // Set the read pointer to the head of the payload.
  buffer->wr_ptr(buffer->base() + 4);

  EapType eapType = request->GetType();

  // Write the type.

  if (eapType.type != 254) // Legacy type
    {
      buffer->copy((const char*)&eapType.type, 1);
    }
  else // Extended type
    {
      // Write vendor fields.
      ACE_UINT16 vid, vty;
      vid = (eapType.vendorId && 0x00ffffff) + (eapType.type << 24);
      vid = ntohs(vid);
      vty = ntohs(eapType.vendorType);
      buffer->copy((const char*)&vid, 4);
      buffer->copy((const char*)&vty, 4);
    }
}


/// A base class parser for EAP response type data of any Type
class EAP_EXPORTS EapResponseParser: public EapRequestParser {};

/// Parser dictionary used for EAP NAK.  The NAK dictionary indicates
/// whether the NAK is legacy NAK or extended NAK.
class EAP_EXPORTS EapNakDict
{
public:
  /// This is used for detemining the encoding rule for Nak.
  /// LegacyNak means Nak with N octets of Nak data while
  /// ExtendedNak means Nak with N*8 octets of NakData.
  enum NakType {
    LegacyNak,
    ExtendedNak
  };

  EapNakDict(NakType type) : type(type) {}

  NakType type;
};

/// Nak parser
typedef AAAParser<AAAMessageBlock*, EapNak*, EapNakDict*> EapNakParser;

/// Use this function to convert raw EAP Nak response payload data
/// (data following the Type field) to application-specific payload
/// data.  As a result of calling this function, the read pointer of
/// the message block points to the end of payload.
template<> inline void
EapNakParser::parseRawToApp()
{
  EapNakDict *d = getDictData();
  EapNak *nak = getAppData();
  AAAMessageBlock *msg = getRawData();

  EapTypeList &tList= nak->TypeList();

  if (d == NULL || d->type == EapNakDict::LegacyNak)    // legacy nak
    {
      EapTypeList::iterator i;
      
      for (i=tList.begin(); i!=tList.end(); i++)
	{
	  ACE_Byte type = *msg->rd_ptr(); 
	  msg->rd_ptr(1);
	  tList.push_back(EapType(type));
	}
      return;
    }
  else  // extended nak
    {
      while (!msg->end())
	{
	  // read the type.
	  ACE_Byte type = *msg->rd_ptr(); 
	  ACE_UINT16 vendorId;
	  ACE_UINT16 vendorType;
	  if (type == 254) // vendor specific type
	    {
	      vendorId=((ACE_UINT16)*msg->rd_ptr() && 0x00ffffff);
	      msg->rd_ptr(4);
	      vendorId = ntohs(vendorId);
	      vendorType=(ACE_UINT16)*msg->rd_ptr();
	      vendorType = ntohs(vendorType);
	      if (vendorType != 3)  // extended Nak
		tList.push_back(EapType(vendorId, vendorType));
	    }
	  else // legacy type
	    {
	      msg->rd_ptr(1);
	      tList.push_back(EapType(type));
	    }
	}
    }
}

/// Use this function to convert 
/// application-specific Nak payload data to raw EAP Nak response
/// payload data (data following the Type field).  As a result of
/// calling this function, the write pointer of the message block
/// points to the end of payload so that the parser can calculate
/// the PDU length by using ACE_Message_Block::wr_ptr() -
/// ACE_Message_Block::base().
template<> inline void
EapNakParser::parseAppToRaw()
{
  EapNakDict *d = getDictData();
  EapNak *nak = getAppData();
  AAAMessageBlock *msg = getRawData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(nak);
  responseParser.parseAppToRaw();     

  EapTypeList &tList= nak->TypeList();

  if (d == NULL || d->type == EapNakDict::LegacyNak)    // legacy nak
    {
      EapTypeList::iterator i;
      
      for (i=tList.begin(); i!=tList.end(); i++)
	{
	  msg->copy((const char*)&(*i).type, 1);
	}
    }

  else // Extended nak
    {
      EapTypeList::iterator i;
      ACE_UINT32 vendorId, vendorType;

      // Set first entry (254,0,3)
      vendorId = (254 << 24);
      vendorId = ntohs(vendorId);
      vendorType = ntohs(3);
      msg->copy((const char*)&vendorId, 4);
      msg->copy((const char*)&vendorType, 4);

      // Set remaining ones.
      for (i=tList.begin(); i!=tList.end(); i++)
	{
	  EapType t = *i;

	  // Convert legacy type representation to vendor-specific
	  // representation.
	  if (t.type!=254)
	    t = EapType(0, t.type);

	  // Write vendor fields.
	  vendorId = (t.vendorId && 0x00ffffff) | (t.type << 24);
	  vendorId = ntohs(vendorId);
	  vendorType = ntohs(t.vendorType);
	  msg->copy((const char*)&vendorId, 4);
	  msg->copy((const char*)&vendorType, 4);
	}
    }
}

/// Request/Identity parser
typedef AAAParser<AAAMessageBlock*, EapIdentity*> 
EapRequestIdentityParser;

/// Use this function to convert raw EAP Identity request/response
/// payload data (data following the Type field) to
/// application-specific payload data.  As a result of calling this
/// function, the read pointer of the message block points to the
/// end of payload.
template<> inline void
EapRequestIdentityParser::parseRawToApp() 
{
  AAAMessageBlock *msg = getRawData();
  EapIdentity *identity = getAppData();
  size_t idStringSize = (msg->wr_ptr()-msg->base()) - (msg->rd_ptr()-msg->base());

  // UTF8 varidation without null-charater check.
  UTF8Checker check;
  if (check(msg->rd_ptr(), idStringSize) != 0)
    {
      EAP_LOG(LM_ERROR, "Invalid UTF8 string");
      throw -1;
    }
  std::string &str = identity->Identity();
  str.assign(msg->rd_ptr(), idStringSize);
  msg->rd_ptr(msg->end());
}

/// Use this function to convert 
/// application-specific Identity payload data to raw EAP Identity
/// request/response payload data (data following the Type field).  As
/// a result of calling this function, the write pointer of the
/// message block points to the end of payload so that the parser can
/// calculate the PDU length by using ACE_Message_Block::wr_ptr() -
/// ACE_Message_Block::base().
template<> inline void
EapRequestIdentityParser::parseAppToRaw()
{
  AAAMessageBlock *msg = getRawData();
  EapIdentity *identity = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(identity);
  responseParser.parseAppToRaw();     

  //  EapRequestParser::parseAppToRaw();     
  std::string &str = identity->Identity();

  UTF8Checker check;
  // UTF8 varidation with null-charater check.
  if (check(str.data(), str.size(), true) != 0)
    {
      EAP_LOG(LM_ERROR, "Invalid UTF8 string");
      throw -1;
    }
  msg->copy(str.data(), str.size());
}

/// Request/Notification parser
class EAP_EXPORTS EapResponseIdentityParser: public EapRequestIdentityParser {};

/// Request/Identity parser
typedef AAAParser<AAAMessageBlock*, EapNotification*> 
EapRequestNotificationParser;

/// Use this function to convert raw EAP Notification request/response
/// payload data (data following the Type field) to
/// application-specific payload data.  As a result of calling this
/// function, the read pointer of the message block points to the
/// end of payload.
template<> inline void
EapRequestNotificationParser::parseRawToApp()
{
  AAAMessageBlock *msg = getRawData();
  EapNotification *notification = getAppData();
  size_t notificationStringSize = (msg->wr_ptr()-msg->base()) - (msg->rd_ptr()-msg->base());

  // UTF8 varidation without null-charater check.
  UTF8Checker check;
  if (check(msg->rd_ptr(), notificationStringSize) != 0)
    {
      EAP_LOG(LM_ERROR, "Invalid UTF8 string");
      throw -1;
    }

  std::string &str = notification->Notification();
  str.assign(msg->rd_ptr(), notificationStringSize);
  msg->rd_ptr(msg->end());
}

/// Use this function to convert 
/// application-specific Notification payload data to raw EAP
/// Notification request/response payload data (data following the
/// Type field).  As a result of calling this function, the write
/// pointer of the message block points to the end of payload so
/// that the parser can calculate the PDU length by using
/// ACE_Message_Block::wr_ptr() - ACE_Message_Block::base().
template<> inline void
EapRequestNotificationParser::parseAppToRaw()
{
  AAAMessageBlock *msg = getRawData();
  EapNotification *notification = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(notification);
  responseParser.parseAppToRaw();     

  //  EapRequestParser::parseAppToRaw();     

  std::string &str = notification->Notification();
  UTF8Checker check;
  // UTF8 varidation with null-charater check.
  if (check(str.data(), str.size(), true) != 0)
    {
      EAP_LOG(LM_ERROR, "Invalid UTF8 string");
      throw -1;
    }

  msg->copy(str.data(), str.size());
}

/// Response/Notification parser
class EAP_EXPORTS EapResponseNotificationParser: public EapRequestIdentityParser 
{};

/// Request/MD5-Challenge parser
typedef AAAParser<AAAMessageBlock*, EapMD5Challenge*> 
EapRequestMD5ChallengeParser;

/// Use this function to convert raw EAP MD5-Challenge request data
/// (data following the Type field) to application-specific payload
/// data.  As a result of calling this function, the read pointer of
/// the message block points to the end of payload.
template<> inline void
EapRequestMD5ChallengeParser::parseRawToApp()
{
  AAAMessageBlock *msg = getRawData();
  EapMD5Challenge *md5Challenge = getAppData();
  size_t contentSize = (msg->wr_ptr()-msg->base()) - (msg->rd_ptr()-msg->base());

  // Read the Value-Size.
  ACE_Byte valueSize = (ACE_Byte)*msg->rd_ptr();

  msg->rd_ptr(1);

  // Read the Value.
  md5Challenge->Value().assign(msg->rd_ptr(), valueSize);
  msg->rd_ptr(valueSize);

  if (valueSize == 0)
    {
      EAP_LOG(LM_ERROR, "Empty value.");
      throw -1;
    }

  // Read the Name.
  ACE_INT16 nameLength = contentSize - (valueSize + 1); 
  md5Challenge->Name().assign(msg->rd_ptr(), nameLength);
  msg->rd_ptr(nameLength);
}

/// Use this function to convert 
/// application-specific MD5-Challenge payload data to raw EAP
/// MD5-Challenge request/response payload data (data following the
/// Type field).  As a result of calling this function, the write
/// pointer of the message block points to the end of payload so
/// that the parser can calculate the PDU length by using
/// ACE_Message_Block::wr_ptr() - ACE_Message_Block::base().
template<> inline void
EapRequestMD5ChallengeParser::parseAppToRaw()
{
  AAAMessageBlock *msg = getRawData();
  EapMD5Challenge *md5Challenge = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(md5Challenge);
  responseParser.parseAppToRaw();     

  //  EapRequestParser::parseAppToRaw();     

  ACE_Byte valueSize = md5Challenge->Value().size();

  // Check the value size.
  if (valueSize == 0)
    {
      EAP_LOG(LM_ERROR, "Empty value.");
      throw -1;
    }

  // Write the Value-Size.
  msg->copy((const char*)&valueSize, sizeof(valueSize));

  // Write the Value.
  msg->copy(md5Challenge->Value().data(), valueSize);

  // Write the Name.
  msg->copy(md5Challenge->Name().data(), md5Challenge->Name().size());
}

/// Request/MD5-Challenge parser
class EAP_EXPORTS EapResponseMD5ChallengeParser
  : public EapRequestMD5ChallengeParser 
{};

#endif // __EAP_PARSER_H__
