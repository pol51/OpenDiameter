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

/*!
 * Open diameter version declaration
 */
#define DIAMETER_VERSION_MAJOR        0x01
#define DIAMETER_VERSION_MINOR        0x00
#define DIAMETER_VERSION_MICRO        0x07

/*!
 * General
 */
#define DIAMETER_PROTOCOL_VERSION     0x1
#define DIAMETER_FLG_SET              0x1
#define DIAMETER_FLG_CLR              0x0

/*!
 * Macros and Preprocessor Definitions
 *
 *  The following definition reserves the base app id of 0:
 */
#define DIAMETER_BASE_APPLICATION_ID  0x0

/*!
 * Header size definition
 */
#define DIAEMTER_HEADER_SIZE          20

/*!
 * DiameterCommandCode provides a way of referring to the AAA command code of
 * a command. It is used when registering callbacks, among others.
 */
typedef AAACommandCode               DiameterCmdCode;

/*!
 * DiameterVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others. Note that
 * vendor id 0 is reserved and is defined by the preprocessor constant
 * AAA_NO_VENDOR_ID.
 */
typedef ACE_UINT32                   DiameterVendorId;

/*!
 * DiameterAvpCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef ACE_UINT32                   DiameterAvpCode;

/*!
 * AAAApplicationId identifies a particular application in the API.
 */
typedef AAAApplicationId             DiameterAppId;

/*!
 * The result codes are values returned from remote servers as part
 * of messages. They correspond directly to the result codes in the
 * Diameter specification [1]:
 */
typedef ACE_UINT32                   DiameterResultCode;

/*!
 * Diameter AVP data type
 */
typedef AAA_AVPDataType              DiameterAvpDataType;

/*!
 * Diameter AVP flags
 */
typedef ACE_UINT32                   DiameterAvpFlag;

/*!
 * An enumeration of the AVP flags that can be present
 * in a diameter header
 */
typedef enum {
    DIAMETER_AVP_FLAG_NONE =                 0,
    DIAMETER_AVP_FLAG_MANDATORY =            0x1,
    DIAMETER_AVP_FLAG_RESERVED =             0x2,
    DIAMETER_AVP_FLAG_VENDOR_SPECIFIC =      0x4,
    DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT =   0x10,
    DIAMETER_AVP_FLAG_UNKNOWN =              0x10000,
    DIAMETER_AVP_FLAG_ENCRYPT =              0x40000,
} DiameterAvpFlagEnum;

/*!
 * Enumerations for IP filter rule AVP
 */
enum {
   DIAMETER_IPFILTER_RULE_IP_OPTION_SSRR = 1,
   DIAMETER_IPFILTER_RULE_IP_OPTION_LSRR,
   DIAMETER_IPFILTER_RULE_IP_OPTION_RR,
   DIAMETER_IPFILTER_RULE_IP_OPTION_TS
};

enum {
   DIAMETER_IPFILTER_RULE_TCP_OPTION_MSS = 1,
   DIAMETER_IPFILTER_RULE_TCP_OPTION_WINDOW,
   DIAMETER_IPFILTER_RULE_TCP_OPTION_SACK,
   DIAMETER_IPFILTER_RULE_TCP_OPTION_TS,
   DIAMETER_IPFILTER_RULE_TCP_OPTION_CC
};

enum {
   DIAMETER_IPFILTER_RULE_TCP_FLAG_FIN = 1,
   DIAMETER_IPFILTER_RULE_TCP_FLAG_SYN,
   DIAMETER_IPFILTER_RULE_TCP_FLAG_RST,
   DIAMETER_IPFILTER_RULE_TCP_FLAG_PSH,
   DIAMETER_IPFILTER_RULE_TCP_FLAG_ACK,
   DIAMETER_IPFILTER_RULE_TCP_FLAG_URG
};

enum {
   DIAMETER_IPFILTER_RULE_ACTION_PERMIT,
   DIAMETER_IPFILTER_RULE_ACTION_DENY
};

