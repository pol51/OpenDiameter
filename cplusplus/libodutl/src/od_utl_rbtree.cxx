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

#include "od_utl_rbtree.h"

OD_Utl_RbTreeNode &OD_Utl_RbTreeNode::operator=
          (OD_Utl_RbTreeNode &source)
{
   left = source.left;
   right = source.right;
   parent = source.parent;
   color = source.color;
   data = source.data;
   return (*this);
}

OD_Utl_RbTreeNode::OD_Utl_RbTreeNode()
{
   left = right = this;
   parent = (0);
   color = RBTREE_CLR_BLACK;
   data = (0);
}

void OD_Utl_RbTreeTree::RotateLeft(OD_Utl_RbTreeNode *x)
{
    OD_Utl_RbTreeNode *y = x->right;

    // establish x->right link 
    x->right = y->left;
    if (y->left != &nil) {
        y->left->parent = x;
    }

    // establish y->parent link 
    if (y != &nil) {
        y->parent = x->parent;
    }
    if (x->parent) {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    } else {
        root = y;
    }

    // link x and y 
    y->left = x;
    if (x != &nil) {
        x->parent = y;
    }
}

void OD_Utl_RbTreeTree::RotateRight(OD_Utl_RbTreeNode *x)
{
    OD_Utl_RbTreeNode *y = x->left;

    // establish x->left link
    x->left = y->right;
    if (y->right != &nil) {
        y->right->parent = x;
    }

    // establish y->parent link 
    if (y != &nil) {
        y->parent = x->parent;
    }
    if (x->parent) {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    } else {
        root = y;
    }

    // link x and y 
    y->right = x;
    if (x != &nil) {
        x->parent = y;
    }
}

