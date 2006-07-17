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
// $Id: framework.h,v 1.61 2006/04/20 21:46:40 vfajardo Exp $
// framework.h: Protocol Framework API based on ACE.
// Written by Yoshihiro Ohba

#ifndef __AAA_PARSER_API_H__
#define __AAA_PARSER_API_H__

/*!
 * Windows specific export declarations
 */
#if defined (WIN32)
#  if defined (AAAPARSER_EXPORTS)
#    define AAAPARSER_EXPORT ACE_Proper_Export_Flag
#    define AAAPARSER_EXPORT_ONLY ACE_Proper_Export_Flag
#    define AAAPARSER_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON
#  else
#    define AAAPARSER_EXPORT ACE_Proper_Import_Flag
#    define AAAPARSER_EXPORT_ONLY
#    define AAAPARSER_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON
#  endif   /* ! AAAPARSER_EXPORTS */
#else
#  define AAAPARSER_EXPORT
#  define AAAPARSER_EXPORT_ONLY
#  define AAAPARSER_SINGLETON_DECLARATION(T)
#  define AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
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
 * Error types encountered during parsing
 * NORMAL : Normal error defined in the Diameter specification.
 * BUG    : Used when application programs misuse this API.
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
 * Data type definitions for AAA Parser
 */
typedef    ACE_INT32                  diameter_integer32_t;
typedef    ACE_UINT64                 diameter_integer64_t;
typedef    ACE_UINT32                 diameter_unsigned32_t;
typedef    ACE_UINT64                 diameter_unsigned64_t;
typedef    diameter_unsigned32_t      diameter_enumerated_t;
typedef    diameter_unsigned32_t      diameter_time_t;  
typedef    std::string                diameter_octetstring_t;
typedef    diameter_octetstring_t     diameter_utf8string_t;  
typedef    diameter_utf8string_t      diameter_identity_t;
typedef    diameter_unsigned32_t      diameter_time_t;
typedef    class AAAAvpContainerList  diameter_grouped_t;

typedef struct {
   public:
      std::string   fqdn;
      ACE_UINT16    port; 
      AAA_UINT8     transport:2;
      AAA_UINT8     protocol:2;
      AAA_UINT8     scheme:2;
} diameter_uri_t;

typedef struct {
   public:
      ACE_UINT16               type;
      diameter_octetstring_t   value;
} diameter_address_t;

/*!
 * values possible for transport field of diameter_diamident_t
 *
 * avp_t is a special type used only in this library
 * for constructing a raw AVP.  When using this type, specify
 * "AVP" as the avp_container type.
 * The string contains the entire AVP including AVP header.
 */
typedef    diameter_octetstring_t     avp_t;

/*!
 * The following are IANA assigned numbers for address family
 * http://www.iana.org/assignments/address-family-numbers
 */
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

/*!
 * Built-in swap functions for 32-bit values
 */
static inline diameter_unsigned32_t AAA_SWAP_32(diameter_unsigned32_t b)
{
    return
      ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
       (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24));
}

/*!
 * Built-in swap functions for 64-bit values
 */
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

/*!
 * Utility class for UINT8
 */
class AAA_UINT8_RANGE
{
  public:
     AAA_UINT8_RANGE(AAA_UINT8 first, AAA_UINT8 last) : 
        first(first), last(last) {
     }
     AAA_UINT8_RANGE() {
     }
     
  public:
     AAA_UINT8 first;
     AAA_UINT8 last;
};

/*!
 * Utility class for UINT16
 */
class AAA_UINT16_RANGE
{
  public:
     AAA_UINT16_RANGE(ACE_UINT16 first, ACE_UINT16 last) :
        first(first), last(last) {
     }
     AAA_UINT16_RANGE(ACE_UINT16 
        single) : first(single), last(single) {
     }
     AAA_UINT16_RANGE() {
     }
     
  public:
     ACE_UINT16 first;
     ACE_UINT16 last;
};

/*! \brief ErrorStatus Error Status
 *
 * parser functions throw this class instance when an error occurs.
 */
