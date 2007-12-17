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

#ifndef  __EAP_GPSK_PARSER_H__
#define  __EAP_GPSK_PARSER_H__

#include "eap_gpsk.hxx"
#include "eap_parser.hxx"
#include "eap_log.hxx"

/// EAP-Request/Gpsk parser
typedef AAAParser<AAAMessageBlock*, EapRequestGpsk*>
EapRequestGpskParser;

inline void
readCipherSuiteList(std::string &strlist, int length,
  EapGpskCipherSuiteList &clist)
{
  char *cc;
  int csuiteLength = length/6;
  for (int i = 0; i < csuiteLength; i++)
    {
      EapGpskCipherSuite csuite;
      cc = cipherList.data();
      csuite.Vendor() = ACE_NTOHL(*(ACE_UINT32*)cc);
      cc += 4;
      csuite.ChiperSuite = ACE_NTOHS(*(ACE_UINT16*)cc);
      cc += 2;
      clist.push_back(csuite);
    }
}

inline void
writeCipherSuiteList(std::string &strlist,
  EapGpskCipherSuiteList &clist)
{
  std::string *container = new std::string(clist.length() * 6, 0);
  char *cc = container->data();
  std::list<EapGpskCipherSuite>::iterator i = clist.begin();
  for (; i != clist.end(); i++)
    {
      EapGpskCipherSuite csuite = *i;
      *(ACE_UINT32*)cc = ACE_HTONL(csuite.Vendor());
      cc += 4;
      *(ACE_UINT16*)cc = ACE_HTONS(csuite.ChiperSuite());
      cc += 2;
    }
  strlist = *container;
  delete container;
}

inline size_t
getMacLength(EapGpskCipherSuite &cipher)
{
  if (response->CSuiteSelected().ChiperSuite() == 1)
    {
       return 16;
    }
  else (response->CSuiteSelected().ChiperSuite() == 2)
    {
       return 32;
    }
  else
    {
      EAP_LOG(LM_ERROR, "Un-support cphier %d (1 or 2 is expected).\n",
        response->CSuiteSelected().ChiperSuite());
      throw -1;
    }
  return 0;
}

/// Use this function to convert raw EAP-Request/Gpsk data (data
/// following the Type field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the Op-Code field.
template<> inline void
EapRequestGpskParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk* request = getAppData();

  // Read Op-Code.
  request->OpCode() = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);
  return;
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk payload data to raw EAP-Request/Gpsk-Request
/// payload data (data following the MsgID field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the MsgID field.
//void EapRequestGpskParser::parseAppToRaw();
template<> inline void
EapRequestGpskParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk* request = getAppData();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = request->OpCode();
  msg->wr_ptr(1);
  return;
}


/// EAP-Request/Gpsk-Request parser
typedef AAAParser<AAAMessageBlock*, EapRequestGpsk1*>
EapRequestGpsk1Parser;

/// Use this function to convert raw EAP-Request/Gpsk1 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapRequestGpsk1Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk1* request = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 1)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (1 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read ID_Server length.
  ACE_UINT16& idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  request->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read RAND_Server.
  request->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read CSuite List length.
  ACE_UINT16& csuiteLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  if (csuiteLength % 6)
    {
      EAP_LOG(LM_ERROR, "Invalid chiper suite length %d (not divisible by 6).\n", csuiteLength);
      throw -1;
    }

  // Read CSuite List.
  std::string cipherList = std::string(msg->rd_ptr(), csuiteLength);
  msg->rd_ptr(csuiteLength);

  readCipherSuiteList(cipherList, csuiteLength, request->CSuiteList);
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk1 payload data to raw EAP-Request/Gpsk1
/// payload data (data following the Op-Code field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the end of the payload.
//void EapRequestGpskRequestParser::parseAppToRaw();
template<> inline void 
EapRequestGpsk1Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk1* request = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(request);
  requestParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = request->OpCode();
  msg->wr_ptr(1);

  // Check ID_Server length.
  int length = request->IDServer().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "ID_Server is empty.");
      throw -1;
    }

  // Write the ID_Server length.
  ACE_UINT16 idServerLength = (ACE_UINT16)length;
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(idServerLength);
  msg->wr_ptr(2);

  // Write the ID_Server
  msg->copy(request->IDServer().data(), idServerLength);

  // Write RAND_Server
  msg->copy(request->RANDServer().data(), 32);

  // Check CSuite List length.
  length = request->CSuiteList().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "CSuite List is empty.");
      throw -1;
    }

  // Write CSuite list length
  ACE_UINT16 csuiteLength = (ACE_UINT16)(request->CSuiteList().length() * 6);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(csuiteLength);
  msg->wr_ptr(2);

  // Write CSuite list
  std::string strlist;
  writeCipherSuiteList(strlist, request->CSuiteList());
  msg->copy(strlist.data(), csuiteLength);
}

