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
#include "pana_message.h"
#include "pana_memory_manager.h"

/*
 *==================================================
 * This section defines the diameter specific parsing
 * functions and objects
 *==================================================
 */

/*
 * Diameter AVP value parser
 */
typedef AAAParser<PANA_MessageBuffer*,
                  AAAAvpContainerEntry*,
                  AAADictionaryEntry*> PANA_AvpValueParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpValueParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpValueParser::parseAppToRaw();

/*  Functor type definition for AVP value parser creator.  The creator
 *  function takes null argument and returns a pointer to an AVP value
 *  parser.
 */
typedef boost::function0<PANA_AvpValueParser*> PANA_AvpValueParserFunctor;

/*
 * A template class for type-specific AVP value parser creator.
 */
template <class T>
class PANA_AvpValueParserCreator
{
    public:
        /*
        * Abstractor * operator
        */
        PANA_AvpValueParser* operator()() {
            return new T();
        }
};

/* This class defines an AVP type.  An AVP type is defined as a set
 * of typename, typevalue, size of the AVP payload and the function
 * objects to create an AVP value parser and AVP container entry for
 * the type.
 */
class PANA_AvpType :
    public AAAAvpType
{
    public:
        PANA_AvpType(char* name,
                     AAAAvpDataType type,
                     ACE_UINT32 size,
                     PANA_AvpValueParserFunctor parserCreator,
                     AAAAvpContainerEntryFunctor containerEntryCreator) :
            AAAAvpType(name, type, size, containerEntryCreator),
            parserCreator(parserCreator) {
        }

        /*
        *  This function is used for creating a type-specific AVP value
        * parser.
        */
        PANA_AvpValueParser* createParser() {
            return parserCreator();
        }

    private:
        PANA_AvpValueParserFunctor parserCreator; /**< The AVP parser creator. */
};

class PANA_AvpTypeList_S :
    public AAAAvpTypeList
{
    friend class ACE_Singleton<PANA_AvpTypeList_S,
                               ACE_Recursive_Thread_Mutex>; /**< type list */

    private:
        PANA_AvpTypeList_S(void) {
            DefaultTypes();
        }
        virtual ~PANA_AvpTypeList_S(void);
        void DefaultTypes();
};

typedef ACE_Singleton<PANA_AvpTypeList_S, ACE_Recursive_Thread_Mutex> PANA_AvpTypeList;
AAA_PARSER_SINGLETON_DECLARE(ACE_Singleton, PANA_AvpTypeList_S, ACE_Recursive_Thread_Mutex);

/*
 * Container definitions
 */
class PANA_AvpContainerEntryManager :
    public AAAAvpContainerEntryMngr
{
    public:
        PANA_AvpContainerEntryManager() :
            AAAAvpContainerEntryMngr(*PANA_AvpTypeList::instance()) {
        }
};

typedef class AAAAvpContainerMngr PANA_AvpContainerManager;

#define PANA_AVP_HEADER_LEN(avp) \
  (avp->avpCode == 0 ? 0 : \
  (avp->flags & PANA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8))

class PANA_AvpList_S :
    public AAAAvpList
{
    friend class ACE_Singleton<PANA_AvpList_S, ACE_Recursive_Thread_Mutex>;

    private:
        PANA_AvpList_S();
        virtual ~PANA_AvpList_S();
};

typedef ACE_Singleton<PANA_AvpList_S, ACE_Recursive_Thread_Mutex> PANA_AvpList;

class PANA_QualifiedAvpList:
    public AAAQualifiedAvpList
{
    public:
        PANA_QualifiedAvpList(AAAAvpParseType pt) :
            AAAQualifiedAvpList(pt) {
        }
        ACE_UINT32 getMinSize(void) {
            return (0);
        }
};

class PANA_Dictionary :
    public AAADictionaryHandle
{
    public:
        PANA_AvpCode            code;
        AAAVendorId             vendorId;
        PANA_QualifiedAvpList*  m_Fixed;     /* fixed */
        PANA_QualifiedAvpList*  m_Required;  /* required */
        PANA_QualifiedAvpList*  m_Optional;  /* optional */
};

