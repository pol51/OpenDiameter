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

#ifndef __OD_UTL_RBTREE_H__
#define __OD_UTL_RBTREE_H__

#include "od_utl_exports.h"

typedef enum {
   RBTREE_CLR_BLACK,
   RBTREE_CLR_RED
} OD_Utl_RbTreeNodeColor;

class OD_UTL_EXPORT OD_Utl_RbTreeData 
{
   public:
      // constructor
      OD_Utl_RbTreeData() { payload = this; };
      virtual ~OD_Utl_RbTreeData() {};
  
      // exposed functions
      void *payload;
  
      // methods
      virtual int operator < (OD_Utl_RbTreeData &cmp) = 0;
      virtual int operator == (OD_Utl_RbTreeData &cmp) = 0;
      virtual void clear(void *userData = 0) = 0;

      // debugging only
      virtual void Dump(void *userData) {};
};

class OD_UTL_EXPORT OD_Utl_RbTreeNode {
   public:
      // constructor
      OD_Utl_RbTreeNode();
  
      // exposed members
      OD_Utl_RbTreeNode *left;
      OD_Utl_RbTreeNode *right;
      OD_Utl_RbTreeNode *parent;
      OD_Utl_RbTreeNodeColor color;
      OD_Utl_RbTreeData *data;

      // methods
      OD_Utl_RbTreeNode &operator=(OD_Utl_RbTreeNode &source);
};

class OD_UTL_EXPORT OD_Utl_RbTreeTree {
   public:
      // constructor
      OD_Utl_RbTreeTree() { root = &nil; };

      // methods
      OD_Utl_RbTreeData *Insert(OD_Utl_RbTreeData *data);
      OD_Utl_RbTreeData *Find(OD_Utl_RbTreeData *data);
      OD_Utl_RbTreeData *Remove(OD_Utl_RbTreeData *data);
      void Clear(void *userData = 0);

      // debugging only
      void Dump(OD_Utl_RbTreeNode *x = 0, void *userData = 0);

   private:
      OD_Utl_RbTreeNode *root;
      OD_Utl_RbTreeNode nil;

      void FixupInsert(OD_Utl_RbTreeNode *x);
      void FixupRemove(OD_Utl_RbTreeNode *x);
      void RotateLeft(OD_Utl_RbTreeNode *x);
      void RotateRight(OD_Utl_RbTreeNode *x);
};

#endif /* __OD_UTL_RBTREE_H__ */
