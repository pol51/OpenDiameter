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

#include "pana_paa_fsm.h"
#include "pana_auth_script.h"
#include "nasd_pana.h"
#include "nasd_eap_passthrough.h"
#include <fstream>
#include <iostream>
#include <iomanip>

class NASD_PaaSession:public NASD_CnAccessProtocol < NASD_EapPassThrough >,
    public PANA_PaaEventInterface {
	/// We assume that the next node is an EAP node
 public:
	NASD_PaaSession(PANA_PaaSessionChannel & ch):NASD_CnAccessProtocol <
	    NASD_EapPassThrough > (ch.Node().Task()), m_PaaSession(ch, *this),
	    m_Running(false) {

		std::string name("pana");
		m_CfgData = (NASD_ApPanaData *)
		    NASD_APPROTO_TBL().Lookup(name);
		if (m_CfgData == NULL) {
			NASD_LOG(LM_ERROR,
				 "(%P|%t) PANA configuration entry not found\n");
		} else {
			m_ScriptCtl.CurrentScript() =
			    m_CfgData->Protocol().EpScript();
		}
	}
	PANA_PaaSession & PaaSession() {
		return m_PaaSession;
	}

	/// Element events
 public:
	int Start() {
		m_PaaSession.Start();
		//NextNode()->Start();//pana node will notify the eap node by itself// deleted by BING LI
		m_Running = true;
		return (0);
	}
	bool IsRunning() {
		return m_Running;
	}
	void Stop() {
		m_PaaSession.Stop();
		NextNode()->Stop();
		m_Running = false;
	}

	/// Node events
 private:
	bool CurrentKey(std::string & key) {
		return false;	// don't generate keys
	}
	bool Identity(std::string & ident) {
		return false;	// don't support
	}
	int ReceiveIngress(AAAMessageBlock & msg) {
		return (0);	// access protocol, no prev node
	}
	int ReceiveEgress(AAAMessageBlock & msg) {
		m_PaaSession.EapSendRequest(&msg);
		return (0);
	}
	std::string GeneratePEMK() {
		std::string msk;

		if (IsKeyAvailable(msk)) {
			hexdump_key(LM_DEBUG, "Master Session key : ",
				    (const u8 *)msk.data(), msk.size());
		}

		ACE_UINT32 sessionId = m_PaaSession.SessionId();

		char strAddr[64];
		ACE_INET_Addr paaAddress;
		sprintf(strAddr, "%s:%d", PANA_CFG_PAA().m_PaaIpAddress.data(),
			PANA_CFG_GENERAL().m_ListenPort);
		paaAddress.string_to_addr(strAddr);
		std::string epid(((char *)paaAddress.get_addr()),
				 paaAddress.get_size());

		ACE_UINT32 keyId =
		    ((FAST_METHOD_TYPE << 8) + GTC_METHOD_TYPE) << 16;

		PANA_PAC_EP_Key pemk(msk, keyId, sessionId, epid);
		std::string pemkStr = pemk.Key();
		return pemkStr;
	}

	void GenerateIKEK(std::string pemk, std::string & key_in,
			  std::string & key_out) {

//The cofiguration information should be input here. The multicast group address that PAC wants to join is used as the mInfoOut. Here we suppose it as "239.205.9.125". THe multicast group address, e.g.the destination address of IGMP query message, is used as the mInfoIn. In IGMP v3, the value of mInforIn is "224.0.0.22" 
//Two key identifications are needed. Here they are supposed as "300" and "400" simply
		std::string mInfoOut = "239.205.9.125";
		std::string mInfoIn = "224.0.0.22";
		ACE_UINT32 keyIdOut = 300;
		ACE_UINT32 keyIdIn = 400;

		char strAddr[64];
		ACE_INET_Addr paaAddress;
		sprintf(strAddr, "%s:%d", PANA_CFG_PAA().m_PaaIpAddress.data(),
			PANA_CFG_GENERAL().m_ListenPort);
		paaAddress.string_to_addr(strAddr);
		std::string epid(((char *)paaAddress.get_addr()),
				 paaAddress.get_size());

		ACE_UINT32 sessionId = m_PaaSession.SessionId();
		printf("session id = %x\n", (sessionId));

		PANA_IKE_Key ikek(pemk, keyIdIn, keyIdOut, mInfoIn, mInfoOut,
				  sessionId, epid);
		key_in = ikek.Key_In();
		key_out = ikek.Key_Out();

		AAA_LOG((LM_DEBUG, "(%P|%t) : IKE key_in (length = %d): ",
			 ikek.Key_In().size()));
		for (unsigned int i = 0; i < ikek.Key_In().size(); i++)
			AAA_LOG((LM_DEBUG, "%02x",
				 (*(((char *)(ikek.Key_In().data())) + i)) &
				 0xff));
		AAA_LOG((LM_DEBUG, "\n"));
		AAA_LOG((LM_DEBUG, "(%P|%t) : IKE key_out (length = %d): ",
			 ikek.Key_Out().size()));
		for (unsigned int i = 0; i < ikek.Key_Out().size(); i++)
			AAA_LOG((LM_DEBUG, "%02x",
				 (*(((char *)(ikek.Key_Out().data())) + i)) &
				 0xff));
		AAA_LOG((LM_DEBUG, "\n"));
	}

	void ConfigureIKEK(std::string & key_in, std::string & key_out,
			   ACE_UINT32 keyIdIn, ACE_UINT32 keyIdOut) {
		std::ofstream file;
		const char filename[] = "/etc/racoon/setkey.paa.conf";
		file.open(filename);

		char strAddr[64];
		ACE_INET_Addr paaAddress;
		sprintf(strAddr, "%s:%d", PANA_CFG_PAA().m_PaaIpAddress.data(),
			PANA_CFG_GENERAL().m_ListenPort);
		paaAddress.string_to_addr(strAddr);

		const char *pacAddr = m_PaaSession.PacAddress().get_host_addr();
		const char *paaAddr = paaAddress.get_host_addr();
		file << "spdadd " << pacAddr <<
		    " 224.0.0.22 any -P in ipsec ah/transport//require;\n";
		file << "spdadd " << paaAddr <<
		    " 239.205.9.125 any -P out ipsec ah/transport//require;\n";
		file << std::setfill('0');
		file << std::hex;
		file << "add " << pacAddr << " 224.0.0.22 ah 0x" << keyIdIn <<
		    " -m transport -A hmac-sha1 0x";
		for (unsigned int i = 0; i < key_in.size(); i++)
			file << std::setw(2) <<
			    ((*(((unsigned char *)(key_in.data())) + i)) &
			     0xff);
		file << ";\n";
		file << "add " << paaAddr << " 239.205.9.125 ah 0x" << keyIdOut
		    << " -m transport -A hmac-sha1 0x";
		for (unsigned int i = 0; i < key_out.size(); i++)
			file << std::setw(2) <<
			    ((*(((unsigned char *)(key_out.data())) + i)) &
			     0xff);
		file << ";\n";
		file.close();
		std::cout << "will set key\n";
		system("setkey -f /etc/racoon/setkey.paa.conf");
	}
	void Success(AAAMessageBlock * msg = 0) {
		m_PaaSession.EapSuccess(msg);
		std::string pemk = GeneratePEMK();
		/* std::string keyIn;std::string keyOut;
		   ACE_UINT32 keyIdOut = 300;
		   ACE_UINT32 keyIdIn =400; 
		   GenerateIKEK(pemk,keyIn,keyOut);  
		   ConfigureIKEK(keyIn,keyOut,keyIdIn,keyIdOut); */
	}
	void Failure(AAAMessageBlock * msg = 0) {
		m_PaaSession.EapFailure(msg);
	}
	void Timeout() {
		m_PaaSession.EapTimeout();
	}
	void Error() {
		m_PaaSession.EapInvalidMessage();
	}

	/// Paa events
 private:
	void EapStart() {
		NextNode()->Stop();
		NextNode()->Start();
	}
	void Authorize(PANA_AuthorizationArgs & args) {
		if (m_CfgData && m_ScriptCtl.CurrentScript().length() > 0) {
			m_ScriptCtl.Seed(args);
			m_ScriptCtl.Add();
		}
	}
	bool IsKeyAvailable(diameter_octetstring_t & key) {
		return NextNode()->CurrentKey(key);
	}
	void Disconnect(ACE_UINT32 cause = 0) {
		if (m_CfgData && m_ScriptCtl.CurrentScript().length() > 0) {
			m_ScriptCtl.Remove();
		}
		Stop();
		NASD_GARBAGE_COLLECTOR().ScheduleForDeletion(*this);
	}
	bool IsUserAuthorized() {
		return true;
	}
	void EapResponse(AAAMessageBlock * request) {
		SendIngress(*request);
	}
	void EapAltReject() {
	}

 private:
	PANA_PaaSession m_PaaSession;
	bool m_Running;
	NASD_ApPanaData *m_CfgData;
	PANA_AuthScriptCtl m_ScriptCtl;
};

