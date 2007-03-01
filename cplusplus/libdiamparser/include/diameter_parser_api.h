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
// $Id: diameter_parser_api.h,v 1.59 2006/05/31 17:53:33 vfajardo Exp $

#ifndef __DIAMETER_PARSER_API_H__
#define __DIAMETER_PARSER_API_H__

///////////////////////////////////////////////////////////////////////////////////
//
//                    !!! WARNING !!!
//
// Backward compatibility Section
//
//   * Includes typedef's to previous class names in version 1.0.7-g and below
//   * Misc backward compatibility definition
//
// THIS FILE IS MAINTAINED FOR BACKWARD COMPATIBILITY ONLY AND NOT
// MEANT TO BE THE MAIN SOURCE OF API DEFINITIONS. PLS USE diameter_parser.h instead
//
///////////////////////////////////////////////////////////////////////////////////

#include "diameter_parser.h"

//
// Old macros mapped to current macros
//
#define   AAA_PROTOCOL_VERSION           DIAMETER_PROTOCOL_VERSION
#define   AAA_BASE_APPLICATION_ID        DIAMETER_BASE_APPLICATION_ID
#define   AAA_NO_VENDOR_ID               DIAMETER_NO_VENDOR_ID
#define   HEADER_SIZE                    DIAMETER_HEADER_SIZE
#define   DIAMETER_FLG_SET               AAA_FLG_SET
#define   DIAMETER_FLG_CLR               AAA_FLG_CLR

//
// Old enums mapped to current enums via macros
//
#define   PARSE_LOOSE                    DIAMETER_PARSE_LOOSE
#define   PARSE_STRICT                   DIAMETER_PARSE_STRICT

//
// Old template class names mapped to current class names
// Note that macros are used for correctness since typedef
// would require changes to exsiting code
//
#define   AAA_ScholarAttribute           DiameterScholarAttribute
#define   AAA_GroupedScholarAttribute    DiameterGroupedScholarAttribute
#define   AAA_VectorAttribute            DiameterVectorAttribute
#define   AAA_GroupedVectorAttribute     DiameterGroupedVectorAttribute
#define   AAA_AvpWidget                  DiameterAvpWidget
#define   AAA_AvpContainerWidget         DiameterAvpContainerWidget

//
// Old basic data types mapped to current data types
//
typedef   AAAInt8                        AAA_INT8;
typedef   AAAUInt8                       AAA_UINT8;
typedef   AAAIpAddr                      IP_ADDR;
typedef   AAAAvpDataType                 AAA_AVPDataType;
typedef   AAAAVPCode                     AAA_AVPCode;
typedef   AAAAVPFlag                     AAA_AVPFlag;
typedef   diameter_avp_t                 avp_t;

//
// Old utility classes operating on old types mapped to current classes
//
typedef   AAAUInt8Range                  AAA_UINT8_RANGE;
typedef   AAAUInt16Range                 AAA_UINT16_RANGE;
typedef   DiameterIPFilterRuleSrcDst     AAA_IPFILTER_RULE_SRCDST;

//
// Old classes mapped to current classes
//
typedef   DiameterAvpType                AvpType;
typedef   DiameterErrorCode              AAAErrorStatus;
typedef   DiameterApplicationId          AAAApplicationId;
typedef   DiameterResultCode             AAAResultCode;
typedef   DiameterAvpFlagEnum            AAA_AVPFlagEnum;

//
// Old data structure classes mapped to current classes
//
typedef   DiameterDictionaryManager      AAADictionaryManager;
typedef   DiameterParseOption            ParseOption;
typedef   DiameterAvpContainerEntryManager  AAAAvpContainerEntryManager;
typedef   DiameterAvpContainerManager    AAAAvpContainerManager;

//
// Old utility classes operating as message parser to current classes
//
typedef   struct diameter_hdr_flag       hdr_flag;
typedef   struct diameter_avp_flag       avp_flag;
typedef   DiameterAvpHeader              AAAAvpHeader;
typedef   DiameterMsgHeader              AAADiameterHeader;
typedef   DiameterMsg                    AAAMessage;
typedef   DiameterMsgHeaderParser        HeaderParser;
typedef   DiameterMsgPayloadParser       PayloadParser;

