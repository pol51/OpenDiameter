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
/* $Id: parser_avpvalue.h,v 1.32 2006/04/21 19:15:39 vfajardo Exp $ */

#ifndef __PARSER_AVPVALUE_H__
#define __PARSER_AVPVALUE_H__

// #define BOOST_SPIRIT_THREADSAFE

#include <ace/Synch.h>
#include <ace/Singleton.h>
#include <ace/CDR_Base.h>
#include "resultcodes.h"
#include "diameter_parser.h"
#include "aaa_parser_avp.h"
#include "aaa_parser_q_avplist.h"
#include "aaa_q_avplist.h"
#include "aaa_g_avplist.h"
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/loops.hpp>
#include <boost/spirit/dynamic/for.hpp>
#include <boost/pool/detail/mutex.hpp>
using namespace boost::spirit;

/// Container entry type definitions.
typedef AAATypeSpecificAvpContainerEntry<diameter_integer32_t>
    DiameterInteger32AvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_unsigned32_t>
    DiameterUnsigned32AvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_integer64_t>
    DiameterInteger64AvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_unsigned64_t>
    DiameterUnsigned64AvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_time_t>
    DiameterTimeAvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<AAAAvpContainerList>
    DiameterGroupedAvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<std::string>
    DiameterStringAvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_uri_t>
    DiameterURIAvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_ipfilter_rule_t>
    DiameterIPFilterRuleAvpContainerEntry;
typedef AAATypeSpecificAvpContainerEntry<diameter_address_t>
    DiameterAddressAvpContainerEntry;

#ifndef BOOST_SPIRIT_THREADSAFE
extern ACE_Mutex AvpGrammarMutex_S;
#endif

class AnyParser : public DiameterAvpValueParser
{
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_DATA_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      e->dataRef().assign(aBuffer->base(), aBuffer->size());
    }
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      std::string& str = e->dataRef();
      if (e->dataType() != AAA_AVP_DATA_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      if ((aBuffer->size() + (size_t)aBuffer->base() - 
           (size_t)aBuffer->wr_ptr()) < str.length())
	/* assuming 32-bit boundary */
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Not enough buffer"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                 AAA_OUT_OF_SPACE);
	  throw st;
	}
      aBuffer->copy(str.data(), str.length());
    }
};

class Integer32Parser : public DiameterAvpValueParser
{
  friend class ACE_Singleton<Integer32Parser, ACE_Recursive_Thread_Mutex>;
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterInteger32AvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_INTEGER32_TYPE && 
	  e->dataType() != AAA_AVP_UINTEGER32_TYPE &&
	  e->dataType() != AAA_AVP_ENUM_TYPE &&
          e->dataType() != AAA_AVP_TIME_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      e->dataRef() = ntohl(*((diameter_integer32_t*)(aBuffer->base())));
    }
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer;
      DiameterInteger32AvpContainerEntry *e;
      getRawData(aBuffer);
      getAppData(e);
      if (e->dataType() != AAA_AVP_INTEGER32_TYPE && 
	  e->dataType() != AAA_AVP_UINTEGER32_TYPE &&
	  e->dataType() != AAA_AVP_ENUM_TYPE &&
          e->dataType() != AAA_AVP_TIME_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      if ((aBuffer->size() + (size_t)aBuffer->base() - 
           (size_t)aBuffer->wr_ptr()) < sizeof(diameter_integer32_t))
	/* assuming 32-bit boundary */
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Not enough buffer"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                 AAA_OUT_OF_SPACE);
	  throw st;
	}
      *((diameter_integer32_t*)(aBuffer->wr_ptr())) = ntohl(e->dataRef());
      aBuffer->wr_ptr(sizeof(diameter_integer32_t));
    }
};


class Integer64Parser : public DiameterAvpValueParser
{
  friend class ACE_Singleton<Integer64Parser, ACE_Recursive_Thread_Mutex>;
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterInteger64AvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_INTEGER64_TYPE &&
	  e->dataType() != AAA_AVP_UINTEGER64_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      e->dataRef() = AAA_NTOH_64(*((diameter_integer64_t*)(aBuffer->base())));
    }
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer;
      DiameterInteger64AvpContainerEntry *e;
      getRawData(aBuffer);
      getAppData(e);
      if (e->dataType() != AAA_AVP_INTEGER64_TYPE &&
	  e->dataType() != AAA_AVP_UINTEGER64_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      if ((aBuffer->size() + (size_t)aBuffer->base() - 
           (size_t)aBuffer->wr_ptr()) < sizeof(diameter_integer64_t))
	/* assuming 32-bit boundary */
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Not enough buffer"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                 AAA_OUT_OF_SPACE);
	  throw st;
	}
      *((diameter_integer64_t*)(aBuffer->wr_ptr())) = AAA_HTON_64(e->dataRef());
      aBuffer->wr_ptr(sizeof(diameter_integer64_t));
    }
};

class OctetstringParser : public DiameterAvpValueParser
{
  friend class ACE_Singleton<OctetstringParser, ACE_Recursive_Thread_Mutex>;
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_STRING_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      std::string &str = e->dataRef();
      str.assign(aBuffer->base(), aBuffer->size());
    }
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_STRING_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      std::string &str = e->dataRef();

      DiameterErrorCode st;
      if (aBuffer->size() + (size_t)aBuffer->base() - 
          (size_t)aBuffer->wr_ptr() < str.length())
	{
	  AAA_LOG((LM_ERROR, "Not enough buffer\n"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                 AAA_OUT_OF_SPACE);
	  throw st;
	}
      aBuffer->copy(str.data(), str.length());
    }
};


