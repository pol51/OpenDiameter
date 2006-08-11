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

#include "aaa_transport_collector.h"

AAA_MsgCollector::AAA_MsgCollector() : 
    m_Buffer(NULL),
    m_Offset(0),
    m_BufSize(0),
    m_MsgLength(0),
    m_Handler(NULL)
{
   m_Buffer = (char*)ACE_OS::malloc(DIAMETER_HEADER_SIZE +
                                    MAX_MSG_LENGTH);
   if (m_Buffer != NULL) {
      m_BufSize = DIAMETER_HEADER_SIZE + MAX_MSG_LENGTH;
      ACE_OS::memset(m_Buffer, 0, m_BufSize);
      m_PersistentError.Reset(0, 0, 
              (m_BufSize * MAX_MSG_BLOCK)/sizeof(ACE_UINT32));
   }   
}

AAA_MsgCollector::~AAA_MsgCollector()
{
   if (m_Buffer) {
      ACE_OS::free(m_Buffer); 
   }
}

void AAA_MsgCollector::Message(void *data, size_t length)
{
   std::string emptyStr(""); 
   if (m_Buffer == NULL || m_Handler == NULL) {
      return;
   }

   int r_bytes = 0;
   bool bHasHeaderError = false;

   while (length > 0) {
      r_bytes = (length > size_t(m_BufSize - m_Offset)) ?
                m_BufSize - m_Offset : length;
      ACE_OS::memcpy(m_Buffer + m_Offset, data, r_bytes);
          
      length -= r_bytes;
      data = (char*)data + r_bytes;
      m_Offset += (r_bytes > 0) ? r_bytes : 0;

      while (m_Offset > DIAMETER_HEADER_SIZE) {
         if (m_MsgLength == 0) {

            DiameterMsgHeader hdr; 
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
               std::string eDesc;
               st.get(eType, eCode, eDesc);

               AAA_RangedValue lengthRange
                  (hdr.length, DIAMETER_HEADER_SIZE, m_BufSize * MAX_MSG_BLOCK);

               if ((eCode == AAA_COMMAND_UNSUPPORTED) &&
                   ! lengthRange.InRange()) {
                   // cannot trust reported header length

                   aBuffer->Release();
                   if (++m_PersistentError) {
                       eDesc = "To many persistent errors in data";
                       m_Handler->Error
                          (AAA_MsgCollectorHandler::TRANSPORT_ERROR, eDesc);
                       return;
                   }
                   m_Offset -= sizeof(ACE_UINT32);
                   ACE_OS::memcpy(m_Buffer, m_Buffer + sizeof(ACE_UINT32), 
                                 m_Offset);
                   continue;
               }
               else { 
                   // can trust reported header length despite error
                   bHasHeaderError = true;
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
                  m_Handler->Error
                      (AAA_MsgCollectorHandler::ALLOC_ERROR, emptyStr);
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

                   DiameterMsgPayloadParser pp;
                   pp.setRawData(aBuffer);
                   pp.setAppData(&msg->acl);
                   pp.setDictData(msg->hdr.getDictHandle());
 
                   aBuffer->rd_ptr(aBuffer->base() + DIAMETER_HEADER_SIZE);
                   try {
                      pp.parseRawToApp();
                   }
                   catch (DiameterErrorCode &st) {
                      throw (0);
                   }

                   aBuffer->Release();
                }
                catch (...) {
                   if (msg.get() == NULL) {
                      m_BufSize = 0; 
                      m_Offset = 0;
                      m_MsgLength = 0;
                      m_Handler->Error
                          (AAA_MsgCollectorHandler::ALLOC_ERROR, emptyStr);
                      return; 
                   }
                   else {
                      aBuffer->Release();
                      m_Offset -= m_MsgLength;
                      ACE_OS::memcpy(m_Buffer, m_Buffer + m_Offset, 
                                     m_MsgLength);
                      m_Handler->Error
                          (AAA_MsgCollectorHandler::PARSING_ERROR, emptyStr);
                      continue;
                   }
                }
            
                m_Handler->Message(msg);

                m_PersistentError.Reset(0, 0, 
                    (m_BufSize * MAX_MSG_BLOCK)/sizeof(ACE_UINT32));
            }
            else { 
                m_Handler->Error
                    (AAA_MsgCollectorHandler::PARSING_ERROR, emptyStr);
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



