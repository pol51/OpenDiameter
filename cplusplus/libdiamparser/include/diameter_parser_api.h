DiameterIPFilterRuleSrcDst                                              */
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
// $Id: diameter_parser_api.h,v 1.59 2006/05/31 17:53:33 vfajardo Exp $

#ifndef __DIAMETER_PARSER_API_H__
#define __DIAMETER_PARSER_API_H__

#include "aaa_parser_api.h"

#define AAA_NO_VENDOR_ID           DIAMETER_NO_VENDOR_ID

#define AAA_BASE_APPLICATION_ID    DIAMETER_BASE_APPLICATION_ID

/*
 * The following definitions are for backward compatibility
 * as a result of code refactoring.
 */
typedef     AAAInt8             AAA_INT8;

typedef     AAAUInt8            AAA_UINT8;

typedef     AAAIpAddr           IP_ADDR;

typedef     AAAAvpDataType      AAA_AVPDataType;

typedef     AAAUInt8Range       AAAUInt8Range;

typedef     AAAUInt16Range      AAA_UINT16_RANGE;

typedef     DiameterErrorCode   AAAErrorStatus;

typedef     DiameterAvpType     AvpType;

typedef     DiameterAVPCode     AAA_AVPCode;

typedef     DiameterDictionaryEntry AAADictionaryEntry;

typedef     DiameterVendorId    AAAVendorId;

typedef     DiameterAVPFlag     DiameterAVPFlag;

enum  {
    NORMAL = AAA_PARSE_ERROR_TYPE_NORMAL,
    BUG    = AAA_PARSE_ERROR_TYPE_BUG
};

enum {
    MISSING_CONTAINER             = AAA_PARSE_ERROR_MISSING_CONTAINER,
    TOO_MUCH_AVP_ENTRIES          = AAA_PARSE_ERROR_TOO_MUCH_AVP_ENTRIES,
    TOO_LESS_AVP_ENTRIES          = AAA_PARSE_ERROR_TOO_LESS_AVP_ENTRIES,
    PROHIBITED_CONTAINER          = AAA_PARSE_ERROR_PROHIBITED_CONTAINER,
    INVALID_CONTAINER_PARAM       = AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM,
    INVALID_CONTAINER_CONTENTS    = AAA_PARSE_ERROR_INVALID_CONTAINER_CONTENTS,
    UNSUPPORTED_FUNCTIONALITY     = AAA_PARSE_ERROR_UNSUPPORTED_FUNCTIONALITY,
    INVALID_PARSER_USAGE          = AAA_PARSE_ERROR_INVALID_PARSER_USAGE,
    MISSING_AVP_DICTIONARY_ENTRY  = AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY,
    MISSING_AVP_VALUE_PARSER      = AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER
};

enum AAAAvpParseType {
    PARSE_TYPE_FIXED_HEAD         = DIAMETER_PARSE_TYPE_FIXED_HEAD,
//     PARSE_TYPE_REQUIRED           = DIAMETER_PARSE_TYPE_REQUIRED,
    PARSE_TYPE_OPTIONAL           = DIAMETER_PARSE_TYPE_OPTIONAL
};

typedef AAAScholarAttribute          AAA_ScholarAttribute;

typedef AAAGroupedScholarAttribute   AAA_GroupedScholarAttribute;

typedef AAAVectorAttribute           AAA_VectorAttribute;

typedef AAAGroupedVectorAttribute    AAA_GroupedVectorAttribute;

typedef AAAAvpWidget                 AAA_AvpWidget;

typedef AAAIdentityAvpWidget         AAA_IdentityAvpWidget;

typedef AAAAddressAvpWidget          AAA_AddressAvpWidget;

typedef AAAInt32AvpWidget            AAA_Int32AvpWidget;

typedef AAAUInt32AvpWidget           AAA_UInt32AvpWidget;

typedef AAAInt64AvpWidget            AAA_Int64AvpWidget;

typedef AAAUInt64AvpWidget           AAA_UInt64AvpWidget;

typedef AAAUtf8AvpWidget             AAA_Utf8AvpWidget;

typedef AAAGroupedAvpWidget          AAA_GroupedAvpWidget;

typedef AAAStringAvpWidget           AAA_StringAvpWidget;

typedef AAADiamUriAvpWidget          AAA_DiamUriAvpWidget;