class DIAMETERPARSER_EXPORT AAAErrorStatus
{
   private:
      int type;          /**< error type (NORMAL or BUG) */
      int code;          /**< either a diameter result code or a bug_code above */
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

/*!
 * Memory management for AAA messages
 */


































typedef ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> AAAMalloc;
typedef ACE_Allocator_Adapter<AAAMalloc> AAAAllocator;

#define AAA_MEMORY_MANAGER_NAME "AAA_Memory_Manager"

class AAAPARSER_EXPORT AAAMemoryManager : public AAAAllocator
{
...
};

typedef ACE_Singleton<AAAMemoryManager, ACE_Recursive_Thread_Mutex> AAAMemoryManager_S;
AAAPARSER_SINGLETON_DECLARE(ACE_Singleton, AAAMemoryManager, ACE_Recursive_Thread_Mutex);

class AAAAvpContainerEntry
{
...
};

template <class T>
class AAATypeSpecificAvpContainerEntry
: public AAAAvpContainerEntry
{
...
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
class AAAPARSER_EXPORT AAAAvpContainer
: public std::vector<AAAAvpContainerEntry*>
{
...
};

/*! \brief AvpContainerList AVP Container List
 *
 * A class that defines a list of AAAAvpContainer and functions
 * for manipulating the list.
 */
class AAAPARSER_EXPORT AAAAvpContainerList
: public std::list<AAAAvpContainer*>

{
..
};

/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * AAAAvpContainerEntryManager and AAAAvpContainerManager is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */
class AAAPARSER_EXPORT AAAAvpContainerEntryManager
{
...
};

/*! \brief AvpContainerEntryManager AVP Container Entry Manager
 *
 * AAAAvpContainerEntryManager and AAAAvpContainerManager is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */
class AAAPARSER_EXPORT AAAAvpContainerManager
{
...
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
class AAAPARSER_EXPORT AAADictionaryManager
{
...
};

class AAAEmptyClass {};

/*! \brief Parser Message Parser Definition
 *
 * AAAParser is a template class for generic parser.
 */
template <class RAWDATA, class APPDATA,
          class DICTDATA = AAAEmptyClass,
          class CODEC = AAAEmptyClass>
class AAAPARSER_EXPORT_ONLY AAAParser
{
...
};

****** Make sure to remove CODEC ******

/// Parser error enumration.
enum DiameterParserError {
  DictionaryError,
  HeaderError,
  PayloadError
};

****** s/DiameterParserError/AAAParserError ******


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
...
};

class AAA_MessageBlockScope
{
...
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

****** Remove protocolId ******


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

/*! \brief UTF8 string format check.
 *  If the data contains a valid UTF8 string, it returns 0.
 *  Otherwise, it returns -1.
 */
class UTF8Checker
{
...
};

/*!
 * Diameter logging facity derived directly from ACE
 */
class AAALogMsg : public ACE_Log_Msg
{
...
};

typedef ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex> AAALogMsg_S;
AAAPARSER_SINGLETON_DECLARE(ACE_Singleton, AAALogMsg, ACE_Recursive_Thread_Mutex);

#define AAA_LOG AAALogMsg_S::instance()->log

/// Scholar and vector data manipulation class.
template <typename T>
class AAA_ScholarAttribute
{
...
};

/// Grouped scholar and vector data manipulation class.
template <typename T>
class AAA_GroupedScholarAttribute : public AAA_ScholarAttribute<T>
{
...
};

template <typename T>
class AAA_VectorAttribute : public std::vector<T>
{
...
};

template <typename T>
class AAA_GroupedVectorAttribute : public AAA_VectorAttribute<T>
{
...
};

/*! \brief Generic AVP widget allocator
 */
template<class D, AAA_AVPDataType t>
class AAA_AvpWidget {
...
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
...
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

/*! Functor type definition for AVP container entry creator.  The
 *  creator function takes an integer argument and returns a pointer
 *  to an AVP container entry.
 */
typedef boost::function1<AAAAvpContainerEntry*, int> AvpContainerEntryFunctor;


------------------------------------------
diameter_parser.h will have the following:
------------------------------------------

/*!
 * Open diameter version declaration
 */
#define AAA_VERSION_MAJOR  0x01
#define AAA_VERSION_MINOR  0x00
#define AAA_VERSION_MICRO  0x05

****** s/AAA/DIAMETER/ in the above define macros ******

/*!
 * AAACommandCode provides a way of referring to the AAA command code of
 * a command. It is used when registering callbacks, among others.
 */
typedef ACE_UINT32 AAACommandCode;

****** s/AAACommandCode/CommandCode/ ******

/*!
 * AAAVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others. Note that
 * vendor id 0 is reserved and is defined by the preprocessor constant
 * AAA_NO_VENDOR_ID.
 */
typedef ACE_UINT32 AAAVendorId;

****** s/AAAVendorId/DiameterVendorId/ ******

/*!
 * AAA_AVPCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef ACE_UINT32 AAA_AVPCode;

****** s/AAA_AVPCode/DiameterAVPCode/ ******

/*!
 * AAASessionHandle is an identifier for a particular AAA session. It is
 * used in the session APIs and when a message is created.
 */
typedef void* AAASessionHandle;

****** s/AAASessionHandle/DiameterSessionHandle/ ******


/*!
 * AAAApplicationId identifies a particular client session to the API.
 * The application id is passed to AAAStartSession(), and is attached to
 * incoming messages, to indicate with which client session the message
 * is associated.
 */
typedef ACE_UINT32 AAAApplicationId;

****** s/AAAApplicationId/DiameterApplicationId/ ******

/*!
 * The result codes are values returned from remote servers as part
 * of messages. They correspond directly to the result codes in the
 * Diameter specification [1]:
 */
typedef ACE_UINT32 AAAResultCode;

****** s/AAAResultCode/DiameterResultCode/ ******

typedef enum {
    AAA_AVP_FLAG_NONE =                 0,
    AAA_AVP_FLAG_MANDATORY =            0x1,
    AAA_AVP_FLAG_RESERVED =             0x2,
    AAA_AVP_FLAG_VENDOR_SPECIFIC =      0x4,
    AAA_AVP_FLAG_END_TO_END_ENCRYPT =   0x10,
    AAA_AVP_FLAG_UNKNOWN =              0x10000,
    AAA_AVP_FLAG_ENCRYPT =              0x40000,
} AAA_AVPFlagEnum;

******* s/AAA/DIAMETER/ in the above enum *******

typedef ACE_UINT32 AAA_AVPFlag;

******* s/AAA_AVPFlag/DiameterAVPFlag/ *******

class AAAPARSER_EXPORT AAADictionaryEntry
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

****** s/AAA/Diameter/ in the above class definition ******

typedef enum {
    AAA_ACCT_EVENT = 1,
    AAA_ACCT_START = 2,
    AAA_ACCT_INTERIM = 3,
    AAA_ACCT_STOP = 4
} AAAAcctMessageType;

****** s/AAA/DIAMETER/ in the above enum ******
****** s/AAAAcctMessageType/DiameterAcctMessageType/ ******

/*!
 * Session type
 */
typedef enum {
    AAA_STYPE_AUTHENTICATION,  /**< Authentication session */
    AAA_STYPE_ACCOUNTING       /**< Accounting session */
} AAASessionType;

****** s/AAA_// in the above enum ******
****** s/AAASessionType/SessionType/ ******

/*!
 * Macros and Preprocessor Definitions
 *
 *  The following definition reserves the vendor id of 0:
 */
#define AAA_NO_VENDOR_ID                                0

#define AAA_BASE_APPLICATION_ID                         0

****** s/AAA_/DIAMETER_/ in the above define macros ******


enum {
  AAA_IPFILTER_RULE_SRCDST_EXACT,
  AAA_IPFILTER_RULE_SRCDST_MASK,
  AAA_IPFILTER_RULE_SRCDST_KEYWORD_ANY,
  AAA_IPFILTER_RULE_SRCDST_KEYWORD_ASSIGNED
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

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

****** s/AAA_/DIAMETER_/ in the above class definition ******

enum {
  AAA_IPFILTER_RULE_IP_OPTION_SSRR=1,
  AAA_IPFILTER_RULE_IP_OPTION_LSRR,
  AAA_IPFILTER_RULE_IP_OPTION_RR,
  AAA_IPFILTER_RULE_IP_OPTION_TS
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

enum {
  AAA_IPFILTER_RULE_TCP_OPTION_MSS=1,
  AAA_IPFILTER_RULE_TCP_OPTION_WINDOW,
  AAA_IPFILTER_RULE_TCP_OPTION_SACK,
  AAA_IPFILTER_RULE_TCP_OPTION_TS,
  AAA_IPFILTER_RULE_TCP_OPTION_CC
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

enum {
  AAA_IPFILTER_RULE_TCP_FLAG_FIN=1,
  AAA_IPFILTER_RULE_TCP_FLAG_SYN,
  AAA_IPFILTER_RULE_TCP_FLAG_RST,
  AAA_IPFILTER_RULE_TCP_FLAG_PSH,
  AAA_IPFILTER_RULE_TCP_FLAG_ACK,
  AAA_IPFILTER_RULE_TCP_FLAG_URG
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

/*! IPFilterRule type. */
class diameter_ipfilter_rule_t
{
...
};

enum {
  AAA_IPFILTER_RULE_ACTION_PERMIT,
  AAA_IPFILTER_RULE_ACTION_DENY
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

enum {
  AAA_IPFILTER_RULE_DIRECTION_IN,
  AAA_IPFILTER_RULE_DIRECTION_OUT
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

/*!
 * values possible for transport field of diameter_uri_t
 */
enum {
  TRANSPORT_PROTO_TCP = 0,
  TRANSPORT_PROTO_SCTP,
  TRANSPORT_PROTO_UDP,
};

****** s/TRANSPORT/DIAMETER_TRANSPORT/ in the above define macros ******

/*!
 * values possible for protocol field of diameter_uri_t
 */
enum {
  AAA_PROTO_DIAMETER = 0,
  AAA_PROTO_RADIUS,
  AAA_PROTO_TACACSPLUS,
};

****** s/AAA_/DIAMETER_/ in the above define macros ******

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
 * Header bit field definition
 */
struct hdr_flag {
  AAA_UINT8 r:1;  /**< Request */
  AAA_UINT8 p:1;  /**< Proxiable */
  AAA_UINT8 e:1;  /**< Error */
  AAA_UINT8 t:1;  /**< Potentially re-transmitted */
  AAA_UINT8 rsvd:4; /**< Reserved */
};

class AAADiameterHeader;

****** s/AAADiameterHeader/DiameterHeader/ ******

class AAAMessage;

****** s/AAAMessage/DiameterMessage/ ******

class AAAAvpHeader;

****** s/AAAAvpHeader/DiameterAvpHeader/ *******
....

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, ParseOption>
HeaderParser;

****** s/HeaderParser/DiameterHeaderParser/ *******

/*! Header parser definition */
typedef AAAParser<AAAMessageBlock*, AAADiameterHeader*, AAADictionaryOption*>
HeaderParserWithProtocol;

****** s/HeaderParserWithProtocol/DiameterHeaderParser/ *******

/*! AVP codec definition */
typedef AAAParser<std::pair<char*, int>, AAAAvpHeader*> AvpHeaderCodec;

****** Do we need this typedef? ******

/*! Payload parser definition */
typedef AAAParser<AAAMessageBlock*, AAAAvpContainerList*,
                  AAADictionaryHandle*, AvpHeaderCodec*>
PayloadParserWithEmptyCodec;

****** s/PayloadParserWithEmptyCodec/DiameterPayloadParser/ ******

/*!
 * Generic Payload parsing definition
 */
template<class CODEC>
class AAAPARSER_EXPORT_ONLY PayloadParserWithCodec :
  public PayloadParserWithEmptyCodec
{
...
};

****** Do we need this class? ******

typedef PayloadParserWithCodec<AvpHeaderCodec>
PayloadParser;

****** Do we need this class? ******

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the read pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an *
 *  error status is thrown.  When the ParseOption is set to
 *  PARSE_LOOSE, command flag validity check will be skipped.  When
 *  ParseOption is set to PARSE_STRICT, an AAADictionaryHandle will be
 *  set inside the AAADiameterHeader so that the command
 *  dictionary can be passed to the payload parser. */
template<> void AAAPARSER_EXPORT_ONLY HeaderParser::parseRawToApp();// throw(AAAErrorStatus);

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
template<> void AAAPARSER_EXPORT_ONLY HeaderParser::parseAppToRaw();// throw(AAAErrorStatus);
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
template<> void AAAPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseRawToApp();// throw(AAAErrorStatus);

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
template<> void AAAPARSER_EXPORT_ONLY PayloadParserWithEmptyCodec::parseAppToRaw();// throw(AAAErrorStatus);
/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., std::pari<char*,int>  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpHeaderCodec::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., std::pari<char*,int>.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpHeaderCodec::parseAppToRaw();// throw(AAAErrorStatus);

/*!
 * Header size definition
 */
#define HEADER_SIZE 20

****** s/HEADER_SIZE/DIAMETER_HEADER_SIZE/ ******

/*! \brief DiameterHeader Diameter Message Header Definition
 *
 * A class used for storing Diameter header fields.
 */
class AAAPARSER_EXPORT AAADiameterHeader
{
...
};

****** s/AAADiameterHeader/DiameterHeader/ ******

/*! \brief Message Diameter Message
 *
 * This class is re-defined as a replacement of the C-based AAAMessage
 * type definition.
 *
 * A class used for carrying AAAAvpContainerList and AAADiameterHeader
 * class instances between applications and the API.
 */
class AAAPARSER_EXPORT AAAMessage
{
...
};

****** s/AAAMessage/DiameterMessage/ ******

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
...
};

s/AAAAvpHeader/DiameterAvpHeader/

typedef AAAParser<AAAMessageBlock*, AAAAvpContainerEntry*,
                     AAADictionaryEntry*, AvpHeaderCodec*> AvpValueParser;

****** s/AvpValueParser/DiameterAvpValueParser/ ******

/*! Parsing starts from the current read pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpValueParser::parseRawToApp();// throw(AAAErrorStatus);

/*! Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void AAAPARSER_EXPORT_ONLY AvpValueParser::parseAppToRaw();// throw(AAAErrorStatus);

/*! Functor type definition for AVP value parser creator.  The creator
 *  function takes null argument and returns a pointer to an AVP value
 *  parser.
 */
typedef boost::function0<AvpValueParser*> AvpValueParserFunctor;

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

****** s/AvpValueParserCreator/DiameterAvpValueParserCreator/

/*! This class defines an AVP type.  An AVP type is defined as a set
 *  of typename, typevalue, size of the AVP payload and the function
 *  objects to create an AVP value parser and AVP container entry for
 *  the type.
 */
class AAAPARSER_EXPORT AvpType {
...
};

****** s/AvpType/DiameterAvpType ******

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
class AAAPARSER_EXPORT AvpTypeList_S : public std::list<AvpType*>
{
...
};

****** s/AvpTypeList_S/DiameterAvpTypeList_S/ ******


typedef ACE_Singleton<AvpTypeList_S, ACE_Recursive_Thread_Mutex> AvpTypeList;
AAAPARSER_SINGLETON_DECLARE(ACE_Singleton, AvpTypeList_S, ACE_Recursive_Thread_Mutex);

/*! \brief Generic Result code AVP checker
 * Assist in setting and getting any Result-Code
 * AVP's present in a message list
 */
class AAA_MsgResultCode
{
...
};

****** s/AAA_MsgResultCode/DiameterMsgResultCode/

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

****** s/AAA_MsgDump/DiameterMsgDump/ ******

//
// General
//
#define AAA_PROTOCOL_VERSION              0x1
#define AAA_FLG_SET                       0x1
#define AAA_FLG_CLR                       0x0

****** s/AAA/DIAMETER/ in the above define macros ******

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_MsgWidget
{
...
};

****** s/AAA_MsgWidget/DiameterMsgWidget/

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_DiameterMsgParserWidget
{
..
};

****** s/AAA_DiameterMsgParserWidget/DiameterMsgParserWidget/

/*! \brief Wrapper functions for message composition/decomposition
 * Assist in composing and decomposing AAAMessage
 */
class AAA_DiameterMsgParserWidgetChecked :
    public AAA_DiameterMsgParserWidget
{
...
};

****** s/AAA_DiameterMsgParserWidgetChecked/DiameterMsgParserWidgetChecked/ ******


#endif // __AAA_PARSER_API_H__