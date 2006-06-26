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
/* $Id: test2.cxx,v 1.27 2004/06/17 21:07:50 yohba Exp $ */
/* 
   Sample program to show how to use diamparser library. 
   Written by Yoshihihiro Ohba (yohba@tari.toshiba.com)
*/

#include <string>
#include <iostream>
#include <ace/INET_Addr.h>
#include "diameter_parser_api.h"
using namespace std;

#define GET_DATA_REF(dataType, data, containerEntryPtr) \
        dataType &data = (containerEntryPtr)->dataRef(Type2Type<dataType>())

unsigned char rbuf[] = 
{ 
  0x01, 0x00, 0x00, 0x74, 0x90, 0x00, 0x01, 0x01, /* header(CER, T-bit set) */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* (length = 156-byte) */
  0x00, 0x00, 0x00, 0x0a,     
                          0x00, 0x00, 0x01, 0x08, /* Origin-Host */
#ifdef ERROR_CASE
  0x00, 0x00, 0x00, 0x13, 'a', 'a', 'a', '.', /* M-flag is off for err test */
#else
  0x40, 0x00, 0x00, 0x13, 'a', 'a', 'a', '.', 
#endif
  'b', 'b', 'b', '.', 'c', 'o', 'm', 0x00, 
  0x00, 0x00, 0x01, 0x28, 0x40, 0x00, 0x00, 0x0f, /* Origin-Realm */
  'a', 'a', 'a', '.', 'c', 'o', 'm',  0x00, 
  0x00, 0x00, 0x01, 0x01, 0x40, 0x00, 0x00, 0x0c, /* Host-IP-Address */
  0x0a, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x0a, /* Vendor-Id */
  0x40, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x01, 0x0d, 0x00, 0x00, 0x00, 0x18, /* Product-Name */
  't', 'o', 's', 'h', 'i', 'b', 'a', '-',
  'd', 'i', 'a', 'm', 'e', 't', 'e', 'r',
  0x00, 0x00, 0x01, 0x16, 0x40, 0x00, 0x00, 0x0c, /* Origin-State-Id */
  0x00, 0x00, 0x2b, 0x67
};


char addr[] = {0x0a, 0x01, 0x01, 0x01};
char orhost_str[] = { "aaa.bbb.com"};

static void 
inet_convert(ACE_INET_Addr& ace_ipaddr,
             diameter_address_t& diam_ipaddr)
{
//   diam_ipaddr contains a raw IP address octet string in network byte order.
    ace_ipaddr.set_address((const char*)(diam_ipaddr.value.data()),
		     diam_ipaddr.value.size(), 0);
}

static void
print_header(AAADiameterHeader &h)
{
  cout << "version = " << (int)h.ver << endl;
  cout << "length=" << h.length << "\n";
  cout << "flags(r,p,e,t)=(" 
       << (int)h.flags.r << "," 
       << (int)h.flags.p << ","
       << (int)h.flags.e << ","
       << (int)h.flags.t << ")" << endl;
  cout << "applicationId = " << h.appId << endl;
  cout << "h-h id = " << h.hh << endl;
  cout << "e-e id = " << h.ee << endl;
}

