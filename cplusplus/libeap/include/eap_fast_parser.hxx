/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */          
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
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
/***************************************************************************
                          eap_fast_parser.cxx  -  description
                             -------------------
    begin                : vie mar 12 2004
    copyright            : (C) 2007 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef  __EAP_FAST_PARSER_HXX__
#define  __EAP_FAST_PARSER_HXX__

#include "eap_fast.hxx"
#include "eap_parser.hxx"

//EAP-FAST Request parser


typedef AAAParser<AAAMessageBlock *, EapRequestFast *> EapRequestFastParser;

template <>
inline void EapRequestFastParser::parseRawToApp()
{

  AAAMessageBlock* msg = getRawData();
  EapRequestFast *request = getAppData();

  ACE_UINT32 length_data = msg->length()-1;
              
  EAP_LOG(LM_DEBUG,"EapRequestFastParser::parseRawToApp DATA LENGTH %d \n ",length_data);

  ACE_Byte flags= *(msg->rd_ptr());

  //Packet is bad-formatted
  if (!FAST_START(flags))
  {
      if (length_data < 0) throw -1;
  }
  else EAP_LOG(LM_DEBUG,"EapRequestFastParser::parseRawToApp: START flag\n");

  //Read flags byte.
  request->set_flags( ACE_Byte( flags ) );

  msg->rd_ptr(1);
  //Flags must be examined to know if FAST Message Length is present 
  if (FAST_LENGTH_INCLUDED(flags))
  {   
	EAP_LOG(LM_DEBUG,"EapRequestFastParser::parseRawToApp: LENGTH INCLUDED %d\n",length_data -4);
       if (length_data < 4)  throw -1; //FAST message length doesn't exit
       //Read FAST Message Length
       request->set_fast_message_length(ntohl(*(ACE_UINT32*)msg->rd_ptr()));
       msg->rd_ptr(4);
       length_data -= 4; //Packet includes a LENGTH field.
  } else request->set_fast_message_length(0); //EAP PACKET has not FAST message length field.

  //Copy data field.
  request->set_is_piggyback(false);

  if(length_data == 0){
      request->set_data(NULL);
      request->set_piggy(NULL);
      if ((flags >> 0x03) == (ACE_Byte)0x00) request->set_is_ack(true); //it is an ack.
      msg->rd_ptr(msg->base()+5);
      return;
  }

  ACE_UINT32 offset=0;
  request->set_is_application (false);


  if(!request->get_is_fragment() && !FAST_START(flags)) {

    while((offset)< length_data){
	// 1th byte is the type, the 4th and 5th byte are the length(which is not include the first 5 bytes)
	u8 * point = (u8 *)(msg->rd_ptr()+offset);
	if( ((*point)&0xff)!=23)//application type =23
	{	offset = offset + ((*(point +3))&0xff)*256 + ((*(point +4))&0xff)+5;
	}
	else break;
    }

    if (offset != 0 && offset != length_data) {request->set_is_piggyback(true);}
    if (offset == 0){request->set_is_application (true);}
  }

    AAAMessageBlock *data;
    AAAMessageBlock *piggy;
    if (request->get_is_piggyback()){
	data  = AAAMessageBlock::Acquire(offset);
	data->copy(msg->rd_ptr(),offset);
	msg->rd_ptr(offset);

	piggy = AAAMessageBlock::Acquire(length_data - offset);
        piggy->copy(msg->rd_ptr(),length_data - offset);
	request->set_piggy(piggy);
    }
    else {
	data = AAAMessageBlock::Acquire(length_data);
     	data->copy(msg->rd_ptr(),length_data);
     	//Assign FAST data.
     	//request->set_data(data);
	request->set_piggy(NULL);    	
    }

   request->set_data(data);
   request->set_is_ack(false);
   msg->rd_ptr(msg->base()+5);
}

template <>
inline void EapRequestFastParser::parseAppToRaw()
{
  AAAMessageBlock *msg = getRawData();
  EapRequestFast* request = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(request);
  requestParser.parseAppToRaw();            //Here msg->wr_ptr() is placed on after Type field.
  //Write flags
  ACE_Byte  flags = request->get_flags();

  *(ACE_Byte*)msg->wr_ptr() = flags;
  msg->wr_ptr(1);
  if (FAST_LENGTH_INCLUDED(flags))
  {
    EAP_LOG(LM_DEBUG,"EapRequestFastParser::parseAppToRaw LENGTH INCLUDED %d\n",request->get_data()->length());
    //Write length
    *(ACE_UINT32*)msg->wr_ptr() = htonl(request->get_data()->length());
    msg->wr_ptr(4);
  }
  //FAST data
  ACE_UINT32 length_to_copy = msg->end()-msg->wr_ptr();
  EAP_LOG(LM_DEBUG,"EapRequestFastParser::parseAppToRaw DATA LENGTH %d \n ",length_to_copy);
  //if (!FAST_START(flags) && (request->get_data() != NULL))
  if ((request->get_data() != NULL))
    msg->copy(request->get_data()->rd_ptr(),length_to_copy);
}

//EAP-FAST Request parser
typedef EapRequestFastParser EapResponseFastParser;  

#endif
