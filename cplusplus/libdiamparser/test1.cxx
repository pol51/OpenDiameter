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
/* $Id: test1.cxx,v 1.3 2006/05/31 17:53:33 vfajardo Exp $ */
/* 
   Sample program to show how to use diamparser library. 
   Written by Yoshihihiro Ohba (yohba@tari.toshiba.com)
*/

#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <string>
#include <iostream>
#include "diameter_parser_api.h"
using namespace std;

#if defined PROTOID_TEST
#define HEADERPARSER HeaderParserWithProtocol
#define SETOPTION(hp, x, y) AAADictionaryOption opt(x, y); hp.setDictData(&opt);
#else
#define HEADERPARSER HeaderParser
#define SETOPTION(hp, x, y) hp.setDictData(x);
#endif

#define GET_DATA_REF(dataType, data, containerEntryPtr) \
        dataType &data = (containerEntryPtr)->dataRef(Type2Type<dataType>())

unsigned char rbuf[] = 
{ 
  //  0x01, 0x00, 0x00, 0xbc, 0xc0, 0x98, 0x96, 0x7f, // header (length = 188)
  0x01, 0x00, 0x01, 0x0c, 0xc0, 0x98, 0x96, 0x7f, // header (length = 268)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x0a, 
                          0x00, 0x00, 0x01, 0x1c, // Proxy Info (Grouped) 
  0x40, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x01, 0x18, //  Proxy Host
  0x40, 0x00, 0x00, 0x13, 'b', 'b', 'b', '.',     
  'c', 'c', 'c', '.', 'c', 'o', 'm', 0x00, 
  0x00, 0x00, 0x00, 0x21, 0x40, 0x00, 0x00, 0x0d, //  Proxy-State
  'a',  'b',  'c',  'd',  'e',  0x00, 0x00, 0x00, 

  0x00, 0x00, 0x01, 0x03, 0x40, 0x00, 0x00, 0x0c, // Acct-Application-Id(1)
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, // Acct-Application-Id(2)
  0x40, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x02, 
  0x00, 0x00, 0x01, 0x25, 0x40, 0x00, 0x00, 0x0f, // Destination-Host
  'a', 'a', 'a', '.', 'c', 'o', 'm', 0x00,    
  0x00, 0x00, 0x01, 0x1a, 0x40, 0x00, 0x00, 0x0f, // Route-Record
  'x', 'x', 'x', '.', 'c', 'o', 'm', 0x00,
  0x00, 0x0f, 0x42, 0x3e, 0x40, 0x00, 0x00, 0x43, // Example-URI
  'a', 'a', 'a', ':', '/', '/', 'h', 'o',         
  's', 't', '.', 'e', 'x', 'a', 'm', 'p',         
  'l', 'e', '.', 'c', 'o', 'm', ':', '6',
  '6', '6', '6', ';', 't', 'r', 'a', 'n',
  's', 'p', 'o', 'r', 't', '=', 't', 'c',
  'p', ';', 'p', 'r', 'o', 't', 'o', 'c',
  'o', 'l', '=', 'd', 'i', 'a', 'm', 'e', 
  't', 'e', 'r', 0x00, 0x00, 0x0f, 0x42, 0x3d,   // Example-IPFilterRule
  0x40, 0x00, 0x00, 0x4e, 'p', 'e', 'r', 'm',    // "permit in ip from assigned 
  'i', 't', ' ', 'i', 'n', ' ', 'i', 'p',        //  to 10.0.1.0/24 22,1000-2000
  ' ', 'f', 'r', 'o', 'm', ' ', 'a', 's',        //  ipoptions !ssrr"
  's', 'i', 'g', 'n', 'e', 'd', ' ', 't', 
  'o', ' ', '1', '0', '.', '0', '.', '1', 
  '.', '0', '/', '2', '4', ' ', '2', '2',
  ',', '1', '0', '0', '0', '-', '2', '0',
  '0', '0', ' ', 'i', 'p', 'o', 'p', 't', 
  'i', 'o', 'n', 's', ' ', '!', 's', 's',
  'r', 'r', 0x00, 0x00
};

unsigned char sbuf[1024];

char addr[] = {0x0a, 0x01, 0x01, 0x01};
//char prstate_str[] = "abcde";
//char dhost_str[] = { "aaa.com"};
char uname_str[] = { "ohba@tari.toshiba.com"};

