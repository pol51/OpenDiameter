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

#ifndef __NASD_CALL_FRAMEWORK_H__
#define __NASD_CALL_FRAMEWORK_H__

#include <map>
#include <string>
#include <ace/Singleton.h>
#include "framework.h"
#include "diameter_parser.h"
#include "nasd_config.h"

class NASD_CallElement :
   public AAA_JobData
{
   public:
      virtual ~NASD_CallElement() {
      }
      virtual int Start() = 0;
      virtual bool IsRunning() = 0;
      virtual void Stop() = 0;     
};

class NASD_CallElementFactoryCreateCallback
{
   public:
      virtual NASD_CallElement *Create(AAA_Task &t) = 0;
      virtual ~NASD_CallElementFactoryCreateCallback() { }
};

class NASD_CallElementFactory
{
   public:
      typedef std::map<std::string, 
                       NASD_CallElementFactoryCreateCallback*> 
                       CallbackMap;
      
   public:
      NASD_CallElementFactory(AAA_Task &t) : 
          m_Task(t) {
      }
      bool Register(std::string &name, 
                    NASD_CallElementFactoryCreateCallback &cb) {
          return m_Map.insert(CallbackMap::value_type(name, &cb)).second;
      }
      bool UnRegister(std::string &id) {
          return (m_Map.erase(id) == 1);
      }
      NASD_CallElement *Create(std::string &name) {
          CallbackMap::iterator i = m_Map.find(name);
          if (i != m_Map.end()) {              
              return (*(i->second)).Create(m_Task);
          }
          return 0;
      }      
      NASD_CallElementFactoryCreateCallback* Lookup(std::string &name) {
          CallbackMap::iterator i = m_Map.find(name);
          return (i != m_Map.end()) ? i->second : 0;
      }
      
   protected:
      AAA_Task &m_Task;
      CallbackMap m_Map;
};

class NASD_CallNode;
typedef NASD_CallNode* NASD_CnPtr;

///
/// Basic definition of a NASD node
/// Notes:
///    Implementors MUST inherit this class
///    and implement ReceiveIngress() and
///    ReceiveEgress() methods
///
class NASD_CallNode : 
   public NASD_CallElement
{
   public:
      NASD_CallNode() :
         m_Prev(NULL),
         m_Next(NULL) {
      }
      NASD_CallNode(NASD_CnPtr prev,
                    NASD_CnPtr next) :
          m_Prev(prev),
          m_Next(next) {
      }
      virtual NASD_CnPtr &NextNode() {
          return m_Next;
      }
      virtual NASD_CnPtr &PrevNode() {
          return m_Prev;
      }
      virtual int SendIngress(AAAMessageBlock &msg) {
          return (m_Next) ? m_Next->ReceiveIngress(msg) : -1;
      }
      virtual int SendEgress(AAAMessageBlock &msg) {
          return (m_Prev) ? m_Prev->ReceiveEgress(msg) : -1;
      }
      virtual bool CurrentKey(std::string &key) = 0;
      virtual bool Identity(std::string &ident) = 0;
      virtual int ReceiveIngress(AAAMessageBlock &msg) = 0;
      virtual int ReceiveEgress(AAAMessageBlock &msg) = 0;
      virtual void Success(AAAMessageBlock *msg = 0) = 0;
      virtual void Failure(AAAMessageBlock *msg = 0) = 0;
      virtual void Timeout() = 0;
      virtual void Error() = 0;
      
   protected:
      NASD_CnPtr m_Prev;
      NASD_CnPtr m_Next;
};

class NASD_CnGarbageCollector :  public AAA_Job
{
    public:
        NASD_CnGarbageCollector(AAA_Task &t) :
            m_GroupedJob(AAA_GroupedJob::Create(t.Job(),
	                 (AAA_JobData*)this)), 
            m_Enabled(true) {
	}
        virtual ~NASD_CnGarbageCollector() {
            m_Enabled = false;
            while (! m_DeleteQueue.empty()) {
               ACE_Time_Value tv(0, 100000);
               ACE_OS::sleep(tv);
	    }
	}
        void ScheduleForDeletion(NASD_CallNode &ch) {
            if (m_Enabled) {
               AAA_ScopeLock<ACE_Mutex> guard(m_Lock);
               m_DeleteQueue.push_back(&ch);
               Schedule(this);
	    }            
	}