void OD_Utl_RbTreeTree::FixupRemove(OD_Utl_RbTreeNode *x)
{
    while (x != root && x->color == RBTREE_CLR_BLACK) {
        if (x == x->parent->left) {
            OD_Utl_RbTreeNode *w = x->parent->right;
            if (w->color == RBTREE_CLR_RED) {
                w->color = RBTREE_CLR_BLACK;
                x->parent->color = RBTREE_CLR_RED;
                RotateLeft (x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RBTREE_CLR_BLACK && 
                w->right->color == RBTREE_CLR_BLACK) {
                w->color = RBTREE_CLR_RED;
                x = x->parent;
            } else {
                if (w->right->color == RBTREE_CLR_BLACK) {
                    w->left->color = RBTREE_CLR_BLACK;
                    w->color = RBTREE_CLR_RED;
                    RotateRight (w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RBTREE_CLR_BLACK;
                w->right->color = RBTREE_CLR_BLACK;
                RotateLeft (x->parent);
                x = root;
            }
        } else {
            OD_Utl_RbTreeNode *w = x->parent->left;
            if (w->color == RBTREE_CLR_RED) {
                w->color = RBTREE_CLR_BLACK;
                x->parent->color = RBTREE_CLR_RED;
                RotateRight (x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RBTREE_CLR_BLACK && 
                w->left->color == RBTREE_CLR_BLACK) {
                w->color = RBTREE_CLR_RED;
                x = x->parent;
            } else {
                if (w->left->color == RBTREE_CLR_BLACK) {
                    w->right->color = RBTREE_CLR_BLACK;
                    w->color = RBTREE_CLR_RED;
                    RotateLeft (w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RBTREE_CLR_BLACK;
                w->left->color = RBTREE_CLR_BLACK;
                RotateRight (x->parent);
                x = root;
            }
        }
    }
    x->color = RBTREE_CLR_BLACK;
}

void OD_Utl_RbTreeTree::FixupInsert(OD_Utl_RbTreeNode *x)
{
    // check Red-Black properties
    while (x != root && x->parent->color == RBTREE_CLR_RED) {
        // violation
        if (x->parent == x->parent->parent->left) {
            OD_Utl_RbTreeNode *y = x->parent->parent->right;
            if (y->color == RBTREE_CLR_RED) {

                // uncle is RBTREE_CLR_RED
                x->parent->color = RBTREE_CLR_BLACK;
                y->color = RBTREE_CLR_BLACK;
                x->parent->parent->color = RBTREE_CLR_RED;
                x = x->parent->parent;
            } else {

                // uncle is RBTREE_CLR_BLACK
                if (x == x->parent->right) {
                    // make x a left child
                    x = x->parent;
                    RotateLeft(x);
                }

                // recolor and rotate 
                x->parent->color = RBTREE_CLR_BLACK;
                x->parent->parent->color = RBTREE_CLR_RED;
                RotateRight(x->parent->parent);
            }
        } else {

            // mirror image of above code 
            OD_Utl_RbTreeNode *y = x->parent->parent->left;
            if (y->color == RBTREE_CLR_RED) {

                // uncle is RBTREE_CLR_RED
                x->parent->color = RBTREE_CLR_BLACK;
                y->color = RBTREE_CLR_BLACK;
                x->parent->parent->color = RBTREE_CLR_RED;
                x = x->parent->parent;
            } else {

                // uncle is RBTREE_CLR_BLACK 
                if (x == x->parent->left) {
                    x = x->parent;
                    RotateRight(x);
                }
                x->parent->color = RBTREE_CLR_BLACK;
                x->parent->parent->color = RBTREE_CLR_RED;
                RotateLeft(x->parent->parent);
            }
        }
    }
    root->color = RBTREE_CLR_BLACK;
}

OD_Utl_RbTreeData *OD_Utl_RbTreeTree::Insert(OD_Utl_RbTreeData *data)
{
    OD_Utl_RbTreeNode *parent = (0), *current = root;

    while (current != &nil) {
       if (*(current->data) == (*data)) {
           return (current->data);
       }
       parent = current;
       current = ((*data) < (*(current->data))) ?
           current->left : current->right;
    }

    current = new OD_Utl_RbTreeNode;
    if (! current) {
        return (0);
    }

    current->left = current->right = &nil;
    current->parent = parent;
    current->color = RBTREE_CLR_RED;
    current->data = data;

    if (parent) {
        if ((*data) < (*(parent->data))) {
            parent->left = current;
        }
        else {
           parent->right = current;
        }
    }
    else {      
        root = current;
    }

    FixupInsert(current);
    return (current->data);
}

OD_Utl_RbTreeData *OD_Utl_RbTreeTree::Find(OD_Utl_RbTreeData *data)
{
    OD_Utl_RbTreeNode *current = root;
    
    while (current != &nil) {
       if (*(current->data) == (*data)) {
           return (current->data);
       }
       current = ((*data) < (*(current->data))) ?
           current->left : current->right;
    }

    return (0);
}

OD_Utl_RbTreeData *OD_Utl_RbTreeTree::Remove(OD_Utl_RbTreeData *data)
{
    OD_Utl_RbTreeData *val;
    OD_Utl_RbTreeNode *x, *y, *z = root;

    while (z != &nil) {
       if (*(z->data) == (*data)) {
           break;
       }
       z = ((*data) < (*(z->data))) ? z->left : z->right;
    }
    
    if (!z || z == &nil) {
        return (0);
    }

    if (z->left == &nil || z->right == &nil) {
        // y has a nil node as a child 
        y = z;
    } else {
        // find tree successor with a nil node as a child 
        y = z->right;
        while (y->left != &nil) {
           y = y->left;
        }
    }

    // x is y's only child
    if (y->left != &nil) {
        x = y->left;
    } else {
        x = y->right;
    }

    // remove y from the parent chain
    x->parent = y->parent;
    if (y->parent) {
        if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
    } else {
        root = x;
    }

    if (y != z) {
        val = z->data;
        z->data = y->data;
    }
    else {
        val = y->data;
    }

    if (y->color == RBTREE_CLR_BLACK) {
        FixupRemove(x);
    }

    delete y;
    return (val);
}

void OD_Utl_RbTreeTree::Clear(void *userData)
{
    OD_Utl_RbTreeData *val;
    while (root != &nil) {
       val = OD_Utl_RbTreeTree::Remove(root->data);
       val->clear(userData);
       delete val;
    }
}

void OD_Utl_RbTreeTree::Dump(OD_Utl_RbTreeNode *x, 
                             void *userData)
{
    if (x == 0) {
        x = root;
    }
    if (x == &nil) {
        return;
    }
    if (x->left != &nil) {
        Dump(x->left, userData);
    }
    if (x->data) {
        x->data->Dump(userData);
    } else {
    }
    if (x->right != &nil) {
        Dump(x->right, userData);
    }
}
