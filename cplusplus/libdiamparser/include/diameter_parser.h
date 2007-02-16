/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2006 Open Diameter Project                          */
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

#ifndef __DIAMETER_PARSER_H__
#define __DIAMETER_PARSER_H__

#include "aaa_parser_api.h"

/*!
 *==================================================
 * Macros and Preprocessor Definitions
 *==================================================
 */

/*!
 * Open diameter version declaration
 */
#define DIAMETER_VERSION_MAJOR  0x01
#define DIAMETER_VERSION_MINOR  0x00
#define DIAMETER_VERSION_MICRO  0x05

/*!
 * Diameter default definitions
 */
#define DIAMETER_NO_VENDOR_ID              0x00
#define DIAMETER_BASE_APPLICATION_ID       0x00

/*!
 * Diameter flag definitions
 */
#define DIAMETER_PROTOCOL_VERSION          0x1
#define DIAMETER_FLAG_SET                  0x1
#define DIAMETER_FLAG_CLR                  0x0

/*!
 *==================================================
 * The following definitions are for diameter specific
 * parsing support. Some basic data types have been
 * derived from the generic aaa_parser_api
 *==================================================
 */

/*!
 * DiameterApplicationId identifies a particular client session to the API.
 * The application id is passed to AAAStartSession(), and is attached to
 * incoming messages, to indicate with which client session the message
 * is associated.
 */
typedef ACE_UINT32     DiameterApplicationId;

/*!
 * The result codes are values returned from remote servers as part
 * of messages. They correspond directly to the result codes in the
 * Diameter specification [1]:
 */
typedef ACE_UINT32     DiameterResultCode;

/*!
 *==================================================
 * Pre-defined enumration
 *==================================================
 */

/*!
 * The AVP flags defines the flags set in the AVP header.
 * They correspond directly to the avp flags defined in the
 * Diameter specification [1]:
 */
typedef enum {
    DIAMETER_AVP_FLAG_NONE                 =   0x0,
    DIAMETER_AVP_FLAG_MANDATORY            =   DIAMETER_FLAG_SET,
    DIAMETER_AVP_FLAG_RESERVED             =   0x2,
    DIAMETER_AVP_FLAG_VENDOR_SPECIFIC      =   0x4,
    DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT   =   0x10,
    DIAMETER_AVP_FLAG_UNKNOWN              =   0x10000,
    DIAMETER_AVP_FLAG_ENCRYPT              =   0x40000,
} DiameterAvpFlagEnum;

/*!
 * values possible for transport field of diameter_uri_t
 */
enum {
    DIAMETER_TRANSPORT_PROTO_TCP           = 0,
    DIAMETER_TRANSPORT_PROTO_SCTP,
    DIAMETER_TRANSPORT_PROTO_UDP,
};

/*!
 * values possible for protocol field of diameter_uri_t
 */
enum {
    DIAMETER_PROTO_DIAMETER = 0,
    DIAMETER_PROTO_RADIUS,
    DIAMETER_PROTO_TACACSPLUS,
};

/*!
 * values possible for scheme field of diameter_uri_t
 */
enum {
    DIAMETER_SCHEME_AAA = 0,
    DIAMETER_SCHEME_AAAS
};

/*!
 * Parser options
 */
enum DiameterParseOption {
    DIAMETER_PARSE_LOOSE     = 0,
    DIAMETER_PARSE_STRICT    = 1,
};

/*!
 * Parser error enumration.
 */
enum DiameterParserError {
    DIAMETER_DICTIONARY_ERROR,
    DIAMETER_HEADER_ERROR,
    DIAMETER_PAYLOAD_ERROR
};

/*!
 *==================================================
 * The following definitions are for diameter specific
 * data type definitions for storing parsed data.
 *==================================================
 */

/*!
 * Data type definitions for AAA Parser
 */
typedef ACE_INT32                  diameter_integer32_t;

typedef ACE_UINT64                 diameter_integer64_t;

typedef ACE_UINT32                 diameter_unsigned32_t;

typedef ACE_UINT64                 diameter_unsigned64_t;

typedef diameter_unsigned32_t      diameter_enumerated_t;

typedef diameter_unsigned32_t      diameter_time_t;

typedef std::string                diameter_octetstring_t;

typedef diameter_octetstring_t     diameter_utf8string_t;

typedef diameter_utf8string_t      diameter_identity_t;

typedef class AAAAvpContainerList  diameter_grouped_t;

typedef diameter_unsigned32_t      diameter_time_t;  // Number of sec relative to 01/01/1900

/*!
 * values possible for transport field of diameter_diamident_t
 *
 * avp_t is a special type used only in this library
 * for constructing a raw AVP.  When using this type, specify
 * "AVP" as the avp_container type.
 * The string contains the entire AVP including AVP header.
 */
typedef diameter_octetstring_t     diameter_avp_t;

