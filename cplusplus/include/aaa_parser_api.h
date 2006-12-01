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

#ifndef __AAA_PARSER_API_H__
#define __AAA_PARSER_API_H__

// Local dependencies
#include "aaa_dictionary_api.h"

// Boost dependencies
#include <boost/function/function0.hpp>
#include <boost/function/function1.hpp>

/*!
 *==================================================
 * AAA Parser AVP data structures
 *
 * The generic AAA parser data structure is as follows:
 *
 *   AAAAvpContainerList
 *        |
 *        +-- AAAAvpContainer
 *        |       |
 *        |       +-- AAAAvpContainerEntry
 *        |       |
 *        |       +-- AAAAvpContainerEntry
 *        |       |
 *        |       +-- ...
 *        |
 *        +-- AAAAvpContainer
 *        |       |
 *        |       +-- AAAAvpContainerEntry
 *        |       |
 *        |       +-- AAAAvpContainerEntry
 *        |       |
 *        |       +-- ...
 *        |
 *        +-- ...
 *
 *
 * This section pertains to memory management mechanisms utilized by
 * AAA parser. The scheme is a pool based method where there is an
 * expected accumulation of memory block for more efficient reuse.
 *==================================================
 */

/*! \brief ContainerEntry Container Entry
 *
 * A union structure to store a value of any type of AVP.  
 */
class AAAAvpContainerEntry
{
        friend class AAAAvpContainerEntryMngr; /**< entry manager */
        friend class AAAAvpContainer; /**< container */

    public:
        /*!
        * Returns a the type of the entry
        */
        AAAAvpDataType& dataType() { 
           return this->type; 
        }

        /*!
        *  Returns a type-specific reference to data.  The argument is a
        * dummy argument that is used by complier to uniquely chose a
        * desired template function.
        */
        template <class T> inline T& dataRef(Type2Type<T>) {
            return *((T*)data);
        }

        /*!
        *  Returns a type-specific pointer to data.  The argument is a
        * dummy argument that is used by complier to uniquely chose a
        * desired template function.
        */
        template <class T> inline T* dataPtr(Type2Type<T>) {
            return (T*)data;
        }

    protected:
        /*! Constuctor. */
        AAAAvpContainerEntry(int type) : 
           type((AAAAvpDataType)type) {
        }

