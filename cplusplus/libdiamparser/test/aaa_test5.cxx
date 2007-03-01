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
/* $Id: test5.cxx,v 1.5 2005/06/28 21:55:46 vfajardo Exp $ */
/* 
   Sample program to show how to use diamparser library. 
   Written by Yoshihihiro Ohba (yohba@tari.toshiba.com)
   Modified by Santosh Kawade (skawade@mahindrabt.com) for thread-safe
   testing.
*/

#include <string>
#include <iostream>
#include <ace/OS.h>
#include <ace/INET_Addr.h>
#include <ace/Thread_Manager.h>
#include "diameter_parser.h"
using namespace std;

#define GET_DATA_REF(dataType, data, containerEntryPtr) \
        dataType &data = (containerEntryPtr)->dataRef(Type2Type<dataType>())

#define NUM_THREADS 100
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
rtest(unsigned char *buf, int size)
{
  // allocates an DiameterMsg in an auto_ptr<>
  DiameterMsgWidget msg(0); 

  // create a checked message parser
DiameterMsgParserWidgetChecked   parser;
  if (parser.ParseRawToApp(msg, buf, size, DIAMETER_PARSE_STRICT) < 0) {
      exit (0);
  }

  // dumps the header
  DiameterMsgHeaderDump::Dump(*msg());

  std::cout << "Disassemble " << msg()->hdr.getCommandName() << " success." 
	    << std::endl;

  // widges allow for searching through a message list
  // for a specific AVP
  DiameterIdentityAvpContainerWidget originHost(msg()->acl);
  DiameterIdentityAvpContainerWidget originRealm(msg()->acl);
  DiameterAddressAvpContainerWidget hostIp(msg()->acl);
  DiameterUInt32AvpContainerWidget vendorId(msg()->acl);
  DiameterUtf8AvpContainerWidget product(msg()->acl);
  DiameterUInt32AvpContainerWidget originState(msg()->acl);
  DiameterUInt32AvpContainerWidget supportedVendorId(msg()->acl);
  DiameterUInt32AvpContainerWidget authAppId(msg()->acl);
  DiameterUInt32AvpContainerWidget acctAppId(msg()->acl);
  DiameterGroupedAvpContainerWidget vendorSpecificId(msg()->acl);
  DiameterUInt32AvpContainerWidget firmware(msg()->acl);

  // the following lookups an AVP using is name

  diameter_identity_t *host = originHost.GetAvp("Origin-Host");
  if (host) {
      std::cout << "Origin-Host = " << *host << std::endl;
  }

  diameter_identity_t *realm = originRealm.GetAvp("Origin-Realm");
  if (realm) {
      std::cout << "Origin-Host = " << *realm << std::endl;
  }

  diameter_address_t *address = hostIp.GetAvp("Host-IP-Address", 0);
  for (int ndx=1; address; ndx++) {
	  ACE_INET_Addr addr;
	  inet_convert(addr, *address);
	  std::cout << "Host-IP-Address = " 
		        << addr.get_host_addr()
		        << std::endl;
      address = hostIp.GetAvp("Host-IP-Address", ndx);
  }

  diameter_unsigned32_t *vid = vendorId.GetAvp("Vendor-Id");
  if (vid) {
      std::cout << "Vendor-Id = " << *vid << std::endl;
  }

  diameter_utf8string_t *prod = product.GetAvp("Product-Name");
  if (prod) {
      std::cout << "Product-Name = " << *prod << std::endl;
  }

  diameter_unsigned32_t *oState = originState.GetAvp("Origin-State-Id");
  if (oState) {
      std::cout << "Origin-State-Id = " << *oState << std::endl;
  }

  diameter_unsigned32_t *svid = supportedVendorId.GetAvp
      ("Supported-Vendor-Id", 0);
  for (int ndx=1; svid; ndx++) {
      std::cout << "Supported-Vendor-Id = " << *svid << std::endl;
      svid = supportedVendorId.GetAvp("Supported-Vendor-Id", ndx);
  }

  diameter_unsigned32_t *fware = firmware.GetAvp("Firmware-Revision");
  if (fware) {
      std::cout << "Firmware-Revision = " << *fware << std::endl;
  }

  diameter_unsigned32_t *authId = authAppId.GetAvp
      ("Auth-Application-Id", 0);
  for (int ndx=1; authId; ndx++) {
      std::cout << "Auth-Application-Id = " << *authId << std::endl;
      authId = authAppId.GetAvp("Auth-Application-Id", ndx);
  }

  diameter_unsigned32_t *acctId = acctAppId.GetAvp
      ("Auth-Application-Id", 0);
  for (int ndx=1; acctId; ndx++) {
      std::cout << "Acct-Application-Id = " << *acctId << std::endl;
      acctId = acctAppId.GetAvp("Acct-Application-Id", ndx);
  }

  diameter_grouped_t *grouped = vendorSpecificId.GetAvp
      ("Vendor-Specific-Application-Id", 0);
  for (int ndx=1; grouped; ndx++) {
      DiameterUInt32AvpContainerWidget gAuthId(*grouped);
      DiameterUInt32AvpContainerWidget gAcctId(*grouped);
      DiameterUInt32AvpContainerWidget gVendorId(*grouped);

      diameter_unsigned32_t *uint32 = gVendorId.GetAvp("Vendor-Id", 0);
      for (int p=1; uint32; p++) {
          uint32 = gVendorId.GetAvp("Vendor-Id", p);
          if (uint32) {
              std::cout << "Vendor-Id = " << *uint32 << std::endl;
          }
      }
        
      uint32 = gAuthId.GetAvp("Auth-Application-Id");
      if (uint32) {
          std::cout << "Auth-Application-Id = " << *uint32 << std::endl;
      }

      uint32 = gAcctId.GetAvp("Acct-Application-Id");
      if (uint32) {
          std::cout << "Acct-Application-Id = " << *uint32 << std::endl;
      }
  }
}