char rAvp[1024];
int  rAvp_len;
#if 0
char sAvp[] = 
{
  0x00, 0x00, 0x01, 0x17, 0x00, 0x00, 0x00, 0x28, /* ANY (Failed-AVP) */
  0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x1d, /* ANY (User-Name) */
  'o', 'h', 'b', 'a', '@', 't', 'a', 'r', 
  'i', '.', 't', 'o', 's', 'h', 'i', 'b', 
  'a', '.', 'c', 'o', 'm', 0x00, 0x00, 0x00
};
int  sAvp_len = sizeof(sAvp);
#endif

void
print_header(AAADiameterHeader &h)
{
  cout << "version = " << (int)h.ver << endl;
  cout << "length=" << h.length << endl;
  cout << "flags(r,p,e,t)=(" 
       << (int)h.flags.r << "," 
       << (int)h.flags.p << ","
       << (int)h.flags.e << ","
       << (int)h.flags.t << ")" << endl;
  cout << "command = " << h.code << endl;
  cout << "applicationId = " << h.appId << endl;
  cout << "h-h id = " << h.hh << endl;
  cout << "e-e id = " << h.ee << endl;
}

void
print_diamuri(diameter_uri_t uri)
{
  ACE_OS::printf("%s://%s:%d;transport=%s;protocol=%s\n", 
		 uri.scheme == AAA_SCHEME_AAA ? "aaa" : "aaas", 
		 uri.fqdn.c_str(), uri.port, 
		 uri.transport == TRANSPORT_PROTO_TCP ? "tcp":"sctp",
		 uri.protocol == AAA_PROTO_DIAMETER ? "diameter":"radius");
}

