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

/*!
 *=======================================================================
 *
 *                    !!! WARNING !!!
 *
 * THIS FILE IS MAINTAINED FOR BACKWARD COMPATIBILITY ONLY AND NOT
 * MEANT TO BE THE MAIN SOURCE OF API DEFINITIONS. PLS. USE 
 * diameter_parser_api.h INSTEAD.
 *
 *=======================================================================
 */

#include "aaa_parser_api.h"

#define AAA_NO_VENDOR_ID           DIAMETER_NO_VENDOR_ID

#define AAA_BASE_APPLICATION_ID    DIAMETER_BASE_APPLICATION_ID

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
    PARSE_TYPE_FIXED_HEAD         = AAA_PARSE_TYPE_FIXED_HEAD,
    PARSE_TYPE_REQUIRED           = AAA_PARSE_TYPE_REQUIRED,
    PARSE_TYPE_OPTIONAL           = AAA_PARSE_TYPE_OPTIONAL
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

typedef DiameterAvpContainerEntryManager  AAAAvpContainerEntryManager;

typedef DiameterAvpContainerManager    AAAAvpContainerManager;

typedef DiameterScholarAttribute       AAA_ScholarAttribute;

typedef DiameterGroupedScholarAttribute AAA_GroupedScholarAttribute;

typedef DiameterVectorAttribute        AAA_VectorAttribute;

typedef DiameterGroupedVectorAttribute  AAA_GroupedVectorAttribute;

/*! \brief Type specific AVP widget classes
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
typedef DiameterIdentityAvpWidget      AAAIdentityAvpWidget;

typedef DiameterAddressAvpWidget       AAAAddressAvpWidget;

typedef DiameterInt32AvpWidget         AAAInt32AvpWidget;

typedef DiameterUInt32AvpWidget        AAAUInt32AvpWidget;

typedef DiameterInt64AvpWidget         AAAInt64AvpWidget;

typedef DiameterUInt64AvpWidget        AAAUInt64AvpWidget;

typedef DiameterUtf8AvpWidget          AAAUtf8AvpWidget;

typedef DiameterGroupedAvpWidget       AAAGroupedAvpWidget;

typedef DiameterStringAvpWidget        AAAStringAvpWidget;

typedef DiameterDiamUriAvpWidget       AAADiamUriAvpWidget;

typedef DiameterEnumAvpWidget          AAAEnumAvpWidget;

typedef DiameterTimeAvpWidget          AAATimeAvpWidget;

/*! \brief Type specific AVP widget lookup and parser
 *
 *  Assist in adding, deleting and modifying AVP's
 *  contained in a message list.
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
typedef DiameterIdentityAvpContainerWidget    AAAIdentityAvpContainerWidget;

typedef DiameterAddressAvpContainerWidget     AAAAddressAvpContainerWidget;

typedef DiameterInt32AvpContainerWidget       AAAInt32AvpContainerWidget;

typedef DiameterUInt32AvpContainerWidget      AAAUInt32AvpContainerWidget;

typedef DiameterInt64AvpContainerWidget       AAAInt64AvpContainerWidget;

typedef DiameterUInt64AvpContainerWidget      AAAUInt64AvpContainerWidget;

typedef DiameterUtf8AvpContainerWidget        AAAUtf8AvpContainerWidget;

typedef DiameterGroupedAvpContainerWidget     AAAGroupedAvpContainerWidget;

typedef DiameterStringAvpContainerWidget      AAAStringAvpContainerWidget;

typedef DiameterUriAvpContainerWidget         AAADiamUriAvpContainerWidget;

typedef DiameterEnumAvpContainerWidget        AAAEnumAvpContainerWidget;

typedef DiameterTimeAvpContainerWidget        AAATimeAvpContainerWidget;

#endif /* __DIAMETER_PARSER_API_H__ */