/* UTF-8 definition in RFC 2279.  
  
 2.  UTF-8 definition

   In UTF-8, characters are encoded using sequences of 1 to 6 octets.
   The only octet of a "sequence" of one has the higher-order bit set to
   0, the remaining 7 bits being used to encode the character value. In
   a sequence of n octets, n>1, the initial octet has the n higher-order
   bits set to 1, followed by a bit set to 0.  The remaining bit(s) of
   that octet contain bits from the value of the character to be
   encoded.  The following octet(s) all have the higher-order bit set to
   1 and the following bit set to 0, leaving 6 bits in each to contain
   bits from the character to be encoded.

   The table below summarizes the format of these different octet types.
   The letter x indicates bits available for encoding bits of the UCS-4
   character value.

   UCS-4 range (hex.)           UTF-8 octet sequence (binary)
   0000 0000-0000 007F   0xxxxxxx
   0000 0080-0000 07FF   110xxxxx 10xxxxxx
   0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
   0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
   0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx

   Encoding from UCS-4 to UTF-8 proceeds as follows:

   1) Determine the number of octets required from the character value
      and the first column of the table above.  It is important to note
      that the rows of the table are mutually exclusive, i.e. there is
      only one valid way to encode a given UCS-4 character.

   2) Prepare the high-order bits of the octets as per the second column
      of the table.

   3) Fill in the bits marked x from the bits of the character value,
      starting from the lower-order bits of the character value and
      putting them first in the last octet of the sequence, then the
      next to last, etc. until all x bits are filled in.

      The algorithm for encoding UCS-2 (or Unicode) to UTF-8 can be
      obtained from the above, in principle, by simply extending each
      UCS-2 character with two zero-valued octets.  However, pairs of
      UCS-2 values between D800 and DFFF (surrogate pairs in Unicode
      parlance), being actually UCS-4 characters transformed through
      UTF-16, need special treatment: the UTF-16 transformation must be
      undone, yielding a UCS-4 character that is then transformed as
      above.

      Decoding from UTF-8 to UCS-4 proceeds as follows:

   1) Initialize the 4 octets of the UCS-4 character with all bits set
      to 0.

   2) Determine which bits encode the character value from the number of
      octets in the sequence and the second column of the table above
      (the bits marked x).

   3) Distribute the bits from the sequence to the UCS-4 character,
      first the lower-order bits from the last octet of the sequence and
      proceeding to the left until no x bits are left.

      If the UTF-8 sequence is no more than three octets long, decoding
      can proceed directly to UCS-2.

        NOTE -- actual implementations of the decoding algorithm above
        should protect against decoding invalid sequences.  For
        instance, a naive implementation may (wrongly) decode the
        invalid UTF-8 sequence C0 80 into the character U+0000, which
        may have security consequences and/or cause other problems.  See
        the Security Considerations section below.

   A more detailed algorithm and formulae can be found in [FSS_UTF],
   [UNICODE] or Annex R to [ISO-10646].

*/

class Utf8stringParser : public OctetstringParser
{
 public:

  void parseRawToApp() throw(DiameterErrorCode)
    {
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      AAADictionaryEntry *avp = getDictData();
      if (e->dataType() != AAA_AVP_UTF8_STRING_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      std::string &str = e->dataRef();

      DiameterErrorCode st;
      e->dataType() = AAA_AVP_STRING_TYPE;
      try {
	OctetstringParser::parseRawToApp();
      }
      catch (DiameterErrorCode &st)	{
	e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
	throw st;
      }
      e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
      UTF8Checker check;
      if (check(str.data(), str.size()) != 0)
        {
            AAA_LOG((LM_ERROR, "Invalid UTF8 string"));
            st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
            throw st;
        }
    }

  void parseAppToRaw() throw(DiameterErrorCode)
    {
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_UTF8_STRING_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      std::string &str = e->dataRef();

      DiameterErrorCode st;
      UTF8Checker check;
      if (check(str.data(), str.size()) != 0)
	{
	  AAA_LOG((LM_ERROR, "Invalid UTF8 string"));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG, 
                 AAA_PARSE_ERROR_INVALID_CONTAINER_CONTENTS);
	  throw st;
	}
      e->dataType() = AAA_AVP_STRING_TYPE;
      try {
	OctetstringParser::parseAppToRaw();
      }
      catch (DiameterErrorCode &st)	{
	e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
	throw st;
      }
      e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
    }
};

class AddressParser : public OctetstringParser
{
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterAddressAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_ADDRESS_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG, 
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_STRING_TYPE);
      setAppData(e2);    // Replace the original container entry with
			 // the new address container entry.

      DiameterErrorCode st;
      try {
	OctetstringParser::parseRawToApp();
      }
      catch (DiameterErrorCode &st)
	{
	  setAppData(e); // Recover the original container entry.
	  em.release(e2);// Release the UTF8 string container entry.
	  throw st;
	}
      setAppData(e);     // Recover the original container entry.

      std::string& str = e2->dataRef(Type2Type<diameter_octetstring_t>());
      diameter_address_t &address = e->dataRef();

      address.type = ntohs(*((ACE_UINT16*)(str.data())));
      address.value.assign(str, 2, str.length() - 2);

      em.release(e2);    // Release the UTF8 string container entry.
    };
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterAddressAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_ADDRESS_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_STRING_TYPE);
      diameter_address_t& address = e->dataRef();
      std::string& str = e2->dataRef(Type2Type<diameter_octetstring_t>());

      ACE_UINT16 tmp = ntohs(address.type);
      char* c = (char*)&tmp;

      str.append(c, 2);
      str.append(address.value);

      setAppData(e2);

      try {
	OctetstringParser::parseAppToRaw();
      }
      catch (DiameterErrorCode &st)
	{
	  AAA_LOG((LM_ERROR, "error\n"));
	  setAppData(e);
	  em.release(e2);
	  throw;
	}
      setAppData(e);
      em.release(e2);
    }
};

struct DiamidentGrammar : public grammar<DiamidentGrammar>
{
  template <typename ScannerT>
  struct definition
  {
    definition(DiamidentGrammar const& self)  
    { 
      diameterIdentity = realm;
      realm = label >> *('.' >> label);  // No recursive rule allowed.
      label = diameterName | diameterDname;
      diameterName = alpha_p >> *(alnum_p | '-');
      diameterDname = digit_p >> +(alnum_p | '-');
    }
    rule<ScannerT> diameterIdentity, realm, label, diameterName, diameterDname;
    rule<ScannerT> const& start() const { return diameterIdentity; }
  };
};

