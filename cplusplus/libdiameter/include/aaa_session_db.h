/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
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

#ifndef __AAA_SESSION_DB_H__
#define __AAA_SESSION_DB_H__

#include <deque>
#include "framework.h"
#include "od_utl_rbtree.h"
#include "aaa_session.h"
#include "ace/RW_Mutex.h"

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionEntry : 
     public DiameterSessionCounter 
{
     public:
        DiameterSessionEntry(AAA_JobData &data) :
            m_Data(data) {
        }
        AAA_JobData &Data() {
            return m_Data;
        }
        int operator=(DiameterSessionEntry &cntr) {
	    m_Data = cntr.Data();
            (DiameterSessionCounter&)(*this) = 
                (DiameterSessionCounter&)cntr;
            return (true);
        }
     private:
        AAA_JobData &m_Data;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionEntryList :
     public std::list<DiameterSessionEntry*>
{
     public:
        DiameterSessionEntryList();
        bool Add(DiameterSessionId &id, 
                 AAA_JobData &data);
        bool Lookup(DiameterSessionId &id,
                    AAA_JobData *&data);
        bool Remove(DiameterSessionId &id);
        void Flush();
     protected:
        typedef std::list<DiameterSessionEntry*>::iterator 
            EntryIterator;
     private:
        DiameterSessionCounter m_LastKnownCounter;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionEntryNode : 
     public OD_Utl_RbTreeData 
{
     public:
        DiameterSessionEntryNode(std::string &id, std::string &opt) {
            m_DiameterId = id;
            m_OptionalVal = opt;
        }
        std::string &DiameterId() {
            return m_DiameterId;
        }
        std::string &OptionalVal() {
            return m_OptionalVal;
        }
        DiameterSessionEntryList &EntryList() {
            return m_Entries;
        }
     protected:
        int operator==(OD_Utl_RbTreeData &cmp) {
            DiameterSessionEntryNode *id = (DiameterSessionEntryNode*)cmp.payload;
            if (m_DiameterId == id->DiameterId()) {
                if (m_OptionalVal.length() > 0) {
                    return (m_OptionalVal == id->OptionalVal()) ?
                            true : false;
                }
                return true;
            }
            return false;
        }
        int operator<(OD_Utl_RbTreeData &cmp) {
            DiameterSessionEntryNode *id = (DiameterSessionEntryNode*)cmp.payload;
            if (m_DiameterId < id->DiameterId()) {
                if (m_OptionalVal.length() > 0) {
                    return (m_OptionalVal < id->OptionalVal()) ?
                            true : false;
                }
                return true;
            }
            return false;
        }
        void clear(void *pload) {
            m_Entries.Flush();
        }
     private:
        std::string m_DiameterId;
        std::string m_OptionalVal;
        DiameterSessionEntryList m_Entries;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionDb : 
    private OD_Utl_RbTreeTree
{
    public:
        bool Add(DiameterSessionId &id, AAA_JobData &data);
        bool Lookup(DiameterSessionId &id, AAA_JobData *&data);
        bool Remove(DiameterSessionId &id);

    private:
        ACE_RW_Mutex m_rwMutex;
};

typedef ACE_Singleton<DiameterSessionDb, 
                      ACE_Recursive_Thread_Mutex> 
                      DiameterSessionDb_S;
#define DIAMETER_SESSION_DB() (*DiameterSessionDb_S::instance()) 

#endif /* __AAA_SESSION_DB_H__ */

