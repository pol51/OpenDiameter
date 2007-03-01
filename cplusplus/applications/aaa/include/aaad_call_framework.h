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

#ifndef __AAAD_CALL_FRAMEWORK_H__
#define __AAAD_CALL_FRAMEWORK_H__

#include "aaad_config.h"
#include "diameter_api.h"

class AAAD_CallElement :
   public AAAD_MapElement
{
   public:
      virtual ~AAAD_CallElement() {
      }
      virtual int Start(AAAApplicationCore &core) = 0;
      virtual bool IsRunning() = 0;
      virtual void Stop() = 0;     
};

class AAAD_CallElementMap
{
   public:
      typedef std::map<std::string, 
                       AAAD_CallElement*> 
                       AAAD_CeMap;

   public:
      virtual ~AAAD_CallElementMap() {
         while (! m_Map.empty()) {
            m_Map.erase(m_Map.begin()); 
         }
      }
      virtual bool Register(AAAD_CallElement &e) {
         AAAD_CeMap::iterator i = m_Map.find(e.Name());
         if (i == m_Map.end()) {
            m_Map.insert(std::pair<std::string, AAAD_CallElement*>
                         (e.Name(), &e));
            return true;
         }
         return false;
      }
      virtual AAAD_CallElement *Lookup(std::string &name) {
         AAAD_CeMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
             return i->second;
         }
         return NULL;
      }
      virtual void Remove(std::string &name) {
         AAAD_CeMap::iterator i = m_Map.find(name);
         if (i != m_Map.end()) {
            m_Map.erase(i);
         }
      }
      virtual void Dump() {
         AAAD_CeMap::iterator i;
         for (i = m_Map.begin(); i != m_Map.end(); i++) {
             i->second->Dump();
         }
      }

   protected:
      AAAD_CeMap m_Map;
};

class AAAD_CallFramework :
   public AAAD_CallElementMap
{
   public:
      int Start(std::string &cfgFile) {

         AAAD_CfgLoader loader(cfgFile.data());
         AAAD_CFG_DATA()->Dump();

         try {
             m_Task.Start(AAAD_CFG_DATA()->ThreadCount());
         }
         catch (...) {
             ACE_ERROR((LM_ERROR, 
                  "(%P|%t) AAAD cannot start task\n"));
             return (-1);
         }

         if (m_Core.Open((char*)AAAD_CFG_DATA()->DiameterCfgFile().data(), 
             m_Task) != AAA_ERR_SUCCESS) {
            AAAD_LOG(LM_INFO, "Failed to open diameter core !!!\n");
            return (-1);
	 }

         AAAD_CeMap::iterator i;
         for (i = m_Map.begin(); i != m_Map.end(); i++) {
	     AAAD_CallElement *c = (AAAD_CallElement*)i->second;
             c->Start(m_Core);
         }
         return (0);
      }
      bool IsRunning() {
	 // TBD: improvement here include signal checks
	 return true;
      }
      void Stop() {
         AAAD_CeMap::iterator i;
         for (i = m_Map.begin(); i != m_Map.end(); i++) {
	     AAAD_CallElement *c = (AAAD_CallElement*)i->second;
             c->Stop();
         }
	 m_Task.Stop();
	 m_Core.Close();
      }

   private:
      AAA_Task m_Task;
      AAAApplicationCore m_Core;
};

typedef ACE_Singleton<AAAD_CallFramework, ACE_Null_Mutex> 
        AAAD_CnFramework_S;
#define AAAD_FRAMEWORK() (*(AAAD_CnFramework_S::instance()))

#endif // __AAAD_CALL_FRAMEWORK_H__



