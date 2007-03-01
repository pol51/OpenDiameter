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


#ifndef __AAA_TRANSPORT_INTERFACE_H__
#define __AAA_TRANSPORT_INTERFACE_H__

#include <iostream>
#include "framework.h"
#include "aaa_data_defs.h"
#include "aaa_garbage_collector.h"
#include "ace/Atomic_Op.h"

#define AAA_IO_CONNECTOR_NAME "Connector"
#define AAA_IO_ACCEPTOR_NAME  "Acceptor"

typedef std::list<diameter_address_t*> DiameterHostIpLst;

// interface implemented by transport
// specific (lower layer) methods. Assumes
// lower layer is connection oriented
template<class ADDR_TYPE>
class DiameterTransportInterface
{
   public:
      virtual int Open() = 0;
      virtual int Close() = 0;

      virtual int Connect(std::string &hostname, int port) = 0;
      virtual int Complete(DiameterTransportInterface<ADDR_TYPE> *&iface) = 0;

      virtual int Listen(int port, ADDR_TYPE addrToBind) = 0;
      virtual int Accept(DiameterTransportInterface<ADDR_TYPE> *&iface) = 0;

      virtual int Send(void *data, size_t length) = 0;
      virtual int Receive(void *data, size_t length,
                       int timeout = 0) = 0;

      virtual int TransportProtocolInUse() = 0;

      virtual ~DiameterTransportInterface() { }
};

// Address Utility
template<class ADDR_TYPE>
class DiameterTransportAddress
{
   public:
      virtual int GetLocalAddresses(size_t &count,
                                    ADDR_TYPE *&addrs) = 0;
      virtual void *GetAddressPtr(ADDR_TYPE &addr) = 0;
      virtual int GetAddressSize(ADDR_TYPE &addr) = 0;

   protected:
      virtual ~DiameterTransportAddress() { }
};

// RX event handler
class Diameter_IO_Base;
class Diameter_IO_RxHandler 
{
   public:
      virtual void Message(void *data,
                           size_t length) = 0;
      virtual void Error(int error,
                         const Diameter_IO_Base *io) = 0;

   protected:
      Diameter_IO_RxHandler() { }
      virtual ~Diameter_IO_RxHandler() { }
};

// Base class for IO object
class Diameter_IO_Base :
   public ACE_Task<ACE_MT_SYNCH>,
   public DiameterGarbageCollectorAttribute
{
   public:
      virtual int Open() = 0;
      virtual int Send(AAAMessageBlock *data) = 0;
      virtual int Close() = 0;
      virtual int TransportProtocolInUse() = 0;
      virtual Diameter_IO_RxHandler *Handler() = 0;
      std::string &Name() {
          return m_Name;
      }
      virtual ~Diameter_IO_Base() { }

   protected:
      Diameter_IO_Base(const char *name="") :
          m_Name(name) {
      }
      std::string m_Name;
};

//
// per transport implementation
//
// Warning: Objects of this class MUST
// be schedule to a task/job queue before
// it is usable and can be delete
//
template<class TX_IF, class RX_HANDLER>
class Diameter_IO : public Diameter_IO_Base
{
   public:
      typedef enum {
         MAX_PACKET_LENGTH = 1024,
         DEFAULT_TIMEOUT = 0, // msec
      };
      Diameter_IO(TX_IF &iface,
             const char *name = "") :
         Diameter_IO_Base(name),
         m_Transport(std::auto_ptr<TX_IF>(&iface)),
         m_Running(false) {
      }
      virtual ~Diameter_IO() {
         // wait for thread to exit
         Close();
         AAA_MutexScopeLock guard(m_Mutex);
         m_Transport.release();
      }
      int Open() {
         if (! m_Running && (activate() == 0)) {
             m_Running = true;
             return m_Transport->Open();
         }
         return (-1);
      }
      int Send(AAAMessageBlock *data) {
         int bytes = m_Transport->Send(data->base(),
                                       data->length());
         return bytes;
      }
      int TransportProtocolInUse() {
         return m_Transport->TransportProtocolInUse();
      }
      int Close() {
         m_Running = false;
         return m_Transport->Close();
      }
      RX_HANDLER *Handler() {
         return &m_RxHandler;
      }

   protected:
      // transport
      std::auto_ptr<TX_IF> m_Transport;

      // reader handler
      RX_HANDLER m_RxHandler;
      char m_ReadBuffer[MAX_PACKET_LENGTH];

      // thread signal
      bool m_Running;
      ACE_Mutex m_Mutex;

   private:
      int svc() {
         int bytes;
         AAA_MutexScopeLock guard(m_Mutex);
         do {
             // Reciever
             bytes = m_Transport->Receive(m_ReadBuffer,
                                          MAX_PACKET_LENGTH);
             try {
                if (bytes > 0) {
                    m_RxHandler.Message(m_ReadBuffer, bytes);
                }
                else if (bytes == 0) {
                    // timeout
                } 
                else if (m_Running) {
                    throw DiameterBaseException(DiameterBaseException::IO_FAILURE,
                                     "Receive error");
                }
             }
             catch (DiameterBaseException &e) {
                switch (e.Code()) {
                   case DiameterBaseException::IO_FAILURE:
                       m_Transport->Close();
                       m_RxHandler.Error(errno, this);
                       m_Running = false;
                       return (0);
                   default:
                       // continue on
                       break;
                }
             }
             catch (...) {
             }
         } while (m_Running);
         return (0);
      }
};