typedef AAAEnumAvpWidget             AAA_EnumAvpWidget;

typedef AAATimeAvpWidget             AAA_TimeAvpWidget;

typedef AAAAvpContainerWidget        AAA_AvpContainerWidget;

typedef DiameterApplicationId        AAAApplicationId;

typedef DiameterResultCode           AAAResultCode;

typedef DiameterAvpFlagEnum          AAA_AVPFlagEnum;

typedef DiameterSessionType          AAASessionType;

typedef diameter_avp_t               avp_t;

typedef DiameterAcctMessageType      AAAAcctMessageType;

typedef enum {
    AAA_AVP_FLAG_NONE                 =   DIAMETER_AVP_FLAG_NONE,
    AAA_AVP_FLAG_MANDATORY            =   DIAMETER_AVP_FLAG_MANDATORY,
    AAA_AVP_FLAG_RESERVED             =   DIAMETER_AVP_FLAG_RESERVED,
    AAA_AVP_FLAG_VENDOR_SPECIFIC      =   DIAMETER_AVP_FLAG_VENDOR_SPECIFIC,
    AAA_AVP_FLAG_END_TO_END_ENCRYPT   =   DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT,
    AAA_AVP_FLAG_UNKNOWN              =   DIAMETER_AVP_FLAG_UNKNOWN,
    AAA_AVP_FLAG_ENCRYPT              =   DIAMETER_AVP_FLAG_ENCRYPT,
} DiameterAvpFlagEnum;

