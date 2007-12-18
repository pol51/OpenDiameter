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

#ifndef __EAP_GPSK_HXX__
#define __EAP_GPSK_HXX__

#include <ace/Basic_Types.h>
#include "eap.hxx"
#include "eap_log.hxx"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(EAP_GPSK_EXPORT)
       #define EAP_GPSK_EXPORTS __declspec(dllexport)
   #else
       #define EAP_GPSK_EXPORTS __declspec(dllimport)
   #endif
#else
   #define EAP_GPSK_EXPORTS
   #define EAP_GPSK_IMPORTS
#endif

/// \def EAP Request/Response Type code temporally assigned for
/// EAP-Gpsk.  This should be replaced with an IANA allocated value.
#define GPSK_METHOD_TYPE  200
#define GPSK_MAX_PKT_SIZE 1024

/// Enumerations
/*
   The following is the initial protected data PData/Specifier registry
   setup:

   o  0x000000 : Reserved
   o  0x000001 : Protected Results Indication
*/
typedef enum {
   EAP_GPSK_PD_SPECIFIER_RESERVED = 0x0,
   EAP_GPSK_PD_SPECIFIER_PROTECTED_RESULT_INDICATION = 0x1,
} EAP_GPSK_PD_SPECIFIER;

/*
   The PData/Specifier field is 24 bits long and all other values are
   available via IANA registration.  Each extension needs to indicate
   whether confidentiality protection for transmission between the EAP
   peer and the EAP server is mandatory.  The following layout
   represents the initial Failure-Code registry setup:

   o  0x00000001: PSK Not Found
   o  0x00000002: Authentication Failure
   o  0x00000003: Authorization Failure
*/
typedef enum {
   EAP_GPSK_FAILURE_PSK_NOT_FOUND = 0x1,
   EAP_GPSK_FAILURE_AUTHENTICATION_FAILURE = 0x2,
   EAP_GPSK_FAILURE_AUTHORIZATION_FAILURE = 0x3
} EAP_GPSK_FAILURE;

/// EAP-Gpsk Cipher Suite
class EAP_GPSK_EXPORTS EapGpskCipherSuite
{
public:
   enum {
      CIPHER_SUITE_AES = 1;
      CIPHER_SUITE_HMAC = 2;
   };

   EapGpskCipherSuite(ACE_UINT32 vendor = 0) :
     vendor(vendor), cihperSuite(CIPHER_SUITE_AES) { }
   ACE_UINT32 &Vendor() { return vendor; }
   ACE_UINT16 &ChiperSuite() { return cipherSuite; }

   size_t KeySize() {
     switch(cipherSuite) {
       case CIPHER_SUITE_AES: return 16;
       case CIPHER_SUITE_HMAC: return 32;
       default:
          EAP_LOG(LM_ERROR, "Un-support cipher %d (1 or 2 is expected).\n",
            response->CSuiteSelected().ChiperSuite());
          throw -1;
          break;
     }
     return 0;
   }

   std::string toString() {
     char cps[6];
     *(ACE_UINT32*)cps = ACE_HTONL(response->CSuiteSelected().Vendor());
     *(ACE_UINT16*)(&cps[4]) = ACE_HTONS(response->CSuiteSelected().ChiperSuite());
     return std::string(cps, sizeof(cps));
   }

   void fromString(char *cps) {
     vendor() = ACE_NTOHL(*(ACE_UINT32*)cps);
     cps += 4;
     cipherSuite = ACE_NTOHS(*(ACE_UINT16*)cps);
   }

   void fromString(std::string &cps) {
     fromString(cps.data());
   }

   bool operator==(EapGpskCipherSuite &st) {
     return ((st.Vendor() == vendor) && (st.CipherSuite() == cipherSuite));
   }

   bool operator!=(EapGpskCipherSuite &st) {
     return !(st == *this);
   }

protected:
   ACE_UINT32 vendor;
   ACE_UINT16 cipherSuite;
};

