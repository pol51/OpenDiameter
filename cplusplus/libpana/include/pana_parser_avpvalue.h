/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open PANA_: Open-source software for the PANA_ and               */
/*                PANA_ related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open PANA_ Project                          */
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

#ifndef __PANA_PARSER_AVPVALUE_H__
#define __PANA_PARSER_AVPVALUE_H__

// ACE headers
#include <ace/Synch.h>
#include <ace/Singleton.h>
#include <ace/CDR_Base.h>

// Local headers
#include "resultcodes.h"
#include "pana_parser.h"
#include "pana_parser_avpvalue.h"

/// Container entry type definitions.
typedef AAATypeSpecificAvpContainerEntry<pana_integer32_t>
    PANA_Integer32AvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<pana_unsigned32_t>
    PANA_Unsigned32AvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<pana_integer64_t>
    PANA_Integer64AvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<pana_unsigned64_t>
    PANA_Unsigned64AvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<pana_time_t>
    PANA_TimeAvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<AAAAvpContainerList>
    PANA_GroupedAvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<std::string>
    PANA_StringAvpContainerEntry;

typedef AAATypeSpecificAvpContainerEntry<pana_address_t>
    PANA_AddressAvpContainerEntry;

class PANA_AnyParser :
    public PANA_AvpValueParser
{
    public:
        void parseRawToApp() throw (AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_DATA_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                       AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            e->dataRef().assign(aBuffer->base(), aBuffer->size());
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            std::string& str = e->dataRef();
            if (e->dataType() != AAA_AVP_DATA_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                       AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            if ((aBuffer->size() + (size_t)aBuffer->base() - 
                (size_t)aBuffer->wr_ptr()) < str.length()) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Not enough buffer"));
                st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                       AAA_OUT_OF_SPACE);
                throw st;
            }
            aBuffer->copy(str.data(), str.length());
        }
};

class PANA_Integer32Parser :
    public PANA_AvpValueParser
{
    friend class ACE_Singleton<PANA_Integer32Parser,
                               ACE_Recursive_Thread_Mutex>;

    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_Integer32AvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_INTEGER32_TYPE && 
                e->dataType() != AAA_AVP_UINTEGER32_TYPE &&
                e->dataType() != AAA_AVP_ENUM_TYPE &&
                e->dataType() != AAA_AVP_TIME_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            e->dataRef() = ntohl(*((pana_integer32_t*)(aBuffer->base())));
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer;
            PANA_Integer32AvpContainerEntry *e;
            getRawData(aBuffer);
            getAppData(e);
            if (e->dataType() != AAA_AVP_INTEGER32_TYPE && 
                e->dataType() != AAA_AVP_UINTEGER32_TYPE &&
                e->dataType() != AAA_AVP_ENUM_TYPE &&
                e->dataType() != AAA_AVP_TIME_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            if ((aBuffer->size() + (size_t)aBuffer->base() - 
                (size_t)aBuffer->wr_ptr()) < sizeof(pana_integer32_t)) {
                /* assuming 32-bit boundary */
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Not enough buffer"));
                st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                        AAA_OUT_OF_SPACE);
                throw st;
            }
            *((pana_integer32_t*)(aBuffer->wr_ptr())) = ntohl(e->dataRef());
            aBuffer->wr_ptr(sizeof(pana_integer32_t));
        }
};

class PANA_Integer64Parser :
    public PANA_AvpValueParser
{
    friend class ACE_Singleton<PANA_Integer64Parser,
                               ACE_Recursive_Thread_Mutex>;

    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_Integer64AvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_INTEGER64_TYPE &&
                e->dataType() != AAA_AVP_UINTEGER64_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            e->dataRef() = AAA_NTOH_64(*((pana_integer64_t*)(aBuffer->base())));
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer;
            PANA_Integer64AvpContainerEntry *e;
            getRawData(aBuffer);
            getAppData(e);
            if (e->dataType() != AAA_AVP_INTEGER64_TYPE &&
                e->dataType() != AAA_AVP_UINTEGER64_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            if ((aBuffer->size() + (size_t)aBuffer->base() - 
                (size_t)aBuffer->wr_ptr()) < sizeof(pana_integer64_t)) {
                /* assuming 32-bit boundary */
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Not enough buffer"));
                st.set(AAA_PARSE_ERROR_TYPE_NORMAL,
                        AAA_OUT_OF_SPACE);
                throw st;
            }
            *((pana_integer64_t*)(aBuffer->wr_ptr())) = AAA_HTON_64(e->dataRef());
            aBuffer->wr_ptr(sizeof(pana_integer64_t));
        }
};

