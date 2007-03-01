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

// victor fajardo: sample client 

#include "diameter_api.h"

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

class AAA_SampleClientAcctSubSession : 
    public DiameterClientAcctSubSession<AAA_SampleClientAcctRecCollector> {
        // AAA client session derived from DiameterClientAcctSession.
        // It provides for all the functionality of a diameter 
        // client accounting session. The ClientAcctSubSession
        // class is a template function that requires a proper
        // version of a record collector as a paramter. Note 
        // that the application is responsible for instantiating 
        // this object
    public:
        AAA_SampleClientAcctSubSession(DiameterClientAcctSession &parent) :
            DiameterClientAcctSubSession<AAA_SampleClientAcctRecCollector>(parent),
            m_IsComplete(false) {
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
            m_IsComplete = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Failed(int recNum) {
            // notification that recNum record was not processed properly
            AAA_LOG((LM_INFO, "(%P|%t) **** record #%d not processed ****\n", recNum));
            m_IsComplete = true;
            return (AAA_ERR_SUCCESS);
        }
        bool IsComplete() {
            return m_IsComplete;
        }

    private:
        bool m_IsComplete;
};

class AAA_SampleAcctClient : public DiameterClientAcctSession
{
        // AAA client session derived from DiameterClientAcctSession.
        // It provides for all the functionality of a diameter 
        // client accounting session. The ClientAcctSubSession
        // class is a template function that requires a proper
        // version of a record collector as a paramter. Note 
        // that the application is responsible for instantiating 
        // this object
    public:
        AAA_SampleAcctClient(AAA_Task &task,
			     diameter_unsigned32_t id) :
            DiameterClientAcctSession(task, id),
            m_AcctSubSession(*this) { // sub-session uses parent
	}

        void SendEventRecord() {
            m_AcctSubSession.Begin(true);
	}

        bool IsComplete() {
            return m_AcctSubSession.IsComplete();
        }

    protected:
        // Accounting sub-session
        AAA_SampleClientAcctSubSession m_AcctSubSession;
};

class AAA_SampleAuthClient : public DiameterClientAuthSession
{
    public:
        AAA_SampleAuthClient(AAA_Task &task,
			     diameter_unsigned32_t authId,
			     diameter_unsigned32_t acctId,
                             int howManyMsg = 1) :
            DiameterClientAuthSession(task, authId),
            m_AcctSession(task, acctId),
            m_HowManyMsg(howManyMsg),
            m_Success(false),
            m_Disconnected(false) {
        }
        virtual void SetAuthSessionState
        (DiameterScholarAttribute<diameter_unsigned32_t> &authState)
        {
            // optional override, called by the library to set 
            // the auth state. Note that this overrides the 
            // settings in the configuration file or applications 
            // sending an auth session state AVP
            authState = DIAMETER_SESSION_STATE_MAINTAINED;
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
        virtual void SetSessionTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this client can send a hint to the server 
            // about the session timeout it prefers. If not
            // overridden, the value in the config file
            // is used
            timeout = 30;
        }
        virtual void SetAuthLifetimeTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this client can send a hint to the server 
            // about the auth lifetime it prefers. If not
            // overridden, the value in the config file
            // is used
            timeout = 29;
        }
        virtual AAAReturnCode ReAuthenticate(diameter_unsigned32_t type) {
            // optional override, called by the library so 
            // this client is informed about a re-auth request
            // initiated by the server. Note that the client
            // must return a valid result-code when exiting
            // this function
            AAA_LOG((LM_INFO, "(%P|%t) **** server re-authentication ****\n"));
            return (AAAReturnCode)(AAA_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // all request messages are handled by this function.
            // AAA clients normally will receive request message
            // in the open state

            // all request messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client is successfully responded
            //                      to server request
            // b. AAA_ERR_FAILURE - client failed. 
            AAA_LOG((LM_INFO, "(%P|%t) **** Request message message received in client ****\n"));
            DiameterMsgHeaderDump::Dump(msg);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {

            // all answer messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client is successfully authenticated
            // b. AAA_ERR_INCOMPLETE - auth not yet completed, muti-round 
            //                         message trip exchange
            // c. AAA_ERR_FAILURE - client authentication failed

            AAA_LOG((LM_INFO, "(%P|%t) Answer message received\n"));
            DiameterMsgHeaderDump::Dump(msg);

            DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
            DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
            DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
            DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);

            diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
            diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
            diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
            diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);

            if (host) {
                AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
            }
            if (realm) {
                AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
            }
            if (uname) {
                AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
            }
            if (authAppId) {
                AAA_LOG((LM_INFO, "(%P|%t) Auth Application Id: %d\n", *authAppId));
            }

            if ((-- m_HowManyMsg) > 0) {
		TxAuthenticationRequest();
                return (AAA_ERR_INCOMPLETE);
	    }
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            // all error messages are handled by this function.
            AAA_LOG((LM_INFO, "(%P|%t) **** Received message with error bit set ****\n"));
            return (AAA_ERR_SUCCESS);
	}
        virtual AAAReturnCode Success() {
            // notification of successful auth
            AAA_LOG((LM_INFO, "(%P|%t) **** user authorized ****\n"));

            // send an event record
            m_AcctSession.SendEventRecord();

            m_Success = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Disconnect() {
            // notification of completed STR/STA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session disconnecting ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** session timeout ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AuthorizationTimeout() {
            // notification of auth lifetime timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** auth timeout ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AbortSession() {
            // notification of completed ASR/ASA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session aborted by server ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        AAAReturnCode TxAuthenticationRequest() {
            std::cout << "Sending request message" << std::endl;

            // sample of how to compose a message using parser widgets

            DiameterMsgWidget msg(300, true, 10000);

            DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
            DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
            DiameterEnumAvpWidget reAuthAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);

            authIdAvp.Get() = 10000; // my application id
            unameAvp.Get() = "username@domain.com";
            reAuthAvp.Get() = 1;

            msg()->acl.add(authIdAvp());
            msg()->acl.add(unameAvp());
            msg()->acl.add(reAuthAvp());

            Send(msg());
            return (AAA_ERR_SUCCESS);
        }
        bool UserAuthorized() {
            return m_Success;
	}
        bool SessionDisconnected() {
            return (m_Disconnected && m_AcctSession.IsComplete());
	}

    protected:
        AAA_SampleAcctClient m_AcctSession;

    private:
        int m_HowManyMsg;
        bool m_Success;
        bool m_Disconnected;
};

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   int msgCountPerSession = 1;
   char *cfgFile = "config/nas1.local.xml";

   if (argc == 2) {
       cfgFile = argv[1];
   }
   else if (argc == 3) {
       cfgFile = argv[1];
       msgCountPerSession = (atoi(argv[2]) > 0) ? 
	      atoi(argv[2]) : msgCountPerSession;
   }
   else if (argc > 2) {
       std::cout << "Usage: aaa_sample_client2 [config file] [num message]\n" 
                 << std::endl;
       exit(0);
   }

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task);
   if (appCore.Open(cfgFile) == AAA_ERR_SUCCESS) {

       /// Wait for connectivity
       do {
           std::cout << "Waiting till this AAA has connectivity" << std::endl;
           ACE_OS::sleep(1);
       } while (appCore.NumActivePeers() == 0);

       /// send the client request
       AAA_SampleAuthClient client(task, 10000, 20000, msgCountPerSession);
       client.Begin("my_client");
       client.TxAuthenticationRequest();

       do {
           std::cout << "Waiting till user is authorized " << std::endl;
           ACE_OS::sleep(1);
       } while (! client.UserAuthorized());

       client.End();

       do {
           std::cout << "Waiting till user is disconnected " << std::endl;
           ACE_OS::sleep(1);
       } while (! client.SessionDisconnected());
   }

   appCore.Close();
   task.Stop();
   return (0);
}



