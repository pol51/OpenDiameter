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
                          eap_tls_parser.cxx  -  description
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


#ifndef  __EAP_TLS_PARSER_HXX__
#define  __EAP_TLS_PARSER_HXX__

#include "eap_tls.hxx"
#include "eap_parser.hxx"

//EAP-TLS Request parser


typedef AAAParser<AAAMessageBlock *, EapRequestTls *> EapRequestTlsParser;

template <>
inline void EapRequestTlsParser::parseRawToApp()
{

  AAAMessageBlock* msg = getRawData();
  EapRequestTls *request = getAppData();

  ACE_INT16 length_packet = ntohs(*(short *)(msg->base()+2)); //Get Packet Length
  ACE_UINT32 length_data = length_packet-5-1;
                                                                  
  EAP_LOG(LM_DEBUG,"EapRequestTlsParser::parseRawToApp LENGTH PACKET %d \n ",length_packet);

  ACE_Byte flags= *(msg->rd_ptr());
  //Packet is bad-formatted
  if (!TLS_START(flags))
  {
      if (length_packet < 6) throw -1;
  }
  else EAP_LOG(LM_DEBUG,"EapRequestTlsParser::parseRawToApp: START flag\n");
  //Read flags byte.
  request->set_flags(flags);
  msg->rd_ptr(1);
  //Flags must be examined to know if TLS Message Length is present 
  if (TLS_LENGTH_INCLUDED(flags))
  {    EAP_LOG(LM_DEBUG,"EapRequestTlsParser::parseRawToApp: LENGTH INCLUDED flag\n");
       if (length_packet < 10)  throw -1; //TLS message length doesn't exit
       //Read TLS Message Length
       request->set_tls_message_length(ntohl(*(ACE_UINT32*)msg->rd_ptr()));
       msg->rd_ptr(4);
       length_data -= 4; //Packet includes a LENGTH field.
  } else request->set_tls_message_length(0); //EAP PACKET has not TLS message length field.
  //Copy data field.

  if (length_data)
  {
     AAAMessageBlock *data = AAAMessageBlock::Acquire(length_data);
     data->copy(msg->rd_ptr(),length_data);
     //Assign TLS data.
     request->set_data(data);
     request->set_is_ack(false);
   }
   else
   {
      request->set_data(NULL);
      if (flags == (ACE_Byte)0x00) request->set_is_ack(true); //it is an ack.
   }
   
}

template <>
inline void EapRequestTlsParser::parseAppToRaw()
{
  AAAMessageBlock *msg = getRawData();
  EapRequestTls* request = getAppData();

  // Write type field
  EapRequestParser requestParser;
  requestParser.setRawData(msg);
  requestParser.setAppData(request);
  requestParser.parseAppToRaw();            //Here msg->wr_ptr() is placed on after Type field.
  //Write flags
  ACE_Byte  flags = request->get_flags();
  *(ACE_Byte*)msg->wr_ptr() = flags;
  msg->wr_ptr(1);
  if (TLS_LENGTH_INCLUDED(flags))
  {
    EAP_LOG(LM_DEBUG,"EapRequestTlsParser::parseAppToRaw LENGTH INCLUDED %d\n",request->get_data()->length());
    //Write length
    *(ACE_UINT32*)msg->wr_ptr() = htonl(request->get_data()->length());
    msg->wr_ptr(4);
  }
  //TLS data
  ACE_UINT32 length_to_copy = msg->end()-msg->wr_ptr();
  EAP_LOG(LM_DEBUG,"EapRequestTlsParser::parseAppToRaw LENGTH PACKET %d \n ",length_to_copy);
  if (!TLS_START(flags) && (request->get_data() != NULL))
    msg->copy(request->get_data()->rd_ptr(),length_to_copy);
}

//EAP-TLS Request parser
typedef EapRequestTlsParser EapResponseTlsParser;  

#endif
