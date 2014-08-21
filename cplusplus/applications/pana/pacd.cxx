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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <ace/Get_Opt.h>
#include <ace/Signal.h>
#include "eap_peerfsm.hxx"
#include "eap_identity.hxx"
#include "eap_md5.hxx"
#include "eap_archie_fsm.hxx"
#include "eap_fast_fsm.hxx"
#include "eap_fast_session.hxx"
#include "eap_fast_mng.hxx"
#include "eap_fast_xml_data.hxx"
#include "eap_fast_xml_parser.hxx"
#include "pana_client_fsm.h"
#include "pacd_config.h"
#include "pana_auth_script.h"
#include "pana_pac_ep_key.h"
#include <iomanip>

#define KEY_TESTING 1
#define USAGE "Usage: pacd -f [configuration file]"
#define PACD_DEFAULT_CFG_FILE "/etc/opendiameter/pac/config/pana_setup.xml"
#define CONFIG_FILE "/etc/opendiameter/pac/config/client.eap-fast.xml"
#define GTC_METHOD_TYPE 6

static std::string g_SharedSecret;
static ACE_Thread_Mutex gMutex;
static ACE_Condition < ACE_Thread_Mutex > gActiveSignal(gMutex);

typedef AAA_JobHandle < AAA_GroupedJob > AppJobHandle;

class PeerData;
class AppPeerSwitchStateMachine;

class EapTask:public AAA_Task {
 public:
	EapTask(std::string & cfgfile):
	    AAA_Task(AAA_SCHED_WFQ, (char *)"EAP"), m_Node(*this, cfgfile) {
	} virtual ~ EapTask() {
	}

	PANA_Node m_Node;
};

// Definition for peer MD5-Challenge method.
class AppEapPeerMD5ChallengeStateMachine:public EapPeerMD5ChallengeStateMachine {
	friend class EapMethodStateMachineCreator
	    < AppEapPeerMD5ChallengeStateMachine >;
 public:
	AppEapPeerMD5ChallengeStateMachine(EapSwitchStateMachine & s)
	:EapPeerMD5ChallengeStateMachine(s) {
	} void InputPassphrase() {
		std::cout << "Setting password" << std::endl;
		Passphrase() = PACD_CONFIG().m_Password;
	}

 private:
	~AppEapPeerMD5ChallengeStateMachine() {
	}
};

// Definition for peer archie state machine.
class AppEapPeerArchieStateMachine:public EapPeerArchieStateMachine {
	friend class EapMethodStateMachineCreator
	    < AppEapPeerArchieStateMachine >;
 public:
	AppEapPeerArchieStateMachine(EapSwitchStateMachine & s)
	:EapPeerArchieStateMachine(s) {
	} std::string & InputSharedSecret() {
		// Invoked on shared secret callback
		return g_SharedSecret;
	}

	std::string & InputIdentity() {
		// Invoked on Auth ID callback
		std::cout << "Received an Archie-Request from "
		    << AuthID() << std::endl;
		return SwitchStateMachine().PeerIdentity();
	}

 private:
	~AppEapPeerArchieStateMachine() {
	}
};

class AppEapPeerFastStateMachine:public EapPeerFastStateMachine {
	friend class EapMethodStateMachineCreator <
	    AppEapPeerFastStateMachine >;
 public:
	AppEapPeerFastStateMachine(EapSwitchStateMachine & s)
	:EapPeerFastStateMachine(s) {
	}
	/* This pure virtual function is a callback used when a shared-secret
	 * needs to be obtained.*///~  std::string& InputSharedSecret()  {//~ return::sharedSecret;//~ } ///// This pure virtual function is a callback used when an AuthID/// needs to be obtained.
	    std::string & InputIdentity() {
		std::cout << "Received an Fast-Request " << std::endl;

		static std::string identity;
		std::cout << "Input username (within 10sec.): " << std::endl;
		//std::cin >> identity;
		identity = std::string("testuser@localdomain.net");
		std::cout << "username = " << identity << std::endl;
		return identity;
	}

