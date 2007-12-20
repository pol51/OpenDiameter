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
typedef AAAParser<AAAMessageBlock*, EapGpskMsg*>
EapGpskMsgParser;

inline void
readCipherSuiteList(std::string &strlist, int length,
  EapGpskCipherSuiteList &clist)
{
  unsigned char *cc;
  int csuiteLength = length/6;
  for (int i = 0; i < csuiteLength; i++)
    {
      EapGpskCipherSuite csuite;
      cc = (unsigned char*)strlist.data();
      csuite.Vendor() = ACE_NTOHL(*(ACE_UINT32*)cc);
      cc += 4;
      csuite.CipherSuite() = ACE_NTOHS(*(ACE_UINT16*)cc);
      cc += 2;
      clist.push_back(csuite);
    }
}

inline void
writeCipherSuiteList(std::string &strlist,
  EapGpskCipherSuiteList &clist)
{
  std::string *container = new std::string(clist.size() * 6, 0);
  unsigned char *cc = (unsigned char*)container->data();
  std::list<EapGpskCipherSuite>::iterator i = clist.begin();
  for (; i != clist.end(); i++)
    {
      EapGpskCipherSuite csuite = *i;
      *(ACE_UINT32*)cc = ACE_HTONL(csuite.Vendor());
      cc += 4;
      *(ACE_UINT16*)cc = ACE_HTONS(csuite.CipherSuite());
      cc += 2;
    }
  strlist = *container;
  delete container;
}

/// Use this function to convert raw EAP-Request/Gpsk data (data
/// following the Type field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the Op-Code field.
template<> inline void
EapGpskMsgParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskMsg* gpsk = getAppData();

  // Read Op-Code.
  gpsk->OpCode() = *(ACE_Byte*)msg->rd_ptr();
  msg->rd_ptr(1);
  return;
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk payload data to raw EAP-Request/Gpsk-Request
/// payload data (data following the MsgID field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the MsgID field.
//void EapGpskMsgParser::parseAppToRaw();
template<> inline void
EapGpskMsgParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskMsg* gpsk = getAppData();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = gpsk->OpCode();
  msg->wr_ptr(1);
  return;
}


/// EAP-Request/Gpsk-Request parser
typedef AAAParser<AAAMessageBlock*, EapGpsk1*>
EapGpsk1Parser;

/// Use this function to convert raw EAP-Request/Gpsk1 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void
EapGpsk1Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk1* gpsk = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 1)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (1 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read ID_Server length.
  ACE_UINT16 idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  gpsk->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read RAND_Server.
  gpsk->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read CSuite List length.
  ACE_UINT16 csuiteLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  if (csuiteLength % 6)
    {
      EAP_LOG(LM_ERROR, "Invalid chiper suite length %d (not divisible by 6).\n", csuiteLength);
      throw -1;
    }

  // Read CSuite List.
  std::string cipherList = std::string(msg->rd_ptr(), csuiteLength);
  msg->rd_ptr(csuiteLength);

  readCipherSuiteList(cipherList, csuiteLength, gpsk->CSuiteList());
  msg->rd_ptr(csuiteLength * 6);
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk1 payload data to raw EAP-Request/Gpsk1
/// payload data (data following the Op-Code field).  As a result of
/// calling this function, the write pointer of the message block
/// points to one octet after the end of the payload.
//void EapRequestGpskRequestParser::parseAppToRaw();
template<> inline void 
EapGpsk1Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk1* gpsk = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(gpsk);
  requestParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = gpsk->OpCode();
  msg->wr_ptr(1);

  // Check ID_Server length.
  int length = gpsk->IDServer().size();
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
  msg->copy(gpsk->IDServer().data(), idServerLength);

  // Write RAND_Server
  msg->copy(gpsk->RANDServer().data(), 32);

  // Check CSuite List length.
  length = gpsk->CSuiteList().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "CSuite List is empty.");
      throw -1;
    }

  // Write CSuite list length
  ACE_UINT16 csuiteLength = (ACE_UINT16)(gpsk->CSuiteList().size() * 6);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(csuiteLength);
  msg->wr_ptr(2);

  // Write CSuite list
  std::string strlist;
  writeCipherSuiteList(strlist, gpsk->CSuiteList());
  msg->copy(strlist.data(), csuiteLength);

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK1.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

/// EAP-Response/Gpsk-Response parser
typedef AAAParser<AAAMessageBlock*, EapGpsk2*>
EapGpsk2Parser;