static void
rtest(unsigned char *buf, int size)
{
  unsigned int i, j;
  AAAMessage msg;
  AAAMessageBlock *aBuffer;
  AAAAvpContainer* c;

  HeaderParser hp;
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  hp.setDictData(PARSE_STRICT);

  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) 
    {
      cout << "header error" << endl;
      aBuffer->Release();
      exit(1);
    }

  print_header(msg.hdr);

  PayloadParser pp;
  aBuffer->size(msg.hdr.length);
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    std::cout << "payload parser error" << endl;
      int type, code;
      std::string avp;
      st.get(type, code, avp);
      std::cout << "Error type=" << type << ", code=" 
		<< code << ", name=" << avp << std::endl;
    msg.acl.releaseContainers();
    aBuffer->Release();
    exit(1);
  }

  std::cout << "Disassemble " << msg.hdr.getCommandName() << " success." 
	    << std::endl;

  if ((c = msg.acl.search("Origin-Host")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, orhost, (*c)[i]);
	  std::cout << "Origin-Host = " << orhost.c_str() << std::endl;
	}
    }

  if ((c = msg.acl.search("Origin-Realm")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_utf8string_t, orrealm, (*c)[i]);
	  std::cout << "Origin-Realm = " << orrealm.c_str() << std::endl;
	}
    }

  if ((c = msg.acl.search("Host-IP-Address")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_address_t, hostip, (*c)[i]);
	  ACE_INET_Addr addr;
	  inet_convert(addr, hostip);
	  std::cout << "Host-IP-Address = " 
		    << addr.get_host_addr()
		    << std::endl;
	}
    }

  if ((c = msg.acl.search("Vendor-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_unsigned32_t, vid, (*c)[i]);
	  std::cout << "Vendor-Id = " << vid << std::endl;
	}
    }

  if ((c = msg.acl.search("Product-Name")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_utf8string_t, product, (*c)[i]);
	  std::cout << "Product-Name = " << product.c_str() << std::endl;
	}
    }

  if ((c = msg.acl.search("Origin-State-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_unsigned32_t, orstatid, (*c)[i]);
	  std::cout << "Origin-State-Id = " << orstatid << std::endl;
	}
    }

  if ((c = msg.acl.search("Supported-Vendor-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_unsigned32_t, sup_vid, (*c)[i]);
	  std::cout << "Supported-Vendor-Id = " << sup_vid << std::endl;
	}
    }

  if ((c = msg.acl.search("Auth-Application-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_integer32_t, auth_appid, (*c)[i]);
	  std::cout << "Auth-Application-Id = " << auth_appid << std::endl;
	}
    }

  if ((c = msg.acl.search("Acct-Application-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_integer32_t, acct_appid, (*c)[i]);
	  std::cout << "Acct-Application-Id = " << acct_appid << std::endl;
	}
    }

  if ((c = msg.acl.search("Vendor-Specific-Application-Id")))
    {
      AAAAvpContainer* cc;
      std::cout << "Vendor-Specific-Application-Id = ";
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(AAAAvpContainerList, acl, (*c)[i]);
	  if ((cc = acl.search("Proxy-Host")))
	    {
	      for (j=0; j<cc->size(); j++)
		{
		  GET_DATA_REF(diameter_unsigned32_t, sup_vid, (*cc)[j]);
		  std::cout << "\t" << "Proxy-Host = " << sup_vid << std::endl;
		}
	    }
	  if ((cc = acl.search("Auth-Application-Id")))
	    {
	      for (j=0; j<cc->size(); j++)
		{
		  GET_DATA_REF(diameter_integer32_t, auth_appid, (*cc)[j]);
		  std::cout << "\t" << "Auth-Application-Id = " << auth_appid 
			    << std::endl;
		}
	    }
	  if ((cc = acl.search("Acct-Application-Id")))
	    {
	      for (j=0; j<cc->size(); j++)
		{
		  GET_DATA_REF(diameter_integer32_t, acct_appid, (*cc)[j]);
		  std::cout << "\t" << "Acct-Application-Id = " << acct_appid 
			    << std::endl;
		}
	    }
	}
    }

  if ((c = msg.acl.search("Firmware-Revision")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_unsigned32_t, firm_rev, (*c)[i]);
	  std::cout << "Firmware-Revision = " << firm_rev << std::endl;
	}
    }

  if ((c = msg.acl.search("AVP")))
    {
      std::cout << "I'm not ready to process this AVP";
    }

  msg.acl.releaseContainers();
  aBuffer->Release();
}


static void
stest(unsigned char *buf, int size)
{
  AAAAvpContainerManager cm;
  AAAAvpContainerEntryManager em;
  AAAAvpContainer *c_orhost = cm.acquire("Origin-Host");
  AAAAvpContainer *c_orrealm = cm.acquire("Origin-Realm");
  AAAAvpContainer *c_hostip = cm.acquire("Host-IP-Address");
  AAAAvpContainer *c_vid = cm.acquire("Vendor-Id");
  AAAAvpContainer *c_product = cm.acquire("Product-Name");
  AAAAvpContainer *c_orstatid = cm.acquire("Origin-State-Id");
  AAAAvpContainerEntry *e;

  hdr_flag flag = {1,0,0,1};
  AAADiameterHeader h(1, 0, flag, 257, 0, 1, 10);

  AAAMessage msg;
  AAAMessageBlock *aBuffer;
  msg.hdr = h;
  HeaderParser hp;
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  hp.setDictData(PARSE_STRICT);

  /* Check validity of flags and gets a AAACommand structure */
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;;
      aBuffer->Release();
      exit(1);
    }

  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_identity_t, orhost, e);
  c_orhost->add(e);

  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_utf8string_t, orrealm, e);
  c_orrealm->add(e);

  e = em.acquire(AAA_AVP_ADDRESS_TYPE);
  GET_DATA_REF(diameter_address_t, hostip, e);
  c_hostip->add(e);

  e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
  GET_DATA_REF(diameter_unsigned32_t, vid, e);
  c_vid->add(e);

  e = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
  GET_DATA_REF(diameter_utf8string_t, product, e);
  c_product->add(e);

  e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
  GET_DATA_REF(diameter_unsigned32_t, orstatid, e);
  c_orstatid->add(e);

  msg.acl.add(c_orhost);
  msg.acl.add(c_orrealm);
  msg.acl.add(c_hostip);
  msg.acl.add(c_vid);
  msg.acl.add(c_product);
  msg.acl.add(c_orstatid);

  /* set values */
  /* Origin-Host */
  orhost.assign(orhost_str);

  /* Origin-Realm */
  orrealm.assign("aaa.com");

  /* Host-IP-Address */
  hostip.type = AAA_ADDRESS_IP;
  hostip.value.assign(addr, sizeof(addr));

  /* Vendor-Id */
  vid = 1;

  /* Product-Name */
  product.assign("toshiba-diameter");

  /* Origin-State-Id */
  orstatid = 1111;

  PayloadParser pp;
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try 
   { 
     pp.parseAppToRaw();
   }
  catch (AAAErrorStatus st)
    {
      std::cout << "assemble failed" << std::endl;
      
      msg.acl.releaseContainers();
      aBuffer->Release();
      exit(1);
    }

  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();

  /* The second calll of hp.set() sets the actual message length */
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
      msg.acl.releaseContainers();
      aBuffer->Release();
      exit(1);
    }

  std::cout << "assemble " << msg.hdr.getCommandName() 
	    << " success. total length = " << msg.hdr.length << std::endl;
  msg.acl.releaseContainers();
  aBuffer->Release();

}