class DiamidentParser : public Utf8stringParser
{
 public:

  void parseRawToApp() throw(DiameterErrorCode)
    {
      DiameterStringAvpContainerEntry *e;
      DiameterErrorCode st;
      AAADictionaryEntry *avp = getDictData();
      getAppData(e);
      if (e->dataType() != AAA_AVP_DIAMID_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
      try {
	Utf8stringParser::parseRawToApp();
      }
      catch (DiameterErrorCode &st)	{
	e->dataType() = AAA_AVP_DIAMID_TYPE;
	throw st;
      }
      e->dataType() = AAA_AVP_DIAMID_TYPE;

      std::string& str = e->dataRef();
#ifndef BOOST_SPIRIT_THREADSAFE
      AAA_MutexScopeLock lock(AvpGrammarMutex_S);
#endif
      struct DiamidentGrammar grammar;
      if (!parse(str.begin(), str.end(), grammar, space_p).full)
	{
	  AAA_LOG((LM_ERROR, "Diamident parse failure."));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
	  throw st;
	}
    };
  
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      DiameterStringAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_DIAMID_TYPE)
      {
         DiameterErrorCode st;
         AAA_LOG((LM_ERROR, "Invalid AVP type."));
         st.set(AAA_PARSE_ERROR_TYPE_BUG,
                AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
         throw st;
      }

      DiameterErrorCode st;
      AAADictionaryEntry *avp = getDictData();

      std::string& str = e->dataRef();
      do {
#ifndef BOOST_SPIRIT_THREADSAFE
         AAA_MutexScopeLock lock(AvpGrammarMutex_S);
#endif
         struct DiamidentGrammar grammar;
         if (!parse(str.begin(), str.end(), grammar, space_p).full)
         {
            AAA_LOG((LM_ERROR, "Diamident parse failure."));
            st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
            throw st;
         }
      } while (0);
      e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
      try {
         Utf8stringParser::parseAppToRaw();
      }
      catch (DiameterErrorCode &st)	{
         e->dataType() = AAA_AVP_DIAMID_TYPE;
         throw st;
      }
	   e->dataType() = AAA_AVP_DIAMID_TYPE;
    }
};

struct SchemeFunctor
{
  SchemeFunctor(diameter_uri_t &uri_) : uri(uri_) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    std::string str(first,last);
    if (str == "aaa")
      uri.scheme=DIAMETER_SCHEME_AAA;
    else
      uri.scheme=DIAMETER_SCHEME_AAAS;
  }
  diameter_uri_t& uri;
};

struct TransportFunctor
{
  TransportFunctor(diameter_uri_t &uri_) : uri(uri_) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    std::string str = std::string(first,last);
    if (str == "tcp")
      uri.transport=DIAMETER_TRANSPORT_PROTO_TCP;
    else if (str == "udp")
      uri.transport=DIAMETER_TRANSPORT_PROTO_UDP;
    else if (str == "sctp")
      uri.transport=DIAMETER_TRANSPORT_PROTO_SCTP;
  }
  diameter_uri_t& uri;
};

struct ProtocolFunctor
{
  ProtocolFunctor(diameter_uri_t &uri_) : uri(uri_) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    std::string str = std::string(first,last);
    if (str == "diameter")
      uri.transport=DIAMETER_PROTO_DIAMETER;
    else if (str == "radius")
      uri.transport=DIAMETER_PROTO_RADIUS;
    else if (str == "tacacs+")
      uri.transport=DIAMETER_PROTO_TACACSPLUS;
  }
  diameter_uri_t& uri;
};

struct FqdnFunctor
{
  FqdnFunctor(diameter_uri_t &uri_) : uri(uri_) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    uri.fqdn.assign(first, last);
  }
  diameter_uri_t& uri;
};

/* 
   Adopted a bug fix sent by Mario Stefanec (mstefanec at
   users.sourceforge.net) for as a resolution to bug 858312.
*/
struct DiamuriGrammar : public grammar<DiamuriGrammar>
{
  public:
  DiamuriGrammar(diameter_uri_t &uri_) : uri(uri_) {}
  template <typename ScannerT>
  struct definition
  {
    definition(DiamuriGrammar const& self)  
    { 
      diameterUri = 
	(str_p("aaa") | str_p("aaas"))[SchemeFunctor(self.uri)] 
	>> str_p("://")
	>> fqdn >> !port >> !transport >> !protocol;
      transport = (str_p(";transport=")
		   >> ((str_p("tcp") | str_p("sctp") | str_p("udp"))
		       [TransportFunctor(self.uri)]));
      protocol = ((str_p(";protocol=")
		   >> (str_p("diameter") | str_p("radius") | str_p("tacacs+"))
		   [ProtocolFunctor(self.uri)]));
      fqdn = (label >> *('.' >> label))[FqdnFunctor(self.uri)];
      label = diameterName | diameterDname;
      diameterName = alpha_p >> *(alnum_p | '-');
      diameterDname = digit_p >> +(alnum_p | '-');
      port = str_p(":") >>  int_p[assign(self.uri.port)];
    }
    rule<ScannerT> diameterUri, fqdn, label, diameterName, diameterDname, port;
    rule<ScannerT> transport, protocol;
    rule<ScannerT> const& start() const { return diameterUri; }
  };
  diameter_uri_t& uri;
};

class DiamuriParser : public Utf8stringParser
{
 public:
  DiamuriParser() {}
  void parseRawToApp() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterURIAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_DIAMURI_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
      setAppData(e2);    // Replace the original container entry with
			 // the new UTF8 string container entry.
      AAADictionaryEntry *avp = getDictData();

      DiameterErrorCode st;
      try {
	Utf8stringParser::parseRawToApp();
      }
      catch (DiameterErrorCode &st)
	{
	  setAppData(e); // Recover the original container entry.
	  em.release(e2);// Release the UTF8 string container entry.
	  throw st;
	}
      setAppData(e);     // Recover the original container entry.

