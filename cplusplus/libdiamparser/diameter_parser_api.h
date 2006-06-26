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
// $Id: diameter_parser_api.h,v 1.59 2006/05/31 17:53:33 vfajardo Exp $

#ifndef __DIAMETER_PARSER_API_H__
#define __DIAMETER_PARSER_API_H__

#include <resultcodes.h>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <boost/function/function0.hpp>
#include <boost/function/function1.hpp>

#include <ace/Basic_Types.h>
#include <ace/INET_Addr.h>
#include <ace/Message_Block.h>
#include <ace/Atomic_Op.h>
#include <ace/Synch.h>
#include <ace/Singleton.h>
#include <ace/Malloc_T.h>
#include <ace/Log_Msg.h>
#include <ace/Local_Memory_Pool.h>

#include "framework.h"

/*!
 * Open diameter version declaration
 */
#define AAA_VERSION_MAJOR  0x01
#define AAA_VERSION_MINOR  0x00
#define AAA_VERSION_MICRO  0x05

/*!
 * Windows specific export declarations
 */
#if defined (WIN32)
#  if defined (DIAMETERPARSER_EXPORTS)
#    define DIAMETERPARSER_EXPORT ACE_Proper_Export_Flag
#    define DIAMETERPARSER_EXPORT_ONLY ACE_Proper_Export_Flag
#    define DIAMETERPARSER_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else
#    define DIAMETERPARSER_EXPORT ACE_Proper_Import_Flag
#    define DIAMETERPARSER_EXPORT_ONLY
#    define DIAMETERPARSER_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif   /* ! DIAMETERPARSER_EXPORTS */
#else
#  define DIAMETERPARSER_EXPORT
#  define DIAMETERPARSER_EXPORT_ONLY
#  define DIAMETERPARSER_SINGLETON_DECLARATION(T)
#  define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     /* WIN32 */

/*!
 * Basic supplimental types for multi-platform support 
 */
typedef char AAA_INT8;
typedef unsigned char AAA_UINT8;

/*! 
 * IP_ADDR provides a way of referring to an IPv4 address, IPv6 address,
 * and IP port. The default implementation (shown here) is defined in
 * the Basic Socket Interface Extensions for IPv6 RFC[10].
 */
typedef ACE_INET_Addr IP_ADDR;

/*!
 * AAACommandCode provides a way of referring to the AAA command code of
 * a command. It is used when registering callbacks, among others.
 */
typedef ACE_UINT32 AAACommandCode;

/*!
 * AAAVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others. Note that
 * vendor id 0 is reserved and is defined by the preprocessor constant
 * AAA_NO_VENDOR_ID.
 */
typedef ACE_UINT32 AAAVendorId;

/*!
 * AAA_AVPCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef ACE_UINT32 AAA_AVPCode;

/*!
 * AAASessionHandle is an identifier for a particular AAA session. It is
 * used in the session APIs and when a message is created.
 */
typedef void* AAASessionHandle;

/*!
 * AAAApplicationId identifies a particular client session to the API.
 * The application id is passed to AAAStartSession(), and is attached to
 * incoming messages, to indicate with which client session the message
 * is associated.
 */
typedef ACE_UINT32 AAAApplicationId;

/*!
 * The result codes are values returned from remote servers as part
 * of messages. They correspond directly to the result codes in the
 * Diameter specification [1]:
 */
typedef ACE_UINT32 AAAResultCode;

/*!
 * API Return Codes
 *  
 * The following is a description of the error codes and the reasons why
 * they can be returned:
 *
 *   AAA_ERR_NOT_FOUND - This code is returned if a handle or id was
 *                       not found.
 *
 *   AAA_ERR_FAILURE - This code is returned if an unspecified failure
 *                     occurred during an AAA operation.
 *
 *   AAA_ERR_SUCCESS - This code is returned if the AAA operation
 *                     succeeded.
 *
 *   AAA_ERR_NOMEM - This code is returned if the operation caused
 *                   memory to be exhausted.
 *
 *   AAA_ERR_PROTO - This code is returned if a AAA protocol error
 *                   occurred.
 *
 *   AAA_ERR_SECURITY - This code is returned if a security check
 *                      failed or another security error occurred.
 *
 *   AAA_ERR_PARAMETER - This code is returned if an invalid parameter
 *                       was passed to an AAA function.
 *
 *   AAA_ERR_CONFIG - This code is returned if an error was encountered
 *                    in a configuration file during library
 *                    initialization.
 *
 *   AAA_ERR_UNKNOWN_CMD - This code is returned if the library
 *                         received a AAA command that is not in the
 *                         set of registered AAA commands.
 *
 *   AAA_ERR_MISSING_AVP - This code is returned if a command was
 *                         received without a required AVP.
 *
 *   AAA_ERR_ALREADY_INIT - This code is returned if an attempt is made
 *                          to initialize the AAA library when it has
 *                          already been initialized.
 *
 *   AAA_ERR_TIMED_OUT - This code is returned when a network operation
 *                       times out.
 *
 *   AAA_ERR_CANNOT_SEND_MSG - This code is returned if the library
 *                             can't send a message. It is also of used
 *                             to application profile extensions that
 *                             encounter the same error condition.
 *
 *   AAA_ERR_ALREADY_REGISTERED - This code is returned by the command
 *                                registration functions if the command
 *                                was already registered.
 *
 *   AAA_ERR_CANNOT_REGISTER - This code is returned by the command
 *                             registration functions if the command
 *                             could not be registered.
 *
 *   AAA_ERR_NOT_INITIALIZED - This code is returned by any function in
 *                             the API except AAAOpen() if the library
 *                             hasn't been initialized.
 *
 *    AAA_ERR_NETWORK_ERROR - This code is returned by any function if
 *                            an error in networking occurs.
 */
typedef enum {
    AAA_ERR_NOT_FOUND =     -2,
    AAA_ERR_FAILURE =       -1,
    AAA_ERR_SUCCESS =        0,
    AAA_ERR_NOMEM,
    AAA_ERR_PROTO,
    AAA_ERR_SECURITY,
    AAA_ERR_PARAMETER,
    AAA_ERR_CONFIG,
    AAA_ERR_UNKNOWN_CMD,
    AAA_ERR_MISSING_AVP,
    AAA_ERR_ALREADY_INIT,
    AAA_ERR_TIMED_OUT,
    AAA_ERR_CANNOT_SEND_MSG,
    AAA_ERR_ALREADY_REGISTERED,
    AAA_ERR_CANNOT_REGISTER,
    AAA_ERR_NOT_INITIALIZED,
    AAA_ERR_NETWORK_ERROR,
    AAA_ERR_MSG_UNPROCESSED,
    AAA_ERR_INVALID_STATE,
    AAA_ERR_PARSING_FAILED,
    AAA_ERR_UNKNOWN_SESSION,
    AAA_ERR_PARSING_ERROR,
    AAA_ERR_INCOMPLETE,
    AAA_ERR_NOSERVICE
} AAAReturnCode;

/*!
 * The following are AVP data type codes. They correspond directly to
 * the AVP data types outline in the Diameter specification [1]:
 */
typedef enum {
    AAA_AVP_DATA_TYPE,
    AAA_AVP_STRING_TYPE,
    AAA_AVP_ADDRESS_TYPE,          
    AAA_AVP_INTEGER32_TYPE,
    AAA_AVP_INTEGER64_TYPE,
    AAA_AVP_UINTEGER32_TYPE,
    AAA_AVP_UINTEGER64_TYPE,
    AAA_AVP_UTF8_STRING_TYPE,
    AAA_AVP_ENUM_TYPE,
    AAA_AVP_DIAMID_TYPE,
    AAA_AVP_DIAMURI_TYPE,
    AAA_AVP_GROUPED_TYPE,
    AAA_AVP_TIME_TYPE,
    AAA_AVP_IPFILTER_RULE_TYPE,
    AAA_AVP_CUSTOM_TYPE,
} AAA_AVPDataType;