void
print_ipfilter_rule(diameter_ipfilter_rule_t& r)
{
  std::string str;
      if (r.action == AAA_IPFILTER_RULE_ACTION_PERMIT)
	str.append("permit ");
      else
	str.append("deny ");

      if (r.dir == AAA_IPFILTER_RULE_DIRECTION_IN)
	str.append("in ");
      else
	str.append("out ");
      if (r.proto == 0)
	str.append("ip ");
      else
	{
	  char protoStr[4];
	  ACE_OS::sprintf(protoStr, "%u ", r.proto);
	  str.append(protoStr);
	}
      str.append("from ");
      if (!r.src.modifier)
	str.append("!");
      if (r.src.representation == AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY)
	str.append("any ");
      else if (r.src.representation 
	       == AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED)
	str.append("assigned ");
      else {
	str.append(r.src.ipno);
	if (r.src.representation == AAA_IPFILTER_RULE_SRCDST_MASK)
	  {
	    char bitsStr[5];
	    ACE_OS::sprintf(bitsStr, "/%u", r.src.bits);
	    str.append(bitsStr);
	  }
	str.append(" ");
      }
      for (std::list<AAA_UINT16_RANGE>::iterator i=r.src.portRangeList.begin();
	   i!=r.src.portRangeList.end();)
	{
	  char portStr[12];
	  if ((*i).first == (*i).last)
	    {
	      ACE_OS::sprintf(portStr, "%d", (*i).first);
	      str.append(portStr);
	    }
	  else
	    {
	      ACE_OS::sprintf(portStr, "%d-%d", (*i).first, (*i).last);
	      str.append(portStr);
	    }
	  if (++i != r.src.portRangeList.end())
	    str.append(",");
	  else
	    str.append(" ");
	}
      str.append("to ");
      if (!r.dst.modifier)
	str.append("!");
      if (r.dst.representation == AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY)
	str.append("any ");
      else if (r.dst.representation 
	       == AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED)
	str.append("assigned ");
      else {
	str.append(r.dst.ipno);
	if (r.dst.representation == AAA_IPFILTER_RULE_SRCDST_MASK)
	  {
	    char bitsStr[5];
	    ACE_OS::sprintf(bitsStr, "/%u", r.dst.bits);
	    str.append(bitsStr);
	  }
	str.append(" ");
      }
      for (std::list<AAA_UINT16_RANGE>::iterator i=r.dst.portRangeList.begin();
	   i!=r.dst.portRangeList.end();)
	{
	  char portStr[12];
	  if ((*i).first == (*i).last)
	    ACE_OS::sprintf(portStr, "%d", (*i).first);
	  else
	    ACE_OS::sprintf(portStr, "%d-%d", (*i).first, (*i).last);
	  str.append(portStr);
	  if (++i != r.dst.portRangeList.end())
	    str.append(",");
	  else
	    str.append(" ");
	}
      if (r.frag)
	str.append(" frag");
      
      for (std::list<int>::iterator i=r.ipOptionList.begin();
	   i!=r.ipOptionList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == AAA_IPFILTER_RULE_IP_OPTION_SSRR)
	    str.append("ssrr");
	  else if (opt == AAA_IPFILTER_RULE_IP_OPTION_LSRR)
	    str.append("lsrr");
	  else if (opt == AAA_IPFILTER_RULE_IP_OPTION_RR)
	    str.append("rr");
	  else if (opt == AAA_IPFILTER_RULE_IP_OPTION_TS)
	    str.append("ts");
	  if (++i != r.ipOptionList.end())
	    str.append(",");
	}
      

      if (r.tcpOptionList.size()>0)
	str.append(" tcpoptions");
      
      for (std::list<int>::iterator i=r.tcpOptionList.begin();
	   i!=r.tcpOptionList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == AAA_IPFILTER_RULE_TCP_OPTION_MSS)
	    str.append("mss");
	  else if (opt == AAA_IPFILTER_RULE_TCP_OPTION_WINDOW)
	    str.append("window");
	  else if (opt == AAA_IPFILTER_RULE_TCP_OPTION_SACK)
	    str.append("sack");
	  else if (opt == AAA_IPFILTER_RULE_TCP_OPTION_TS)
	    str.append("ts");
	  else if (opt == AAA_IPFILTER_RULE_TCP_OPTION_CC)
	    str.append("cc");
	  if (++i != r.tcpOptionList.end())
	    str.append(",");
	}
      

      if (r.established)
	str.append(" established");
      
      if (r.setup)
	str.append(" setup");

      if (r.tcpFlagList.size()>0)
	str.append(" tcpflags");
      
      for (std::list<int>::iterator i=r.tcpFlagList.begin();
	   i!=r.tcpFlagList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == AAA_IPFILTER_RULE_TCP_FLAG_FIN)
	    str.append("fin");
	  else if (opt == AAA_IPFILTER_RULE_TCP_FLAG_SYN)
	    str.append("syn");
	  else if (opt == AAA_IPFILTER_RULE_TCP_FLAG_RST)
	    str.append("rst");
	  else if (opt == AAA_IPFILTER_RULE_TCP_FLAG_PSH)
	    str.append("psh");
	  else if (opt == AAA_IPFILTER_RULE_TCP_FLAG_ACK)
	    str.append("ack");
	  else if (opt == AAA_IPFILTER_RULE_TCP_FLAG_URG)
	    str.append("urg");
	  if (++i != r.tcpFlagList.end())
	    str.append(",");
	}
      
      if (r.icmpTypeRangeList.size()>0)
	str.append(" icmptypes");
      
      for (std::list<AAA_UINT8_RANGE>::iterator i=r.icmpTypeRangeList.begin();
	   i!=r.icmpTypeRangeList.end();)
	{
	  char typeStr[8];
	  if ((*i).first == (*i).last)
	    ACE_OS::sprintf(typeStr, "%d", (*i).first);
	  else
	    ACE_OS::sprintf(typeStr, "%d-%d", (*i).first, (*i).last);
	  str.append(typeStr);
	  if (++i != r.icmpTypeRangeList.end())
	    str.append(",");
	}
      std::cout << str.c_str() << std::endl;
}