enum {
   DIAMETER_IPFILTER_RULE_DIRECTION_IN,
   DIAMETER_IPFILTER_RULE_DIRECTION_OUT
};

enum {
   DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
   DIAMETER_IPFILTER_RULE_SRCDST_MASK,
   DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
   DIAMETER_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

class DIAMETER_IPFILTER_RULE_SRCDST
{
   public:
      DIAMETER_IPFILTER_RULE_SRCDST(AAA_UINT8 repr=DIAMETER_IPFILTER_RULE_SRCDST_EXACT,
                                    diameter_utf8string_t ipno=std::string(),
                                    AAA_UINT8 b=0,
                                    bool mod=true) : 
         bits(b),
         modifier(mod), 
         representation(repr), 
         ipno(ipno) { 
      }
      
   public:
      bool       modifier;         /*! Modifier '!' maps to false */
      AAA_UINT8  representation;   /*! One of the following:
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

      AAA_UINT8 bits;
      diameter_utf8string_t ipno;
      std::list<AAA_UINT16_RANGE> portRangeList;   /*! list of port ranges. */
};

/*! 
 * IPFilterRule type. 
 */
class diameter_ipfilter_rule_t 
{
   public:
      diameter_ipfilter_rule_t() : 
         frag(false), 
         established(false), 
         setup(false) {
      }

   public:   
      AAA_UINT8 action; /*! 
                           AAA_IPFILTER_RULE_ACTION_PERMIT or 
                           AAA_IPFILTER_RULE_ACTION_DENY 
                         */
      AAA_UINT8 dir;    /*! 
                           AAA_IPFILTER_RULE_DIRECTION_IN or 
                           AAA_IPFILTER_RULE_DIRECTION_OUT 
                         */
      AAA_UINT8 proto;  /*! 
                           The value 0 means wildcard number that matches
                           any protocol.
                         */
                         
     DIAMETER_IPFILTER_RULE_SRCDST src;
     DIAMETER_IPFILTER_RULE_SRCDST dst;

     /// Option rules.
     bool frag; /* indicates fragmented packets except for the first
		           fragment 
		         */
     std::list<int> ipOptionList; /*! IP Option list.  An entry with a
                                      negative value means that the option
                                      corresponding to its positive value
                                      is negated. 
                                    */
     std::list<int> tcpOptionList; /*! TCP Option list. An entry with a
	                                   negative value means that the option
                                       corresponding to its positive value
                                       is negated.  
                                    */
     bool established; /*! TCP packets only.  Match packets that have the
                           RST or ACK bits set. 
                        */
     bool setup; /*! TCP packets only.  Match packets that have the SYN
                     bit set but no ACK bit 
                  */
     std::list<int> tcpFlagList; /*! TCP Flag list. An entry with a
                                     negative value means that the flag
                                     corresponding to its positive value
                                     is negated.  */
     std::list<AAA_UINT8_RANGE> icmpTypeRangeList; /*! ICMP Type Range list */
} ;

/*!
 * values possible for transport field of diameter_uri_t
 */
enum {
   DIAMETER_TRANSPORT_PROTO_TCP = 0,
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
 * values possible for Auth-Request-Type
 */
enum {
   AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY = 1,
   AUTH_REQUEST_TYPE_AUTHORIZE_ONLY = 2,
   AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE = 3
};

/*!
 * Header bit field definition
 */
struct hdr_flag {
   AAA_UINT8 r     :1;  /**< Request */
   AAA_UINT8 p     :1;  /**< Proxiable */
   AAA_UINT8 e     :1;  /**< Error */
   AAA_UINT8 t     :1;  /**< Potentially re-transmitted */
   AAA_UINT8 rsvd  :4;  /**< Reserved */
};

/*!
 * Diameter Message Structures
 */
class DiameterHeader;
class DiameterMessage;
class DiameterAvpHeader;

/*!
 * This class represents a diameter dictionary entry
 */
class AAAPARSER_EXPORT DiameterDictionaryEntry
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
      DiameterDictionaryEntry(DiameterAvpCode code,
                              const char *name,
                              DiameterAvpDataType type,
                              DiameterVendorId id,
                              DiameterAvpFlag flg,
                              int proto);