/*!
 * The following are used for AVP header flags and for flags in the AVP
 * wrapper struct and AVP dictionary definitions. The first six
 * correspond to the AVP flags defined in the Diameter specification
 * [1]. Some of these are also used in the wrapper struct and dictionary
 * definitions also. The last four are used only in  AAA_AVP and
 *  AAADictionaryEntry:
 *
 *     AAA_AVP_FLAG_NONE -Indicates that no AVP flags are set.
 *
 *     AAA_AVP_FLAG_MANDATORY - Represents the 'M' flag in the Diameter
 *                              AVP header [1], meaning the AVP is
 *                              mandatory.
 *
 *     AAA_AVP_FLAG_RESERVED - Represents the 'R' flag in the Diameter
 *                             AVP header [1]. This flag is reserved and
 *                             should not be set.
 *
 *     AAA_AVP_FLAG_VENDOR_SPECIFIC - Represents the 'V' flag in the
 *                                    Diameter AVP header [1], meaning
 *                                    that the AVP is vendor specific.
 *                                    If this flag is set, the header
 *                                    will contain a vendor identifier.
 *
 *     AAA_AVP_FLAG_END_TO_END_ENCRYPT - Represents the 'P' flag in the
 *                                       Diameter AVP header [1], meaning
 *                                       that the AVP is end-to-end
 *                                       encrypted [3].
 *
 *     AAA_AVP_FLAG_UNKNOWN - Indicates that the AVP was not located in
 *                            the AVP dictionary. This flag is only used
 *                            in AAA_AVP.
 *
 *     AAA_AVP_FLAG_ENCRYPT - Indicates that the AVP was either marked as
 *                            AAA_AVP_FLAG_END_TO_END_ENCRYPT or that it
 *                            was hop-by-hop encrypted (and thus that the
 *                            AAA_AVP_FLAG_END_TO_END_ENCRYPT flag is not
 *                            set on in the AVP header). If the AVP is
 *                            not end-to-end encrypted, then it will be
 *                            either one of the two standard hop-by-hop
 *                            encrypted AVPs, Integrity-Check-Value and
 *                            Encrypted-Payload [1].  This flag is only
 *                            used in AAA_AVP.
 */
typedef enum {
    AAA_AVP_FLAG_NONE =                 0,
    AAA_AVP_FLAG_MANDATORY =            0x1,
    AAA_AVP_FLAG_RESERVED =             0x2,
    AAA_AVP_FLAG_VENDOR_SPECIFIC =      0x4,
    AAA_AVP_FLAG_END_TO_END_ENCRYPT =   0x10,
    AAA_AVP_FLAG_UNKNOWN =              0x10000,
    AAA_AVP_FLAG_ENCRYPT =              0x40000,
} AAA_AVPFlagEnum;

typedef ACE_UINT32 AAA_AVPFlag;

/*!
 * Dictionary Entry Definition
 *
 *  The following structure is returned by the dictionary entry lookup
 *  functions. It contains information about a particular AVP in the
 *  dictionary:
 */
class DIAMETERPARSER_EXPORT AAADictionaryEntry
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
                     AAA_AVPDataType type, 
                     AAAVendorId id, 
		     AAA_AVPFlag flg,
                     int proto);

  AAA_AVPCode     avpCode;  /**< AVP code */
  std::string     avpName;
  AAA_AVPDataType avpType;  /**< AVP type */
  AAAVendorId     vendorId; /**< Vendor ID */
  AAA_AVPFlag     flags;    /**< AVP flags */
  int             protocol; /**< Protocol */  
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
} AAAAcctMessageType;

/*!
 * Session type
 */
typedef enum {
    AAA_STYPE_AUTHENTICATION,  /**< Authentication session */
    AAA_STYPE_ACCOUNTING       /**< Accounting session */
} AAASessionType;

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

// http://www.iana.org/assignments/address-family-numbers
enum {
    AAA_ADDR_FAMILY_RESERVED    = 0,   // Reserved
    AAA_ADDR_FAMILY_IPV4        = 1,   // IP (IP version 4)
    AAA_ADDR_FAMILY_IPV6        = 2,   // IP6 (IP version 6)
    AAA_ADDR_FAMILY_NASP        = 3,   // NSAP
    AAA_ADDR_FAMILY_HDLC        = 4,   // HDLC (8-bit multidrop)
    AAA_ADDR_FAMILY_BBN         = 5,   // 5    BBN 1822
    AAA_ADDR_FAMILY_802         = 6,   // 802 (includes all 802 media plus Ethernet "canonical format")
    AAA_ADDR_FAMILY_E163        = 7,   // E.163
    AAA_ADDR_FAMILY_E164        = 8,   // E.164 (SMDS, Frame Relay, ATM)
    AAA_ADDR_FAMILY_F69         = 9,   // F.69 (Telex)
    AAA_ADDR_FAMILY_X121        = 10,  // X.121 (X.25, Frame Relay)
    AAA_ADDR_FAMILY_IPX         = 11,  // IPX
    AAA_ADDR_FAMILY_APPLETALK   = 12,  // Appletalk
    AAA_ADDR_FAMILY_DECIV       = 13,  // Decnet IV
    AAA_ADDR_FAMILY_BANYAN      = 14,  // Banyan Vines
    AAA_ADDR_FAMILY_E164_NSAP   = 15,  // E.164 with NSAP format subaddress 
    AAA_ADDR_FAMILY_DNS         = 16,  // DNS (Domain Name System)
    AAA_ADDR_FAMILY_DN          = 17,  // Distinguished Name
    AAA_ADDR_FAMILY_AS          = 18,  // AS Number
    AAA_ADDR_FAMILY_XTP4        = 19,  // XTP over IP version 4
    AAA_ADDR_FAMILY_XTP6        = 20,  // XTP over IP version 6
    AAA_ADDR_FAMILY_XTP         = 21,  // XTP native mode XTP
    AAA_ADDR_FAMILY_FBRCH_PORT  = 22,  // Fibre Channel World-Wide Port Name
    AAA_ADDR_FAMILY_FBRCH_NODE  = 23,  // Fibre Channel World-Wide Node Name
    AAA_ADDR_FAMILY_GWID        = 24  // GWID
};

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

#if __BYTE_ORDER == __BIG_ENDIAN
#define AAA_NTOH_32(x) (x)
#define AAA_NTOH_64(x) (x)
#define AAA_HTON_32(x) (x)
#define AAA_HTON_64(x) (x)
#else
#define AAA_NTOH_32(x) AAA_SWAP_32(x)
#define AAA_NTOH_64(x) AAA_SWAP_64(x)
#define AAA_HTON_32(x) AAA_SWAP_32(x)
#define AAA_HTON_64(x) AAA_SWAP_64(x)
#endif

class AAA_UINT8_RANGE
{
  public:
  AAA_UINT8_RANGE(AAA_UINT8 first, AAA_UINT8 last) : first(first), last(last)
  {}
  AAA_UINT8_RANGE() 
  {}
  AAA_UINT8 first;
  AAA_UINT8 last;
};

class AAA_UINT16_RANGE
{
  public:
  AAA_UINT16_RANGE(ACE_UINT16 first, ACE_UINT16 last) : 
  first(first), last(last)
  {}
  AAA_UINT16_RANGE(ACE_UINT16 single) : first(single), last(single)
  {}
  AAA_UINT16_RANGE() 
  {}
  ACE_UINT16 first;
  ACE_UINT16 last;
};

enum {
  AAA_IPFILTER_RULE_SRCDST_EXACT,
  AAA_IPFILTER_RULE_SRCDST_MASK,
  AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
  AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

class AAA_IPFILTER_RULE_SRCDST
{
 public:
  AAA_IPFILTER_RULE_SRCDST(AAA_UINT8 repr=AAA_IPFILTER_RULE_SRCDST_EXACT,
			   diameter_utf8string_t ipno=std::string(),
			   AAA_UINT8 bits=0,
			   bool mod=true) 
    : modifier(mod), representation(repr), ipno(ipno), bits(bits)
    {}
  bool modifier; /*! Modifier '!' maps to false */
  AAA_UINT8
  representation; /*! One of the following:
		    AAA_IPFILTER_RULE_SRCDST_EXACT,
		    AAA_IPFILTER_RULE_SRCDST_MASK,
		    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
		    AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSGINED. 

		    When representation is
		    AAA_IPFILTER_RULE_SRCDST_EXACT, only ipno is
		    used.  When representation is
		    AAA_IPFILTER_RULE_SRCDST_MASK, ipno and bits
		    are used.  For other represntations, both ipno and
		    bits are not used.
		  */

