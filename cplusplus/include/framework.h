/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2006 Open Diameter Project                          */
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
// $Id: framework.h,v 1.61 2006/04/20 21:46:40 vfajardo Exp $
// framework.h: Protocol Framework API based on ACE.
// Written by Yoshihiro Ohba

#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

#include <aaa_global_config.h>
#include <list>
#include <map>
#include <string>
#include <ace/Task.h>
#include <ace/Task_T.h>
#include <ace/Thread.h>
#include <ace/Thread_Manager.h>
#include <ace/Event_Handler.h>
#include <ace/Synch.h>
#include <ace/Reactor.h>
#include <ace/Thread_Exit.h>
#include <ace/Time_Value.h>
#include <ace/Singleton.h>
#include <ace/Token.h>
#include <ace/Log_Msg.h>
#include <ace/Atomic_Op_T.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

/// Logging facility.
#define FRAMEWORK_LOG ACE_Log_Msg::instance()->log

/*! This class is used to generate a lightweight class from a given
 * class.  This template is borrowed from a book "Mordern C++ Design"
 * by Andrei Alexandrescu.
 */
template <class T> struct Type2Type
{
  typedef T OriginalType; /**< Original type instance */
};

/// Error codes used for the framework API
enum AAA_Error { 
		NoInitialState, 
		FoundDuplicateStateTableEntry,
		ActivationFailure,
		TaskNotRunning,
		NoData
	};


/*! \page guards Re-entrancy guards

This is a generic class that provides scope locking
using template parameter specific locks.

*/
template <class LOCK>
class AAA_ScopeLock
{
 public:
  AAA_ScopeLock(LOCK &l) : lock(l) {
    lock.acquire();
  }
  ~AAA_ScopeLock() {
    lock.release();
  }
  LOCK &Lock() {
    return lock;
  }

 private:
  LOCK &lock;
};

/*! \page guards Reference count guard

This is a generic class that provides scope reference
counter increment and decrement function

*/
template <class ARG>
class AAA_ScopeIncrement
{
   public:
      AAA_ScopeIncrement(ARG &c) : count(c) {
         count ++;
      }
      ~AAA_ScopeIncrement() {
         count --;
      }
   
   private:
      ARG &count;
};

/// scope guards using ACE_Token
typedef AAA_ScopeLock<ACE_Token> AAA_TokenScopeLock;

/// scope guards using ACE_Mutex
typedef AAA_ScopeLock<ACE_Mutex> AAA_MutexScopeLock;

/// scope guards using ACE_ThreadMutex
typedef AAA_ScopeLock<ACE_Thread_Mutex> AAA_ThreadMutexScopeLock;

class AAA_JobData {};

class AAA_JobDeleter;
class AAA_Task;

/*! \page job Job

This is the base class of "job", is scheduled and served by a task.

*/
/// This is the base class of "job".  See \ref tagJob for detailed
/// information.
class AAA_Job
{
  friend class AAA_JobDeleter;

 public:
  /// Constructor.
  AAA_Job(AAA_JobData *d=0, char* name=0) : data(d), priority(1), weight(1)
  {
    if (name) this->name = std::string(name);
    refcount = 1;
  }

  /// This pure virtual function is used for serving the job.  The
  /// number of backlog is returned.  
  virtual int Serve()=0;

  /// This is called by other jobs to request for scheduling to this
  /// job.
  /// This is called by other jobs to request for scheduling to this
  /// job.  The backlogSize contains the current backlog size of the
  /// job and used by the scheduling job to enforce various scheduling
  /// policies.
  virtual int Schedule(AAA_Job*, size_t backlogSize=1)=0;

  /// Delete the job.  Returns 1 when the object has been really
  /// deleted.  Otherwise, 0 is returned.
  void Delete() { if (--refcount <=0 ) delete this; }

  /// Indicates whether there is a backlog or not.
  virtual bool ExistBacklog() { return false; }

  /// The number of backlog.
  virtual size_t BacklogSize() { return 0; }

  /// Obtain the name of the job.
  inline std::string& Name() { return name; }

  /// Obtain the reference to job data.
  inline AAA_JobData* Data() { return data; }

  /// Obtain the reference to job data, with converted to a specific
  /// data type.
  template <class T> inline T* Data(Type2Type<T>) { return (T*)data; }

  /// Obtain the reference to priority.
  unsigned& Priority() { return priority; }

  /// Obtain the reference to weight.
  unsigned& Weight() { return weight; }

  /// Increase the job reference counter.
  void Acquire() { ++refcount; }

  /// Decrease the job reference counter by the specified number.  If
  /// refcount becomes less than 0, then delete the job and return -1,
  /// otherwise, return 0.
  int Release(int n=1) 
  { 
    refcount -= n;
    if (refcount<0) {
      delete this;
      return -1;
    }
    return 0;
  }

  /// Indicates whether the job is in a state which can accept
  /// Schedule() method.
  virtual bool Running() { return true; }

  bool operator==(AAA_Job* job) { return (this==job); }

  bool operator!=(AAA_Job* job) { return (this!=job); }

 protected:

  /// Virtual destructor.
  virtual ~AAA_Job() {}

 private:  

  /// Job name.
  std::string name;

  /// Opaque job ata
  AAA_JobData* data;

  /// Reference counter.  The value can be changed only via
  /// JobDeleter's () operator.
  ACE_Atomic_Op<ACE_Thread_Mutex, int> refcount;

  /// Indicates the priority of the job (1=highest priority)
  unsigned priority;

  /// Indicates the weight of the job in the given priority.
  unsigned weight;
};

/*! jobHandle Handle to a Job

    This class provides a "handle" to a job, providing a safe way for
    memory deallocation when an exception occurs in the constructor of
    a class which generates a job object.
*/
template <class JOB>
class AAA_JobHandle : public boost::shared_ptr<JOB>
{
 public:
  AAA_JobHandle(JOB* job) : boost::shared_ptr<JOB>(job, AAA_JobDeleter()) {}

  JOB& Job() { return (JOB&)*boost::shared_ptr<JOB>::get(); }
};

/*! jobDeleter A class to delete a job

    This class is used for deleting a job.  This class is used in
    AAA_JobHandle to specify a deleting method other than built-in
    delete method.
*/
class AAA_JobDeleter
{
 public:
  void operator()(AAA_Job *job) { job->Delete(); }
  static void Delete(AAA_Job *job) { job->Delete(); }
};

