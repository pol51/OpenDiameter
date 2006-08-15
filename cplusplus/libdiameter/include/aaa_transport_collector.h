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

#ifndef __AAA_TRANSPORT_COLLECTOR_H__
#define __AAA_TRANSPORT_COLLECTOR_H__

#include "diameter_parser.h"
#include "aaa_transport_interface.h"

class DiameterMsgCollectorHandler 
{
    public:
        typedef enum {
           PARSING_ERROR   = 3000,
           ALLOC_ERROR     = 3001,
           TRANSPORT_ERROR = 3002,
           INVALID_MSG     = 3003,
        } COLLECTOR_ERROR;    
    public:
        virtual void Message(std::auto_ptr<DiameterMsg> msg) = 0;
        virtual void Error(COLLECTOR_ERROR error, 
            std::string &io_name) = 0;
        virtual ~DiameterMsgCollectorHandler() { }
};

class DiameterMsgCollector : public Diameter_IO_RxHandler
{
    public:
        typedef enum {
           MAX_MSG_LENGTH  = 2048,
           MAX_MSG_BLOCK   = 10
        };
    
    public:
        void RegisterHandler(DiameterMsgCollectorHandler &h) {
            m_Handler = &h;
        }
        void RemoveHandler() {
            m_Handler = NULL;
        }

        DiameterMsgCollector();
        virtual ~DiameterMsgCollector();

        void Message(void *data, size_t length);
        void Error(int error, const Diameter_IO_Base *io) {
            m_Handler->Error(DiameterMsgCollectorHandler::TRANSPORT_ERROR,
                    const_cast<Diameter_IO_Base*>(io)->Name());
        }

    private:
        char *m_Buffer; // buffer of unprocessed received data
        int m_Offset; // current read offset from buffer
        int m_BufSize; // allocated size of rdBuffer
        int m_MsgLength; // current message length
        DiameterMsgCollectorHandler *m_Handler;
        DiameterRangedValue m_PersistentError;
};

#endif