      std::string& str = e2->dataRef(Type2Type<diameter_utf8string_t>());
      diameter_uri_t &uri = e->dataRef();

#ifndef BOOST_SPIRIT_THREADSAFE
      AAA_MutexScopeLock lock(AvpGrammarMutex_S);
#endif
      struct DiamuriGrammar grammar(uri_);

      grammar.uri.protocol = DIAMETER_TRANSPORT_PROTO_TCP;
      grammar.uri.transport = DIAMETER_PROTO_DIAMETER;
      if (!parse(str.begin(), str.end(), grammar, space_p).full)
      {
          AAA_LOG((LM_ERROR, "DiamURI parse failure."));
          st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
          em.release(e2);// Release the UTF8 string container entry.
          throw st;
      }
      uri = grammar.uri; // Since the left variable is a reference,
	   		 // grammer.uri data structure is copied into
			 // the container entry where the left
			 // variable refers to.
      em.release(e2);    // Release the UTF8 string container entry.
    };
  
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterURIAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_DIAMURI_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
      diameter_uri_t& uri = e->dataRef();
      std::string& str = e2->dataRef(Type2Type<diameter_octetstring_t>());

      char tmp[20];

      if (uri.scheme == DIAMETER_SCHEME_AAA)
	{
	  str.assign("aaa://");
	}
      else
	{
	  str.assign("aaas://");
	}
      str.append(uri.fqdn);
      sprintf(tmp, ":%d", uri.port);
      str.append(tmp);
      sprintf(tmp, ";transport=%s", 
	      uri.transport == DIAMETER_TRANSPORT_PROTO_TCP ? "tcp" : "sctp");
      str.append(tmp);
      sprintf(tmp, ";protocol=%s", 
	      uri.protocol == DIAMETER_PROTO_DIAMETER ? "diameter" : "radius");
      str.append(tmp);

      setAppData(e2); // Replace the original container entry with the
		      // new UTF8 string container entry.
      try {
	Utf8stringParser::parseAppToRaw();
      }
      catch (DiameterErrorCode &st)
	{
	  setAppData(e);  // Recover the original container entry.
	  em.release(e2); // Release the UTF8 string container entry.
	  throw st;
	}
      setAppData(e); // Recover the original container entry.
      em.release(e2);// Release the UTF8 string container entry.
    }
private:
  diameter_uri_t uri_;
};

struct KeywordFunctor
{
  KeywordFunctor(AAAUInt8 &ref) : ref(ref) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    std::string str = std::string(first,last);
    if (str == "permit")
      ref = DIAMETER_IPFILTER_RULE_ACTION_PERMIT;
    else if (str == "deny")
      ref = DIAMETER_IPFILTER_RULE_ACTION_DENY;
    else if (str == "in")
      ref = DIAMETER_IPFILTER_RULE_DIRECTION_IN;
    else if (str == "out")
      ref = DIAMETER_IPFILTER_RULE_DIRECTION_OUT;
    else if (str == "any")
      ref = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY;
    else if (str == "assigned")
      ref = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED;
    AAA_LOG((LM_DEBUG, "IPFilterRule parser: keyword = %s\n", str.c_str()));
  }
  template <typename IteratorT>
  void operator()(IteratorT val) const
  {
    if (val == '/')
      ref = DIAMETER_IPFILTER_RULE_SRCDST_MASK;
    AAA_LOG((LM_DEBUG, "IPFilterRule parser: keyword = /\n"));
  }
  AAAUInt8 &ref;
};

struct IPAddressFunctor
{
  IPAddressFunctor(diameter_utf8string_t& ipno) : ipno(ipno) {}
  template <typename IteratorT>
  void operator()(IteratorT first, IteratorT last) const
  {
    ipno.assign(first,last);
  }
  diameter_utf8string_t& ipno;
};

struct Int8RangeListFunctor1
{
  Int8RangeListFunctor1(std::list<AAAUInt8Range> &l) : rangeList(l) {}
  template <typename T>
  void operator()(T const& val) const
  {
    rangeList.push_back(AAAUInt8Range(val, val));
  }
  std::list<AAAUInt8Range>& rangeList;
};

struct Int8RangeListFunctor2
{
  Int8RangeListFunctor2(std::list<AAAUInt8Range> &l) : rangeList(l) {}
  template <typename T>
  void operator()(T const& val) const
  {
    rangeList.back().last = val;
  }
  std::list<AAAUInt8Range>& rangeList;
};

struct Int16RangeListFunctor1
{
  Int16RangeListFunctor1(std::list<AAAUInt16Range> &l) : rangeList(l) {}
  template <typename T>
  void operator()(T const& val) const
  {
    rangeList.push_back(AAAUInt16Range(val, val));
  }
  std::list<AAAUInt16Range>& rangeList;
};

struct Int16RangeListFunctor2
{
  Int16RangeListFunctor2(std::list<AAAUInt16Range> &l) : rangeList(l) {}
  template <typename T>
  void operator()(T const& val) const
  {
    rangeList.back().last = val;
  }
  std::list<AAAUInt16Range>& rangeList;
};

struct BoolFunctor
{
  BoolFunctor(bool& ref, bool val) : ref(ref), value(val) {}
  template <typename T>
  void operator()(T const& first, T const& last) const
  {
    ref = value;
  }
  template <typename T>
  void operator()(T const& c) const
  {
    ref = value;
  }
  bool& ref;
  bool value;
};

struct IntListFunctor
{
  public:
  IntListFunctor(std::list<int>& l, int v) : iList(l), value(v) {}
  template <typename T>
  void operator()(T const& first, T const& last) const
  {
    iList.push_back(value);
  }
  std::list<int>& iList;
  int value;
};

