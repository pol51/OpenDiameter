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

#ifndef __AAA_DICTIONARY_API_H__
#define __AAA_DICTIONARY_API_H__

// Standard C++ dependencies
#include <string>
#include <list>

// ACE dependencies
#include <ace/Basic_Types.h>

// Local dependencies
#include "framework.h"
#include "aaa_parser_defs.h"

typedef struct
{
  ACE_UINT16 min;
  ACE_UINT16 max;
} AAARangeQualifier;

typedef enum
{
  AAA_QUALIFIER_INFINITY = 65535, /* 2^16 -1 */

};

class AAADictionaryEntry
{
    public:
        AAADictionaryEntry(AAAAVPCode code,
                           const char *name,
                           AAAAvpDataType type,
                           AAAVendorId vid,
                           AAAAVPFlag flg) :
            avpName(name),
            avpCode(code),
            avpType(type),
            vendorId(vid),
            flags(flg) {
        }
        virtual ~AAADictionaryEntry() {
        }

    public:
        std::string         avpName;  /**< AVP name */
        AAAAVPCode          avpCode;  /**< AVP code */
        AAAAvpDataType      avpType;  /**< AVP type */
        AAAVendorId         vendorId; /**< Vendor ID */
        AAAAVPFlag          flags;    /**< AVP flags */
};

class AAADictionaryHandle {
};

class AAAAvpList :
    public std::list<AAADictionaryEntry*>
{
    public:
        void add(AAADictionaryEntry* avp) {
            if (this->search(avp->avpName) != NULL) {
                AAA_LOG((LM_ERROR, "duplicated AVP definition [%s].\n",
                            avp->avpName.c_str()));
                exit(1);
            }

            mutex.acquire();
            push_back(avp);
            mutex.release();
        }
        AAADictionaryEntry* search(const std::string& avpName) {
            mutex.acquire();
            std::list<AAADictionaryEntry*>::iterator i;
            for (i = begin(); i!=end(); i++) {
                if ((*i)->avpName == avpName) {
                    mutex.release();
                    return *i;
                }
            }
            mutex.release();
            return NULL;
        }
        AAADictionaryEntry* search(AAAAVPCode code,
                                   AAAVendorId vendor) {
            mutex.acquire();
            std::list<AAADictionaryEntry*>::iterator i;
            for (i = begin(); i!=end(); i++) {
                if (((*i)->avpCode == code) && ((*i)->vendorId == vendor)) {
                    mutex.release();
                    return *i;
                }
            }
            mutex.release();
            return NULL;
        }

    protected:
        AAAAvpList() {
        }
        virtual ~AAAAvpList() {
            std::list<AAADictionaryEntry*>::iterator i;
            for (i=begin(); i!=end(); i++) {
                delete *i;
            }
        }

    private:
        ACE_Thread_Mutex mutex;
};

typedef struct /* AVP with qualifier (min,max) */
{
    AAADictionaryEntry *avp;
    AAARangeQualifier qual;
} AAAQualifiedAVP;

class AAAQualifiedAvpList :
    public std::list<AAAQualifiedAVP*>
{
    public:
        AAAQualifiedAvpList(AAAAvpParseType pt) {
            parseType = pt;
        }
        virtual ~AAAQualifiedAvpList() {
            for (iterator i=begin(); i!=end(); i++) {
                delete *i;
            }
        }
        void add(AAAQualifiedAVP* q_avp) {
            push_back(q_avp);
        }
        inline AAAAvpParseType& getParseType(void) {
            return parseType;
        }
        virtual ACE_UINT32 getMinSize(void) = 0;

    private:
        AAAAvpParseType parseType;
};

template<class COMMAND>
class AAACommandList :
    public std::list<COMMAND*>
{
    public:
        void add(COMMAND *com) {
            if (search(com->name.c_str()) != NULL) {
                AAA_LOG((LM_ERROR, "Attempt to load duplicate command definition.\n"));
                exit(1);
            }
            mutex.acquire();
            push_back(com);
            mutex.release();
        }
        virtual COMMAND* search(const char*name) {
            mutex.acquire();
            typename std::list<COMMAND*>::iterator c = this->begin();
            for (; c != this->end(); c++) {
                if ((*c)->name == std::string(name)) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    protected:
        AAACommandList() {
        }
        virtual ~AAACommandList() {
        }

    protected:
        ACE_Thread_Mutex mutex;
};

template<class GROUP>
class AAAGroupedAvpList :
    public std::list<GROUP*>
{
    public:
        void add(GROUP* gavp) {
            if (search(gavp->code, gavp->vendorId) != NULL) {
                AAA_LOG((LM_ERROR, "duplicated grouped AVP.\n"));
                exit(1);
            }
            mutex.acquire();
            push_back(gavp);
            mutex.release();
        }
        virtual GROUP* search(ACE_UINT32 code,
                              ACE_UINT32 vendorId) {
            mutex.acquire();
            typename std::list<GROUP*>::iterator c=this->begin();
            for (; c!=this->end(); c++) {
                if ((*c)->code == code && (*c)->vendorId == vendorId) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    protected:
        AAAGroupedAvpList() {
        }
        virtual ~AAAGroupedAvpList() {
        }

    protected:
        ACE_Thread_Mutex mutex;
};

#endif // __AAA_DICTIONARY_API_H__