typedef struct {
    public:
        std::string    fqdn;
        ACE_UINT16     port;
        AAAInt8        transport:2;
        AAAInt8        protocol:2;
        AAAInt8        scheme:2;
} diameter_uri_t;

typedef struct
{
    public:
        ACE_UINT16               type;
        diameter_octetstring_t   value;
} diameter_address_t;

/*!
 *==================================================
 * The following are definitions for IP filter rule AVP
 *==================================================
 */
enum {
    DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
    DIAMETER_IPFILTER_RULE_SRCDST_MASK,
    DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
    DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

class DiameterIPFilterRuleSrcDst
{
    public:
        DiameterIPFilterRuleSrcDst(AAAUInt8 repr=DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
                                   diameter_utf8string_t ipno=std::string(),
                                   AAAUInt8 bits=0,
                                   bool mod=true) :
            modifier(mod),
            representation(repr),
            ipno(ipno),
            bits(bits) {
        }

    public:
        bool         modifier;        /*! Modifier '!' maps to false */
        AAAUInt8     representation;  /*! One of the following:
                                          DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
                                          DIAMETER_IPFILTER_RULE_SRCDST_MASK,
                                          DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
                                          DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSGINED.

                                          When representation is
                                          DIAMETER_IPFILTER_RULE_SRCDST_EXACT, only ipno is
                                          used.  When representation is
                                          DIAMETER_IPFILTER_RULE_SRCDST_MASK, ipno and bits
                                          are used.  For other represntations, both ipno and
                                          bits are not used.
                                        */

        diameter_utf8string_t  ipno;
        AAAUInt8               bits;
        std::list<AAAUInt16Range> portRangeList;   /*! list of port ranges. */
};

enum {
    DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR    =   1,
    DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR,
    DIAMETER_IPFILTER_RULE_IP_OPTION_RR,
    DIAMETER_IPFILTER_RULE_IP_OPTION_TS
};

enum {
    DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS    =   1,
    DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW,
    DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK,
    DIAMETER_IPFILTER_RULE_TCP_OPTION_TS,
    DIAMETER_IPFILTER_RULE_TCP_OPTION_CC
};

enum {
    DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN=1,
    DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN,
    DIAMETER_IPFILTER_RULE_TCP_FLAG_RST,
    DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH,
    DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK,
    DIAMETER_IPFILTER_RULE_TCP_FLAG_URG
};

/*! IPFilterRule type. */
class diameter_ipfilter_rule_t {
    public:
        diameter_ipfilter_rule_t() :
            frag(false),
            established(false),
            setup(false) {
        }

        AAAUInt8   action; /*! DIAMETER_IPFILTER_RULEACTION_PERMIT or
                             *  DIAMETER_IPFILTER_RULEACTION_DENY
                             */
        AAAUInt8   dir;    /*!
                             *  DIAMETER_IPFILTER_RULEDIRECTION_IN or
                             *  DIAMETER_IPFILTER_RULEDIRECTION_OUT
                             */
        AAAUInt8   proto;  /*!
                             *  The value 0 means wildcard number that matches
                             *  any protocol
                             */

        DiameterIPFilterRuleSrcDst src;
        DiameterIPFilterRuleSrcDst dst;

        /// Option rules.
        bool frag;          /*! indicates fragmented packets except for the first
                             *  fragment
                             */
        std::list<int> ipOptionList;  /*! IP Option list.  An entry with a
                                       *  negative value means that the option
                                       *  corresponding to its positive value
                                       *  is negated.
                                       */
        std::list<int> tcpOptionList; /*! TCP Option list. An entry with a
                                       * negative value means that the option
                                       * corresponding to its positive value
                                       * is negated.
                                       */
        bool           established;   /*! TCP packets only.  Match packets that have the
                                       * RST or ACK bits set.
                                       */
        bool           setup;         /*! TCP packets only.  Match packets that have the SYN
                                       * bit set but no ACK bit 
                                       */
        std::list<int> tcpFlagList;   /*! TCP Flag list. An entry with a
                                       *  negative value means that the flag
                                       *  corresponding to its positive value
                                       *  is negated.
                                       */
        std::list<AAAUInt8Range> icmpTypeRangeList; /*! ICMP Type Range list */
} ;

enum {
    DIAMETER_IPFILTER_RULE_ACTION_PERMIT,
    DIAMETER_IPFILTER_RULE_ACTION_DENY
};

enum {
    DIAMETER_IPFILTER_RULE_DIRECTION_IN,
    DIAMETER_IPFILTER_RULE_DIRECTION_OUT
};

/*!
 *==================================================
 * The following are definitions for diameter
 * specific dictionary loading
 *==================================================
 */

/*! DictionaryManager Dictionary Manager Definition
 *
 * This class is used for managing dictionary
 */
class AAA_PARSER_EXPORT DiameterDictionaryManager
{
    public:
        /*!
        * Constructor
        */
        DiameterDictionaryManager() {
        }

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
        bool getCommandCode(char *commandName,
                            AAACommandCode *commandCode,
                            DiameterApplicationId *appId);