const unsigned maxNumPriority = 3;
const unsigned maxMaxWeight = 3;

/*! queueJob Job with Queue 

  This class defines a job with a single FIFO queue to store objects
  of any type.  Enqueue, dequeue, remove and flush operations are
  defined in this class.  The LOCK parameter must be either
  ACE_Thread_Mutex, ACE_Recursive_Thread_Mutex, or ACE_Null_Mutex.

*/
template <class T, class LOCK = ACE_Thread_Mutex>
  class AAA_QueueJob : public AAA_Job
{
 public:
  /// Constructor. 
  AAA_QueueJob(AAA_JobData *d=0, char* name=0, 
	       unsigned numPriority=1, unsigned maxWeight=1) throw (int)
 : AAA_Job(d, name), 
 numPriority(numPriority), maxWeight(maxWeight), total(0),
 cond(std::auto_ptr< ACE_Condition<LOCK> >
      (new ACE_Condition<LOCK>(lock))),
 signaled(false)

 {
   if (numPriority > maxNumPriority)
     {
       FRAMEWORK_LOG(LM_ERROR, "[%N] Too many priorities.\n");
       throw 1;
     }
   if (maxWeight > maxMaxWeight)
     {
       FRAMEWORK_LOG(LM_ERROR, "[%N] Too large weight.\n");
       throw 1;
     }
   indexQueue = boost::shared_array< std::list<unsigned> >
   (new std::list<unsigned>[numPriority]);

   dataQueue = boost::shared_array< std::list<T> >
   (new std::list<T>[numPriority*maxWeight]);
 }

  /// Overloaded by derived class's destractors.
  virtual ~AAA_QueueJob() {
   indexQueue.reset();
   dataQueue.reset();
  }

  /// Inherited from the base class.
  virtual int Serve()=0;

  /// Inherited from the base class.
  virtual int Schedule(AAA_Job*, size_t backlogSize=1)=0;

 /// Reimplemented from AAA_Job.
 inline bool ExistBacklog() 
 { 
   // Lock is not needed here, because this does not change the state
   // of the object.
   return (total>0);
 }

 inline size_t BacklogSize() { return total; }

 /// Enqueue an entry.  Returns the total number of entries after the
 /// enqueue operation.  Enqueue() can be blocking when "blocking"
 /// argument is true and the queue is full.
 inline int Enqueue(T entry, bool blocking=false,
		    unsigned priority=1, unsigned weight=1) 
 { 
   AAA_ScopeLock<LOCK> g(lock); 

   // In the case where one or more threads can enter here
   // simultaneously (this is the case when Dequeue is called by a
   // task), conditional mutex is needed to avoid possible blocking
   // on Enqueue() when the queue is full.

   while (!signaled && total==MaxSize()) 
   { 
     if (!blocking)
       {
	 FRAMEWORK_LOG(LM_DEBUG, "[%N] Enqueue is blocked.\n");
	 return 0;
       }
     cond->wait(); 
   }

   if (signaled && blocking)
      return -1;

   unsigned pIndex = Index(priority);
   unsigned dIndex = Index(priority, weight);


   // The enqueueing algorithm is as follows.  

   // 1) Compare the weight of the job with the queue length of
   // the data queue where the job is enqueued.

   // 2) If the queue length of the data queue is less than the
   // weight, then enqueue the index of the data queue to the index
   // queue that is corresponding to the the priority of the job.

   // 3) Enqueue the job to the data queue.

   // Step (2) means that the larger the weight, the more the job
   // gets the chance to be served.  

   if (dataQueue[dIndex].size() < weight)
     indexQueue[pIndex].push_back(dIndex);

   dataQueue[dIndex].push_back(entry); 
   total++;

   // Wake up waiting threads.  This signal() call can be outside of
   // the scope lock.
   cond->signal();
   return total;
  }

 /// Dequeue an entry.  When blocking is false and the queue is empty,
 /// Dequeue() immediately return 0 without performing dequeue.  When
 /// blocking is true, Dequeue() wait until the queue entry is
 /// availble.  When "signaled" is true, (this is the case when job
 /// serving threads need to be deleted), -1 is returned.  When an
 /// entry is successfuly dequeued, 1 is returned.
 int Dequeue(T &entry, bool blocking=false) throw(int)
  { 
    do {
      AAA_ScopeLock<LOCK> g(lock);
      // In the case where one or more threads can enter here
      // simultaneously (this is the case when Dequeue is called by a
      // task), conditional mutex is needed to avoid possible blocking
      // on Dequeue() when the queue is empty.

      while (!signaled && total==0) 
	{ 
	  if (!blocking)
	    return 0;
	  cond->wait(); 
	}

      if (signaled && blocking)
	return -1;

      // The serving algorithm is as follows:

      // 1) Find the highest priority class that has a backlog.

      // 2) Dequeue an index from the index queue of the found
      // class. The dequeued index points the data queue to serve.  

      // 3) Dequeue an entry from the data queue corresponding to the
      // index.

      // 4) After step (3), if the queue length of the data queue is
      // not less than the weight given to that queue, then re-enqueue
      // the index to the index queue of the same priority class.

      // Step 2) of Enqueue() and Step 4) of Dequeue are complement
      // operations, which provides both "work conserving" and "O(1)
      // complexity" characteristics.
      
      do {
	bool found=false;
	for (unsigned p=0; p<numPriority; p++)
	  {
	    if (indexQueue[p].empty())
	      continue;

	    unsigned dIndex = indexQueue[p].front();
	    indexQueue[p].pop_front();
	    entry = dataQueue[dIndex].front();
	    dataQueue[dIndex].pop_front();
	    if (dataQueue[dIndex].size() >= (dIndex % maxWeight) + 1)
	      indexQueue[p].push_back(dIndex);
	    found=true;
	    total--;
	    break;
	  }

	if (!found)
	  {
	    FRAMEWORK_LOG(LM_ERROR, "[%N] BUG (dequeue error).\n");
	    return 0;
	  }

      } while (0);
    } while (0);
    if (total>0) 
      cond->signal();
    return 1;
  }