/// EAP-Response/Gpsk-Response parser
typedef AAAParser<AAAMessageBlock*, EapResponseGpsk2*>
EapResponseGpsk2Parser;

/// Use this function to convert raw EAP-Response/Gpsk2 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseGpsk2Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpsk2* response = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 2)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (2 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read ID_Peer length.
  ACE_UINT16& idPeerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Peer.
  request->IDPeer() = std::string(msg->rd_ptr(), idPeerLength);
  msg->rd_ptr(idPeerLength);

  // Read ID_Server length.
  ACE_UINT16& idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  request->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read RAND_Peer.
  response->RANDPeer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read RAND_Server.
  response->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read CSuite List length.
  ACE_UINT16& csuiteLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read CSuite List.
  std::string cipherList = std::string(msg->rd_ptr(), csuiteLength);
  msg->rd_ptr(csuiteLength);
  readCipherSuiteList(cipherList, csuiteLength, response->CSuiteList);

  // Read CSuite Selected.
  response->CSuiteSelected().Vendor() = ACE_NTOHL(*(ACE_UINT32*)msg->rd_ptr());
  msg->rd_ptr(4);
  response->CSuiteSelected().ChiperSuite() = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload length.
  ACE_UINT16& pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  request->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());

  // Read Payload MAC
  request->Mac() = std::string(msg->rd_ptr(), macLength);
}

/// Use this function to convert application-specific
/// EAP-Response/Gpsk2 payload data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
//void EapResponseGpskResponseParser::parseAppToRaw();
template<> inline void 
EapResponseGpsk2Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpskResponse* response = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(response);
  responseParser.parseAppToRaw();     

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = request->OpCode();
  msg->wr_ptr(1);

  // Check ID_Peer length.
  int length = response->IDPeer().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "ID_Peer is empty.");
      throw -1;
    }

  // Write the ID_Peer length.
  ACE_UINT16 idPeerLength = (ACE_UINT16)length;
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(idPeerLength);
  msg->wr_ptr(2);

  // Write the ID_Peer
  msg->copy(response->IDPeer().data(), idPeerLength);

  // Check ID_Server length.
  length = response->IDServer().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "ID_Server is empty.");
      throw -1;
    }

  // Write the ID_Server length.
  ACE_UINT16 idServerLength = (ACE_UINT16)length;
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(idServerLength);
  msg->wr_ptr(2);

  // Write the ID_Server
  msg->copy(response->IDServer().data(), idServerLength);

  // Write RAND_Peer
  msg->copy(response->RANDPeer().data(), 32);

  // Write RAND_Server
  msg->copy(response->RANDServer().data(), 32);

  // Check CSuite List length.
  length = response->CSuiteList().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "CSuite List is empty.");
      throw -1;
    }

  // Write CSuite list length
  ACE_UINT16 csuiteLength = (ACE_UINT16)(response->CSuiteList().length() * 6);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(csuiteLength);
  msg->wr_ptr(2);

  // Write CSuite list
  std::string strlist;
  writeCipherSuiteList(strlist, response->CSuiteList());
  msg->copy(strlist.data(), csuiteLength);

  // Write CSuite Selection
  *(ACE_UINT32*)msg->wr_ptr() = ACE_HTONL(response->CSuiteSelected().Vendor());
  msg->wr_ptr(4);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(response->CSuiteSelected().ChiperSuite());
  msg->wr_ptr(2);

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC. Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());
  msg->copy(response->Mac().data(), macLength);
}

/// EAP-Request/Gpsk-Confirm parser
typedef AAAParser<AAAMessageBlock*, EapRequestGpsk3*>
EapRequestGpsk3Parser;

