/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
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

/* 
   diameter_mip4_parser.hxx
   Parser Data Structure in Diameter MIP Application 
   Written by Miriam Tauil
   Created May 25, 2004.
*/

#ifndef __DIAMETER_MIP4_PARSER_H__
#define __DIAMETER_MIP4_PARSER_H__

#include <vector>
#include "diameter_parser.h"
/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(DIAMETER_MIP4_PARSER_EXPORT)
       #define DIAMETER_MIP4_PARSER_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_MIP4_PARSER_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_MIP4_PARSER_EXPORTS
   #define DIAMETER_MIP4_PARSER_EXPORTS
#endif


#define PRINT_MSG_CONTENT  // for debugging information

const diameter_unsigned32_t Mip4ApplicationId = 2;
const AAACommandCode MipAmrCommandCode = 260;
const AAACommandCode MipHarCommandCode = 262;


/// Definition for MIP-MN-AAA-Auth-Info AVP internal structure.
class mip_mn_aaa_auth_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (MipMnAaaSpi.IsSet())
      {
	c = cm.acquire("MIP-MN-AAA-SPI");
	MipMnAaaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (MipAuthInputDataLength.IsSet())
      {
	c = cm.acquire("MIP-Auth-Input-Data-Length");
	MipAuthInputDataLength.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (MipAuthenticatorLength.IsSet())
      {
	c = cm.acquire("MIP-Authenticator-Length");
	MipAuthenticatorLength.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
	if (MipAuthenticatorOffset.IsSet())
      {
	c = cm.acquire("MIP-Authenticator-Offset");
	MipAuthenticatorOffset.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("MIP-MN-AAA-SPI")))
      {
	MipMnAaaSpi.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Auth-Input-Data-Length")))
      {
	MipAuthInputDataLength.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Authenticator-Length")))
      {
	MipAuthenticatorLength.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Authenticator-Offset")))
      {
	MipAuthenticatorOffset.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> MipMnAaaSpi;
  DiameterScholarAttribute<diameter_unsigned32_t> MipAuthInputDataLength;
  DiameterScholarAttribute<diameter_unsigned32_t> MipAuthenticatorLength;
  DiameterScholarAttribute<diameter_unsigned32_t> MipAuthenticatorOffset;
};
/// Definition for MIP-Originating-Foreign-AAA-Info AVP internal structure.
class mip_originating_foreign_aaa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (OriginRealm.IsSet())
      {
	c = cm.acquire("Origin-Realm");
	OriginRealm.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (OriginHost.IsSet())
      {
	c = cm.acquire("Origin-Host");
	OriginHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Origin-Realm")))
      {
	OriginRealm.CopyFrom(*c);
      }
    if ((c = cl.search("Origin-Host")))
      {
	OriginHost.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> OriginRealm;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginHost;
   // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};
/// Definition for MIP-Home-Agent-Host-Info AVP internal structure.
class mip_home_agent_host_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (DestinationRealm.IsSet())
      {
	c = cm.acquire("Destination-Realm");
	DestinationRealm.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (DestinationHost.IsSet())
      {
	c = cm.acquire("Destination-Host");
	DestinationHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Destination-Realm")))
      {
	DestinationRealm.CopyFrom(*c);
      }
    if ((c = cl.search("Destination-Host")))
      {
	DestinationHost.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> DestinationRealm;
  DiameterScholarAttribute<diameter_unsigned32_t> DestinationHost;
   // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};
/// Definition for Proxy-Info AVP internal structure.
class proxyinfo_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (ProxyHost.IsSet())
      {
	c = cm.acquire("Proxy-Host");
	ProxyHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (ProxyState.IsSet())
      {
	c = cm.acquire("Proxy-State");
	ProxyState.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Proxy-Host")))
      {
	ProxyHost.CopyFrom(*c);
      }
    if ((c = cl.search("Proxy-State")))
      {
	ProxyState.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_identity_t> ProxyHost;
  DiameterScholarAttribute<diameter_octetstring_t> ProxyState;
  // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

/// Definition for AMR message contents internal structure.

//////  Definitions of classes for AMA  /////////
// Definition for MIP-MN-to-FA-MSA-Info AVP internal structure.
class mip_mn_to_fa_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipNonce.IsSet())
      {
	c = cm.acquire("MIP-Nonce");
	MipNonce.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
    if (MipMnAaaSpi.IsSet())
      {
	c = cm.acquire("MIP-MN-AAA-SPI");
	MipMnAaaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Nonce")))
      {
	MipNonce.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-MN-AAA-SPI")))
      {
	MipMnAaaSpi.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_octetstring_t> MipNonce;
  DiameterScholarAttribute<diameter_unsigned32_t> MipMnAaaSpi;
 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

// Definition for MIP-MN-to-HA-MSA-Info AVP internal structure.
class mip_mn_to_ha_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
   if (MipReplayMode.IsSet())
      {
	c = cm.acquire("MIP-Replay-Mode");
	MipReplayMode.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipNonce.IsSet())
      {
	c = cm.acquire("MIP-Nonce");
	MipNonce.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
    if (MipMnAaaSpi.IsSet())
      {
	c = cm.acquire("MIP-MN-AAA-SPI");
	MipMnAaaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
	if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
   if ((c = cl.search("MIP-Replay-Mode")))
      {
	MipReplayMode.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Nonce")))
      {
	MipNonce.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-MN-AAA-SPI")))
      {
	MipMnAaaSpi.CopyFrom(*c);
      }
   if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_enumerated_t> MipReplayMode;
  DiameterScholarAttribute<diameter_octetstring_t> MipNonce;
  DiameterScholarAttribute<diameter_unsigned32_t> MipMnAaaSpi;
 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

// Definition for MIP-FA-to-HA-MSA-Info AVP internal structure.
class mip_fa_to_ha_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipFaToHaSpi.IsSet())
      {
	c = cm.acquire("MIP-FA-to-HA-SPI");
	MipFaToHaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
	if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipSessionKey.IsSet())
      {
	c = cm.acquire("MIP-Session-Key");
	MipSessionKey.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }

    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-FA-to-HA-SPI")))
      {
	MipFaToHaSpi.CopyFrom(*c);
      }
	if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Session-Key")))
      {
	MipSessionKey.CopyFrom(*c);
      }
 
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> MipFaToHaSpi;
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_octetstring_t> MipSessionKey;

 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};
// Definition for MIP-FA-to-MN-MSA-Info AVP internal structure.
class mip_fa_to_mn_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipFaToMnSpi.IsSet())
      {
	c = cm.acquire("MIP-FA-to-MN-SPI");
	MipFaToMnSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
	if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipSessionKey.IsSet())
      {
	c = cm.acquire("MIP-Session-Key");
	MipSessionKey.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }

    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-FA-to-MN-SPI")))
      {
	MipFaToMnSpi.CopyFrom(*c);
      }
	if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Session-Key")))
      {
	MipSessionKey.CopyFrom(*c);
      }
 
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> MipFaToMnSpi;
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_octetstring_t> MipSessionKey;

 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};
// Definition for MIP-HA-to-MN-MSA-Info AVP internal structure.
class mip_ha_to_mn_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
   if (MipReplayMode.IsSet())
      {
	c = cm.acquire("MIP-Replay-Mode");
	MipReplayMode.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipSessionKey.IsSet())
      {
	c = cm.acquire("MIP-Session-Key");
	MipSessionKey.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
	if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
   if ((c = cl.search("MIP-Replay-Mode")))
      {
	MipReplayMode.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Session-Key")))
      {
	MipSessionKey.CopyFrom(*c);
      }
   if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_enumerated_t> MipReplayMode;
  DiameterScholarAttribute<diameter_octetstring_t> MipSessionKey;
 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

// Definition for MIP-HA-to-FA-MSA-Info AVP internal structure.
class mip_ha_to_fa_msa_info_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;

    if (MipHaToFaSpi.IsSet())
      {
	c = cm.acquire("MIP-HA-to-FA-SPI");
	MipHaToFaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
	if (MipAlgorithmType.IsSet())
      {
	c = cm.acquire("MIP-Algorithm-Type");
	MipAlgorithmType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (MipSessionKey.IsSet())
      {
	c = cm.acquire("MIP-Session-Key");
	MipSessionKey.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }

    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
 
    if ((c = cl.search("MIP-HA-to-FA-SPI")))
      {
	MipHaToFaSpi.CopyFrom(*c);
      }
	if ((c = cl.search("MIP-Algorithm-Type")))
      {
	MipAlgorithmType.CopyFrom(*c);
      }
    if ((c = cl.search("MIP-Session-Key")))
      {
	MipSessionKey.CopyFrom(*c);
      }
 
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> MipHaToFaSpi;
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_octetstring_t> MipSessionKey;

 // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

class MipFeatureVectorOperations {

public:
#define FV_MN_HOME_ADDR_REQUESTED      1
#define FV_HOME_ADDR_ALLOC_ONLY_IN_HOME_REALM  2
#define FV_MN_HA_REQUESTED          4
#define FV_FOREIGN_HA_AVAILABLE     8
#define FV_MN_HA_KEY_REQUESTED      16
#define FV_MN_FA_KEY_REQUESTED      32
#define FV_FA_HA_KEY_REQUESTED      64
#define FV_HA_IN_FOREIGN_NET       128
#define FV_CO_LOCATED_MN_BIT       256

  static void SetMipFeatureVectorBits( DiameterScholarAttribute<diameter_unsigned32_t> MipFeatureVector, int bitToSetOn ) 
  {

    if (bitToSetOn < 1 || bitToSetOn > 256)
    {
      AAA_LOG((LM_ERROR, "[%N] SetMipFeatureVectorBits: Wrong parameter.\n"));
      return;
    }
   
    diameter_unsigned32_t _mipFeatureVector =  MipFeatureVector();
  
    _mipFeatureVector |=  bitToSetOn;  
    MipFeatureVector.Set( _mipFeatureVector);
  }

  static bool IsFeatureVectorSet(DiameterScholarAttribute<diameter_unsigned32_t> MipFeatureVector, int bit )
  {
    diameter_unsigned32_t _mipFeatureVector =  MipFeatureVector();
    if ( _mipFeatureVector & bit )
      return true;
    else
      return false;
  }
};

class AMR_Data
{
 public:
  AMR_Data()
  {}

  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    UserName.Clear();
    DestinationRealm.Clear();
    OriginHost.Clear();
    OriginRealm.Clear();
   
    MipRegRequest.Clear();
    MipMnAaaAuth.Clear(); 
    AcctMultiSessionId.Clear();
    DestinationHost.Clear();
    OriginStateId.Clear();
    
    MipMobileNodeAddress.Clear();
    MipHomeAgentAddress.Clear();
    MipFeatureVector.Clear();
    MipOriginatingForeignAaa.Clear();

    AuthorizationLifetime.Clear();
    AuthSessionState.Clear();

    MipFaChallenge.Clear();
    MipCandidateHomeAgentHost.Clear();
    MipHomeAgentHost.Clear();
    MipHaToFaSpi.Clear();

    ProxyInfo.Clear();
    RouteRecord.Clear();
    Avp.Clear();
  }

  /// AMR AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_identity_t>  DestinationRealm;
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;

  DiameterScholarAttribute<diameter_octetstring_t> MipRegRequest;
  DiameterGroupedScholarAttribute<mip_mn_aaa_auth_info_t> MipMnAaaAuth;

  // optional AVPs
  DiameterScholarAttribute<diameter_utf8string_t> AcctMultiSessionId; 
  
  DiameterScholarAttribute<diameter_identity_t>  DestinationHost;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  
  DiameterScholarAttribute<diameter_address_t> MipMobileNodeAddress;
  DiameterScholarAttribute<diameter_address_t> MipHomeAgentAddress;
  DiameterScholarAttribute<diameter_unsigned32_t> MipFeatureVector;
  DiameterGroupedScholarAttribute<mip_originating_foreign_aaa_info_t> MipOriginatingForeignAaa;
  DiameterGroupedScholarAttribute<mip_home_agent_host_info_t> MipHomeAgentHost;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthorizationLifetime;

  
  /* In Section 8 of RFC3588:

  "An access device that does not expect to send a re-authorization or
  a session termination request to the server MAY include the Auth-
  Session-State AVP with the value set to NO_STATE_MAINTAINED as a
  hint to the server.  If the server accepts the hint, it agrees that
  since no session termination message will be received once service
  to the user is terminated, it cannot maintain state for the session.
  If the answer message from the server contains a different value in
  the Auth-Session-State AVP (or the default value if the AVP is
  absent), the access device MUST follow the server's directives.
  Note that the value NO_STATE_MAINTAINED MUST NOT be set in
  subsequent re- authorization requests and answers."

  In Section 8.1 of RFC3588:

  "There are four different authorization session state machines
  supported in the Diameter base protocol.  The first two describe a
  session in which the server is maintaining session state, indicated
  by the value of the Auth-Session-State AVP (or its absence).  One
  describes the session from a client perspective, the other from a
  server perspective.  The second two state machines are used when the
  server does not maintain session state.  Here again, one describes
  the session from a client perspective, the other from a server
  perspective."

  In Section 8.11 of RFC3588:

  "8.11.  Auth-Session-State AVP
                                                                                
  The Auth-Session-State AVP (AVP Code 277) is of type Enumerated and
  specifies whether state is maintained for a particular session.  The
  client MAY include this AVP in requests as a hint to the server, but
  the value in the server's answer message is binding.  The following
  values are supported:
                                                                                
  STATE_MAINTAINED              0
      This value is used to specify that session state is being
      maintained, and the access device MUST issue a session termination
      message when service to the user is terminated.  This is the
      default value.
                                                                                
  NO_STATE_MAINTAINED           1
      This value is used to specify that no session termination messages
      will be sent by the access device upon expiration of the
      Authorization-Lifetime."

    */
  DiameterScholarAttribute<diameter_enumerated_t> AuthSessionState;
  DiameterScholarAttribute<diameter_octetstring_t> MipFaChallenge;
  DiameterScholarAttribute<diameter_identity_t> MipCandidateHomeAgentHost;
  DiameterScholarAttribute<diameter_unsigned32_t> MipHaToFaSpi;

  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_identity_t> RouteRecord;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

/// Definition for AMA message contents internal structure.
class AMA_Data
{
 public:
    
  AMA_Data()   
  {
  }

  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    ResultCode.Clear();

    OriginHost.Clear();
    OriginRealm.Clear();

  // optional AVPs
    AcctMultiSessionId.Clear();
    UserName.Clear();
    AuthorizationLifetime.Clear();
    AuthSessionState.Clear();  
  
    ErrorReportingHost.Clear();
    ErrorMessage.Clear(); 
    ReAuthRequestType.Clear(); 

    MipFeatureVector.Clear();
    MipRegReply.Clear();
    MipMnToFaMsa.Clear();
    MipMnToHaMsa.Clear();
    MipFaToMnMsa.Clear();
    MipFaToHaMsa.Clear();
    MipHaToMnMsa.Clear();
    MipMsaLifetime.Clear();
    MipAlgorithmType.Clear();
    MipHomeAgentAddress.Clear();
    MipMobileNodeAddress.Clear();
    MipFilterRule.Clear();
    OriginStateId.Clear();

    OriginStateId.Clear();
    ProxyInfo.Clear();
    Avp.Clear();
  }

  /// AMA AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_unsigned32_t> ResultCode;

  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;

  // optional AVPs
  DiameterScholarAttribute<diameter_utf8string_t> AcctMultiSessionId; 
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthorizationLifetime;
  DiameterScholarAttribute<diameter_enumerated_t> AuthSessionState;  
  
  DiameterScholarAttribute<diameter_identity_t>  ErrorReportingHost;
  DiameterScholarAttribute<diameter_utf8string_t> ErrorMessage; 
  DiameterScholarAttribute<diameter_enumerated_t> ReAuthRequestType; 

  DiameterScholarAttribute<diameter_unsigned32_t> MipFeatureVector;
  DiameterScholarAttribute<diameter_octetstring_t> MipRegReply;
  DiameterGroupedScholarAttribute<mip_mn_to_fa_msa_info_t> MipMnToFaMsa;
  DiameterGroupedScholarAttribute<mip_mn_to_ha_msa_info_t> MipMnToHaMsa;
  DiameterGroupedScholarAttribute<mip_fa_to_mn_msa_info_t> MipFaToMnMsa;
  DiameterGroupedScholarAttribute<mip_fa_to_ha_msa_info_t> MipFaToHaMsa;
  DiameterGroupedScholarAttribute<mip_ha_to_mn_msa_info_t> MipHaToMnMsa;
  DiameterScholarAttribute<diameter_unsigned32_t> MipMsaLifetime;
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;
  DiameterScholarAttribute<diameter_address_t> MipHomeAgentAddress;
  DiameterScholarAttribute<diameter_address_t> MipMobileNodeAddress;
  DiameterVectorAttribute<diameter_ipfilter_rule_t> MipFilterRule;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

class HAR_Data
{
 public:
  HAR_Data()
  {
  }

  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    AuthorizationLifetime.Clear();
    AuthSessionState.Clear();
    MipRegRequest.Clear();
    OriginHost.Clear();
    OriginRealm.Clear();
    UserName.Clear();
    DestinationRealm.Clear();   
    MipFeatureVector.Clear();  
    DestinationHost.Clear();
    MipMnToHaMsa.Clear();
    MipMnToFaMsa.Clear();
    MipHaToMnMsa.Clear();
    MipHaToFaMsa.Clear();

    MipMsaLifetime.Clear();
    MipOriginatingForeignAaa.Clear();    
    MipMobileNodeAddress.Clear();
    MipHomeAgentAddress.Clear();
    MipAlgorithmType.Clear();

    MipFilterRule.Clear();
    OriginStateId.Clear();
    ProxyInfo.Clear();
    RouteRecord.Clear();
    Avp.Clear();
  }

  /// HAR AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthorizationLifetime;
  
 /*

  "8.11.  Auth-Session-State AVP
                                                                                
  The Auth-Session-State AVP (AVP Code 277) is of type Enumerated and
  specifies whether state is maintained for a particular session.  The
  client MAY include this AVP in requests as a hint to the server, but
  the value in the server's answer message is binding.  The following
  values are supported:
                                                                                
  STATE_MAINTAINED              0
      This value is used to specify that session state is being
      maintained, and the access device MUST issue a session termination
      message when service to the user is terminated.  This is the
      default value.
                                                                                
  NO_STATE_MAINTAINED           1
      This value is used to specify that no session termination messages
      will be sent by the access device upon expiration of the
      Authorization-Lifetime."

    */
  DiameterScholarAttribute<diameter_enumerated_t> AuthSessionState;
  DiameterScholarAttribute<diameter_octetstring_t> MipRegRequest; 
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;  
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_identity_t>  DestinationRealm;
  DiameterScholarAttribute<diameter_unsigned32_t> MipFeatureVector;

  // optional AVPs 
  
  DiameterScholarAttribute<diameter_identity_t>  DestinationHost;
  DiameterGroupedScholarAttribute<mip_mn_to_ha_msa_info_t> MipMnToHaMsa;  
  DiameterGroupedScholarAttribute<mip_mn_to_fa_msa_info_t> MipMnToFaMsa;
  DiameterGroupedScholarAttribute<mip_ha_to_mn_msa_info_t> MipHaToMnMsa;
  DiameterGroupedScholarAttribute<mip_ha_to_fa_msa_info_t> MipHaToFaMsa;

  DiameterScholarAttribute<diameter_unsigned32_t> MipMsaLifetime;
  DiameterGroupedScholarAttribute<mip_originating_foreign_aaa_info_t> MipOriginatingForeignAaa;
  DiameterScholarAttribute<diameter_address_t> MipMobileNodeAddress;
  DiameterScholarAttribute<diameter_address_t> MipHomeAgentAddress;
  DiameterScholarAttribute<diameter_enumerated_t> MipAlgorithmType;


  DiameterVectorAttribute<diameter_ipfilter_rule_t> MipFilterRule;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_identity_t> RouteRecord;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};



/// Definition for HAA message contents internal structure.
class HAA_Data
{
 public:
	   
  HAA_Data()   
  {
  }

  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    ResultCode.Clear();
    OriginHost.Clear();
    OriginRealm.Clear();
    UserName.Clear();
    AcctMultiSessionId.Clear(); 
    ErrorReportingHost.Clear(); 
    ErrorMessage.Clear(); 
    MipRegReply.Clear(); 
    MipHomeAgentAddress.Clear(); 
    MipMobileNodeAddress.Clear(); 
    MipFaToHaSpi.Clear(); 
    MipFaToMnSpi.Clear(); 

    OriginStateId.Clear();
    ProxyInfo.Clear();
    Avp.Clear();
  }

  /// HAA AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_unsigned32_t> ResultCode;

  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;

  // optional AVPs
  DiameterScholarAttribute<diameter_utf8string_t> AcctMultiSessionId; 
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  
  DiameterScholarAttribute<diameter_identity_t>  ErrorReportingHost;
  DiameterScholarAttribute<diameter_utf8string_t> ErrorMessage; 
  DiameterScholarAttribute<diameter_octetstring_t> MipRegReply;

  DiameterScholarAttribute<diameter_address_t> MipHomeAgentAddress;
  DiameterScholarAttribute<diameter_address_t> MipMobileNodeAddress;
  DiameterScholarAttribute<diameter_unsigned32_t> MipFaToHaSpi;
  DiameterScholarAttribute<diameter_unsigned32_t> MipFaToMnSpi;

  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

typedef AAAParser<DiameterMsg*, AMR_Data*> AMR_Parser;
typedef AAAParser<DiameterMsg*, AMA_Data*> AMA_Parser;

typedef AAAParser<DiameterMsg*, HAR_Data*> HAR_Parser;
typedef AAAParser<DiameterMsg*, HAA_Data*> HAA_Parser;

template<> void AMR_Parser::parseRawToApp();
template<> void AMR_Parser::parseAppToRaw();

template<> void AMA_Parser::parseRawToApp();
template<> void AMA_Parser::parseAppToRaw();

template<> void HAR_Parser::parseRawToApp();
template<> void HAR_Parser::parseAppToRaw();

template<> void HAA_Parser::parseRawToApp();
template<> void HAA_Parser::parseAppToRaw();

#endif //__DIAMETER_MIP4_PARSER_H__