 /// Remove all entries that match specified entry.  Returns total
 /// number of returned entries.
 inline int Remove(T entry)
 { 
   AAA_ScopeLock<LOCK> g(lock); 
   int totalRemove=0;
   for (unsigned i=0; i<numPriority; i++)
     for (unsigned j=i*maxWeight; j<(i+1)*maxWeight; j++)
     {
       unsigned preSize = (unsigned)dataQueue[j].size();
       dataQueue[j].remove(entry);
       unsigned postSize = (unsigned)dataQueue[j].size();
       if (preSize != postSize)
	 {
	   totalRemove =+ preSize - postSize;
	   total -= preSize - postSize;
	   // Adjust the index queue so that the index for the removed entry 
	   AdjustIndexQueue(i, j);
	 }
     }
   if (totalRemove>0) 
     cond->signal();
   return totalRemove;
 }

  /// Flush the event queue.
  virtual void Flush() 
  { 
    AAA_ScopeLock<LOCK> g(lock); 
    for (unsigned i=0; i<numPriority; indexQueue[i++].clear());
    for (unsigned i=0; i<numPriority*maxWeight; dataQueue[i++].clear());
    total=0;
    cond->signal();
  }

 public:
 void Signal() 
 { 
   signaled=true;
   cond->broadcast(); 
 }

 /// Returns the maximum size of the queue.
 inline size_t MaxSize() { return (size_t)-1; }

 protected:

  /// lock associated with list
  LOCK lock;

 private:

  inline unsigned Index(unsigned p)
    {
      p = (p>maxNumPriority) ? maxNumPriority : p;
      return p-1;
    }

 inline unsigned Index(unsigned p, unsigned w)
    {
      p = (p>maxNumPriority) ? maxNumPriority : p;
      w = (w>maxMaxWeight) ? maxMaxWeight : w;
      return (p-1)*maxWeight + (w-1);
    }

 void AdjustIndexQueue(unsigned pIndex, unsigned dIndex)
 {
   std::list<unsigned> &iQueue = indexQueue[pIndex];
   std::list<unsigned>::iterator i;
   unsigned j=0;
   for (i=iQueue.begin(); i!=iQueue.end(); )
     {
       if ((*i) == dIndex)
	 i = (++j > dataQueue[dIndex].size()) ? iQueue.erase(i) : i++;
       else
	 i++;
     }
 }

 unsigned numPriority;  // number of priority
 unsigned maxWeight;    // maximum weight value

 // This queue stores indexes for the dataQueue per priority.
 boost::shared_array< std::list<unsigned> > indexQueue;

 // This queue stores data for per priority and per weight 
 boost::shared_array< std::list<T> > dataQueue;

 unsigned total;

 /// Used for synchronization among threads.
 std::auto_ptr< ACE_Condition<LOCK> > cond;

 /// Used for sending signal to waiting threads.
 bool signaled;
};

/// Event.
typedef int AAA_Event;

/*! eventQueueJob Job with Event Queue 

  This class defines a job with a single FIFO queue to
  store events.

*/
/// This is the base class of "event queue job".  See \ref
/// tagEventQueueJob for detailed information.
typedef AAA_QueueJob<AAA_Event> AAA_EventQueueJob;

/*! jobQueueJob Job with Job Queue 

  This class defines a backlogging job with a single FIFO queue to
  store pointers to jobs, which realizes hierarchically executed jobs.

*/
typedef AAA_QueueJob<AAA_Job*> AAA_JobQueueJob;

/// Scheduling policy.  AAA_SCHED_WFQ and AAA_SCHED_PRIORITY can be
/// specified at the same time by specifying with (AAA_SCHED_WFQ |
/// AAA_SCHED_PRIORITY)
#define AAA_SCHED_FIFO      0  
#define AAA_SCHED_WFQ       1
#define AAA_SCHED_PRIORITY  2

typedef ACE_UINT16 AAA_SchedulingPolicy;


/*! \page groupedJob Grouped Job 

  This class is used for defining a job that multiplexes one or more
  "subjobs".  Each subjob can be of different types.  For example,
  when this class is used within a single protocol implementation, a
  sub-job may be dedicated to incoming message processing of the
  protocol while another sub-job may be dedicated to state machine
  handling of the protocol.  In another example, a subjob may be
  dedicated to protocol processing of a particular protocol while
  another subjob may be dedicated to protocol processing of another
  procotol.  It is also possible to define a job group in a
  hiearchical manner, by defining a subjob to be a class derived from
  AAA_JobQueueJob.  This class provides the basis of organizing jobs
  in extensible and modular fashion in multithreading environments.

  The grouped job can call its parent job's Schedule() method unless
  it is the root job in the job hierarchy (e.g., the job in AAA_Task).
  A child job can cancel its scheduling request that has been issued
  to its parent job via the parent job's Remove() method.  When a
  child job is processed by its parent job, it is processed as a
  shared object by using Acquire and Release methods.

  The grouped job implements job three scheduling policies FIFO
  queueing, Weighted Fair Queueing (WFQ) and Priority scheduing.  WFQ
  and Priority scheduing can be combined in a way that WFQ is
  performed for each priority class.  When there are jobs with
  different priorities in the job queue, a job with the highest
  priority is always served.

  Application can specify whether Dequeue() method in Serve() is
  blocking or non-blocking, by the "blocking" boolean variable in
  constructor.

  Since this is a shared object, the object is always allocated in
  heap area and should not be deleted unless no other entity shares
  the object.  Thus, both constructor and destructor are defined as
  private methods.  Object allocation needs to be performed via static
  "Create" method.  Object deallocation needs to be performed via
  AAA_Deleter::Delete(AAA_Job*) or AAA_Deletor::()(AAA_Job*) static
  methods.

*/
class AAA_GroupedJob : public AAA_QueueJob<AAA_Job*, ACE_Thread_Mutex>
{
 public:
  /// Default constructor is prohibited.
  static AAA_GroupedJob* 
    Create(AAA_Job &parent, AAA_JobData* d, 
	   char *name=0, 
	   AAA_SchedulingPolicy policy=AAA_SCHED_FIFO,
	   bool blocking=false,
	   unsigned numPriority=1, unsigned maxWeight=1) throw(AAA_Error)
  {
    if (d==0)
      {
	FRAMEWORK_LOG(LM_ERROR, "AAA_JobData must be non-null.\n");
	throw NoData;
      }
    return new AAA_GroupedJob(parent, d, name, policy, blocking,
			      numPriority, maxWeight);
  }