class PANA_OctetstringParser :
    public PANA_AvpValueParser
{
        friend class ACE_Singleton<PANA_OctetstringParser,
                                   ACE_Recursive_Thread_Mutex>;

    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_STRING_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            std::string &str = e->dataRef();
            str.assign(aBuffer->base(), aBuffer->size());
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_STRING_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            std::string &str = e->dataRef();

            AAAErrorCode st;
            if (aBuffer->size() + (size_t)aBuffer->base() -
                (size_t)aBuffer->wr_ptr() < str.length()) {
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

class PANA_Utf8stringParser :
    public PANA_OctetstringParser
{
    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_UTF8_STRING_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            std::string &str = e->dataRef();

            AAAErrorCode st;
            e->dataType() = AAA_AVP_STRING_TYPE;
            try {
                PANA_OctetstringParser::parseRawToApp();
            }
            catch (AAAErrorCode &st) {
                e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
                throw st;
            }
            e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
            UTF8Checker check;
            if (check(str.data(), str.size()) != 0)
                {
                    AAA_LOG((LM_ERROR, "Invalid UTF8 string"));
                    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_VALUE);
                    throw st;
                }
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_StringAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_UTF8_STRING_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            std::string &str = e->dataRef();

            AAAErrorCode st;
            UTF8Checker check;
            if (check(str.data(), str.size()) != 0) {
                AAA_LOG((LM_ERROR, "Invalid UTF8 string"));
                st.set(AAA_PARSE_ERROR_TYPE_BUG, 
                        AAA_PARSE_ERROR_INVALID_CONTAINER_CONTENTS);
                throw st;
            }
            e->dataType() = AAA_AVP_STRING_TYPE;
            try {
                PANA_OctetstringParser::parseAppToRaw();
            }
            catch (AAAErrorCode &st)	{
                e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
                throw st;
            }
            e->dataType() = AAA_AVP_UTF8_STRING_TYPE;
        }
};

class PANA_AddressParser :
    public PANA_OctetstringParser
{
    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_AvpContainerEntryManager em;
            PANA_AddressAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_ADDRESS_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG, 
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_STRING_TYPE);
            setAppData(e2);    // Replace the original container entry with
                               // the new address container entry.

            AAAErrorCode st;
            try {
                PANA_OctetstringParser::parseRawToApp();
            }
            catch (AAAErrorCode &st) {
                setAppData(e); // Recover the original container entry.
                em.release(e2);// Release the UTF8 string container entry.
                throw st;
            }
            setAppData(e);     // Recover the original container entry.

            std::string& str = e2->dataRef(Type2Type<pana_octetstring_t>());
            pana_address_t &address = e->dataRef();

            address.type = ntohs(*((ACE_UINT16*)(str.data())));
            address.value.assign(str, 2, str.length() - 2);

            em.release(e2);    // Release the UTF8 string container entry.
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_AvpContainerEntryManager em;
            PANA_AddressAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_ADDRESS_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            AAAAvpContainerEntry *e2 = em.acquire(AAA_AVP_STRING_TYPE);
            pana_address_t& address = e->dataRef();
            std::string& str = e2->dataRef(Type2Type<pana_octetstring_t>());

            ACE_UINT16 tmp = ntohs(address.type);
            char* c = (char*)&tmp;

            str.append(c, 2);
            str.append(address.value);

            setAppData(e2);

            try {
                PANA_OctetstringParser::parseAppToRaw();
            }
            catch (AAAErrorCode &st)
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

class PANA_GroupedParser :
    public PANA_AvpValueParser
{
    public:
        void parseRawToApp() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_GroupedAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_GROUPED_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            AAADictionaryEntry *avp = getDictData();
            AAAAvpContainerList* acl = e->dataPtr();
            PANA_GroupedAVP* gavp;
            AAAErrorCode st;

            /* find grouped avp structure */
#ifdef DEBUG
            cout << "Getting Grouped AVP" << avp->avpName << "\n";
#endif

            if ((gavp = PANA_GroupedAvpList::instance()
                ->search(avp->avpCode, avp->vendorId)) == NULL) {
                    AAA_LOG((LM_ERROR, "Grouped AVP not found."));
                    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_UNSUPPORTED);
                    throw st;
            }

            do {
                PANA_PayloadParser qc;
                qc.setRawData(aBuffer);
                qc.setAppData(acl);
                qc.setDictData(gavp);

                try {
                    qc.parseRawToApp();
                }
                catch (AAAErrorCode &st) {
                    AAA_LOG((LM_ERROR, "Grouped AVP failure"));
                    throw;
                }
            } while (0);
        }
        void parseAppToRaw() throw(AAAErrorCode) {
            PANA_MessageBuffer* aBuffer = getRawData();
            PANA_GroupedAvpContainerEntry *e;
            getAppData(e);
            if (e->dataType() != AAA_AVP_GROUPED_TYPE) {
                AAAErrorCode st;
                AAA_LOG((LM_ERROR, "Invalid AVP type."));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            AAADictionaryEntry *avp = getDictData();
            AAAAvpContainerList *acl = e->dataPtr();
            PANA_GroupedAVP* gavp;
            AAAErrorCode st;

            if ((gavp = PANA_GroupedAvpList::instance()
                ->search(avp->avpCode, avp->vendorId)) == NULL) {
                AAA_LOG((LM_ERROR, "Grouped AVP not found"));
                st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_UNSUPPORTED);
                throw st;
            }

            do {
                PANA_PayloadParser qc;
                qc.setRawData(aBuffer);
                qc.setAppData(acl);
                qc.setDictData(gavp);

                try {
                    qc.parseAppToRaw();
                }
                catch (AAAErrorCode &st) {
                    AAA_LOG((LM_ERROR, "Grouped AVP failure"));
                    throw;
                }
            } while (0);
        }
};

// The timer parser implementation.  Since the time structure can be
// simply parsed as a simple 32-bit unsinged integer, the parser
// treats the data as Unsigned32 type instead of OctetString.
typedef PANA_Integer32Parser PANA_TimeParser;

#endif // __PANA_PARSER_AVPVALUE_H__

