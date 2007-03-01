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

#include "aaa_transport_collector.h"

DiameterRxMsgCollector::DiameterRxMsgCollector() :
    m_Buffer(NULL),
    m_Offset(0),
    m_BufSize(0),
    m_MsgLength(0),
    m_Handler(NULL)
{
   m_Buffer = (char*)ACE_OS::malloc(DIAMETER_HEADER_SIZE +
                                    DIAMETER_CFG_TRANSPORT()->rx_buffer_size);
   if (m_Buffer != NULL) {
      m_BufSize = DIAMETER_HEADER_SIZE + DIAMETER_CFG_TRANSPORT()->rx_buffer_size;
      ACE_OS::memset(m_Buffer, 0, m_BufSize);
      m_PersistentError.Reset(0, 0,
              (m_BufSize * 
               MSG_COLLECTOR_MAX_MSG_BLOCK)/sizeof(ACE_UINT32));
   }
}

DiameterRxMsgCollector::~DiameterRxMsgCollector()
{
   if (m_Buffer) {
      ACE_OS::free(m_Buffer);
   }
}

void DiameterRxMsgCollector::Message(void *data, size_t length)
{
   if (m_Buffer == NULL || m_Handler == NULL) {
      return;
   }

   int r_bytes = 0;
   bool bHasHeaderError = false;
   std::string eDesc;

   while (length > 0) {
      r_bytes = (length > size_t(m_BufSize - m_Offset)) ?
                m_BufSize - m_Offset : length;
      ACE_OS::memcpy(m_Buffer + m_Offset, data, r_bytes);

      length -= r_bytes;
      data = (char*)data + r_bytes;
      m_Offset += (r_bytes > 0) ? r_bytes : 0;

      while (m_Offset > DIAMETER_HEADER_SIZE) {

         DiameterMsgHeader hdr;
         memset(&hdr, 0x0, sizeof(hdr));

         if (m_MsgLength == 0) {

            AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire
                (m_Buffer, DIAMETER_HEADER_SIZE);

            DiameterMsgHeaderParser hp;
            hp.setRawData(aBuffer);
            hp.setAppData(&hdr);
            hp.setDictData(DIAMETER_PARSE_STRICT);

            try {
               hp.parseRawToApp();
               bHasHeaderError = false;
            }
            catch (DiameterErrorCode &st) {
               int eCode;
               AAA_PARSE_ERROR_TYPE eType;
               st.get(eType, eCode, eDesc);

               AAA_RangedValue lengthRange
                  (hdr.length, DIAMETER_HEADER_SIZE, m_BufSize * 
                   MSG_COLLECTOR_MAX_MSG_BLOCK);

               if ((eCode == AAA_COMMAND_UNSUPPORTED) &&
                   ! lengthRange.InRange()) {
                   // cannot trust reported header length

                   aBuffer->Release();
                   if (++m_PersistentError) {
                       eDesc = "To many persistent errors in data";
                       m_Handler->Error
                          (DiameterRxMsgCollectorHandler::CORRUPTED_BYTE_STREAM, eDesc);
                       return;
                   }
                   m_Offset -= sizeof(ACE_UINT32);
                   ACE_OS::memcpy(m_Buffer, m_Buffer + sizeof(ACE_UINT32),
                                 m_Offset);
                   continue;
               }
               else if (eType == AAA_PARSE_ERROR_TYPE_NORMAL) {
                   // can trust reported header length despite error
                   bHasHeaderError = true;
                   std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(hdr, eCode);
                   m_Handler->SendErrorAnswer(answerMsg);
               }
               else {
                   eDesc = "Bug present in the parsing code !!!!";
                   m_Handler->Error
                      (DiameterRxMsgCollectorHandler::BUG_IN_PARSING_CODE, eDesc);
                   return;
               }
            }
            aBuffer->Release();
            m_MsgLength = hdr.length;
         }

         if (m_MsgLength > m_Offset) {
            if (m_MsgLength > m_BufSize) {
               m_Buffer = (char*)ACE_OS::realloc(m_Buffer,
                                                 m_MsgLength + 1);
               if (m_Buffer) {
                  ACE_OS::memset(m_Buffer + m_BufSize, 0,
                                 m_MsgLength - m_BufSize + 1);
                  m_BufSize = m_MsgLength + 1;
               }
               else {
                  m_BufSize = 0;
                  m_Offset = 0;
                  m_MsgLength = 0;

                  eDesc = "Byte buffer too small but failed to re-alloc";
                  m_Handler->Error
                      (DiameterRxMsgCollectorHandler::MEMORY_ALLOC_ERROR, eDesc);
                  return; 
               }
            }
            else {
	       break;
            }
         }
         else {

            if (! bHasHeaderError) {

                AAAMessageBlock *aBuffer = NULL;
                std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
                try {
                   if (msg.get() == NULL) {
                      throw (0);
                   }
                   aBuffer = AAAMessageBlock::Acquire
                                (m_Buffer, m_MsgLength);
                   if (aBuffer == NULL) {
                      throw (0);
                   }

                   if (hdr.getDictHandle() == 0) {

                      DiameterMsgHeaderParser hp;
                      hp.setRawData(aBuffer);
                      hp.setAppData(&msg->hdr);
                      hp.setDictData(DIAMETER_PARSE_STRICT);

                      try {
                         hp.parseRawToApp();
                      }
                      catch (DiameterErrorCode &st) {
                         throw (0); 
                      }
                   }
                   else {
                      msg->hdr = hdr;
                   }

                   DiameterMsgPayloadParser pp;
                   pp.setRawData(aBuffer);
                   pp.setAppData(&msg->acl);
                   pp.setDictData(msg->hdr.getDictHandle());

                   aBuffer->rd_ptr(aBuffer->base() + DIAMETER_HEADER_SIZE);
                   pp.parseRawToApp();
                   aBuffer->Release();
                }
                catch (DiameterErrorCode &st) {

                   aBuffer->Release();

                   int eCode;
                   AAA_PARSE_ERROR_TYPE eType;
                   st.get(eType, eCode, eDesc);

                   if (eType == AAA_PARSE_ERROR_TYPE_NORMAL) {

                       SendFailedAvp(st);

                       aBuffer->Release();
                       m_Offset -= m_MsgLength;
                       ACE_OS::memcpy(m_Buffer, m_Buffer + m_Offset,
                                      m_MsgLength);

                       eDesc = "Payload error encounted in newly arrived message";
                       m_Handler->Error
                           (DiameterRxMsgCollectorHandler::PARSING_ERROR, eDesc);
                       continue;
                   }
                   else {
                       eDesc = "Bug present in the parsing code !!!!";
                       m_Handler->Error
                          (DiameterRxMsgCollectorHandler::BUG_IN_PARSING_CODE, eDesc);
                       return;
                   }
                } catch (...) {
                   if ((msg.get() == NULL) || (aBuffer == NULL)) {
                      m_BufSize = 0;
                      m_Offset = 0;
                      m_MsgLength = 0;

                      eDesc = "Byte buffer too small but failed to re-alloc";
                      m_Handler->Error
                          (DiameterRxMsgCollectorHandler::MEMORY_ALLOC_ERROR, eDesc);
                      return;
                   }
                   if (aBuffer) {
                      aBuffer->Release();
                   }
                }

                m_Handler->Message(msg);

                m_PersistentError.Reset(0, 0,
                    (m_BufSize *
                     MSG_COLLECTOR_MAX_MSG_BLOCK)/sizeof(ACE_UINT32));
            }
            else {
                eDesc = "Header error encounted in newly arrived message";
                m_Handler->Error
                    (DiameterRxMsgCollectorHandler::PARSING_ERROR, eDesc);
            }

            if (m_MsgLength < m_Offset) {
               ACE_OS::memcpy(m_Buffer, m_Buffer + m_MsgLength,
                              m_Offset - m_MsgLength);
               m_Offset -= m_MsgLength;
            }
            else {
               m_Offset = 0;
            }

            ACE_OS::memset(m_Buffer + m_Offset, 0, m_BufSize - m_Offset);
            m_MsgLength = 0;
         }
      }
   }
}

