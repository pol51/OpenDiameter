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

#ifndef __AAA_SESSION_GARBAGE_COLLECTOR_H__
#define __AAA_SESSION_GARBAGE_COLLECTOR_H__

#include "framework.h"

template <class T>
class AAA_SessionGarbageCollector :
    public AAA_Job
{
    public:
        AAA_SessionGarbageCollector(AAA_Task &t) :
            m_GroupedJob(AAA_GroupedJob::Create(t.Job(),
	                 (AAA_JobData*)this)), 
            m_Enabled(true) {
	}
        virtual ~AAA_SessionGarbageCollector() {
            m_Enabled = false;
            while (! m_DeleteQueue.IsEmpty()) {
               ACE_Time_Value tv(0, 100000);
               ACE_OS::sleep(tv);
	    }
	}
        void ScheduleForDeletion(T &obj) {
            if (m_Enabled) {
               m_DeleteQueue.Enqueue(&obj);
               Schedule(this);
	    }            
	}

    protected:
        int Schedule(AAA_Job *job, size_t backlogSize=1) {
            return m_GroupedJob->Schedule(job, backlogSize);
        }
        int Serve() {
	    T *obj = m_DeleteQueue.Dequeue();
            delete obj;
            AAA_LOG(LM_INFO, "(%P|%t) *** Garbage collection occuring ***\n");
            return (0);           
	}

    private:
        AAA_JobHandle<AAA_GroupedJob> m_GroupedJob;
        AAA_ProtectedQueue<T*> m_DeleteQueue;
        bool m_Enabled;
};

template <class T>
class AAA_SessionGarbageCollectorSingleton
{
    public:
        virtual ~AAA_SessionGarbageCollectorSingleton() {
	}
        void Initialize(AAA_Task &t) {
            m_Instance = std::auto_ptr< AAA_SessionGarbageCollector<T> > 
		    (new AAA_SessionGarbageCollector<T>(t));
	}
        AAA_SessionGarbageCollector<T> &Instance() {
            // this will throw when used improperly
            return (*m_Instance);
	}

    private:
        std::auto_ptr< AAA_SessionGarbageCollector<T> > 
              m_Instance;
};

#endif // __AAA_SESSION_GARBAGE_COLLECTOR_H__


