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

// $Id: ParserTest.cxx,v 1.6 2005/10/07 23:37:57 vfajardo Exp $ 

#include <iostream>
#include "radius_parser.h"
#include "radius_dictionary.h"

#if ETHEREAL_DUMP
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

unsigned char radPkt1[] = {
	0x02, 0x0a, 0x00, 0x31,    // code, ident, length
	0x01, 0x02, 0x03, 0x04,    // authenticator
	0x05, 0x06, 0x07, 0x08,    // authenticator
	0x09, 0x0a, 0x0b, 0x0c,    // authenticator
	0x0d, 0x0e, 0x0f, 0x10,    // authenticator
	
	0x01, 0x11, 'n',  'a',     // user-name avp
	'm',  'e',  '@',  'd',
	'o',  'm',  'a',  'i',
	'n',  '.',  'c',  'o',
	'm',
	
	      0x04, 0x06, 0x01,    // NAS-IP-Address
	0x0a, 0xa8, 0xC0,
	
	                  0x05,    // NAS-Port
	0x06, 0x00, 0x01, 0x08,
	0x01,
	 
	      0x00, 0x00, 0x00     // padding
};

unsigned char radPkt2[] = {
	0x01, 0x01, 0x00, 0x28,    // code, ident, length
	
	0x01, 0x02, 0x03, 0x04,    // authenticator
	0x05, 0x06, 0x07, 0x08,    // authenticator
	0x01, 0x02, 0x03, 0x04,    // authenticator
	0x05, 0x06, 0x07, 0x08,    // authenticator
	
	0x01, 0x08, 0x6d, 0x79,    // username avp
	0x6e, 0x61, 0x6d, 0x65,   
	
	0x05, 0x06, 0x00, 0x00,    // NAS-Port
	0x07, 0x14,
	
	0x04, 0x06, 0xc0, 0xa8,    // NAS-IP-address
	0x0f, 0x01,
	
	            0x00, 0x00     // padding
};

void DecomposeStream1()
{
	RADIUS_Packet pkt;
	AAAMessageBlock *aBuf = AAAMessageBlock::Acquire((char*)radPkt1, sizeof(radPkt1));
	
	RADIUS_PacketParser parser;
	parser.reverse(pkt, *aBuf);
	
	RADIUS_PacketDump::Dump(pkt);
	aBuf->Release();
	pkt.Attributes().Clear(); // was pre-allocated by parser
}

void DecomposeStream2()
{
	RADIUS_Packet pkt;
	AAAMessageBlock *aBuf = AAAMessageBlock::Acquire((char*)radPkt2, sizeof(radPkt2));
	
	RADIUS_PacketParser parser;
	parser.reverse(pkt, *aBuf);
	
	RADIUS_PacketDump::Dump(pkt);
	aBuf->Release();
	pkt.Attributes().Clear(); // was pre-allocated by parser
}

void ComposeStream()
{
	static unsigned char auth[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
	};
		
	RADIUS_Packet pkt;
	pkt.Code() = RADIUS_CODE_ACCESS_REQUEST;
	pkt.Id() = 1;
	ACE_OS::memcpy(pkt.Authenticator(), auth, sizeof(auth));
	
	RADIUS_OctetString userName(1);
	RADIUS_Integer nasPort(5);
	RADIUS_IPv4Address nasIpAddress(4);
	
	userName.Value() = "myname";
	nasPort.Value() = 1812;
	nasIpAddress.Value().set("192.168.15.1:0");
	
	pkt.Attributes().Add(&userName);
	pkt.Attributes().Add(&nasPort);
	pkt.Attributes().Add(&nasIpAddress);
	
	AAAMessageBlock *aBuf = AAAMessageBlock::Acquire(1024);
	RADIUS_PacketParser parser;
	parser.forward(pkt, *aBuf);
	
#if ETHEREAL_DUMP

#define HELLO_PORT 1812
#define HELLO_GROUP "192.168.10.10"

    /* set up destination address */
	struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
    addr.sin_port=htons(HELLO_PORT);
	
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    for (int i = 0; i < 10; i++) {
        sendto(s, aBuf->base(), aBuf->length(), 0, (struct sockaddr *)&addr, sizeof(addr));
        ACE_OS::sleep(1);
    }
	close(s);
#endif
	
	aBuf->Release();
}

int main(int argc, char *argv[])
{   
   if (argc != 2) {
   	   std::cout << "Usage: " << argv[0];
   	   std::cout << " {dictionary file}" << std::endl;
   	   return (0);
   }   
   
   try {
       std::string fname = argv[1];
       RADIUS_DICT_LOAD(fname);
       
       ComposeStream();       
       std::cout << "***** success *****" << std::endl;
       
       DecomposeStream1();       
       std::cout << "***** success *****" << std::endl;
       
       DecomposeStream2();       
       std::cout << "***** success *****" << std::endl;
   }
   catch (RADIUS_Exception &e) {
   	   std::cout << "Error: " << e.Description() << std::endl;
   }
   return (0);
}