    protected:
        int Schedule(AAA_Job *job, size_t backlogSize=1) {
            return m_GroupedJob->Schedule(job, backlogSize);
        }
        int Serve() {
            NASD_CnPtr node = NULL;
            try {
                AAA_ScopeLock<ACE_Mutex> guard(m_Lock);
                if (m_DeleteQueue.size() == 0) {
                    NASD_LOG(LM_INFO, 
                        "(%P|%t) WARNING: Garbage collector schedule with empty queue\n");
                    return (0);
		}
                node = m_DeleteQueue.front();
                if (! node->IsRunning()) {
                    m_DeleteQueue.pop_front();
                }
                else {
		    node->Stop();
	            Schedule(this);
                    NASD_LOG(LM_INFO, 
                        "(%P|%t) *** Re-scheduling collection ***\n");
                    return (0);
	        }
	    }
            catch (...) {
	    }
            if (node) {
                NASD_LOG(LM_INFO, 
                    "(%P|%t) *** Garbage collection occuring ***\n");

                /// TBD: Fix this !!!
                ACE_Time_Value tm(3, 0);
		ACE_OS::sleep(tm);
                delete node;
	    }
            return (0);           
	}

    private:
        ACE_Mutex m_Lock;
        AAA_JobHandle<AAA_GroupedJob> m_GroupedJob;       
        std::list<NASD_CnPtr> m_DeleteQueue;
        bool m_Enabled;
};

class NASD_CnGarbageCollectorSingleton
{
    public:
        virtual ~NASD_CnGarbageCollectorSingleton() {
	}
        void Initialize(AAA_Task &t) {
            m_Instance = std::auto_ptr<NASD_CnGarbageCollector> 
		    (new NASD_CnGarbageCollector(t));
	}
        NASD_CnGarbageCollector &Instance() {
            // this will throw when used improperly
            return (*m_Instance);
	}

    private:
        std::auto_ptr< NASD_CnGarbageCollector > 
              m_Instance;
};

typedef ACE_Singleton<NASD_CnGarbageCollectorSingleton, ACE_Null_Mutex> 
        NASD_CnGarbageCollector_S;
#define NASD_CnGarbageCollector_I NASD_CnGarbageCollector_S::instance()
#define NASD_GARBAGE_COLLECTOR() NASD_CnGarbageCollector_I->Instance()

class NASD_CnAccessPolicy :
   public NASD_CallElement
{
   public:
      NASD_CnAccessPolicy(AAA_Task &t) {
      }
      virtual bool Execute() = 0;
};

class NASD_CnAaaProtocol :
   public NASD_CallNode
{
   public:
      NASD_CnAaaProtocol(AAA_Task &t) {
      }
};

class NASD_CnInitCallback :
    public NASD_CallElementFactoryCreateCallback
{
    public:
        virtual bool Initialize(AAA_Task &t) {
           return true;
        }
        virtual bool UnInitialize()  {
           return true;
        }
        virtual NASD_CallElement *Create(AAA_Task &t) {
           return NULL;
	}
    protected:
        NASD_CnInitCallback() {
	}
        virtual ~NASD_CnInitCallback() {
	}
};