 protected:
	std::string & InputConfigFile() {
		static std::string configFile((char *)CONFIG_FILE);
		return configFile;
	}

 private:
	~AppEapPeerFastStateMachine() {
	}
};

// EAP peer state machine
class AppPeerSwitchStateMachine:public EapPeerSwitchStateMachine {
 public:
	AppPeerSwitchStateMachine(ACE_Reactor & r, AppJobHandle & h, int type)
	:EapPeerSwitchStateMachine(r, h) {
	} void Send(AAAMessageBlock * b);

	void Success();

	void Failure();

	void Notification(std::string & str);

	void Abort();

	std::string & InputIdentity();
};

// EAP channel definition, bounded to PANA
class PeerChannel:public PANA_ClientEventInterface, public ACE_Event_Handler {
 public:
	PeerChannel(PANA_Node & n,
		    AppPeerSwitchStateMachine & s):
	    m_Eap(s),
	    m_PaC(n, *this), m_AuthScriptCtl(PACD_CONFIG().m_AuthScript) {
		m_PaC.Start();
	} virtual ~ PeerChannel() {
	}
	void SendEapResponse(AAAMessageBlock * msg) {
		m_PaC.EapSendResponse(msg);
	}
	void EapStart() {
		m_Eap.Stop();
		m_Eap.Start();
	}
	void EapRequest(AAAMessageBlock * request) {
		m_Eap.Receive(request);
	}
	std::string GeneratePEMK() {
		std::string msk;
		if (IsKeyAvailable(msk)) {
			hexdump_key(LM_DEBUG, "Master Session key : ",
				    (const u8 *)msk.data(), msk.size());
		}

		ACE_UINT32 sessionId = m_PaC.GetSessionId();

		ACE_INET_Addr & paaAddress = m_PaC.GetPaaAddress();
		std::string epid(((char *)paaAddress.get_addr()),
				 paaAddress.get_size());

		ACE_UINT32 keyId =
		    ((FAST_METHOD_TYPE << 8) + GTC_METHOD_TYPE) << 16;

		PANA_PAC_EP_Key pemk(msk, keyId, sessionId, epid);
		std::string PEMK = pemk.Key();
		return PEMK;
	}
	void GenerateIKEK(std::string pemk, std::string & key_in,
			  std::string & key_out, ACE_UINT32 keyIdIn,
			  ACE_UINT32 keyIdOut) {
		//The configuration information should be input here. The multicast group address that PAC wants to join is used as the mInfoIn. Here we suppose it as "239.205.9.125". THe multicast group address, e.g.the destination address of IGMP query message, is used as the mInfoOut. In IGMP v3, the value of mInforIn is "224.0.0.22"  
		std::string mInfoIn = "239.205.9.125";
		std::string mInfoOut = "224.0.0.22";

		ACE_UINT32 sessionId = m_PaC.GetSessionId();

		ACE_INET_Addr & paaAddress = m_PaC.GetPaaAddress();
		std::string epid(((char *)paaAddress.get_addr()),
				 paaAddress.get_size());

		PANA_IKE_Key ikek(pemk, keyIdIn, keyIdOut, mInfoIn, mInfoOut,
				  sessionId, epid);

		key_in = ikek.Key_In();
		key_out = ikek.Key_Out();

		AAA_LOG((LM_DEBUG, "(%P|%t) : IKE key_in : "));
		for (unsigned int i = 0; i < ikek.Key_In().size(); i++)
			AAA_LOG((LM_DEBUG, "%02x",
				 (*(((char *)(ikek.Key_In().data())) + i)) &
				 0xff));
		AAA_LOG((LM_DEBUG, "\n"));
		AAA_LOG((LM_DEBUG, "(%P|%t) : IKE key_out : "));
		for (unsigned int i = 0; i < ikek.Key_Out().size(); i++)
			AAA_LOG((LM_DEBUG, "%02x",
				 (*(((char *)(ikek.Key_Out().data())) + i)) &
				 0xff));
		AAA_LOG((LM_DEBUG, "\n"));
	}
	void ConfigureIKEK(std::string & key_in, std::string & key_out,
			   ACE_UINT32 keyIdIn, ACE_UINT32 keyIdOut) {
		std::ofstream file;
		const char filename[] = "/etc/racoon/setkey.pac.conf";
		file.open(filename);

		std::cout << "key_in (len= " << key_in.size() << ") : ";
		for (unsigned int i = 0; i < key_in.size(); i++)
			printf("%2x",
			       ((*(((char *)key_in.data()) + i)) & 0xff));
		const char *pacAddr = "127.0.0.1";
		const char *paaAddr = m_PaC.GetPaaAddress().get_host_addr();
		file << "spdadd " << pacAddr <<
		    " 224.0.0.22 any -P out ipsec ah/transport//require;\n";
		file << "spdadd " << paaAddr <<
		    " 239.205.9.125 any -P in ipsec ah/transport//require;\n";
		file << std::setfill('0');
		file << std::hex;
		file << "add " << pacAddr << " 224.0.0.22 ah 0x" << keyIdOut <<
		    " -m transport -A hmac-sha1 0x";
		for (unsigned int i = 0; i < key_out.size(); i++)
			file << std::setw(2) <<
			    ((*(((unsigned char *)(key_out.data())) + i)) &
			     0xff);
		file << ";\n";
		file << "add " << paaAddr << " 239.205.9.125 ah 0x" << keyIdIn
		    << " -m transport -A hmac-sha1 0x";
		for (unsigned int i = 0; i < key_in.size(); i++)
			file << std::setw(2) <<
			    ((*(((unsigned char *)(key_in.data())) + i)) &
			     0xff);
		file << ";\n";
		file.close();
		std::cout << "will set key\n";
		system("setkey -f /etc/racoon/setkey.paa.conf");
	}
	void Success() {
		m_PaC.EapSuccess();
	}
	void Failure() {
		m_PaC.EapFailure();
	}
	void EapAltReject() {
	}
	void Authorize(PANA_AuthorizationArgs & args) {
		std::string pemk = GeneratePEMK();
		/* std::string keyIn;std::string keyOut;
		   //Two key identifications are needed. Here they are supposed as "400" and "300" simply
		   ACE_UINT32 keyIdOut = 400;
		   ACE_UINT32 keyIdIn =300; 
		   GenerateIKEK(pemk,keyIn,keyOut,keyIdIn,keyIdOut);

		   ConfigureIKEK(keyIn,keyOut,keyIdIn,keyIdOut); */
		// Seed the auth-script
		m_AuthScriptCtl.Seed(args);
		m_AuthScriptCtl.Add();
	}
	virtual bool IsKeyAvailable(pana_octetstring_t & key) {
#ifdef KEY_TESTING
		static int toggle = 0;
		static char *keys[] = { (char *)
			    "0123456789012345678901234567890123456789012345678901234567890123",
			(char *)
			"3210987654321098765432109876543210987654321098765432109876543210"
		};
		key.assign(keys[toggle]);
		toggle = (toggle) ? 0 : 1;
		return true;
#else
		if (m_Eap.KeyAvailable()) {
			key.assign(m_Eap.KeyData().data(),
				   m_Eap.KeyData().size());
			return true;
		}
		return false;
#endif
	}
	void Disconnect(ACE_UINT32 cause) {
		std::cout << "PANA Disconnection" << std::endl;
		m_AuthScriptCtl.Remove();
		gActiveSignal.signal();
	}
	void Ping() {
		m_PaC.Ping();
	}
	void ReAuthenticate() {
		m_PaC.ReAuthenticate();
	}
	void Stop() {
		m_PaC.Stop();
	}
	void Abort() {
		m_PaC.Abort();
	}
 private:
	int handle_timeout(const ACE_Time_Value & tv, const void *arg) {
		std::cout << "Authorization period expired !!!" << std::endl;
		Stop();
		return (0);
	}