struct IPFilterRuleGrammar : public grammar<IPFilterRuleGrammar>
{
  public:
  IPFilterRuleGrammar(diameter_ipfilter_rule_t &ipfilter_rule) 
    : r(ipfilter_rule) {}
  template <typename ScannerT>
  struct definition
  {
    definition(IPFilterRuleGrammar const& self)  
    { 
      AAAUInt8 tmp;
      uint_parser<unsigned, 10, 1, 3> uint3_p;
      ipfilterRule = 
	(str_p("permit") | str_p("deny"))[KeywordFunctor(self.r.action)] 
	>> (str_p("in") | str_p("out"))[KeywordFunctor(self.r.dir)] 
	>> (str_p("ip")[KeywordFunctor(tmp)] | 
	    limit_d(0u,255u)[uint3_p[assign(self.r.proto)]])
	>> str_p("from")[KeywordFunctor(tmp)] >> src 
	>> str_p("to")[KeywordFunctor(tmp)] >> dst >> !options;

      src = !ch_p('!')[BoolFunctor(self.r.src.modifier, false)]
	>> ((str_p("any") | str_p("assigned"))
	[KeywordFunctor(self.r.src.representation)]
	| (ipno[IPAddressFunctor(self.r.src.ipno)] >> 
	   !((ch_p('/')[KeywordFunctor(self.r.src.representation)]
	      >> limit_d(0u,128u)[uint3_p[assign(self.r.src.bits)]]))))
	>> !srcports;

      dst = !ch_p('!')[BoolFunctor(self.r.dst.modifier, false)]
	>> ((str_p("any") | str_p("assigned"))
	[KeywordFunctor(self.r.dst.representation)]
	| (ipno[IPAddressFunctor(self.r.dst.ipno)] >> 
	   !((ch_p('/')[KeywordFunctor(self.r.dst.representation)]
	      >> limit_d(0u,128u)[uint3_p[assign(self.r.dst.bits)]]))))
	>> !dstports;

      ipno = ipv4_dotted_form | ipv6_canonical_form;

      ipv4_dotted_form = 
	limit_d(0u,255u)[uint3_p] >> ch_p('.') >>
	limit_d(0u,255u)[uint3_p] >> ch_p('.') >>
	limit_d(0u,255u)[uint3_p] >> ch_p('.') >>
	limit_d(0u,255u)[uint3_p];

      uint_parser<unsigned, 16, 1, 4> hex4_p;

      ipv6_canonical_form = +(hex4_p | str_p("::"));

#if 0   // too complex rule...
      ipv6_canonical_form = 
	repeat_p(7)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	limit_d(0x0,0xffff)[hex4_p] % ch_p(':') |
	str_p("::") |
	str_p("::") >> 
	repeat_p(0,5)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	repeat_p(0,5)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>	str_p("::") |
	repeat_p(1)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>
	str_p("::") >> 
	repeat_p(0,4)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	repeat_p(2)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>
	str_p("::") >> 
	repeat_p(0,3)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	repeat_p(3)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>
	str_p("::") >> 
	repeat_p(0,2)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	repeat_p(4)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>
	str_p("::") >> 
	repeat_p(0,1)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] |
	repeat_p(5)[limit_d(0x0,0xffff)[hex4_p] >> ch_p(':')] >>
	limit_d(0x0,0xffff)[hex4_p] >>
	str_p("::") >> 
	limit_d(0x0,0xffff)[hex4_p];
#endif
      
      srcports = 
	(uint_p[Int16RangeListFunctor1(self.r.src.portRangeList)] >>
	 !(ch_p('-') >>
	   uint_p[Int16RangeListFunctor2(self.r.src.portRangeList)]))
	% ch_p(',');

      dstports = 
	(uint_p[Int16RangeListFunctor1(self.r.dst.portRangeList)] >>
	 !(ch_p('-') >>
	   uint_p[Int16RangeListFunctor2(self.r.dst.portRangeList)]))
	% ch_p(',');

      options = +(frag | ipoptions | tcpoptions | established | setup
		  | tcpflags | icmptypes);

      frag = str_p("frag")[BoolFunctor(self.r.frag, true)];

      ipoptions = str_p("ipoptions") >> (ssrr | lsrr | rr | ipts) % ch_p(',');

      ssrr = (ch_p('!') >> str_p("ssrr"))
	[IntListFunctor(self.r.ipOptionList, 
			-DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR)] | 
	str_p("ssrr")
	[IntListFunctor(self.r.ipOptionList, 
			DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR)];

      lsrr = (ch_p('!') >> str_p("lsrr"))
	[IntListFunctor(self.r.ipOptionList, 
			-DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR)] | 
	str_p("lsrr")
	[IntListFunctor(self.r.ipOptionList, 
			DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR)];

      rr = (ch_p('!') >> str_p("rr"))
	[IntListFunctor(self.r.ipOptionList, 
			-DIAMETER_IPFILTER_RULE_IP_OPTION_RR)] | 
	str_p("rr")
	[IntListFunctor(self.r.ipOptionList, DIAMETER_IPFILTER_RULE_IP_OPTION_RR)];

      ipts = (ch_p('!') >> str_p("ts"))
	[IntListFunctor(self.r.ipOptionList, 
			-DIAMETER_IPFILTER_RULE_IP_OPTION_TS)] |
	str_p("ts")
	[IntListFunctor(self.r.ipOptionList, DIAMETER_IPFILTER_RULE_IP_OPTION_TS)];

      tcpoptions = str_p("tcpoptions") >> 
	(mss | window | sack | tcpts | cc) % ch_p(',');

      mss = (ch_p('!') >> str_p("mss"))
	[IntListFunctor(self.r.tcpOptionList, 
			-DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS)] | 
	str_p("mss")
	[IntListFunctor(self.r.tcpOptionList, 
			DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS)];

      window = (ch_p('!') >> str_p("window"))
	[IntListFunctor(self.r.tcpOptionList, 
			-DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW)] | 
	str_p("window")
	[IntListFunctor(self.r.tcpOptionList, 
			DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW)];

      sack = (ch_p('!') >> str_p("sack"))
	[IntListFunctor(self.r.tcpOptionList, 
			-DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK)] | 
	str_p("sack")
	[IntListFunctor(self.r.tcpOptionList, 
			DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK)];