        /*!
        * Used for retrieving the dictionary handle for
        * a given command.
        *
        * \param code Command code
        * \param id application Id
        * \param rflag Request flag
        */
        AAADictionaryHandle *getDictHandle(AAACommandCode code,
                                            DiameterApplicationId id,
                                            int rflag);

        /*!
        * Used for retrieving the dictionary handle for
        * a given command.
        *
        * \param cmdName Command name
        */
        AAADictionaryHandle *getDictHandle(char *cmdName);
};

/*!
 *==================================================
 * The following are definitions for diameter
 * specific error codes including failed-avp
 * determination
 *==================================================
 */

/*! \brief DiameterErrorCode Diameter Specific Error Code
 *
 * parser functions throw this class instance when an error occurs.
 */
class AAA_PARSER_EXPORT_ONLY DiameterErrorCode :
   public AAAErrorCode
{
   public:
      /*!
       * constructor
       */
      DiameterErrorCode(void) {
      };

      /*!
       * Access function to retrieve all private data 
       *
       * \param type Error type
       * \param code Result or Bug code
       * \param avp AVP that had the error
       */
      void get(AAA_PARSE_ERROR_TYPE &type,
               int &code,
               std::string &avp) {
          AAAErrorCode::get(type, code);
          avp = this->avp;
      }

      /*!
       * Access function to set all private data 
       *
       * \param type Error type to set
       * \param code Result or Bug code to set
       * \param data Data dictionary entry
       */
      void set(AAA_PARSE_ERROR_TYPE type,
               int code,
               AAADictionaryEntry* data);

      /*!
       * Access function to retrieve some private data.
       * Overlaod AAAErrorCode::get(..) to fix function
       * hiding.
       *
       * \param type Error type
       * \param code Result or Bug code
       */
      void get(AAA_PARSE_ERROR_TYPE &t, int &c) {
          AAAErrorCode::get(t, c);
      }

      /*!
       * Access function to set some private data 
       * Overlaod AAAErrorCode::set(..) to fix function
       * hiding.
       *
       * \param type Error type to set
       * \param code Result or Bug code to set
       */
      void set(AAA_PARSE_ERROR_TYPE t, int c) {
          AAAErrorCode::set(t, c);
      }

      /*!
       * !!! Backward compatibility only !!!
       *
       * \param type Error type
       * \param code Result or Bug code
       * \param avp AVP that had the error
       */
      void get(int &type,
               int &code,
               std::string &avp) {
          get((AAA_PARSE_ERROR_TYPE&)type, code, avp);
      }

      /*!
       * !!! Backward compatibility only !!!
       *
       * \param type Error type
       * \param code Result or Bug code
       */
      void get(int &t, int &c) {
          get((AAA_PARSE_ERROR_TYPE&)t, c);
      }

   private:
      std::string avp;   /**< errornous AVP */
};

/*!
 *==================================================
 * This section defines the diameter message structure
 *
 * A diameter message is represented by a DiameterMessage
 * object. This inherits from AAAAvpContainerList which
 * contains one or more AVPs composing the messages
 *
 *==================================================
 */

/*!
 * AVP header flags
 */
struct diameter_avp_flag {
    AAAUInt8    v;        /**< Vendor flag */
    AAAUInt8    m;        /**< Mandatory flag */
    AAAUInt8    p;        /**< end-to-end security flag */
};

/*!
 * AVP header
 */
class DiameterAvpHeader
{
    public:
        DiameterAvpHeader() :
            code(0),
            length(0),
            vendor(0),
            value_p(0),
            parseType(AAA_PARSE_TYPE_OPTIONAL) {
            memset(&flag, 0, sizeof(flag));
        }
        inline AAAAvpParseType& ParseType() {
            return parseType;
        }

    public:
        ACE_UINT32                 code;         /**< AVP code */
        struct diameter_avp_flag   flag;         /**< AVP flags */
        ACE_UINT32                 length  :24;  /**< AVP length */
        ACE_UINT32                 vendor;       /**< Vendor code */
        char*                      value_p;      /**< Value */