  diameter_utf8string_t ipno; 
  AAA_UINT8 bits;

  std::list<AAA_UINT16_RANGE> portRangeList;   /*! list of port ranges. */
};

enum {
  AAA_IPFILTER_RULE_IP_OPTION_SSRR=1,
  AAA_IPFILTER_RULE_IP_OPTION_LSRR,
  AAA_IPFILTER_RULE_IP_OPTION_RR,
  AAA_IPFILTER_RULE_IP_OPTION_TS
};

enum {
  AAA_IPFILTER_RULE_TCP_OPTION_MSS=1,
  AAA_IPFILTER_RULE_TCP_OPTION_WINDOW,
  AAA_IPFILTER_RULE_TCP_OPTION_SACK,
  AAA_IPFILTER_RULE_TCP_OPTION_TS,
  AAA_IPFILTER_RULE_TCP_OPTION_CC
};

enum {
  AAA_IPFILTER_RULE_TCP_FLAG_FIN=1,
  AAA_IPFILTER_RULE_TCP_FLAG_SYN, 
  AAA_IPFILTER_RULE_TCP_FLAG_RST, 
  AAA_IPFILTER_RULE_TCP_FLAG_PSH,
  AAA_IPFILTER_RULE_TCP_FLAG_ACK,
  AAA_IPFILTER_RULE_TCP_FLAG_URG
};

/*! IPFilterRule type. */
class diameter_ipfilter_rule_t 
{
  public:
  diameter_ipfilter_rule_t() : frag(false), established(false), setup(false)
  {}

  AAA_UINT8 action; /*! 
		      AAA_IPFILTER_RULE_ACTION_PERMIT or 
		      AAA_IPFILTER_RULE_ACTION_DENY 
		    */
  AAA_UINT8 dir; /*! 
		   AAA_IPFILTER_RULE_DIRECTION_IN or 
		   AAA_IPFILTER_RULE_DIRECTION_OUT 
		 */
  AAA_UINT8 proto; /*! 
		     The value 0 means wildcard number that matches
		     any protocol.
		   */
  AAA_IPFILTER_RULE_SRCDST src, dst;

  /// Option rules.
  bool frag; /* indicates fragmented packets except for the first
		fragment */
  std::list<int> ipOptionList; /*! IP Option list.  An entry with a
				 negative value means that the option
				 corresponding to its positive value
				 is negated. */

  std::list<int> tcpOptionList; /*! TCP Option list. An entry with a
				 negative value means that the option
				 corresponding to its positive value
				 is negated.  */
  bool established; /*! TCP packets only.  Match packets that have the
		      RST or ACK bits set. */
  bool setup; /*! TCP packets only.  Match packets that have the SYN
		bit set but no ACK bit */

  std::list<int> tcpFlagList; /*! TCP Flag list. An entry with a
				  negative value means that the flag
				  corresponding to its positive value
				  is negated.  */
  std::list<AAA_UINT8_RANGE> icmpTypeRangeList; /*! ICMP Type Range list */
} ;

enum {
  AAA_IPFILTER_RULE_ACTION_PERMIT,
  AAA_IPFILTER_RULE_ACTION_DENY 
};

enum {
  AAA_IPFILTER_RULE_DIRECTION_IN,
  AAA_IPFILTER_RULE_DIRECTION_OUT 
};

/*!
 * values possible for transport field of diameter_diamident_t 
 *
 * avp_t is a special type used only in this library 
 * for constructing a raw AVP.  When using this type, specify 
 * "AVP" as the avp_container type.
 * The string contains the entire AVP including AVP header.
 */
typedef diameter_octetstring_t avp_t;

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

/*!
 * Error types
 * NORMAL: Normal error defined in the Diameter specification.
 * BUG: Used when application programs misuse this API.
 */
enum  {
  NORMAL  = 0,   
  BUG = 1,       
};
  
/*!
 * The following error code is defined for error type "BUG"
 */
enum {
  MISSING_CONTAINER = 1,
  TOO_MUCH_AVP_ENTRIES,
  TOO_LESS_AVP_ENTRIES,
  PROHIBITED_CONTAINER,
  INVALID_CONTAINER_PARAM,
  INVALID_CONTAINER_CONTENTS,
  UNSUPPORTED_FUNCTIONALITY,
  INVALID_PARSER_USAGE,
  MISSING_AVP_DICTIONARY_ENTRY,
  MISSING_AVP_VALUE_PARSER
};

/*!
 * values possible for type field of diameter_address_t 
 */
typedef enum {
    AAA_ADDRESS_RESERVED = 0,  
    AAA_ADDRESS_IP,   // IP (IP version 4)
    AAA_ADDRESS_IP6,  // IP6 (IP version 6)
    AAA_ADDRESS_NSAP, // NSAP
    AAA_ADDRESS_HDLC, // (8-bit multidrop)
    AAA_ADDRESS_BBN,  // 1822
    AAA_ADDRESS_802,  // 802 (includes all 802 media plus Ethernet "canonical format")
    AAA_ADDRESS_E163, // E.163   
    AAA_ADDRESS_E164, // E.164 (SMDS, Frame Relay, ATM)
    AAA_ADDRESS_F69,  // F.69 (Telex)
    AAA_ADDRESS_X121, // (X.25, Frame Relay)
    AAA_ADDRESS_IPX,  // IPX
    AAA_ADDRESS_ATALK, // Appletalk
    AAA_ADDRESS_DECIV, // Decnet IV
    AAA_ADDRESS_BVINE, // Banyan Vines
    AAA_ADDRESS_E164N, // E.164 with NSAP format subaddress
    AAA_ADDRESS_DNS,   // DNS (Domain Name System)
    AAA_ADDRESS_DN,    // Distinguished Name
    AAA_ADDRESS_ASN,   // AS Number
    AAA_ADDRESS_XTPV4, // XTP over IP version 4 
    AAA_ADDRESS_XTPV6, // XTP over IP version 6
    AAA_ADDRESS_XTP,   // native mode XTP
    AAA_ADDRESS_FCP,   // Fibre Channel World-Wide Port Name
    AAA_ADDRESS_FCN,   // Fibre Channel World-Wide Node Name
    AAA_ADDRESS_GWID,  // GWID
    AAA_ADDRESS_RESERVED2 = 65535
} AAA_ADDRESS;

/*! \brief ErrorStatus Error Status
 *
 * parser functions throw this class instance when an error occurs.
 */
class DIAMETERPARSER_EXPORT AAAErrorStatus
{
   private:
      int type;     /**< error type (NORMAL or BUG) */

      int code;     /**< either a diameter result code or a bug_code above */

      std::string avp;   /**< errornous AVP */

   public:
      /*!
       * constructor
       */
      AAAErrorStatus(void) {
         type = NORMAL;
         code = AAA_SUCCESS;
      };

      /*!
       * Access function to retrieve all private data 
       *
       * \param type Error type
       * \param code Result or Bug code
       * \param avp AVP that had the error
       */
      void get(int &type, int &code, std::string &avp);

      /*!
       * Access function to retrieve some private data 
       *
       * \param type Error type
       * \param code Result or Bug code
       */
      void get(int &type, int &code);

      /*!
       * Access function to set some private data 
       *
       * \param type Error type to set
       * \param code Result or Bug code to set
       */
      void set(int type, int code);

      /*!
       * Access function to set all private data 
       *
       * \param type Error type to set
       * \param code Result or Bug code to set
       * \param data Data dictionary entry
       * \param codec Parser specific codec
       */
      void set(int type, int code, 
               AAADictionaryEntry* data,
               void *codec);
};

typedef ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> AAAMalloc;
typedef ACE_Allocator_Adapter<AAAMalloc> AAAAllocator;

#define AAA_MEMORY_MANAGER_NAME "AAA_Memory_Manager"

/*!
 * AAA Memory Manager
 *
 * This class provides heap based memory management.
 * Allocations are cumulative. Each call to malloc()
 * method will result in accumulations of memory 
 * chunks from the manager in to the allocator list.
 * A call to free will return the chunk into the
 * allocator free list but not the manager. 
 */
class DIAMETERPARSER_EXPORT AAAMemoryManager : public AAAAllocator
{
  friend class ACE_Singleton<AAAMemoryManager, ACE_Recursive_Thread_Mutex>; /**< memory manager */