      tcpts = (ch_p('!') >> str_p("ts"))
	[IntListFunctor(self.r.tcpOptionList, 
			-DIAMETER_IPFILTER_RULE_TCP_OPTION_TS)] | 
	str_p("ts")
	[IntListFunctor(self.r.tcpOptionList, 
			DIAMETER_IPFILTER_RULE_TCP_OPTION_TS)];

      cc = (ch_p('!') >> str_p("cc"))
	[IntListFunctor(self.r.tcpOptionList, 
			-DIAMETER_IPFILTER_RULE_TCP_OPTION_CC)] | 
	str_p("cc")
	[IntListFunctor(self.r.tcpOptionList, 
			DIAMETER_IPFILTER_RULE_TCP_OPTION_CC)];

      established = str_p("established")
	[BoolFunctor(self.r.established, true)];

      setup = str_p("setup")[BoolFunctor(self.r.setup, true)];

      tcpflags = str_p("tcpflags") >> 
	(fin | syn | rst| psh | ack | urg) % ch_p(',');

      fin = (ch_p('!') >> str_p("fin"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN)] | 
	str_p("fin")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN)];
      syn = (ch_p('!') >> str_p("syn"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN)] | 
	str_p("syn")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN)];
      rst = (ch_p('!') >> str_p("rst"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_RST)] | 
	str_p("rst")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_RST)];

      psh = (ch_p('!') >> str_p("psh"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH)] | 
	str_p("psh")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH)];

      ack = (ch_p('!') >> str_p("ack"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK)] | 
	str_p("ack")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK)];

      urg = (ch_p('!') >> str_p("urg"))
	[IntListFunctor(self.r.tcpFlagList, 
			-DIAMETER_IPFILTER_RULE_TCP_FLAG_URG)] | 
	str_p("urg")
	[IntListFunctor(self.r.tcpFlagList, 
			DIAMETER_IPFILTER_RULE_TCP_FLAG_URG)];

      icmptypes = 
	(uint_p[Int8RangeListFunctor1(self.r.icmpTypeRangeList)] >> 
	 !(ch_p('-') >> 
	   uint_p[Int8RangeListFunctor2(self.r.icmpTypeRangeList)]))
	% ch_p(',');
    }

    rule<ScannerT> ipfilterRule, src, dst, ipno, options;
    rule<ScannerT> ipv4_dotted_form, ipv6_canonical_form, srcports, dstports;
    rule<ScannerT> frag, ipoptions;
    rule<ScannerT> tcpoptions, established, setup, tcpflags, icmptypes;
    rule<ScannerT> ssrr, lsrr, rr, ipts;
    rule<ScannerT> mss, window, sack, tcpts, cc;
    rule<ScannerT> fin, syn, rst, psh, ack, urg;
    rule<ScannerT> const& start() const { return ipfilterRule; }
  };
  diameter_ipfilter_rule_t& r;
};

class IPFilterRuleParser : public Utf8stringParser
{
 public:
  IPFilterRuleParser() {}
  void parseRawToApp() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterIPFilterRuleAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_IPFILTER_RULE_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
      setAppData(e2);    // Replace the original container entry with
			 // the new UTF8 string container entry.
      AAADictionaryEntry *avp = getDictData();

      DiameterErrorCode st;
      try {
	Utf8stringParser::parseRawToApp();
      }
      catch (DiameterErrorCode &st)
	{
	  setAppData(e); // Recover the original container entry.
	  em.release(e2);// Release the UTF8 string container entry.
	  throw st;
	}
      setAppData(e);     // Recover the original container entry.

      std::string& str = e2->dataRef(Type2Type<diameter_utf8string_t>());
      diameter_ipfilter_rule_t &ipfilter_rule = e->dataRef();

      // Set default values.

#ifndef BOOST_SPIRIT_THREADSAFE
      AAA_MutexScopeLock lock(AvpGrammarMutex_S);
#endif
      struct IPFilterRuleGrammar grammar(ipfilter_rule_);
      grammar.r.proto = 0;
      grammar.r.src.modifier = true;
      grammar.r.src.representation = DIAMETER_IPFILTER_RULE_SRCDST_EXACT;
      grammar.r.dst.modifier = true;
      grammar.r.dst.representation = DIAMETER_IPFILTER_RULE_SRCDST_EXACT;
      grammar.r.frag = false;
      grammar.r.established = false;
      grammar.r.setup = false;

