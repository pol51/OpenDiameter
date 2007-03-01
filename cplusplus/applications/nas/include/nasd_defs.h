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

#ifndef __NASD_DEFS_H__
#define __NASD_DEFS_H__

#include <map>
#include <list>
#include <string>
#include <ace/Singleton.h>

#define NASD_LOG ACE_Log_Msg::instance()->log

class NASD_MapElement
{
   public:
      virtual ~NASD_MapElement() {
      }
      std::string &Name() {
         return m_Name;
      }
      virtual void Dump() {
      }
   protected:
      std::string m_Name;
};

class NASD_Map
{
   public:
      typedef std::map<std::string, NASD_MapElement*> NASD_ElementMap;

   public:
      virtual ~NASD_Map() {
         while (! m_Map.empty()) {
            std::auto_ptr<NASD_MapElement> e(m_Map.begin()->second);
            m_Map.erase(m_Map.begin()); 
         }
      }
      virtual bool Register(std::auto_ptr<NASD_MapElement> e) {
         NASD_ElementMap::iterator i = m_Map.find(e->Name());
         if (i == m_Map.end()) {
            NASD_MapElement *item = e.get();
            m_Map.insert(std::pair<std::string, NASD_MapElement*>
                         (e->Name(), item));
            e.release();
            return true;
         }
         return false;
      }
      virtual NASD_MapElement *Lookup(std::string &name) {
         NASD_ElementMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
             return i->second;
         }
         return NULL;
      }
      virtual void Remove(std::string &name) {
         NASD_ElementMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
            std::auto_ptr<NASD_MapElement> cleanup(i->second);
            m_Map.erase(i);
         }
      }
      virtual void Dump() {
         NASD_ElementMap::iterator i;
         for (i = m_Map.begin(); i != m_Map.end(); i++) {
             i->second->Dump();
         }
      }

   protected:
      NASD_ElementMap m_Map;
};

template <class PROTOCOL>
class NASD_ProtocolElement : 
   public NASD_MapElement
{
   public:
      PROTOCOL &Protocol() {
         return m_Protocol;
      }
      bool &Enabled() {
         return m_bEnabled;
      }
      virtual void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)    ** Protocol: %s\n", 
                  m_Name.data());
         NASD_LOG(LM_INFO, 
                  "(%P|%t)        Enabled: %s\n", 
                  m_bEnabled ? "true" : "false");
         m_Protocol.Dump();
      }
   protected:
      bool m_bEnabled;
      PROTOCOL m_Protocol;
};

class NASD_ApPanaElement
{
   public:
      NASD_ApPanaElement() {
      }
      std::string &CfgFile() {
         return m_CfgFile;
      }
      std::string &EpScript() {
         return m_EpScript;
      }
      void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)       Cfg File: %s\n", 
                  m_CfgFile.data());
         NASD_LOG(LM_INFO, 
                  "(%P|%t)           EP S: %s\n", 
                  m_EpScript.data());
      }
   protected:
      std::string m_CfgFile;
      std::string m_AuthScript;
      std::string m_EpScript;
};

class NASD_Ap8021xElement
{
   public:
      void Dump() {
      }
};

typedef NASD_ProtocolElement<NASD_ApPanaElement> 
        NASD_ApPanaData;
typedef NASD_ProtocolElement<NASD_Ap8021xElement> 
        NASD_Ap8021xData;
typedef NASD_Map NASD_AccessProtocolTable;

class NASD_AaaLocalEapAuth
{
   public:
      std::string &SharedSecretFile() {
         return m_SharedSecretFile;
      }
      std::string &Identity() {
         return m_Identity;
      }
      void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)         Secret: %s\n", 
                  m_SharedSecretFile.data());
         NASD_LOG(LM_INFO, 
                  "(%P|%t)       Identity: %s\n", 
                  m_Identity.data());
      }
   protected:
      std::string m_SharedSecretFile;
      std::string m_Identity;
};

class NASD_AaaDiameterEap
{
   public:
      std::string &DiameterCfgFile() {
         return m_DiameterCfgFile;
      }
      void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)        Cfgfile: %s\n", 
                  m_DiameterCfgFile.data());
      }
   protected:
      std::string m_DiameterCfgFile;
};

typedef NASD_ProtocolElement<NASD_AaaLocalEapAuth> 
        NASD_AaaLocalEapAuthData;
typedef NASD_ProtocolElement<NASD_AaaDiameterEap> 
        NASD_AaaDiameterEapData;
typedef NASD_Map NASD_AaaProtocolTable;

template <class POLICY>
class NASD_PolicyElement : 
   public NASD_MapElement
{
   public:
      POLICY &Policy() {
         return m_Policy;
      }
      virtual void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)  Access Policy: %s\n", 
                  m_Name.data());
         m_Policy.Dump();
      }
   protected:
      POLICY m_Policy;
};

class NASD_ScriptPolicy
{
   public:
      std::string &ScriptFile() {
         return m_ScriptFile;
      }
      void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)         Script: %s\n", 
                  m_ScriptFile.data());
      }
   protected:
      std::string m_ScriptFile;
};

