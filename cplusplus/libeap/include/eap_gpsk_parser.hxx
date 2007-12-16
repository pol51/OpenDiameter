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

// Archie parser definition.
// Written by Yoshihiro Ohba (yohba@tari.toshiba.com)

#ifndef  __EAP_ARCHIE_PARSER_H__
#define  __EAP_ARCHIE_PARSER_H__

#include "eap_archie.hxx"
#include "eap_parser.hxx"
#include "eap_log.hxx"

/// EAP-Request/Archie parser
typedef AAAParser<AAAMessageBlock*, EapRequestArchie*> 
EapRequestArchieParser;

inline void
copyWithPadding(AAAMessageBlock *msg, const char *data, 
		int dataLength, int blockSize)
{
  msg->copy(data, dataLength ? dataLength : blockSize);
  if (dataLength)
    {
      ACE_OS::memset(msg->wr_ptr(), 0, blockSize - dataLength);
      msg->wr_ptr(blockSize - dataLength);
    }
}

/// Use this function to convert raw EAP-Request/Archie data (data
/// following the Type field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the MsgID field.
template<> inline void
EapRequestArchieParser::parseRawToApp() 
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchie* request = getAppData();
  // Read msgID.
  request->MsgID() = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);
  return;
}

/// Use this function to convert application-specific
/// EAP-Request/Archie payload data to raw EAP-Request/Archie-Request
/// payload data (data following the MsgID field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the MsgID field.
//void EapRequestArchieParser::parseAppToRaw();
template<> inline void
EapRequestArchieParser::parseAppToRaw() 
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchie* request = getAppData();

  // Write msgID.
  *(ACE_Byte*)msg->wr_ptr() = request->MsgID();
  msg->wr_ptr(1);
  return;
}


/// EAP-Request/Archie-Request parser
typedef AAAParser<AAAMessageBlock*, EapRequestArchieRequest*> 
EapRequestArchieRequestParser;

/// Use this function to convert raw EAP-Request/Archie-Request data (data
/// following the MsgID field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapRequestArchieRequestParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchieRequest* request = getAppData();

  ACE_Byte msgId = *(ACE_Byte*)msg->rd_ptr();
  // Read msgID.
  if (msgId != 1)
    {
      EAP_LOG(LM_ERROR, "Invalid msgID %d (1 is expected).\n", msgId);
      throw -1;
    }

  msg->rd_ptr(2);  
  
  // Read NaiLength.
  ACE_Byte& naiLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  // Read AuthID.
  request->AuthID() = std::string(msg->rd_ptr(), naiLength ? naiLength : 256);
  msg->rd_ptr(256);

  // Read Session ID.
  request->SessionID() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);
}

/// Use this function to convert application-specific
/// EAP-Request/Archie payload data to raw EAP-Request/Archie-Request
/// payload data (data following the MsgID field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the end of the payload.
//void EapRequestArchieRequestParser::parseAppToRaw();
template<> inline void 
EapRequestArchieRequestParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchieRequest* request = getAppData();
  
  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(request);
  requestParser.parseAppToRaw();     

  // Write msgID.
  *(ACE_Byte*)msg->wr_ptr() = request->MsgID();
  msg->wr_ptr(1);

  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.

  // Check AuthID length.
  int length = request->AuthID().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "AuthID is empty.");
      throw -1;
    }
  ACE_Byte naiLength = (ACE_Byte)length;

  *(ACE_Byte*)msg->wr_ptr() = naiLength;
  msg->wr_ptr(1);

  copyWithPadding(msg, request->AuthID().data(), naiLength, 256);

  msg->copy(request->SessionID().data(), 32);
}

/// EAP-Response/Archie-Response parser
typedef AAAParser<AAAMessageBlock*, EapResponseArchieResponse*> 
EapResponseArchieResponseParser;

/// Use this function to convert raw EAP-Response/Archie-Response data (data
/// following the MsgID field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseArchieResponseParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseArchieResponse* response = getAppData();

  // Read msgID.
  if (*(ACE_Byte*)msg->rd_ptr() != 2)
    {
      EAP_LOG(LM_ERROR, "Invalid msgID.");
      throw -1;
    }

  msg->rd_ptr(2);

  // Read NaiLength.
  ACE_Byte& naiLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  // Read session id.
  response->SessionID() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read peer id.
  response->PeerID() = std::string(msg->rd_ptr(), naiLength ? naiLength : 256);
  msg->rd_ptr(256);

  // Read nonceP.
  response->NonceP() = std::string(msg->rd_ptr(), 40);
  msg->rd_ptr(40);

  // Read binding.
  response->Binding().bType = ntohs(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  ACE_Byte& sLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  ACE_Byte& pLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  response->Binding().addrS = 
    std::string(msg->rd_ptr(), sLength ? sLength : 256);
  msg->rd_ptr(256);

  response->Binding().addrP = 
    std::string(msg->rd_ptr(), pLength ? pLength : 256);
  msg->rd_ptr(256);

  // Read MAC1
  response->Mac1() = std::string(msg->rd_ptr(), 12);
  msg->rd_ptr(12);
}

