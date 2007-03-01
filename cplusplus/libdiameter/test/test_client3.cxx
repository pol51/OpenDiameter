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

// victor fajardo: sample accounting client 

#include "diameter_api.h"

static int msgCountPerSession = 5;
static char *cfgFile = "config/nas1.local.xml";

class AAA_SampleClientAcctRecCollector : 
    public DiameterClientAcctRecCollector
{
    public:
        virtual void GenerateRecord(AAAAvpContainerList &avpList,
                                    int recordType,
                                    int recordNum) {
           /// Called by the library to ask the client application
           /// to generate records that will be stored in vendor 
           /// specific AVP's. These avp's will then be added to
           /// the avpList before sending the ACR

	   /// Note that Example-Accounting-Record AVP is appended
           /// to the ACR dictionary definition in dictionary.xml.
           /// Real applications may also need to append Vendor-
           /// Specific-Application-Id here
           DiameterUtf8AvpWidget recAvp("Example-Accounting-Record");
           recAvp.Get() = "My example record should show up in the server";
           avpList.add(recAvp());
	}

        virtual bool IsLastRecordInStorage() {
           /// Checks the client app if there is a a record
           /// that has temporarily been stored due to prior
           /// transmission failure. The check should be
           /// based on application specific storage
           return (false);
	}

        virtual bool IsStorageSpaceAvailable() {
           /// Checks the client app if it will be able
           /// to store a record in case transmission 
           /// fails. 
           return (true);
	}

        virtual AAAReturnCode StoreLastRecord(int recordType) {
           /// Asks the client app to temporarily 
           /// store the last known record. This maybe 
           /// due to transmission and the library is
           /// attempting to retry. Applications MUST 
           /// provide thier own storage here
           return (AAA_ERR_SUCCESS);
	}

        virtual AAAReturnCode DeleteLastRecord(int recordType) {
           /// Asks the client app to delete the
           /// last temporarily stored record if any
           return (AAA_ERR_SUCCESS);
	}
};

class AAA_SampleClientSubSession : 
    public DiameterClientAcctSubSession<AAA_SampleClientAcctRecCollector> {
        // AAA client session derived from DiameterClientAcctSession.
        // It provides for all the functionality of a diameter 
        // client accounting session. The ClientAcctSubSession
        // class is a template function that requires a proper
        // version of a record collector as a paramter. Note 
        // that the application is responsible for instantiating 
        // this object
    public:
        AAA_SampleClientSubSession(DiameterClientAcctSession &parent) :
            DiameterClientAcctSubSession<AAA_SampleClientAcctRecCollector>(parent),
            m_HowManyRecProcessed(0) {
        }
        virtual void SetDestinationHost
        (DiameterScholarAttribute<diameter_identity_t> &dHost)
        {
            // optional override, called by the library to 
            // set the destination host. Note that this 
            // overrides applications sending a destination
            // host AVP
            dHost = "server.isp.net";
        }
        virtual void SetDestinationRealm
        (DiameterScholarAttribute<diameter_identity_t> &dRealm)
        {
            // optional override, called by the library 
            // to set the destination realm. Note that 
            // this overrides applications sending a 
            // destination realm AVP
            dRealm = "isp.net";
        }
        /// This function is used for setting realtime required 
        /// AVP as a hint to the server
        virtual void SetRealTimeRequired
        (DiameterScholarAttribute<diameter_enumerated_t> &rt)
        {
        }
        /// This function is used for setting acct interim 
        /// interval AVP as a hint to the server
        virtual void SetInterimInterval
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
        }
        /// This function is used for setting RADIUS acct 
        /// session id for RADIUS/DIAMETER translations
        virtual void SetRadiusAcctSessionId
        (DiameterScholarAttribute<diameter_octetstring_t> &sid)
        {
        }
        /// This function is used for setting multi-session 
        /// id AVP 
        virtual void SetMultiSessionId
        (DiameterScholarAttribute<diameter_utf8string_t> &sid)
        {
        }
        virtual AAAReturnCode Success() {
            // notification of successful ACR exchange for all record type
            AAA_LOG((LM_INFO, "(%P|%t) **** record exchange completed ****\n"));
            m_HowManyRecProcessed ++;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Failed(int recNum) {
            // notification that recNum record was not processed properly
            AAA_LOG((LM_INFO, "(%P|%t) **** record #%d not processed ****\n", recNum));
            return (AAA_ERR_SUCCESS);
        }
        int NumberOfRecordsProcessed() {
            return m_HowManyRecProcessed;
	}
    private:
        int m_HowManyRecProcessed; 
};

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task);
   if (appCore.Open(cfgFile) == AAA_ERR_SUCCESS) {

       /// Wait for connectivity
       do {
           std::cout << "Waiting till this AAA has connectivity" << std::endl;
           ACE_OS::sleep(1);
       } while (appCore.NumActivePeers() == 0);

       /// Each accounting session can have multiple
       /// sub-sessions. The main/parent session has
       /// the Session-Id and each sub session has
       /// an Accounting-Sub-Session-Id
       DiameterClientAcctSession parent(task, 20000);
       for (int x = 0; x < 5; x ++) {
           // A new sub session id is created for each new sub session
           AAA_SampleClientSubSession subSession(parent);

           int numMsgToWaitFor = 0;
           if (x % 2) {
              // test event accouting (only one event msg is sent in this test)
              subSession.Begin(true);
              numMsgToWaitFor = 1;
           }
           else {
              // test sequenced accouting (start/interim/stop)
              subSession.Begin(false);
              numMsgToWaitFor = msgCountPerSession;
           }

           /// wait till all records are processed
           do {
              std::cout << "Waiting till records are processed" << std::endl;
              ACE_OS::sleep(1);
           } while (subSession.NumberOfRecordsProcessed() < numMsgToWaitFor);

           std::cout << "ending the sub-session" << std::endl;
           subSession.End();

           if (! (x % 2)) {
               /// wait till the stop record is done 
               do {
                  std::cout << "Waiting till stop records is processed" << std::endl;
                  ACE_OS::sleep(1);
               } while (subSession.NumberOfRecordsProcessed() <= numMsgToWaitFor);
           }
       }
   }

   appCore.Close();
   task.Stop();
   return (0);
}



