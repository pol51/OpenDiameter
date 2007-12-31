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

#include "od_utl_sha1.h"
#include "pana_security_assoc.h"
#include "pana_exceptions.h"
#include "pana_parser.h"
#include "openssl/hmac.h"
#include "openssl/sha.h"

ACE_UINT32 PANA_MSK::m_GlobalId = 0;

#define PANA_SA_DEBUG 1

PANA_MessageBuffer *PANA_MsgByteStream::Get(PANA_Message &msg)
{
    m_RawBuffer->reset();

    PANA_HeaderParser hp;
    hp.setRawData(m_RawBuffer);
    hp.setAppData(&msg);

    hp.parseAppToRaw();

    // Parse the payload
    PANA_PayloadParser pp;
    pp.setRawData(m_RawBuffer);
    pp.setAppData(&(msg.avpList()));
    pp.setDictData(hp.getDictData());

    pp.parseAppToRaw();

    // Re-do the header again to set the length
    msg.length() = m_RawBuffer->wr_ptr() - m_RawBuffer->base();
    hp.parseAppToRaw();

#if PANA_SA_DEBUG
    printf("RAW MSG [%d]: ", msg.length());
    for (size_t i=0; i<msg.length(); i++) {
        printf("%02X ", (unsigned char)((char*)m_RawBuffer->base())[i]);
    }
    printf("\n");
#endif
    return m_RawBuffer;
}

void PANA_AuthKey::Generate(PANA_Nonce &pacNonce,
                            PANA_Nonce &paaNonce,
                            pana_octetstring_t &iPAR,
                            pana_octetstring_t &iPAN,
                            pana_octetstring_t &msk,
                            ACE_UINT32 keyId)
{
   /*
    The PANA_AUTH_KEY is derived from the available MSK and it is used to
    integrity protect PANA messages.  The PANA_AUTH_KEY is computed in
    the following way:

      PANA_AUTH_KEY = prf+(MSK, I_PAR|I_PAN|PaC_nonce|PAA_nonce|Key_ID)

    where the prf+ function is defined in IKEv2 [RFC4306].  The
    pseudo-random function to be used for the prf+ function is negotiated
    using PRF-Algorithm AVP in the initial PANA-Auth-Request and
    PANA-Auth-Answer exchange with 'S' (Start) bit set.  The length of
    PANA_AUTH_KEY depends on the integrity algorithm in use.  See
    Section 5.4 for the detailed usage of the PANA_AUTH_KEY.  I_PAR and
    I_PAN are the initial PANA-Auth-Request and PANA-Auth-Answer messages
    (the PANA header and the following PANA AVPs) with 'S' (Start) bit
    set, respectively.  PaC_nonce and PAA_nonce are values of the Nonce
    AVP carried in the first non-initial PANA-Auth-Answer and
    PANA-Auth-Request messages in the authentication and authorization
    phase or the first PANA-Auth-Answer and PANA-Auth-Request messages in
    the re-authentication phase, respectively.  Key_ID is the value of
    the Key-Id AVP.
   */

    ACE_UINT32 incLen = 0;
    ACE_UINT32 seedLen = iPAR.length() + iPAN.length() + pacNonce.Get().size() + paaNonce.Get().size() + sizeof(keyId);
    unsigned char *seed = new unsigned char[seedLen];

    memcpy(seed, iPAR.data(), iPAR.length());
    incLen += iPAR.length();

    memcpy(seed + incLen, iPAN.data(), iPAN.length());
    incLen += iPAN.length();

    memcpy(seed + incLen, pacNonce.Get().data(), pacNonce.Get().size());
    incLen += pacNonce.Get().size();

    memcpy(seed + incLen, paaNonce.Get().data(), paaNonce.Get().size());
    incLen += paaNonce.Get().size();

    keyId = ACE_NTOHL(keyId);
    memcpy(seed + incLen, &keyId, sizeof(keyId));
    incLen += sizeof(keyId);

    /*
      prf+ (K,S) = T1 | T2 | T3 | T4 | ...

      where:
      T1 = prf (K, S | 0x01)
      T2 = prf (K, T1 | S | 0x02)
      T3 = prf (K, T2 | S | 0x03)
      T4 = prf (K, T3 | S | 0x04)
    */
    ACE_UINT32 resLen = 0;
    unsigned char sbuf[PANA_AUTH_HMACSIZE * PANA_AUTH_HMACSIZE_FACTOR];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    for (unsigned char tokenCount = 1; tokenCount <= PANA_AUTH_HMACSIZE_FACTOR; tokenCount ++) {

        HMAC_Init_ex(&ctx, msk.data(), msk.size(), EVP_sha1(), NULL);
        if (tokenCount > 1) {
            HMAC_Update(&ctx, sbuf + (PANA_AUTH_HMACSIZE * (tokenCount - 2)), PANA_AUTH_HMACSIZE);
        }
        HMAC_Update(&ctx, seed, seedLen);
        HMAC_Update(&ctx, &tokenCount, 1);
        HMAC_Final(&ctx,  sbuf + (PANA_AUTH_HMACSIZE * (tokenCount - 1)), &resLen);
    }

    HMAC_CTX_cleanup(&ctx);

    m_Value.assign((char*)(sbuf), PANA_AUTH_HMACSIZE);
    m_IsSet = true;

#if PANA_SA_DEBUG
    printf("Auth key[%d]: ", m_Value.size());
    for (size_t i=0; i<m_Value.size(); i++) {
        printf("%02X ", (unsigned char)((char*)m_Value.data())[i]);
    }
    printf("\n");
#endif
}