void
rtest(unsigned char *buf, int size)
{
  unsigned int i, j;
  AAAMessage msg;
  HEADERPARSER hp;
  AAAMessageBlock *aBuffer;

  aBuffer = AAAMessageBlock::Acquire((char*)rbuf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  SETOPTION(hp, PARSE_STRICT, 0)
  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    cout << "header error" << endl;
    exit(1);
  }

  cout << "received command: " << msg.hdr.getCommandName() << endl;

  print_header(msg.hdr);

  PayloadParser pp;
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st)
    {
      aBuffer->Release();
      int code, type;
      std::string avp;
      msg.status.get(type, code);
      std::cout << "Disassemble failure.  Status code = " << code << std::endl;
      exit(1);
    }
      
  aBuffer->Release();

  std::cout << "Disassemble success." << std::endl;
  if (AAAAvpContainer* c_prinfo = msg.acl.search("Proxy-Info"))
    {
      for (i=0; i<c_prinfo->size(); i++)
	{
	  GET_DATA_REF(AAAAvpContainerList, acl, (*c_prinfo)[i]);

	  std::cout << "Proxy Info = [" << std::endl;
	  if (AAAAvpContainer *c_prhost = acl.search("Proxy-Host"))
	    {
	      for (j=0; j<c_prhost->size(); j++)
		{
		  GET_DATA_REF(diameter_identity_t, prhost, (*c_prhost)[j]);
		  std::cout << "\t" << "Proxy-Host = " 
			    << prhost.c_str() << std::endl;
		}
	    }
	  if (AAAAvpContainer *c_prstate = acl.search("Proxy-State"))
	    {
	      for (j=0; j<c_prstate->size(); j++)
		{
		  GET_DATA_REF(diameter_octetstring_t, state, (*c_prstate)[j]);
		  std::cout << "\t" << "Proxy-State = " << state.c_str() 
			    << std::endl;
		}
	    }
	  std::cout << "]" << std::endl;
	}
    }
  if (AAAAvpContainer* c_appid = msg.acl.search("Acct-Application-Id"))
    {
      for (i=0; i<c_appid->size(); i++)
	{
	  GET_DATA_REF(diameter_integer32_t, appid, (*c_appid)[i]);
	  std::cout << "Acct-Application-Id = " << appid << std::endl;
	}
    }
  if (AAAAvpContainer* c_dhost = msg.acl.search("Destination-Host"))
    {
      for (i=0; i<c_dhost->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, dhost, (*c_dhost)[i]);
	  std::cout << "Destination-Host = " << dhost.c_str() << std::endl;
	}
    }
  if (AAAAvpContainer* c_rrecord = msg.acl.search("Route-Record"))
    {
      for (i=0; i<c_rrecord->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, rrecord, (*c_rrecord)[i]);
	  std::cout << "Route-Record = " << rrecord.c_str() << std::endl;
	}
    }
  if (AAAAvpContainer* c_uri = msg.acl.search("Example-URI"))
    {
      for (i=0; i<c_uri->size(); i++)
	{
	  std::cout << "Example-URI = ";
	  GET_DATA_REF(diameter_uri_t, uri, (*c_uri)[i]);
	  print_diamuri(uri);
	}
    }
  if (AAAAvpContainer* c_rule = msg.acl.search("Example-IPFilterRule"))
    {
      for (i=0; i<c_rule->size(); i++)
	{
	  std::cout << "Example-IPFilterRule = ";
	  GET_DATA_REF(diameter_ipfilter_rule_t, rule, (*c_rule)[i]);
	  print_ipfilter_rule(rule);
	}
    }
  if (AAAAvpContainer* c_any = msg.acl.search("AVP"))
    {
      for (i=0; i<c_any->size(); i++)
	{
	  GET_DATA_REF(avp_t,any, (*c_any)[i]);
	  std::cout << "Received one AVP of type \"AVP\", ";
	  std::cout << "  please parse this AVP by yourself" << std::endl;
	  ACE_OS::memcpy(rAvp, any.c_str(), rAvp_len = any.length());
	}
    }
  msg.acl.releaseContainers();
}

