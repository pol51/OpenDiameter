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

#include <iostream>
#include "radius_dictionary.h"
#include "od_utl_xml_parser.h"

#define RADIUS_DICT_DEBUG 0

template <class T>
class RADIUS_XMLDataIterator : 
   public OD_Utl_XMLElementParserWithData<T>
{
   public:
      RADIUS_XMLDataIterator(std::string &name, T &data) :
         OD_Utl_XMLElementParserWithData<T>(name, data) {
      }
      virtual int populate(DOMNode *n, std::string &name) {
         DOMNode *sibling = n->getFirstChild();
         while (sibling != NULL) {
             if (sibling->getNodeType() == 
                 DOMNode::ELEMENT_NODE) {
                 char *c_str = XMLString::transcode
                                   (sibling->getNodeName());
                 if (XMLString::compareString
                     (c_str, name.c_str()) == 0) {
                     this->svc(sibling);
                 }
                 XMLString::release(&c_str);
             }
             sibling = sibling->getNextSibling();
         }
         return (0);
      }
};

template<class T, class ARG>
class RADIUS_XMLDataIteratorWithArg : 
   public RADIUS_XMLDataIterator<T>
{
   public:
      RADIUS_XMLDataIteratorWithArg(std::string &name, 
                                    T &data,
                                    ARG &arg) :
	 RADIUS_XMLDataIterator<T>(name, data),
         m_Arg(arg) {
      }
      ARG &Arg() {
	 return m_Arg;
      }      

   protected:
      ARG &m_Arg;
};

class RADIUS_XMLDataTypeNames : 
   public RADIUS_XMLDataIterator<RADIUS_DictDataTypeNames>
{
   public:
      RADIUS_XMLDataTypeNames(std::string &name, 
                              RADIUS_DictDataTypeNames &data) :
	 RADIUS_XMLDataIterator<RADIUS_DictDataTypeNames>
         (name, data) {
      }
      int svc(DOMNode *n) {
         char *c_str = getTextContent(n);
         if (c_str) {
	    std::string newType = c_str;
            m_Payload.push_back(newType);
            XMLString::release(&c_str);
         }
         return (0);
      }
};

class RADIUS_XMLDataValueTree : 
   public RADIUS_XMLDataIterator<RADIUS_ValueTreeByName>
{
   public:
      RADIUS_XMLDataValueTree(std::string &name, 
			      RADIUS_ValueTreeByName &data) :
	 RADIUS_XMLDataIterator<RADIUS_ValueTreeByName>
         (name, data) {
      }
      int svc(DOMNode *n) {
         RADIUS_DictValue *value(new RADIUS_DictValue);

         std::string tagName = "name";
         OD_Utl_XMLDataString name(tagName, value->Name());
         name.populate(n->getFirstChild());

         tagName = "attribute";
         OD_Utl_XMLDataString attr_name(tagName, 
                                        value->AttrName());
         attr_name.populate(n->getFirstChild());

         tagName = "value";
         ACE_UINT32 holder;
         OD_Utl_XMLDataUInt32 content(tagName, holder);
         content.populate(n->getFirstChild());
         value->Value() = holder;
       
         m_Payload.Add(value->Name(), *value);
         print(*value);
         return (0);
      }
      void print(RADIUS_DictValue &v) {
#if RADIUS_DICT_DEBUG
         std::cout << "       Value: " << v.Name() << std::endl;
         std::cout << "              Attribute=" << v.AttrName() << std::endl;
         std::cout << "              Content=" << v.Value() << std::endl;
#endif
      }
};

class RADIUS_XMLDataAttrTree : 
   public RADIUS_XMLDataIteratorWithArg<RADIUS_AttributeTreeByName,
                                        RADIUS_AttributeTreeByType>
{
   public:
      RADIUS_XMLDataAttrTree(std::string &name, 
                             RADIUS_AttributeTreeByName &data,
                             RADIUS_AttributeTreeByType &arg) :
	 RADIUS_XMLDataIteratorWithArg<RADIUS_AttributeTreeByName,
                                       RADIUS_AttributeTreeByType>
         (name, data, arg) {
      }
      int svc(DOMNode *n) {
         RADIUS_DictAttribute *attr(new RADIUS_DictAttribute);

         std::string tagName = "name";
         OD_Utl_XMLDataString name(tagName, attr->Name());
         name.populate(n->getFirstChild());

         tagName = "attr_type";
         unsigned int holder = 0;
         OD_Utl_XMLDataUInt32 attr_type(tagName, holder);
         attr_type.populate(n->getFirstChild());
         attr->AttrType() = holder;

         tagName = "data_type";
         OD_Utl_XMLDataString data_type(tagName, 
                               attr->DataType());
         data_type.populate(n->getFirstChild());

         tagName = "encrypt";
         std::string flg;
         OD_Utl_XMLDataString encrypt(tagName, flg);
         encrypt.populate(n->getFirstChild());
         attr->Flags() = (flg == "true") ?
                          RADIUS_DictAttribute::ENCRYPT :
                          RADIUS_DictAttribute::NONE;
       
         int attrType = attr->AttrType();
         m_Payload.Add(attr->Name(), *attr);
         m_Arg.Add(attrType, *attr);

         print(*attr);
         return (0);
      }
      void print(RADIUS_DictAttribute &a) {
#if RADIUS_DICT_DEBUG
         std::cout << "   Attribute: " << a.Name() << std::endl;
         std::cout << "              Type=" << a.AttrType() << std::endl;
         std::cout << "              Data=" << a.DataType() << std::endl;
         std::cout << "              Encrypt=";
         if (a.Flags() & RADIUS_DictAttribute::ENCRYPT) {
            std::cout << "true" << std::endl;
	 }
         else {
            std::cout << "false" << std::endl;
	 }
#endif
      }
};

