#include "pana_pac_ep_key.h"


void PANA_PAC_EP_Key::generate_key(pana_octetstring_t &aaaKey,
                  ACE_UINT32 keyId,
                  pana_octetstring_t &sessionId,   
                  pana_octetstring_t &epdeviceId)
{
     
	std::string pac_ep_string = "PaC-EP master key";
        ACE_UINT16 S_len;
        S_len=(ACE_UINT16)(pac_ep_string.length() + sessionId.size() + sizeof(ACE_UINT32) + epdeviceId.size());
        //                  "PaC-EP master key" | Session ID          | Key-ID            | EP-Device-Id

        ACE_Byte *S = new ACE_Byte [S_len];
        ACE_Byte *pac_ep_master_key = new ACE_Byte [40]; ;
        ACE_OS::memcpy(S,pac_ep_string.data(),pac_ep_string.length());
        ACE_Byte *aux = (ACE_Byte *)(S+pac_ep_string.length());
        ACE_OS::memcpy(aux,sessionId.data(),sessionId.size());
        
        aux = aux + sessionId.size();
             
        ACE_OS::memcpy(aux,&keyId,sizeof(ACE_UINT32));
        aux = aux + sizeof(ACE_UINT32);
        ACE_OS::memcpy(aux,epdeviceId.data(),epdeviceId.size());
             
        PANA_PrfPlus prf_plus(PRF_HMAC_SHA1);
        prf_plus.PRF_plus(2,(ACE_Byte *)aaaKey.data(),aaaKey.size(),S,S_len,pac_ep_master_key);
        
        printf("pana_pac_ep_key\n");
        for (int i=0 ; i < 32; i++)
	       printf("%02x",pac_ep_master_key[i]); 
	      printf("\n");       


        m_Key.assign((const char *)pac_ep_master_key,32);
        delete [] S;
        delete [] pac_ep_master_key;




     /*pana_octetstring_t pac_ep_string = "PaC-EP master key";
     ACE_UINT16 S_len;
        S_len=(ACE_UINT16)(pac_ep_string.length() + sessionId.size() + sizeof(ACE_UINT32) + epdeviceId.size());
        //                  "PaC-EP master key" | Session ID          | Key-ID            | EP-Device-Id

        ACE_Byte *S = new ACE_Byte [S_len];
        ACE_Byte *pac_ep_master_key = new ACE_Byte [20]; ;
        ACE_OS::memcpy(S,pac_ep_string.data(),pac_ep_string.length());
        ACE_Byte *aux = (ACE_Byte *)(S+pac_ep_string.length());
        ACE_OS::memcpy(aux,sessionId.data(),sessionId.size());
        
        aux = aux + sessionId.size();
             
        ACE_OS::memcpy(aux,&keyId,sizeof(ACE_UINT32));
        aux = aux + sizeof(ACE_UINT32);
        ACE_OS::memcpy(aux,epdeviceId.data(),epdeviceId.size());
             
        PrfPlus prf_plus(PRF_HMAstd::string pac_ep_string = "PaC-EP master key";
        ACE_UINT16 S_len;
        S_len=(ACE_UINT16)(pac_ep_string.length() + sessionId.size() + sizeof(ACE_UINT32) + epdeviceId.size());
        //                  "PaC-EP master key" | Session ID          | Key-ID            | EP-Device-Id

        ACE_Byte *S = new ACE_Byte [S_len];
        ACE_Byte *pac_ep_master_key = new ACE_Byte [40]; ;
        ACE_OS::memcpy(S,pac_ep_string.data(),pac_ep_string.length());
        ACE_Byte *aux = (ACE_Byte *)(S+pac_ep_string.length());
        ACE_OS::memcpy(aux,sessionId.data(),sessionId.size());
        
        aux = aux + sessionId.size();
             
        ACE_OS::memcpy(aux,&keyId,sizeof(ACE_UINT32));
        aux = aux + sizeof(ACE_UINT32);
        ACE_OS::memcpy(aux,epdeviceId.data(),epdeviceId.size());
             
        PrfPlus prf_plus(PRF_HMAC_SHA1);
        prf_plus.PRF_plus(2,(ACE_Byte *)aaaKey.data(),aaaKey.size(),S,S_len,pac_ep_master_key);
        
        printf("pana_pac_ep_key\n");
        for (int i=0 ; i < 32; i++)
	       printf("%02x",pac_ep_master_key[i]); 
	      printf("\n");       


        m_Key.assign((const char *)pac_ep_master_key,32);
        delete [] S;
        delete [] pac_ep_master_key;
C_SHA1);
        prf_plus.PRF((ACE_Byte *)aaaKey.data(), aaaKey.size(), S, S_len, pac_ep_master_key);
	printf("pana_pac_ep_key\n");
        for (int i=0 ; i < 20; i++)
	  printf("%02x",pac_ep_master_key[i]); 
	printf("\n");       


        m_Key.assign((const char *)pac_ep_master_key,20);
        delete [] S;
        delete [] pac_ep_master_key;*/


        

}