 private:
  /*!
   * constructor
   */
  AAAMemoryManager() : AAAAllocator(AAA_MEMORY_MANAGER_NAME) {}
};

typedef ACE_Singleton<AAAMemoryManager, ACE_Recursive_Thread_Mutex> AAAMemoryManager_S;
DIAMETERPARSER_SINGLETON_DECLARE(ACE_Singleton, AAAMemoryManager, ACE_Recursive_Thread_Mutex);

/*! \brief ContainerEntry Container Entry
 *
 * A union structure to store a value of any type of AVP.  
 */
class AAAAvpContainerEntry
{
     friend class AAAAvpContainerEntryManager; /**< entry manager */
     friend class AAAAvpContainer; /**< container */

  public:
    /*!
     * Returns a the type of the entry
     */
    AAA_AVPDataType& dataType() { return this->type; }

    /*!
     *  Returns a type-specific reference to data.  The argument is a
     * dummy argument that is used by complier to uniquely chose a
     * desired template function.
     */
    template <class T> inline T& dataRef(Type2Type<T>)
      {
	return *((T*)data);
      }

    /*!
     *  Returns a type-specific pointer to data.  The argument is a
     * dummy argument that is used by complier to uniquely chose a
     * desired template function.
     */
    template <class T> inline T* dataPtr(Type2Type<T>)
      {
	return (T*)data;
      }

  protected:
    /*! Constuctor. */
    AAAAvpContainerEntry(int type) : type((AAA_AVPDataType)type) {}

    /*! Destructor */
    virtual ~AAAAvpContainerEntry() {}

    /*!
     * Abstracted new operator
     *
     * \param s size of block to allocate
     */
    void* operator new(size_t s) 
    { return AAAMemoryManager_S::instance()->malloc(s); }

    /*!
     * Abstracted delete operator
     *
     * \param p data block to free
     */
    void operator delete(void *p) 
    { AAAMemoryManager_S::instance()->free(p); }

    AAA_AVPDataType type; /**< AVP type */

    void*           data; /**< User-defined type data */
};


/*! Use this template for defining type-specific container entries.
*/
template <class T>
class AAATypeSpecificAvpContainerEntry 
: public AAAAvpContainerEntry
{
 public:
  /*!
   *  Constructor with assigning a specific type value.
   *
   * \param type AVP type
   */
  AAATypeSpecificAvpContainerEntry(int type) : AAAAvpContainerEntry(type) 
    { 
      data = (new(AAAMemoryManager_S::instance()->malloc(sizeof(T))) T);
    }

  /*!
   *  Destructor.
   */
  ~AAATypeSpecificAvpContainerEntry() 
    { 
      ((T*)data)->T::~T();
      AAAMemoryManager_S::instance()->free(data);
    }

  /*!
   * Returns a type-specific pointer to data.  
   */
  inline T* dataPtr() const { return (T*)data; }

  /*!
   * Returns a type-specific reference to data.  
   */
  inline T& dataRef() const { return *((T*)data); }

  /*!
   * Abstracted new operator
   *
   * \param s size of block to allocate
   */
  void* operator new(size_t s) 
  { return AAAMemoryManager_S::instance()->malloc(s); }

  /*!
   * Abstracted delete operator
   *
   * \param p data block to free
   */

  void operator delete(void *p) 
  { AAAMemoryManager_S::instance()->free(p); }

};


enum AAAAvpParseType {
  PARSE_TYPE_FIXED_HEAD = 0,
  PARSE_TYPE_FIXED_TAIL,
  PARSE_TYPE_REQUIRED, 
  PARSE_TYPE_OPTIONAL
};

/*!\brief AvpContainer AVP Container Definition
 *
 * A class used for passing AVP data between applications and the API.
 */
class DIAMETERPARSER_EXPORT AAAAvpContainer 
: public std::vector<AAAAvpContainerEntry*>
{
      friend class AAAAvpContainerList; /**< container list */

   public:
      /*!
       * constructor
       */
      AAAAvpContainer() : flag(false), parseType(PARSE_TYPE_OPTIONAL)
	 {}

      /*!
       * destructor
       */
      ~AAAAvpContainer() {}

      /*!
       * This function returns all the AAAAvpContainerEntry pointers in
       * the container to the free list.  It is the responsibility of
       * applications to call this function as soon as processing of the
       * containers are completed. 
       */
      inline void releaseEntries() 
	{ for (unsigned i=0; i<size(); i++) { delete (*this)[i]; }}

      /*!
       * This function adds a AAAAvpContainerEntry pointer to the
       * container. 
       *
       * \param e AVP entry to add
       */
      inline void add(AAAAvpContainerEntry* e) { resize(size()+1, e); }

      /*!
       * This function removes a AAAAvpContainerEntry pointer from the
       * container. 
       *
       * \param e AVP entry to remove
       */
      void remove(AAAAvpContainerEntry* e);

      /*!
       * Access method to retrive the AVP name.
       * Returns character string name of the AVP.
       */
      inline const char* getAvpName() const { return avpName.c_str(); }

      /*!
       * Access method to set the AVP name.
       *
       * \param name New AVP name
       */
      inline void setAvpName(const char* name) 
      { 
        avpName = std::string(name);
      }

      /*!
       * Abstracted new operator
       *
       * \param s size of block to allocate
       */
      void* operator new(size_t s) 
	{ return AAAMemoryManager_S::instance()->malloc(s); }

      /*!
       * Abstracted delete operator
       *
       * \param p data block to free
       */
      void operator delete(void *p) 
	{ AAAMemoryManager_S::instance()->free(p); }

      /*!
       * Returns a reference to the AVP type
       */
      inline AAAAvpParseType& ParseType() { return parseType; }

   protected:
      std::string avpName;        /**< AVP name */

      bool flag;  /**< Internal use */

      AAAAvpParseType parseType;  /**< Parse type (strict or loose) */
};


/*! \brief AvpContainerList AVP Container List
 *
 * A class that defines a list of AAAAvpContainer and functions 
 * for manipulating the list.
 */
class DIAMETERPARSER_EXPORT AAAAvpContainerList 
: public std::list<AAAAvpContainer*>
  
{
   public:
      /*!
       * constructor
       */
      AAAAvpContainerList() {}

      /*!
       * destructor
       */
      ~AAAAvpContainerList();

      /*!
       * This function appends the specified container to the internal list.
       *
       * \param c AVP Container to add
       */
      inline void add(AAAAvpContainer* c) { push_back(c); }

      /*!
       * This function prepends the specified container to the internal list.
       *
       * \param c AVP Container to prepend
       */
      inline void prepend(AAAAvpContainer* c) { push_front(c); }

      /*!
       * This function returns all the containers in the list to the free
       * list.  It is the responsibility of applications to call
       * this function as soon as processing of the containers are completed.
       */
      void releaseContainers();

      /*!
       * This function searches the internal list for a container
       * corresponding to the specified name.  
       *
       * \param name AVP name to search
       */
      AAAAvpContainer* search(const char* name);

      /*!
       * Abstracted new operator
       *
       * \param s size of block to allocate
       */
      void* operator new(size_t s) 
	{ return AAAMemoryManager_S::instance()->malloc(s); }

      /*!
       * Abstracted new operator
       *
       * \param s size of block to allocate (proxy only)
       * \param p pointer to existing block 
       */
      void* operator new(size_t s, void*p) 
	{ return p; }

      /*!
       * Abstracted delete operator
       *
       * \param p data block to free
       */
      void operator delete(void *p) 
	{ AAAMemoryManager_S::instance()->free(p); }

      void operator delete(void *p, void*q)
	{ AAAMemoryManager_S::instance()->free(p); }	

      AAAAvpContainer* search(AAADictionaryEntry*);/**< q_avplist parser */

      void reset(); /**< reset the container flags for re-parsing */
    
 private:
      AAAAvpContainer* search(const char*, bool);  /**< Internal use */
      AAAAvpContainer* search(bool);         /**< Internal use */
};

/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * AAAAvpContainerEntryManager and AAAAvpContainerManager is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */ 
class DIAMETERPARSER_EXPORT AAAAvpContainerEntryManager
{
   public:
      /*!
       * This function assigns a AAAAvpContainerEntry resource of a
       * specific type.
       *
       * \param type Data type to acquire
       */
      AAAAvpContainerEntry *acquire(AAA_AVPDataType type);

