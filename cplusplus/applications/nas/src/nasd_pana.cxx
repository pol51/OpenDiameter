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

class NASD_PaaSession : 
    public NASD_CnAccessProtocol<NASD_EapPassThrough>,
    public PANA_PaaEventInterface
{
    /// We assume that the next node is an EAP node
    public:
	NASD_PaaSession(PANA_PaaSessionChannel &ch) :
            NASD_CnAccessProtocol<NASD_EapPassThrough>
                  (ch.Node().Task()),
            m_PaaSession(ch, *this),
            m_Running(false) {

            std::string name("pana");
            m_CfgData = (NASD_ApPanaData*)
                  NASD_APPROTO_TBL().Lookup(name);
            if (m_CfgData == NULL) {
                NASD_LOG(LM_ERROR, 
                  "(%P|%t) PANA configuration entry not found\n");
	    }
            else {
                m_ScriptCtl.CurrentScript() =
                  m_CfgData->Protocol().EpScript();
	    }
        }
        PANA_PaaSession &PaaSession() {
            return m_PaaSession;
	}
        
    /// Element events
    public:
        int Start() {
            m_PaaSession.Start();
            NextNode()->Start();
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
        bool CurrentKey(std::string &key) {
	    return false; // don't generate keys
        }
        bool Identity(std::string &ident) {
	    return false; // don't support
        }
	int ReceiveIngress(AAAMessageBlock &msg) {
            return (0); // access protocol, no prev node
	}
	int ReceiveEgress(AAAMessageBlock &msg) {
            m_PaaSession.EapSendRequest(&msg);
            return (0);
	}
	void Success(AAAMessageBlock *msg = 0) {
            m_PaaSession.EapSuccess(msg);
	}
	void Failure(AAAMessageBlock *msg = 0) {
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
#if defined(PANA_MPA_SUPPORT)
        bool IsPacIpAddressAvailable(PANA_DeviceId &ip,
                                     PANA_DeviceId &local,
                                     ACE_INET_Addr &remote) {
           ACE_INET_Addr pacIp("192.168.1.100:0");
           PANA_DeviceIdConverter::PopulateFromAddr(pacIp, ip);
           return false;
        }
#endif
	void Authorize(PANA_AuthorizationArgs &args) {
	    if (m_CfgData &&
                m_ScriptCtl.CurrentScript().length() > 0) {
                m_ScriptCtl.Seed(args);
                m_ScriptCtl.Add();
	    }
	}
	bool IsKeyAvailable(diameter_octetstring_t &key) {
	    return NextNode()->CurrentKey(key);
	}
	void Disconnect(ACE_UINT32 cause = 0) {
	    if (m_CfgData &&
                m_ScriptCtl.CurrentScript().length() > 0) {
                m_ScriptCtl.Remove();
	    }
            Stop();
            NASD_GARBAGE_COLLECTOR().
                    ScheduleForDeletion(*this);
	}
	void Error(ACE_UINT32 resultCode) {
	    if (m_CfgData &&
                m_ScriptCtl.CurrentScript().length() > 0) {
                m_ScriptCtl.Remove();
	    }
            Stop();
            NASD_GARBAGE_COLLECTOR().
                    ScheduleForDeletion(*this);
	}
	bool IsUserAuthorized() {
            return true;
	}
	void EapResponse(AAAMessageBlock *request) {
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

bool NASD_PanaInitializer::Initialize(AAA_Task &t)
{
    if (! m_Node.get()) {

        NASD_LOG(LM_INFO, "(%P|%t) Initializing PANA access protocol\n");

        std::string name("pana");
        NASD_ApPanaData *pana = (NASD_ApPanaData*)
            NASD_APPROTO_TBL().Lookup(name);
        if (pana == NULL) {
            NASD_LOG(LM_ERROR, "(%P|%t) PANA configuration entry not found\n");
            return false;
	}

        m_Node = std::auto_ptr<PANA_Node>(new PANA_Node(t, pana->Protocol().CfgFile()));
        if (m_Node.get()) {
            m_Factory = std::auto_ptr<NASD_PanaSessionFactory>
		    (new NASD_PanaSessionFactory(*m_Node));
	}
        else {
            NASD_LOG(LM_ERROR, "(%P|%t) Allocation failure in PANA node !!!\n");
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