static void
mstest(unsigned char *buf, int size)
{
  AAAAvpContainerManager cm;
  AAAAvpContainerEntryManager em;
  AAAAvpContainer *c_dhost = cm.acquire("Destination-Host");
  AAAAvpContainer *c_acctId = cm.acquire("Acct-Application-Id");
  AAAAvpContainer *c_proxyInfo = cm.acquire("Proxy-Info");
  AAAAvpContainer *c_rrec = cm.acquire("Route-Record");
  AAAAvpContainerEntry *e;

  hdr_flag flag = {1,0,0,1};
  AAADiameterHeader h(1, 0, flag, 9999999, 0, 1, 10);

  AAAMessage msg;
  AAAMessageBlock *aBuffer;
  msg.hdr = h;
  HeaderParser hp;
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  hp.setDictData(PARSE_STRICT);

  /* Check validity of flags and gets a AAACommand structure */
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;;
      aBuffer->Release();
      exit(1);
    }

  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_identity_t, dhost, e);
  c_dhost->add(e);

  e = em.acquire(AAA_AVP_UINTEGER32_TYPE);
  GET_DATA_REF(diameter_unsigned32_t, acctId, e);
  c_acctId->add(e);

  e = em.acquire(AAA_AVP_GROUPED_TYPE);
  GET_DATA_REF(diameter_grouped_t, proxyInfo1, e);
  c_proxyInfo->add(e);

  e = em.acquire(AAA_AVP_GROUPED_TYPE);
  GET_DATA_REF(diameter_grouped_t, proxyInfo2, e);
  c_proxyInfo->add(e);

  e = em.acquire(AAA_AVP_DIAMID_TYPE);
  GET_DATA_REF(diameter_identity_t, rrec, e);
  c_rrec->add(e);

  msg.acl.add(c_dhost);
  msg.acl.add(c_acctId);
  msg.acl.add(c_proxyInfo);
  msg.acl.add(c_rrec);

  /* set values */
  dhost = "host1";
  acctId = 1;
  rrec = "route-rec";

  /* proxy info */
  for (int i = 0; i < 2; i ++ ) {

      AAAAvpContainer *c_proxyHost = cm.acquire("Proxy-Host");
      AAAAvpContainer *c_proxyState = cm.acquire("Proxy-State");

      e = em.acquire(AAA_AVP_DIAMID_TYPE);
      GET_DATA_REF(diameter_identity_t, proxyHost, e);
      c_proxyHost->add(e);

      e = em.acquire(AAA_AVP_STRING_TYPE);
      GET_DATA_REF(diameter_octetstring_t, proxyState, e);
      c_proxyState->add(e);

      if (i==0)
	{
	  proxyHost = "proxy-host1";
	  proxyState = "proxy-state1";
	  proxyInfo1.add(c_proxyHost);
	  proxyInfo1.add(c_proxyState);      
	}
      else
	{
	  proxyHost = "proxy-host2";
	  proxyState = "proxy-state2";
	  proxyInfo2.add(c_proxyHost);
	  proxyInfo2.add(c_proxyState);      
	}
  }

  PayloadParser pp;
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try 
   { 
     pp.parseAppToRaw();
   }
  catch (AAAErrorStatus st)
    {
      std::cout << "assemble failed" << std::endl;
      
      msg.acl.releaseContainers();
      aBuffer->Release();
      exit(1);
    }

  msg.hdr.length = aBuffer->wr_ptr() - aBuffer->base();

  /* The second calll of hp.set() sets the actual message length */
  try {
    hp.parseAppToRaw();
  }
  catch (AAAErrorStatus st)
    {
      std::cout << "header error" << std::endl;
      msg.acl.releaseContainers();
      aBuffer->Release();
      exit(1);
    }

  std::cout << "assemble " << msg.hdr.getCommandName() 
	    << " success. total length = " << msg.hdr.length << std::endl;
  msg.acl.releaseContainers();
  aBuffer->Release();

}

