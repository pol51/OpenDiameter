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

#ifndef __AAA_TRANSPORT_COLLECTOR_H__
#define __AAA_TRANSPORT_COLLECTOR_H__

#include "diameter_parser.h"
#include "aaa_transport_interface.h"

typedef enum {
    MSG_COLLECTOR_MAX_MSG_LENGTH  = 2048,
    MSG_COLLECTOR_MAX_MSG_BLOCK   = 10
};

class DiameterRxMsgCollectorHandler
{
    public:
        typedef enum {
           PARSING_ERROR          = 3000,
           BUG_IN_PARSING_CODE    = 3001,
           MEMORY_ALLOC_ERROR     = 3002,
           CORRUPTED_BYTE_STREAM  = 3003,
           INVALID_MSG            = 3004,
           TRANSPORT_ERROR        = 3005
        } COLLECTOR_ERROR;
    public:    
        virtual void Message(std::auto_ptr<DiameterMsg> msg) = 0;
        virtual void Error(COLLECTOR_ERROR error,
                           std::string &io_name) = 0;
        virtual int SendErrorAnswer(std::auto_ptr<DiameterMsg> &msg) = 0;
        virtual ~DiameterRxMsgCollectorHandler() { }
};

class DiameterRxMsgCollector : public Diameter_IO_RxHandler
{
    public:
        void RegisterHandler(DiameterRxMsgCollectorHandler &h) {
            m_Handler = &h;
        }
        void RemoveHandler() {
            m_Handler = NULL;
        }

        DiameterRxMsgCollector();
        virtual ~DiameterRxMsgCollector();

        void Message(void *data, size_t length);
        void Error(int error, const Diameter_IO_Base *io) {
            m_Handler->Error(DiameterRxMsgCollectorHandler::TRANSPORT_ERROR,
                    const_cast<Diameter_IO_Base*>(io)->Name());
        }

    protected:
        void SendFailedAvp(DiameterErrorCode &st);

    private:
        char *m_Buffer; // buffer of unprocessed received data
        int m_Offset; // current read offset from buffer
        int m_BufSize; // allocated size of rdBuffer
        int m_MsgLength; // current message length
        DiameterRxMsgCollectorHandler *m_Handler;
        AAA_RangedValue m_PersistentError;
};

class DiameterTxMsgCollector : public ACE_Task<ACE_MT_SYNCH>
{
    private:
        class TransmitDatum {
            public:
                TransmitDatum(DiameterMsg *msg,
                              Diameter_IO_Base *io,
                              bool consume) :
                     m_Msg(msg),
                     m_IO(io),
                     m_Consume(consume) {
                }
                virtual ~TransmitDatum() {
                    if (m_Consume && m_Msg) {
                        delete m_Msg;
                    }
                }
                DiameterMsg *GetMsg() {
                    return m_Msg;
                }
                Diameter_IO_Base *GetIO() {
                    return m_IO;
                }
                bool IsConsumed() {
                    return m_Consume;
                }

            private:
                DiameterMsg      *m_Msg;
                Diameter_IO_Base *m_IO;
                bool              m_Consume;
        };

    public:
        DiameterTxMsgCollector() :
            m_Active(false),
            m_Condition(*(new ACE_Mutex)),
            m_Buffer(NULL),
            m_BlockCount(1) {
        }
        virtual ~DiameterTxMsgCollector() {
            Stop();
            delete &(m_Condition.mutex());
        }
        bool Start() {
            if (! m_Active && (activate() == 0)) {
                m_Active = true;
            }
            return m_Active;
        }
        void Stop() {
            m_Active = false;
            m_Condition.signal();
            // wait for thread to exit
            AAA_MutexScopeLock guard(m_ExitMutex);
        }
        int Send(std::auto_ptr<DiameterMsg> &msg,
                 Diameter_IO_Base *io,
                 bool consume) {
            DiameterMsg *ptr = (consume) ? msg.release() : msg.get();
            m_SendQueue.Enqueue(new TransmitDatum(ptr, io, consume));
            AAA_MutexScopeLock guard(m_Condition.mutex());
            m_Condition.signal();
            return (0);
        }

    private:
        //
        // Variables for transmitter thread
        //
        bool m_Active;
        ACE_Mutex m_ExitMutex;
        ACE_Condition<ACE_Mutex> m_Condition;
        AAA_ProtectedQueue<TransmitDatum*> m_SendQueue;

        //
        // Variables for buffer pool
        //
        AAAMessageBlock *m_Buffer;
        short m_BlockCount;

    private:
        int SafeSend(DiameterMsg *msg,
                     Diameter_IO_Base *io);

        int svc() {
           AAA_MutexScopeLock guard(m_ExitMutex);
           m_Buffer = AAAMessageBlock::Acquire(DIAMETER_CFG_TRANSPORT()->rx_buffer_size);
           do {
               AAA_MutexScopeLock guard(m_Condition.mutex());
               m_Condition.wait();
               while (! m_SendQueue.IsEmpty()) {
                   std::auto_ptr<TransmitDatum> datum(m_SendQueue.Dequeue());
                   SafeSend(datum->GetMsg(), datum->GetIO());
               }
           } while (m_Active);
            m_Buffer->Release();
           return (0);
        }
};

#endif