  static AAA_GroupedJob* 
    Create(AAA_JobData* d, char *name=0, 
	   AAA_SchedulingPolicy policy=AAA_SCHED_FIFO,
	   bool blocking=false,
	   unsigned numPriority=1, unsigned maxWeight=1) throw(AAA_Error)
  {
    if (d==0)
      {
	FRAMEWORK_LOG(LM_ERROR, "AAA_JobData must be non-null.\n");
	throw NoData;
      }
    return new AAA_GroupedJob(d, name, policy, blocking,
			      numPriority, maxWeight);
  }

  void Start() throw(AAA_Error) { running=true; }

  void Stop() 
  { 
    running=false; Flush(); 
    Signal();
  }
  
  void Flush()
  {
    while (ExistBacklog())
      {
	AAA_Job* job;
	Dequeue(job);   // Non-blocking dequeue.
	job->Release();
      }
  }

  void Remove(AAA_Job *job)
  {
    // Root job relies on Acquire/Release mechinism instead of
    // Remove().
    int n = AAA_QueueJob<AAA_Job*, ACE_Thread_Mutex>::Remove(job);
    job->Release(n);
  }
	
  /// Reimplemented from AAA_Job.  
  int Schedule(AAA_Job *job , size_t backlogSize=1)
  {
    if (!Running())
      return (-1);

    job->Acquire();

    unsigned priority, weight = 0;
    EnforceSchedulingPolicy(job, priority, weight);

    // If there is already a backlog for this session, do nothing.
    // Job weight is taken into account here.
    if (backlogSize > weight)
      return (0);

    // Since weight is taken into account by above backlog-weight
    // comparison, just specify the priority in Enqueue().
    do {
      int result = Enqueue(job, blocking);
      if (result <= 0)
	return (-1);
    } while (0);

    //    Enqueue(job, priority);

    // If this is not a root job, then ask the parent to schedule me.
    return (!IsRoot()) ? parent.Schedule(this) : 0;
  }

  /// Reimplemented from AAA_Job.
  int Serve()
  {
    AAA_Job* job;

    do {
      int result = Dequeue(job, blocking);
      if (result <= 0)
	return result;
    } while (0);

    if (job->Release() == -1)
      return (int)BacklogSize();

    unsigned childBacklogSize = job->Serve();

    unsigned priority, weight = 0;
    EnforceSchedulingPolicy(job, priority, weight);

    // If there is already a backlog for this session, do nothing.
    // Job weight is taken into account here.
    if (childBacklogSize < weight)
      return (int)BacklogSize();

    job->Acquire();

    // Since weight is taken into account by above backlog-weight
    // comparison, just specify the priority in Enqueue().
    return Enqueue(job, priority ? true : false);
  }

 protected:

 private:
  /// Constructor for non-root job.
   AAA_GroupedJob(AAA_Job &parent, 
		  AAA_JobData* d, 
		  char *name=0,
  		  AAA_SchedulingPolicy policy=AAA_SCHED_FIFO,
		  bool blocking=false, 
		  unsigned numPriority=1, unsigned maxWeight=1)
     : AAA_QueueJob<AAA_Job*, ACE_Thread_Mutex>(d, name, 
						numPriority, maxWeight), 
    parent(parent), policy(policy), running(false), blocking(blocking)
  {
    // Increment the reference counter of the parent.
    parent.Acquire();
  }

  /// Constructor for root job.
  AAA_GroupedJob(AAA_JobData* d, char *name=0,
		 AAA_SchedulingPolicy policy=AAA_SCHED_FIFO, 
		 bool blocking=false,
		 unsigned numPriority=1, unsigned maxWeight=1)
    : AAA_QueueJob<AAA_Job*, ACE_Thread_Mutex>(d, name,
					       numPriority, maxWeight), 
    parent(*this), policy(policy), running(false), blocking(blocking)
  {}

  /// Destructor.  This class is defined as a leaf class.
  ~AAA_GroupedJob() { 
    Stop(); 
    if (!IsRoot())
      parent.Release(); 
  }

  /// Indicates whether this is the root job in a job hierarchy.
  inline bool IsRoot() { return (parent == this); }
  

  /// Enforce the scheduling policy and obtain the adjusted priority
  /// and weight as a result.
  void EnforceSchedulingPolicy(AAA_Job *job, 
			       unsigned &priority,
                               unsigned &weight)
  {
    /// Infinite job scheduling weight.
    const int infinity = 100000;
 
    priority = job->Priority();
    weight = job->Weight();

    // Adjust the priority and weight to the scheduling policy.
    if (policy != AAA_SCHED_WFQ)
      weight = infinity;
    
    if (policy != AAA_SCHED_PRIORITY)
      priority = 1;
  }

  /// Parent job.
  AAA_Job& parent;

  /// Scheduling policy.  AAA_SCHED_WFQ and AAA_SCHED_PRIORITY can be
  /// specified at the same time by specifying with (AAA_SCHED_WFQ |
  /// AAA_SCHED_PRIORITY)
  unsigned policy;

  bool running;

  /// Specifies whether the Dequeue() operation in Serve() method is
  /// blocking or nor.
  bool blocking;
};

/*! \page task Task

This is the base class of "task", which consists of one thread that
handles timer events and one or more thread that processes "jobs."  A
task is the root job of in a job hierarchy.

*/
/// The base class for "task".  See \ref tagTask for detailed
/// information.
class AAA_Task : public ACE_Task<ACE_MT_SYNCH>
{
public:

  /// Constuctor.  The int parameter specifies the number of job
  /// serving threads and the name parameter specifies the name of the
  /// task.  The policy parameter specifies the scheduling policy
  /// represented by a "bit-or" combination of AAA_SCHED_FIFO,
  /// AAA_SCHED_WFQ and AAA_SCHED_PRIORITY.  The task name will be
  /// used for logging purpose.  The number of priority classes and
  /// the maximum weight value can also be specified.
  AAA_Task(AAA_SchedulingPolicy policy=AAA_SCHED_FIFO, char *name=0,
	   unsigned numPriority=1, unsigned maxWeight=1) 
    : handle
    (AAA_JobHandle<AAA_GroupedJob>
     (AAA_GroupedJob::Create((AAA_JobData*)this, name, policy, true,
			     numPriority, maxWeight))),
    cond(std::auto_ptr< ACE_Condition<ACE_Mutex> >
	 (new ACE_Condition<ACE_Mutex>(mutex)))
    {
      reactor(0);
    }