static void
mrtest(unsigned char *buf, int size)
{
  unsigned int i;
  AAAMessage msg;
  AAAMessageBlock *aBuffer;
  AAAAvpContainer* c;

  HeaderParser hp;
  aBuffer = AAAMessageBlock::Acquire((char*)buf, size);
  hp.setRawData(aBuffer);
  hp.setAppData(&msg.hdr);
  hp.setDictData(PARSE_STRICT);

  try {
    hp.parseRawToApp();
  }
  catch (AAAErrorStatus st) 
    {
      cout << "header error" << endl;
      aBuffer->Release();
      exit(1);
    }

  print_header(msg.hdr);

  PayloadParser pp;
  aBuffer->size(msg.hdr.length);
  pp.setRawData(aBuffer);
  pp.setAppData(&msg.acl);
  pp.setDictData(msg.hdr.getDictHandle());

  try {
    pp.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    std::cout << "payload parser error" << endl;
      int type, code;
      std::string avp;
      st.get(type, code, avp);
      std::cout << "Error type=" << type << ", code=" 
		<< code << ", name=" << avp << std::endl;
    msg.acl.releaseContainers();
    aBuffer->Release();
    exit(1);
  }

  std::cout << "Disassemble [length = " << msg.hdr.length << "] " << msg.hdr.getCommandName() << " success." 
	    << std::endl;

  if ((c = msg.acl.search("Destination-Host")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, dhost, (*c)[i]);
	  std::cout << "Destination-Host = " << dhost.c_str() << std::endl;
	}
    }

  if ((c = msg.acl.search("Acct-Application-Id")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_unsigned32_t, id, (*c)[i]);
	  std::cout << "Acct-Application-Id = " << id << std::endl;
	}
    }

  if ((c = msg.acl.search("Route-Record")))
    {
      for (i=0; i<c->size(); i++)
	{
	  GET_DATA_REF(diameter_identity_t, rrec, (*c)[i]);
	  std::cout << "Route-Record = " << rrec.c_str() << std::endl;
	}
    }

   AAAAvpContainerList::iterator y;
   for (y = msg.acl.begin(); y != msg.acl.end(); y ++ ) 
    {
      AAAAvpContainer* cntr = (*y);

      if (ACE_OS::strcmp(cntr->getAvpName(), "Proxy-Info") == 0) 
       {
	 std::cout << "Proxy-Info AVP" << std::endl;
         GET_DATA_REF(diameter_grouped_t, pInfo, (*cntr)[0]);

         if ((c = pInfo.search("Proxy-Host")))
          {
            for (i=0; i<c->size(); i++)
	     {
	       GET_DATA_REF(diameter_identity_t, phost, (*c)[i]);
	       std::cout << "Proxy-Host = " << phost.c_str() << std::endl;
	     }
          }          
         if ((c = pInfo.search("Proxy-State")))
          {
            for (i=0; i<c->size(); i++)
	     {
	       GET_DATA_REF(diameter_octetstring_t, pstate, (*c)[i]);
	       std::cout << "Proxy-State = " << pstate.c_str() << std::endl;
	     }
          }          
       }
    }

  msg.acl.releaseContainers();
  aBuffer->Release();
}

int
main(int argc, char** argv)
{
  unsigned char sbuf[1024];
  AAADictionaryManager dm;

  // Start logging with specifying the use of syslog
  //  ACE_Log_Msg::instance()->open(argv[0], ACE_Log_Msg::SYSLOG);

  // Read dictionary file.
  dm.init("./dictionary.xml");

  rtest(rbuf, sizeof(rbuf));
  stest(sbuf, sizeof(sbuf));

  std::cout << std::endl << std::endl;

  stest(sbuf, sizeof(sbuf));
  rtest(sbuf, sizeof(sbuf));

  std::cout << std::endl << std::endl;

  mstest(sbuf, sizeof(sbuf));
  mrtest(sbuf, sizeof(sbuf));

  return (0);
}
