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

// $Id: DictTest.cxx,v 1.2 2005/10/06 20:50:57 vfajardo Exp $ 

#include <iostream>
#include "radius_dictionary.h"

void DictionaryTestPrintAttribute(RADIUS_DictAttribute &a)
{
   std::cout << "FOUND          : " << a.Name() << std::endl;
   std::cout << "                 Type=" << a.AttrType() << std::endl;
   std::cout << "                 Data=" << a.DataType() << std::endl;
   std::cout << "                 Encrypt=";
   if (a.Flags() & RADIUS_DictAttribute::ENCRYPT) {
      std::cout << "true" << std::endl;
   }
   else {
      std::cout << "false" << std::endl;
   }
}

void DictionaryTestPrintValue(RADIUS_DictValue &v)
{
   std::cout << "FOUND          : " << v.Name() << std::endl;
   std::cout << "                 Attribute=" << v.AttrName() << std::endl;
   std::cout << "                 Value=" << v.Value() << std::endl;
}

void DictionaryTest()
{
   typedef struct {
     char *name;
     int id;
   } AttrSearch;

   AttrSearch attrTestTable[] = { { "User-Name", 1 },
                                  { "EAP-Sim-NOTIFICATION", 1548 },
                                  { "EAP-Type-MD5", 1284 },
                                  { "Cisco-NAS-Port", 2 },
                                  { "Cisco-Gateway-Id", 18 },
                                  { "MS-RAS-Vendor", 9 }, 
                                  { "MS-Acct-EAP-Type", 24 } };
   char *valueTestTable[] = { "Van-Jacobson-TCP-IP",
                              "Line-disconnected",
                              "kmd5-128" };
   int vendorTestTable[] = { 43,  // 3com
                             311, // microsoft
                             9,   // cisco
                             10415, // 3GPP
                             1584 };  // bay networks
   char *typeTestTable[] = { "string", 
                             "ipv6addr",
                             "bogus" };

   std::cout << "Base protocol search ...." << std::endl;
   for (int n=0; n<sizeof(attrTestTable)/sizeof(AttrSearch); n++) {
       AttrSearch &search = attrTestTable[n];
       std::string name = search.name;
       try {
          RADIUS_DictAttribute &a = RADIUS_DICT_ATTR(name);
          RADIUS_DictAttribute &b = RADIUS_DICT_ATTR(search.id);
          if (a.AttrType() == b.AttrType()) {
              DictionaryTestPrintAttribute(a);
          }
          else {
             std::cout << "**** mis-match ****" << std::endl;
          }
       }
       catch (...) {
          std::cout << "NOT FOUND      : " << name << std::endl;
       }
   }

   std::cout << "Vendor search ...." << std::endl;
   for (int i=0; i<sizeof(attrTestTable)/sizeof(AttrSearch); i++) {
     for (int y=0; y<sizeof(vendorTestTable)/sizeof(int);y++) {
       AttrSearch &search = attrTestTable[i];
       std::string name = search.name;
       try {
          RADIUS_DictAttribute &a = RADIUS_DICT_VENDOR_ATTR(name, vendorTestTable[y]);
          RADIUS_DictAttribute &b = RADIUS_DICT_VENDOR_ATTR(search.id, vendorTestTable[y]);
          if (a.AttrType() == b.AttrType()) {
	     DictionaryTestPrintAttribute(a);
	  }
          else {
             std::cout << "**** mis-match ****" << std::endl;
	  }
       }
       catch (...) {
 	  std::cout << "NOT FOUND      : " << name;
          std::cout << ", " << search.id << std::endl;
       }
     }
   }

   std::cout << "Value search ...." << std::endl;
   for (int i=0; i<sizeof(valueTestTable)/sizeof(char*); i++) {
     std::string search = valueTestTable[i];
     try {
        RADIUS_DictValue &v = RADIUS_DICT_VALUE(search);
        DictionaryTestPrintValue(v);
     }
     catch (...) {
        std::cout << "NOT FOUND      : " << search << std::endl;;
     }
   }
   for (int i=0; i<sizeof(valueTestTable)/sizeof(char*); i++) {
     for (int y=0; y<sizeof(vendorTestTable)/sizeof(int);y++) {
       std::string search = valueTestTable[i];
       try {
          RADIUS_DictValue &v = RADIUS_DICT_VENDOR_VALUE(search, vendorTestTable[y]);
          DictionaryTestPrintValue(v);
       }
       catch (...) {
          std::cout << "NOT FOUND      : " << search << std::endl;;
       }
     }
   }


   std::cout << "Typename search ...." << std::endl;
   for (int q=0; q<sizeof(typeTestTable)/sizeof(char*); q++) {
       std::string type=typeTestTable[q];
       try {
          std::cout << "FOUND          : " << type << std::endl;;
       }
       catch (...) {
          std::cout << "NOT FOUND      : " << type << std::endl;
       }
   }
}

int main(int argc, char *argv[])
{   
   if (argc > 1) {
       std::string fname = argv[1];
       RADIUS_DICT_LOAD(fname);
       DictionaryTest();
   }
   else {
   	   std::cout << "Usage: " << argv[0];
   	   std::cout << " {dictionary file}" << std::endl;
   }   
   return (0);
}