// base class for acceptor and connector classes
template<class TX_IF, class RX_HANDLER>
class Diameter_IO_Factory : public ACE_Task<ACE_MT_SYNCH>
{
   public:
      Diameter_IO_Factory(char *name = "") :
          m_Perpetual(false),
          m_Active(false),
          m_Name(name) {
      }
      virtual ~Diameter_IO_Factory() {
      }
      virtual int Open() {
          return m_Transport.Open();
      }
      virtual int Close() {
          return m_Transport.Close();
      }

      // for the subscriber layer
      virtual int Success(Diameter_IO_Base *io) = 0;
      virtual int Failed() = 0;

      // for the TX layer
      virtual int Create(TX_IF *&newTransport) = 0;

      // re-scheduling attribute
      bool &Perpetual() {
          return m_Perpetual;
      }

   protected:
      bool m_Perpetual;
      bool m_Active;
      std::string m_Name;
      TX_IF m_Transport;
      ACE_Mutex m_Mutex;

   protected:
      bool Activate() {
          if (! m_Active && (activate() == 0)) {
              m_Active = true;
          }
          return m_Active;
      }
      void Shutdown() {
          // wait for thread to exit
          if (m_Active) {
              m_Active = false;
              AAA_MutexScopeLock guard(m_Mutex);
          }
      }

   private:
      int svc() {
          int rc = 0;
          TX_IF *newTransport;
          AAA_MutexScopeLock guard(m_Mutex);
          do {
              if ((rc = Create(newTransport)) > 0) {
                  Diameter_IO<TX_IF, RX_HANDLER> *io =
                     new Diameter_IO<TX_IF, RX_HANDLER>
                          (*newTransport, m_Name.c_str());
                  if (io) {
                      try {
                          Success(io);
                          io->Open();
                          if (m_Perpetual) {
                              continue;
                          }
                          m_Active = false;
                          return (0);
                      }
                      catch (...) {
                          AAA_LOG((LM_ERROR,
                                  "(%P|%t) Factory %s error\n",
                                  m_Name.c_str()));
                      }
                      newTransport->Close();
                      delete newTransport;
                  }
                  else {
                      AAA_LOG((LM_ERROR,
                              "(%P|%t) Factory %s memory allocation error\n",
                              m_Name.c_str()));
                  }
                  Failed();
                  m_Active = false;
              }
              else if (rc < 0) {
                  AAA_LOG((LM_ERROR, "(%P|%t) IO Factory error: %s [%d=%s]\n",
                          m_Name.c_str(), errno, strerror(errno)));
                  Failed();
                  m_Active = false;
              }
              // timeout, re-run
          } while (m_Active);
          return (0);
      }
};

// acceptor model for upper layer IO
template<class TX_IF, class ADDR_TYPE, class RX_HANDLER>
class Diameter_IO_Acceptor : public Diameter_IO_Factory<TX_IF, RX_HANDLER>
{
   public:
      Diameter_IO_Acceptor() :
          Diameter_IO_Factory<TX_IF, RX_HANDLER>(AAA_IO_ACCEPTOR_NAME) {
              Diameter_IO_Factory<TX_IF, RX_HANDLER>::Perpetual() = true;
      }
      int Open(int port, ADDR_TYPE addrToBind) {
          if (Diameter_IO_Factory<TX_IF, RX_HANDLER>::Open() >= 0) {
              if (Diameter_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Listen
                  (port, addrToBind) >= 0) {
                  return Diameter_IO_Factory<TX_IF, RX_HANDLER>::Activate();
              }
          }
          return (-1);
      }
      int Close() {
          Diameter_IO_Factory<TX_IF, RX_HANDLER>::Close();
          Diameter_IO_Factory<TX_IF, RX_HANDLER>::Shutdown();
          return (0);
      }

   protected:
      int Create(TX_IF *&newTransport) {
          AAA_LOG((LM_INFO, "(%P|%t) Waiting for incomming connection ...\n"));
          return Diameter_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Accept
              (reinterpret_cast<DiameterTransportInterface<ADDR_TYPE> *&>(newTransport));
      }
};

// connector model for upper layer IO
template<class TX_IF, class ADDR_TYPE, class RX_HANDLER>
class Diameter_IO_Connector : public Diameter_IO_Factory<TX_IF, RX_HANDLER>
{
   public:
      Diameter_IO_Connector() :
          Diameter_IO_Factory<TX_IF, RX_HANDLER>(AAA_IO_CONNECTOR_NAME) {
              Diameter_IO_Factory<TX_IF, RX_HANDLER>::Perpetual() = false;
      }
      int Open(std::string &hostname, int port) {
          if (Diameter_IO_Factory<TX_IF, RX_HANDLER>::Open() >= 0) {
             if (Diameter_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Connect
                 (hostname, port) >= 0) {
                 AAA_LOG((LM_INFO, "(%P|%t) Trying to connect to to %s:%d\n",
                            hostname.c_str(), port));
                 return Diameter_IO_Factory<TX_IF, RX_HANDLER>::Activate();
             }
          }
          return (-1);
      }

   protected:
      int Create(TX_IF *&newTransport) {
          AAA_LOG((LM_INFO, "(%P|%t) Checking if connection attempt succeeded ...\n"));
          return Diameter_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Complete
                 ((DiameterTransportInterface<ADDR_TYPE> *&)newTransport);
      }
};

///
/// Internal garbage collector definitions
///
typedef DiameterGarbageCollectorSingleton<Diameter_IO_Base>
             DiameterIOGC;

typedef ACE_Singleton<DiameterIOGC,
                      ACE_Recursive_Thread_Mutex> 
                      DiameterIOGC_S;

#define DIAMETER_IO_GC_ROOT() (DiameterIOGC_S::instance())
#define DIAMETER_IO_GC()      (DiameterIOGC_S::instance()->Instance())

#endif 