//
// Old utility classes operating on message mapped to current classes
//
typedef   DiameterMsgWidget              AAA_MsgWidget;
typedef   DiameterMsgResultCode          AAA_MsgResultCode;
typedef   DiameterMsgHeaderDump          AAA_MsgDump;
typedef   DiameterMsgParserWidget        AAA_DiameterMsgParserWidget;
typedef   DiameterMsgParserWidgetChecked AAA_DiameterMsgParserWidgetChecked;

//
// Old wrapper classes for AVP widgets mapped to new classes
//
typedef   DiameterIdentityAvpWidget      AAA_IdentityAvpWidget;
typedef   DiameterAddressAvpWidget       AAA_AddressAvpWidget;
typedef   DiameterInt32AvpWidget         AAA_Int32AvpWidget;
typedef   DiameterUInt32AvpWidget        AAA_UInt32AvpWidget;
typedef   DiameterInt64AvpWidget         AAA_Int64AvpWidget;
typedef   DiameterUInt64AvpWidget        AAA_UInt64AvpWidget;
typedef   DiameterUtf8AvpWidget          AAA_Utf8AvpWidget;
typedef   DiameterGroupedAvpWidget       AAA_GroupedAvpWidget;
typedef   DiameterStringAvpWidget        AAA_StringAvpWidget;
typedef   DiameterDiamUriAvpWidget       AAA_DiamUriAvpWidget;
typedef   DiameterEnumAvpWidget          AAA_EnumAvpWidget;
typedef   DiameterTimeAvpWidget          AAA_TimeAvpWidget;

//
// Old wrapper classes for AVP container widgets mapped to new classes
//
typedef   DiameterIdentityAvpContainerWidget  AAA_IdentityAvpContainerWidget;
typedef   DiameterAddressAvpContainerWidget   AAA_AddressAvpContainerWidget;
typedef   DiameterInt32AvpContainerWidget     AAA_Int32AvpContainerWidget;
typedef   DiameterUInt32AvpContainerWidget    AAA_UInt32AvpContainerWidget;
typedef   DiameterInt64AvpContainerWidget     AAA_Int64AvpContainerWidget;
typedef   DiameterUInt64AvpContainerWidget    AAA_UInt64AvpContainerWidget;
typedef   DiameterUtf8AvpContainerWidget      AAA_Utf8AvpContainerWidget;
typedef   DiameterGroupedAvpContainerWidget   AAA_GroupedAvpContainerWidget;
typedef   DiameterStringAvpContainerWidget    AAA_StringAvpContainerWidget;
typedef   DiameterUriAvpContainerWidget       AAA_DiamUriAvpContainerWidget;
typedef   DiameterEnumAvpContainerWidget      AAA_EnumAvpContainerWidget;
typedef   DiameterTimeAvpContainerWidget      AAA_TimeAvpContainerWidget;

//
// Old enumerations mapped to current enums
//
enum  {
    NORMAL                             = AAA_PARSE_ERROR_TYPE_NORMAL,
    BUG                                = AAA_PARSE_ERROR_TYPE_BUG
};

enum {
    MISSING_CONTAINER                  = AAA_PARSE_ERROR_MISSING_CONTAINER,
    TOO_MUCH_AVP_ENTRIES               = AAA_PARSE_ERROR_TOO_MUCH_AVP_ENTRIES,
    TOO_LESS_AVP_ENTRIES               = AAA_PARSE_ERROR_TOO_LESS_AVP_ENTRIES,
    PROHIBITED_CONTAINER               = AAA_PARSE_ERROR_PROHIBITED_CONTAINER,
    INVALID_CONTAINER_PARAM            = AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM,
    INVALID_CONTAINER_CONTENTS         = AAA_PARSE_ERROR_INVALID_CONTAINER_CONTENTS,
    UNSUPPORTED_FUNCTIONALITY          = AAA_PARSE_ERROR_UNSUPPORTED_FUNCTIONALITY,
    INVALID_PARSER_USAGE               = AAA_PARSE_ERROR_INVALID_PARSER_USAGE,
    MISSING_AVP_DICTIONARY_ENTRY       = AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY,
    MISSING_AVP_VALUE_PARSER           = AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER
};

