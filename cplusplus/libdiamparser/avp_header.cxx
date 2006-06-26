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
/* $Id: avp_header.cxx,v 1.22 2006/03/16 17:01:50 vfajardo Exp $ */

#include "avp_header.h"
#include "parser.h"

template<>
void AvpHeaderCodec::parseRawToApp()
{
  std::pair<char*, int> p = getRawData();
  AAAAvpHeader *h = getAppData();

  ACE_OS::memset(h, 0, sizeof(*h));

  h->code = ACE_NTOHL(*((ACE_UINT32*)p.first)); p.first+=4;
  h->flag.v = (*p.first & 0x80) ? 1 : 0;
  h->flag.m = (*p.first & 0x40) ? 1 : 0;
  h->flag.p = (*p.first & 0x20) ? 1 : 0;

  h->length = ACE_NTOHL(*((ACE_UINT32*)p.first)) & 0x00ffffff; p.first+=4;
#ifdef WIN32
  if (h->length == 0 || h->length > (ACE_UINT32)p.second)
#else
  if (h->length == 0 || h->length > p.second)
#endif
    {
      AAAErrorStatus st;
      AAA_LOG(LM_ERROR, "invalid message length\n");
      st.set(NORMAL, AAA_INVALID_MESSAGE_LENGTH);
      throw st;
    }
  if (h->flag.v == 1)
    {
      h->vendor = ACE_NTOHL(*((ACE_UINT32*)p.first)); p.first+=4;
    }
  h->value_p = p.first;
  rawData = p;
}

template<>
void AvpHeaderCodec::parseAppToRaw()
{
  std::pair<char*, int> p = getRawData();
  AAAAvpHeader *h = getAppData();

  *((ACE_UINT32*)p.first) = ACE_HTONL(h->code);
  p.first+=4;
  /* initialize this field to prepare for bit OR operation */
  *((ACE_UINT32*)p.first) = 0;               
  if (h->flag.v)
    {
      *p.first|=0x80;
    }
  if (h->flag.m)
    {
      *p.first|=0x40;
    }
#if 0
  if (h->flag.p)
    {
      *p.first|=0x20;
    }
#endif

  *((ACE_UINT32*)p.first) |= ACE_HTONL(h->length & 0x00ffffff);
  p.first+=4;

  if (h->flag.v)
    {
      *((ACE_UINT32*)p.first) = ACE_HTONL(h->vendor);
      p.first+=4;
    }
  rawData = p;
}

/// Create a list of AVP headers.
void
AAAAvpHeaderList::create(AAAMessageBlock *aBuffer)
  throw(AAAErrorStatus)
{
  AAAAvpHeader h;
  AAAErrorStatus st;
  char *start = aBuffer->rd_ptr();
  char *end = aBuffer->base()+aBuffer->size();

  /* codec checks */
  if (codec == NULL)
    {
      AAA_LOG(LM_ERROR, "No codec available\n");
      st.set(BUG, INVALID_PARSER_USAGE);
      throw st;
    }
    
  for (char *p=start; p<end; p+=adjust_word_boundary(h.length))
    {
      /* header decoding */
      codec->setRawData(std::pair<char*, int>(p, end-p));
      codec->setAppData(&h);
      codec->parseRawToApp();
      
      h.value_p = p;      // Store the pointer to the header head
      push_back(h);  
    }

  aBuffer->rd_ptr(end);
}