  virtual ~AAA_Task()
  {
     Stop();
  }
  
  /// Activate specified number of threads.  A negative integer is
  /// thrown when the task could not be started.
  void Start(int n) throw(AAA_Error)
  {
    if (Running())
      return;

    if (activate(THR_NEW_LWP|THR_JOINABLE, n+1) == -1)
      {
	throw ActivationFailure;
      }

    // Wait until the timer thread is up and running.
    do {
      AAA_MutexScopeLock guard(mutex);
      while (!Running()) { cond->wait(); }
    } while (0);

    Job().Start();
  }
    
  /// Wait until all threads complete.
    
  void Stop()
  {
    if (!Running())
      return;

    // Send delete signal to threads waiting on scheduling queue.
    Job().Stop();

    // Send signal to delete the timer thread.
    reactor()->schedule_timer(this, (const void*)0, ACE_Time_Value(0));
	
    // Wait for all activated threads to complete.
    wait(); 
  }

  /// Schedule a timer event to occur with
  /// a specified time delay. The timer event will be continuously
  /// occur at the specified interval parameter if the interval is
  /// non-zero.  The specified argment is used when the event is
  /// executed.
  inline long ScheduleTimer(ACE_Event_Handler* eventHandler, const void* arg, 
			    ACE_Time_Value delay, ACE_Time_Value interval=ACE_Time_Value::zero)
  {
    if (Running())
      return reactor()->schedule_timer(eventHandler, arg, delay, interval);
    return (-1);
  }

  /// Cancel the scheduled timer.  The canceled timer argument is
  /// returned in the second argument.
  inline int CancelTimer(ACE_UINT32 timerHandle, const void** arg)
  {
    if (Running())
      return reactor()->cancel_timer(timerHandle, arg);
    return (-1);
  }

  inline bool Running() { return reactor() ? true : false; }

  inline AAA_GroupedJob& Job() { return handle.Job(); }

  inline AAA_JobHandle<AAA_GroupedJob>& JobHandle() { return handle; }

 protected:

 private:

  /// This function is reimplementation of virtual functions of
  /// the parent class.
  int svc()
  {
    mutex.acquire();
    if (!reactor())
      {
	std::auto_ptr<ACE_Reactor> r(new ACE_Reactor);
	reactor(r.get());
	mutex.release();
	cond->signal();
	while (1) // Loop for timer thread loop.
	  if (reactor()->handle_events() == -1)
	    FRAMEWORK_LOG(LM_ERROR, "[%N] handle_events().\n", 
			  Job().Name().c_str());
	return 0;
      }
    mutex.release();
    while (1)  // Loop for serving threads.
      {
	if (Job().Serve()<0)
	  return 0;
      }
    return 0;
  }
  
  int close(u_long flags=0) { 
    return 0; 
  }

  int handle_timeout(const ACE_Time_Value &tv, const void *arg)
  {
    reactor(0);
    ACE_Thread::exit();
    return 0;
  }

  AAA_JobHandle<AAA_GroupedJob> handle;

  /// Used for synchronization among threads.
  std::auto_ptr< ACE_Condition<ACE_Mutex> > cond;

  /// Used for synchronization among threads.
  ACE_Mutex mutex;
};

/*! \page State Machine

    This is a general API used for handling protocol state machines.
    Applications can use the API to define a state machine for any
    protocol.

*/
/// State.
typedef int AAA_State;

/// Action.  Action functions needs to be defined as () operator
/// funtions.
template <class ARG> class AAA_Action 
{
 public:
  virtual void operator()(ARG&)=0;
 protected:
  virtual ~AAA_Action() {}
  AAA_Action() {}
};

/// A special class for action with null operation.  Used as a
/// singleton in class AAA_NullAction.
template <class ARG> class AAA_NullAction_S : public AAA_Action<ARG>
{
  friend class ACE_Singleton<AAA_NullAction_S<ARG>, 
    ACE_Recursive_Thread_Mutex>;
 public:
  void operator()(ARG&) {}
 private:
  AAA_NullAction_S() {}
  ~AAA_NullAction_S() {}
};

/// A class that is used for generating a reference to the instance of
/// AAA_NullAction_S<ARG>.
template <class ARG> class AAA_NullAction 
{
 public:
  AAA_Action<ARG>& operator()() 
  { 
    return *ACE_Singleton<AAA_NullAction_S<ARG>, ACE_Recursive_Thread_Mutex>::
      instance();
  }
};

/*! \page stateTableEntry State Table Entry

  A state table entry consists of previous state, event, next state,
  action and a flag to indicate if the event is wildcard.  There are
  two constructors, one for normal events and the other for wildcard
  event.  ARG represents the class of the argment of the () operator
  of each action.

*/
/// This is the base class for state entry.  See \ref tagStateTable
/// for more information.
template <class ARG> class AAA_StateTableEntry 
{
public:
  /// Constructor with specifying state, event, next_event and action.
  AAA_StateTableEntry(AAA_State st1, AAA_Event ev, AAA_State st2, 
		      AAA_Action<ARG>& ac=AAA_NullAction<ARG>()()) : 
  prevState(st1), event(ev), nextState(st2), action(ac), isWildcardEvent(false)
  {}
  /// Constructor without specifying event (i.e., matching any event).
  AAA_StateTableEntry(AAA_State st1, AAA_State st2, 
		      AAA_Action<ARG>& ac=AAA_NullAction<ARG>()()) :
  prevState(st1), nextState(st2), action(ac), isWildcardEvent(true)
  {}

  /// Destructor.
  ~AAA_StateTableEntry() {}

  inline AAA_State PrevState() { return prevState; }
  inline AAA_Event Event() { return event; }
  inline AAA_State NextState() { return nextState; }
  inline AAA_Action<ARG>& Action() { return action; }
  inline bool IsWildcardEvent() { return isWildcardEvent; }

 protected:
  AAA_State   prevState;   
  AAA_Event   event;       
  AAA_State   nextState;
  AAA_Action<ARG>& action;
  bool isWildcardEvent;
};

