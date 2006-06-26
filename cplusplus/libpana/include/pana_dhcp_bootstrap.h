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

#ifndef __PANA_DHCP_BOOTSTRAP_H__
#define __PANA_DHCP_BOOTSTRAP_H__

#include "pana_exports.h"
#include "pana_message.h"
#include "pana_nonce_generator.h"

/*
     Bootstrapping RFC3118 Delayed DHCP Authentication
       Using EAP-based Network Access Authentication
          <draft-yegin-eap-boot-rfc3118-00.txt>
*/

class PANA_EXPORT PANA_DhcpKey
{
    public:
        PANA_DhcpKey(diameter_octetstring_t &aaaKey,
                     ACE_UINT32 secretId,
                     diameter_octetstring_t &nonceClient,   
                     diameter_octetstring_t &nonceNas) {
           Generate(aaaKey, secretId, nonceClient, nonceNas); 
        }

        diameter_octetstring_t &GetKey() { 
           return m_Value; 
        }

    private:       
        diameter_octetstring_t m_Value;

        void Generate(diameter_octetstring_t &aaaKey,
                      ACE_UINT32 secredId,
                      diameter_octetstring_t &nonceClient,   
                      diameter_octetstring_t &nonceNas);
};

class PANA_EXPORT PANA_DhcpSecretIdPool
{
    /*
      A 32-bit value that identifies the DHCP Key produced as a
      result of the bootstrapping process. This value is determined
      by the PAA and sent to the PaC. The PAA determines this value
      by randomly picking a number from the available secret ID pool.
      If PaC's response does not contain DHCP-AVP then this value is
      returned to the available identifiers pool. Otherwise, it is
      allocated to the PaC until the DHCP SA expires. The PaC MUST
      set this field to all 0s in its response.
     */
    public:
        ACE_UINT32 Allocate() {
            ACE_UINT32 ltime;
            ACE_System_Time::get_local_system_time(ltime);
            do { 
               ltime += ACE_UINT32(ACE_OS::rand());
               if (! IsAvailable(ltime)) {
                   m_UsageMap.insert(std::pair<ACE_UINT32, ACE_UINT32>
                       (ltime, ltime));
                   return ltime; 
	       }
	    } while (true);
            return 0;
        }
        bool IsAvailable(ACE_UINT32 id) {
            return (m_UsageMap.find(id) != m_UsageMap.end());
	}
        void Release(ACE_UINT32 id) {
            std::map<ACE_UINT32, ACE_UINT32>::iterator i;
            i = m_UsageMap.find(id);
            if (i != m_UsageMap.end()) {
                m_UsageMap.erase(i);               
	    }
	}

    private:
	std::map<ACE_UINT32, ACE_UINT32> m_UsageMap;
};

class PANA_EXPORT PANA_DhcpSecurityAssociation
{
    public:
        PANA_DhcpSecurityAssociation() : 
           m_Enabled(false),
           m_SecretId(0) {
           PANA_NonceGenerator::Get(m_LocalNonce); 
        }
        bool &Enable() {
           return m_Enabled;
        }
        ACE_UINT32 &SecretId() {
           return m_SecretId;
        }
        diameter_octetstring_t &PeerNonce() {
           return m_PeerNonce;
        }
        diameter_octetstring_t &LocalNonce() {
           return m_LocalNonce;
        }

    protected:
        bool m_Enabled;
        ACE_UINT32 m_SecretId;

        /*
          Contains the random data generated by the transmitting entity.
          This field contains the Nonce_client value when the AVP is sent
          by PaC, and the Nonce_NAS value when the AVP is sent by PAA.
          Nonce value MUST be randomly chosen and MUST be at least 128
          bits in size. Nonce values MUST NOT be reused.
         */
        diameter_octetstring_t m_PeerNonce;
        diameter_octetstring_t m_LocalNonce;
};

class PANA_EXPORT PANA_PacDhcpSecurityAssociation :
    public PANA_DhcpSecurityAssociation
{
    public:
        bool CheckPBR(PANA_Message &pbr);
        void AffixToPBA(PANA_Message &pba);

        void DhcpKey(diameter_octetstring_t &aaaKey,
                     diameter_octetstring_t &dhcpKey) {
           PANA_DhcpKey key(aaaKey, 
                            m_SecretId, 
                            m_LocalNonce,
                            m_PeerNonce);
           dhcpKey = key.GetKey();
        }
};

class PANA_EXPORT PANA_PaaDhcpSecurityAssociation :
    public PANA_DhcpSecurityAssociation
{
    public:
        void AffixToPBR(PANA_Message &pba);
        bool CheckPBA(PANA_Message &pbr);
   
        void DhcpKey(diameter_octetstring_t &aaaKey,
                     diameter_octetstring_t &dhcpKey) {
           PANA_DhcpKey key(aaaKey, 
                            m_SecretId, 
                            m_PeerNonce,
                            m_LocalNonce);
           dhcpKey = key.GetKey();
       }

    private:
        PANA_DhcpSecretIdPool m_SecretIdPool;
};

#endif /* __PANA_DHCP_BOOTSTRAP_H__ */