      if (!parse(str.begin(), str.end(), grammar, space_p).full)
	{
	  AAA_LOG((LM_ERROR, "IPFilterRule parseRawToApp failure."));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
	  em.release(e2);// Release the UTF8 string container entry.
	  throw st;
	}
      ipfilter_rule = grammar.r;
      em.release(e2);    // Release the UTF8 string container entry.
    };
  
  void parseAppToRaw() throw(DiameterErrorCode)
    {
      DiameterAvpContainerEntryManager em;
      DiameterIPFilterRuleAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_IPFILTER_RULE_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_UTF8_STRING_TYPE);
      diameter_ipfilter_rule_t& r = e->dataRef();
      std::string& str = e2->dataRef(Type2Type<diameter_utf8string_t>());

      if (r.action == DIAMETER_IPFILTER_RULE_ACTION_PERMIT)
	str.append("permit ");
      else
	str.append("deny ");
      if (r.dir == DIAMETER_IPFILTER_RULE_DIRECTION_IN)
	str.append("in ");
      else
	str.append("out ");
      if (r.proto == 0)
	str.append("ip ");
      else
	{
	  char protoStr[4];
	  ACE_OS::sprintf(protoStr, "%u ", r.proto);
	  str.append(protoStr);
	}
      str.append("from ");
      if (!r.src.modifier)
	str.append("!");
      if (r.src.representation == DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY)
	str.append("any ");
      else if (r.src.representation 
	       == DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED)
	str.append("assigned ");
      else {
	str.append(r.src.ipno);
	if (r.src.representation == DIAMETER_IPFILTER_RULE_SRCDST_MASK)
	  {
	    char bitsStr[5];
	    ACE_OS::sprintf(bitsStr, "/%u", r.src.bits);
	    str.append(bitsStr);
	  }
	str.append(" ");
      }
      for (std::list<AAAUInt16Range>::iterator i=r.src.portRangeList.begin();
	   i!=r.src.portRangeList.end();)
	{
	  char portStr[12];
	  if ((*i).first == (*i).last)
	    {
	      ACE_OS::sprintf(portStr, "%u", (*i).first);
	      str.append(portStr);
	    }
	  else
	    {
	      ACE_OS::sprintf(portStr, "%u-%u", (*i).first, (*i).last);
	      str.append(portStr);
	    }
	  if (++i != r.src.portRangeList.end())
	    str.append(",");
	  else
	    str.append(" ");
	}
      
      str.append("to ");
      if (!r.dst.modifier)
	str.append("!");
      if (r.dst.representation == DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY)
	str.append("any ");
      else if (r.dst.representation 
	       == DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED)
	str.append("assigned ");
      else {
	str.append(r.dst.ipno);
	if (r.dst.representation == DIAMETER_IPFILTER_RULE_SRCDST_MASK)
	  {
	    char bitsStr[5];
	    ACE_OS::sprintf(bitsStr, "/%u", r.dst.bits);
	    str.append(bitsStr);
	  }
	str.append(" ");
      }
      for (std::list<AAAUInt16Range>::iterator i=r.dst.portRangeList.begin();
	   i!=r.dst.portRangeList.end();)
	{
	  char portStr[12];
	  if ((*i).first == (*i).last)
	    ACE_OS::sprintf(portStr, "%u", (*i).first);
	  else
	    ACE_OS::sprintf(portStr, "%u-%u", (*i).first, (*i).last);
	  str.append(portStr);
	  if (++i != r.dst.portRangeList.end())
	    str.append(",");
	  else
	    str.append(" ");
	}
      if (r.frag)
	str.append(" frag");
      
      if (r.ipOptionList.size()>0)
	str.append(" ipoptions");
      
      for (std::list<int>::iterator i=r.ipOptionList.begin();
	   i!=r.ipOptionList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR)
	    str.append("ssrr");
	  else if (opt == DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR)
	    str.append("lsrr");
	  else if (opt == DIAMETER_IPFILTER_RULE_IP_OPTION_RR)
	    str.append("rr");
	  else if (opt == DIAMETER_IPFILTER_RULE_IP_OPTION_TS)
	    str.append("ts");
	  if (++i != r.ipOptionList.end())
	    str.append(",");
	}
      

      if (r.tcpOptionList.size()>0)
	str.append(" tcpoptions ");
      
      for (std::list<int>::iterator i=r.tcpOptionList.begin();
	   i!=r.tcpOptionList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS)
	    str.append("mss");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW)
	    str.append("window");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK)
	    str.append("sack");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_OPTION_TS)
	    str.append("ts");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_OPTION_CC)
	    str.append("cc");
	  if (++i != r.tcpOptionList.end())
	    str.append(",");
	}
      

      if (r.established)
	str.append(" established");
      
      if (r.setup)
	str.append(" setup");

      if (r.tcpFlagList.size()>0)
	str.append(" tcpflags");
      
      for (std::list<int>::iterator i=r.tcpFlagList.begin();
	   i!=r.tcpFlagList.end();)
	{
	  bool negation=false;
	  int opt = *i;
	  if (opt < 0)
	    {
	      negation=true;
	      opt = -opt;
	    }
	  if (negation)
	    str.append("!");
	  if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN)
	    str.append("fin");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN)
	    str.append("syn");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_RST)
	    str.append("rst");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH)
	    str.append("psh");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK)
	    str.append("ack");
	  else if (opt == DIAMETER_IPFILTER_RULE_TCP_FLAG_URG)
	    str.append("urg");
	  if (++i != r.tcpFlagList.end())
	    str.append(",");
	}
      
      if (r.icmpTypeRangeList.size()>0)
	str.append(" icmptypes");
      
      for (std::list<AAAUInt8Range>::iterator i=r.icmpTypeRangeList.begin();
	   i!=r.icmpTypeRangeList.end();)
	{
	  char typeStr[8];
	  if ((*i).first == (*i).last)
	    ACE_OS::sprintf(typeStr, "%u", (*i).first);
	  else
	    ACE_OS::sprintf(typeStr, "%u-%u", (*i).first, (*i).last);
	  str.append(typeStr);
	  if (++i != r.icmpTypeRangeList.end())
	    str.append(",");
	}

      AAA_LOG((LM_DEBUG, "Set IPFilterRule = \"%s\"\n", str.c_str()));

      do {
#ifndef BOOST_SPIRIT_THREADSAFE
         AAA_MutexScopeLock lock(AvpGrammarMutex_S);
#endif
         struct IPFilterRuleGrammar grammar(ipfilter_rule_);
         if (!parse(str.begin(), str.end(), grammar, space_p).full)
         {
	    AAA_LOG((LM_ERROR, "IPFilterRule parseAppToRaw failure."));
	    AAADictionaryEntry *avp = getDictData();
	    DiameterErrorCode st;
	    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE, avp);
	    em.release(e2);// Release the UTF8 string container entry.
	    throw st;
	  }
      } while (0);

      setAppData(e2); // Replace the original container entry with the
		      // new UTF8 string container entry.
      try {
	Utf8stringParser::parseAppToRaw();
      }
      catch (DiameterErrorCode &st)
	{
	  setAppData(e);  // Recover the original container entry.
	  em.release(e2); // Release the UTF8 string container entry.
	  throw st;
	}
      setAppData(e); // Recover the original container entry.
      em.release(e2);// Release the UTF8 string container entry.
    }
private:
  diameter_ipfilter_rule_t ipfilter_rule_;
};

class GroupedParser : public DiameterAvpValueParser
{
 public:
  void parseRawToApp() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterGroupedAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_GROUPED_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAADictionaryEntry *avp = getDictData();

      AAAAvpContainerList* acl = e->dataPtr();
      DiameterGroupedAVP* gavp;
      DiameterErrorCode st;