      /*!
       * This function release a AAAAvpContainerEntry resource.
       *
      * \param entry AVP Container entry
       */
      inline void release(AAAAvpContainerEntry* entry) { delete entry; }
};

/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * AAAAvpContainerEntryManager and AAAAvpContainerManager is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */ 
class DIAMETERPARSER_EXPORT AAAAvpContainerManager
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

/*! A class used as a handle for internal dictionary structures. */
class AAADictionaryHandle {};

/*! DictionaryManager Dictionary Manager Definition
 *
 * This class is used for managing dictionary
 */
class DIAMETERPARSER_EXPORT AAADictionaryManager
{
   public:
      /*!
       * Constructor
       *
       * \param proto Protocol 
       */
      AAADictionaryManager(int p=-1) : protocol(p) { }
    
      /*!
       * This function initializes the command and AVP dictionaries based
       * on an appropriate set of command and AVP dictionary files
       * described in XML format. 
       *
       * \param dictFile XML based Dictionary File
       */ 
      void init(char *dictFile);

      /*!
       * Used for translating the command name into a pair of Command
       * Code and Application Identifier.
       *
       * \param commandName Name of command code to retrieve
       * \param commandCode Command Code
       * \param appId Application Identifier
       */ 
      boolean_t getCommandCode(char *commandName,
			       AAACommandCode *commandCode,
			       AAAApplicationId *appId);

      /*!
       * Used for retrieving the dictionary handle for
       * a given command.
       *
       * \param code Command code
       * \param id application Id
       * \param rflag Request flag
       */
      AAADictionaryHandle *getDictHandle(AAACommandCode code, 
					 AAAApplicationId id,
                                         int rflag);

      /*!
       * Used for retrieving the dictionary handle for
       * a given command.
       *
       * \param cmdName Command name
       */
      AAADictionaryHandle *getDictHandle(char *cmdName);
    
   private:
      int protocol;
};

class AAAEmptyClass {};

/*! \brief Parser Message Parser Definition
 *
 * AAAParser is a template class for generic parser.
 */  
template <class RAWDATA, class APPDATA, 
          class DICTDATA = AAAEmptyClass,
          class CODEC = AAAEmptyClass>
class DIAMETERPARSER_EXPORT_ONLY AAAParser
{
   public:
      /*!
       * constructor
       */
      AAAParser() {}

      /*!
       * destructor
       */
      virtual ~AAAParser() {};
    
      /*!
       * Parse raw data and translate it into application level data. 
       */
      virtual void parseRawToApp();

      /*!
       * Parse application level data and translate it into raw data. 
       */
      virtual void parseAppToRaw();

      /*!
       * Set raw data to the parser.
       *
       * \param data Parser Data
       */
      void setRawData(RAWDATA data) { rawData = data; }

      /*!
       * Set application level data to the parser.
       *
       * \param data Parser Data
       */
      void setAppData(APPDATA data) { appData = data; }

      /*!
       * Set dictionary data to the parser.
       *
       * \param data Dictionary data
       */
      void setDictData(DICTDATA data) { dictData = data; }

      /*!
       * Get raw data from the parser.
       */
      RAWDATA getRawData() { return rawData; }

      /*! This template is used for obtaining raw data casted to a
       * specific type.
       */
      template <class T> void getRawData(T*& data) { data = (T*)rawData; }

      /*!
       * Get application level data from the parser.
       */
      APPDATA getAppData() { return appData; }

      /*! This template is used for obtaining application data
       *  casted to a * specific type.
       */
      template <class T> void getAppData(T*& data) { data = (T*)appData; }

      /*!
       * Get dictionary data from the parser.
       */
      DICTDATA getDictData() { return dictData; }

      /*! This template is used for obtaining dictionary data
       *  casted to a * specific type.
       */
      template <class T> void getDictData(T*& data) { data = (T*)dictData; }

      /*!
       * Set codec to the parser.
       *
       * \param codec New codec
       */
      void setCodec(CODEC c) { codec = c; }

      /*!
       * Get codec from the parser.
       */
      CODEC getCodec() { return codec; }

      /*! This template is used for obtaining the codec
       *  casted to a * specific type.
       */
      template <class T> void getCodec(T*& c) { codec = (T*)c; }
      
   private:
      RAWDATA rawData;   /**< Raw data  */

      APPDATA appData;   /**< Application data translated from/to raw data */

      DICTDATA dictData;  /**< Dictionary data */
      
      CODEC codec; /* Codec object */
};

/// Parser error enumration.
enum DiameterParserError {
  DictionaryError,
  HeaderError,
  PayloadError
};

/*! \brief MessageBuffer Message Buffer Definitions
 *
 * AAAMessageBlock is a class for containing raw data.
 * 
 * Offset contains the current pointer location relative to the 
 * address of data.  Offset is set only after "write" operation and 
 * not updated for "read" operation.
 */
class AAAMessageBlock: public ACE_Message_Block
{
   public:
      /*!
       * Acquire by reference.  The message block points to the specified
       * location with the size set to the specified value.
       *
       * \param buf User buffer passed to message block
       * \param s size of user buffer
       */
      static AAAMessageBlock* Acquire(char *buf, ACE_UINT32 s) { return new AAAMessageBlock(buf,s); }

      /*!
       * Acquire by allocation.  A new message block of specified size is
       * created.
       *
       * \param s size to be internally allocated
       */
      static AAAMessageBlock* Acquire(ACE_UINT32 s) { return new AAAMessageBlock(s); }

      /*!
       * Acquire by duplication.  Referece count of the original message
       * data is incremented by one.
       *
       * \param b original message
       */
      static AAAMessageBlock* Acquire(AAAMessageBlock* b) { return new AAAMessageBlock(b); }

      /*!
       * Release function.  Referece count of the message data is
       * decremented by one.  If the reference count becomes 0, the
       * entire message block is deleted.  It is definitely more
       * efficient to directly call release().
       */
      void Release() { release(); }

  protected:
      /*!
       * A message block is created with pointing to the buffer
       * location and having a specified size.
       *
       * \param buf User buffer passed to message block
       * \param s size of user buffer
       */
      AAAMessageBlock(char *buf, ACE_UINT32 s) { init(buf, s); }

      /*!
       * A message block consisting of null characters of the
       * specified size is created.
       *
       * \param s size to be internally allocated
       */
      AAAMessageBlock(ACE_UINT32 s) { init(s); }

      /*!
       * A shallow-copy constractor.  This constructor just duplicates
       * (ie. a shallow copy) the data block of the incoming Message
       * Block, since the incoming Message Block always has a data block
       * has a data block allocated from the heap.
       */
      AAAMessageBlock(AAAMessageBlock *b) : ACE_Message_Block((const ACE_Message_Block&)*b,0) {}

      /*!
       * Abstracted new operator
       *
       * \param s size of block to allocate (proxy only)
       */
      void* operator new(size_t s) 
      { return AAAMemoryManager_S::instance()->malloc(s); }

      /*!
       * Abstracted delete operator
       *
       * \param p data block to free
       */
      void operator delete(void *p) 
      { AAAMemoryManager_S::instance()->free(p); }

private:
      /*!
       * Destractor which is never used.
       */
      ~AAAMessageBlock() {}
};

/*! \brief Automated scoped release of message block
 *
 * Utility class that provides a scoped persistence
 * to an AAAMessageBlock object.
 */
class AAA_MessageBlockScope
{
    public:
       AAA_MessageBlockScope(ACE_Message_Block *&mb) : mb_(mb) { }
       ~AAA_MessageBlockScope(void) { mb_->release(); }

    protected:
       ACE_Message_Block *&mb_;
};

/*!
 * Parser options
 */
enum ParseOption {
  PARSE_LOOSE = 0,
  PARSE_STRICT = 1,
};

/*!
 * Dictionary data with protocol Id
 */
class AAADictionaryOption
{
public:
    AAADictionaryOption() {}
    AAADictionaryOption(ParseOption opt, int id) :
        option(opt), protocolId(id) {}
    
