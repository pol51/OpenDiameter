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

/// EAP-Gpsk Cipher Suite
class EAP_GPSK_EXPORTS EapGpskCipherSuite
{
public:
   EapGpskCipherSuite(ACE_UINT32 vendor = 0) :
     vendor(vendor), cihperSuite(1) { }
   ACE_UINT32 &Vendor() { return vendor; }
   ACE_UINT16 &ChiperSuite() { return chiperSuite; }

protected:
   ACE_UINT32 vendor;
   ACE_UINT16 chiperSuite;
};

/// EAP-Gpsk/Request-Gpsk message.
class EAP_GPSK_EXPORTS EapRequestGpsk: public EapRequest
{
public:
  EapRequestGpsk(ACE_Byte opCode) : 
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

/// EAP-Gpsk/Response-Gpsk message.
class EAP_GPSK_EXPORTS EapResponseGpsk: public EapRequestGpsk 
{
public:
  EapResponseGpsk(ACE_Byte msgID) : EapRequestGpsk(msgID) {}
};

/// EAP-Request/Gpsk1 payload.
class EAP_GPSK_EXPORTS EapRequestGpsk1: public EapRequestGpsk
{
public:
  /// Initialized with a specific message id (1).
  EapRequestGpskRequest() : EapRequestGpsk(1) {}

  /// Use this function to obtain a reference to ID_Server.
  std::string& IDServer() { return idServer; }

  /// Use this function to obtain a reference to RAND_Server.
  std::string& RANDServer() { return randServer; }

  /// Use this function to obtain a reference to CSuite_List.
  std::list<EapGpskCipherSuite>& CSuiteList() { return csuiteList; }

protected:

  /// The ID_Server fo the EAP server.
  std::string idServer;

  /// 32-octet random number for RAND_Server.
  std::string randServer;

  /// The CSuite_List given by the EAP server.
  std::list<EapGpskCipherSuite> csuiteList;
};

/// EAP-Response/Gpsk2 payload.
class EAP_GPSK_EXPORTS EapResponseGpsk2: public EapResponseGpsk
{
public:
  /// Initialized with a specific message id (2).
  EapResponseGpskResponse() : EapResponseGpsk(2) {}

  /// Use this function to obtain a reference to peerID.
  std::string& IDPeer() { return idPeer; }

  /// Use this function to obtain a reference to ID_Server.
  std::string& IDServer() { return idServer; }

  /// Use this function to obtain a reference to RAND_Peer.
  std::string& RANDPeer() { return randPeer; }

  /// Use this function to obtain a reference to RAND_Server.
  std::string& RANDServer() { return randServer; }

  /// Use this function to obtain a reference to CSuite_List.
  std::list<EapGpskCipherSuite>& CSuiteList() { return csuiteList; }

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
  std::list<EapGpskCipherSuite> csuiteList;

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
class EAP_GPSK_EXPORTS EapRequestGpsk3: public EapRequestGpsk
{
public:
  /// Initialized with a specific message id (3).
  EapRequestGpskRequest() : EapRequestGpsk(3) {}

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
  std::string& PayloadMAC() { return payloadMac; }

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
  std::string payloadMac;
};

/// EAP-Response/Gpsk4 payload.
class EAP_GPSK_EXPORTS EapResponseGpsk4: public EapResponseGpsk
{
public:
  /// Initialized with a specific message id (4).
  EapResponseGpskResponse() : EapResponseGpsk(4) {}

  /// Use this function to obtain a reference to csuiteSelected.
  std::string& PDPayload() { return pdPayload; }

  /// Use this function to obtain a reference to KS-octet payload MAC.
  std::string& PayloadMAC() { return payloadMac; }

  /// Use this function to obtain a reference to csuiteSelected.
  EapGpskCipherSuite& CSuiteSelected() { return csuiteSelected; }

private:

  /// The PD_Payload_Block fo the EAP server.
  std::string pdPayload;

  /// KS-octet payload MAC.
  std::string payloadMac;

  /// Selected cipher suite.
  EapGpskCipherSuite csuiteSelected;
};

/// EAP-Request/Gpsk-Fail payload.
class EAP_GPSK_EXPORTS EapRequestGpskFail: public EapRequestGpsk
{
public:
  /// Initialized with a specific message id (5).
  EapRequestGpskFail() : EapRequestGpsk(5) {}

  /// Use this function to obtain a reference to failure code.
  ACE_UINT32& FailureCode() { return failureCode; }

private:

  /// 32-octet random number for RAND_Peer.
  ACE_UINT32 failureCode;
};

/// EAP-Response/Gpsk-Fail payload.
typedef EapRequestGpskFail EapResponseGpskFail;

/// EAP-Request/Gpsk-Protected-Fail payload.
class EAP_GPSK_EXPORTS EapRequestGpskProtectedFail: public EapRequestGpskFail
{
public:
  /// Initialized with a specific message id (6).
  EapRequestGpsProtectedkFail() : EapRequestGpsk(6) {}

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