    private:
        AAAAvpParseType            parseType;
}; 

/*!
 * Header bit field definition
 */
struct diameter_hdr_flag {
    AAAUInt8    r    :1;  /**< Request */
    AAAUInt8    p    :1;  /**< Proxiable */
    AAAUInt8    e    :1;  /**< Error */
    AAAUInt8    t    :1;  /**< Potentially re-transmitted */
    AAAUInt8    rsvd :4;  /**< Reserved */
};

/*!
 * Header size definition
 */
#define DIAMETER_HEADER_SIZE 20

/*! \brief DiameterHeader Diameter Message Header Definition
 *
 * A class used for storing Diameter header fields.
 */
class AAA_PARSER_EXPORT DiameterMsgHeader
{
        friend class AAAParser<AAAMessageBlock*,
                               DiameterMsgHeader*,
                               DiameterParseOption>; /**< Parser friend */

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
        DiameterMsgHeader(AAAUInt8 ver,
                          ACE_UINT32 length,
                          struct diameter_hdr_flag flags,
                          AAACommandCode code,
                          DiameterApplicationId appId,
                          ACE_UINT32 hh,
                          ACE_UINT32 ee) {
            this->ver = ver;
            this->length = length;
            this->flags = flags;
            this->code = code;
            this->appId = appId;
            this->hh = hh;
            this->ee = ee;
            this->dictHandle = 0;
        }

        /*!
        * destructor
        */
        DiameterMsgHeader() {
            this->ver = 0;
            this->length = 0;
            this->code = 0;
            this->appId = 0;
            this->hh = 0;
            this->ee = 0;
            this->dictHandle = 0;
        }

        /*!
        * returns the current dictionary handle
        */
        inline AAADictionaryHandle *getDictHandle() { 
            return dictHandle;
        }

        /*!
         * copy operator
         */
        DiameterMsgHeader &operator=(DiameterMsgHeader &hdr) {
            this->ver = hdr.ver;
            this->length = hdr.length;
            this->flags = hdr.flags;
            this->code = hdr.code;
            this->appId = hdr.appId;
            this->hh = hdr.hh;
            this->ee = hdr.ee;
            this->dictHandle = hdr.getDictHandle();
            return *this;
        }

        /*!
        * returns the command name
        */
        const char* getCommandName();

    public:
        AAAUInt8                   ver;         /**< Version */
        ACE_UINT32                 length:24;   /**< Message length (payload) */
        struct diameter_hdr_flag   flags;       /**< Header flags */
        AAACommandCode             code:24;     /**< Command code */
        DiameterApplicationId      appId;       /**< Application ID */
        ACE_UINT32                 hh;          /**< Hop-to-Hop ID */
        ACE_UINT32                 ee;          /**< End-to-End ID */