	AppPeerSwitchStateMachine & m_Eap;
	PANA_PacSession m_PaC;
	PANA_AuthScriptCtl m_AuthScriptCtl;
};

// Peer Application
class PeerApplication:public AAA_JobData {
 public:
	PeerApplication(EapTask & task,
			int type, int innerType):m_Handle(AppJobHandle
							  (AAA_GroupedJob::Create
							   (task.Job(), this,
							    (char *)"peer"))),
	    m_Eap(boost::shared_ptr < AppPeerSwitchStateMachine >
		  (new
		   AppPeerSwitchStateMachine(*task.reactor(), m_Handle, type))),
	    m_Channel(task.m_Node, *m_Eap),
	    m_Md5Method(EapContinuedPolicyElement(EapType(4))),
	    m_ArchieMethod(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE))),
	    m_FastMethod(EapContinuedPolicyElement(EapType(FAST_METHOD_TYPE))) {
		if (type == FAST_METHOD_TYPE) {
			m_Eap->Policy().InitialPolicyElement(&m_FastMethod);
			m_Eap->Policy().InnerEapMethodType(EapType(innerType));
		} else {
			if (type == ARCHIE_METHOD_TYPE) {
				m_Eap->Policy().InitialPolicyElement
				    (&m_ArchieMethod);
			} else {
				m_Eap->Policy().
				    InitialPolicyElement(&m_Md5Method);
			}
		}
	}
	PeerChannel & Channel() {
		return m_Channel;
	}
	AppPeerSwitchStateMachine & Eap() {
		return *m_Eap;
	}

 private:
	AppJobHandle m_Handle;
	boost::shared_ptr < AppPeerSwitchStateMachine > m_Eap;
	PeerChannel m_Channel;
	EapContinuedPolicyElement m_Md5Method;
	EapContinuedPolicyElement m_ArchieMethod;
	EapContinuedPolicyElement m_FastMethod;
};