typedef NASD_PolicyElement<NASD_ScriptPolicy> 
        NASD_PolicyScriptData;
typedef NASD_Map NASD_PolicyTable;

class NASD_RouteEntry : 
   public NASD_MapElement
{
   public:
      typedef std::list<std::string> AccessPolicyList;
   public:
      std::string &Nai() {
         return m_Name;
      }
      AccessPolicyList &PolicyList() {
         return m_PolicyList;
      }      
      std::string &AaaProtocol() {
         return m_AaaProtocol;
      }
      virtual void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)            NAI: %s\n", 
                  m_Name.data());
         NASD_LOG(LM_INFO, 
                  "(%P|%t)            AAA: %s\n", 
                  m_AaaProtocol.data());
	 AccessPolicyList::iterator i;
         for (i = m_PolicyList.begin(); 
              i != m_PolicyList.end(); i++) {
            NASD_LOG(LM_INFO, 
                     "(%P|%t)         Policy: %s\n", 
                     (*i).data());
	 }
      }
   protected:
      AccessPolicyList m_PolicyList;
      std::string m_AaaProtocol;
};

class NASD_RouteMap : 
   public NASD_Map
{
   public:
      NASD_MapElement *Lookup(std::string &Nai) {
	 ///
         /// Lookup method
	 ///
         /// 1. Exact match on NAI, else
         /// 2. Nai is stripped of username and 
         ///    domain is used for exact match, else
         /// 3. Default route is used
         ///

	 NASD_ElementMap::iterator i = m_Map.find(Nai);
	 if (i != m_Map.end()) {
	     return i->second;
	 }
	 else {
	     unsigned int pos = Nai.find("@");
         if ((pos > 0) && (pos < Nai.length())) {
	         std::string realm(Nai, pos + 1, Nai.length());
             i = m_Map.find(realm);
             if (i != m_Map.end()) {
	             return i->second;
             }
         }
         else {
             NASD_LOG(LM_INFO, 
                  "(%P|%t) Cannot determine domain in [%s]\n", 
                  Nai.data());
	     }
	 }
         return &m_DefaultRoute;
      }
      NASD_RouteEntry &DefaultRoute() {
         return m_DefaultRoute;
      }
      virtual void Dump() {
         NASD_Map::Dump();
         NASD_LOG(LM_INFO, "(%P|%t)   *Default Route*\n");
         DefaultRoute().Dump();
      }
   protected:
      NASD_RouteEntry m_DefaultRoute;
};

typedef NASD_RouteMap NASD_CallRouteTable;

class NASD_CallManagementData
{
   public:
      int &ThreadCount() {
         return m_ThreadCount;
      }
      NASD_CallRouteTable &CallRouteTable() {
         return m_CallRouteTable;
      }
      NASD_AaaProtocolTable &AAAProtocolTable() {
         return m_AAAProtocolTable;
      }
      NASD_PolicyTable &PolicyTable() {
         return m_PolicyTable;
      }
      NASD_AccessProtocolTable &AccessProtocolTable() {
         return m_AccessProtocolTable;
      }
      void Dump() {
         NASD_LOG(LM_INFO, 
                  "(%P|%t)--- Call Management ---\n");
         NASD_LOG(LM_INFO, 
                  "(%P|%t)     Thread Cnt: %d\n", m_ThreadCount);

         NASD_LOG(LM_INFO, 
                  "(%P|%t)--- Call Routing    ---\n");
         m_CallRouteTable.Dump();

         NASD_LOG(LM_INFO, 
                  "(%P|%t)--- Access Protocol ---\n");
         m_AccessProtocolTable.Dump();

         NASD_LOG(LM_INFO, 
                  "(%P|%t)---   AAA Protocol  ---\n");
         m_AAAProtocolTable.Dump();

         NASD_LOG(LM_INFO, 
                  "(%P|%t)---  Access Policy  ---\n");
         m_PolicyTable.Dump();
      }
   protected:
      int m_ThreadCount;
      NASD_CallRouteTable m_CallRouteTable;
      NASD_AaaProtocolTable m_AAAProtocolTable;
      NASD_PolicyTable m_PolicyTable;
      NASD_AccessProtocolTable m_AccessProtocolTable;
};

// Global database
typedef ACE_Singleton<NASD_CallManagementData, 
                      ACE_Recursive_Thread_Mutex> 
                      NASD_CallManagementData_S;

// Access macros for all databases
#define NASD_CALLMNGT_DATA()  (NASD_CallManagementData_S::instance())
#define NASD_CALLROUTE_TBL()  (NASD_CallManagementData_S::instance()->CallRouteTable())
#define NASD_AAAPROTO_TBL()   (NASD_CallManagementData_S::instance()->AAAProtocolTable())
#define NASD_APPROTO_TBL()    (NASD_CallManagementData_S::instance()->AccessProtocolTable())
#define NASD_POLICY_TBL()     (NASD_CallManagementData_S::instance()->PolicyTable())

#endif // __NASD_DEFS_H__