PANA_PaaSession *NASD_PanaSessionFactory::Create()
{
	NASD_PaaSession *s = new NASD_PaaSession(*this);
	s->Start();
	return (s) ? &(s->PaaSession()) : NULL;
}

bool NASD_PanaInitializer::Initialize(AAA_Task & t)
{
	if (!m_Node.get()) {

		NASD_LOG(LM_INFO,
			 "(%P|%t) Initializing PANA access protocol\n");

		std::string name("pana");
		NASD_ApPanaData *pana = (NASD_ApPanaData *)
		    NASD_APPROTO_TBL().Lookup(name);

		if (pana == NULL) {
			NASD_LOG(LM_ERROR,
				 "(%P|%t) PANA configuration entry not found\n");
			return false;
		}

		if (!pana->Enabled()) {	//disabled
			NASD_LOG(LM_ERROR, "(%P|%t) PANA is disabled.\n");
			return false;
		}

		m_Node =
		    std::auto_ptr < PANA_Node >
		    (new PANA_Node(t, pana->Protocol().CfgFile()));
		if (m_Node.get()) {
			m_Factory = std::auto_ptr < NASD_PanaSessionFactory >
			    (new NASD_PanaSessionFactory(*m_Node));
		} else {
			NASD_LOG(LM_ERROR,
				 "(%P|%t) Allocation failure in PANA node !!!\n");
			return false;
		}
	}
	return true;
}

bool NASD_PanaInitializer::UnInitialize()
{
	m_Factory.reset();
	m_Node.reset();
	return true;
}