    ParseOption option;
    int protocolId;
};

class AAADiameterHeader;
class AAAMessage;
class AAAAvpHeader;

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, ParseOption>
HeaderParser;

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, AAADictionaryOption*>
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
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  payload.  When parsing fails, an error status is thrown.  When a
 *  non-null dictionary data is set (the dictionary handle can be
 *  obtained from the header instance as a result of successful header
 *  paring), validity check on command semantics (e.g.,
 *  fixed/required/optional AVP check, check for the minumum/maximum
 *  number of elements in each AVP) is performed (strict parsing).
 *  When a null dictionary data is set, validity check on command
 *  semantics are not performed (loose parsing). 
 */
template<> void DIAMETERPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  payload.  When parsing fails, an error status is thrown.  When a
 *  non-null dictionary data is set (the dictionary handle can be
 *  obtained from the header instance as a result of successful header
 *  paring), validity check on command semantics (e.g.,
 *  fixed/required/optional AVP check, check for the minumum/maximum
 *  number of elements in each AVP) is performed (strict parsing).
 *  When a null dictionary data is set, validity check on command
 *  semantics are not performed (loose parsing).
 */
template<> void DIAMETERPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseAppToRaw();// throw(AAAErrorStatus);

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

/*!
 * Header size definition
 */
#define HEADER_SIZE 20

/*! \brief DiameterHeader Diameter Message Header Definition
 *
 * A class used for storing Diameter header fields.
 */
class DIAMETERPARSER_EXPORT AAADiameterHeader
{
      friend class AAAParser<AAAMessageBlock*, AAADiameterHeader*, ParseOption>; /**< Parser friend */
      friend class AAAParser<AAAMessageBlock*, AAADiameterHeader*, AAADictionaryOption*>; /**< Parser friend */

   public:
      /*!
       * constructor
       *
       * \param ver Version
       * \param length Initial length
       * \param flags Header flag value
       * \param code Message command code
       * \param appId Diameter application ID
       * \param hh Hop-to-Hop ID
       * \param ee End-to-End ID
       */
      AAADiameterHeader(AAA_UINT8 ver, 
	  	        ACE_UINT32 length, 
		        struct hdr_flag flags, 
		        AAACommandCode code, 
		        AAAApplicationId appId, 
		        ACE_UINT32 hh, 
		        ACE_UINT32 ee) {
          this->ver = ver; 
          this->length = length; 
          this->flags = flags;
          this->code = code; 
          this->appId = appId; 
          this->hh = hh;
          this->ee = ee;
      }

      /*!
       * destructor 
       */
      AAADiameterHeader() {}

      /*!
       * returns the current dictionary handle
       */
      inline AAADictionaryHandle *getDictHandle() { return dictHandle; }

      /*!
       * returns the command name
       */
      const char* getCommandName();

      AAA_UINT8 ver; /**< Version */

      ACE_UINT32 length:24; /**< Message length (payload) */

      struct hdr_flag flags; /**< Header flags */

      AAACommandCode code:24; /**< Command code */

      AAAApplicationId appId; /**< Application ID */

      ACE_UINT32 hh; /**< Hop-to-Hop ID */

      ACE_UINT32 ee; /**< End-to-End ID */

 private:
      AAADictionaryHandle* dictHandle;
};

/*! \brief Message Diameter Message
 *
 * This class is re-defined as a replacement of the C-based AAAMessage
 * type definition.
 *
 * A class used for carrying AAAAvpContainerList and AAADiameterHeader 
 * class instances between applications and the API.
 */
class DIAMETERPARSER_EXPORT AAAMessage
{
   public:
      AAADiameterHeader hdr; /**< Message header */

      AAAAvpContainerList acl; /**< AVP container list */

      AAAErrorStatus status; /**< Error status */

      IP_ADDR originator; /**< Originator IP address  */

      IP_ADDR sender; /**< Sender IP address */

      time_t secondsTillExpire; /**< Expiration time of message */

      time_t startTime; /**< Time of transmission */
};

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
typedef boost::function0<AvpValueParser*> AvpValueParserFunctor;

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

/*! This class defines an AVP type.  An AVP type is defined as a set
 *  of typename, typevalue, size of the AVP payload and the function
 *  objects to create an AVP value parser and AVP container entry for
 *  the type.
 */
class DIAMETERPARSER_EXPORT AvpType {
 public:

  /*!
   * constructor
   *
   * \param name AVP name
   * \param type AVP type
   * \param size AVP data size
   * \param parserCreator parser creator object
   * \param containerEntryCreator entry creator object
   */
  AvpType(char* name, AAA_AVPDataType type, ACE_UINT32 size,
	  AvpValueParserFunctor parserCreator,
	  AvpContainerEntryFunctor containerEntryCreator) :
    name(name), type(type), size(size),
    parserCreator(parserCreator),
    containerEntryCreator(containerEntryCreator) {}

  /*!
   *  This function is used for obtaining the AVP type name.
   */
  char* getName(void) { return name; };

  /*!
   *  This function is used for obtaining the AVP type value.
   */
  AAA_AVPDataType getType(void) { return type; };

  /*!
   * This function is used for obtaining the minimum AVP payload size.
   */
  ACE_UINT32 getMinSize(void) { return size; };

  /*!
   *  This function is used for creating a type-specific AVP value
   * parser.
   */
  AvpValueParser* createParser() { return parserCreator(); }

  /*!
   *  This function is used for crating a type-specific container
   * entry.
   */
    AAAAvpContainerEntry* createContainerEntry(int type) 
    { return containerEntryCreator(type); }

 private:

    char *name; /**< name of the avp type */

    AAA_AVPDataType type; /**< enumerate type */

    ACE_UINT32 size;  /**< minimum size of this avp payload (0 means variable size) */

    AvpValueParserFunctor parserCreator; /**< The AVP parser creator. */

    AvpContainerEntryFunctor containerEntryCreator; /**< The AVP parser creator. */
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
class DIAMETERPARSER_EXPORT AvpTypeList_S : public std::list<AvpType*> 
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

/*! \brief UTF8 string format check.
 *  If the data contains a valid UTF8 string, it returns 0.
 *  Otherwise, it returns -1.
 */
class UTF8Checker
{
 public:
  /*!
   * constructor
   */
  UTF8Checker() {}

  /*! When nullCheck is true, the check will fail if a null octet is
   *  found in the data.
   */
  int operator()(const char *data, unsigned length, bool nullCheck=false)
  {
    unsigned i = 0;
    while (i<length)
      {
	unsigned char b = data[i++];
      
	// Check the first octet of the current UTF8 character.
	// Null check
	if (b == 0x00) { if (nullCheck) return -1; }
	// The first bit check.
	else if ((b >> 7) == 0x00) { continue; }  // 7-bit ASCII character.

	// If the first 7 bits are all '1's, this is not an UTF8 character.
	if ((b >> 1) == 0x76) { return -1; } // Out of UTF8 character range.

	b <<= 1;
	// Count the number of '1' of the first octet
	int count=0;
	for (count=0; count<5; count++) { if ((b && 0x40) == 0) break; }
	
	// The count value must be greater than 0.
	if (count==0) { return -1; } // Out of UTF8 character range.

	// Check remaining octet(s)
	for (int j=0; j<count; j++) { 
	  if (i >= length) { return -1; }
	  if ((data[i++] >> 6) != 0x02) { return -1; }
	}
      }
    return 0;
  }
};

/*!
 * Diameter logging facity derived directly from ACE
 */
class AAALogMsg : public ACE_Log_Msg
{
       friend class ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex>;    /**< ACE logger */