/// Use this function to convert raw EAP-Response/Gpsk2 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void
EapGpsk2Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk2* gpsk = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 2)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (2 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read ID_Peer length.
  ACE_UINT16 idPeerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Peer.
  gpsk->IDPeer() = std::string(msg->rd_ptr(), idPeerLength);
  msg->rd_ptr(idPeerLength);

  // Read ID_Server length.
  ACE_UINT16 idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  gpsk->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read RAND_Peer.
  gpsk->RANDPeer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read RAND_Server.
  gpsk->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read CSuite List length.
  ACE_UINT16 csuiteLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read CSuite List.
  std::string cipherList = std::string(msg->rd_ptr(), csuiteLength);
  msg->rd_ptr(csuiteLength);
  readCipherSuiteList(cipherList, csuiteLength, gpsk->CSuiteList());

  // Read CSuite Selected.
  gpsk->CSuiteSelected().fromString(msg->rd_ptr());
  msg->rd_ptr(6);

  // Read PD Payload length.
  ACE_UINT16 pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  gpsk->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Read Payload MAC
  gpsk->MAC() = std::string(msg->rd_ptr(), gpsk->CSuiteSelected().KeySize());
  msg->rd_ptr(gpsk->CSuiteSelected().KeySize());
}

/// Use this function to convert application-specific
/// EAP-Response/Gpsk2 payload data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
//void EapResponseGpskResponseParser::parseAppToRaw();
template<> inline void 
EapGpsk2Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk2* gpsk = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(gpsk);
  responseParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = gpsk->OpCode();
  msg->wr_ptr(1);

  // Check ID_Peer length.
  int length = gpsk->IDPeer().size();
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
  msg->copy(gpsk->IDPeer().data(), idPeerLength);

  // Check ID_Server length.
  length = gpsk->IDServer().size();
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
  msg->copy(gpsk->IDServer().data(), idServerLength);

  // Write RAND_Peer
  msg->copy(gpsk->RANDPeer().data(), 32);

  // Write RAND_Server
  msg->copy(gpsk->RANDServer().data(), 32);

  // Check CSuite List length.
  length = gpsk->CSuiteList().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "CSuite List is empty.");
      throw -1;
    }

  // Write CSuite list length
  ACE_UINT16 csuiteLength = (ACE_UINT16)(gpsk->CSuiteList().size() * 6);
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(csuiteLength);
  msg->wr_ptr(2);

  // Write CSuite list
  std::string strlist;
  writeCipherSuiteList(strlist, gpsk->CSuiteList());
  msg->copy(strlist.data(), csuiteLength);

  // Write CSuite Selection
  std::string cipherSuiteSelected = gpsk->CSuiteSelected().toString();
  msg->copy(cipherSuiteSelected.data(), 6);

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC.
  if (gpsk->MAC().size() > 0)
  {
     msg->copy(gpsk->MAC().data(), gpsk->CSuiteSelected().KeySize());
  }

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK2.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

/// EAP-Request/Gpsk-Confirm parser
typedef AAAParser<AAAMessageBlock*, EapGpsk3*>
EapGpsk3Parser;

/// Use this function to convert raw EAP-Request/Gpsk3 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void 
EapGpsk3Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk3* gpsk = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 3)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (3 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read RAND_Peer.
  gpsk->RANDPeer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read RAND_Server.
  gpsk->RANDServer() = std::string(msg->rd_ptr(), 32);
  msg->rd_ptr(32);

  // Read ID_Server length.
  ACE_UINT16 idServerLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read ID_Server.
  gpsk->IDServer() = std::string(msg->rd_ptr(), idServerLength);
  msg->rd_ptr(idServerLength);

  // Read CSuite Selected.
  gpsk->CSuiteSelected().fromString(msg->rd_ptr());
  msg->rd_ptr(6);

  // Read PD Payload length.
  ACE_UINT16 pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  gpsk->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Read Payload MAC
  gpsk->MAC() = std::string(msg->rd_ptr(), gpsk->CSuiteSelected().KeySize());
  msg->rd_ptr(gpsk->CSuiteSelected().KeySize());
}

/// Use this function to convert application-specific
/// EAP-Request/Gpsk3 data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapGpsk3Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk3* gpsk = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(gpsk);
  requestParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = gpsk->OpCode();
  msg->wr_ptr(1);

  // Write RAND_Peer
  msg->copy(gpsk->RANDPeer().data(), 32);

  // Write RAND_Server
  msg->copy(gpsk->RANDServer().data(), 32);

  // Check ID_Server length.
  ACE_UINT16 length = gpsk->IDServer().size();
  if (length == 0)
    {
      EAP_LOG(LM_ERROR, "ID_Server is empty.");
      throw -1;
    }

  // Write the ID_Server length.
  *(ACE_UINT16*)msg->wr_ptr() = ACE_HTONS(length);
  msg->wr_ptr(2);

  // Write the ID_Server
  msg->copy(gpsk->IDServer().data(), length);

  // Write CSuite Selection
  std::string cipherSuiteSelected = gpsk->CSuiteSelected().toString();
  msg->copy(cipherSuiteSelected.data(), 6);

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC.
  if (gpsk->MAC().size() > 0)
  {
     msg->copy(gpsk->MAC().data(), gpsk->CSuiteSelected().KeySize());
  }

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK3.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