static void
stest(unsigned char *buf, int size)
{
  // constructor of this class will automatically
  // set the following values
  // diameter_hdr_flag flag = {1,0,0,1};
  // DiameterMsgHeader h(1, 0, flag, 257, 0, 0, 0);
  DiameterMsgWidget msg(257);

  // the following widgets allocates AVP containers
  DiameterIdentityAvpWidget originHost("Origin-Host");
  DiameterIdentityAvpWidget originRealm("Origin-Realm");
  DiameterAddressAvpWidget  hostIp("Host-IP-Address");
  DiameterUInt32AvpWidget   vendorId("Vendor-Id");
  DiameterUtf8AvpWidget     product("Product-Name");
  DiameterUInt32AvpWidget   originStateId("Origin-State-Id");

  // the following actions allocates and assigns a
  // value to the AVP container
  originHost.Get() = "toshiba-diameter";
  originRealm.Get() = "aaa.com";

  diameter_address_t &ipAvp = hostIp.Get();
  ipAvp.type = AAA_ADDRESS_IP;
  ipAvp.value.assign(addr, sizeof(addr));

  vendorId.Get() = 1;
  product.Get() = "opendiamter";
  originStateId.Get() = 12345;

  msg()->acl.add(originHost());
  msg()->acl.add(originRealm());
  msg()->acl.add(hostIp());
  msg()->acl.add(vendorId());
  msg()->acl.add(product());
  msg()->acl.add(originStateId());

  // composes the message
  DiameterMsgParserWidgetChecked parser;
  if (parser.ParseAppToRaw(msg, buf, size, DIAMETER_PARSE_STRICT) < 0) {
      exit (0);
  }

  std::cout << "assemble " << msg()->hdr.getCommandName() 
            << " success. total length = " << msg()->hdr.length 
            << std::endl;
}

ACE_THR_FUNC_RETURN worker(void *p)
{
        int i = 10;
        unsigned char sbuf[1024];
        for(i = 0 ; i !=100 ; i++)
        {
                ACE_OS::memset(sbuf,'\0',1024);
                stest(sbuf, sizeof(sbuf));
		rtest(sbuf, sizeof(sbuf));
        }
        return 0;
}

int
main(int argc, char** argv)
{
  DiameterDictionaryManager dm;

  int grp_id = 0;

  ACE_Thread_Manager *threads = ACE_Thread_Manager::instance();

  // Start logging with specifying the use of syslog
  //  ACE_Log_Msg::instance()->open(argv[0], ACE_Log_Msg::SYSLOG);

  // Read dictionary file.
  dm.init("./config/dictionary.xml");
  grp_id = threads->spawn_n(NUM_THREADS, worker);
  if (grp_id < 1) {
      printf("Thread creation failed\n");
  }

  printf( " ************** Finished spawning threads \n");

  threads->wait_grp(grp_id);

  return 0;                              
}