void PANA_SecurityAssociation::GenerateAuthKey()
{
    m_AuthKey.Generate(m_PacNonce,
                       m_PaaNonce,
                       m_PARStart,
                       m_PANStart,
                       m_MSK.Get(),
                       m_MSK.Id());
}

void PANA_SecurityAssociation::GenerateAuthAvpValue
           (const char *PDU,
            ACE_UINT32 PDULength,
            pana_octetstring_t &authValue)
{
   /*
      5.4.  Message Authentication

        A PANA message can contain an AUTH AVP for cryptographically
        protecting the message.

        When an AUTH AVP is included in a PANA message, the value field of
        the AUTH AVP is calculated by using the PANA_AUTH_KEY in the
        following way:

            AUTH AVP value = PANA_AUTH_HASH(PANA_AUTH_KEY, PANA_PDU)

        where PANA_PDU is the PANA message including the PANA header, with
        the AUTH AVP value field first initialized to 0.  PANA_AUTH_HASH
        represents the integrity algorithm negotiated using Integrity-
        Algorithm AVP in the initial PANA-Auth-Request and PANA-Auth-Answer
        exchange with 'S' (Start) bit set.  The PaC and PAA MUST use the same
        integrity algorithm to calculate an AUTH AVP they originate and
        receive.
     */

    unsigned int reslen = 0;
    unsigned char sbuf[PANA_AUTH_HMACSIZE];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));

    HMAC(EVP_sha1(), m_AuthKey.Get().data(), m_AuthKey.Get().size(),
         (const unsigned char*)PDU, PDULength, sbuf, &reslen);

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
    PANA_UInt32AvpWidget keyIdAvp(PANA_AVPNAME_KEYID);
    keyIdAvp.Get() = m_MSK.Id();
    msg.avpList().add(keyIdAvp());
    return (true);
}

bool PANA_SecurityAssociation::AddAuthAvp(PANA_Message &msg)
{
    // add auth-avp
    PANA_StringAvpWidget authAvp(PANA_AVPNAME_AUTH);
    pana_octetstring_t &auth = authAvp.Get();
    msg.avpList().add(authAvp());

    // init this to zero so we can compute true AUTH
    char sbuf[PANA_AUTH_HMACSIZE];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
    auth.assign(sbuf, sizeof(sbuf));

    PANA_MsgByteStream streamConverter;
    PANA_MessageBuffer *rawBuf = streamConverter.Get(msg);

    // generate auth value
    GenerateAuthAvpValue(rawBuf->base(),
                         msg.length(),
                         auth);
    msg.avpList().reset();

    return (true);
}

bool PANA_SecurityAssociation::ValidateAuthAvp(PANA_Message &msg)
{
    try {
        PANA_StringAvpContainerWidget authAvp(msg.avpList());
        pana_octetstring_t *auth = authAvp.GetAvp(PANA_AVPNAME_AUTH);
        if (auth == NULL) {
            throw (PANA_Exception(PANA_Exception::FAILED,
                                  "Missing AUTH-AVP"));
        }

        // temporarily save the AUTH value and insert 0 auth
        pana_octetstring_t mvalue;
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
        hp.setRawData(aBuffer);
        hp.setAppData(&msg);

        hp.parseAppToRaw();

        // Parse the payload
        PANA_PayloadParser pp;
        pp.setRawData(aBuffer);
        pp.setAppData(&(msg.avpList()));
        pp.setDictData(hp.getDictData());

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
        AAA_LOG((LM_ERROR, "(%P|%t) AUTH value is invalid\n"));
    }
    catch (AAAErrorCode &st) {
        AAA_LOG((LM_ERROR, "(%P|%t) Parsing error is session transmitter\n"));
    }
    catch (PANA_Exception &e) {
        AAA_LOG((LM_ERROR, "(%P|%t) %s\n", e.description().data()));
    }
    return (false);
}