/// EAP-Gpsk Cipher Suite List
class EAP_GPSK_EXPORTS EapGpskCipherSuiteList :
   public std::list<EapGpskCipherSuite>
{
public:
   std::list<EapGpskCipherSuite> &operator=(std::list<EapGpskCipherSuite> &from) {
      // clear the current content first
      while (! this->empty()) {
         this->pop_front();
      }

      // now copy new content
      std::list<EapGpskCipherSuite>::iterator i = from.begin();
      for (; i != from.end(); i++) {
          this->push_back(*i);
      }
   }

   bool operator==(std::list<EapGpskCipherSuite> &from) {
      if (this->length() != from.length()) {
         return false;
      }

      // MUST be equal in content and order
      std::list<EapGpskCipherSuite>::iterator i = from.begin();
      std::list<EapGpskCipherSuite>::iterator n = this->begin();
      for (; i != from.end(); i++, n++) {
         if (*i != *n) {
           return false;
         }
      }
      return true;
   }

   bool isPresent(EapGpskCipherSuite &csuite) {
      std::list<EapGpskCipherSuite>::iterator i = from.begin();
      for (; i != from.end(); i++) {
         if (*i == suite) {
           return true;
         }
      }
      return false;
   }
};

/// EAP-Gpsk/Request-Gpsk message.
class EAP_GPSK_EXPORTS EapGpskMsg: public EapRequest
{
public:
  EapGpskMsg(ACE_Byte opCode) :
    EapRequest(EapType(GPSK_METHOD_TYPE)), opCode(opCode) {}

  /// Use this function to obtain a reference to msgID.
  ACE_Byte& OpCode() { return opCode; }

  /// Enumerator 
  enum messageID {
    GPSK1 = 1,    /// Enum for Gpsk1
    GPSK2 = 2,    /// Enum for Gpsk2
    GPSK3 = 3,    /// Enum for Gpsk3
    GPSK4 = 4,    /// Enum for Gpsk4
    GPSKFail = 5,  /// Enum for Gpsk-Fail
    GPSKProtectedFail = 6,  /// Enum for Gpsk-Protected-Fail
  };

protected:
  /// Gpsk message id
  ACE_Byte opCode;
};

/// EAP-Request/Gpsk1 payload.
class EAP_GPSK_EXPORTS EapGpsk1: public EapGpskMsg
{
public:
  /// Initialized with a specific message id (1).
  EapGpsk1() : EapGpskMsg(GPSK1) {}

  /// Use this function to obtain a reference to ID_Server.
  std::string& IDServer() { return idServer; }

  /// Use this function to obtain a reference to RAND_Server.
  std::string& RANDServer() { return randServer; }

  /// Use this function to obtain a reference to CSuite_List.
  EapGpskCipherSuiteList& CSuiteList() { return csuiteList; }

protected:

  /// The ID_Server fo the EAP server.
  std::string idServer;

  /// 32-octet random number for RAND_Server.
  std::string randServer;

  /// The CSuite_List given by the EAP server.
  EapGpskCipherSuiteList csuiteList;
};

/// EAP-Response/Gpsk2 payload.
class EAP_GPSK_EXPORTS EapGpsk2: public EapGpskMsg
{
public:
  /// Initialized with a specific message id (2).
  EapGpsk2() : EapGpskMsg(GPSK2) {}

  /// Use this function to obtain a reference to peerID.
  std::string& IDPeer() { return idPeer; }

  /// Use this function to obtain a reference to ID_Server.
  std::string& IDServer() { return idServer; }

  /// Use this function to obtain a reference to RAND_Peer.
  std::string& RANDPeer() { return randPeer; }

  /// Use this function to obtain a reference to RAND_Server.
  std::string& RANDServer() { return randServer; }

  /// Use this function to obtain a reference to CSuite_List.
  EapGpskCipherSuiteList& CSuiteList() { return csuiteList; }

  /// Use this function to obtain a reference to csuiteSelected.
  EapGpskCipherSuite& CSuiteSelected() { return csuiteSelected; }

  /// Use this function to obtain a reference to csuiteSelected.
  std::string& PDPayload() { return pdPayload; }

  /// Use this function to obtain a reference to KS-octet payload MAC.
  std::string& MAC() { return mac; }

private:

  /// The ID_Peer fo the EAP server.
  std::string idPeer;

  /// The ID_Server fo the EAP server.
  std::string idServer;

  /// 32-octet random number for RAND_Peer.
  std::string randPeer;

  /// 32-octet random number for RAND_Server.
  std::string randServer;

