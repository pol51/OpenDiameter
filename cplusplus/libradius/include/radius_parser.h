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

#ifndef __RADIUS_PARSER_H__
#define __RADIUS_PARSER_H__

#include "radius_packet.h"
#include "diameter_parser.h"

template<class SRC, class DEST>
class RADIUS_Parser
{
   public:
      virtual void forward(SRC &src, DEST &dest) = 0;
      virtual void reverse(SRC &src, DEST &dest) = 0;
      virtual ~RADIUS_Parser() { }
};

class RADIUS_OctetStringParser : 
   public 
      RADIUS_Parser<RADIUS_OctetString, AAAMessageBlock>
{
   public:
      void forward(RADIUS_OctetString &attr, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_OctetString &attr, 
                   AAAMessageBlock &buf);
};

typedef RADIUS_OctetStringParser   RADIUS_UTF8StringParser;

class RADIUS_IntegerParser : 
   public RADIUS_Parser<RADIUS_Integer, AAAMessageBlock>
{
   public:
      void forward(RADIUS_Integer &attr, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_Integer &attr, 
                   AAAMessageBlock &buf);
};

typedef RADIUS_IntegerParser RADIUS_TimeParser;
typedef RADIUS_IntegerParser RADIUS_InterfaceIdParser;

class RADIUS_IpAddressParser : 
   public RADIUS_Parser<RADIUS_IpAddress, AAAMessageBlock>
{
   public:
      void forward(RADIUS_IpAddress &attr, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_IpAddress &attr, 
                   AAAMessageBlock &buf);
};

typedef RADIUS_IpAddressParser RADIUS_IPv4AddressParser;
typedef RADIUS_IpAddressParser RADIUS_IPv6AddressParser;
typedef RADIUS_IpAddressParser RADIUS_IPv6PrefixParser;

class RADIUS_AttrParser : public RADIUS_Parser<RADIUS_AttrBase*,
                                               AAAMessageBlock>
{
   public:
      RADIUS_AttrParser(int tlen = 0) : 
         m_TotalLength(tlen) {
      }
      void forward(RADIUS_AttrBase *&attr, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_AttrBase *&attr, 
                   AAAMessageBlock &buf);
   private:
      int m_TotalLength;
};

class RADIUS_AttrListParser : public RADIUS_Parser<RADIUS_Packet,
                                                   AAAMessageBlock>
{
   public:
      void forward(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
};

class RADIUS_HeaderParser : public RADIUS_Parser<RADIUS_Packet,
                                                 AAAMessageBlock>
{
   public:
      void forward(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
                   
   private:
      static RADIUS_CODES m_ValidCodes[RADIUS_NUM_VALID_MSG];
      bool ValidateCode(RADIUS_Packet &pkt) {
      	  for (int i = 0; i < RADIUS_NUM_VALID_MSG; i ++) {
      	  	  if (pkt.Code() == m_ValidCodes[i]) {
      	  	  	  return true;
      	  	  }
      	  }
      	  return false;
      }
};

class RADIUS_PacketParser : public RADIUS_Parser<RADIUS_Packet,
                                                 AAAMessageBlock>
{
   public:
      void forward(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
      void reverse(RADIUS_Packet &pkt, 
                   AAAMessageBlock &buf);
};

class RADIUS_DataTypeParserMap
{
	protected:
       class AttrParser : 
           public RADIUS_Parser<RADIUS_AttrBase*, AAAMessageBlock> {
       };

       template <class ATTR, class PARSER>
       class TypeAllocatorAndParser :
           public AttrParser {
           public:
              void forward(RADIUS_AttrBase *&attr, AAAMessageBlock &buf) {
              	 m_Parser.forward((ATTR&)*attr, buf);
              }
              void reverse(RADIUS_AttrBase *&attr, AAAMessageBlock &buf) {
                 attr = new ATTR;
                 if (attr) {
                     m_Parser.reverse((ATTR&)*attr, buf);
                 }
                 else {
                     throw RADIUS_Exception(RADIUS_ALLOC_ERROR, 
  	                             "Allocation error in attr forward parsing");
                 }
              }
           protected:
              PARSER m_Parser;
       };
      
	public:
	  RADIUS_DataTypeParserMap() {
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("string"), &m_Utf8Parser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("integer"), &m_IntParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("ipaddr"), &m_IPv4AddrParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("date"), &m_TimeParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("ifid"), &m_IfIdParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("ipv6addr"), &m_IPv6AddrParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("ipv6prefix"), &m_IPv6PrefParser));
	  	 m_ParserMap.insert(std::pair<std::string, AttrParser*>
	  	                   (std::string("octet"), &m_OctetParser));
	  };
	  
      void reverse(std::string &type, RADIUS_AttrBase *&attr, AAAMessageBlock &buf) {
      	 std::map<std::string, AttrParser*>::iterator i = 
      	    m_ParserMap.find(type);
      	 if (i != m_ParserMap.end()) {
      	 	 i->second->reverse(attr, buf);
      	 }
      }
      void forward(std::string &type, RADIUS_AttrBase *&attr, AAAMessageBlock &buf) {
      	 std::map<std::string, AttrParser*>::iterator i = 
      	    m_ParserMap.find(type);
      	 if (i != m_ParserMap.end()) {
      	 	i->second->forward(attr, buf);
      	 }
      }
	
    protected:
       std::map<std::string, AttrParser*> m_ParserMap;
      
	private:
       TypeAllocatorAndParser<RADIUS_UTF8String, 
                              RADIUS_UTF8StringParser> 
                              m_Utf8Parser;
       TypeAllocatorAndParser<RADIUS_OctetString, 
                              RADIUS_OctetStringParser> 
                              m_OctetParser;
       TypeAllocatorAndParser<RADIUS_Integer, 
                              RADIUS_IntegerParser> 
                              m_IntParser;
       TypeAllocatorAndParser<RADIUS_Time, 
                              RADIUS_TimeParser> 
                              m_TimeParser;
       TypeAllocatorAndParser<RADIUS_InterfaceId, 
                              RADIUS_InterfaceIdParser> 
                              m_IfIdParser;
       TypeAllocatorAndParser<RADIUS_IPv4Address, 
                              RADIUS_IPv4AddressParser> 
                              m_IPv4AddrParser;
       TypeAllocatorAndParser<RADIUS_IPv6Address, 
                              RADIUS_IPv6AddressParser> 
                              m_IPv6AddrParser;
       TypeAllocatorAndParser<RADIUS_IPv6Prefix, 
                              RADIUS_IPv6PrefixParser> 
                              m_IPv6PrefParser;
};

typedef ACE_Singleton<RADIUS_DataTypeParserMap, 
                      ACE_Null_Mutex> 
                      RADIUS_DataTypeParserMap_S;

#define RADIUS_FORWARD_PARSER_MAP(x, y, z)  RADIUS_DataTypeParserMap_S::instance()->forward((x), (y), (z))
#define RADIUS_REVERSE_PARSER_MAP(x, y, z)  RADIUS_DataTypeParserMap_S::instance()->reverse((x), (y), (z))

class RADIUS_PacketDump
{
	public:
	   static void Dump(RADIUS_Packet &pkt);
};

#endif // __RADIUS_PARSER_H__

