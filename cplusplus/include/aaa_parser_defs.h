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

#ifndef __AAA_PARSER_DEFS_H__
#define __AAA_PARSER_DEFS_H__

// Standard C++ dependencies
#include <vector>
#include <map>
#include <string>
#include <list>

// ACE dependencies
#include <ace/Basic_Types.h>
#include <ace/INET_Addr.h>
#include <ace/Message_Block.h>
#include <ace/Atomic_Op.h>
#include <ace/Synch.h>
#include <ace/Singleton.h>
#include <ace/Malloc_T.h>
#include <ace/Log_Msg.h>
#include <ace/Local_Memory_Pool.h>

// Local dependencies
#include "framework.h"
#include "resultcodes.h"

/*!
 *==================================================
 * Windows specific export declarations
 *==================================================
 */
#if defined (WIN32)
#  if defined (AAA_PARSER_EXPORTS)
#    define AAA_PARSER_EXPORT ACE_Proper_Export_Flag
#    define AAA_PARSER_EXPORT_ONLY ACE_Proper_Export_Flag
#    define AAA_PARSER_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define AAA_PARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else
#    define AAA_PARSER_EXPORT ACE_Proper_Import_Flag
#    define AAA_PARSER_EXPORT_ONLY
#    define AAA_PARSER_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define AAA_PARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif   /* ! AAA_PARSER_EXPORTS */
#else
#  define AAA_PARSER_EXPORT
#  define AAA_PARSER_EXPORT_ONLY
#  define AAA_PARSER_SINGLETON_DECLARATION(T)
#  define AAA_PARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     /* WIN32 */

/*!
 *==================================================
 * Basic supplimental macros for byte ordering
 *==================================================
 */

/*!
 * The following macros are utility functions format
 * platform independent byte ordering
 */
static inline ACE_UINT32 AAA_SWAP_32(ACE_UINT32 b) {
    return
        ((((b) & 0xff000000) >> 24) | (((b) & 0x00ff0000) >>  8) | \
        (((b) & 0x0000ff00) <<  8) | (((b) & 0x000000ff) << 24));
}

