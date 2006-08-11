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

#include "ace/OS.h"
#include "pana_defs.h"
#include "pana_exceptions.h"
#include "pana_message.h"

template<> void PANA_HeaderParser::parseRawToApp()
{
   DiameterErrorCode st;
   DiameterDictionaryOption *opt = getDictData();
   AAAMessageBlock *aBuffer = reinterpret_cast<AAAMessageBlock*>(getRawData());
   PANA_MsgHeader *h = reinterpret_cast<PANA_MsgHeader*>(getAppData());

   char *p = aBuffer->rd_ptr();

   // version
   h->version() = AAAUInt8(*((AAAUInt8*)(p)));
   p += sizeof(ACE_UINT16);

   // length
   h->length() = ACE_NTOHS(*((ACE_UINT16*)(p)));
   p += sizeof(ACE_UINT16);

   // flags
   h->flags().request = *((AAAUInt8*)(p)) & 0x80 ? 1 : 0;
   h->flags().separate = *((AAAUInt8*)(p)) & 0x40 ? 1 : 0;
   h->flags().nap = *((AAAUInt8*)(p)) & 0x20 ? 1 : 0;
   h->flags().reserved = 0;
   p += sizeof(ACE_UINT16);

   // type
   h->type() = ACE_NTOHS(*((ACE_UINT16*)(p)));
   p += sizeof(ACE_UINT16);

   // sequence number
   h->seq() = ACE_UINT32(ACE_NTOHL(*((ACE_UINT32*)(p))));
   p += sizeof(ACE_UINT32);

   // start of AVP's
   aBuffer->rd_ptr(p);

   if (opt->option == PARSE_LOOSE) {
      return;
   }

   DiameterDictionaryManager dm(opt->protocolId);
   DiameterDictionaryHandle *handle = NULL;

   if ((handle = dm.getDictHandle(h->type(), 
                    0, h->flags().request)) == NULL) {
       ACE_DEBUG((LM_ERROR, "command (%d,r-flag=%1d,proto=%d) not found\n",
                  h->type(), h->flags().request, opt->protocolId));
       st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
       throw st;
   }

   h->setDictHandle(handle);
}

template<> void PANA_HeaderParser::parseAppToRaw()
{
   DiameterErrorCode st;
   DiameterDictionaryOption *opt = getDictData();
   AAAMessageBlock *aBuffer = reinterpret_cast<AAAMessageBlock*>
                                               (getRawData());
   PANA_MsgHeader *h = reinterpret_cast<PANA_MsgHeader*>
                                        (getAppData());

   aBuffer->wr_ptr(aBuffer->base());

   DiameterDictionaryManager dm(opt->protocolId);
   DiameterDictionaryHandle *handle = NULL;

   if (opt->option != PARSE_LOOSE) {
       if ((handle = dm.getDictHandle(h->type(), 
                 0, h->flags().request)) == NULL) {
          ACE_DEBUG((LM_ERROR, "command (%d,r-flag=%1d,proto=%d) not found\n",
                     h->type(), h->flags().request, opt->protocolId));
	      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
	      throw st;
	  }
   }

   char *p = aBuffer->base();

   // version
   *((ACE_UINT16*)(p)) = 0;
   *((AAAUInt8*)(p)) = h->version();
   p += sizeof(ACE_UINT16);

   // length
   *((ACE_UINT16*)(p)) = ACE_HTONS(h->length());
   p += sizeof(ACE_UINT16);

   // flags
   *((ACE_UINT16*)(p)) = 0;
   *((AAAUInt8*)(p)) |= h->flags().request ? 0x80 : 0x0;
   *((AAAUInt8*)(p)) |= h->flags().separate ? 0x40 : 0x0;
   *((AAAUInt8*)(p)) |= h->flags().nap ? 0x20 : 0x0;
   p += sizeof(ACE_UINT16);

   // type
   *((ACE_UINT16*)(p)) = ACE_HTONS(h->type());
   p += sizeof(ACE_UINT16);

   // seq number
   *((ACE_UINT32*)(p)) = ACE_UINT32(ACE_HTONL(h->seq()));
   p += sizeof(ACE_UINT32);

   aBuffer->wr_ptr(p);
   h->setDictHandle(handle);
}

PANA_MsgHeader::PANA_MsgHeader()
{
   m_Version = PANA_VERSION;
   m_Length  = 0;
   m_Type    = 0;
   m_SeqNum  = 0;
   m_DictHandle = NULL;
   ACE_OS::memset(&m_Flags, 0, sizeof(PANA_MsgHeader::Flags));
}

void PANA_AvpHeaderCodec::parseRawToApp()
{
  std::pair<char*, int> p = getRawData();
  AAAAvpHeader *h = getAppData();

  ACE_OS::memset(h, 0, sizeof(*h));

  h->code = ACE_NTOHS(*((ACE_UINT16*)p.first)); p.first+=2;
  h->flag.v = (*p.first & 0x80) ? 1 : 0;
  h->flag.m = (*p.first & 0x40) ? 1 : 0;
  h->flag.p = (*p.first & 0x20) ? 1 : 0;
  p.first +=2;

  h->length = ACE_NTOHS(*((ACE_UINT16*)p.first)); p.first+=4;  
  if (h->length == 0 || h->length > (ACE_UINT32)p.second)
    {
      DiameterErrorCode st;
      AAA_LOG(LM_ERROR, "invalid message length\n");
      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_MESSAGE_LENGTH);
      throw st;
    }
  if (h->flag.v == 1)
    {
      h->vendor = ACE_NTOHL(*((ACE_UINT32*)p.first)); p.first+=4;
    }
  h->value_p = p.first;
  setRawData(p);
}

void PANA_AvpHeaderCodec::parseAppToRaw()
{
  std::pair<char*, int> p = getRawData();
  AAAAvpHeader *h = getAppData();

  *((ACE_UINT16*)p.first) = ACE_NTOHS(h->code);
  p.first+=2;
  /* initialize this field to prepare for bit OR operation */
  *((ACE_UINT16*)p.first) = 0;               
  if (h->flag.v)
    {
      *((ACE_UINT16*)p.first)|=0x80;
    }
  if (h->flag.m)
    {
      *((ACE_UINT16*)p.first)|=0x40;
    }
  p.first+=2;

  *((ACE_UINT16*)p.first) |= ACE_NTOHS(h->length);
  p.first+=2;
  *((ACE_UINT16*)p.first) = 0;
  p.first+=2;

  if (h->flag.v)
    {
      *((ACE_UINT32*)p.first) = ACE_NTOHL(h->vendor);
      p.first+=4;
    }
  setRawData(p);
}