// This is a test for NAS to originate a request message.
void
stest_nas_request(unsigned char *buf, int size)
{
  AAAAvpContainerManager cm;
  AAAAvpContainerEntryManager em;
  AAAAvpContainer *c_appid = cm.acquire("Acct-Application-Id");
  AAAAvpContainer *c_dhost = cm.acquire("Destination-Host");
  AAAAvpContainer *c_uri = cm.acquire("Example-URI");
  AAAAvpContainer *c_rule = cm.acquire("Example-IPFilterRule");
  hdr_flag flag = {1,1,0};
  AAADiameterHeader h(1, 0, flag, 9999999, 0, 1, 10);
  AAAMessage msg;
  AAAAvpContainerEntry *e;

  msg.hdr = h;
  HEADERPARSER hp;
  AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  /* 
     The first call of hp.set() checks validity of 
     flags and gets a AAACommand structure.
  */

  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  SETOPTION(hp, PARSE_STRICT, 0)
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st) {
    std::cout << "header error" << std::endl;
    exit(1);
  }

  e = em.acquire(AAA_AVP_INTEGER32_TYPE);
  GET_DATA_REF(diameter_integer32_t, appid1, e);
  c_appid->add(e);

  e = em.acquire(AAA_AVP_INTEGER32_TYPE);
  GET_DATA_REF(diameter_integer32_t, appid2, e);
  c_appid->add(e);

  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_identity_t, dhost, e);
  c_dhost->add(e);

  e = em.acquire(AAA_AVP_DIAMURI_TYPE);
  GET_DATA_REF(diameter_uri_t, uri, e);
  c_uri->add(e);

  e = em.acquire(AAA_AVP_IPFILTER_RULE_TYPE);
  GET_DATA_REF(diameter_ipfilter_rule_t, rule, e);
  c_rule->add(e);

  msg.acl.add(c_dhost);
  msg.acl.add(c_appid);
  msg.acl.add(c_uri);
  msg.acl.add(c_rule);

  /* Acct-Application-Id */
  appid1 = 1;
  appid2 = 2;
  /* Destination-Host */
  dhost.assign("aaa.com");
  /* Example-URI */
  uri.scheme = AAA_SCHEME_AAA;
  uri.fqdn = std::string("host.example.com");
  uri.port = 6666;
  uri.transport = TRANSPORT_PROTO_TCP;
  /* Example-IPFilterRule */
  rule.action = AAA_IPFILTER_RULE_ACTION_PERMIT;
  rule.dir = AAA_IPFILTER_RULE_DIRECTION_IN;
  rule.proto = 0;
  rule.src = AAA_IPFILTER_RULE_SRCDST(AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED);
  rule.dst = AAA_IPFILTER_RULE_SRCDST(AAA_IPFILTER_RULE_SRCDST_MASK, 
				      std::string("10.0.1.0"), 24);
  rule.dst.portRangeList.push_back(AAA_UINT16_RANGE(22));
  rule.dst.portRangeList.push_back(AAA_UINT16_RANGE(1000,2000));
  rule.ipOptionList.push_back(-AAA_IPFILTER_RULE_IP_OPTION_SSRR);
 
  PayloadParser pp;
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try {
    pp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "assemble failed" << std::endl;
      aBuffer->Release();
      exit(1);
    }

  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();

  // Set the actual message length to header
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
      aBuffer->Release();
      exit(1);
    }

  std::cout << "assemble success. total length (must be 208) = " 
       << msg.hdr.length << std::endl;
  
  // release all containers after parse.
  msg.acl.releaseContainers();
  aBuffer->Release();
}

// This is a test routine for Diameter proxy which adds 
// Route-Record AVP and Proxy-State AVP to a message.
void
stest_proxy_request(unsigned char *buf, int size)
{
  AAAMessage msg;
  HEADERPARSER hp;
  AAAAvpContainerManager cm;
  AAAAvpContainerEntryManager em;
  AAAAvpContainerEntry *e;
  AAAMessageBlock *aBuffer;

  // parse header
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  SETOPTION(hp, PARSE_LOOSE, 0)
  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    std::cout << "header error" << std::endl;
    aBuffer->Release();
    exit(1);
  }

  AAAAvpContainer *c_prinfo = cm.acquire("Proxy-Info");
  AAAAvpContainer *c_rrecord = cm.acquire("Route-Record");
  AAAAvpContainer *c_any = cm.acquire("AVP");  // wildcard AVP

  msg.acl.add(c_rrecord);
  msg.acl.add(c_prinfo);
  msg.acl.add(c_any);

  PayloadParser pp;
  pp.setRawData(aBuffer);
  aBuffer->size(msg.hdr.length);  // Adjust the size to the actual length.
  pp.setAppData(&msg.acl);
  pp.setDictData(NULL);    // No dictionary to be used.
  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to get specific sets of AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  // add one Route-Record AVP
  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  c_rrecord->add(e);
  GET_DATA_REF(diameter_identity_t, rrecord, e);
  rrecord.assign("xxx.com");

  // add one Proxy-Info AVP
  e = em.acquire(AAA_AVP_GROUPED_TYPE);
  GET_DATA_REF(diameter_grouped_t, group, e);
  c_prinfo->add(e);

  AAAAvpContainer *c_prhost = cm.acquire("Proxy-Host");
  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_identity_t, prhost, e);
  prhost.assign("bbb.ccc.com"); 
  c_prhost->add(e);
  group.add(c_prhost);
  
  AAAAvpContainer *c_prstate = cm.acquire("Proxy-State");
  e = em.acquire(AAA_AVP_STRING_TYPE);
  GET_DATA_REF(diameter_identity_t, prstate, e);
  prstate.assign("abcde");
  c_prstate->add(e);
  group.add(c_prstate);

  // Move the wildcard AVP container to the head, since this container 
  // may contain AVPs that are position-constraint.
  msg.acl.remove(c_any);
  msg.acl.prepend(c_any);

  pp.setRawData(aBuffer);
  aBuffer->wr_ptr(aBuffer->base()+HEADER_SIZE);
  try {
    pp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to set specific sets of AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  // Adjust the header;
  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();
  aBuffer->wr_ptr(aBuffer->base());
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
    }

  std::cout << "setting specific avp success.  total length (must be 268) = " 
	    << msg.hdr.length << std::endl;

  // Release container after parse.
  msg.acl.releaseContainers();
  aBuffer->Release();
}

