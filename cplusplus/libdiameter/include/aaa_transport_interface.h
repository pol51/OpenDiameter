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


#ifndef __AAA_TRANSPORT_INTERFACE_H__
#define __AAA_TRANSPORT_INTERFACE_H__

#include <iostream>
#include "framework.h"
#include "aaa_data_defs.h"
#include "ace/Atomic_Op.h"

#define AAA_IO_ACCEPTOR_NAME  "Acceptor"
#define AAA_IO_CONNECTOR_NAME "Connector"

// interface implemented by transport
// specific (lower layer) methods. Assumes
// lower layer is connection oriented
class AAA_TransportInterface
{
   public:
      virtual int Open() = 0;
      virtual int Close() = 0;
    
      virtual int Connect(std::string &hostname, int port) = 0;
      virtual int Complete(AAA_TransportInterface *&iface) = 0;
    
      virtual int Listen(int port) = 0;
      virtual int Accept(AAA_TransportInterface *&iface) = 0;
    
      virtual int Send(void *data, size_t length) = 0;
      virtual int Receive(void *data, size_t length,
                       int timeout = 0) = 0;
      virtual ~AAA_TransportInterface() { }
};

// Address Utility
template<class ADDR_TYPE>
class AAA_TransportAddress
{
   public:
      virtual int GetLocalAddresses(size_t &count,
                                    ADDR_TYPE *&addrs) = 0;
      virtual void *GetAddressPtr(ADDR_TYPE &addr) = 0;
      virtual int GetAddressSize(ADDR_TYPE &addr) = 0;

   protected:
      virtual ~AAA_TransportAddress() { }
};

// RX event handler
class AAA_IO_Base;
class AAA_IO_RxHandler 
{
   public:
      virtual void Message(void *data,
                           size_t length) = 0;
      virtual void Error(int error,
                         const AAA_IO_Base *io) = 0;
    
   protected:
      AAA_IO_RxHandler() { }
      virtual ~AAA_IO_RxHandler() { }
};

// Base class for IO object
class AAA_IO_Base : public ACE_Task<ACE_MT_SYNCH>
{
   public:
      virtual int Open() = 0;
      virtual int Send(AAAMessageBlock *data) = 0;
      virtual int Close() = 0;
      virtual AAA_IO_RxHandler *Handler() = 0;
      std::string &Name() {
          return m_Name;
      }
      virtual ~AAA_IO_Base() { }

