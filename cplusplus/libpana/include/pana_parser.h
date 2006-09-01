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

#ifndef __PANA_PARSER_H__
#define __PANA_PARSER_H__

#include "aaa_parser_api.h"

/*
 *==================================================
 * Macros and Preprocessor Definitions
 *==================================================
 */

/*
 * PANA flag definitions
 */
#define PANA_PROTOCOL_VERSION          0x1
#define PANA_FLAG_SET                  0x1
#define PANA_FLAG_CLR                  0x0

/*
 *==================================================
 * The following definitions are for PANA specific
 * parsing support. Some basic data types have been
 * derived from the generic aaa_parser_api
 *==================================================
 */

/*
 * PANAAvpCode provides a way of referring to the code number of an AVP.
 * It is used as a parameter to the dictionary functions, and a field in
 * the AVP struct.
 */
typedef ACE_UINT16     PANA_AvpCode;

/*
 * PANAVendorId provides a way of referring to the vendor identification
 * code. It is used when registering callbacks, among others. 
 */
typedef ACE_UINT32     PANA_VendorId;

/*
 * PANAAvpFlag provides a way of referring to the AVP flags carried
 * in the AVP header. It indicates whether an AVP is vendor or mandatory.
 */
typedef ACE_UINT16     PANA_AvpFlag;

/*
 *==================================================
 * Pre-defined enumration
 *==================================================
 */

/*
 * The AVP flags defines the flags set in the AVP header.
 * They correspond directly to the avp flags defined in the
 * pana-draft-12 specification [1]:
 */
typedef enum {
    PANA_AVP_FLAG_NONE                 =   0x0,
    PANA_AVP_FLAG_MANDATORY            =   0x4000,
    PANA_AVP_FLAG_VENDOR_SPECIFIC      =   0x8000,
    PANA_AVP_FLAG_RESERVED             =   0x0000,
} PANA_AvpFlagEnum;

/*
 *==================================================
 * The following definitions are for diameter specific
 * data type definitions for storing parsed data.
 *==================================================
 */

/*
 * Data type definitions for AAA Parser
 */
typedef ACE_INT32                  pana_integer32_t;

typedef ACE_UINT64                 pana_integer64_t;

typedef ACE_UINT32                 pana_unsigned32_t;

typedef ACE_UINT64                 pana_unsigned64_t;

typedef pana_unsigned32_t          pana_enumerated_t;

typedef pana_unsigned32_t          pana_time_t;

typedef std::string                pana_octetstring_t;

typedef pana_octetstring_t         pana_utf8string_t;

typedef class AAAAvpContainerList  pana_grouped_t;

/*
 * values possible for transport field of pana_diamident_t
 *
 * avp_t is a special type used only in this library
 * for constructing a raw AVP.  When using this type, specify
 * "AVP" as the avp_container type.
 * The string contains the entire AVP including AVP header.
 */
typedef pana_octetstring_t         pana_avp_t;

typedef struct
{
    public:
        ACE_UINT16               type;
        pana_octetstring_t   value;
} pana_address_t;

/*
 *==================================================
 * The following are definitions for the pana
 * dictionary
 *==================================================
 */

class PANA_DictionaryEntry
{
    public:
        PANA_DictionaryEntry(PANA_AvpCode code,
                             const char *name,
                             AAAAvpDataType type,
                             PANA_VendorId vid,
                             PANA_AvpFlag flg) :
            m_AvpName(name),
            m_AvpType(type),
            m_AvpCode(code),
            m_VendorId(vid),
            m_Flags(flg) {
        }


    public:
        std::string         m_AvpName;
        AAAAvpDataType      m_AvpType;
        PANA_AvpCode        m_AvpCode;
        PANA_VendorId       m_VendorId;
        PANA_AvpFlag        m_Flags;
};

/*
 * A class used as a handle for internal dictionary structures.
 */
class PANA_DictionaryHandle {
};

class PANA_AvpTypeList_S :
    public AAAAvpTypeList
{
    friend class ACE_Singleton<PANA_AvpTypeList_S,
                               ACE_Recursive_Thread_Mutex>; /**< type list */

    private:
        PANA_AvpTypeList_S(void) {
            registerDefaultTypes();
        }
        virtual ~PANA_AvpTypeList_S(void) {
        }
        void registerDefaultTypes();
};

typedef ACE_Singleton<PANA_AvpTypeList_S, ACE_Recursive_Thread_Mutex> PANA_AvpTypeList;
AAA_PARSER_SINGLETON_DECLARE(ACE_Singleton, PANA_AvpTypeList_S, ACE_Recursive_Thread_Mutex);

/*
 *==================================================
 * This section defines the diameter specific parsing
 * functions and objects
 *==================================================
 */

/*
 * Diameter AVP value parser
 */
typedef AAAParser<AAAMessageBlock*,
                  AAAAvpContainerEntry*,
                  PANA_DictionaryEntry*> PANA_AvpValueParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpValueParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., AAAMessageBlock.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpValueParser::parseAppToRaw();




