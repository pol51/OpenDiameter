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

#include "od_utl_md5.h"
#include "pana_dhcp_bootstrap.h"

#define PANA_DHCP_DEBUG   0

void PANA_DhcpKey::Generate(diameter_octetstring_t &aaaKey,
                            ACE_UINT32 secretId,
                            diameter_octetstring_t &nonceClient,   
                            diameter_octetstring_t &nonceNas)
{
#if PANA_DHCP_DEBUG
    printf("AAA key: %d\n", aaaKey.size());
    for (int i=0; i<aaaKey.size(); i++) {
        printf("%02X ", ((unsigned char*)aaaKey.data())[i]);
    }
    printf("\n");
    printf("Secret Id: 0x%x\n", secretId);
    printf("nonce Client: %d\n", nonceClient.size());
    for (int i=0; i<nonceClient.size(); i++) {
        printf("%02X ", ((unsigned char*)nonceClient.data())[i]);
    }
    printf("\n");
    printf("nonce NAS: %d\n", nonceNas.size());
    for (int i=0; i<nonceNas.size(); i++) {
        printf("%02X ", ((unsigned char*)nonceNas.data())[i]);
    }
    printf("\n");
#endif

    /*
      The key derivation procedure is reused from IKE [RFC2409]. 
      The character '|' denotes concatenation.


        DHCP Key = HMAC-MD5(AAA-key, const | Secret ID | 
                            Nonce_client | Nonce_NAS)
    */

    AAA_UINT8 digest[16];
    OD_Utl_Md5 md5;
    md5.Update((AAA_UINT8*)aaaKey.data(), aaaKey.size());
    md5.Update((AAA_UINT8*)&secretId, sizeof(ACE_UINT32));
    md5.Update((AAA_UINT8*)nonceClient.data(), nonceClient.size());
    md5.Update((AAA_UINT8*)nonceNas.data(), nonceNas.size());
    md5.Final(digest);

    m_Value.assign((char*)digest, sizeof(digest));

#if PANA_DHCP_DEBUG
    printf("DCHP key: %d\n", m_Value.size());
    for (int i=0; i<m_Value.size(); i++) {
        printf("%02X ", ((unsigned char*)m_Value.data())[i]);
    }
    printf("\n");
#endif
}

bool PANA_PacDhcpSecurityAssociation::CheckPBR
          (PANA_Message &pbr)
{
    /*
      Absence of this AVP in the PANA-Bind-Request message sent by the PAA
      indicates unavailability of this additional service. In that case,
      PaC MUST NOT include DHCP-AVP in its response, and PAA MUST ignore
      received DHCP-AVP. When this AVP is received by the PaC, it may or
      may not include the AVP in its response depending on its desire to
      create a DHCP SA. A DHCP SA can be created as soon as each entity
      has received and sent one DHCP-AVP.
     */
    PANA_DhcpAvpContainerWidget dhcp_c(pbr.avpList());
    PANA_DhcpData_t *dhcp_d = dhcp_c.GetAvp(PANA_AVPNAME_DHCP);
    if (dhcp_d && Enable()) {
        SecretId() = dhcp_d->id;
        PeerNonce() = dhcp_d->nonce;
        return true;
    }
    else {
        Enable() = false;
    }
    return Enable();
} 

void PANA_PacDhcpSecurityAssociation::AffixToPBA
          (PANA_Message &pba)
{
    /*
      A new PANA AVP is defined in order to bootstrap DHCP SA. The DHCP-
      AVP is included in the PANA-Bind-Request message if PAA (NAS) is
      offering DHCP SA bootstrapping service. If the PaC wants to proceed
      with creating DHCP SA at the end of the PANA authentication, it MUST
      include DHCP-AVP in its PANA-Bind-Answer message.

      Absence of this AVP in the PANA-Bind-Request message sent by the PAA
      indicates unavailability of this additional service. In that case,
      PaC MUST NOT include DHCP-AVP in its response, and PAA MUST ignore
      received DHCP-AVP. When this AVP is received by the PaC, it may or
      may not include the AVP in its response depending on its desire to
      create a DHCP SA. A DHCP SA can be created as soon as each entity
      has received and sent one DHCP-AVP.
     */
    if (Enable()) {
        PANA_DhcpData_t dhcp_d;
        dhcp_d.id = 0;
        dhcp_d.nonce = LocalNonce();

        PANA_DhcpAvpWidget dhcp_c(PANA_AVPNAME_DHCP);
        dhcp_c.Get() = dhcp_d;

        pba.avpList().add(dhcp_c());
    }
}

void PANA_PaaDhcpSecurityAssociation::AffixToPBR
          (PANA_Message &pbr)
{
    /*
      Absence of this AVP in the PANA-Bind-Request message sent by the PAA
      indicates unavailability of this additional service. In that case,
      PaC MUST NOT include DHCP-AVP in its response, and PAA MUST ignore
      received DHCP-AVP. When this AVP is received by the PaC, it may or
      may not include the AVP in its response depending on its desire to
      create a DHCP SA. A DHCP SA can be created as soon as each entity
      has received and sent one DHCP-AVP.
     */
    if (Enable()) {
        SecretId() = m_SecretIdPool.Allocate();

        PANA_DhcpData_t dhcp_d;
        dhcp_d.id = SecretId();
        dhcp_d.nonce = LocalNonce();

        PANA_DhcpAvpWidget dhcp_c(PANA_AVPNAME_DHCP);
        dhcp_c.Get() = dhcp_d;

        pbr.avpList().add(dhcp_c());
    }
}

bool PANA_PaaDhcpSecurityAssociation::CheckPBA
          (PANA_Message &pba)
{
    /*
      Absence of this AVP in the PANA-Bind-Request message sent by the PAA
      indicates unavailability of this additional service. In that case,
      PaC MUST NOT include DHCP-AVP in its response, and PAA MUST ignore
      received DHCP-AVP. When this AVP is received by the PaC, it may or
      may not include the AVP in its response depending on its desire to
      create a DHCP SA. A DHCP SA can be created as soon as each entity
      has received and sent one DHCP-AVP.
     */
    PANA_DhcpAvpContainerWidget dhcp_c(pba.avpList());
    PANA_DhcpData_t *dhcp_d = dhcp_c.GetAvp(PANA_AVPNAME_DHCP);
    if (dhcp_d && Enable()) {
        PeerNonce() = dhcp_d->nonce;
        return true;
    }
    else {
        Enable() = false;
    }
    return Enable();
}