   protected:
      AAA_IO_Base(const char *name="") :
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
class AAA_IO : public AAA_IO_Base
{
   public:
      typedef enum {
         MAX_PACKET_LENGTH = 1024,
         DEFAULT_TIMEOUT = 0, // msec
      };
      AAA_IO(TX_IF &iface,
             const char *name = "") :
         AAA_IO_Base(name),
         m_Transport(std::auto_ptr<TX_IF>(&iface)),
         m_Running(false) {
      }
      virtual ~AAA_IO() {
         Close();
         // wait for thread to exit
         while (thr_count() > 0) {
             ACE_Time_Value tm(0, 100000);
             ACE_OS::sleep(tm);
         }
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
         data->Release();
         return bytes;
      }
      int Close() {
         // close the transport
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
      
   private:    
      int svc() {
         int bytes;
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
                    m_Transport->Close();
                    m_Running = false;
                    m_RxHandler.Error(errno, this);
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
class AAA_IO_Factory : public ACE_Task<ACE_MT_SYNCH>
{
   public:
      AAA_IO_Factory(char *name = "") :
          m_Perpetual(false),
          m_Running(false),
          m_Name(name) {
      }
      ~AAA_IO_Factory() {
          Close();
         
          // wait for thread to exit
          while (thr_count() > 0) {
              ACE_Time_Value tm(0, 100000);
              ACE_OS::sleep(tm);
          }
      }
      virtual int Open() {
          return m_Transport.Open();
      }
      virtual int Close() {
          // close the transport
          m_Running = false;
          return m_Transport.Close();
      }

      // for the subscriber layer
      virtual int Success(AAA_IO_Base *io) = 0;
      virtual int Failed() = 0;

      // for the TX layer
      virtual int Create(TX_IF *&newTransport) = 0;

      // re-scheduling attribute
      bool &Perpetual() {
          return m_Perpetual;
      }
    
   protected:
      bool m_Perpetual;
      bool m_Running;
      std::string m_Name;
      TX_IF m_Transport;

   protected:
      bool Activate() {
          if (! m_Running && (activate() == 0)) {
              m_Running = true;
          }
          return m_Running;
      }
    
   private:
      int svc() {
          int rc = 0;
          TX_IF *newTransport;
          do {
              if ((rc = Create(newTransport)) > 0) {
                  AAA_IO<TX_IF, RX_HANDLER> *io =
                     new AAA_IO<TX_IF, RX_HANDLER>
                          (*newTransport, m_Name.data());
                  if (io) {
                      try {
                          Success(io);
                          io->Open();
                          if (m_Perpetual) {
                              continue;
                          }
                          return (0);
                      }
                      catch (...) {
                          AAA_LOG(LM_ERROR,
                                  "(%P|%t) Factory %s error\n",
                                  m_Name.data());
                      }
                      newTransport->Close();
                      delete newTransport;
                  }
                  else {
                      AAA_LOG(LM_ERROR,
                              "(%P|%t) Factory %s memory allocation error\n",
                              m_Name.data());
                  }
                  Failed();
                  m_Running = false;
              }
              else if (rc < 0) {
                  AAA_LOG(LM_ERROR, "(%P|%t) Transport interface error: %s [%d=%s]\n",
                          m_Name.data(), errno, strerror(errno));
                  Failed();
                  m_Running = false;
              }
              // timeout, re-run
          } while (m_Running);
          return (0);
      }
};

// acceptor model for upper layer IO
template<class TX_IF, class RX_HANDLER>
class AAA_IO_Acceptor : public AAA_IO_Factory<TX_IF, RX_HANDLER>
{
   public:
      AAA_IO_Acceptor() :
          AAA_IO_Factory<TX_IF, RX_HANDLER>(AAA_IO_ACCEPTOR_NAME) { 
              AAA_IO_Factory<TX_IF, RX_HANDLER>::Perpetual() = true;
      }
      int Open(int port) {
          if (AAA_IO_Factory<TX_IF, RX_HANDLER>::Open() >= 0) {
              if (AAA_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Listen
                  (port) >= 0) {
                  AAA_LOG(LM_ERROR, "(%P|%t) Listening at %d\n",
                          port);
                  return AAA_IO_Factory<TX_IF, RX_HANDLER>::Activate();
              }
          }
          return (-1);
      }

   protected:
      int Create(TX_IF *&newTransport) {
          return AAA_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Accept
              (reinterpret_cast<AAA_TransportInterface*&>(newTransport));
      }
};

// connector model for upper layer IO
template<class TX_IF, class RX_HANDLER>
class AAA_IO_Connector : public AAA_IO_Factory<TX_IF, RX_HANDLER>
{
   public:
      AAA_IO_Connector() :
          AAA_IO_Factory<TX_IF, RX_HANDLER>(AAA_IO_CONNECTOR_NAME) { 
              AAA_IO_Factory<TX_IF, RX_HANDLER>::Perpetual() = false;
      }
      int Open(std::string &hostname, int port) {
          if (AAA_IO_Factory<TX_IF, RX_HANDLER>::Open() >= 0) {
             if (AAA_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Connect
                 (hostname, port) >= 0) {
                 AAA_LOG(LM_ERROR, "(%P|%t) Connection attempt to %s:%d\n",
                            hostname.data(), port);
                 return AAA_IO_Factory<TX_IF, RX_HANDLER>::Activate();
             }
          }
          return (-1);
      }

   protected:
      int Create(TX_IF *&newTransport) {
          return AAA_IO_Factory<TX_IF, RX_HANDLER>::m_Transport.Complete
                 ((AAA_TransportInterface*&)newTransport);
      }
};

#endif 