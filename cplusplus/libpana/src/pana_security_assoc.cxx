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

#include "od_utl_sha1.h"
#include "pana_security_assoc.h"
#include "pana_exceptions.h"
#include "pana_memory_manager.h"

void PANA_AuthKey::Generate(PANA_Nonce &pac,
                            PANA_Nonce &paa,
                            diameter_octetstring_t &aaaKey,
                            diameter_utf8string_t &sessionId)
{
    //
    // The PANA_AUTH_Key is used to integrity protect PANA messages and
    // derived from AAA-Key(s).  When two AAA-Keys (AAA-Key1 and AAA-Key2)
    // are generated as a result of double EAP authentication (see Section
    // 4.3) the compound AAA-Key can be computed as follows ('|' indicates
    // concatenation):
    //
    //    AAA-Key = AAA-Key1 | AAA-Key2
    //
    //    PANA_AUTH_KEY = The first N bits of
    //              HMAC_SHA1(AAA-Key, PaC_nonce | PAA_nonce | Session-ID)
    //
    // where the value of N depends on the integrity protection algorithm in
    // use, i.e., N=160 for HMAC-SHA1.  The length of AAA-Key MUST be N bits
    // or longer.  See Section Section 4.1.6 for the detailed usage of the
    // PANA_AUTH_KEY.
    //
#if PANA_SA_DEBUG
    printf("AAA key[%d]: ", aaaKey.size());
    for (size_t i=0; i<aaaKey.size(); i++) {
        printf("%02X ", (unsigned char)((char*)aaaKey.data())[i]);
    }
    printf("\n");
    printf("Pac Nonce[%d]: ", pac.Get().size());
    for (size_t i=0; i<pac.Get().size(); i++) {
        printf("%02X ", (unsigned char)((char*)pac.Get().data())[i]);
    }
    printf("\n");
    printf("Paa Nonce[%d]: ", paa.Get().size());
    for (size_t i=0; i<paa.Get().size(); i++) {
        printf("%02X ", (unsigned char)((char*)paa.Get().data())[i]);
    }
    printf("\n");
#endif

    OD_Utl_Sha1 sha1;
    sha1.Update((unsigned char*)aaaKey.data(), aaaKey.size());
    sha1.Update((unsigned char*)pac.Get().data(), pac.Get().size());
    sha1.Update((unsigned char*)paa.Get().data(), paa.Get().size());
    sha1.Update((unsigned char*)sessionId.data(), sessionId.size());
    sha1.Final();

    char sbuf[PANA_AUTH_HMACSIZE];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
    sha1.GetHash((unsigned char*)sbuf);

    m_Value.assign((char*)(sbuf), sizeof(sbuf));
    m_IsSet = true;

#if PANA_SA_DEBUG
    printf("Auth key[%d]: ", m_Value.size());
    for (size_t i=0; i<m_Value.size(); i++) {
        printf("%02X ", (unsigned char)((char*)m_Value.data())[i]);
    }
    printf("\n");
#endif
}

void PANA_SecurityAssociation::GenerateAuthKey
    (diameter_utf8string_t &sessionId) 
{
    if (m_Type == DOUBLE) {
        diameter_octetstring_t combinedKey;
        if (! m_AAAKey1.IsSet()) {
            throw (PANA_Exception(PANA_Exception::FAILED, 
                   "Auth key generation failed, no keys present"));
        }
        combinedKey = m_AAAKey1.Get();
        if (m_AAAKey2.IsSet()) {
            combinedKey += m_AAAKey2.Get();
        }
        Set(combinedKey);
    }
    m_AuthKey.Generate(m_PacNonce,
                      m_PaaNonce,
                      m_Value,
                      sessionId);
}

void PANA_SecurityAssociation::GenerateAuthAvpValue
           (const char *PDU,
            ACE_UINT32 PDULength,
            diameter_octetstring_t &authValue)
{
    //
    //   A PANA message can contain a AUTH (Message Authentication Code) AVP 
    //   for cryptographically protecting the message. 
    //
    //   When a AUTH AVP is included in a PANA message, the value field of the 
    //   AUTH AVP is calculated by using the PANA_AUTH_Key in the following 
    //   way: 
    //
    //       AUTH AVP value = HMAC_SHA1(PANA_AUTH_Key, PANA_PDU) 
    //
    //   where PANA_PDU is the PANA message including the PANA header, with 
    //   the AUTH AVP value field first initialized to 0.  
    //    
    OD_Utl_Sha1 sha1;
    sha1.Update((unsigned char*)m_AuthKey.Get().data(), 
                 m_AuthKey.Get().size());
    sha1.Update((unsigned char*)PDU, PDULength);
    sha1.Final();

    // TBD: network byte ordering needs done

    char sbuf[PANA_AUTH_HMACSIZE];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
    sha1.GetHash((unsigned char*)sbuf);
    authValue.assign((char*)sbuf, sizeof(sbuf));

#if PANA_SA_DEBUG
    printf("Generate - Auth Key[%d]: ", m_AuthKey.Get().size());
    for (size_t i=0; i<m_AuthKey.Get().size(); i++) {
        printf("%02X ", (unsigned char)((char*)m_AuthKey.Get().data())[i]);
    }
    printf("\n");
    
    printf("Generate - PDU [%d]: ", PDULength);
    for (size_t i=0; i<PDULength; i++) {
        printf("%02X ", (unsigned char)PDU[i]);
    }
    printf("\n");

    printf("Generate - AUTH Value [%d]: ", authValue.size());
    for (size_t i=0; i<authValue.size(); i++) {
        printf("%02X ", (unsigned char)((char*)authValue.data())[i]);
    }
    printf("\n");
#endif
}