/*! \page stateTable State Table

  A state table contains a list of state table entries (\ref
  stateTableEntry), which defines the behavior of the state machine.
  The state table contains a special entry called <em>initial state
  entry</em> from which each state machine starts its operation.  It
  is expected that the list of state table entries are constucted in
  the constructor by using AddStateTableEntry() and
  AddWildcardStateTableEntry() functions.  Each state table is defined
  as a static class and the single object instance are shared among
  multiple instances of state machines that use the state table.

*/
/// The base template class for state table (see \ref tagStateTable).
template <class ARG> class AAA_StateTable : 
public std::list< AAA_StateTableEntry <ARG> * >
{
public:
  AAA_StateTable() : isInitialStateSet(false) {}

  virtual ~AAA_StateTable() 
  { 
    for (typename std::list< AAA_StateTableEntry <ARG> * >::iterator 
	 i=std::list< AAA_StateTableEntry <ARG> * >::begin(); 
         i!=std::list< AAA_StateTableEntry <ARG> * >::end();
         i ++) {
         delete (*i);
    }
  }

  
  /// This function is used for getting an initial state
  AAA_State InitialState() throw (AAA_Error)
  {
    if (! isInitialStateSet)
      throw NoInitialState;
    return initialState; 
  }

  /// This function is used for setting an initial state
  void InitialState(AAA_State st) 
  {
    initialState = st; 
    isInitialStateSet = true;
  }

  /// This function is used for finding an entry for a specific pairw
  /// of state and event.
  bool FindStateTableEntry(AAA_State st, AAA_Event ev, 
			   AAA_StateTableEntry<ARG>*& entry)
  {
    bool found=false;
    for (typename std::list< AAA_StateTableEntry <ARG> * >::iterator 
	 i=std::list< AAA_StateTableEntry <ARG> * >::begin(); 
         i!=std::list< AAA_StateTableEntry <ARG> * >::end(); 
         i++)
      {
	AAA_StateTableEntry<ARG> *e = *i;
	if (e->PrevState() != st)
	  continue;

	if (e->IsWildcardEvent())
	  {
	    entry = e;
	    found=true;
	    continue;
	  }

	if (e->Event() == ev)
	  {
	    entry = e;
	    return true;
	  }
      }
    return found;
  }

  /// This function is used for finding an entry for a specific state 
  /// with wildcard event.
  bool FindStateTableEntry(AAA_State st, AAA_StateTableEntry<ARG>*& entry)
  {
    for (typename std::list< AAA_StateTableEntry <ARG> * >::iterator 
	 i=std::list< AAA_StateTableEntry <ARG> * >::begin(); 
         i!=std::list< AAA_StateTableEntry <ARG> * >::end(); 
         i++)
      {
	entry  = *i;
	if (entry->PrevState() != st)
	  continue;
	if (entry->IsWildcardEvent())
	  return true;
      }
    return false;
  }

protected:
  /// This function is used for adding an entry for a specific event.
  void AddStateTableEntry(AAA_State pSt, AAA_Event ev, 
			  AAA_State nSt, 
			  AAA_Action<ARG>& ac= AAA_NullAction<ARG>()())
    throw (AAA_Error)
  {
    AAA_StateTableEntry<ARG> *dummy;
    if (FindStateTableEntry(pSt, ev, dummy))
      {
	throw FoundDuplicateStateTableEntry;
      }
    push_back(new AAA_StateTableEntry<ARG>(pSt, ev, nSt, ac));
  }

  /// This function is used for adding an entry for wildcard event.
  void AddWildcardStateTableEntry(AAA_State pSt, AAA_State nSt, 
				  AAA_Action<ARG>& ac= AAA_NullAction<ARG>()())
      throw (AAA_Error)
  {
    AAA_StateTableEntry<ARG> *dummy;
    if (FindStateTableEntry(pSt, dummy))
      throw FoundDuplicateStateTableEntry;

    push_back(new AAA_StateTableEntry<ARG>(pSt,nSt,ac));
  }
private:
  AAA_State initialState;
  bool isInitialStateSet;
};

/*! \page stateMachine State Machine

  This is the base class for handling state machines in general.  A
  state machine consists of a set of state variables, a set of
  functions to manage the state variables and a state transition table
  (or a <b>state table</b> see \ref stateTable).  The base class does
  not handle timers.  For state machines with timer event handling,
  AAA_StateMachineWithTimer (or child classes of
  AAA_StateMachineWithTimer) can be used.

*/
/// The base state machine class.  See \ref tagStateMachine
class AAA_StateMachineBase
{
public:
  /// This function is used for starting the state machine.
  virtual void Start()=0;

  /// This function is used for stopping the state machine.
  virtual void Stop()=0;

  /// This function is used for restarting the state machine.
  virtual void Restart() { Stop(); Start(); }

  /// This function is used for checking whether the state machine is running
  virtual bool Running()=0;

  /// This function is used for passing event to the instance of this
  /// class.
  virtual void Event(AAA_Event ev)=0;

  /// Reference assignment operator.
  AAA_StateMachineBase& operator=(AAA_StateMachineBase& sm) { return sm; }

  /// This function is used for passing event to the instance of this
  /// class.
  std::string& Name() { return name; }

protected:
  AAA_StateMachineBase(char *name=0) 
  {
    if (name)
      this->name = std::string(name);
  }

  /// Overloaded by derived class's destractors.
  virtual ~AAA_StateMachineBase() {}

 private:
  std::string name;
};

/// Abstract class for state machine with action argment type (see
/// \ref stateMachine).
template <class ARG> 
class AAA_StateMachine : public AAA_StateMachineBase
{
public:
  /// Overloaded by derived class's destractors.
  virtual ~AAA_StateMachine() {}

  /// This function is used for starting the state machine.
  virtual void Start() throw(AAA_Error)
  { 
    state = stateTable.InitialState();
    running = true;
  }

  /// This function is used for stopping the state machine.
  inline void Stop() { running=false; }

  /// This function is used for checking whether the state machine has been 
  /// started.
  inline bool Running() { return running; }