enum {
    AAA_IPFILTER_RULE_SRCDST_EXACT             = DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
    AAA_IPFILTER_RULE_SRCDST_MASK              = DIAMETER_IPFILTER_RULE_SRCDST_MASK,
    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY       = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED  = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

typedef class DiameterIPFilterRuleSrcDst    AAA_IPFILTER_RULESRCDST;

enum {
    AAA_IPFILTER_RULEIP_OPTION_SSRR     = DIAMETER_IPFILTER_RULEIP_OPTION_SSRR,
    AAA_IPFILTER_RULEIP_OPTION_LSRR     = DIAMETER_IPFILTER_RULEIP_OPTION_LSRR,
    AAA_IPFILTER_RULEIP_OPTION_RR       = DIAMETER_IPFILTER_RULEIP_OPTION_RR,
    AAA_IPFILTER_RULEIP_OPTION_TS       = DIAMETER_IPFILTER_RULEIP_OPTION_TS
};

enum {
    AAA_IPFILTER_RULETCP_OPTION_MSS     = DIAMETER_IPFILTER_RULETCP_OPTION_MSS,
    AAA_IPFILTER_RULETCP_OPTION_WINDOW  = DIAMETER_IPFILTER_RULETCP_OPTION_WINDOW,
    AAA_IPFILTER_RULETCP_OPTION_SACK    = DIAMETER_IPFILTER_RULETCP_OPTION_SACK,
    AAA_IPFILTER_RULETCP_OPTION_TS      = DIAMETER_IPFILTER_RULETCP_OPTION_TS,
    AAA_IPFILTER_RULETCP_OPTION_CC      = DIAMETER_IPFILTER_RULETCP_OPTION_CC
};

enum {
    AAA_IPFILTER_RULETCP_FLAG_FIN       = DIAMETER_IPFILTER_RULETCP_FLAG_FIN,
    AAA_IPFILTER_RULETCP_FLAG_SYN       = DIAMETER_IPFILTER_RULETCP_FLAG_SYN,
    AAA_IPFILTER_RULETCP_FLAG_RST       = DIAMETER_IPFILTER_RULETCP_FLAG_RST,
    AAA_IPFILTER_RULETCP_FLAG_PSH       = DIAMETER_IPFILTER_RULETCP_FLAG_PSH,
    AAA_IPFILTER_RULETCP_FLAG_ACK       = DIAMETER_IPFILTER_RULETCP_FLAG_ACK,
    AAA_IPFILTER_RULETCP_FLAG_URG       = DIAMETER_IPFILTER_RULETCP_FLAG_URG
};

enum {
    AAA_IPFILTER_RULEACTION_PERMIT      = DIAMETER_IPFILTER_RULEACTION_PERMIT,
    AAA_IPFILTER_RULEACTION_DENY        = DIAMETER_IPFILTER_RULEACTION_DENY
};

enum {
    AAA_IPFILTER_RULEDIRECTION_IN       = DIAMETER_IPFILTER_RULEDIRECTION_IN,
    AAA_IPFILTER_RULEDIRECTION_OUT      = DIAMETER_IPFILTER_RULEDIRECTION_OUT
};

enum {
    AAA_TRANSPORT_PROTO_TCP             = DIAMETER_TRANSPORT_PROTO_TCP,
    AAA_TRANSPORT_PROTO_SCTP            = DIAMETER_TRANSPORT_PROTO_SCTP,
    AAA_TRANSPORT_PROTO_UDP             = DIAMETER_TRANSPORT_PROTO_UDP,
};

enum {
    AAA_PROTO_DIAMETER                  = DIAMETER_PROTO_DIAMETER,
    AAA_PROTO_RADIUS                    = DIAMETER_PROTO_RADIUS,
    AAA_PROTO_TACACSPLUS                = DIAMETER_PROTO_TACACSPLUS,
};

enum {
    AAA_SCHEME_AAA                      = DIAMETER_SCHEME_AAA,
    AAA_SCHEME_AAAS                     = DIAMETER_SCHEME_AAAS
};

typedef DiameterDictionaryManager       AAADictionaryManager;

enum ParseOption {
    AAA_PARSE_LOOSE                     = DIAMETER_PARSE_LOOSE,
    AAA_PARSE_STRICT                    = DIAMETER_PARSE_STRICT,
};

enum ParserError {
    DictionaryError                     = DiameterDictionaryError,
    HeaderError                         = DiameterHeaderError,
    PayloadError                        = DiameterPayloadError
};

typedef struct diameter_hdr_flag       hdr_flag;

#define HEADER_SIZE                    DIAMETER_HEADER_SIZE

typedef class DiameterMsgHeader        AAADiameterHeader;

typedef class DiameterMsg              AAAMessage;

typedef class DiameterMsgHeaderParser  HeaderParser;

typedef class DiameterMsgPayloadParser PayloadParser;

typedef struct diameter_avp_flag       avp_flag;

typedef class DiameterAvpHeader        AAAAvpHeader;

typedef class DiameterMsgResultCode    AAA_MsgResultCode;

typedef class DiameterMsgHeaderDump    AAA_MsgDump;

#define DIAMETER_PROTOCOL_VERSION      AAA_PROTOCOL_VERSION

#define DIAMETER_FLG_SET               AAA_FLG_SET

#define DIAMETER_FLG_CLR               AAA_FLG_CLR

typedef DiameterMsgWidget              AAA_MsgWidget;

--------------------------------------------------------------------------------------------------------------------
/*!
 * Open diameter version declaration
 */
#define DIAMETER_VERSION_MAJOR  0x01
#define DIAMETER_VERSION_MINOR  0x00
#define DIAMETER_VERSION_MICRO  0x05

/*!
 * AAACommandCode provides a way of referring to the AAA command code of
 * a command. It is used when registering callbacks, among others.
 */
typedef ACE_UINT32 AAACommandCode;

/*!
 * DiameterVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others. Note that
 * vendor id 0 is reserved and is defined by the preprocessor constant
 * DIAMETER_NO_VENDOR_ID.
 */
typedef ACE_UINT32 DiameterVendorId;

/*!
 * AAA_AVPCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef ACE_UINT32 AAA_AVPCode;

/*!
 * DiameterApplicationId identifies a particular client session to the API.
 * The application id is passed to AAAStartSession(), and is attached to
 * incoming messages, to indicate with which client session the message
 * is associated.
 */
typedef ACE_UINT32 DiameterApplicationId;

/*!
 * The result codes are values returned from remote servers as part
 * of messages. They correspond directly to the result codes in the
 * Diameter specification [1]:
 */
typedef ACE_UINT32 DiameterResultCode;

/*!
 * The following are used for AVP header flags and for flags in the AVP
 * wrapper struct and AVP dictionary definitions. The first six
 * correspond to the AVP flags defined in the Diameter specification
 * [1]. Some of these are also used in the wrapper struct and dictionary
 * definitions also. The last four are used only in  AAA_AVP and
 *  AAADictionaryEntry:
 *
 *     DIAMETER_AVP_FLAG_NONE -Indicates that no AVP flags are set.
 *
 *     DIAMETER_AVP_FLAG_MANDATORY - Represents the 'M' flag in the Diameter
 *                              AVP header [1], meaning the AVP is
 *                              mandatory.
 *
 *     DIAMETER_AVP_FLAG_RESERVED - Represents the 'R' flag in the Diameter
 *                             AVP header [1]. This flag is reserved and
 *                             should not be set.
 *
 *     DIAMETER_AVP_FLAG_VENDOR_SPECIFIC - Represents the 'V' flag in the
 *                                    Diameter AVP header [1], meaning
 *                                    that the AVP is vendor specific.
 *                                    If this flag is set, the header
 *                                    will contain a vendor identifier.
 *
 *     DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT - Represents the 'P' flag in the
 *                                       Diameter AVP header [1], meaning
 *                                       that the AVP is end-to-end
 *                                       encrypted [3].
 *
 *     DIAMETER_AVP_FLAG_UNKNOWN - Indicates that the AVP was not located in
 *                            the AVP dictionary. This flag is only used
 *                            in AAA_AVP.
 *
 *     DIAMETER_AVP_FLAG_ENCRYPT - Indicates that the AVP was either marked as
 *                            DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT or that it
 *                            was hop-by-hop encrypted (and thus that the
 *                            DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT flag is not
 *                            set on in the AVP header). If the AVP is
 *                            not end-to-end encrypted, then it will be
 *                            either one of the two standard hop-by-hop
 *                            encrypted AVPs, Integrity-Check-Value and
 *                            Encrypted-Payload [1].  This flag is only
 *                            used in AAA_AVP.
 */
typedef enum {
    DIAMETER_AVP_FLAG_NONE =                 0,
    DIAMETER_AVP_FLAG_MANDATORY =            0x1,
    DIAMETER_AVP_FLAG_RESERVED =             0x2,
    DIAMETER_AVP_FLAG_VENDOR_SPECIFIC =      0x4,
    DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT =   0x10,
    DIAMETER_AVP_FLAG_UNKNOWN =              0x10000,
    DIAMETER_AVP_FLAG_ENCRYPT =              0x40000,
} AAA_AVPFlagEnum;

typedef ACE_UINT32 DiameterAVPFlag;

/*!
 * Dictionary Entry Definition
 *
 *  The following structure is returned by the dictionary entry lookup
 *  functions. It contains information about a particular AVP in the
 *  dictionary:
 */
class AAA_PARSER_EXPORT AAADictionaryEntry
{
 public:
  /*!
   * constructor
   *
   * \param code AVP code
   * \param name AVP name
   * \param type AVP type
   * \param id vendor id
   * \param flg AVP flags
   * \param proto Protocol
   */
  AAADictionaryEntry(AAA_AVPCode code, 
                     const char *name, 
                     AAAAvpDataType type, 
                     DiameterVendorId id, 
		     DiameterAVPFlag flg);