        /*! Destructor */
        virtual ~AAAAvpContainerEntry() {
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate
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

    protected:
        AAAAvpDataType  type;   /**< AVP type */
        void*           data;   /**< User-defined type data */
};

/*! \brief AAATypeSpecificAvpContainerEntry Type specific container entry
 *
 * Use this template for defining type-specific container entries.
 */
template <class T>
class AAATypeSpecificAvpContainerEntry :
    public AAAAvpContainerEntry
{
    public:
        /*!
        *  Constructor with assigning a specific type value.
        *
        * \param type AVP type
        */
        AAATypeSpecificAvpContainerEntry(int type) :
            AAAAvpContainerEntry(type) {
            data = (new(AAAMemoryManager_S::instance()->malloc(sizeof(T))) T);
        }

        /*!
        *  Destructor.
        */
        virtual ~AAATypeSpecificAvpContainerEntry() {
            ((T*)data)->T::~T();
            AAAMemoryManager_S::instance()->free(data);
        }

        /*!
        * Returns a type-specific pointer to data.
        */
        inline T* dataPtr() const {
            return (T*)data; 
        }

        /*!
        * Returns a type-specific reference to data.  
        */
        inline T& dataRef() const { 
            return *((T*)data);
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate
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
};

/*! A template functor class for type-specific AVP container entry
  creator. */
template <class T>
class AAAAvpContainerEntryCreator
{
    public:
        /*!
        * Abstracted new operator
        *
        * \param type entry type
        */
        AAAAvpContainerEntry* operator()(AAAAvpDataType type) {
            return new T(type);
        }
};

/*! Functor type definition for AVP container entry creator.  The
 *  creator function takes an integer argument and returns a pointer
 *  to an AVP container entry.
 */
typedef boost::function1<AAAAvpContainerEntry*,
                         AAAAvpDataType> AAAAvpContainerEntryFunctor;

/*!\brief AvpContainer AVP Container Definition
 *
 * A class used for passing AVP data between applications and the API.
 */
class AAAAvpContainer :
    public std::vector<AAAAvpContainerEntry*>
{
        friend class AAAAvpContainerList; /**< container list */

    public:
        /*!
        * constructor
        */
        AAAAvpContainer() :
            flag(false),
            parseType(AAA_PARSE_TYPE_OPTIONAL) {
        }

        /*!
        * destructor
        */
        virtual ~AAAAvpContainer() {
        }

        /*!
        * This function returns all the AAAAvpContainerEntry pointers in
        * the container to the free list.  It is the responsibility of
        * applications to call this function as soon as processing of the
        * containers are completed.
        */
        void releaseEntries() {
            for (unsigned i=0; i < size(); i++) {
                delete (*this)[i];
            }
        }

        /*!
        * This function adds a AAAAvpContainerEntry pointer to the
        * container.
        *
        * \param e AVP entry to add
        */
        void add(AAAAvpContainerEntry* e) {
            resize(size()+1, e);
        }

        /*!
        * This function removes a AAAAvpContainerEntry pointer from the
        * container.
        *
        * \param e AVP entry to remove
        */
        void remove(AAAAvpContainerEntry* e) {
            for (iterator i = begin(); i != end(); i++) {
                if (*i == e) {
                    erase(i);
                    break;
                }
            }
        }

        /*!
        * Access method to retrive the AVP name.
        * Returns character string name of the AVP.
        */
        const char* getAvpName() const {
            return avpName.c_str();
        }

        /*!
        * Access method to set the AVP name.
        *
        * \param name New AVP name
        */
        void setAvpName(const char* name) {
            avpName.assign(name);
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate
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

        /*!
        * Returns a reference to the AVP type
        */
        AAAAvpParseType& ParseType() {
            return parseType;
        }

    protected:
        bool flag;                   /**< Internal use */
        AAAAvpParseType parseType;        /**< Parse type (strict or loose) */
        std::string avpName;         /**< AVP name */
};

/*! \brief AvpContainerList AVP Container List
 *
 * A class that defines a list of AAAAvpContainer and functions 
 * for manipulating the list.
 */
class AAAAvpContainerList :
    public std::list<AAAAvpContainer*>
{
    public:
        /*!
        * constructor
        */
        AAAAvpContainerList() {
        }

        /*!
        * destructor
        */
        virtual ~AAAAvpContainerList() {
            this->releaseContainers();
        }

        /*!
        * This function appends the specified container to the internal list.
        *
        * \param c AVP Container to add
        */
        inline void add(AAAAvpContainer* c) {
            push_back(c);
        }

        /*!
        * This function prepends the specified container to the internal list.
        *
        * \param c AVP Container to prepend
        */
        inline void prepend(AAAAvpContainer* c) {
            push_front(c);
        }

        /*!
        * This function returns all the containers in the list to the free
        * list.  It is the responsibility of applications to call
        * this function as soon as processing of the containers are completed.
        */
        void releaseContainers() {
            for (iterator i = begin(); i != end(); i++) {
                AAAAvpContainer *c = *i;
                c->releaseEntries();
                delete c;
            }
            erase(begin(), end());
        }

        /*!
        * This function searches the internal list for a container
        * corresponding to the specified name.
        *
        * \param name AVP name to search
        */
        AAAAvpContainer* search(const char* name) {
            for (iterator i = begin(); i != end(); i++) {
                AAAAvpContainer *c = *i;
                if (ACE_OS::strcmp(c->getAvpName(), name) == 0) {
                    return c;
                }
            }
            return NULL;
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate
        */
        void* operator new(size_t s) {
            return AAAMemoryManager_S::instance()->malloc(s);
        }

        /*!
        * Abstracted new operator
        *
        * \param s size of block to allocate (proxy only)
        * \param p pointer to existing block 
        */
        void* operator new(size_t s, void*p) {
            return p;
        }

        /*!
        * Abstracted delete operator
        *
        * \param p data block to free
        */
        void operator delete(void *p) {
            AAAMemoryManager_S::instance()->free(p);
        }

        /*!
        * Abstracted delete operator
        *
        * \param p data block to free
        * \param q opaque data
        */
        void operator delete(void *p, void*q) {
            AAAMemoryManager_S::instance()->free(p);
        }

        /*!
        * Resets this container for re-parsing
        *
        */
        void reset() {
            for (iterator i = begin(); i != end(); i++) {
                AAAAvpContainer *c = *i;
                c->flag = false;
                for (unsigned int j=0; j < c->size(); j++) {
                    if (AAA_AVP_GROUPED_TYPE == (*c)[j]->dataType()) {
                        (*c)[j]->dataRef
                            (Type2Type<AAAAvpContainerList>()).reset();
                    }
                }
            }
        }

    private:
        AAAAvpContainer* search(const char* name, bool flg) {
            for (iterator i = begin(); i != end(); i++) {
                AAAAvpContainer *c = *i;
                if (c->flag == flg &&
                    ACE_OS::strcmp(c->getAvpName(), name) == 0) {
                    return c;
                }
            }
            return NULL;
        }

        AAAAvpContainer* search(bool flg) {
            for (iterator i = begin(); i != end(); i++) {
                AAAAvpContainer *c = *i;
                if (c->flag == flg) {
                    return c;
                }
            }
            return NULL;
        }
};

/*!
 *==================================================
 * AAA Parser AVP type structures
 * The generic AAA parser avp type structure is as follows:
 *
 *   AAAAvpTypeList
 *        |
 *        +-- AAAAvpType
 *        |
 *        +-- AAAAvpType
 *        |
 *        +-- ...
 *
 *==================================================
 */

/*! This class defines an AVP type.  An AVP type is defined as a set
 *  of typename, typevalue, size of the AVP payload.
 */
class AAAAvpType
{
    public:
        /*!
        * constructor
        *
        * \param name AVP name
        * \param type AVP type
        * \param size AVP data size
        * \param containerEntryCreator entry creator object
        */
        AAAAvpType(char* name,
                   AAAAvpDataType type,
                   ACE_UINT32 size,
                   AAAAvpContainerEntryFunctor creator) :
            name(name),
            type(type),
            size(size),
            containerEntryCreator(creator) {
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
        AAAAvpDataType getType(void) {
            return type;
        }

        /*!
        * This function is used for obtaining the minimum AVP payload size.
        */
        ACE_UINT32 getMinSize(void) {
            return size; 
        }

        /*!
         * Container entry creator
         */
        AAAAvpContainerEntry *createContainerEntry() {
            return containerEntryCreator(type);
        }

    private:
        char *           name;  /**< name of the avp type */
        AAAAvpDataType   type;  /**< enumerate type */
        ACE_UINT32       size;  /**< minimum size of this avp payload (0 means variable size) */
        AAAAvpContainerEntryFunctor containerEntryCreator; /**< Container entry creator */
};

class AAAAvpTypeList :
    public std::list<AAAAvpType*>
{
    public:
        /*!
        * Returns a pointer to the AVP type instance
        *
        * \param type AVP type
        */
        AAAAvpType* search(ACE_UINT32 type) {
            for (iterator i = begin(); i != end(); i ++) {
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
        AAAAvpType* search(const char* name) {
            for (iterator i = begin(); i != end(); i ++) {
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
        void add(AAAAvpType* avpType) {
            mutex.acquire();
            push_back(avpType);
            mutex.release();
        }

    private:
        ACE_Thread_Mutex mutex; /**< mutex protector */
};

/*!
 *==================================================
 * Allocations management for AAAAvpContainerEntry
 * and AAAAvpContainer
 *==================================================
 */

/*! \brief AAAAvpContainerEntryMngr AVP Container Entry Manager
 *
 * AAAAvpContainerEntryMngr and AAAAvpContainerMngr is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */
class AAAAvpContainerEntryMngr
{
    public:
        /*!
        * constructor
        */
        AAAAvpContainerEntryMngr(AAAAvpTypeList &lst) :
            typeList(lst) {
        }

        /*!
        * This function assigns a AAAAvpContainerEntry resource of a
        * specific type.
        *
        * \param type Data type to acquire
        */
        AAAAvpContainerEntry *acquire(AAAAvpDataType type) {
            AAAAvpContainerEntry *e;
            // Search creator;
            AAAAvpType *avpType =  typeList.search(type);
            if (avpType == NULL) {
                AAAErrorCode cd;
                AAA_LOG((LM_ERROR, "Pre-defined type not found", type));
                cd.set(AAA_PARSE_ERROR_TYPE_BUG,
                       AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM);
                throw cd;
            }
            e = avpType->createContainerEntry();
            return e;
        }

        /*!
        * This function release a AAAAvpContainerEntry resource.
        *
        * \param entry AVP Container entry
        */
        inline void release(AAAAvpContainerEntry* entry) {
            delete entry;
        }

    private:
        AAAAvpTypeList &typeList;
};

/*! \brief AAAAvpContainerMngr AVP Container Manager
 *
 * AAAAvpContainerEntryMngr and AAAAvpContainerMngr is used for
 * assigning and releasing AAAAvpContainerEntry and AAAAvpContainer
 * resources.   Managing policy for assigning and releasing the
 * resources is concealed within the class definition.  In other
 * words, any kind of managing policy is applicable.
 */
class AAAAvpContainerMngr
{
    public:
        /*!
        * This function assigns a AAAAvpContainer resource of a
        * specific name.
        *
        * \param name AVP name
        */
        AAAAvpContainer *acquire(const char* name) {
            AAAAvpContainer *c = new AAAAvpContainer;
            c->setAvpName(name);
            return (c);
        }

        /*!
        * This function release a AAAAvpContainer resource.
        *
        * \param entry AVP entry
        */
        void release(AAAAvpContainer* entry) {
            delete entry;
        }
};

/*! \brief Generic AVP widget allocator
 *
 *  This template class is a wrapper class format
 *  the most common AVP operations. Users should
 *  use this class for manipulating AVP containers.
 */
template<typename D, AAAAvpDataType t, typename EM>
class AAAAvpWidget {
    public:
        AAAAvpWidget(char *name) {
            AllocContainer(name);
        }
        AAAAvpWidget(char *name, D &value) {
            AllocContainer(name);
            Get() = value;
        }
        AAAAvpWidget(AAAAvpContainer *avp) :
            m_cAvp(avp) {
            m_refCount = 1;
        }
        virtual ~AAAAvpWidget() {
            DeAllocContainer();
        }
        D &Get() {
            EM em;
            AAAAvpContainerEntry *e = em.acquire(t);
            m_cAvp->add(e);
            m_refCount ++;
            return e->dataRef(Type2Type<D>());
        }
        AAAAvpContainer *operator()() {
            m_refCount ++;
            return m_cAvp;
        }
        bool empty() {
            return (m_cAvp->size() == 0);
        }

    protected:
        void AllocContainer(char *name) {
            m_refCount = 0;
            AAAAvpContainerMngr cm;
            m_cAvp = cm.acquire(name);
        }
        void DeAllocContainer() {
            // Use refCount to de-alloc un-used container
            // If a reference was made to the container
            // we assume it is being used
            if (this->empty() && (m_refCount == 0)) {
               AAAAvpContainerMngr cm;
               cm.release(m_cAvp);
            }
        }

    private:
        AAAAvpContainer *m_cAvp;
        unsigned short m_refCount;
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
template<typename D, AAAAvpDataType t, typename EM>
class AAAAvpContainerWidget
{
    public:
       AAAAvpContainerWidget(AAAAvpContainerList &lst) :
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
              AAAAvpWidget<D, t, EM> avpWidget(name);
              list.add(avpWidget());
              return avpWidget.Get();
          }
          else if ((c->size() == 0) || append) {
              AAAAvpWidget<D, t, EM> avpWidget(c);
              return avpWidget.Get();
          }
          else {
              return (*c)[0]->dataRef(Type2Type<D>());
          }
       }
       void AddAvp(AAAAvpContainerWidget<D, t, EM> &avp) {
           list.add(avp());
       }
       void DelAvp(char *name) {
          std::list<AAAAvpContainer*>::iterator i;
          for (i=list.begin(); i!=list.end();i++) {
              AAAAvpContainer *c = *i;
              if (ACE_OS::strcmp(c->getAvpName(), name) == 0) {
                  list.erase(i);
                  AAAAvpContainerMngr cm;
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
 * This section defines the generic parsing template
 *==================================================
 */

/*! \brief Parser Message Parser Definition
 *
 * AAAParser is a template class for generic parser.
 */
template <class RAWDATA,
          class APPDATA,
          class DICTDATA = AAAEmptyClass>
class AAAParser
{
    public:
        /*!
        * constructor
        */
        AAAParser() {
        }

        /*!
        * destructor
        */
        virtual ~AAAParser() {
        }

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
        void setRawData(RAWDATA data) {
            rawData = data;
        }

        /*!
        * Set application level data to the parser.
        *
        * \param data Parser Data
        */
        void setAppData(APPDATA data) {
            appData = data;
        }

        /*!
        * Set dictionary data to the parser.
        *
        * \param data Dictionary data
        */
        void setDictData(DICTDATA data) {
            dictData = data;
        }

        /*!
        * Get raw data from the parser.
        */
        RAWDATA getRawData() {
            return rawData;
        }

        /*! This template is used for obtaining raw data casted to a
        * specific type.
        */
        template <class T> void getRawData(T*& data) {
            data = (T*)rawData;
        }

        /*!
        * Get application level data from the parser.
        */
        APPDATA getAppData() {
            return appData;
        }

        /*! This template is used for obtaining application data
        *  casted to a * specific type.
        */
        template <class T> void getAppData(T*& data) {
            data = (T*)appData;
        }

        /*!
        * Get dictionary data from the parser.
        */
        DICTDATA getDictData() {
            return dictData; 
        }

        /*! This template is used for obtaining dictionary data
        *  casted to a * specific type.
        */
        template <class T> void getDictData(T*& data) {
            data = (T*)dictData;
        }

    private:
        RAWDATA      rawData;   /**< Raw data  */
        APPDATA      appData;   /**< Application data translated from/to raw data */
        DICTDATA     dictData;  /**< Dictionary data */
};

#endif // __AAA_PARSER_API_H__