  /// This function is used for passing event to the instance of this
  /// class.

  void Event(AAA_Event ev)
  {
    // Throw an exception if state machine is not started.
    if (!running)
      {
	FRAMEWORK_LOG(LM_ERROR, 
		      "StateMachine[%s] state machine is not running.\n", 
		      Name().c_str());
	return;
      }

    AAA_StateTableEntry<ARG>* entry = 0;

    // Search state table for StateTableEntry containing the event
    // defined in the current state.
    if (!stateTable.FindStateTableEntry(state, ev, entry))
      {
	// Cannot found state table that accepts event.
	FRAMEWORK_LOG
	  (LM_ERROR, 
	   "StateMachine[%s] cannot accept event %d at state %d.\n", 
	   Name().c_str(), ev, state);
	return;
      }

    // Change the state.
    state = entry->NextState();

    // Execute the action for the current StateTableEntry.
    entry->Action()(actionArg);
  }

  /// Reference assignment operator.
  AAA_StateMachine<ARG>& operator=(AAA_StateMachine<ARG>& sm) { return sm; }

protected:
  AAA_StateMachine(ARG &arg, AAA_StateTable<ARG> &table, char *name=0)
    : AAA_StateMachineBase(name),
    stateTable(table), actionArg(arg), running(false)
  {}

  AAA_StateTable<ARG>& stateTable;
  AAA_State state;
  ARG& actionArg;
private:
  /// If this value is false, no new event is not scheduled, but
  /// already scheduled event will be executed.
  bool running;
};

/*! \page timerTypeAllocator Timer Type Allocator

  Timer type allocator is used for allocating timer types used in
  AAA_StateMachineWithTimer.  The allocator is used as singleton.
  Call Allocate() method for actual allocation.

*/
/// Timer type allocator.  See \ref timerTypeAllocator.
class AAA_TimerTypeAllocator_S
{
  friend class ACE_Singleton<AAA_TimerTypeAllocator_S, 
    ACE_Recursive_Thread_Mutex>;
 public:
  inline int Allocate() { return ++lastAllocatedType.value_i(); }
 private:
  AAA_TimerTypeAllocator_S() {}
  ~AAA_TimerTypeAllocator_S() {}
  ACE_Atomic_Op<ACE_Thread_Mutex, int> lastAllocatedType;
};

/// The singleton for AAA_TimerTypeAllocator.
typedef ACE_Singleton<AAA_TimerTypeAllocator_S, ACE_Recursive_Thread_Mutex> 
AAA_TimerTypeAllocator;

typedef std::map<int,ACE_UINT32> TimerHandleMap;

/*! \page stateMachineWithTimer State Machine With Timer Management

  This state machine supports timer event handling.  The state machine
  maintains multiple timer handlers, where one timer handler is
  assigned for each timer type.  For example, one can use different
  timer handlers for message retransmission timers and session timeout
  timers. It is the responsibility of the appliction to classify the
  timers into different types.  A unique timer type is assigned by
  using AAA_TimerTypeAllocator.  The timer type 0 (zero) is used by
  default.

*/
/// State machine with timer handling.  See \ref
/// stateMachineWithTimer.
template <class ARG>
class AAA_StateMachineWithTimer : public AAA_StateMachine<ARG>
{
public:
  /// This function is used for scheduling a timer event.  Once
  /// scheduled, the same timer event will occur at every specified
  /// interval.  To stop the timer event, use CancelTimer().
  void ScheduleTimer(AAA_Event ev, ACE_UINT32 sec, 
		     ACE_UINT32 usec=0, int type=0)
  {
    AAA_Event *eventP = new AAA_Event(ev);
    ACE_UINT32 timerHandle;
    // Cancel the already scheduled timer for this type.
    CancelTimer(type);
    timerHandle =  reactor.schedule_timer(
          (ACE_Event_Handler*)&timerEventHandler, 
	  (const void*)eventP, 
	  ACE_Time_Value(sec, usec), 
	  ACE_Time_Value(sec, usec));
    timerHandleMap.insert(std::pair<int, ACE_UINT32>(type, timerHandle));
  }

  /// This function is used for canceling a timer event.  
  void CancelTimer(int type=0) 
  {
    AAA_Event *ev;

    ACE_UINT32 timerHandle;
    TimerHandleMap::iterator i;
    i = timerHandleMap.find(type);
    if (i == timerHandleMap.end())
      return;

    timerHandle = i->second;
    timerHandleMap.erase(i);
    if (reactor.cancel_timer(timerHandle, (const void**)&ev) == 1)
      delete ev;
  }

  /// Cancel all scheduled timers for this state machine.
  void CancelAllTimer()
  {
    while (! timerHandleMap.empty()) {
      TimerHandleMap::iterator i = timerHandleMap.begin();
      CancelTimer(i->first);  
    }
  }

 protected:
  // separate class derivation for ACE_Event_Handler
  // to protect against changes to ACE_Event_Handler
  class AAA_FsmTimerEventHandler : public ACE_Event_Handler
  {
  public:
    AAA_FsmTimerEventHandler(AAA_StateMachineWithTimer<ARG> &fsm)
                             : fsm_ref(fsm) { }

  private:
    /// reimplementated from ACE_Event_Handler
    int handle_timeout(const ACE_Time_Value &tv, const void *arg)
    {
      AAA_Event event = *(AAA_Event*)arg;

      /// Important: The pointer to AAA_Event must not be deleted here.
      /// This is because the return value of handle_timeout is always
      /// zero (0), which means that the same timer event will be
      /// scheduled by the reactor.
      fsm_ref.Timeout(event);
      return (tv ? 0 : 0);
    }
    AAA_StateMachineWithTimer<ARG> &fsm_ref;
  };
    
  /// Constructor.  ACE_Reactor maintains the timer queue and is bound
  /// to a specific thread.
  AAA_StateMachineWithTimer(ARG &arg, AAA_StateTable<ARG> &table, 
			    ACE_Reactor &r, char *name=0)
      : AAA_StateMachine<ARG>(arg, table, name), reactor(r),
        timerEventHandler(*this)        
  {}

  /// Overloaded by derived class's destractors.
  virtual ~AAA_StateMachineWithTimer()  { CancelAllTimer(); }