/// Use this function to convert raw EAP-Request/Gpsk3 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapRequestGpsk3Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk3* request = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 3)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (3 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read RAND_Peer.
  response->RANDPeer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read RAND_Server.
  response->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read ID_Server length.
  ACE_UINT16& idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  request->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read CSuite Selected.
  response->CSuiteSelected().Vendor() = ACE_NTOHL(*(ACE_UINT32*)msg->rd_ptr());
  msg->rd_ptr(4);
  response->CSuiteSelected().ChiperSuite() = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload length.
  ACE_UINT16& pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  request->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Read Payload MAC - Only AES and SHA is supported
  int macLength = getMacLength(request->CSuiteSelected());
  request->Mac() = std::string(msg->rd_ptr(), macLength);
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk3 data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapRequestGpsk3Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapRequestGpsk3* request = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(request);
  requestParser.parseAppToRaw();     

  // Write RAND_Peer
  msg->copy(request->RANDPeer().data(), 32);

  // Write RAND_Server
  msg->copy(request->RANDServer().data(), 32);

  // Check ID_Server length.
  length = response->IDServer().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "ID_Server is empty.");
      throw -1;
    }

  // Write the ID_Server length.
  ACE_UINT16 idServerLength = (ACE_UINT16)length;
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(idServerLength);
  msg->wr_ptr(2);

  // Write the ID_Server
  msg->copy(response->IDServer().data(), idServerLength);

  // Write CSuite Selection
  *(ACE_UINT32*)msg->wr_ptr() = ACE_HTONL(response->CSuiteSelected().Vendor());
  msg->wr_ptr(4);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(response->CSuiteSelected().ChiperSuite());
  msg->wr_ptr(2);

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC. Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());
  msg->copy(response->Mac().data(), macLength);
}

/// EAP-Response/Gpsk4 parser
typedef AAAParser<AAAMessageBlock*, EapResponseGpsk4*>
EapResponseGpsk4Parser;

/// Use this function to convert raw EAP-Response/Gpsk4 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseGpsk4Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpsk4* response = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 4)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (4 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read PD Payload length.
  ACE_UINT16& pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  response->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Read Payload MAC - Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());
  request->Mac() = std::string(msg->rd_ptr(), macLength);
}

/// Use this function to convert application-specific
/// EAP-Response/Gpsk4 data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapResponseGpsk4Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpsk4* reponse = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(reponse);
  responseParser.parseAppToRaw();

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC. Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());
  msg->copy(response->Mac().data(), macLength);
}

/// EAP-Response/GpskFail parser
typedef AAAParser<AAAMessageBlock*, EapResponseGpskFail*>
EapResponseGpskFailParser;

/// Use this function to convert raw EAP/Gpsk-Fail data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseGpskFailParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpskFail* fail = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 5)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (5 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read failure code
  fail->FailureCode() = ACE_NTOHL(*(ACE_UINT32*)msg->rd_ptr());
}

/// Use this function to convert application-specific
/// EAP/Gpsk-Fail data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapResponseGpskFailParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpsk4* fail = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(fail);
  responseParser.parseAppToRaw();

  *(ACE_UINT32*)msg->wr_ptr() = fail->FailureCode();
  msg->wr_ptr(4);
}

/// EAP/Gpsk-Protected-Fail parser
typedef AAAParser<AAAMessageBlock*, EapRequestGpskProtectedFail*>
EapResponseGpskProtectedFailParser;

/// Use this function to convert raw EAP/Gpsk-Protected-Fail data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapResponseGpskProtectedFailParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpskFail* pfail = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 6)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (6 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read failure code
  pfail->FailureCode() = ACE_NTOHL(*(ACE_UINT32*)msg->rd_ptr());

  // Read Payload MAC - Only AES and SHA is supported
  int macLength = getMacLength(pfail->CSuiteSelected());
  pfail->Mac() = std::string(msg->rd_ptr(), macLength);
}

/// Use this function to convert application-specific
/// EAP/Gpsk-Protected-Fail data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapResponseGpskProtectedFailParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapResponseGpsk4* pfail = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(fail);
  responseParser.parseAppToRaw();

  *(ACE_UINT32*)msg->wr_ptr() = pfail->FailureCode();
  msg->wr_ptr(4);

  // Write MAC. Only AES and SHA is supported
  int macLength = getMacLength(response->CSuiteSelected());
  msg->copy(response->Mac().data(), macLength);
}

#endif // __EAP_GPSK_PARSER_H__