// This is a test routine for Diameter server which originates 
// an answer message by modifying a request message by 
// removing Route-Record AVPs.
void
stest_server_answer(unsigned char *buf, int size)
{
  AAAMessage msg;
  HEADERPARSER hp;
  unsigned int i,j;
  AAAMessageBlock *aBuffer;

  // parse header
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  SETOPTION(hp, PARSE_STRICT, 0)
  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    std::cout << "header error" << std::endl;
    aBuffer->Release();
    exit(1);
  }

  // Read payload buffer
  PayloadParser pp;
  aBuffer->size(msg.hdr.length); // Adjust the size to the actual length.
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());
  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to get AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  if (AAAAvpContainer* c_prinfo = msg.acl.search("Proxy-Info"))
    {
      for (i=0; i<c_prinfo->size(); i++)
	{
	  GET_DATA_REF(AAAAvpContainerList, acl, (*c_prinfo)[i]);
	  std::cout << "Proxy Info = [" << std::endl;
	  if (AAAAvpContainer *c_prhost = acl.search("Proxy-Host"))
	    {
	      for (j=0; j<c_prhost->size(); j++)
		{
		  GET_DATA_REF(diameter_identity_t, prhost, (*c_prhost)[j]);
		  std::cout << "\t" << "Proxy-Host = " << prhost.c_str() 
			    << std::endl;
		}
	    }
	  if (AAAAvpContainer *c_prstate = acl.search("Proxy-State"))
	    {
	      for (j=0; j<c_prstate->size(); j++)
		{
		  GET_DATA_REF(diameter_octetstring_t, state, (*c_prstate)[j]);
		  std::cout << "\t" << "Proxy-State = " << state.c_str() 
			    << std::endl;
		}
	    }
	  std::cout << "]" << std::endl;
	}
    }
  if (AAAAvpContainer* c_appid = msg.acl.search("Acct-Application-Id"))
    {
      for (i=0; i<c_appid->size(); i++)
	{
	  GET_DATA_REF(diameter_integer32_t, appid, (*c_appid)[i]);
	  std::cout << "Acct-Application-Id = " << appid << std::endl;
	}
    }
  if (AAAAvpContainer* c_dhost = msg.acl.search("Destination-Host"))
    {
      for (i=0; i<c_dhost->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, dhost, (*c_dhost)[i]);
	  std::cout << "Destination-Host = " << dhost.c_str() << std::endl;
	  std::cout << "length = " << dhost.length() << std::endl;
	}
    }
  if (AAAAvpContainer* c_rrecord = msg.acl.search("Route-Record"))
    {
      for (i=0; i<c_rrecord->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, rrecord, (*c_rrecord)[i]);
	  std::cout << "Route-Record = " << rrecord.c_str() << std::endl;
	}
    }
  if (AAAAvpContainer* c_any = msg.acl.search("AVP"))
    {
      for (i=0; i<c_any->size(); i++)
	{
	  GET_DATA_REF(avp_t, any, (*c_any)[i]);
	  std::cout << "Received one AVP of type \"AVP\", ";
	  std::cout << "  please parse this AVP by yourself" << std::endl;
	  ACE_OS::memcpy(rAvp, any.c_str(), rAvp_len = any.length());
	}
    }

  // Clear "r" flag to make an answer message header
  msg.hdr.flags.r = 0;  // answer message

  // Parse the header again to get a dictionary entry for answer message
  aBuffer->wr_ptr(aBuffer->base());
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st) {
    std::cout << "header error" << std::endl;
    aBuffer->Release();
    exit(1);
  }

  // Remove Route-Record AVPs
  if (AAAAvpContainer* c_rrecord = msg.acl.search("Route-Record"))
    {
      msg.acl.remove(c_rrecord);
      c_rrecord->releaseEntries();
    }

  // Write the AVP container list back to the payload
  try {
    pp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to set AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  // Adjust the header;
  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();
  aBuffer->wr_ptr(aBuffer->base());
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
    }

  //  std::cout << "setting specific avp success.  total length (must be 172) = " 
  std::cout << "setting specific avp success.  total length (must be 252) = " 
	    << msg.hdr.length << std::endl;

  // release container after parse.
  msg.acl.releaseContainers();
  aBuffer->Release();
}