bool PANA_SecurityAssociation::AddKeyIdAvp(PANA_Message &msg)
{
    ACE_UINT32 keyId;
    if (m_Type == DOUBLE) {
        if (! m_AAAKey1.IsSet()) {
            return (false);
        }
        else if (! m_AAAKey2.IsSet()) {
            keyId = m_AAAKey1.Id();
        }
        else {
            keyId = m_AAAKey2.Id();
        }
    }
    else {
        return (false);
    }
    DiameterUInt32AvpWidget keyIdAvp(PANA_AVPNAME_KEYID);
    keyIdAvp.Get() = ACE_HTONL(keyId);
    msg.avpList().add(keyIdAvp());
    return (true);
}

bool PANA_SecurityAssociation::AddAuthAvp(PANA_Message &msg)
{
    // add auth-avp
    DiameterStringAvpWidget authAvp(PANA_AVPNAME_AUTH);
    diameter_octetstring_t &auth = authAvp.Get();
    msg.avpList().add(authAvp());

    // init this to zero so we can compute true AUTH
    char sbuf[PANA_AUTH_HMACSIZE];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
    auth.assign(sbuf, sizeof(sbuf));

    PANA_MessageBuffer *rawBuf = PANA_MESSAGE_POOL()->malloc();

    PANA_HeaderParser hp;
    DiameterDictionaryOption opt(PARSE_STRICT, PANA_DICT_PROTOCOL_ID);
    hp.setRawData(reinterpret_cast<AAAMessageBlock*>(rawBuf));
    hp.setAppData(static_cast<PANA_MsgHeader*>(&msg));
    hp.setDictData(&opt);

    hp.parseAppToRaw();

    // Parse the payload
    PANA_PayloadParser pp;
    pp.setRawData(reinterpret_cast<AAAMessageBlock*>(rawBuf));
    pp.setAppData(&(msg.avpList()));
    pp.setDictData(msg.getDictHandle());

    pp.parseAppToRaw();

    // Re-do the header again to set the length
    msg.length() = rawBuf->wr_ptr() - rawBuf->base();
    hp.parseAppToRaw();

    // generate auth value
    GenerateAuthAvpValue(rawBuf->base(), 
                         msg.length(),
                         auth);
    PANA_MESSAGE_POOL()->free(rawBuf);
    msg.avpList().reset();

    return (true);
}

bool PANA_SecurityAssociation::ValidateAuthAvp(PANA_Message &msg)
{
    try {
        DiameterStringAvpContainerWidget authAvp(msg.avpList());
        diameter_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
        if (auth == NULL) {
            throw (PANA_Exception(PANA_Exception::FAILED, 
                                  "Missing AUTH-AVP"));
        }

        // temporarily save the AUTH value and insert 0 auth
        diameter_octetstring_t mvalue;
        mvalue.assign(auth->data(), auth->size());

#if PANA_SA_DEBUG
        printf("Validate - Received auth[%d]: ", mvalue.size());
        for (size_t i=0; i<mvalue.size(); i++) {
            printf("%02X ", (unsigned char)((char*)mvalue.data())[i]);
        }
        printf("\n");
#endif

        // reset to zero
        char sbuf[PANA_AUTH_HMACSIZE];
        ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
        auth->assign(sbuf, sizeof(sbuf));

        PANA_MessageBuffer *aBuffer = PANA_MESSAGE_POOL()->malloc();
        msg.avpList().reset();

        // parse the message 
        PANA_HeaderParser hp;
        DiameterDictionaryOption opt(PARSE_STRICT, PANA_DICT_PROTOCOL_ID);
        hp.setRawData(reinterpret_cast<AAAMessageBlock*>(aBuffer));
        hp.setAppData(static_cast<PANA_MsgHeader*>(&msg));
        hp.setDictData(&opt);

        hp.parseAppToRaw();

        // Parse the payload
        PANA_PayloadParser pp;
        pp.setRawData(reinterpret_cast<AAAMessageBlock*>(aBuffer));
        pp.setAppData(&(msg.avpList()));
        pp.setDictData(msg.getDictHandle());

        pp.parseAppToRaw();

        // Re-do the header again to set the length
        msg.length() = aBuffer->wr_ptr() - aBuffer->base();
        hp.parseAppToRaw();

        GenerateAuthAvpValue(aBuffer->base(), 
                             msg.length(),
                             *auth);        
        PANA_MESSAGE_POOL()->free(aBuffer);

#if PANA_SA_DEBUG
        printf("Validate - Auth Key[%d]: ", m_AuthKey.Get().size());
        for (size_t i=0; i<m_AuthKey.Get().size(); i++) {
            printf("%02X ", (unsigned char)((char*)m_AuthKey.Get().data())[i]);
        }
        printf("\n");
        
        printf("Validate - PDU [%d]: ", aBuffer->length());
        for (size_t i=0; i<aBuffer->length(); i++) {
            printf("%02X ", (unsigned char)((char*)aBuffer->base())[i]);
        }
        printf("\n");

        printf("Validate - Computed AUTH Value [%d]: ", auth->size());
        for (size_t i=0; i<auth->size(); i++) {
            printf("%02X ", (unsigned char)((char*)auth->data())[i]);
        }
        printf("\n");
#endif

        // do comparison
        if (*auth == mvalue) {
            return (true);
        }
        ACE_DEBUG((LM_ERROR, "(%P|%t) AUTH value is invalid\n"));
    }
    catch (DiameterErrorCode &st) {
        ACE_DEBUG((LM_ERROR, "(%P|%t) Parsing error is session transmitter\n"));
    }  
    catch (PANA_Exception &e) {
        ACE_DEBUG((LM_ERROR, "(%P|%t) %s\n", e.description().data()));
    }
    return (false);
}

