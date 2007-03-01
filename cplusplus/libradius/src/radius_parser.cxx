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

#include <iostream>
#include "radius_parser.h"
#include "radius_dictionary.h"

#define RADIUS_PARSER_DEBUG   1
#if RADIUS_PARSER_DEBUG
#define RADIUS_PARSER_DEBUG_LOG(x)   printf x;
#else
#define RADIUS_PARSER_DEBUG_LOG(x)
#endif

RADIUS_CODES RADIUS_HeaderParser::m_ValidCodes[] = {
   RADIUS_CODE_ACCESS_REQUEST,
   RADIUS_CODE_ACCESS_ACCEPT,
   RADIUS_CODE_ACCESS_REJECT,
   RADIUS_CODE_ACCT_REQUEST,
   RADIUS_CODE_ACCT_RESPONSE,
   RADIUS_CODE_ACCESS_CHLLNG,
   RADIUS_CODE_STATUS_SERVER,
   RADIUS_CODE_STATUS_CLIENT,
   RADIUS_CODE_RESERVED
};

void RADIUS_OctetStringParser::forward(RADIUS_OctetString &strAttr, 
                                       AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.wr_ptr();
   RADIUS_OCTET *length = type + sizeof(RADIUS_OCTET);
   RADIUS_OCTET *data =  length + sizeof(RADIUS_OCTET);
   size_t remainingSpace = buf.size() - buf.length();
            
   if (remainingSpace < strAttr.Value().length()) {
   	   std::string errMsg
   	       ("Parsing error, no more space left on stream buf for type ");
   	   errMsg += strAttr.Type();
       throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
   }
   
   *type = (RADIUS_OCTET)strAttr.Type();   
   *length = (RADIUS_OCTET)(strAttr.Value().length() + (sizeof(RADIUS_OCTET)*2));
   ACE_OS::memcpy(data, strAttr.Value().data(), strAttr.Value().length());
   strAttr.Length() = *length;
   buf.wr_ptr(buf.wr_ptr() + *length);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Octet forward, [%d, %d], Value[%s]\n",
       *type, *length, strAttr.Value().data()));
}

void RADIUS_OctetStringParser::reverse(RADIUS_OctetString &strAttr, 
                                       AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.rd_ptr();
   RADIUS_OCTET *length = (type + sizeof(RADIUS_OCTET));
   RADIUS_OCTET *data = type + sizeof(RADIUS_OCTET)*2;
     
   strAttr.Type() = *type;
   strAttr.Length() = *length;
   strAttr.Value().assign((const char*)data, *length - sizeof(RADIUS_OCTET)*2);
   buf.rd_ptr(buf.rd_ptr() + *length);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Octet reverse, [%d, %d], Value[%s]\n",
       strAttr.Type(), strAttr.Length(), strAttr.Value().data()));
}

void RADIUS_IntegerParser::forward(RADIUS_Integer &intAttr, 
                                   AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.wr_ptr();
   RADIUS_OCTET *length = type + sizeof(RADIUS_OCTET);
   RADIUS_OCTET *data =  length + sizeof(RADIUS_OCTET);
   size_t remainingSpace = buf.size() - buf.length();
            
   if (remainingSpace < sizeof(ACE_UINT32)) {
   	   std::string errMsg
   	       ("Parsing error, no more space left on stream buf for type ");
   	   errMsg += intAttr.Type();
       throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
   }
   
   *type = (RADIUS_OCTET)intAttr.Type();
   *length = sizeof(ACE_UINT32) + (sizeof(RADIUS_OCTET)*2);
   *((ACE_UINT32*)data) = (ACE_UINT32)ACE_HTONL(intAttr.Value());
   intAttr.Length() = *length;
   buf.wr_ptr(buf.wr_ptr() + *length);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Integer forward, [%d, %d], Value[%d]\n",
       *type, *length, intAttr.Value()));
}