      /* find grouped avp structure */
#ifdef DEBUG
      cout << "Getting Grouped AVP" << avp->avpName << "\n";
#endif

      if ((gavp = DiameterGroupedAvpList::instance()
	   ->search(avp->avpCode, avp->vendorId)) == NULL)
	    {
	      AAA_LOG((LM_ERROR, "Grouped AVP not found."));
	      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_UNSUPPORTED, avp);
	      throw st;
	    }

      do {
	QualifiedAvpListParser qc;
	qc.setRawData(aBuffer);
	qc.setAppData(acl);
	qc.setDictData(gavp);

	try {
	  qc.parseRawToApp();
	}
	catch (DiameterErrorCode &st) {
	  AAA_LOG((LM_ERROR, "Grouped AVP failure"));
	  throw;
	}
      } while (0);
    };

  void parseAppToRaw() throw(DiameterErrorCode)
    {
      AAAMessageBlock* aBuffer = getRawData();
      DiameterGroupedAvpContainerEntry *e;
      getAppData(e);
      if (e->dataType() != AAA_AVP_GROUPED_TYPE)
	{
	  DiameterErrorCode st;
	  AAA_LOG((LM_ERROR, "Invalid AVP type."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	  throw st;
	}
      AAADictionaryEntry *avp = getDictData();

      AAAAvpContainerList *acl = e->dataPtr();
      DiameterGroupedAVP* gavp;
      DiameterErrorCode st;

      if ((gavp = DiameterGroupedAvpList::instance()
	   ->search(avp->avpCode, avp->vendorId)) == NULL)
	{
	  AAA_LOG((LM_ERROR, "Grouped AVP not found"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_UNSUPPORTED, avp);
	  throw st;
	}

      do {
	QualifiedAvpListParser qc;
	qc.setRawData(aBuffer);
	qc.setAppData(acl);
	qc.setDictData(gavp);

	try {
	  qc.parseAppToRaw();
	}
	catch (DiameterErrorCode &st) {
	  AAA_LOG((LM_ERROR, "Grouped AVP failure"));
	  throw;
	}
      } while (0);
    }
};

/* In RFC3588:

      Time
         The Time format is derived from the OctetString AVP Base
         Format. The string MUST contain four octets, in the same format
         as the first four bytes are in the NTP timestamp format. The
         NTP Timestamp format is defined in chapter 3 of [SNTP].
                                                                                
         This represents the number of seconds since 0h on 1 January
         1900 with respect to the Coordinated Universal Time (UTC).
                                                                                
         On 6h 28m 16s UTC, 7 February 2036 the time value will
         overflow. SNTP [SNTP] describes a procedure to extend the time
         to 2104. This procedure MUST be supported by all DIAMETER
         nodes.
 */
/* And from RFC2030 (SNTP):

3. NTP Timestamp Format
                                                                                
   SNTP uses the standard NTP timestamp format described in RFC-1305 and
   previous versions of that document. In conformance with standard
   Internet practice, NTP data are specified as integer or fixed-point
   quantities, with bits numbered in big-endian fashion from 0 starting
   at the left, or high-order, position. Unless specified otherwise, all
   quantities are unsigned and may occupy the full field width with an
   implied 0 preceding bit 0.
                                                                                
   Since NTP timestamps are cherished data and, in fact, represent the
   main product of the protocol, a special timestamp format has been
   established. NTP timestamps are represented as a 64-bit unsigned
   fixed-point number, in seconds relative to 0h on 1 January 1900. The
   integer part is in the first 32 bits and the fraction part in the
   last 32 bits. In the fraction part, the non-significant low order can
   be set to 0.

      It is advisable to fill the non-significant low order bits of the
      timestamp with a random, unbiased bitstring, both to avoid
      systematic roundoff errors and as a means of loop detection and
      replay detection (see below). One way of doing this is to generate
      a random bitstring in a 64-bit word, then perform an arithmetic
      right shift a number of bits equal to the number of significant
      bits of the timestamp, then add the result to the original
      timestamp.
                                                                                
   This format allows convenient multiple-precision arithmetic and
   conversion to UDP/TIME representation (seconds), but does complicate
   the conversion to ICMP Timestamp message representation, which is in
   milliseconds. The maximum number that can be represented is
   4,294,967,295 seconds with a precision of about 200 picoseconds,
   which should be adequate for even the most exotic requirements.
                                                                                
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           Seconds                             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  Seconds Fraction (0-padded)                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                                                                
   Note that, since some time in 1968 (second 2,147,483,648) the most
   significant bit (bit 0 of the integer part) has been set and that the
   64-bit field will overflow some time in 2036 (second 4,294,967,296).
   Should NTP or SNTP be in use in 2036, some external means will be
   necessary to qualify time relative to 1900 and time relative to 2036
   (and other multiples of 136 years). There will exist a 200-picosecond
   interval, henceforth ignored, every 136 years when the 64-bit field
   will be 0, which by convention is interpreted as an invalid or
   unavailable timestamp.
                                                                                
      As the NTP timestamp format has been in use for the last 17 years,
      it remains a possibility that it will be in use 40 years from now
      when the seconds field overflows. As it is probably inappropriate
      to archive NTP timestamps before bit 0 was set in 1968, a
      convenient way to extend the useful life of NTP timestamps is the
      following convention: If bit 0 is set, the UTC time is in the
      range 1968-2036 and UTC time is reckoned from 0h 0m 0s UTC on 1
      January 1900. If bit 0 is not set, the time is in the range 2036-
      2104 and UTC time is reckoned from 6h 28m 16s UTC on 7 February
      2036. Note that when calculating the correspondence, 2000 is not a
      leap year. Note also that leap seconds are not counted in the
      reckoning.
 */
/// The timer parser implementation.  Since the time structure can be
/// simply parsed as a simple 32-bit unsinged integer, the parser
/// treats the data as Unsigned32 type instead of OctetString.
typedef Integer32Parser TimeParser;

#endif // __PARSER_AVPVALUE_H__