void AppPeerSwitchStateMachine::Send(AAAMessageBlock * b)
{
	JobData(Type2Type < PeerApplication > ()).Channel().SendEapResponse(b);
}

void AppPeerSwitchStateMachine::Success()
{
	//EAPFAST_session_t_peer* session_peer = ((AppEapPeerFastStateMachine &)MethodStateMachine()).get_fast_session();
	/*size_t len;
	   this->keyAvailable = true;
	   u8* MSK = eap_fast_get_msk(session_peer->get_simck(),&len);
	   this->keyData.clear();
	   this->keyData.assign((char *)MSK,len); */

	std::cout << "Authentication success at peer" << std::endl;
	std::cout << "Welcome to the world, " << PeerIdentity()
	    << " !!!" << std::endl;
	JobData(Type2Type < PeerApplication > ()).Channel().Success();
}

void AppPeerSwitchStateMachine::Failure()
{
	std::cout << "Authentication failure detected at peer" << std::endl;
	std::cout << "Sorry, " << PeerIdentity()
	    << " try next time !!!" << std::endl;
	JobData(Type2Type < PeerApplication > ()).Channel().Failure();
	Stop();
}

void AppPeerSwitchStateMachine::Notification(std::string & str)
{
	std::cout << "Following notification received" << std::endl;
	std::cout << str << std::endl;
}

void AppPeerSwitchStateMachine::Abort()
{
	std::cout << "Peer aborted for an error in state machine" << std::endl;
	JobData(Type2Type < PeerApplication > ()).Channel().Abort();
}

std::string & AppPeerSwitchStateMachine::InputIdentity()
{
	std::cout << "Setting username: "
	    << PACD_CONFIG().m_Username << std::endl;
	return PACD_CONFIG().m_Username;
}

class PeerInitializer {
 public:
	PeerInitializer(EapTask & t):m_Task(t) {
		Start();
	}
	~PeerInitializer() {
		Stop();
	}