+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*
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

        friend class AAAParser<AAAMessageBlock*,
                               DiameterMsgHeader*,
                               DiameterDictionaryOption*>; /**< Parser friend */

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
        }

        /*!
        * destructor
        */
        DiameterMsgHeader() {
        }

        /*!
        * returns the current dictionary handle
        */
        inline DiameterDictionaryHandle *getDictHandle() { 
            return dictHandle;
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
        DiameterDictionaryHandle   *dictHandle;
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
 *  DiameterParseOption is set to DIAMETER_PARSE_STRICT, an DiameterDictionaryHandle will be
 *  set inside the DiameterMsgHeader so that the command
 *  dictionary can be passed to the payload parser. */
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgHeaderParser::parseRawToApp();

/*! Parsing starts from the head of the raw data, i.e.,
 *  AAAMessageBlock.  When parsing is successful, the write pointer
 *  will proceed to one octet after the last octet of the header,
 *  i.e., the first octet of the payload.  When parsing fails, an
 *  error status is thrown.  When the DiameterParseOption is set to
 *  DIAMETER_PARSE_LOOSE, command flag validity check will be skipped.  When
 *  DiameterParseOption is set to DIAMETER_PARSE_STRICT, an DiameterDictionaryHandle will be
 *  set inside the DiameterMsgHeader so that the command dictionary
 *  can be passed to the payload parser.
 */
template<> void AAA_PARSER_EXPORT_ONLY DiameterMsgHeaderParser::parseAppToRaw();

/*!
 * Payload parser definition
 */
typedef AAAParser<AAAMessageBlock*,
                  AAAAvpContainerList*,
                  DiameterDictionaryHandle*> DiameterMsgPayloadParser;

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
                e->dataRef(Type2Type<T>()) = (*this)[i];
                c.add(e);
            }
        }
        inline DiameterGroupedVectorAttribute<T> &operator=
            (DiameterGroupedVectorAttribute<T>& value) {
            AAAVectorAttribute<T>::isSet = true;
            (std::vector<T>&)(*this)=value;
            return *this;
        }
};

/*! \brief Generic AVP widget allocator
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
template<class D, AAAAvpDataType t>
class DiameterAvpWidget {
    public:
        DiameterAvpWidget(char *name) {
            DiameterAvpContainerManager cm;
            m_cAvp = cm.acquire(name);
        }
        DiameterAvpWidget(char *name, D &value) {
            DiameterAvpContainerManager cm;
            m_cAvp = cm.acquire(name);
            Get() = value;
        }
        DiameterAvpWidget(AAAAvpContainer *avp) :
            m_cAvp(avp) {
        }
        ~DiameterAvpWidget() {
        }
        D &Get() {
            DiameterAvpContainerEntryManager em;
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

/*! \brief Generic AVP widget lookup and parser
 *
 *  Assist in adding, deleting and modifying AVP's
 *  contained in a message list.
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
template<class D, AAAAvpDataType t>
class DiameterAvpContainerWidget
{
    public:
       DiameterAvpContainerWidget(AAAAvpContainerList &lst) :
           list(lst) {
       }
       D *GetAvp(char *name, unsigned int index=0) {
          AAAAvpContainer* c = list.search(name);
          if (c && (index < c->size())) {
              AAAAvpContainerEntry *e = (*c)[index];
              return e->dataPtr(Type2Type<D>());
          }
          return (0);
       }
       D &AddAvp(char *name, bool append = false) {
          AAAAvpContainer* c = list.search(name);
          if (! c) {
              DiameterAvpWidget<D, t> avpWidget(name);
              list.add(avpWidget());
              return avpWidget.Get();
          }
          else if ((c->size() == 0) || append) {
              DiameterAvpWidget<D, t> avpWidget(c);
              return avpWidget.Get();
          }
          else {
              return (*c)[0]->dataRef(Type2Type<D>());
          }
       }
       void AddAvp(DiameterAvpContainerWidget<D, t> &avp) {
           list.add(avp());
       }
       void DelAvp(char *name) {
          std::list<AAAAvpContainer*>::iterator i;
          for (i=list.begin(); i!=list.end();i++) {
              AAAAvpContainer *c = *i;
              if (ACE_OS::strcmp(c->getAvpName(), name) == 0) {
                  list.erase(i);
                  DiameterAvpContainerManager cm;
                  cm.release(c);
                  break;
              }
          }
       }
       unsigned int GetAvpCount(char *name) {
          AAAAvpContainer* c = list.search(name);
          return (c) ? c->size() : 0;
       }

    private:
       AAAAvpContainerList &list;
};

/*!
 *==================================================
 * Predefined diameter widget types
 *==================================================
 */

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
            AAA_LOG(LM_INFO, "Msg widget is un-assigned\n");
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
            catch (DiameterErrorCode &st) {
                ErrorDump(st);
            }
            catch (...) {
                AAA_LOG(LM_INFO, "Parser error: Unknown fatal !!!\n");
                exit (0);
            }
            return (-1);
        }
        static void ErrorDump(DiameterErrorCode &st) {
            AAA_LOG(LM_INFO, "Parser error: ");
            AAA_PARSE_ERROR_TYPE type;
            int code;
            std::string avp;
            st.get(type, code, avp);
            AAA_LOG(LM_INFO, "Error type=%d, code=%d, name=%s\n",
                    type, code, avp.data());
        }
};

#endif // __PANA_PARSER_H__