/// EAP-Response/Gpsk4 parser
typedef AAAParser<AAAMessageBlock*, EapGpsk4*>
EapGpsk4Parser;

/// Use this function to convert raw EAP-Response/Gpsk4 data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void
EapGpsk4Parser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk4* gpsk = getAppData();

  ACE_Byte opCode = *(ACE_Byte*)msg->rd_ptr();
  // Read Op-Code.
  if (opCode != 4)
    {
      EAP_LOG(LM_ERROR, "Invalid opCode %d (4 is expected).\n", opCode);
      throw -1;
    }

  msg->rd_ptr(1);

  // Read PD Payload length.
  ACE_UINT16 pdPayloadLength = ACE_NTOHS(*(ACE_UINT16*)msg->rd_ptr());
  msg->rd_ptr(2);

  // Read PD Payload
  gpsk->PDPayload() = std::string(msg->rd_ptr(), pdPayloadLength);
  msg->rd_ptr(pdPayloadLength);

  // Read Payload MAC
  gpsk->MAC() = std::string(msg->rd_ptr(), msg->wr_ptr() - msg->rd_ptr());
  msg->rd_ptr(msg->wr_ptr());
}

/// Use this function to convert application-specific
/// EAP-Response/Gpsk4 data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapGpsk4Parser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpsk4* gpsk = getAppData();

  // Write type field
  EapResponseParser responseParser;
  responseParser.setRawData(msg);
  responseParser.setAppData(gpsk);
  responseParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = gpsk->OpCode();
  msg->wr_ptr(1);

  // Write payload length [TBD: no payload supported set this to 0]
  *(ACE_UINT16*)msg->wr_ptr() = 0;
  msg->wr_ptr(2);

  // Write MAC.
  if (gpsk->MAC().size() > 0)
  {
     msg->copy(gpsk->MAC().data(), gpsk->MAC().size());
  }

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK4.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

/// EAP-Request for GpskFail parser
typedef AAAParser<AAAMessageBlock*, EapGpskFail*>
EapGpskFailParser;

/// Use this function to convert raw EAP/Gpsk-Fail data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void
EapGpskFailParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskFail* fail = getAppData();

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
  msg->rd_ptr(4);
}

/// Use this function to convert application-specific
/// EAP/Gpsk-Fail data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapGpskFailParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskFail* fail = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(fail);
  requestParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = fail->OpCode();
  msg->wr_ptr(1);

  // Write Failure Code.
  *(ACE_UINT32*)msg->wr_ptr() = ACE_HTONL(fail->FailureCode());
  msg->wr_ptr(4);

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK-Fail.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

/// EAP/Gpsk-Protected-Fail parser
typedef AAAParser<AAAMessageBlock*, EapGpskProtectedFail*>
EapGpskProtectedFailParser;

/// Use this function to convert raw EAP/Gpsk-Protected-Fail data (data
/// following the Op-Code field) to application-specific payload data.
/// As a result of calling this function, the read pointer of the
/// message block points to one octet after the end of the payload.
template<> inline void
EapGpskProtectedFailParser::parseRawToApp()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskProtectedFail* pfail = getAppData();

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
  msg->rd_ptr(4);

  // Read Payload MAC
  pfail->MAC() = std::string(msg->rd_ptr(), msg->wr_ptr() - msg->rd_ptr());
  msg->rd_ptr(msg->wr_ptr());
}

/// Use this function to convert application-specific
/// EAP/Gpsk-Protected-Fail data to raw payload data
/// (data following the Op-Code field).  As a result of calling this
/// function, the write pointer of the message block points to one
/// octet after the end of the payload.
template<> inline void
EapGpskProtectedFailParser::parseAppToRaw()
{
  AAAMessageBlock* msg = getRawData();
  EapGpskProtectedFail* pfail = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(pfail);
  requestParser.parseAppToRaw();

  // Write Op-Code.
  *(ACE_Byte*)msg->wr_ptr() = pfail->OpCode();
  msg->wr_ptr(1);

  // Write Failure Code.
  *(ACE_UINT32*)msg->wr_ptr() = ACE_HTONL(pfail->FailureCode());
  msg->wr_ptr(4);

  // Write MAC.
  if (pfail->MAC().size() > 0)
  {
     msg->copy(pfail->MAC().data(), pfail->MAC().size());
  }

  if ((size_t)msg->wr_ptr() - (size_t)msg->base() < 0)
    {
      EAP_LOG(LM_ERROR, "Buffer overflow on GPSK-Protected-Fail.");
      throw -1;
    }

  // resize buffer
  msg->size(msg->wr_ptr() - msg->base());
}

#endif // __EAP_GPSK_PARSER_H__