void DiameterRxMsgCollector::SendFailedAvp(DiameterErrorCode &st)
{
   // AAA_OUT_OF_SPACE
   // AAA_INVALID_AVP_VALUE
   // AAA_AVP_UNSUPPORTED
   // AAA_MISSING_AVP
   // AAA_INVALID_AVP_BITS
   // AAA_AVP_OCCURS_TOO_MANY_TIMES

   // TBD:
   // std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(msg, eCode);
   // m_Handler->SendErrorAnswer(answerMsg);
}

int DiameterTxMsgCollector::SafeSend(DiameterMsg *msg,
                                     Diameter_IO_Base *io)
{
   do {
       msg->acl.reset();
       m_Buffer->reset();

       DiameterMsgHeaderParser hp;
       hp.setRawData(m_Buffer);
       hp.setAppData(&msg->hdr);
       hp.setDictData(DIAMETER_PARSE_STRICT);

       try {
          hp.parseAppToRaw();
       }
       catch (DiameterErrorCode &st) {
          ACE_UNUSED_ARG(st);
          return (-1);
       }

       DiameterMsgPayloadParser pp;
       pp.setRawData(m_Buffer);
       pp.setAppData(&msg->acl);
       pp.setDictData(msg->hdr.getDictHandle());

       try { 
          pp.parseAppToRaw();
       }
       catch (DiameterErrorCode &st) {
          AAA_PARSE_ERROR_TYPE type;
          int code;
          st.get(type, code);
          if ((type == AAA_PARSE_ERROR_TYPE_NORMAL) && (code == AAA_OUT_OF_SPACE)) {
              if (m_BlockCount < MSG_COLLECTOR_MAX_MSG_BLOCK) {
                  m_BlockCount ++;
                  m_Buffer->Release();
                  m_Buffer = AAAMessageBlock::Acquire
                             (DIAMETER_CFG_TRANSPORT()->rx_buffer_size * m_BlockCount);
                  continue;
              }
              AAA_LOG((LM_ERROR, "(%P|%t) Not enough block space for transmission\n"));
          }
          return (-1);
      }

      msg->hdr.length = m_Buffer->wr_ptr() - m_Buffer->base();
      try {
          hp.parseAppToRaw();
      }
      catch (DiameterErrorCode &st) {
          return (-1);
      }
      break;
   }
   while (true);

   m_Buffer->length(msg->hdr.length);
   return io->Send(m_Buffer);
}