  AAA_AVPCode     avpCode;  /**< AVP code */
  std::string     avpName;
  AAAAvpDataType  avpType;  /**< AVP type */
  DiameterVendorId     vendorId; /**< Vendor ID */
  DiameterAVPFlag     flags;    /**< AVP flags */
};

/*!
 * Accounting Types
 *
 *  The following type allows the client to specify which direction to
 *  search for an AVP in the AVP list:
 */
typedef enum {
    AAA_ACCT_EVENT = 1,
    AAA_ACCT_START = 2,
    AAA_ACCT_INTERIM = 3,
    AAA_ACCT_STOP = 4
} DiameterAcctMessageType;

/*!
 * Macros and Preprocessor Definitions
 *
 *  The following definition reserves the vendor id of 0:
 */
#define AAA_NO_VENDOR_ID                                0

#define AAA_BASE_APPLICATION_ID                         0

/*!
 * Data type definitions for AAA Parser
 */
typedef ACE_INT32 diameter_integer32_t;
typedef ACE_UINT64 diameter_integer64_t;
typedef ACE_UINT32 diameter_unsigned32_t;
typedef ACE_UINT64 diameter_unsigned64_t;
typedef diameter_unsigned32_t diameter_enumerated_t;
typedef diameter_unsigned32_t diameter_time_t;  
typedef std::string diameter_octetstring_t;
typedef diameter_octetstring_t diameter_utf8string_t;  
typedef struct {
 public:
  std::string fqdn;
  ACE_UINT16 port; 
  AAA_UINT8 transport:2;
  AAA_UINT8 protocol:2;
  AAA_UINT8 scheme:2;
} diameter_uri_t;
typedef diameter_utf8string_t diameter_identity_t;
typedef diameter_octetstring_t diameter_ipaddress_t;   // XXX: this
						       // will be
						       // deprecated.
						       // Use
						       // diameter_address_t
						       // instead.
typedef struct {
  public:
  ACE_UINT16 type;
  diameter_octetstring_t value;
} diameter_address_t;

typedef class AAAAvpContainerList diameter_grouped_t;

/*! Number of seconds (represented in host byte order) relative to 0h
 *  on 1 January 1900.
 */
typedef diameter_unsigned32_t diameter_time_t;

static inline diameter_unsigned32_t AAA_SWAP_32(diameter_unsigned32_t b) 
{
    return
      ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
       (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24)); 
}

