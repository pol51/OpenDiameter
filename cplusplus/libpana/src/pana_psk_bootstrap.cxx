#include "pana_psk_bootstrap.h"

void PANA_PSKKey::generate_key(pana_octetstring_t &pac_epkey,
                  pana_octetstring_t &chain)
{
	ACE_Byte *psk = new ACE_Byte[40];
	ACE_UINT16 S_len=chain.size();
        ACE_Byte *S = new ACE_Byte [S_len];
	ACE_OS::memcpy(S,chain.data(),chain.size());
	ACE_Byte *pac_ep_master_key = new ACE_Byte[pac_epkey.size()];
        ACE_OS::memcpy(pac_ep_master_key,pac_epkey.data(),pac_epkey.size());
	PANA_PrfPlus prf_plus(PRF_HMAC_SHA1);
        prf_plus.PRF_plus(2,pac_ep_master_key,20,S,S_len,psk);
        m_Key.assign((const char *)psk,32);
	delete [] S;
        delete [] pac_ep_master_key;
	delete [] psk;
}
