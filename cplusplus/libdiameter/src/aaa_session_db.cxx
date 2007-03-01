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

#include "ace/OS.h"
#include "ace/Guard_T.h"
#include "aaa_session_db.h"
#include "aaa_log_facility.h"

DiameterSessionEntryList::DiameterSessionEntryList() 
{
     ACE_Time_Value tm = ACE_OS::gettimeofday();
     m_LastKnownCounter.High() = tm.sec();
     m_LastKnownCounter.Low() = tm.usec();
}

bool DiameterSessionEntryList::Add(DiameterSessionId &id, 
                               AAA_JobData &data)
{
    DiameterSessionEntry *newEntry = new DiameterSessionEntry(data);
    if (newEntry) {
        DiameterSessionCounter nullCounter, *idCounter = &id;
        if (*idCounter == nullCounter) {
            ++ m_LastKnownCounter;
            *((DiameterSessionCounter*)newEntry) = m_LastKnownCounter;
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

bool DiameterSessionEntryList::Lookup(DiameterSessionId &id,
                                  AAA_JobData *&data)
{
    for (EntryIterator i = begin(); i != end(); i ++) {
        DiameterSessionCounter *c = *i;
        if (*c == id) {
            data = &(((DiameterSessionEntry*)c)->Data());
            return true;
        }
    }
    return false;
}

bool DiameterSessionEntryList::Remove(DiameterSessionId &id)
{
    for (EntryIterator i = begin(); i != end(); i ++) {
        DiameterSessionCounter *c = *i;
        if (*c == id) {
            erase(i);
            delete c;
            return true;
        }
    }
    return false;
}

void DiameterSessionEntryList::Flush()
{
    while (! empty()) {
        DiameterSessionEntry *e = front();
        pop_front();
        delete e;
    }
}

bool DiameterSessionDb::Add(DiameterSessionId &id, AAA_JobData &data)
{
    ACE_Write_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    DiameterSessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        DiameterSessionEntryNode *prevNode = (DiameterSessionEntryNode*)node;
        AAA_JobData *search;
        if (prevNode->EntryList().Lookup(id, search)) {
            return false;
        }
        return prevNode->EntryList().Add(id, data);
    }
    else {
        DiameterSessionEntryNode *newNode = new DiameterSessionEntryNode
            (id.DiameterId(), id.OptionalValue());
        if (newNode) {
            newNode->EntryList().Add(id, data);
            Insert(newNode);
            return true;
        }
    }
    return false;
}

bool DiameterSessionDb::Lookup(DiameterSessionId &id, AAA_JobData *&data)
{
    ACE_Read_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    DiameterSessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        DiameterSessionEntryNode *prevNode = (DiameterSessionEntryNode*)node;
        return prevNode->EntryList().Lookup(id, data);
    }
    return false;
}

bool DiameterSessionDb::Remove(DiameterSessionId &id)
{
    ACE_Write_Guard<ACE_RW_Mutex> guard(m_rwMutex);
    DiameterSessionEntryNode searchNode
        (id.DiameterId(), id.OptionalValue());
    OD_Utl_RbTreeData *node = Find(&searchNode);
    if (node) {
        DiameterSessionEntryNode *prevNode = (DiameterSessionEntryNode*)node;
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