   public:
      int                  protocol; /**< Protocol */
      std::string          avpName;  /**< AVP name */
      DiameterAvpCode      avpCode;  /**< AVP code */
      DiameterAvpDataType  avpType;  /**< AVP type */
      DiameterVendorId     vendorId; /**< Vendor ID */
      DiameterAvpFlag      flags;    /**< AVP flags */
};

/*! 
 * Actual diameter header parser definition 
 */
typedef AAAParser<AAAMessageBlock*, 
                  DiameterHeader*, 
                  ParseOption>            DiameterHeaderParser;
typedef AAAParser<AAAMessageBlock*, 
                  DiameterHeader*, 
                  AAADictionaryOption*>   DiameterHeaderParserWithProtocol;
typedef AAAParser<std::pair<char*, int>, 
                  AAAAvpHeader*>          AvpHeaderCodec;
typedef AAAParser<AAAMessageBlock*, 
                  AAAAvpContainerList*,
                  AAADictionaryHandle*, 
                  AvpHeaderCodec*>        PayloadParserWithEmptyCodec;

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

typedef PayloadParserWithCodec<AvpHeaderCodec> DiameterPayloadParser;

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the read pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an *
 *  error status is thrown.  When the ParseOption is set to
 *  PARSE_LOOSE, command flag validity check will be skipped.  When
 *  ParseOption is set to PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the DiameterHeader so that the command
 *  dictionary can be passed to the payload parser. */
template<> void AAAPARSER_EXPORT_ONLY DiameterHeaderParser::parseRawToApp();

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the write pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an
 *  error status is thrown.  When the ParseOption is set to
 *  PARSE_LOOSE, command flag validity check will be skipped.  When
 *  ParseOption is set to PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the DiameterHeader so that the command dictionary
 *  can be passed to the payload parser.
 */
template<> void AAAPARSER_EXPORT_ONLY DiameterHeaderParser::parseAppToRaw();

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
template<> void AAAPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseRawToApp();

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
template<> void AAAPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseAppToRaw();

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., std::pari<char*,int>  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpHeaderCodec::parseRawToApp();

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., std::pari<char*,int>.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpHeaderCodec::parseAppToRaw();

/*! \brief DiameterHeader Diameter Message Header Definition
 *
 * A class used for storing Diameter header fields.
 */
class DIAMETERPARSER_EXPORT DiameterHeader
{
   public:
      friend class AAAParser<AAAMessageBlock*, 
                             DiameterHeader*, 
                             ParseOption>; 
      friend class AAAParser<AAAMessageBlock*, 
                             DiameterHeader*, 
                             AAADictionaryOption*>; 

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
      DiameterHeader(AAA_UINT8 ver, 
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
      virtual ~DiameterHeader() {
      }

      /*!
       * returns the current dictionary handle
       */
      inline AAADictionaryHandle *getDictHandle() { 
      	 return dictHandle; 
      }

      /*!
       * returns the command name
       */
      const char* getCommandName();

   public:
      AAA_UINT8          ver;        /**< Version */
      ACE_UINT32         length:24;  /**< Message length (payload) */
      struct hdr_flag    flags;      /**< Header flags */
      AAACommandCode     code:24;    /**< Command code */
      AAAApplicationId   appId;      /**< Application ID */
      ACE_UINT32         hh;         /**< Hop-to-Hop ID */
      ACE_UINT32         ee;         /**< End-to-End ID */

   private:
      AAADictionaryHandle* dictHandle;
};

/*! \brief Message Diameter Message
 *
 * This class is re-defined as a replacement of the C-based DiameterMessage
 * type definition.
 *
 * A class used for carrying AAAAvpContainerList and DiameterHeader 
 * class instances between applications and the API.
 */
class DIAMETERPARSER_EXPORT DiameterMessage
{
   public:
      DiameterHeader         hdr;                 /**< Message header */
      AAAAvpContainerList    acl;                 /**< AVP container list */
      AAAErrorStatus         status;              /**< Error status */
      IP_ADDR                originator;          /**< Originator IP address  */
      IP_ADDR                sender;              /**< Sender IP address */
      time_t                 secondsTillExpire;   /**< Expiration time of message */
      time_t                 startTime;           /**< Time of transmission */
};

/*! \brief AVP header flags
 *
 */
struct avp_flag {
   AAA_UINT8   v;   /**< Vendor flag */
   AAA_UINT8   m;   /**< Mandatory flag */
   AAA_UINT8   p;   /**< end-to-end security flag */
};

/*! \brief AVP header
 *
 */
class DiameterAvpHeader
{
   public:
      DiameterAvpHeader() : 
         code(0), 
         length(0), 
         vendor(0), 
         value_p(0),
         parseType(PARSE_TYPE_OPTIONAL) {
      memset(&flag, 0, sizeof(flag));
   }
   