    private:
        /* 
	 * protected constructors/destructors to prevent derivation
	 */
        AAALogMsg() { }
        ~AAALogMsg() { }
};

typedef ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex> AAALogMsg_S;
DIAMETERPARSER_SINGLETON_DECLARE(ACE_Singleton, AAALogMsg, ACE_Recursive_Thread_Mutex);

#define AAA_LOG AAALogMsg_S::instance()->log

/// Scholar and vector data manipulation class.
template <typename T>
class AAA_ScholarAttribute
{
 public:
  AAA_ScholarAttribute() : isSet(false) {}
  AAA_ScholarAttribute(T &val) : value(val), isSet(true) {}
  virtual ~AAA_ScholarAttribute() {}
  inline void Clear() { isSet = false; }
  inline void Set(T val) { value=val; isSet=true; }
  virtual void CopyFrom(AAAAvpContainer &c) 
  { 
    value=c[0]->dataRef(Type2Type<T>()); isSet=true; 
  }
  void CopyTo(AAAAvpContainer &c, AAA_AVPDataType t) 
  {
    AAAAvpContainerEntryManager em;
    AAAAvpContainerEntry *e = em.acquire(t);
    e->dataRef(Type2Type<T>()) = value;
    c.add(e);
  }
  inline bool operator==(const T& v) const { return value == v; }
  inline T& operator()() { return value; }
  inline T& operator=(T v) { isSet = true; value=v; return value; }
  inline bool& IsSet() { return isSet; }

 protected:
  T value;
  bool isSet;
};

/// Grouped scholar and vector data manipulation class.
template <typename T>
class AAA_GroupedScholarAttribute : public AAA_ScholarAttribute<T>
{
public:
  void CopyFrom(AAAAvpContainer &c)
  {
      AAAAvpContainerList& cl = 
	c[0]->dataRef(Type2Type<AAAAvpContainerList>());
      AAA_ScholarAttribute<T>::value.CopyFrom(cl);
      AAA_ScholarAttribute<T>::isSet = true;
  }
  void CopyTo(AAAAvpContainer &c)
  {
    AAAAvpContainerEntryManager em;
    AAAAvpContainerEntry *e = em.acquire(AAA_AVP_GROUPED_TYPE);
    AAA_ScholarAttribute<T>::value.CopyTo
          (e->dataRef(Type2Type<AAAAvpContainerList>()));
    c.add(e);
  }
  inline T& operator=(T v) 
  { 
    AAA_ScholarAttribute<T>::isSet = true; 
    AAA_ScholarAttribute<T>::value=v; 
    return AAA_ScholarAttribute<T>::value; 
  }
};

template <typename T>
class AAA_VectorAttribute : public std::vector<T>
{
 public:
  AAA_VectorAttribute() : isSet(false) {}
  virtual ~AAA_VectorAttribute() {}
  inline void Clear() { isSet = false; std::vector<T>::clear(); }
  inline std::vector<T>& operator()() { return *this; }
  inline std::vector<T>& operator=(std::vector<T>& value) 
  { 
    isSet = true; 
    (std::vector<T>&)(*this)=value; 
	return *this; 
  }
  virtual void CopyFrom(AAAAvpContainer &c) 
  { 
    isSet = true; 
    if (std::vector<T>::size() < c.size())
      std::vector<T>::resize(c.size());
    for (unsigned i=0; i<c.size(); i++)
      (*this)[i] = c[i]->dataRef(Type2Type<T>());
  }
  void CopyTo(AAAAvpContainer &c, AAA_AVPDataType t) 
  {
    AAAAvpContainerEntryManager em;
    AAAAvpContainerEntry *e = em.acquire(t);
    for (unsigned i=0; i<std::vector<T>::size(); i++)
      {
	e = em.acquire(t);
	e->dataRef(Type2Type<T>()) = (*this)[i];
	c.add(e);
      }
  }
  inline bool& IsSet() { return isSet; }

protected:
  bool isSet;
};

template <typename T>
class AAA_GroupedVectorAttribute : public AAA_VectorAttribute<T>
{
public:
  void CopyFrom(AAAAvpContainer &c)
  {
    AAA_VectorAttribute<T>::isSet = true; 
    if (AAA_VectorAttribute<T>::size() < c.size())
      std::vector<T>::resize(c.size());
    for (unsigned i=0; i<c.size(); i++)
      {
	AAAAvpContainerList& cl = 
	  c[i]->dataRef(Type2Type<AAAAvpContainerList>());
	(*this)[i].CopyFrom(cl);
	AAA_VectorAttribute<T>::isSet = true;
      }
  }
  void CopyTo(AAAAvpContainer &c)
  {
    AAAAvpContainerEntryManager em;
    AAAAvpContainerEntry *e;
    for (unsigned i=0; i<AAA_VectorAttribute<T>::size(); i++)
      {
	e = em.acquire(AAA_AVP_GROUPED_TYPE);
	(*this)[i].CopyTo(e->dataRef(Type2Type<AAAAvpContainerList>()));
	e->dataRef(Type2Type<T>()) = (*this)[i];
	c.add(e);
      }
  }
  inline AAA_GroupedVectorAttribute<T> &operator=
      (AAA_GroupedVectorAttribute<T>& value)
  {    
     AAA_VectorAttribute<T>::isSet = true;
     (std::vector<T>&)(*this)=value; 
     return *this;
  }

};

/*! \brief Generic AVP widget allocator
 */
template<class D, AAA_AVPDataType t>
class AAA_AvpWidget {
    public:
        AAA_AvpWidget(char *name) {
            AAAAvpContainerManager cm;
            m_cAvp = cm.acquire(name);
        }
        AAA_AvpWidget(char *name, D &value) {
            AAAAvpContainerManager cm;
            m_cAvp = cm.acquire(name);
            Get() = value;
        }
        AAA_AvpWidget(AAAAvpContainer *avp) :
            m_cAvp(avp) {
        }
        ~AAA_AvpWidget() {
        }
        D &Get() {
            AAAAvpContainerEntryManager em;
            AAAAvpContainerEntry *e = em.acquire(t);
            m_cAvp->add(e);            
            return e->dataRef(Type2Type<D>());
        }
        AAAAvpContainer *operator()() {
            return m_cAvp;
        }
        bool empty() {
            return (m_cAvp->size() == 0);
        }
    private:
        AAAAvpContainer *m_cAvp;
};

typedef AAA_AvpWidget<diameter_identity_t,
                      AAA_AVP_DIAMID_TYPE> AAA_IdentityAvpWidget;
typedef AAA_AvpWidget<diameter_address_t,
                      AAA_AVP_ADDRESS_TYPE> AAA_AddressAvpWidget;
typedef AAA_AvpWidget<diameter_integer32_t,
                      AAA_AVP_INTEGER32_TYPE> AAA_Int32AvpWidget;
typedef AAA_AvpWidget<diameter_unsigned32_t,
                      AAA_AVP_UINTEGER32_TYPE> AAA_UInt32AvpWidget;
typedef AAA_AvpWidget<diameter_integer64_t,
                      AAA_AVP_INTEGER64_TYPE> AAA_Int64AvpWidget;
typedef AAA_AvpWidget<diameter_unsigned64_t,
                      AAA_AVP_UINTEGER64_TYPE> AAA_UInt64AvpWidget;
typedef AAA_AvpWidget<diameter_utf8string_t,
                     AAA_AVP_UTF8_STRING_TYPE> AAA_Utf8AvpWidget;
typedef AAA_AvpWidget<diameter_grouped_t,
                     AAA_AVP_GROUPED_TYPE> AAA_GroupedAvpWidget;
typedef AAA_AvpWidget<diameter_octetstring_t,
                     AAA_AVP_STRING_TYPE> AAA_StringAvpWidget;
typedef AAA_AvpWidget<diameter_uri_t,
                     AAA_AVP_DIAMURI_TYPE> AAA_DiamUriAvpWidget;
typedef AAA_AvpWidget<diameter_enumerated_t,
                     AAA_AVP_ENUM_TYPE> AAA_EnumAvpWidget;
typedef AAA_AvpWidget<diameter_time_t,
                     AAA_AVP_TIME_TYPE> AAA_TimeAvpWidget;

/*! \brief Generic AVP widget lookup and parser
 * Assist in adding, deleting and modifying AVP's
 * contained in a message list.
 */
template<class D, AAA_AVPDataType t>
class AAA_AvpContainerWidget
{
    public:
       AAA_AvpContainerWidget(AAAAvpContainerList &lst) :
           m_List(lst) {
       }
       D *GetAvp(char *name, unsigned int index=0) {
          AAAAvpContainer* c = m_List.search(name);
          if (c && (index < c->size())) {
              AAAAvpContainerEntry *e = (*c)[index];
              return e->dataPtr(Type2Type<D>());
          }
          return (0);
       }
       D &AddAvp(char *name, bool append = false) {
          AAAAvpContainer* c = m_List.search(name);
          if (! c) {
              AAA_AvpWidget<D, t> avpWidget(name);
              m_List.add(avpWidget());
              return avpWidget.Get();
          }
          else if ((c->size() == 0) || append) {
              AAA_AvpWidget<D, t> avpWidget(c);
              return avpWidget.Get();
          }
          else {
              return (*c)[0]->dataRef(Type2Type<D>());
          }
       }
       void AddAvp(AAA_AvpContainerWidget<D, t> &avp) {
           m_List.add(avp());
       }
       void DelAvp(char *name) {
          std::list<AAAAvpContainer*>::iterator i;
          for (i=m_List.begin(); i!=m_List.end();i++) {
              AAAAvpContainer *c = *i;
              if (ACE_OS::strcmp(c->getAvpName(), name) == 0) {
                  m_List.erase(i);
                  AAAAvpContainerManager cm;
                  cm.release(c);
                  break;
              }
          }
       }
       unsigned int GetAvpCount(char *name) {
          AAAAvpContainer* c = m_List.search(name);
          return (c) ? c->size() : 0;
       }

