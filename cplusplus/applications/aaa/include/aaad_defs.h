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

#ifndef __AAAD_DEFS_H__
#define __AAAD_DEFS_H__

#include <map>
#include <list>
#include <string>
#include <ace/Singleton.h>

#define AAAD_LOG ACE_Log_Msg::instance()->log

class AAAD_MapElement
{
   public:
      virtual ~AAAD_MapElement() {
      }
      std::string &Name() {
         return m_Name;
      }
      virtual void Dump() {
      }
   protected:
      std::string m_Name;
};

class AAAD_Map
{
   public:
      typedef std::map<std::string, AAAD_MapElement*> AAAD_ElementMap;

   public:
      virtual ~AAAD_Map() {
         while (! m_Map.empty()) {
            std::auto_ptr<AAAD_MapElement> e(m_Map.begin()->second);
            m_Map.erase(m_Map.begin()); 
         }
      }
      virtual bool Register(std::auto_ptr<AAAD_MapElement> e) {
         AAAD_ElementMap::iterator i = m_Map.find(e->Name());
         if (i == m_Map.end()) {
            AAAD_MapElement *item = e.get();
            m_Map.insert(std::pair<std::string, AAAD_MapElement*>
                         (e->Name(), item));
            e.release();
            return true;
         }
         return false;
      }
      virtual AAAD_MapElement *Lookup(std::string &name) {
         AAAD_ElementMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
            return i->second;
         }
         return NULL;
      }
      virtual AAAD_MapElement *Match(std::string &name) {
	 AAAD_ElementMap::iterator i = m_Map.begin();
         for (; i != m_Map.end(); i++) {
	    AAAD_MapElement *e = i->second;
            if (name.find(e->Name()) != std::string::npos) {
	       return e;
	    }
	 }
         return NULL;
      }
      virtual void Remove(std::string &name) {
         AAAD_ElementMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
            std::auto_ptr<AAAD_MapElement> cleanup(i->second);
            m_Map.erase(i);
         }
      }
      virtual void Dump() {
         AAAD_ElementMap::iterator i;
         for (i = m_Map.begin(); i != m_Map.end(); i++) {
             i->second->Dump();
         }
      }

   protected:
      AAAD_ElementMap m_Map;
};

template <class APPLICATION>
class AAAD_ApplicationElement : 
   public AAAD_MapElement
{
   public:
      APPLICATION &Application() {
         return m_Application;
      }
      bool &Enabled() {
         return m_bEnabled;
      }
      virtual void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)    Application: %s\n", 
                  m_Name.data());
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)        Enabled: %s\n", 
                  m_bEnabled ? "true" : "false");
         m_Application.Dump();
      }
   protected:
      bool m_bEnabled;
      APPLICATION m_Application;
};

class AAAD_AppDiamEapElement
{
   public:
      std::string &LocalIdentity() {
         return m_LocalIdentity;
      }
      std::string &UserDbFile() {
         return m_UserDbFile;
      }
      void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)    Local Ident: %s\n", 
                  m_LocalIdentity.data());
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)   User Db File: %s\n",                   
                  m_UserDbFile.data());
      }
   protected:
      std::string m_LocalIdentity;
      std::string m_UserDbFile;
};

///
/// Add more application definitions here
///
typedef AAAD_ApplicationElement<AAAD_AppDiamEapElement> 
        AAAD_AppDiamEapData;
typedef AAAD_Map AAAD_ApplicationTable;

class AAAD_CfgData
{
   public:
      int &ThreadCount() {
         return m_ThreadCount;
      }
      std::string &DiameterCfgFile() {
         return m_DiameterCfgFile;
      }
      AAAD_ApplicationTable &ApplicationTable() {
         return m_ApplicationTable;
      }
      void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)--- Cfg Data ---\n");

         AAAD_LOG(LM_INFO, 
                  "(%P|%t)     Thread Cnt: %d\n", m_ThreadCount);

         AAAD_LOG(LM_INFO, 
                  "(%P|%t)  Diam Cfg File: %s\n", 
                  m_DiameterCfgFile.data());

         AAAD_LOG(LM_INFO, 
                  "(%P|%t)--- Application Table ---\n");
         m_ApplicationTable.Dump();
      }
   protected:
      int m_ThreadCount;
      std::string m_DiameterCfgFile;
      AAAD_ApplicationTable m_ApplicationTable;
};

// Cfg database
typedef ACE_Singleton<AAAD_CfgData, 
                      ACE_Recursive_Thread_Mutex> 
                      AAAD_CfgData_S;

// Access macros for all databases
#define AAAD_CFG_DATA()  (AAAD_CfgData_S::instance())
#define AAAD_APP_TBL()   (AAAD_CfgData_S::instance()->ApplicationTable())

#endif // __AAAD_DEFS_H__