   public:
      ACE_UINT32         code;        /**< AVP code */
      struct avp_flag    flag;        /**< AVP flags */
      ACE_UINT32         length:24;   /**< AVP length */
      ACE_UINT32         vendor;      /**< Vendor code */
      char*              value_p;     /**< Value */
      
   public:      
      inline AAAAvpParseType& ParseType() { 
         return parseType; 
      }
      
   private:
      AAAAvpParseType parseType;
}; 

/*! 
 * Actual diameter avp parser definition 
 */
typedef AAAParser<AAAMessageBlock*, 
                  AAAAvpContainerEntry*,
                  DiameterDictionaryEntry*, 
                  AvpHeaderCodec*>   DiameterAvpValueParser;

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY DiameterAvpValueParser::parseRawToApp();

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY DiameterAvpValueParser::parseAppToRaw();

/*! Functor type definition for AVP value parser creator.  The creator
 *  function takes null argument and returns a pointer to an AVP value
 *  parser.
 */
typedef boost::function0<DiameterAvpValueParser*> DiameterAvpValueParserFunctor;

/*! A template class for type-specific AVP value parser creator. */
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
class DIAMETERPARSER_EXPORT DiameterAvpType {
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
      AvpType(char* name, 
              AAA_AVPDataType type, 
              ACE_UINT32 size,
              DiameterAvpValueParserFunctor parserCreator,
              AvpContainerEntryFunctor containerEntryCreator) :
         name(name), 
         type(type), 
         size(size),
         parserCreator(parserCreator),
         containerEntryCreator(containerEntryCreator) {
      }

      /*!
       *  This function is used for obtaining the AVP type name.
       */
      char* getName(void) { 
      	 return name; 
      }

      /*!
       *  This function is used for obtaining the AVP type value.
       */
      AAA_AVPDataType getType(void) { 
      	 return type; 
      }

      /*!
       * This function is used for obtaining the minimum AVP payload size.
       */
      ACE_UINT32 getMinSize(void) { 
      	 return size; 
      }

      /*!
       *  This function is used for creating a type-specific AVP value
       * parser.
       */
      DiameterAvpValueParser* createParser() { 
      	 return parserCreator();
      }

      /*!
       *  This function is used for crating a type-specific container
       * entry.
       */
      AAAAvpContainerEntry* createContainerEntry(int type) { 
      	 return containerEntryCreator(type);
      }