  /// This function is called from handle_timeout() when a timer event
  /// occurs.
  virtual void Timeout(AAA_Event ev)=0;

private:
  ACE_Reactor& reactor;
  TimerHandleMap timerHandleMap;
  AAA_FsmTimerEventHandler timerEventHandler;
};

/*!
 *! \brief AAALogMsg Generic/Global log facility
 * Log facility
 * Open Diameter logging facity derived directly from ACE
 */
class AAALogMsg :
    public ACE_Log_Msg
{
    friend class ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex>;    /**< ACE logger */

    private:
        /*
         * protected constructors/destructors to prevent derivation
         */
        AAALogMsg() {
        }
        ~AAALogMsg() {
        }
};

typedef ACE_Singleton<AAALogMsg, ACE_Recursive_Thread_Mutex> AAALogMsg_S;
ACE_EXPORT_SINGLETON_DECLARE(ACE_Singleton, AAALogMsg, ACE_Recursive_Thread_Mutex);

#define AAA_LOG(X) do { \
                         AAALogMsg_S::instance()->log X; \
                     } while(0)

template <class ARG>
class AAA_ProtectedQueue :
   private std::list<ARG>
{
   public:
      virtual ~AAA_ProtectedQueue() {
      }
      virtual void Enqueue(ARG arg) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         std::list<ARG>::push_back(arg);  
      }
      virtual ARG Dequeue() {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         ARG a = std::list<ARG>::front();
         std::list<ARG>::pop_front();
         return a;
      }
      virtual bool IsEmpty() {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         return std::list<ARG>::empty() ? true : false;
      }

   private:
      ACE_RW_Mutex m_Lock;
};

template <class ARG>
class AAA_IterAction
{
   public:
      // return TRUE to delete entry in iteration
      // return FALSE to sustain entry
      virtual bool operator()(ARG&)=0;

   protected:
      virtual ~AAA_IterAction() {
      }
      AAA_IterAction() {
      }
};

template <class ARG>
class AAA_IterActionDelete :
   public AAA_IterAction<ARG>
{
   public:
      // return TRUE to delete entry in iteration
      // return FALSE to sustain entry
      virtual bool operator()(ARG&) {
         return true;
      }
      virtual ~AAA_IterActionDelete() {
      }
};

template <class ARG>
class AAA_IterActionNone :
   public AAA_IterAction<ARG>
{
   public:
      // return TRUE to delete entry in iteration
      // return FALSE to sustain entry
      virtual bool operator()(ARG&) {
         return false;
      }
      virtual ~AAA_IterActionNone() {
      }
};

template <class INDEX,
          class DATA>
class AAA_ProtectedMap  :
   private std::map<INDEX, DATA>
{
   public:
      virtual ~AAA_ProtectedMap() {
      }
      virtual void Add(INDEX ndx, DATA data) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         std::map<INDEX, DATA>::insert(std::pair
             <INDEX, DATA>(ndx, data));
      }
      virtual bool Lookup(INDEX ndx, DATA &data) {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i;
         i = std::map<INDEX, DATA>::find(ndx);
         if (i != std::map<INDEX, DATA>::end()) {
             data = i->second;
             return (true);
         }
         return (false);
       }
      virtual bool Remove(INDEX ndx,
                          AAA_IterAction<DATA> &e) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i;
         i = std::map<INDEX, DATA>::find(ndx);
         if (i != std::map<INDEX, DATA>::end()) {
             e(i->second);
             std::map<INDEX, DATA>::erase(i);
             return (true);
         }
         return (false);
      }
      virtual bool IsEmpty() {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         return std::map<INDEX, DATA>::empty() ? true : false;
      }
      virtual void Iterate(AAA_IterAction<DATA> &e) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i =
             std::map<INDEX, DATA>::begin();
         while (i != std::map<INDEX, DATA>::end()) {
             if (e(i->second)) {
                 typename std::map<INDEX, DATA>::iterator h = i;
                 i ++;
                 std::map<INDEX, DATA>::erase(h);
                 continue;
             }
             i ++;
         }
      }

   private:
      ACE_RW_Mutex m_Lock;
};

template <class ARG>
class AAA_ProtectedPtrQueue
{
   public:
      void Enqueue(std::auto_ptr<ARG> a) {
         m_Queue.Enqueue(a.release());
      }
      std::auto_ptr<ARG> Dequeue() {
         std::auto_ptr<ARG> arg(m_Queue.Dequeue());
         return arg;
      }

   private:
      AAA_ProtectedQueue<ARG*> m_Queue;
};

template <class ARG>
class AAA_ProtectedPtrMap
{
   public:
      void Enqueue(std::auto_ptr<ARG> a) {
         m_Queue.Enqueue(a.release());
      }
      std::auto_ptr<ARG> Dequeue() {
         std::auto_ptr<ARG> arg(m_Queue.Dequeue());
         return arg;
      }

   private:
      AAA_ProtectedQueue<ARG*> m_Queue;
};

class AAA_RangedValue
{
   public:
      typedef enum {
          DEFAULT_LOW  = 0,
          DEFAULT_HIGH = 3,
      };

   public:
      AAA_RangedValue(int level = DEFAULT_LOW,
                      int low = DEFAULT_LOW, 
                      int high = DEFAULT_HIGH) {
          Reset(level, low, high);
      }
      virtual ~AAA_RangedValue() {
      }
      virtual int operator++() {
          m_CurrentLevel += 1;
          return (m_CurrentLevel > m_HighThreshold) ? true : false;
      }
      virtual int operator--() {
          m_CurrentLevel -= 1;
          return (m_CurrentLevel < m_LowThreshold) ? true : false;
      }
      virtual int operator()() {
          return m_CurrentLevel;
      }
      virtual bool InRange() {
          return ((m_CurrentLevel > m_LowThreshold) &&
                  (m_CurrentLevel < m_HighThreshold));
      }
      void Reset(int level = DEFAULT_LOW,
                 int low = DEFAULT_LOW, 
                 int high = DEFAULT_HIGH) {
          m_CurrentLevel = level;
          m_LowThreshold = low;
          m_HighThreshold = high;
      } 

   private:
      int m_CurrentLevel;
      int m_LowThreshold;
      int m_HighThreshold;
};

#endif // __FRAMEWORK_H__
