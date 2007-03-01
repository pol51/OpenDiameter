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


#ifndef __NASD_POLICY_SCRIPT_H__
#define __NASD_POLICY_SCRIPT_H__

#include "nasd_call_framework.h"

class NASD_PolicyScript : 
   public NASD_CnAccessPolicy
{
   public:
       NASD_PolicyScript(AAA_Task &t) :
	  NASD_CnAccessPolicy(t) {
       }
       virtual int Start() {
          std::string name("script");
          NASD_PolicyScriptData *policy = (NASD_PolicyScriptData*)
               NASD_POLICY_TBL().Lookup(name);
          if (policy == NULL) {
              NASD_LOG(LM_ERROR, "(%P|%t) Script policy configuration entry not found\n");
          }
          else {
	      m_Script = policy->Policy().ScriptFile();
	  }
          return (0);
       }
       virtual bool IsRunning() {
          return true;
       }
       virtual void Stop() {
       }
       virtual bool Execute() {
          if (m_Script.length() > 0) {
              system(m_Script.data());
	  }
          else {
              NASD_LOG(LM_ERROR, "(%P|%t) WARNING: No script file defined\n");
	  }
          return true;
       }
    private:
       std::string m_Script;
};

class NASD_PolicyScriptInitializer : 
    public NASD_CnInitCallback
{
    public:
        virtual bool Initialize(AAA_Task &t) {

           NASD_LOG(LM_INFO, "(%P|%t) Initializing Diameter-EAP AAA protocol\n");

           if (m_Policy.get() == NULL) {
               m_Policy = std::auto_ptr<NASD_PolicyScript>
                   (new NASD_PolicyScript(t));
               m_Policy->Start();
           }
           return true;
	}
        virtual bool UnInitialize() {
           m_Policy.reset();
           return true;
	}
        virtual NASD_CallElement *Create(AAA_Task &t) {
           return m_Policy.get();
	}

    private:
        std::auto_ptr<NASD_PolicyScript> m_Policy;
};

#endif // __NASD_POLICY_SCRIPT_H__