static inline ACE_UINT64 AAA_SWAP_64(ACE_UINT64 b) {
    union {
        ACE_UINT64 ll;
        ACE_UINT32 l[2];
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

static inline unsigned int adjust_word_boundary(unsigned int len) {
    return ((ACE_UINT32)((((len-1)>>2)+1)<<2));
}

/*!
 *==================================================
 * Basic supplimental types for multi-platform abstraction support.
 * These types are not represented well in ACE so additional 
 * defintions has been provided here
 *==================================================
 */
typedef     char              AAAInt8;
typedef     unsigned char     AAAUInt8;

/*! 
 * IP_ADDR provides a way of referring to an IPv4 address, IPv6 address,
 * and IP port. The default implementation (shown here) is defined in
 * the Basic Socket Interface Extensions for IPv6 RFC[10].
 */
typedef     ACE_INET_Addr     AAAIpAddr;

/*!
 * AAACommandCode provides a way of referring to the AAA command code of
 * a command. It is used when registering callbacks, among others.
 */
typedef     ACE_UINT32        AAACommandCode;

/*!
 * AAAAVPCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef     ACE_UINT32        AAAAVPCode;

/*!
 * AAAVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others.
 */
typedef     ACE_UINT32        AAAVendorId;

/*!
 * DiameterAVPFlag provides a way of referring to the AVP flags carried
 * in the AVP header. It indicates whether an AVP is proxyable, mandatory,
 * etc.
 */
typedef     ACE_UINT32        AAAAVPFlag;

/*!
 * Empty class is a stub class used as proxy to
 * un-used variables or parameters
 */
class AAAEmptyClass {
};

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
 *
 *    AAA_ERR_MSG_UNPROCESSED - This code is returned if a message is
 *                              left unparsed
 *
 *    AAA_ERR_INVALID_STATE - This code is returned if a statemachine
 *                            receives an event that switches it to
 *                            an unsupported state
 *
 *    AAA_ERR_PARSING_FAILED - This code is returned if a parsing
 *                            error is encountered. This is a general
 *                            error and has not associated specific
 *                            cause
 *
 *    AAA_ERR_UNKNOWN_SESSION - This code is returned if a message
 *                             is received for an unknown session
 *
 *    AAA_ERR_PARSING_ERROR - This code is returned if a parsing
 *                            error is encountered. It should be
 *                            accompanied by an AAAErrorCode object
 *
 *    AAA_ERR_INCOMPLETE - This code is returned if a session
 *                         spans multiple message exchanges
 *
 *    AAA_ERR_NOSERVICE - This code is returned if the service
 *                        cannot be provided
 */
typedef enum {
    AAA_ERR_NOT_FOUND    =     -2,
    AAA_ERR_FAILURE      =       -1,
    AAA_ERR_SUCCESS      =        0,
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
} AAAAvpDataType;

/*!
 * Definitions of current IANA assigned
 * address family
 * http://www.iana.org/assignments/address-family-numbers
 */
typedef enum {
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
} AAA_ADDR_FAMILY;

/*!
 * Parser type definitions
 */
enum AAAAvpParseType {
    AAA_PARSE_TYPE_FIXED_HEAD = 0,
    AAA_PARSE_TYPE_REQUIRED,
    AAA_PARSE_TYPE_OPTIONAL
};

/*!
 * Error types
 *
 * AAA_PARSE_ERROR_TYPE_NORMAL : Normal error defined in the Diameter specification.
 * AAA_PARSE_ERROR_TYPE_BUG    : Used when application programs misuse this API.
 */
typedef enum  {
    AAA_PARSE_ERROR_TYPE_NORMAL  = 0,
    AAA_PARSE_ERROR_TYPE_BUG     = 1,
} AAA_PARSE_ERROR_TYPE;

/*!
 * The following error code is defined for
 * error type "AAA_PARSE_ERROR_TYPE_BUG"
 */
typedef enum {
    AAA_PARSE_ERROR_MISSING_CONTAINER = 1,
    AAA_PARSE_ERROR_TOO_MUCH_AVP_ENTRIES,
    AAA_PARSE_ERROR_TOO_LESS_AVP_ENTRIES,
    AAA_PARSE_ERROR_PROHIBITED_CONTAINER,
    AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM,
    AAA_PARSE_ERROR_INVALID_CONTAINER_CONTENTS,
    AAA_PARSE_ERROR_UNSUPPORTED_FUNCTIONALITY,
    AAA_PARSE_ERROR_INVALID_PARSER_USAGE,
    AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY,
    AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER
} AAA_PARSE_ERROR;

/*!
 * values possible for Auth-Request-Type
 */
typedef enum {
    AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY = 1,
    AUTH_REQUEST_TYPE_AUTHORIZE_ONLY = 2,
    AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE = 3
} AUTH_REQUEST_TYPE;

/*!
 * values possible for network address type
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

/*! \brief ErrorCode Error Code
 *
 * parser functions throw this class instance when an error occurs.
 */
class AAAErrorCode
{
    public:
        /*!
         * constructor
         */
        AAAErrorCode(void) {
            type = AAA_PARSE_ERROR_TYPE_NORMAL;
            code = AAA_SUCCESS;
        };

        /*!
         * destructor
         */
        virtual ~AAAErrorCode() {
        }

        /*!
         * Access function to retrieve some private data 
         *
         * \param type Error type
         * \param code Result or Bug code
         */
        virtual void get(AAA_PARSE_ERROR_TYPE &t, int &c) {
            t = type;
            c = code;
        }

        /*!
         * Access function to set some private data 
         *
         * \param type Error type to set
         * \param code Result or Bug code to set
         */
        virtual void set(AAA_PARSE_ERROR_TYPE t, int c) {
            type = t;
            code = c;
        }

    protected:
        AAA_PARSE_ERROR_TYPE type;  /**< error type (AAA_PARSE_ERROR_TYPE_NORMAL
                                     *   or AAA_PARSE_ERROR_TYPE_BUG)
                                     */
        int code;  /**< either a diameter result code or a bug_code above */
};

/*!
 *==================================================
 * Utility classes in support of 
 * manipulating basic error types
 *==================================================
 */

/*! \brief AAAUInt8Range Unsigned Int8 Range
 *
 * Utility classes for breaking down 
 * Unsigned int8 ranges
 */
class AAAUInt8Range
{
    public:
        AAAUInt8Range(AAAInt8 first, AAAInt8 last) : 
            first(first), 
            last(last) {
        }
        AAAUInt8Range() {
        }

    public:
        AAAInt8 first;
        AAAInt8 last;
};

/*! \brief AAAUInt16Range Unsigned Int16 Range
 *
 * Utility classes for breaking down 
 * Unsigned int16 ranges
 */
class AAAUInt16Range
{
    public:
        AAAUInt16Range(ACE_UINT16 first, ACE_UINT16 last) :
            first(first),
            last(last) {
        }
        AAAUInt16Range(ACE_UINT16 single) : first(single), last(single) {
        }
        AAAUInt16Range() {
        }

    public:
        ACE_UINT16 first;
        ACE_UINT16 last;
};

/*! \brief UTF8 string format check.
 *
 *  If the data contains a valid UTF8 string, it returns 0.
 *  Otherwise, it returns -1.
 */
class UTF8Checker
{
    public:
        /*!
        * constructor
        */
        UTF8Checker() {
        }

        /*! When nullCheck is true, the check will fail if a null octet is
        *  found in the data.
        */
        int operator()(const char *data, unsigned length, bool nullCheck=false) {
            unsigned i = 0;
            while (i<length) {
                unsigned char b = data[i++];

                // Check the first octet of the current UTF8 character.
                // Null check
                if (b == 0x00) {
                    if (nullCheck) return -1;
                }

                // The first bit check.
                else if ((b >> 7) == 0x00) {
                    continue;
                }  // 7-bit ASCII character.

                // If the first 7 bits are all '1's, this is not an UTF8 character.
                if ((b >> 1) == 0x76) {
                    return -1;
                } // Out of UTF8 character range.

                b <<= 1;
                // Count the number of '1' of the first octet
                int count=0;
                for (count=0; count<5; count++) {
                    if ((b && 0x40) == 0) break;
                }

                // The count value must be greater than 0.
                if (count==0) {
                    return -1;
                } // Out of UTF8 character range.

                // Check remaining octet(s)
                for (int j=0; j<count; j++) {
                    if (i >= length) {
                        return -1;
                    }
                    if ((data[i++] >> 6) != 0x02) {
                        return -1;
                    }
                }
            }
            return 0;
        }
};

/*!
 *==================================================
 * AAA Parser Memory Management Mechanisms
 *
 * This section pertains to memory management mechanisms utilized by
 * AAA parser. The scheme is a pool based method where there is an
 * expected accumulation of memory block for more efficient reuse.
 *==================================================
 */

#define AAA_MEMORY_MANAGER_NAME "AAAMemoryManager"

typedef ACE_Malloc<ACE_LOCAL_MEMORY_POOL, 
                   ACE_SYNCH_MUTEX> 
                   AAAMalloc;
typedef ACE_Allocator_Adapter<AAAMalloc> AAAAllocator;

/*!\brief AAAMemoryManager Memory Manager
 *
 * This class provides heap based memory management.
 * Allocations are cumulative. Each call to malloc()
 * method will result in accumulations of memory 
 * chunks from the manager in to the allocator list.
 * A call to free will return the chunk into the
 * allocator free list but not the manager.
 */
class AAAMemoryManager :
    public AAAAllocator
{
    friend class ACE_Singleton<AAAMemoryManager,
                               ACE_Recursive_Thread_Mutex>; /**< memory manager */

    private:
        /*!
        * constructor
        */
        AAAMemoryManager() :
            AAAAllocator(AAA_MEMORY_MANAGER_NAME) {
        }
};

/*
 * !!! IMPORTANT !!!
 *
 * To be more efficient, AAAMemoryManager is declared as 
 * a singleton for each executable that has included this 
 * header.
 */
typedef ACE_Singleton<AAAMemoryManager, 
                      ACE_Recursive_Thread_Mutex> 
                      AAAMemoryManager_S;

AAA_PARSER_SINGLETON_DECLARE(ACE_Singleton,
                             AAAMemoryManager,
                             ACE_Recursive_Thread_Mutex);

/*!
 *==================================================
 * Raw message data structures
 *==================================================
 */

/*! \brief MessageBuffer Message Buffer Definitions
 *
 * AAAMessageBlock is a class for containing raw data.
 * 
 * Offset contains the current pointer location relative to the 
 * address of data.  Offset is set only after "write" operation and 
 * not updated for "read" operation.
 */
class AAAMessageBlock:
    public ACE_Message_Block
{
    public:
        /*!
        * Acquire by reference.  The message block points to the specified
        * location with the size set to the specified value.
        *
        * \param buf User buffer passed to message block
        * \param s size of user buffer
        */
        static AAAMessageBlock* Acquire(char *buf, ACE_UINT32 s) {
            return new AAAMessageBlock(buf,s);
        }

        /*!
        * Acquire by allocation.  A new message block of specified size is
        * created.
        *
        * \param s size to be internally allocated
        */
        static AAAMessageBlock* Acquire(ACE_UINT32 s) {
            return new AAAMessageBlock(s);
        }

        /*!
        * Acquire by duplication.  Referece count of the original message
        * data is incremented by one.
        *
        * \param b original message
        */
        static AAAMessageBlock* Acquire(AAAMessageBlock* b) {
            return new AAAMessageBlock(b);
        }

        /*!
        * Release function.  Referece count of the message data is
        * decremented by one.  If the reference count becomes 0, the
        * entire message block is deleted.  It is definitely more
        * efficient to directly call release().
        */
        void Release() {
            release();
        }

    protected:
        /*!
        * A message block is created with pointing to the buffer
        * location and having a specified size.
        *
        * \param buf User buffer passed to message block
        * \param s size of user buffer
        */
        AAAMessageBlock(char *buf, ACE_UINT32 s) {
            init(buf, s);
        }

        /*!
        * A message block consisting of null characters of the
        * specified size is created.
        *
        * \param s size to be internally allocated
        */
        AAAMessageBlock(ACE_UINT32 s) {
            init(s);
        }

        /*!
        * A shallow-copy constractor.  This constructor just duplicates
        * (ie. a shallow copy) the data block of the incoming Message
        * Block, since the incoming Message Block always has a data block
        * has a data block allocated from the heap.
        */
        AAAMessageBlock(AAAMessageBlock *b) :
            ACE_Message_Block((const ACE_Message_Block&)*b,0) {
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate (proxy only)
        */
        void* operator new(size_t s) {
            return AAAMemoryManager_S::instance()->malloc(s);
        }

        /*!
        * Abstracted delete operator
        *
        * \param p data block to free
        */
        void operator delete(void *p) { 
            AAAMemoryManager_S::instance()->free(p);
        }

    private:
        /*!
        * Destractor which is never used.
        */
        ~AAAMessageBlock() {
        }
};

/*! \brief Automated scoped release of message block
 *
 * Utility class that provides a scoped persistence
 * to an AAAMessageBlock object.
 */
class AAAMessageBlockScope
{
    public:
        AAAMessageBlockScope(ACE_Message_Block *&mb) :
            block(mb) {
        }
        ~AAAMessageBlockScope(void) {
            block->release(); 
        }

    protected:
       ACE_Message_Block *&block;
};

/*!
 *==================================================
 * The following classes are utility classes for simplifying
 * composition and de-composition of AVP entries and list.
 *==================================================
 */

/*! AAAScholarAttribute
 *
 * Scholar and vector data manipulation class.
 */
template <typename T>
class AAAScholarAttribute
{
    public:
        AAAScholarAttribute() :
            isSet(false) {
        }
        AAAScholarAttribute(T &val) :
            value(val),
            isSet(true) {
        }
        virtual ~AAAScholarAttribute() {
        }
        virtual void Clear() {
            isSet = false;
        }
        virtual void Set(T val) {
            value = val;
            isSet = true;
        }
        inline bool operator==(const T& v) const {
            return value == v;
        }
        virtual T& operator()() {
            return value;
        }
        virtual T& operator=(T v) {
            isSet = true;
            value = v;
            return value;
        }
        virtual bool& IsSet() {
            return isSet;
        }

    protected:
        T value;
        bool isSet;
};

/*! AAAVectorAttribute
 *
 * Grouped scholar and vector data manipulation class.
 */
template <typename T>
class AAAVectorAttribute :
    public std::vector<T>
{
    public:
        AAAVectorAttribute() :
            isSet(false) {
        }
        virtual ~AAAVectorAttribute() {
        }
        virtual void Clear() {
            isSet = false; std::vector<T>::clear();
        }
        virtual std::vector<T>& operator()() {
            return *this;
        }
        virtual std::vector<T>& operator=(std::vector<T>& value) {
            isSet = true;
            (std::vector<T>&)(*this)=value;
            return *this;
        }
        virtual bool& IsSet() {
            return isSet;
        }

    protected:
        bool isSet;
};

#endif // __AAA_PARSER_API_H__