    private:
        AAADictionaryHandle   *dictHandle;
};

/*! \brief Message Diameter Message
 *
 * A class used for carrying AAAAvpContainerList and DiameterMsgHeader 
 * class instances between applications and the API.
 */
class DiameterMsg
{
    public:
        DiameterMsgHeader     hdr;                  /**< Message header */
        AAAAvpContainerList   acl;                  /**< AVP container list */
        DiameterErrorCode     status;               /**< Error status */
        AAAIpAddr             originator;           /**< Originator IP address  */
        AAAIpAddr             sender;               /**< Sender IP address */
        time_t                secondsTillExpire;    /**< Expiration time of message */
        time_t                startTime;            /**< Time of transmission */
};

/*!
 *==================================================
 * This section defines the diameter specific parsing
 * functions and objects
 *==================================================
 */

/*!
 * Diameter AVP value parser
 */
typedef AAAParser<AAAMessageBlock*,
                  AAAAvpContainerEntry*,
                  AAADictionaryEntry*> DiameterAvpValueParser;

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAA_PARSER_EXPORT_ONLY DiameterAvpValueParser::parseRawToApp();

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAA_PARSER_EXPORT_ONLY DiameterAvpValueParser::parseAppToRaw();

/*!
 * Diameter header parser definition
 */
typedef AAAParser<AAAMessageBlock*,
                  DiameterMsgHeader*,
                  DiameterParseOption> DiameterMsgHeaderParser;

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the read pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an *
 *  error status is thrown.  When the DiameterParseOption is set to
 *  DIAMETER_PARSE_LOOSE, command flag validity check will be skipped.  When
 *  DiameterParseOption is set to DIAMETER_PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the DiameterMsgHeader so that the command
 *  dictionary can be passed to the payload parser. */
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgHeaderParser::parseRawToApp();

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the write pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an
 *  error status is thrown.  When the DiameterParseOption is set to
 *  DIAMETER_PARSE_LOOSE, command flag validity check will be skipped.  When
 *  DiameterParseOption is set to DIAMETER_PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the DiameterMsgHeader so that the command dictionary
 *  can be passed to the payload parser.
 */
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgHeaderParser::parseAppToRaw();

/*!
 * Payload parser definition
 */
typedef AAAParser<AAAMessageBlock*,
                  AAAAvpContainerList*,
                  AAADictionaryHandle*> DiameterMsgPayloadParser;

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
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgPayloadParser::parseRawToApp();// throw(DiameterErrorCode);

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
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgPayloadParser::parseAppToRaw();// throw(DiameterErrorCode);

/*! Functor type definition for AVP value parser creator.  The creator
 *  function takes null argument and returns a pointer to an AVP value
 *  parser.
 */
typedef boost::function0<DiameterAvpValueParser*> DiameterAvpValueParserFunctor;

/*!
 * A template class for type-specific AVP value parser creator.
 */
template <class T>
class DiameterAvpValueParserCreator
{
    public:
        /*!
        * Abstractor * operator
        */
        DiameterAvpValueParser* operator()() {
            return new T();
        }
};

/*! This class defines an AVP type.  An AVP type is defined as a set
 *  of typename, typevalue, size of the AVP payload and the function
 *  objects to create an AVP value parser and AVP container entry for
 *  the type.
 */
class DiameterAvpType :
    public AAAAvpType
{
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
        DiameterAvpType(char* name,
                        AAAAvpDataType type,
                        ACE_UINT32 size,
                        DiameterAvpValueParserFunctor parserCreator,
                        AAAAvpContainerEntryFunctor containerEntryCreator) :
            AAAAvpType(name, type, size, containerEntryCreator),
            parserCreator(parserCreator) {
        }

        /*!
        *  This function is used for creating a type-specific AVP value
        * parser.
        */
        DiameterAvpValueParser* createParser() {
            return parserCreator();
        }

    private:
        DiameterAvpValueParserFunctor parserCreator; /**< The AVP parser creator. */
};

/*! \brief AVP Type List Class Definition. 
 *
 *  Every AVP type MUST have an
 *  instance for this singleton class with specifying an appropriate
 *  function used for creating the AVP value parser.  This class is
 *  also used for adding user-defined AVP types and parsers.
 *
 *  Example usage:
 *
 *  #include "diameter_api.h"
 *  #define AAA_AVP_MYDATA_TYPE 10000
 *  static DiameterAvpValueParserCreator<MyParser>
 *         myValueParserCreator;
 *  static AvpContainerEntryCreator<MyAvpContainerEntry>
 *         myContainerEntryCreator;
 *  AvpTypeList::instance()->add(new AvpType("MyAvp", AAA_AVP_MYDATA_TYPE, 0,
 *                               myParserCreator, myContainerEntryCreator));
 *
 *  MyAvpParser and MyAvpContainerEntry classes will need to be
 *  defined.
 */
class AAA_PARSER_EXPORT DiameterAvpTypeList_S :
    public AAAAvpTypeList
{
    friend class ACE_Singleton<DiameterAvpTypeList_S, 
                               ACE_Recursive_Thread_Mutex>; /**< type list */

    private:
        /*!
        * protected consturctor
        */
        DiameterAvpTypeList_S(void) {
            registerDefaultTypes();
        }

        /*!
        * protected destructor
        */
        ~DiameterAvpTypeList_S(void);

        /*!
        * private method for registering known types
        */
        void registerDefaultTypes();
};

typedef ACE_Singleton<DiameterAvpTypeList_S, ACE_Recursive_Thread_Mutex> DiameterAvpTypeList;
AAA_PARSER_SINGLETON_DECLARE(ACE_Singleton, DiameterAvpTypeList_S, ACE_Recursive_Thread_Mutex);


/*! \brief DiameterAvpContainerEntryManager AVP Container Entry Manager
 *
 * This class is a specialization of AAAAvpContainerEntryManager
 * and links itself directly with DiameterAvpTypeList
 */
class DiameterAvpContainerEntryManager :
    public AAAAvpContainerEntryMngr
{
    public:
        DiameterAvpContainerEntryManager() :
            AAAAvpContainerEntryMngr(*DiameterAvpTypeList::instance()) {
        }
};

/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * This class is a specialization of AAAAvpContainerEntryManager
 * and links itself directly with DiameterAvpTypeList
 */
typedef class AAAAvpContainerMngr DiameterAvpContainerManager;

/*! DiameterScholarAttribute
 *
 * Specialization of scholar and vector data manipulation class.
 */
template <typename T>
class DiameterScholarAttribute :
    public AAAScholarAttribute<T>
{
    public:
        DiameterScholarAttribute() {
        }
        DiameterScholarAttribute(T &val) :
            AAAScholarAttribute<T>(val) {
        }
        virtual void CopyFrom(AAAAvpContainer &c) {
            AAAScholarAttribute<T>::value = c[0]->dataRef(Type2Type<T>());
            AAAScholarAttribute<T>::isSet = true;
        }
        void CopyTo(AAAAvpContainer &c,
                    AAAAvpDataType t) {
            DiameterAvpContainerEntryManager em;
            AAAAvpContainerEntry *e = em.acquire(t);
            e->dataRef(Type2Type<T>()) = AAAScholarAttribute<T>::value;
            c.add(e);
        }
        /*! overload the operator=() so as not
         * to hide the base class implementation
         */
        virtual T& operator=(T v) {
            return AAAScholarAttribute<T>::operator=(v);
        }
};

/*! DiameterGroupedScholarAttribute
 *
 * Specialization of scholar and vector data manipulation class.
 */
template <typename T>
class DiameterGroupedScholarAttribute :
    public DiameterScholarAttribute<T>
{
    public:
        void CopyFrom(AAAAvpContainer &c) {
            AAAAvpContainerList& cl =
                c[0]->dataRef(Type2Type<AAAAvpContainerList>());
            AAAScholarAttribute<T>::value.CopyFrom(cl);
            AAAScholarAttribute<T>::isSet = true;
        }
        void CopyTo(AAAAvpContainer &c) {
            DiameterAvpContainerEntryManager em;
            AAAAvpContainerEntry *e = em.acquire(AAA_AVP_GROUPED_TYPE);
            AAAScholarAttribute<T>::value.CopyTo
                (e->dataRef(Type2Type<AAAAvpContainerList>()));
            c.add(e);
        }
        virtual T& operator=(T v) {
            AAAScholarAttribute<T>::isSet = true;
            AAAScholarAttribute<T>::value=v;
            return AAAScholarAttribute<T>::value;
        }
};

/*! DiameterVectorAttribute
 *
 * Specialization of scholar and vector data manipulation class.
 */
template <typename T>
class DiameterVectorAttribute :
    public AAAVectorAttribute<T>
{
    public:
        virtual void CopyFrom(AAAAvpContainer &c) {
            AAAVectorAttribute<T>::isSet = true;
            if (std::vector<T>::size() < c.size())
            std::vector<T>::resize(c.size());
            for (unsigned i=0; i<c.size(); i++) {
                (*this)[i] = c[i]->dataRef(Type2Type<T>());
            }
        }
        void CopyTo(AAAAvpContainer &c,
                    AAAAvpDataType t) {
            DiameterAvpContainerEntryManager em;
            AAAAvpContainerEntry *e = em.acquire(t);
            for (unsigned i=0; i<std::vector<T>::size(); i++) {
                e = em.acquire(t);
                e->dataRef(Type2Type<T>()) = (*this)[i];
                c.add(e);
            }
        }
        /*! overload the operator=() so as not
         * to hide the base class implementation
         */
        virtual std::vector<T>& operator=(std::vector<T>& value) {
            return AAAVectorAttribute<T>::operator=(value);
        }
};

/*! DiameterGroupedVectorAttribute
 *
 * Specialization of scholar and vector data manipulation class.
 */
template <typename T>
class DiameterGroupedVectorAttribute :
    public DiameterVectorAttribute<T>
{
    public:
        void CopyFrom(AAAAvpContainer &c) {
            AAAVectorAttribute<T>::isSet = true;
            if (AAAVectorAttribute<T>::size() < c.size()) {
                std::vector<T>::resize(c.size());
            }
            for (unsigned i=0; i<c.size(); i++) {
                AAAAvpContainerList& cl =
                c[i]->dataRef(Type2Type<AAAAvpContainerList>());
                (*this)[i].CopyFrom(cl);
                AAAVectorAttribute<T>::isSet = true;
            }
        }
        void CopyTo(AAAAvpContainer &c) {
            DiameterAvpContainerEntryManager em;
            AAAAvpContainerEntry *e;
            for (unsigned i=0; i<AAAVectorAttribute<T>::size(); i++) {
                e = em.acquire(AAA_AVP_GROUPED_TYPE);
                (*this)[i].CopyTo(e->dataRef(Type2Type<AAAAvpContainerList>()));
                c.add(e);
            }
        }
        /*! overload the operator=() so as not
         * to hide the base class implementation
         */
        virtual std::vector<T>& operator=(std::vector<T>& value) {
            return DiameterVectorAttribute<T>::operator=(value);
        }
};

/*!
 *==================================================
 * Predefined diameter widget types
 *==================================================
 */

/* Overrides from generic definition */
template<typename D, AAAAvpDataType t>
class DiameterAvpWidget :
    public AAAAvpWidget<D, t, DiameterAvpContainerEntryManager>
{
    public:
        DiameterAvpWidget(char *name) :
            AAAAvpWidget<D, t, DiameterAvpContainerEntryManager>(name) {
        }
        DiameterAvpWidget(char *name, D &value) :
            AAAAvpWidget<D, t, DiameterAvpContainerEntryManager>(name, value) {
        }
        DiameterAvpWidget(AAAAvpContainer *avp) :
            AAAAvpWidget<D, t, DiameterAvpContainerEntryManager>(avp) {
        }
};

/* Overrides from generic definition */
template<typename D, AAAAvpDataType t>
class DiameterAvpContainerWidget :
    public AAAAvpContainerWidget<D, t, DiameterAvpContainerEntryManager>
{
    public:
       DiameterAvpContainerWidget(AAAAvpContainerList &lst) :
           AAAAvpContainerWidget<D, t, DiameterAvpContainerEntryManager>(lst) {
       }
};

/*! \brief Type specific AVP widget classes
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
typedef DiameterAvpWidget<diameter_identity_t,
                          AAA_AVP_DIAMID_TYPE> DiameterIdentityAvpWidget;
typedef DiameterAvpWidget<diameter_address_t,
                          AAA_AVP_ADDRESS_TYPE> DiameterAddressAvpWidget;
typedef DiameterAvpWidget<diameter_integer32_t,
                          AAA_AVP_INTEGER32_TYPE> DiameterInt32AvpWidget;
typedef DiameterAvpWidget<diameter_unsigned32_t,
                          AAA_AVP_UINTEGER32_TYPE> DiameterUInt32AvpWidget;
typedef DiameterAvpWidget<diameter_integer64_t,
                          AAA_AVP_INTEGER64_TYPE> DiameterInt64AvpWidget;
typedef DiameterAvpWidget<diameter_unsigned64_t,
                          AAA_AVP_UINTEGER64_TYPE> DiameterUInt64AvpWidget;
typedef DiameterAvpWidget<diameter_utf8string_t,
                          AAA_AVP_UTF8_STRING_TYPE>  DiameterUtf8AvpWidget;
typedef DiameterAvpWidget<diameter_grouped_t,
                          AAA_AVP_GROUPED_TYPE> DiameterGroupedAvpWidget;
typedef DiameterAvpWidget<diameter_octetstring_t,
                          AAA_AVP_STRING_TYPE> DiameterStringAvpWidget;
typedef DiameterAvpWidget<diameter_uri_t,
                          AAA_AVP_DIAMURI_TYPE> DiameterDiamUriAvpWidget;
typedef DiameterAvpWidget<diameter_enumerated_t,
                          AAA_AVP_ENUM_TYPE> DiameterEnumAvpWidget;
typedef DiameterAvpWidget<diameter_time_t,
                          AAA_AVP_TIME_TYPE> DiameterTimeAvpWidget;

/*! \brief Type specific AVP widget lookup and parser
 *
 *  Assist in adding, deleting and modifying AVP's
 *  contained in a message list.
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
typedef DiameterAvpContainerWidget<diameter_identity_t,
                                   AAA_AVP_DIAMID_TYPE>
                        DiameterIdentityAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_address_t,
                                   AAA_AVP_ADDRESS_TYPE>
                        DiameterAddressAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_integer32_t,
                                   AAA_AVP_INTEGER32_TYPE>
                        DiameterInt32AvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_unsigned32_t,
                                   AAA_AVP_UINTEGER32_TYPE>
                        DiameterUInt32AvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_integer64_t,
                                   AAA_AVP_INTEGER64_TYPE>
                        DiameterInt64AvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_unsigned64_t,
                                   AAA_AVP_UINTEGER64_TYPE>
                        DiameterUInt64AvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_utf8string_t,
                                   AAA_AVP_UTF8_STRING_TYPE>
                        DiameterUtf8AvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_grouped_t,
                                   AAA_AVP_GROUPED_TYPE>
                        DiameterGroupedAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_octetstring_t,
                                   AAA_AVP_STRING_TYPE>
                        DiameterStringAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_uri_t,
                                   AAA_AVP_DIAMURI_TYPE>
                        DiameterUriAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_enumerated_t,
                                   AAA_AVP_ENUM_TYPE>
                        DiameterEnumAvpContainerWidget;
typedef DiameterAvpContainerWidget<diameter_time_t,
                                   AAA_AVP_TIME_TYPE>
                        DiameterTimeAvpContainerWidget;

/*!
 *==================================================
 * This section defines utility classes for easing
 * the use of the parser objects above
 *==================================================
 */

/*! \brief Generic Result code AVP checker
 *
 * Assist in setting and getting any Result-Code
 * AVP's present in a message list
 */
class DiameterMsgResultCode
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
        DiameterMsgResultCode(DiameterMsg &msg) :
            message(msg) {
        }
        diameter_unsigned32_t ResultCode() {
            DiameterUInt32AvpContainerWidget resultCode(message.acl);
            diameter_unsigned32_t *rc = resultCode.GetAvp("Result-Code");
            return (rc) ? *rc : 0;
        }
        void ResultCode(diameter_unsigned32_t c) {
            DiameterUInt32AvpContainerWidget resultCode(message.acl);
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
        DiameterMsg &message;
};

/*! \brief Generic message header printer
 *
 * This is a useful class for debuggging
 */
class DiameterMsgHeaderDump {
    public:
        static void Dump(DiameterMsg &msg) {
            AAA_LOG((LM_INFO, "(%P|%t) Message header dump\n"));
            AAA_LOG((LM_INFO, "          version = %d\n", int(msg.hdr.ver)));
            AAA_LOG((LM_INFO, "          length  = %d\n", int(msg.hdr.length)));
            AAA_LOG((LM_INFO, "     flags(r,p,e,t) = (%d,%d,%d,%d)\n",
                    int(msg.hdr.flags.r), int(msg.hdr.flags.p),
                    int(msg.hdr.flags.e), int(msg.hdr.flags.t)));
            AAA_LOG((LM_INFO, "          command = %d\n", int(msg.hdr.code)));
            AAA_LOG((LM_INFO, "       hop-by-hop = %d\n", int(msg.hdr.hh)));
            AAA_LOG((LM_INFO, "       end-to-end = %d\n", int(msg.hdr.ee)));
            AAA_LOG((LM_INFO, "   Application id = %d\n", int(msg.hdr.appId)));
        }
};

/*! \brief Wrapper functions for message composition/decomposition
 *
 * Assist in composing and decomposing DiameterMsg
 */
class DiameterMsgWidget
{
    public:
        DiameterMsgWidget(int code,
                    int request = true,
                    int appId = DIAMETER_BASE_APPLICATION_ID) :
            message(std::auto_ptr<DiameterMsg>(new DiameterMsg)) {
            ACE_OS::memset(&(message->hdr), 0, sizeof(message->hdr));
            message->hdr.ver = DIAMETER_PROTOCOL_VERSION;
            message->hdr.length = 0;
            message->hdr.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
            message->hdr.flags.p = DIAMETER_FLAG_CLR;
            message->hdr.flags.e = DIAMETER_FLAG_CLR;
            message->hdr.flags.t = DIAMETER_FLAG_CLR;
            message->hdr.code = code;
            message->hdr.appId = appId;
        }
        ~DiameterMsgWidget() {
        }
        std::auto_ptr<DiameterMsg> &operator()() {
            return message;
        }
        DiameterMsg &Release() {
            return *(message.release());
        }
        void Dump() {
            if (message.get()) {
                DiameterMsgHeaderDump::Dump(*message);
                return;
            }
            AAA_LOG((LM_INFO, "Msg widget is un-assigned\n"));
        }