   private:
      char *                           name;             /**< name of the avp type */
      AAA_AVPDataType                  type;             /**< enumerate type */
      ACE_UINT32                       size;             /**< minimum size of this avp 
                                                              payload (0 means variable 
                                                              size) */
      DiameterAvpValueParserFunctor    parserCreator;    /**< The AVP parser creator. */
      AvpContainerEntryFunctor         containerEntryCreator; /**< The AVP parser creator. */
};

/*! \brief Diameter AVP Type List Class Definition. 
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
 *  static AvpValueParserCreator<MyParser> 
 *         myValueParserCreator;
 *  static AvpContainerEntryCreator<MyAvpContainerEntry> 
 *         myContainerEntryCreator;
 *  DiameterAvpTypeList::instance()->add(new AvpType("MyAvp", AAA_AVP_MYDATA_TYPE, 0, 
 *                                       myParserCreator, myContainerEntryCreator));
 *
 *  MyAvpParser and MyAvpContainerEntry classes will need to be
 *  defined.
 */
class DIAMETERPARSER_EXPORT DiameterAvpTypeList_S : 
   public std::list<AvpType*> 
{
      friend class ACE_Singleton<DiameterAvpTypeList_S, 
                                 ACE_Recursive_Thread_Mutex>;
                              
   public:
      /*!
       * Returns a pointer to the AVP type instance
       * 
       * \param type AVP type
       */
      DiameterAvpType* search(ACE_UINT32 type) {
         for (iterator i = begin(); i != end(); i++) {
            if ((ACE_UINT32)((*i)->getType()) == type) {
               return *i;
            }
         }
         return NULL;
      }
      
      /*!
       * Returns a pointer to the AVP type instance
       * 
       * \param name AVP name
       */
      DiameterAvpType* search(const char* name) {
         for (iterator i = begin(); i != end(); i++) {
            if (ACE_OS::strcmp((*i)->getName(), name) == 0) {
               return *i;
            }
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
      DiameterAvpTypeList_S(void) { 
      	 registerDefaultTypes(); 
      }

      /*!
       * protected destructor
       */
      virtual ~DiameterAvpTypeList_S(void) {
      }

      /*!
       * private method for registering known types
       */
      void registerDefaultTypes();

   private:
      ACE_Thread_Mutex  mutex;    /**< mutex protector */
};

typedef ACE_Singleton<DiameterAvpTypeList_S, 
                      ACE_Recursive_Thread_Mutex> DiameterAvpTypeList;
DIAMETERPARSER_SINGLETON_DECLARE(ACE_Singleton, DiameterAvpTypeList_S, ACE_Recursive_Thread_Mutex);

/*! \brief Generic Result code AVP checker
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
       DiameterMsgResultCode(DiameterMessage &msg) :
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
       DiameterMessage &m_Msg;
};

/*! \brief Generic message header printer
 */
class DiameterMsgDump {
   public:
      static void Dump(DiameterMessage &msg) {
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

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class DiameterMsgWidget
{
    public:
       DiameterMsgWidget(int code,
                         int request = true,
                         int appId = AAA_BASE_APPLICATION_ID) :
          m_Msg(std::auto_ptr<AAAMessage>(new AAAMessage)) {
          ACE_OS::memset(&(m_Msg->hdr), 0, sizeof(m_Msg->hdr));
          m_Msg->hdr.ver = DIAMETER_PROTOCOL_VERSION;
          m_Msg->hdr.length = 0;
          m_Msg->hdr.flags.r = request ? DIAMETER_FLG_SET : DIAMETER_FLG_CLR;
          m_Msg->hdr.flags.p = DIAMETER_FLG_CLR;
          m_Msg->hdr.flags.e = DIAMETER_FLG_CLR;
          m_Msg->hdr.flags.t = DIAMETER_FLG_CLR;
          m_Msg->hdr.code = code;
          m_Msg->hdr.appId = appId;
       }
       ~DiameterMsgWidget() {
       }
       std::auto_ptr<AAAMessage> &operator()() {
          return m_Msg;
       }
       DiameterMessage &Release() {
          return *(m_Msg.release());
       }
       void Dump() {
          if (m_Msg.get()) {
              DiameterMsgDump::Dump(*m_Msg);
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
class DiameterMsgParserWidget
{
public:
    virtual int ParseRawToApp(DiameterMsgWidget &msg,
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
    virtual int ParseAppToRaw(DiameterMsgWidget &msg,
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
    virtual ~DiameterMsgParserWidget() {
    }
};

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
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
        AAA_MessageBlockScope block_guard(aBlock); 
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
        AAA_MessageBlockScope block_guard(aBlock); 
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

#endif // __DIAMETER_PARSER_H__