    private:
       AAAAvpContainerList &m_List;
};

typedef AAA_AvpContainerWidget<diameter_identity_t, AAA_AVP_DIAMID_TYPE>
           AAA_IdentityAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_address_t, AAA_AVP_ADDRESS_TYPE>
           AAA_AddressAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_integer32_t, AAA_AVP_INTEGER32_TYPE>
           AAA_Int32AvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_unsigned32_t, AAA_AVP_UINTEGER32_TYPE>
           AAA_UInt32AvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_integer64_t, AAA_AVP_INTEGER64_TYPE>
           AAA_Int64AvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_unsigned64_t, AAA_AVP_UINTEGER64_TYPE>
           AAA_UInt64AvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_utf8string_t, AAA_AVP_UTF8_STRING_TYPE>
           AAA_Utf8AvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_grouped_t, AAA_AVP_GROUPED_TYPE>
           AAA_GroupedAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_octetstring_t, AAA_AVP_STRING_TYPE>
           AAA_StringAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_uri_t, AAA_AVP_DIAMURI_TYPE>
           AAA_DiamUriAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_enumerated_t, AAA_AVP_ENUM_TYPE>
           AAA_EnumAvpContainerWidget;
typedef AAA_AvpContainerWidget<diameter_time_t, AAA_AVP_TIME_TYPE>
           AAA_TimeAvpContainerWidget;

/*! \brief Generic Result code AVP checker
 * Assist in setting and getting any Result-Code
 * AVP's present in a message list
 */
class AAA_MsgResultCode
{
    public:
        typedef enum {
            RCODE_NOT_PRESENT,
            RCODE_INFORMATIONAL,
            RCODE_SUCCESS,
            RCODE_PROTOCOL_ERROR,
            RCODE_TRANSIENT_FAILURE,
            RCODE_PERMANENT_FAILURE
       } RCODE;
    
    public:
       AAA_MsgResultCode(AAAMessage &msg) :
           m_Msg(msg) {
       }
       diameter_unsigned32_t ResultCode() {
           AAA_UInt32AvpContainerWidget resultCode(m_Msg.acl);
           diameter_unsigned32_t *rc = resultCode.GetAvp("Result-Code");
           return (rc) ? *rc : 0;
       }
       void ResultCode(diameter_unsigned32_t c) {
           AAA_UInt32AvpContainerWidget resultCode(m_Msg.acl);
           resultCode.AddAvp("Result-Code") = c;
       }
       static RCODE InterpretedResultCode(diameter_unsigned32_t code) {
           for (int i=RCODE_INFORMATIONAL; i<=RCODE_PERMANENT_FAILURE;
               i++) {
               code -= 1000;
               if ((code/1000) == 0) {
                  return RCODE(i);
              }
           }
           return (RCODE_PERMANENT_FAILURE);
       }
       RCODE InterpretedResultCode() {
           diameter_unsigned32_t code = ResultCode();
           return (code) ? InterpretedResultCode(code) :
                           RCODE_NOT_PRESENT;
       }
    
    private:
       AAAMessage &m_Msg;
};

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
class AAA_MsgWidget
{
    public:
        AAA_MsgWidget(int code,
                      int request = true,
                      int appId = AAA_BASE_APPLICATION_ID) :
           m_Msg(std::auto_ptr<AAAMessage>(new AAAMessage)) {
           ACE_OS::memset(&(m_Msg->hdr), 0, sizeof(m_Msg->hdr));
           m_Msg->hdr.ver = AAA_PROTOCOL_VERSION;
           m_Msg->hdr.length = 0;
           m_Msg->hdr.flags.r = request ? AAA_FLG_SET : AAA_FLG_CLR;
           m_Msg->hdr.flags.p = AAA_FLG_CLR;
           m_Msg->hdr.flags.e = AAA_FLG_CLR;
           m_Msg->hdr.flags.t = AAA_FLG_CLR;
           m_Msg->hdr.code = code;
           m_Msg->hdr.appId = appId;
        }
        ~AAA_MsgWidget() {
        }
        std::auto_ptr<AAAMessage> &operator()() {
           return m_Msg;
        }
        AAAMessage &Release() {
           return *(m_Msg.release());
        }
        void Dump() {
           if (m_Msg.get()) {
               AAA_MsgDump::Dump(*m_Msg);
               return;
           }
           AAA_LOG(LM_INFO, "Msg widget is un-assigned\n");
        }
    private:
        std::auto_ptr<AAAMessage> m_Msg;
};

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_DiameterMsgParserWidget
{
public:
    virtual int ParseRawToApp(AAA_MsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
       HeaderParser hp;
       hp.setRawData(&rawBuf);
       hp.setAppData(&msg()->hdr);
       hp.setDictData((ParseOption)option);
       hp.parseRawToApp();
       rawBuf.size(msg()->hdr.length);

       PayloadParser pp;
       pp.setRawData(&rawBuf);
       pp.setAppData(&msg()->acl);
       pp.setDictData(msg()->hdr.getDictHandle());
       pp.parseRawToApp();
       return (msg()->hdr.length);
    }
    virtual int ParseAppToRaw(AAA_MsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
       HeaderParser hp;
       hp.setRawData(&rawBuf);
       hp.setAppData(&msg()->hdr);
       hp.setDictData((ParseOption)option);
       hp.parseAppToRaw();

       PayloadParser pp;
       pp.setRawData(&rawBuf);
       pp.setAppData(&msg()->acl);
       pp.setDictData(msg()->hdr.getDictHandle());
       pp.parseAppToRaw();

       msg()->hdr.length = rawBuf.wr_ptr() - rawBuf.base();
       rawBuf.wr_ptr(rawBuf.base());
       hp.parseAppToRaw();
       return (msg()->hdr.length);
    }
    virtual ~AAA_DiameterMsgParserWidget() {
    }
};

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_DiameterMsgParserWidgetChecked : 
    public AAA_DiameterMsgParserWidget
{
public:
    virtual int ParseRawToApp(AAA_MsgWidget &msg,
                              void *buf,
                              int bufSize,
                              int option) {
        AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire
            ((char*)buf, bufSize);
        ACE_Message_Block *aBlock = aBuffer;
        AAA_MessageBlockScope block_guard(aBlock); 
        return ParseRawToApp(msg, *aBuffer, option);
    }
    virtual int ParseRawToApp(AAA_MsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
        try {
            return AAA_DiameterMsgParserWidget::ParseRawToApp
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
    virtual int ParseAppToRaw(AAA_MsgWidget &msg,
                              void *buf,
                              int bufSize,
                              int option) {
        AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire
            ((char*)buf, bufSize);
        ACE_Message_Block *aBlock = aBuffer;
        AAA_MessageBlockScope block_guard(aBlock); 
        return ParseAppToRaw(msg, *aBuffer, option);
    }
    virtual int ParseAppToRaw(AAA_MsgWidget &msg,
                              AAAMessageBlock& rawBuf,
                              int option) {
        try {
            return AAA_DiameterMsgParserWidget::ParseAppToRaw
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