void RADIUS_IntegerParser::reverse(RADIUS_Integer &intAttr, 
                                   AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.rd_ptr();
   RADIUS_OCTET *length = (type + sizeof(RADIUS_OCTET));
   RADIUS_OCTET *data = type + sizeof(RADIUS_OCTET)*2;
   
   intAttr.Type() = *type;
   intAttr.Length() = *length;
   intAttr.Value() = ACE_NTOHL(*((ACE_UINT32*)data));
   buf.rd_ptr(buf.rd_ptr() + *length);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Integer reverse, [%d, %d], Value[%d]\n",
       intAttr.Type(), intAttr.Length(), intAttr.Value()));
}

void RADIUS_IpAddressParser::forward(RADIUS_IpAddress &addr, 
                                     AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.wr_ptr();
   RADIUS_OCTET *length = type + sizeof(RADIUS_OCTET);
   RADIUS_OCTET *data =  length + sizeof(RADIUS_OCTET);
   size_t remainingSpace = buf.size() - buf.length();
         
   *type = (RADIUS_OCTET)addr.Type();
   
#if defined (ACE_HAS_IPV6)
   if (addr.Value().get_type() == AF_INET6) {
   	   if (remainingSpace < sizeof(struct in6_addr)) {
           std::string errMsg
   	           ("Parsing error, no more space left on stream buf for type ");
           errMsg += addr.Type();
           throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
       }
       sockaddr_in6 *in = (sockaddr_in6*)addr.Value().get_addr();
       ACE_OS::memcpy(data, (char*)&in->sin6_addr.s6_addr,
                      sizeof(struct in6_addr));
       *length = (RADIUS_OCTET)(sizeof(struct in6_addr) + (sizeof(RADIUS_OCTET)*2));
       
       RADIUS_PARSER_DEBUG_LOG(("PARSER: IPv6 addr forward, [%d, %d]\n",
             *type, *length));
   }
   else if (addr.Value().get_type() == AF_INET) { 
#else
   if (addr.Value().get_type() == AF_INET) { 
#endif
   	   if (remainingSpace < sizeof(ACE_UINT32)) {
           std::string errMsg
   	           ("Parsing error, no more space left on stream buf for type ");
           errMsg += addr.Type();
           throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
   	   }
       ACE_UINT32 ipAddr = ACE_HTONL(addr.Value().get_ip_address());
       ACE_OS::memcpy(data, (char*)&ipAddr, sizeof(ipAddr));
       *length = (RADIUS_OCTET)(sizeof(ACE_UINT32) + (sizeof(RADIUS_OCTET)*2));
       
       RADIUS_PARSER_DEBUG_LOG(("PARSER: IPv4 addr forward, [%d, %d], Value[%x]\n",
             *type, *length, ipAddr));
   }
   addr.Length() = *length;
   buf.wr_ptr(buf.wr_ptr() + *length);
}

void RADIUS_IpAddressParser::reverse(RADIUS_IpAddress &addr, 
                                     AAAMessageBlock &buf)
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.rd_ptr();
   RADIUS_OCTET *length = type + sizeof(RADIUS_OCTET);
   RADIUS_OCTET *data = type + sizeof(RADIUS_OCTET)*2;
   
   if (addr.Value().set_address((const char*)data, 
       *length - sizeof(RADIUS_OCTET)*2) < 0) {
       throw RADIUS_Exception(RADIUS_PARSE_ERROR, 
                 "Cannot convert raw stream to inet address");
   }
   addr.Type() = *type;
   addr.Length() = *length;
   buf.rd_ptr(buf.rd_ptr() + *length);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: IP address reverse, [%d, %d]\n",
       addr.Type(), addr.Length()));
}

void RADIUS_AttrParser::forward(RADIUS_AttrBase *&attr, 
                                AAAMessageBlock &buf) 
{
   try {
       RADIUS_DictAttribute &dict = RADIUS_DICT_ATTR(attr->Type());
       RADIUS_FORWARD_PARSER_MAP(dict.DataType(), attr, buf);
   }
   catch (...) {
   	   std::string errMsg
   	       ("Parsing error, AVP not found in dictionary, avp is " + attr->Type());
       throw RADIUS_Exception(RADIUS_UNKNOWN_AVP, errMsg);
   }
}

