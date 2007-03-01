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

#ifndef __PANA_SECURITY_ASSOC_H__
#define __PANA_SECURITY_ASSOC_H__

#include "pana_exports.h"
#include "pana_message.h"

// mandatory key algorithm and integrity schemes
#define PANA_PRF_HMAC_SHA1         0x02
#define PANA_AUTH_HMAC_SHA1_160    0x07
#define PANA_AUTH_ALGORITHM()      ((PANA_PRF_HMAC_SHA1 << 16)|PANA_AUTH_HMAC_SHA1_160)
#define PANA_AUTH_HMACSIZE         20

class PANA_EXPORT PANA_Nonce :
    public PANA_ScholarValue<pana_octetstring_t>
{
    public:
        PANA_Nonce() {
        }
        PANA_Nonce(pana_octetstring_t &str) {
            Set(str);
        }
        bool operator==(PANA_Nonce &n) {
            if (Get().size() != n.Get().size()) {
               return (false);
            }
            return ACE_OS::memcmp(m_Value.data(),
                          n.Get().data(),
                          n.Get().size()) ? false : true;
        }
        PANA_Nonce &operator=(PANA_Nonce &n) {
            Set(n.Get());
            return *this;
        }
        void Generate() {
            ACE_UINT32 v[4];
            for (unsigned int i=0; i<sizeof(v)/sizeof(ACE_UINT32); i++) {
                 v[i] = ACE_OS::rand();
            }
            m_Value.assign((char*)v, sizeof(v));
            m_IsSet = true;
        }
};

class PANA_EXPORT PANA_MSK :
    public PANA_ScholarValue<pana_octetstring_t>
{
    public:
        PANA_MSK() {
            Reset();
        }
        ACE_UINT32 &Id() {
            return m_Id;
        }
        void Reset() {
            m_GlobalId ++;
            m_Id = m_GlobalId;
        }

    private:
        ACE_UINT32 m_Id;

    private:
        static ACE_UINT32 m_GlobalId;
};

class PANA_EXPORT PANA_AuthKey :
    public PANA_ScholarValue<pana_octetstring_t>
{
    public:
        void Generate(PANA_Nonce &pac,
                      PANA_Nonce &paa,
                      pana_octetstring_t &aaaKey,
                      ACE_UINT32 sessionId,
                      ACE_UINT32 keyId);
};

class PANA_EXPORT PANA_SecurityAssociation
{
    public:
        PANA_SecurityAssociation() {
        }
        PANA_Nonce &PacNonce() {
            return m_PacNonce;
        }
        PANA_Nonce &PaaNonce() {
            return m_PaaNonce;
        }
        void Reset() {
            m_MSK.Reset();
            m_AuthKey.Reset();
        }
        PANA_MSK &MSK() {
            return m_MSK;
        }
        PANA_AuthKey &Auth() {
            return m_AuthKey;
        }

        void GenerateAuthKey(ACE_UINT32 sessionId);
        bool AddKeyIdAvp(PANA_Message &msg);
        bool AddAuthAvp(PANA_Message &msg);
        bool ValidateAuthAvp(PANA_Message &msg);

    protected:
       void GenerateAuthAvpValue(const char *PDU,
                                ACE_UINT32 PDULength,
                                pana_octetstring_t &authValue);

    private:
        PANA_AuthKey m_AuthKey;
        PANA_Nonce   m_PacNonce;
        PANA_Nonce   m_PaaNonce;
        PANA_MSK     m_MSK;
};

#endif /* __PANA_SECURITY_ASSOC_H__ */