static inline diameter_unsigned64_t AAA_SWAP_64(diameter_unsigned64_t b)
{
    union { 
        diameter_unsigned64_t ll;
        diameter_unsigned32_t l[2]; 
    } w, r;
    w.ll = b;
    r.l[0] = AAA_SWAP_32( w.l[1] );
    r.l[1] = AAA_SWAP_32( w.l[0] );
    return r.ll;
}

enum {
  DIAMETER_IPFILTER_RULESRCDST_EXACT,
  DIAMETER_IPFILTER_RULESRCDST_MASK,
  DIAMETER_IPFILTER_RULESRCDST_KEYWORD_ANY,
  DIAMETER_IPFILTER_RULESRCDST_KEYWORD_ASSIGNED
};

class DiameterIPFilterRuleSrcDst
{
 public:
  DiameterIPFilterRuleSrcDst(AAA_UINT8 repr=DIAMETER_IPFILTER_RULESRCDST_EXACT,
			   diameter_utf8string_t ipno=std::string(),
			   AAA_UINT8 bits=0,
			   bool mod=true) 
    : modifier(mod), representation(repr), ipno(ipno), bits(bits)
    {}
  bool modifier; /*! Modifier '!' maps to false */
  AAA_UINT8
  representation; /*! One of the following:
		    DIAMETER_IPFILTER_RULESRCDST_EXACT,
		    DIAMETER_IPFILTER_RULESRCDST_MASK,
		    DIAMETER_IPFILTER_RULESRCDST_KEYWORD_ANY,
		    DIAMETER_IPFILTER_RULESRCDST_KEYWORD_ASSGINED. 

		    When representation is
		    DIAMETER_IPFILTER_RULESRCDST_EXACT, only ipno is
		    used.  When representation is
		    DIAMETER_IPFILTER_RULESRCDST_MASK, ipno and bits
		    are used.  For other represntations, both ipno and
		    bits are not used.
		  */

  diameter_utf8string_t ipno; 
  AAA_UINT8 bits;

  std::list<AAA_UINT16_RANGE> portRangeList;   /*! list of port ranges. */
};

enum {
  DIAMETER_IPFILTER_RULEIP_OPTION_SSRR=1,
  DIAMETER_IPFILTER_RULEIP_OPTION_LSRR,
  DIAMETER_IPFILTER_RULEIP_OPTION_RR,
  DIAMETER_IPFILTER_RULEIP_OPTION_TS
};

enum {
  DIAMETER_IPFILTER_RULETCP_OPTION_MSS=1,
  DIAMETER_IPFILTER_RULETCP_OPTION_WINDOW,
  DIAMETER_IPFILTER_RULETCP_OPTION_SACK,
  DIAMETER_IPFILTER_RULETCP_OPTION_TS,
  DIAMETER_IPFILTER_RULETCP_OPTION_CC
};

enum {
  DIAMETER_IPFILTER_RULETCP_FLAG_FIN=1,
  DIAMETER_IPFILTER_RULETCP_FLAG_SYN, 
  DIAMETER_IPFILTER_RULETCP_FLAG_RST, 
  DIAMETER_IPFILTER_RULETCP_FLAG_PSH,
  DIAMETER_IPFILTER_RULETCP_FLAG_ACK,
  DIAMETER_IPFILTER_RULETCP_FLAG_URG
};

enum {
  DIAMETER_IPFILTER_RULEACTION_PERMIT,
  DIAMETER_IPFILTER_RULEACTION_DENY 
};

enum {
  DIAMETER_IPFILTER_RULEDIRECTION_IN,
  DIAMETER_IPFILTER_RULEDIRECTION_OUT 
};