void RADIUS_AttrParser::reverse(RADIUS_AttrBase *&attr, 
                                AAAMessageBlock &buf) 
{
   RADIUS_OCTET *type = (RADIUS_OCTET*)buf.rd_ptr();
   RADIUS_OCTET *length = (RADIUS_OCTET*)type + sizeof(RADIUS_OCTET);
   size_t readSpace = m_TotalLength - (buf.rd_ptr() - buf.base());

   if (*length > readSpace) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
                 "AVP has length exceeding packet");
   }
   else if (*length <= 0) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
                 "AVP has length of zero");
   }
   	   
   try {
       RADIUS_DictAttribute &dict = RADIUS_DICT_ATTR(*type);
       RADIUS_REVERSE_PARSER_MAP(dict.DataType(), attr, buf);
   }
   catch (...) {
   	   std::string errMsg
   	       ("Parsing error, AVP not found in dictionary, avp is " + *type);
       throw RADIUS_Exception(RADIUS_UNKNOWN_AVP, errMsg);
   }
}

void RADIUS_AttrListParser::forward(RADIUS_Packet &pkt, 
                                    AAAMessageBlock &buf)
{
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Attribute forward parsing\n"));
   
   RADIUS_AttrParser attrParser;
   RADIUS_AttrListIter i = pkt.Attributes().begin();
   for (; i != pkt.Attributes().end(); i ++) {
   	  attrParser.forward((*i), buf);
   }
}

void RADIUS_AttrListParser::reverse(RADIUS_Packet &pkt, 
                                    AAAMessageBlock &buf)
{
   RADIUS_PARSER_DEBUG_LOG(("PARSER: Attribute reverse parsing\n"));
   
   RADIUS_AttrBase *attr = NULL;
   RADIUS_AttrParser attrParser(buf.length() + RADIUS_PKT_MIN_SIZE);
   while ((buf.rd_ptr() - buf.base()) < pkt.Length()) {
   	  attrParser.reverse(attr, buf);
   	  pkt.Attributes().Add(attr);
   }
}

void RADIUS_HeaderParser::reverse(RADIUS_Packet &pkt, 
                                  AAAMessageBlock &buf)
{
   if (buf.size() < RADIUS_PKT_MIN_SIZE) {
       std::string errMsg
  	           ("Parsing error, no more space left on buf for msg ");
       errMsg += pkt.Code();
       throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
   }
   
   RADIUS_OCTET *p = (RADIUS_OCTET*)buf.rd_ptr();

   pkt.Code() = *p; p++;
   pkt.Id() = *p; p++;
   pkt.Length() = ACE_NTOHS(*((RADIUS_SHORT*)p)); p += 2;
   ACE_OS::memcpy(pkt.Authenticator(), p, 
                  sizeof(RADIUS_OCTET)*
                  RADIUS_PktHeader::AuthenticatorLen);
   
   p += sizeof(RADIUS_OCTET)*RADIUS_PktHeader::AuthenticatorLen;
   
   if (pkt.Length() > RADIUS_PKT_MAX_SIZE) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
  	                          "Parsing error, packet length greater than allowed");
   }
   else if (pkt.Length() < RADIUS_PKT_MIN_SIZE) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
  	                          "Parsing error, packet less than allowed radius header");
   }
   else if (! ValidateCode(pkt)) {
       throw RADIUS_Exception(RADIUS_UNKNOWN_CODE, 
  	                          "Parsing error, packet has unknown msg code");
   }
   
   buf.rd_ptr((char*)p);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: message header reverse, [%d, %d, %d], Auth[%d,%d,%d,%d]\n",
       pkt.Code(), pkt.Id(), pkt.Length(), 
       ((int*)pkt.Authenticator())[0],
       ((int*)pkt.Authenticator())[1],
       ((int*)pkt.Authenticator())[2],
       ((int*)pkt.Authenticator())[3]));
}