// This is a test routine for Diameter proxy which removes
// a Proxy-State AVP from an answer message.
void
stest_proxy_answer(unsigned char *buf, int size)
{
  AAAMessage msg;
  HEADERPARSER hp;
  AAAAvpContainerManager cm;
  AAAAvpContainerEntryManager em;
  AAAAvpContainerEntry *e;
  AAAMessageBlock *aBuffer;

  // parse the header
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  SETOPTION(hp, PARSE_STRICT, 0)

  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    std::cout << "header error" << std::endl;
    aBuffer->Release();
    exit(1);
  }

  AAAAvpContainer *c_prinfo = cm.acquire("Proxy-Info");
  AAAAvpContainer *c_any = cm.acquire("AVP");  // wildcard AVP

  // This calling order of add() is very important.
  // Wildcard AVP must be specified as the last add.
  msg.acl.add(c_prinfo);
  msg.acl.add(c_any);

  PayloadParser pp;
  aBuffer->size(msg.hdr.length);
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(NULL);    // No dictionary to be used.
  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to get specific sets of AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  // remove one Proxy-Info AVP (assuming that the last entry is the target)
  if (c_prinfo->size() > 0)
    {
      e = (*c_prinfo)[c_prinfo->size()-1];
      c_prinfo->remove(e);
      em.release(e);
      if (c_prinfo->size() == 0)
	{
	  msg.acl.remove(c_prinfo);
	}
    }

  // Move the wildcard AVP container to the head, since this container 
  // may contain AVPs that are position-constraint.
  msg.acl.remove(c_any);
  msg.acl.prepend(c_any);

  aBuffer->wr_ptr(HEADER_SIZE);
  try {
    pp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "failed to set specific sets of AVPs." << std::endl;
      aBuffer->Release();
      exit(1);
    }

  // adjast the header;
  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();
  aBuffer->wr_ptr(aBuffer->base());
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
    }

  //  cout << "setting specific avp success.  total length (must be 128) = " 
  cout << "setting specific avp success.  total length (must be 208) = " 
       << msg.hdr.length << endl;

  // release container after parse.
  msg.acl.releaseContainers();
  aBuffer->Release();
}

int
main(int argc, char** argv)
{
  AAADictionaryManager dm;

  // Start logging with specifying the use of syslog
  //ACE_Log_Msg::instance()->open(argv[0], ACE_Log_Msg::SYSLOG);
  //ACE_Log_Msg::instance()->enable_debug_messages();

  // Read dictionary file.
  dm.init("./dictionary.xml");

  rtest(rbuf, sizeof(rbuf));
  stest_nas_request(sbuf, sizeof(sbuf));
  stest_proxy_request(sbuf, sizeof(sbuf));
  stest_server_answer(sbuf, sizeof(sbuf));
  stest_proxy_answer(sbuf, sizeof(sbuf));

  return (0);
}