/// Use this function to convert application-specific
/// EAP-Response/Archie-Response payload data to raw payload data
/// (data following the MsgID field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
//void EapResponseArchieResponseParser::parseAppToRaw();
template<> inline void 
EapResponseArchieResponseParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseArchieResponse* response = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(response);
  responseParser.parseAppToRaw();     

  // Write msgID.
  *(ACE_Byte*)msg->wr_ptr() = response->MsgID();
  msg->wr_ptr(1);

  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.

  // Check PeerID length.
  int length = response->PeerID().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "PeerID is empty.");
      throw -1;
    }
  ACE_Byte naiLength = (ACE_Byte)length;

  *(ACE_Byte*)msg->wr_ptr() = naiLength;
  msg->wr_ptr(1);

  // Write session id.
  msg->copy(response->SessionID().data(), 32);

  // Write peer id.
  copyWithPadding(msg, response->PeerID().data(), naiLength, 256);

  // Write nonceP.
  msg->copy(response->NonceP().data(), 40);

  // Write binding.
  *(ACE_UINT16*)msg->wr_ptr() = ntohs(response->Binding().bType);
  msg->wr_ptr(2);

  ACE_Byte sLength = (ACE_Byte)response->Binding().addrS.size();
  *msg->wr_ptr() = sLength;
  msg->wr_ptr(1);

  ACE_Byte pLength = (ACE_Byte)response->Binding().addrP.size();
  *msg->wr_ptr() = pLength;
  msg->wr_ptr(1);

  copyWithPadding(msg, response->Binding().addrS.data(), sLength, 256);
  copyWithPadding(msg, response->Binding().addrP.data(), pLength, 256);

  // Write MAC1.
  msg->copy(response->Mac1().data(), 12);
}

/// EAP-Request/Archie-Confirm parser
typedef AAAParser<AAAMessageBlock*, EapRequestArchieConfirm*> 
EapRequestArchieConfirmParser;

/// Use this function to convert raw EAP-Request/Archie-Confirm data (data
/// following the MsgID field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapRequestArchieConfirmParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchieConfirm* confirm = getAppData();

  // Read msgID.
  if (*(ACE_Byte*)msg->rd_ptr() != 3)
    {
      EAP_LOG(LM_ERROR, "Invalid msgID.");
      throw -1;
    }

  msg->rd_ptr(3);  

  // Read session id.
  confirm->SessionID() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read nonceA.
  confirm->NonceA() = std::string(msg->rd_ptr(), 40);
  msg->rd_ptr(40);

  // Read binding.
  confirm->Binding().bType = ntohs(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  ACE_Byte& sLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  ACE_Byte& pLength = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);

  confirm->Binding().addrS = 
    std::string(msg->rd_ptr(), sLength ? sLength : 256);
  msg->rd_ptr(256);

  confirm->Binding().addrP = 
    std::string(msg->rd_ptr(), pLength ? pLength : 256);
  msg->rd_ptr(256);

  // Read MAC2
  confirm->Mac2() = std::string(msg->rd_ptr(), 12);
  msg->rd_ptr(12);
}

/// Use this function to convert application-specific
/// EAP-Request/Archie-Confirm data to raw payload data
/// (data following the MsgID field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void 
EapRequestArchieConfirmParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestArchieConfirm* confirm = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(confirm);
  requestParser.parseAppToRaw();     

  // Write msgID.
  *(ACE_Byte*)msg->wr_ptr() = confirm->MsgID();
  msg->wr_ptr(1);

  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.
  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.

  // Write session id.
  msg->copy(confirm->SessionID().data(), 32);

  // Write nonceA.
  msg->copy(confirm->NonceA().data(), 40);

  // Write binding.
  *(ACE_UINT16*)msg->wr_ptr() = ntohs(confirm->Binding().bType);
  msg->wr_ptr(2);

  ACE_Byte sLength = (ACE_Byte)confirm->Binding().addrS.size();
  *msg->wr_ptr() = sLength;
  msg->wr_ptr(1);

  ACE_Byte pLength = (ACE_Byte)confirm->Binding().addrP.size();
  *msg->wr_ptr() = pLength;
  msg->wr_ptr(1);

  copyWithPadding(msg, confirm->Binding().addrS.data(), sLength, 256);
  copyWithPadding(msg, confirm->Binding().addrP.data(), pLength, 256);

  // Write MAC2
  msg->copy(confirm->Mac2().data(), 12);
}

/// EAP-Response/Archie-Finisht parser
typedef AAAParser<AAAMessageBlock*, EapResponseArchieFinish*> 
EapResponseArchieFinishParser;

/// Use this function to convert raw EAP-Response/Archie-Finish data (data
/// following the MsgID field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseArchieFinishParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseArchieFinish* finish = getAppData();

  // Read msgID.
  if (*(ACE_Byte*)msg->rd_ptr() != 4)
    {
      EAP_LOG(LM_ERROR, "Invalid msgID.");
      throw -1;
    }

  msg->rd_ptr(3);  

  // Read session id.
  finish->SessionID() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read MAC3
  finish->Mac3() = std::string(msg->rd_ptr(), 12);
  msg->rd_ptr(12);
}

/// Use this function to convert application-specific
/// EAP-Response/Archie-Finish data to raw payload data
/// (data following the MsgID field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void 
EapResponseArchieFinishParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseArchieFinish* finish = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(finish);
  responseParser.parseAppToRaw();     

  // Write msgID.
  *(ACE_Byte*)msg->wr_ptr() = finish->MsgID();
  msg->wr_ptr(1);

  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.
  *msg->wr_ptr() = 0;  msg->wr_ptr(1);  // Skip Reserved field.

  // Write session id.
  msg->copy(finish->SessionID().data(), 32);

  // Write MAC3
  msg->copy(finish->Mac3().data(), 12);
}

#endif // __EAP_ARCHIE_PARSER_H__