/*!
 * values possible for transport field of diameter_diamident_t 
 *
 * avp_t is a special type used only in this library 
 * for constructing a raw AVP.  When using this type, specify 
 * "AVP" as the avp_container type.
 * The string contains the entire AVP including AVP header.
 */
typedef diameter_octetstring_t diameter_avp_t;

/*!
 * values possible for transport field of diameter_uri_t 
 */
enum {
  TRANSPORT_PROTO_TCP = 0,
  TRANSPORT_PROTO_SCTP,
  TRANSPORT_PROTO_UDP,
};

/*!
 * values possible for protocol field of diameter_uri_t 
 */
enum {
  AAA_PROTO_DIAMETER = 0,
  AAA_PROTO_RADIUS,
  AAA_PROTO_TACACSPLUS,
};

/*!
 * values possible for scheme field of diameter_uri_t 
 */
enum {
  AAA_SCHEME_AAA = 0,
  AAA_SCHEME_AAAS
};

/*!
 * values possible for Auth-Request-Type
 */
enum {
  AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY = 1,
  AUTH_REQUEST_TYPE_AUTHORIZE_ONLY = 2,
  AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE = 3
};

/*!
 * default value for port field of diameter_diamident_t 
 */
#define DIAMETER_DEFAULT_PORT 10001










/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * AAAAvpContainerEntryManager and AAAAvpContainerManager is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */ 
class AAA_PARSER_EXPORT AAAAvpContainerManager
{
   public:
      /*!
       * This function assigns a AAAAvpContainer resource of a
       * specific name.
       *
       * \param name AVP name
       */
      AAAAvpContainer *acquire(const char* name);

      /*!
       * This function release a AAAAvpContainer resource.
       *
       * \param entry AVP entry
       */
      void release(AAAAvpContainer* entry);
};

/*!
 * Header bit field definition
 */
struct hdr_flag {
  AAA_UINT8 r:1;  /**< Request */
  AAA_UINT8 p:1;  /**< Proxiable */
  AAA_UINT8 e:1;  /**< Error */
  AAA_UINT8 t:1;  /**< Potentially re-transmitted */
  AAA_UINT8 rsvd:4; /**< Reserved */
};

/*!
 * Boolean definition
 */
typedef bool boolean_t;

class AAAEmptyClass {};

class AAADiameterHeader;
class AAAMessage;
class AAAAvpHeader;

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, ParseOption>
HeaderParser;

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, DiameterDictionaryOption*>
HeaderParserWithProtocol;

/*! AVP codec definition */
typedef AAAParser<std::pair<char*, int>, AAAAvpHeader*> AvpHeaderCodec;

/*! Payload parser definition */
typedef AAAParser<AAAMessageBlock*, AAAAvpContainerList*, 
                  AAADictionaryHandle*, AvpHeaderCodec*>
PayloadParserWithEmptyCodec;

/*!
 * Generic Payload parsing definition
 */
template<class CODEC>
class DIAMETERPARSER_EXPORT_ONLY PayloadParserWithCodec : 
  public PayloadParserWithEmptyCodec
{
   public:
      PayloadParserWithCodec() {
         PayloadParserWithEmptyCodec::setCodec(&codec);
      }
                   
   private:
      CODEC codec;
};

typedef PayloadParserWithCodec<AvpHeaderCodec> 
PayloadParser;
                  
/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the read pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an *
 *  error status is thrown.  When the ParseOption is set to
 *  PARSE_LOOSE, command flag validity check will be skipped.  When
 *  ParseOption is set to PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the AAADiameterHeader so that the command
 *  dictionary can be passed to the payload parser. */
template<> void DIAMETERPARSER_EXPORT_ONLY HeaderParser::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the write pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an
 *  error status is thrown.  When the ParseOption is set to
 *  PARSE_LOOSE, command flag validity check will be skipped.  When
 *  ParseOption is set to PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the AAADiameterHeader so that the command dictionary
 *  can be passed to the payload parser.
 */
template<> void DIAMETERPARSER_EXPORT_ONLY HeaderParser::parseAppToRaw();// throw(AAAErrorStatus);

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., std::pari<char*,int>  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void DIAMETERPARSER_EXPORT_ONLY AvpHeaderCodec::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., std::pari<char*,int>.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void DIAMETERPARSER_EXPORT_ONLY AvpHeaderCodec::parseAppToRaw();// throw(AAAErrorStatus);