class RADIUS_XMLDataBaseTree :
   public OD_Utl_XMLElementParserWithData<RADIUS_DictAttrCollection>
{
   public:
      RADIUS_XMLDataBaseTree(std::string &name, 
         RADIUS_DictAttrCollection &data) :
         OD_Utl_XMLElementParserWithData<RADIUS_DictAttrCollection>
         (name, data) {
      }
      int svc(DOMNode *n) {
         std::string tagName = "attribute";
         RADIUS_XMLDataAttrTree attrTree(tagName, 
                                         m_Payload.AttrByName(), 
                                         m_Payload.AttrByType());
         attrTree.populate(n, tagName);

         tagName = "value";
         RADIUS_XMLDataValueTree valueTree(tagName, 
                                           m_Payload.ValueByName());
         valueTree.populate(n, tagName);
         return (0);
      }
};

class RADIUS_XMLDataVendorsAttr :
   public RADIUS_XMLDataIterator<RADIUS_VendorsAttributes>
{
   public:
      RADIUS_XMLDataVendorsAttr(std::string &name, 
         RADIUS_VendorsAttributes &data) :
         RADIUS_XMLDataIterator<RADIUS_VendorsAttributes>
         (name, data) {
      }
      int svc(DOMNode *n) {
         RADIUS_DictAttrCollection *vendorTree
            (new RADIUS_DictAttrCollection);
         char *c_str = getAttribute(n, "name");
         if (! c_str) {
	    throw RADIUS_Exception(0, "vendor has no name");
	 }
         vendorTree->Name() = c_str;
	 XMLString::release(&c_str);

         c_str = getAttribute(n, "id");
         if (! c_str) {
	    throw RADIUS_Exception(0, "vendor has no id");
	 }
         vendorTree->Id() = ACE_OS::atoi(c_str);
	 XMLString::release(&c_str);

	 std::string tagName = "attribute";
         RADIUS_XMLDataAttrTree attrTree(tagName, 
                                         vendorTree->AttrByName(),
                                         vendorTree->AttrByType());
         attrTree.populate(n, tagName);
         m_Payload.Add(vendorTree->Id(), 
                       *vendorTree);

	 tagName = "value";
         RADIUS_XMLDataValueTree valueTree(tagName, 
                                           vendorTree->ValueByName());
         valueTree.populate(n, tagName);
         return (0);
      }
};

class RADIUS_XMLDictionary : 
   public OD_Utl_XMLElementParserWithData<RADIUS_Dictionary>
{
   public:
      RADIUS_XMLDictionary(std::string &name, 
         RADIUS_Dictionary &data) :
         OD_Utl_XMLElementParserWithData<RADIUS_Dictionary>
         (name, data) {
      }
      int svc(DOMNode *n) {

         std::string tagName;

         tagName = "typename";
         RADIUS_XMLDataTypeNames types
                (tagName, m_Payload.m_DataTypes);
         types.populate(n, tagName);

         tagName = "base";
         RADIUS_XMLDataBaseTree base
                (tagName, m_Payload.m_BaseAttr);
         base.populate(n->getFirstChild());

         tagName = "vendor";
         RADIUS_XMLDataVendorsAttr vendor
                (tagName, m_Payload.m_VendorsAttr);
         vendor.populate(n, tagName);

         return (0);
      }
};

void RADIUS_Dictionary::Load(std::string &fname)
{
    std::string dictRoot = "radius_dictionary";

    OD_Utl_XMLTreeParser parser;
    RADIUS_XMLDictionary dict(dictRoot, 
           *RADIUS_Dict_S::instance());

    if (parser.open(fname, dict) != 0) {
       throw (RADIUS_Exception(0,
             "Fatal: Unable to parse dictionary"));
    }
}