typedef struct 
{
    AAAUInt8 request:1;   /* request */
    AAAUInt8 stateless:1; /* stateless handshake */
    AAAUInt8 reserved:6;
} PANA_CommandFlags;

class PANA_Command :
    public PANA_Dictionary
{
    public:
        std::string       name;
        PANA_CommandFlags flags;
};

class PANA_CommandList_S :
    public AAACommandList<PANA_Command>
{
        friend class ACE_Singleton<PANA_CommandList_S, ACE_Recursive_Thread_Mutex>;
    public:
        PANA_Command* search(PANA_AvpCode code) {
            mutex.acquire();
            iterator c = this->begin();
            for (; c != this->end(); c++) {
                if ((*c)->code == code) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    private:

        virtual ~PANA_CommandList_S() {
            for (iterator i=begin(); i!=end(); i++) {
                delete (*i)->m_Fixed;
                delete (*i)->m_Required;
                delete (*i)->m_Optional;
                delete *i;
            }
        }
};

typedef ACE_Singleton<PANA_CommandList_S, ACE_Recursive_Thread_Mutex> PANA_CommandList;

class PANA_GroupedAVP :
    public PANA_Dictionary {
};

class PANA_GroupedAvpList_S :
    public AAAGroupedAvpList<PANA_GroupedAVP>
{
    friend class ACE_Singleton<PANA_GroupedAvpList_S, ACE_Recursive_Thread_Mutex>;

    public:
        PANA_GroupedAVP* search(ACE_UINT32 code,
                                ACE_UINT32 vendorId) {
            mutex.acquire();
            for (iterator c=begin(); c!=end(); c++) {
                if ((*c)->code == code && (*c)->vendorId == vendorId) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    private:
        virtual ~PANA_GroupedAvpList_S() {
            for (iterator i=begin(); i!=end(); i++) {
                delete (*i)->m_Fixed;
                delete (*i)->m_Required;
                delete (*i)->m_Optional;
                delete *i;
            }
        }
};

typedef ACE_Singleton<PANA_GroupedAvpList_S, ACE_Recursive_Thread_Mutex>
    PANA_GroupedAvpList;

class PANA_AvpHeaderList :
    public std::list<PANA_AvpHeader>
{
    public:
        PANA_AvpHeaderList() {
        }
        void create(PANA_MessageBuffer *aBuffer) throw(AAAErrorCode);
};

class PANA_AvpRawData
{
    public:
        union {
            PANA_MessageBuffer *msg;
            PANA_AvpHeaderList *ahl;
        };
};

/*
 * AVP header parser
 */
typedef AAAParser<PANA_AvpRawData*,
                  PANA_AvpHeader*,
                  AAADictionaryEntry*>
                  PANA_AvpHeaderParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpHeaderParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpHeaderParser::parseAppToRaw();

/*
 * AVP parser
 */
typedef AAAParser<PANA_AvpRawData*,
                  AAAAvpContainer*,
                  AAADictionaryEntry*>
                  PANA_AvpParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_AvpParser::parseAppToRaw();

/*
 * PANA Message Header parser
 */
typedef AAAParser<PANA_MessageBuffer*,
                  PANA_MsgHeader*,
                  PANA_Dictionary*> PANA_HeaderParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_HeaderParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_HeaderParser::parseAppToRaw();

/*
 * Payload parser definition
 */
typedef AAAParser<PANA_MessageBuffer*,
                  AAAAvpContainerList*,
                  PANA_Dictionary*> PANA_PayloadParser;

/*  Parsing starts from the current read pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the read
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_PayloadParser::parseRawToApp();

/* Parsing starts from the current write pointer of the raw data,
 *  i.e., PANA_MessageBuffer.  When parsing is successful, the write
 *  pointer will proceed to one octet after the last octet of the
 *  AVP value.  When parsing fails, an error status is thrown.  A
 *  non-null dictionary data must be specified.
 */
template<> void PANA_PayloadParser::parseAppToRaw();

#endif // __PANA_PARSER_H__
