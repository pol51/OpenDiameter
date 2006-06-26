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

#ifndef __RADIUS_DICTIONARY_H__
#define __RADIUS_DICTIONARY_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "radius_defs.h"
#include "od_utl_rbtree_dbase.h"

typedef std::list<std::string> RADIUS_DictDataTypeNames;

class RADIUS_DictAttribute
{
   public:
      typedef enum {
         NONE = 0,
         ENCRYPT = 0x1,
      };

   public:
      RADIUS_DictAttribute() :
         m_AttrType(0) {
      }
      std::string &Name() {
         return m_Name;
      }
      std::string &DataType() {
         return m_DataType;
      }
      unsigned int &AttrType() {
         return m_AttrType;
      }
      RADIUS_OCTET &Flags() {
         return m_Flags;
      }

   protected:
      std::string m_Name;
      std::string m_DataType;
      unsigned int m_AttrType;
      RADIUS_OCTET m_Flags;
};

typedef OD_Utl_DbaseTree<std::string,
                         RADIUS_DictAttribute> 
                         RADIUS_AttributeTreeByName;
typedef OD_Utl_DbaseTree<int,
                         RADIUS_DictAttribute> 
                         RADIUS_AttributeTreeByType;

class RADIUS_DictValue
{
   public:
      RADIUS_DictValue() :
         m_Value(0) {
      }
      std::string &Name() {
         return m_Name;
      }
      std::string &AttrName() {
         return m_AttrName;
      }
      int &Value() {
         return m_Value;
      }

   protected:
      std::string m_Name;
      std::string m_AttrName;
      int m_Value;
};

typedef OD_Utl_DbaseTree<std::string,
                         RADIUS_DictValue> 
                         RADIUS_ValueTreeByName;

class RADIUS_DictAttrCollection
{
   public:
      RADIUS_AttributeTreeByName &AttrByName() {
	 return m_AttribsByName;
      }
      RADIUS_AttributeTreeByType &AttrByType() {
	 return m_AttribsByType;
      }
      RADIUS_ValueTreeByName &ValueByName() {
	 return m_ValuesByName;
      }
      std::string &Name() {
	 return m_Name;
      }
      int &Id() {
	 return m_Id;
      }      

   private:
      int m_Id;
      std::string m_Name;
      RADIUS_AttributeTreeByName m_AttribsByName;
      RADIUS_AttributeTreeByType m_AttribsByType;
      RADIUS_ValueTreeByName m_ValuesByName;
};

typedef OD_Utl_DbaseTree<int, RADIUS_DictAttrCollection> 
                         RADIUS_VendorsAttributes;

class RADIUS_XMLDictionary;
class RADIUS_Dictionary
{
   public:
      typedef enum {
         ENTRY_NOT_FOUND = -100
      };

   public:
      void Load(std::string &fname);

      RADIUS_DictAttribute &Lookup(std::string &attrName) {
         return *(m_BaseAttr.AttrByName().Search(attrName));
      }
      RADIUS_DictAttribute &Lookup(int type) {
         return *(m_BaseAttr.AttrByType().Search(type));
      }
      RADIUS_DictAttribute &Lookup(std::string &attrName,
                                   int vendorId) {
         RADIUS_DictAttrCollection *c = m_VendorsAttr.Search(vendorId);
         return *(c->AttrByName().Search(attrName));
      }
      RADIUS_DictAttribute &Lookup(int type,
                                   int vendorId) {
         RADIUS_DictAttrCollection *c = m_VendorsAttr.Search(vendorId);
         return *(c->AttrByType().Search(type));
      }
      RADIUS_DictValue &LookupValue(std::string &valueName) {
         return *(m_BaseAttr.ValueByName().Search(valueName));
      }
      RADIUS_DictValue &LookupValue(std::string &valueName,
                                    int vendorId) {
         RADIUS_DictAttrCollection *c = m_VendorsAttr.Search(vendorId);
         return *(c->ValueByName().Search(valueName));
      }

   private:
      RADIUS_DictDataTypeNames m_DataTypes;
      RADIUS_DictAttrCollection m_BaseAttr;
      RADIUS_VendorsAttributes m_VendorsAttr;

      friend class RADIUS_XMLDictionary;
      friend class ACE_Singleton<RADIUS_Dictionary, 
	                         ACE_Null_Mutex>;

      RADIUS_Dictionary() {
      }
      ~RADIUS_Dictionary() {
      }
};

typedef ACE_Singleton<RADIUS_Dictionary, 
                      ACE_Null_Mutex> 
                      RADIUS_Dict_S;

#define RADIUS_DICT_LOAD(x)           RADIUS_Dict_S::instance()->Load((x))
#define RADIUS_DICT_ATTR(x)           RADIUS_Dict_S::instance()->Lookup((x))
#define RADIUS_DICT_VENDOR_ATTR(x,y)  RADIUS_Dict_S::instance()->Lookup((x),(y))
#define RADIUS_DICT_VALUE(x)          RADIUS_Dict_S::instance()->LookupValue((x))
#define RADIUS_DICT_VENDOR_VALUE(x,y) RADIUS_Dict_S::instance()->LookupValue((x),(y))

#endif // __RADIUS_DICTIONARY_H__

