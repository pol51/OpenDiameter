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

#include "ace/OS.h"
#include "ace/Guard_T.h"
#include "aaa_session_db.h"
#include "aaa_log_facility.h"

bool AAA_SessionEntryList::Add(AAA_SessionId &id, 
                               AAA_JobData &data)
{
    AAA_SessionEntry *newEntry = new AAA_SessionEntry(data);
    if (newEntry) {   
        AAA_SessionCounter nullCounter, *idCounter = &id;
        if (*idCounter == nullCounter) {
            if (empty()) {
	        ACE_Time_Value tm = ACE_OS::gettimeofday();
                newEntry->High() = tm.sec();
                newEntry->Low() = tm.usec();
            }
            else {
                AAA_SessionEntry *lastEntry = front();
                *newEntry = *lastEntry;
                ++ (*((AAA_SessionCounter*)newEntry));
            }
            id.High() = newEntry->High();
            id.Low() = newEntry->Low();
	}
        else {
	    newEntry->High() = id.High();
            newEntry->Low() = id.Low();
	}
        newEntry->Data() = data;
        push_front(newEntry);
        return true;
    }
    return false;
}

bool AAA_SessionEntryList::Lookup(AAA_SessionId &id,
                                  AAA_JobData *&data)
{
    for (EntryIterator i = begin(); i != end(); i ++) {
        AAA_SessionCounter *c = *i;
        if (*c == id) {
            data = &(((AAA_SessionEntry*)c)->Data());
            return true;
        }
    }
    return false;
}

bool AAA_SessionEntryList::Remove(AAA_SessionId &id)
{
    for (EntryIterator i = begin(); i != end(); i ++) {
        AAA_SessionCounter *c = *i;
        if (*c == id) {
            erase(i);
            delete c;
            return true;
        }
    }
    return false;
}

void AAA_SessionEntryList::Flush()
{
    while (! empty()) {
        AAA_SessionEntry *e = front();
        pop_front();
        delete e;
    }
}

bool AAA_SessionDb::Add(AAA_SessionId &id, AAA_JobData &data)
{
    ACE_Write_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    AAA_SessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        AAA_SessionEntryNode *prevNode = (AAA_SessionEntryNode*)node;
        AAA_JobData *search;
        if (prevNode->EntryList().Lookup(id, search)) {
            return false;
        }
        return prevNode->EntryList().Add(id, data);
    }
    else {
        AAA_SessionEntryNode *newNode = new AAA_SessionEntryNode
            (id.DiameterId(), id.OptionalValue());
        if (newNode) {
            newNode->EntryList().Add(id, data);
            Insert(newNode);
            return true;
        }
    }
    return false;
}

bool AAA_SessionDb::Lookup(AAA_SessionId &id, AAA_JobData *&data)
{
    ACE_Read_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    AAA_SessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        AAA_SessionEntryNode *prevNode = (AAA_SessionEntryNode*)node;
        return prevNode->EntryList().Lookup(id, data);
    }
    return false;
}

bool AAA_SessionDb::Remove(AAA_SessionId &id)
{
    ACE_Write_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    AAA_SessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        AAA_SessionEntryNode *prevNode = (AAA_SessionEntryNode*)node;
        if (prevNode->EntryList().Remove(id)) {
            if (prevNode->EntryList().empty()) {
                node = OD_Utl_RbTreeTree::Remove(&searchNode);
                delete node;
            }
            return true;
        }
    }
    return false;
}