/*! \brief AVP header flags
 *
 */
struct avp_flag {
  AAA_UINT8 v; /**< Vendor flag */
  AAA_UINT8 m; /**< Mandatory flag */
  AAA_UINT8 p; /**< end-to-end security flag */
};

/*! \brief AVP header
 *
 */
class AAAAvpHeader
{
 public:
  AAAAvpHeader() : code(0), length(0), 
                   vendor(0), value_p(0),
                   parseType(PARSE_TYPE_OPTIONAL) {
    memset(&flag, 0, sizeof(flag));
  }
  ACE_UINT32 code; /**< AVP code */
  struct avp_flag flag; /**< AVP flags */
  ACE_UINT32 length:24; /**< AVP length */
  ACE_UINT32 vendor; /**< Vendor code */
  char* value_p; /**< Value */
  inline AAAAvpParseType& ParseType() { 
     return parseType; 
  }
 private:
  AAAAvpParseType parseType;
}; 

typedef AAAParser<AAAMessageBlock*, AAAAvpContainerEntry*, 
		     AAADictionaryEntry*, AvpHeaderCodec*> AvpValueParser;

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void DIAMETERPARSER_EXPORT_ONLY AvpValueParser::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void DIAMETERPARSER_EXPORT_ONLY AvpValueParser::parseAppToRaw();// throw(AAAErrorStatus);

/*! Functor type definition for AVP container entry creator.  The
 *  creator function takes an integer argument and returns a pointer
 *  to an AVP container entry.
 */
typedef boost::function1<AAAAvpContainerEntry*, int> AvpContainerEntryFunctor;

/*! Functor type definition for AVP value parser creator.  The creator
 *  function takes null argument and returns a pointer to an AVP value
 *  parser.
 */
typedef boost::function0<AvpValueParser*> DiameterAvpValueParserFunctor;

/*! A template functor class for type-specific AVP container entry
  creator. */
template <class T>
class AvpContainerEntryCreator
{
 public:
  /*!
   * Abstracted new operator
   *
   * \param type entry type
   */
  AAAAvpContainerEntry* operator()(int type) { return new T(type); }
};

/*! A template class for type-specific AVP value parser creator. */
template <class T>
class AvpValueParserCreator
{
 public:
  /*!
   * Abstractor * operator
   */
  AvpValueParser* operator()() { return new T(); }
};

/*! \brief AVP Type List Class Definition. Every AVP type MUST have an
 *  instance for this singleton class with specifying an appropriate
 *  function used for creating the AVP value parser.  This class is
 *  also used for adding user-defined AVP types and parsers.
 *
 *  Example usage: 
 *
 *  #include "diameter_api.h"
 *  #define AAA_AVP_MYDATA_TYPE 10000
 *  static AvpValueParserCreator<MyParser> 
 *         myValueParserCreator;
 *  static AvpContainerEntryCreator<MyAvpContainerEntry> 
 *         myContainerEntryCreator;
 *  AvpTypeList::instance()->add(new AvpType("MyAvp", AAA_AVP_MYDATA_TYPE, 0, 
 *                               myParserCreator, myContainerEntryCreator));
 *
 *  MyAvpParser and MyAvpContainerEntry classes will need to be
 *  defined.
 */
class AAA_PARSER_EXPORT AvpTypeList_S : public std::list<AvpType*> 
{
  friend class ACE_Singleton<AvpTypeList_S, ACE_Recursive_Thread_Mutex>; /**< type list */
 public:
  /*!
   * Returns a pointer to the AVP type instance
   * 
   * \param type AVP type
   */
  AvpType* search(ACE_UINT32 type) 
    {
      for (iterator i = begin(); i != end(); i++)
	{
	  if ((ACE_UINT32)((*i)->getType()) == type)
	    return *i;
	}
      return NULL;
    }
  /*!
   * Returns a pointer to the AVP type instance
   * 
   * \param name AVP name
   */
  AvpType* search(const char* name)
    {
      for (iterator i = begin(); i != end(); i++)
	{
	  if (ACE_OS::strcmp((*i)->getName(), name) == 0)
	    return *i;
	}
      return NULL;
    }
  /*!
   * Adds an AVP type instance to the list
   *
   * \param avpType instance of an AVP type
   */
  void add(AvpType* avpType) {
    mutex.acquire(); 
    push_back(avpType); 
    mutex.release();
  }
 private:
  /*!
   * protected consturctor
   */
  AvpTypeList_S(void) { registerDefaultTypes(); };