typedef enum {
    AAA_AVP_FLAG_NONE                  = DIAMETER_AVP_FLAG_NONE,
    AAA_AVP_FLAG_MANDATORY             = DIAMETER_AVP_FLAG_MANDATORY,
    AAA_AVP_FLAG_RESERVED              = DIAMETER_AVP_FLAG_RESERVED,
    AAA_AVP_FLAG_VENDOR_SPECIFIC       = DIAMETER_AVP_FLAG_VENDOR_SPECIFIC,
    AAA_AVP_FLAG_END_TO_END_ENCRYPT    = DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT,
    AAA_AVP_FLAG_UNKNOWN               = DIAMETER_AVP_FLAG_UNKNOWN,
    AAA_AVP_FLAG_ENCRYPT               = DIAMETER_AVP_FLAG_ENCRYPT,
} AAAAvpFlagEnum;

enum {
    AAA_IPFILTER_RULE_SRCDST_EXACT             = DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
    AAA_IPFILTER_RULE_SRCDST_MASK              = DIAMETER_IPFILTER_RULE_SRCDST_MASK,
    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY       = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED  = DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};


enum {
    AAA_IPFILTER_RULE_IP_OPTION_SSRR           = DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR,
    AAA_IPFILTER_RULE_IP_OPTION_LSRR           = DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR,
    AAA_IPFILTER_RULE_IP_OPTION_RR             = DIAMETER_IPFILTER_RULE_IP_OPTION_RR,
    AAA_IPFILTER_RULE_IP_OPTION_TS             = DIAMETER_IPFILTER_RULE_IP_OPTION_TS
};

enum {
    AAA_IPFILTER_RULE_TCP_OPTION_MSS           = DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS,
    AAA_IPFILTER_RULE_TCP_OPTION_WINDOW        = DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW,
    AAA_IPFILTER_RULE_TCP_OPTION_SACK          = DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK,
    AAA_IPFILTER_RULE_TCP_OPTION_TS            = DIAMETER_IPFILTER_RULE_TCP_OPTION_TS,
    AAA_IPFILTER_RULE_TCP_OPTION_CC            = DIAMETER_IPFILTER_RULE_TCP_OPTION_CC
};

enum {
    AAA_IPFILTER_RULE_TCP_FLAG_FIN             = DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN,
    AAA_IPFILTER_RULE_TCP_FLAG_SYN             = DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN,
    AAA_IPFILTER_RULE_TCP_FLAG_RST             = DIAMETER_IPFILTER_RULE_TCP_FLAG_RST,
    AAA_IPFILTER_RULE_TCP_FLAG_PSH             = DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH,
    AAA_IPFILTER_RULE_TCP_FLAG_ACK             = DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK,
    AAA_IPFILTER_RULE_TCP_FLAG_URG             = DIAMETER_IPFILTER_RULE_TCP_FLAG_URG
};

enum {
    AAA_IPFILTER_RULE_ACTION_PERMIT            = DIAMETER_IPFILTER_RULE_ACTION_PERMIT,
    AAA_IPFILTER_RULE_ACTION_DENY              = DIAMETER_IPFILTER_RULE_ACTION_DENY
};

enum {
    AAA_IPFILTER_RULE_DIRECTION_IN             = DIAMETER_IPFILTER_RULE_DIRECTION_IN,
    AAA_IPFILTER_RULE_DIRECTION_OUT            = DIAMETER_IPFILTER_RULE_DIRECTION_OUT
};

enum {
    TRANSPORT_PROTO_TCP                        = DIAMETER_TRANSPORT_PROTO_TCP,
    TRANSPORT_PROTO_SCTP                       = DIAMETER_TRANSPORT_PROTO_SCTP,
    TRANSPORT_PROTO_UDP                        = DIAMETER_TRANSPORT_PROTO_UDP,
};

enum {
    AAA_PROTO_DIAMETER                         = DIAMETER_PROTO_DIAMETER,
    AAA_PROTO_RADIUS                           = DIAMETER_PROTO_RADIUS,
    AAA_PROTO_TACACSPLUS                       = DIAMETER_PROTO_TACACSPLUS,
};

enum {
    AAA_SCHEME_AAA                             = DIAMETER_SCHEME_AAA,
    AAA_SCHEME_AAAS                            = DIAMETER_SCHEME_AAAS
};

enum ParserError {
    DictionaryError                            = DIAMETER_DICTIONARY_ERROR,
    HeaderError                                = DIAMETER_HEADER_ERROR,
    PayloadError                               = DIAMETER_PAYLOAD_ERROR
};

#endif /* __DIAMETER_PARSER_API_H__ */