class NASD_CnInitializer : 
    public NASD_CallElementFactory
{
    public:
        NASD_CnInitializer() :
           NASD_CallElementFactory(m_Task) {
        }
        AAA_Task &Task() {
           return m_Task;
	}
        bool IsRunning() {
           return m_Task.Running();
	}
        bool Start(const char *cfgfile = 0) {

           /// Garbage collection
           NASD_CnGarbageCollector_I->Initialize(m_Task);
           
           /// Configuration parsing
           NASD_CfgLoader cfgLoader(cfgfile);
           NASD_CALLMNGT_DATA()->Dump();

           /// AAA_Task
           m_Task.Start(NASD_CALLMNGT_DATA()->ThreadCount());

           /// Initialize all registered members
	   NASD_CallElementFactory::CallbackMap::iterator i;
           for (i = m_Map.begin(); i != m_Map.end(); i++) {
                NASD_CnInitCallback *cb = (NASD_CnInitCallback*)i->second;
                cb->Initialize(m_Task);
	   }           
           return true;
	}
        void Stop() {
           /// Un-initialize all registered members
	   NASD_CallElementFactory::CallbackMap::iterator i;
           for (i = m_Map.begin(); i != m_Map.end(); i++) {
                NASD_CnInitCallback *cb = (NASD_CnInitCallback*)i->second;
                cb->UnInitialize();
	   }           

           m_Task.Stop();
	}

    private:
        AAA_Task m_Task;
};

typedef ACE_Singleton<NASD_CnInitializer, ACE_Null_Mutex> 
        NASD_CnInitializer_S;
#define NASD_CnInitializer_I NASD_CnInitializer_S::instance() 

class NASD_CallRouting :
   public NASD_CallNode
{
   public:
      virtual bool IncommingCall(std::string &nai) {

         // Lookup the route
         NASD_RouteEntry *rte = (NASD_RouteEntry*)NASD_CALLROUTE_TBL().
                                      Lookup(nai);
         if (rte == NULL) {
             NASD_LOG(LM_INFO, "(%P|%t) Route failure for [%s]\n", 
                      nai.data());
             return false;
	 }

         NASD_LOG(LM_INFO, "(%P|%t) Routing call for [%s] using [%s]\n", 
                  nai.data(), rte->Nai().data());

         // Execute all access policies
	 NASD_RouteEntry::AccessPolicyList::iterator i;
         for (i = rte->PolicyList().begin(); 
              i != rte->PolicyList().end();
              i ++) {
             NASD_CnAccessPolicy *CnAp = (NASD_CnAccessPolicy*)
                    NASD_CnInitializer_I->Create(*i);
             if (CnAp) {
                 if (! CnAp->Execute()) {
                     NASD_LOG(LM_INFO, "(%P|%t) Failed on access policy [%s]\n", 
                              (*i).data());
                     return false;
                 }
	     }
             else {
                 NASD_LOG(LM_INFO, "(%P|%t) Policy [%s] is not registered\n", 
                          (*i).data());
	     }
	 }

         // Create AAA protocols
         NASD_CnPtr CnAaa = (NASD_CnAaaProtocol*)
             NASD_CnInitializer_I->Create(rte->AaaProtocol());
         if (CnAaa == NULL) {
             NASD_LOG(LM_INFO, "(%P|%t) Failed to create %s for %s\n", 
                      rte->AaaProtocol().data(), nai.data());
             return false;
	 }

         NASD_LOG(LM_INFO, "(%P|%t) Call routed to [%s]\n", 
                  rte->AaaProtocol().data());

         // Bind passthrough with aaa
         NextNode() = CnAaa;
         CnAaa->PrevNode() = this;

         // Start the AAA protocol
         CnAaa->Start();
         return true;
      }

   protected:
      virtual ~NASD_CallRouting() {
      }
};

template <class LOCAL_PASSTHROUGH>
class NASD_CnAccessProtocol :
   public NASD_CallNode
{
   public:
      NASD_CnAccessProtocol(AAA_Task &t) :
         m_LocalPassThrough(t) {
         NextNode() = &m_LocalPassThrough;
         m_LocalPassThrough.PrevNode() = this;
      }    
      virtual ~NASD_CnAccessProtocol() {
	 NASD_CnPtr cnPtr = m_LocalPassThrough.NextNode();
         while (cnPtr) {
	    NASD_GARBAGE_COLLECTOR().
                ScheduleForDeletion(*cnPtr);
            cnPtr = cnPtr->NextNode();
	 }
      }    
      LOCAL_PASSTHROUGH &PassThrough() {
         return m_LocalPassThrough;
      }

   protected:
      LOCAL_PASSTHROUGH m_LocalPassThrough;
};

#endif // __NASD_H__