  /// The CSuite_List given by the EAP server.
  EapGpskCipherSuiteList csuiteList;

  /// selected cipher suite.
  EapGpskCipherSuite csuiteSelected;

  /// The number of octets used in the PD_Payload_Block field.
  ACE_UINT16 pdPayloadLength;

  /// The PD_Payload_Block fo the EAP server.
  std::string pdPayload;

  /// KS-octet payload MAC.
  std::string mac;
};

/// EAP-Request/Gpsk3 payload.
class EAP_GPSK_EXPORTS EapGpsk3: public EapGpskMsg
{
public:
  /// Initialized with a specific message id (3).
  EapGpsk3() : EapGpskMsg(GPSK3) {}

  /// Use this function to obtain a reference to RAND_Peer.
  std::string& RANDPeer() { return randPeer; }

  /// Use this function to obtain a reference to RAND_Server.
  std::string& RANDServer() { return randServer; }

  /// Use this function to obtain a reference to ID_Server.
  std::string& IDServer() { return idServer; }

  /// Use this function to obtain a reference to csuiteSelected.
  EapGpskCipherSuite& CSuiteSelected() { return csuiteSelected; }

  /// Use this function to obtain a reference to csuiteSelected.
  std::string& PDPayload() { return pdPayload; }

  /// Use this function to obtain a reference to KS-octet payload MAC.
  std::string& MAC() { return mac; }

protected:

  /// 32-octet random number for RAND_Peer.
  std::string randPeer;

  /// 32-octet random number for RAND_Server.
  std::string randServer;

  /// The ID_Server fo the EAP server.
  std::string idServer;

  /// Selected cipher suite.
  EapGpskCipherSuite csuiteSelected;

  /// The PD_Payload_Block fo the EAP server.
  std::string pdPayload;

  /// KS-octet payload MAC.
  std::string mac;
};

/// EAP-Response/Gpsk4 payload.
class EAP_GPSK_EXPORTS EapGpsk4: public EapGpskMsg
{
public:
  /// Initialized with a specific message id (4).
  EapGpsk4() : EapGpskMsg(GPSK4) {}

  /// Use this function to obtain a reference to csuiteSelected.
  std::string& PDPayload() { return pdPayload; }

  /// Use this function to obtain a reference to KS-octet payload MAC.
  std::string& MAC() { return mac; }

  /// Use this function to obtain a reference to csuiteSelected.
  EapGpskCipherSuite& CSuiteSelected() { return csuiteSelected; }

private:

  /// The PD_Payload_Block fo the EAP server.
  std::string pdPayload;

  /// KS-octet payload MAC.
  std::string mac;

  /// Selected cipher suite.
  EapGpskCipherSuite csuiteSelected;
};

/// EAP-Request/Gpsk-Fail payload.
class EAP_GPSK_EXPORTS EapGpskFail: public EapGpskMsg
{
public:
  /// Initialized with a specific message id (5).
  EapGpskFail() : EapGpskMsg(GPSKFail) {}

  /// Use this function to obtain a reference to failure code.
  ACE_UINT32& FailureCode() { return failureCode; }

private:

  /// 32-octet random number for RAND_Peer.
  ACE_UINT32 failureCode;
};

/// EAP-Response/Gpsk-Fail payload.
typedef EapGpskFail EapResponseGpskFail;

/// EAP-Request/Gpsk-Protected-Fail payload.
class EAP_GPSK_EXPORTS EapRequestGpskProtectedFail: public EapGpskFail
{
public:
  /// Initialized with a specific message id (6).
  EapRequestGpsProtectedkFail() : EapGpskMsg(GPSKProtectedFail) {}

  /// Use this function to obtain a reference to KS-octet payload MAC.
  std::string& PayloadMAC() { return payloadMac; }

  /// Use this function to obtain a reference to csuiteSelected.
  EapGpskCipherSuite& CSuiteSelected() { return csuiteSelected; }

private:

  /// KS-octet payload MAC.
  std::string payloadMac;

  /// Selected cipher suite.
  EapGpskCipherSuite csuiteSelected;
};

/// EAP-Response/Gpsk-Protected-Fail payload.
typedef EapRequestGpskProtectedFail EapResponseGpskProtectedFail;

#endif // __EAP_GPSK_HXX__