 private:
	void Start() {
		m_MethodRegistrar.registerMethod
		    (std::string("MD5-Challenge"), EapType(4),
		     Peer, m_AppPeerMD5ChallengeCreator);

		m_MethodRegistrar.registerMethod
		    (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE),
		     Peer, m_AppPeerArchieCreator);

		m_MethodRegistrar.registerMethod
		    (std::string("Fast"), EapType(FAST_METHOD_TYPE),
		     Peer, m_AppPeerFastCreator);

		try {
			m_Task.Start(PACD_CONFIG().m_ThreadCount);
		}
		catch( ...) {
			std::cout << "Task failed to start !!\n" << std::endl;
			exit(1);
		}
	}
	void Stop() {
		m_Task.Stop();
	}

	EapTask & m_Task;
	EapMethodRegistrar m_MethodRegistrar;
	EapMethodStateMachineCreator
	    < AppEapPeerMD5ChallengeStateMachine > m_AppPeerMD5ChallengeCreator;
	EapMethodStateMachineCreator
	    < AppEapPeerArchieStateMachine > m_AppPeerArchieCreator;
	EapMethodStateMachineCreator
	    < AppEapPeerFastStateMachine > m_AppPeerFastCreator;
};

static PeerApplication *gPacReference = NULL;

#if defined (ACE_HAS_SIG_C_FUNC)
extern "C" {
#endif
	static void PacdSigHandler(int signo) {
		if (gPacReference) {
			switch (signo) {
			case SIGUSR1:
				gPacReference->Channel().Ping();
				break;
				case SIGHUP:gPacReference->
				    Channel().ReAuthenticate();
				break;
				case SIGTERM:gPacReference->Channel().Stop();
				break;
				default:break;
			}
		} else {
			std::cout <<
			    "Signal has been received but reference is not yet ready"
			    << std::endl;
		}
	}
#if defined (ACE_HAS_SIG_C_FUNC)
}
#endif

int main(int argc, char *argv[])
{
	std::string panaCfgfile = PACD_DEFAULT_CFG_FILE;

	// verify command line options
	ACE_Get_Opt opt(argc, argv, "f:", 1);
	for (int c; (c = opt()) != (-1);) {
		switch (c) {
		case 'f':
			panaCfgfile.assign(opt.optarg);
			break;
		}
	}

	// Load configuration file
	PACD_CONFIG_OPEN(panaCfgfile);

	// Get shared secret.
	fstream secret(PACD_CONFIG().m_Secret.data(), ios::in | ios::binary);
	if (secret) {
		unsigned char buffer[64];
		if (!secret.eof()) {
			memset(buffer, 0, sizeof(buffer));
			secret.read((char *)buffer, sizeof(buffer));
			g_SharedSecret.assign((char *)buffer, sizeof(buffer));
		}
		secret.close();
	} else {
		std::cout << "Cannot open file: " << PACD_CONFIG().m_Secret
		    << std::endl;
		return (-1);
	}

	ACE_Sig_Action sa(reinterpret_cast < ACE_SignalHandler >
			  (PacdSigHandler));
	sa.register_action(SIGUSR1);
	sa.register_action(SIGHUP);
	sa.register_action(SIGTERM);

	EapTask task(PACD_CONFIG().m_PaCCfgFile);
	try {
		PeerInitializer init(task);
		PeerApplication peer(task,
				     PACD_CONFIG().m_EapMethod,
				     PACD_CONFIG().m_InnerEapMethod);
		sleep(1000);
		gPacReference = &peer;
		gActiveSignal.wait();
		// Insurance policy to make sure all threads are gone
		// Stop() already waits for the threads but ExistBacklog()
		// can have loopholes in it.
		ACE_Time_Value tm(3);
		ACE_OS::sleep(tm);
	}
	catch( ...) {
	}
	task.Stop();
	std::cout << "PaC Existing." << std::endl;
	return 0;
}