void RADIUS_HeaderParser::forward(RADIUS_Packet &pkt, 
                                  AAAMessageBlock &buf)
{
   if (buf.size() < RADIUS_PKT_MIN_SIZE) {
       std::string errMsg
  	           ("Parsing error, raw buffer less than header size for msg ");
       errMsg += pkt.Code();
       throw RADIUS_Exception(RADIUS_BUF_TO_SMALL, errMsg);
   }
   else if (! ValidateCode(pkt)) {
       throw RADIUS_Exception(RADIUS_UNKNOWN_CODE, 
  	                          "Parsing error, packet has unknown msg code");
   }
   
   buf.wr_ptr(buf.base());

   RADIUS_OCTET *p = (RADIUS_OCTET*)buf.base();

   *(p) = pkt.Code();
   *(++p) = pkt.Id();
   *((RADIUS_SHORT*)(++p)) = ACE_HTONS(pkt.Length());
   p += 2;
   ACE_OS::memcpy(p, pkt.Authenticator(),
                  sizeof(RADIUS_OCTET)*
                  RADIUS_PktHeader::AuthenticatorLen);

   p += sizeof(RADIUS_OCTET)*RADIUS_PktHeader::AuthenticatorLen;
   buf.wr_ptr((char*)p);
   
   RADIUS_PARSER_DEBUG_LOG(("PARSER: message header forward, [%d, %d, %d], Auth[%d,%d,%d,%d]\n",
       pkt.Code(), pkt.Id(), pkt.Length(), 
       ((int*)pkt.Authenticator())[0],
       ((int*)pkt.Authenticator())[1],
       ((int*)pkt.Authenticator())[2],
       ((int*)pkt.Authenticator())[3]));
}

void RADIUS_PacketParser::forward(RADIUS_Packet &pkt, 
                                  AAAMessageBlock &buf)
{
   buf.rd_ptr(buf.base());
   buf.wr_ptr(buf.base() + RADIUS_PKT_MIN_SIZE);
   
   RADIUS_AttrListParser attrParser;
   attrParser.forward(pkt, buf);
   
   pkt.Length() = RADIUS_PKT_MIN_SIZE;
   RADIUS_AttrListIter i = pkt.Attributes().begin();
   for (; i != pkt.Attributes().end(); i ++) {
   	  pkt.Length() += (*i)->Length();
   }   
   
   buf.wr_ptr(buf.base());
   RADIUS_HeaderParser hdrParser;
   hdrParser.forward(pkt, buf);   
   buf.wr_ptr(buf.base() + pkt.Length());
}

void RADIUS_PacketParser::reverse(RADIUS_Packet &pkt, 
                                  AAAMessageBlock &buf)
{
   if (buf.length() > RADIUS_PKT_MAX_SIZE) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
  	                          "Parsing error, udp packet length greater than allowed");
   }
   else if (buf.size() < RADIUS_PKT_MIN_SIZE) {
       throw RADIUS_Exception(RADIUS_MALFORMED_PACKET, 
  	                          "Parsing error, udp packet less than allowed radius header");
   }

   buf.rd_ptr(buf.base());
   
   RADIUS_HeaderParser hdrParser;
   hdrParser.reverse(pkt, buf);
   
   RADIUS_AttrListParser attrParser;
   attrParser.reverse(pkt, buf);
}

void RADIUS_PacketDump::Dump(RADIUS_Packet &pkt)
{
	std::cout << "*** packet dump ***"        << std::endl;
	std::cout << "Code   : " << (int)pkt.Code()    << std::endl;
	std::cout << "Id     : " << (int)pkt.Id()      << std::endl;
	std::cout << "Length : " << (int)pkt.Length()  << std::endl;
    std::cout << "Auth   : " << std::endl;
	for (int i = 0; i < RADIUS_PktHeader::AuthenticatorLen; i++) {
        std::cout << pkt.Authenticator()[i] << std::endl;
	}
	
    RADIUS_AttrListIter i = pkt.Attributes().begin();
    for (; i != pkt.Attributes().end(); i ++) {
        std::cout << "  AVP  :" << (int)(*i)->Type();
        std::cout << ", Length :" << (int)(*i)->Length() << std::endl;	
    }	
}
