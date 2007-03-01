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

// victor fajardo: sample accounting server

#include "diameter_api.h"

class AAA_SampleRecStorage : 
    public DiameterServerAcctRecStorage
{
        // see definition of DiameterServerAcctRecStorage
        // in aaa_session_acct_server_fsm.h
    public:
        virtual bool IsSpaceAvailableOnDevice() {
           /// Checks the server application if there is 
           /// enough storage space to hold the next
           /// record. Application must check thier specific
           /// storage schemes.
           return (true);
        }

        virtual void StoreRecord(AAAAvpContainerList &avpList,
                                 int recordType,
                                 int recordNum) {
           /// Asks the server application to store 
           /// the newly arrived record. Note that 
           /// the server application must be aware of
           /// the application specific AVP's it needs to
           /// process. Also, avpList contains all the
           /// relevant avp's in the ACR message such
           /// as Accounting-Sub-Session-Id, Acct-Session
           /// -Id ... etc which the application may wish
           /// to track
           DiameterUtf8AvpContainerWidget recAvp(avpList);
           diameter_utf8string_t *rec = recAvp.GetAvp("Example-Accounting-Record");
           if (rec) {
               std::cout << "Record: " << *rec << std::endl;
           }
        }
        
        virtual void UpdateAcctResponse(DiameterMsg &aca) {
           /// If you wish to add AVP's to the ACA
           /// before it is sent, you need to override
           /// this method and insert your AVP's here
           
           // as an example, add a timestamp to your aca
           time_t currentTime = time(0);
           if (currentTime > 0) {
              DiameterTimeAvpWidget tstampAvp("Event-Timestamp");
              tstampAvp.Get() = currentTime;
              aca.acl.add(tstampAvp());
           }
        }
};

class AAA_SampleServer : 
    public DiameterServerAcctSession<AAA_SampleRecStorage>
{
        // AAA serve session derived from DiameterServerAcctSession.
        // It provides for all the functionality of a diameter 
        // accounting server session. Note that the server 
        // session factory is responsible for instantiating 
        // this object. DiameterServerAcctSession is also a template
        // class that requires an DiameterServerAcctRecStorage derived
        // class as a parameter.
    public:
        AAA_SampleServer(AAA_Task &task,
                         diameter_unsigned32_t id) :
            DiameterServerAcctSession<AAA_SampleRecStorage>
                   (task, 
                    id, 
                    true) // dictates whether this session is stateful 
        {             
        }
        /// This function is used for setting realtime required 
        /// AVP as a hint to the server
        virtual void SetRealTimeRequired
        (DiameterScholarAttribute<diameter_enumerated_t> &rt)
        {
            /// The following are possible values:
            ///   DIAMETER_ACCT_REALTIME_DELIVER_AND_GRANT
            ///   ACCT_REALTIME_GRANT_AND_STORE
            ///   ACCT_REALTIME_GRANT_AND_LOSE
            rt = DIAMETER_ACCT_REALTIME_DELIVER_AND_GRANT;
        }
        /// This function is used for setting acct interim 
        /// interval AVP dictated by the server to client
        virtual void SetInterimInterval
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            timeout = 2; // tell client to generate record every 2 sec
        }
        virtual AAAReturnCode Success() {
            // notification of successful ACR exchange for all record type
            AAA_LOG((LM_INFO, "(%P|%t) **** record exchange completed ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Failed(int recNum) {
            // notification that recNum record was not processed properly
            AAA_LOG((LM_INFO, "(%P|%t) **** record #%d not processed ****\n", recNum));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout if this
            // session was stateful
            AAA_LOG((LM_INFO, "(%P|%t) **** session timeout ****\n"));
            return (AAA_ERR_SUCCESS);
        }
};

// Server session factory. Unlike AAA clients, server
// sessions need to be created on demand. This factory
// is responsible for creating new server sessions
// based on incomming new request.
typedef DiameterServerSessionAllocator<AAA_SampleServer> 
        SampleServerAllocator;

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task, "config/isp.local.xml");
   SampleServerAllocator allocator(task, 20000);
   appCore.RegisterServerSessionFactory(allocator);

   while (true) {
      std::cout << "Just wait here and let factory take care of new sessions" << std::endl;
      ACE_OS::sleep(10);
   }

   appCore.Close();
   task.Stop();
   return (0);
}






