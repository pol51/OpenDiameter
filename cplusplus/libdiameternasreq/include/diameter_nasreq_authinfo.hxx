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

/* $Id: */
/* 
   diameter_nasreq_authinfo.hxx
   Authentication information definition for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created April 28, 2004.
*/

#ifndef __NASREQ_AUTHINFO_H__
#define __NASREQ_AUTHINFO_H__

#include <openssl/md5.h>
#include "framework.h"
#include "diameter_nasreq_parser.hxx"


enum DiameterNasreqAuthenticationType {
  NASREQ_AUTHENTICATION_TYPE_NONE,
  NASREQ_AUTHENTICATION_TYPE_PAP,
  NASREQ_AUTHENTICATION_TYPE_CHAP,
  NASREQ_AUTHENTICATION_TYPE_ARAP,
}; 

/// The base class for Diameter NASREQ authentication information.
class DiameterNasreqAuthenticationInfo
{
public:
  DiameterNasreqAuthenticationInfo
  (DiameterNasreqAuthenticationType t=NASREQ_AUTHENTICATION_TYPE_NONE)
    : authenticationType(t), prompt(false)
  {}

  DiameterNasreqAuthenticationInfo
  (diameter_utf8string_t& username,
   DiameterNasreqAuthenticationType t=NASREQ_AUTHENTICATION_TYPE_NONE)
    : authenticationType(t), userName(username), prompt(false)
  {}

  DiameterNasreqAuthenticationType& AuthenticationType() 
  { return authenticationType; }
  // Used for setting/getting username.
  diameter_utf8string_t& UserName() { return userName; }

private:
  DiameterNasreqAuthenticationType authenticationType;
  diameter_utf8string_t userName;
  bool prompt;
};

/// The class for storing PAP information.
class PAP_Info : public DiameterNasreqAuthenticationInfo
{
public:
  PAP_Info(diameter_utf8string_t& username, diameter_utf8string_t& password) : 
    DiameterNasreqAuthenticationInfo(username, NASREQ_AUTHENTICATION_TYPE_PAP),
    userPassword(password)
  {}
  PAP_Info(diameter_utf8string_t& password) : 
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_PAP),
    userPassword(password)
  {}
  PAP_Info() : 
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_PAP)
  {}
  // Used for setting/getting user password.
  diameter_utf8string_t& UserPassword() { return userPassword; }

  bool Validate(diameter_utf8string_t& password)
  {
    if (password == userPassword)
      return true;
    return false;
  }

private:
  diameter_utf8string_t userPassword;
};

/// The class for storing CHAP information.
class CHAP_Info : public DiameterNasreqAuthenticationInfo
{
public:
  /// Constructor.
  CHAP_Info(diameter_utf8string_t& username,
	    chap_auth_t auth, diameter_octetstring_t& challenge) : 
    DiameterNasreqAuthenticationInfo(username, 
				     NASREQ_AUTHENTICATION_TYPE_CHAP),
    chapAuth(auth), chapChallenge(challenge)
  {}

  /// Constructor.
  CHAP_Info(chap_auth_t auth, diameter_octetstring_t& challenge) : 
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_CHAP),
    chapAuth(auth), chapChallenge(challenge)
  {}

  /// Constructor.
  CHAP_Info() : 
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_CHAP)
  {}

  /// Validator.
  bool Validate(diameter_octetstring_t& secret)
  {
    if (!chapAuth.ChapAlgorithm.IsSet())
      {
	AAA_LOG((LM_ERROR, "%N: missing CHAP algorithm.\n."));
	return false;
      }
    if (chapAuth.ChapAlgorithm() != CHAP_ALGORITHM_MD5)
      {
	AAA_LOG((LM_ERROR, "%N: invalid CHAP algorithm\n."));
	return false;
      }
    if (!chapAuth.ChapResponse.IsSet())
      {
	AAA_LOG((LM_ERROR, "%N: missing CHAP response\n."));
	return false;
      }
    if (!chapAuth.ChapIdent.IsSet())
      {
	AAA_LOG((LM_ERROR, "%N: missing CHAP identifier\n."));
	return false;
      }

    // Compute MD5.

    /* RFC 1994 (PPP CHAP):
      The Response Value is the one-way hash calculated over a stream of
      octets consisting of the Identifier, followed by (concatenated
      with) the "secret", followed by (concatenated with) the Challenge
      Value.  The length of the Response Value depends upon the hash
      algorithm used (16 octets for MD5).
    */

    // Initialize the result.
    std::string md5Result(MD5_DIGEST_LENGTH, '\0');

    // Do MD5.
    std::string rawResponse(chapAuth.ChapIdent());
    rawResponse.append((std::string&)secret);
    rawResponse.append((std::string&)chapChallenge);
    MD5((const unsigned char*)rawResponse.data(), 
	(unsigned)rawResponse.size(), (unsigned char*)md5Result.data());
    if (md5Result != chapAuth.ChapResponse())
      {
	AAA_LOG((LM_ERROR, "%N: validation failed\n."));
	return false;
      }
    return true;
  }

  /// Used for setting/getting chap auth.
  chap_auth_t& ChapAuth() { return chapAuth; }

  /// Used for setting/getting chap challenge.
  diameter_octetstring_t& ChapChallenge() { return chapChallenge; }

private:
  chap_auth_t chapAuth;
  diameter_octetstring_t chapChallenge;
};

/// The class for storing ARAP information.
class ARAP_Info : public DiameterNasreqAuthenticationInfo
{
public:
  ARAP_Info(diameter_utf8string_t& username,
	    diameter_octetstring_t& password,
	    diameter_octetstring_t& challengeResponse,
	    diameter_unsigned32_t retry=0) : 
    DiameterNasreqAuthenticationInfo(username, NASREQ_AUTHENTICATION_TYPE_ARAP),
    arapPassword(password), arapChallengeResponse(challengeResponse),
    passwordRetry(retry),
    isFirst(true)
  {}

  ARAP_Info(diameter_octetstring_t password,
	    diameter_octetstring_t challengeResponse,
	    diameter_unsigned32_t retry=0) : 
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_ARAP),
    arapPassword(password), arapChallengeResponse(challengeResponse),
    passwordRetry(retry),
    isFirst(true)
  {}

  ARAP_Info() :
    DiameterNasreqAuthenticationInfo(NASREQ_AUTHENTICATION_TYPE_ARAP),
    isFirst(true)
  {}

  /// Used for setting/getting ARAP Password.
  diameter_octetstring_t& ArapPassword() { return arapPassword; }

  /// Used for setting/getting ARAP ChallengeResponse.
  diameter_octetstring_t& ArapChallengeResponse() 
  { return arapChallengeResponse; }

  /// Used for setting/getting ARAP Security.
  diameter_unsigned32_t& ArapSecurity() { return arapSecurity; }

  /// Used for setting/getting ARAP SecurityData.
  std::vector<diameter_octetstring_t>& ArapSecurityData() 
  { return arapSecurityData; }

  /// Used for setting/getting Password Retry (which is a part of ARAP).
  diameter_unsigned32_t& PasswordRetry() { return passwordRetry; }

  /// Used for setting/getting isFirst value which indicates whether
  /// the ARAP authentication is in its first round or not.
  bool& IsFirst() { return isFirst; }

private:
  diameter_octetstring_t arapPassword;
  diameter_octetstring_t arapChallengeResponse;
  diameter_unsigned32_t arapSecurity;
  std::vector<diameter_octetstring_t> arapSecurityData;
  diameter_unsigned32_t passwordRetry;
  bool isFirst;
};

#endif