  /*!
   * protected destructor
   */
  ~AvpTypeList_S(void) {};

  /*!
   * private method for registering known types
   */
  void registerDefaultTypes();

  ACE_Thread_Mutex mutex; /**< mutex protector */
};

typedef ACE_Singleton<AvpTypeList_S, ACE_Recursive_Thread_Mutex> AvpTypeList;
DIAMETERPARSER_SINGLETON_DECLARE(ACE_Singleton, AvpTypeList_S, ACE_Recursive_Thread_Mutex);

typedef ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex> AAALogMsg_S;
DIAMETERPARSER_SINGLETON_DECLARE(ACE_Singleton, AAALogMsg, ACE_Recursive_Thread_Mutex);

#define AAA_LOG AAALogMsg_S::instance()->log

/*! \brief Generic message header printer
 */
class AAA_MsgDump {
   public:
      static void Dump(AAAMessage &msg) {
          AAA_LOG(LM_INFO, "(%P|%t) Message header dump\n");
          AAA_LOG(LM_INFO, "          version = %d\n", int(msg.hdr.ver));
          AAA_LOG(LM_INFO, "          length  = %d\n", int(msg.hdr.length));
          AAA_LOG(LM_INFO, "     flags(r,p,e,t) = (%d,%d,%d,%d)\n",
                    int(msg.hdr.flags.r), int(msg.hdr.flags.p),
                    int(msg.hdr.flags.e), int(msg.hdr.flags.t));
          AAA_LOG(LM_INFO, "          command = %d\n", int(msg.hdr.code));
          AAA_LOG(LM_INFO, "       hop-by-hop = %d\n", int(msg.hdr.hh));
          AAA_LOG(LM_INFO, "       end-to-end = %d\n", int(msg.hdr.ee));
          AAA_LOG(LM_INFO, "   Application id = %d\n", int(msg.hdr.appId));
      }
};

//
// General 
//
#define AAA_PROTOCOL_VERSION              0x1
#define AAA_FLG_SET                       0x1
#define AAA_FLG_CLR                       0x0

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_DiameterMsgParserWidgetChecked : 
    public DiameterMsgParserWidget
{
public:
    virtual int ParseRawToApp(DiameterMsgWidget &msg,
                              void *buf,
                              int bufSize,
                              int option) {
        AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire
            ((char*)buf, bufSize);
        ACE_Message_Block *aBlock = aBuffer;
        AAAMessageBlockScope block_guard(aBlock); 
        return ParseRawToApp(msg, *aBuffer, option);
    }
    virtual int ParseRawToApp(DiameterMsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
        try {
            return DiameterMsgParserWidget::ParseRawToApp
                      (msg, rawBuf, option);
        }
        catch (AAAErrorStatus &st) {
            ErrorDump(st);
        }
        catch (...) {
            AAA_LOG(LM_INFO, "Parser error: Unknown fatal !!!\n");
            exit (0);
        }
        return (-1);
    }
    virtual int ParseAppToRaw(DiameterMsgWidget &msg,
                              void *buf,
                              int bufSize,
                              int option) {
        AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire
            ((char*)buf, bufSize);
        ACE_Message_Block *aBlock = aBuffer;
        AAAMessageBlockScope block_guard(aBlock); 
        return ParseAppToRaw(msg, *aBuffer, option);
    }
    virtual int ParseAppToRaw(DiameterMsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
        try {
            return DiameterMsgParserWidget::ParseAppToRaw
                      (msg, rawBuf, option);
        }
        catch (AAAErrorStatus &st) {
            ErrorDump(st);
        }
        catch (...) {
            AAA_LOG(LM_INFO, "Parser error: Unknown fatal !!!\n");
            exit (0);
        }
        return (-1);
    }
    static void ErrorDump(AAAErrorStatus &st) {
        AAA_LOG(LM_INFO, "Parser error: ");
        int type, code;
        std::string avp;
        st.get(type, code, avp);
        AAA_LOG(LM_INFO, "Error type=%d, code=%d, name=%s\n",
                type, code, avp.data());
    }
};

#endif /* __DIAMETER_PARSER_API_H__ */



