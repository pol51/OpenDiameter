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

#ifndef __OD_UTL_RBTREE_DBASE_H__
#define __OD_UTL_RBTREE_DBASE_H__

#include "od_utl_rbtree.h"

template<class TYPE, class ARG>
class OD_Utl_DbaseTree : public OD_Utl_RbTreeTree
{
   public:
     typedef enum {
         ENTRY_ALLOC_FAILURE,
         ENTRY_INSERTION_FAILURE,
         ENTRY_NOT_FOUND
     } DB_ERROR;

   protected:
     class OD_Utl_DbEntry : public OD_Utl_RbTreeData {
         public:
            OD_Utl_DbEntry(TYPE &index) : m_Index(index) { 
	    }
            int operator<(OD_Utl_RbTreeData &cmp) {
               OD_Utl_DbEntry &compare = reinterpret_cast<OD_Utl_DbEntry&>(cmp);
               return (m_Index < compare.Index());
            }
            int operator==(OD_Utl_RbTreeData &cmp) {
               OD_Utl_DbEntry &compare = reinterpret_cast<OD_Utl_DbEntry&>(cmp);
               return (m_Index == compare.Index());
            }
            void clear(void *userData = 0) { 
	    }
            TYPE &Index() { 
	       return m_Index; 
	    }
         protected:
            TYPE m_Index;
     };

   public:
      OD_Utl_DbaseTree() { 
      }
      virtual ~OD_Utl_DbaseTree() { 
      }
      void Add(TYPE &index, ARG &arg) {
         OD_Utl_DbEntry *newEntry = new OD_Utl_DbEntry(index);
         if (newEntry) {
            newEntry->payload = reinterpret_cast<void*>(&arg);
            if (Insert(newEntry)) {
               return;
            }
            throw (ENTRY_INSERTION_FAILURE);
         }
         throw (ENTRY_ALLOC_FAILURE);
      }
      void Remove(TYPE &index, ARG **arg = NULL) {
         OD_Utl_DbEntry lookupEntry(index), *searchEntry;
         searchEntry = static_cast<OD_Utl_DbEntry*>
             (OD_Utl_RbTreeTree::Remove(&lookupEntry));
         if (searchEntry) {
            if (arg) {
               *arg = reinterpret_cast<ARG*>(searchEntry->payload);
            }
            return;
         }
         throw (ENTRY_NOT_FOUND);
      }
      ARG *Search(TYPE &index) {
         OD_Utl_DbEntry lookupEntry(index), *searchEntry;
         searchEntry = static_cast<OD_Utl_DbEntry*>
             (OD_Utl_RbTreeTree::Find(&lookupEntry));
         if (searchEntry) {
            return reinterpret_cast<ARG*>(searchEntry->payload);
         }
         throw (ENTRY_NOT_FOUND);
      }
};

#endif // __OD_UTL_RBTREE_DBASE_H__