    private:
        std::auto_ptr<DiameterMsg> message;
};

/*! \brief Wrapper functions for message composition/decomposition
 *
 * Assist in composing and decomposing DiameterMsg
 */
class DiameterMsgParserWidget
{
    public:
        virtual int ParseRawToApp(DiameterMsgWidget &msg,
                                  AAAMessageBlock& rawBuf,
                                  int option) {
            DiameterMsgHeaderParser hp;
            hp.setRawData(&rawBuf);
            hp.setAppData(&msg()->hdr);
            hp.setDictData((DiameterParseOption)option);
            hp.parseRawToApp();
            rawBuf.size(msg()->hdr.length);

            DiameterMsgPayloadParser pp;
            pp.setRawData(&rawBuf);
            pp.setAppData(&msg()->acl);
            pp.setDictData(msg()->hdr.getDictHandle());
            pp.parseRawToApp();
            return (msg()->hdr.length);
        }
        virtual int ParseAppToRaw(DiameterMsgWidget &msg,
                                AAAMessageBlock& rawBuf,
                                int option) {
            DiameterMsgHeaderParser hp;
            hp.setRawData(&rawBuf);
            hp.setAppData(&msg()->hdr);
            hp.setDictData((DiameterParseOption)option);
            hp.parseAppToRaw();

            DiameterMsgPayloadParser pp;
            pp.setRawData(&rawBuf);
            pp.setAppData(&msg()->acl);
            pp.setDictData(msg()->hdr.getDictHandle());
            pp.parseAppToRaw();

            msg()->hdr.length = rawBuf.wr_ptr() - rawBuf.base();
            rawBuf.wr_ptr(rawBuf.base());
            hp.parseAppToRaw();
            return (msg()->hdr.length);
        }
        virtual ~DiameterMsgParserWidget() {
        }
};

/*! \brief Wrapper functions for message composition/decomposition
 *
 * Assist in composing and decomposing DiameterMsg
 */
class DiameterMsgParserWidgetChecked :
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
            catch (DiameterErrorCode &st) {
                ErrorDump(st);
            }
            catch (...) {
                AAA_LOG((LM_INFO, "Parser failure: Unknown Error !!!\n"));
                DiameterErrorCode st;
                st.set(AAA_PARSE_ERROR_TYPE_BUG, 0);
                throw st;
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
            catch (DiameterErrorCode &st) {
                ErrorDump(st);
            }
            catch (...) {
                AAA_LOG((LM_INFO, "Parser failure: Unknown Error !!!\n"));
                DiameterErrorCode st;
                st.set(AAA_PARSE_ERROR_TYPE_BUG, 0);
                throw st;
            }
            return (-1);
        }
        static void ErrorDump(DiameterErrorCode &st) {
            AAA_LOG((LM_INFO, "Parser error: "));
            AAA_PARSE_ERROR_TYPE type;
            int code;
            std::string avp;
            st.get(type, code, avp);
            AAA_LOG((LM_INFO, "Error type=%d, code=%d, name=%s\n", type, code, avp.data()));
        }
};

#endif // __DIAMETER_PARSER_H__
