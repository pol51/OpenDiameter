#include "pana_pac_ep_key.h"


void PANA_PAC_EP_Key::generate_key(pana_octetstring_t &aaaKey,
                  ACE_UINT32 keyId,
                  ACE_UINT32 sessionId,   
                  pana_octetstring_t epdeviceId)
{
     
	std::string pac_ep_string = "IETF PEMK";
        ACE_UINT16 S_len;
        S_len=(ACE_UINT16)(pac_ep_string.length() + sizeof(sessionId) + sizeof(ACE_UINT32) + epdeviceId.size());
        //                  "PaC-EP master key" | Session ID          | Key-ID            | EP-Device-Id

        ACE_Byte *S = new ACE_Byte [S_len];
        ACE_Byte *pac_ep_master_key = new ACE_Byte [256];
		memset(pac_ep_master_key,0,256);
		//ACE_Byte *pac_ep_master_key = new ACE_Byte [40]; 
        ACE_OS::memcpy(S,pac_ep_string.data(),pac_ep_string.length());
        ACE_Byte *aux = (ACE_Byte *)(S+pac_ep_string.length());
        ACE_OS::memcpy(aux,&sessionId,sizeof(sessionId));
        
        aux = aux + sizeof(sessionId);
             
        ACE_OS::memcpy(aux,&keyId,sizeof(ACE_UINT32));
        aux = aux + sizeof(ACE_UINT32);
        ACE_OS::memcpy(aux,epdeviceId.data(),epdeviceId.size());
             
        PANA_PrfPlus prf_plus(PRF_HMAC_SHA1);
        //prf_plus.PRF((ACE_Byte *)aaaKey.data(),aaaKey.size(),S,S_len,pac_ep_master_key);
          
		int iterNum = PEMK_SIZE/prf_plus.prf_size+1;
        prf_plus.PRF_plus(iterNum,(ACE_Byte *)aaaKey.data(),aaaKey.size(),S,S_len,pac_ep_master_key);

	AAA_LOG((LM_DEBUG, "PaC-EP Master Key (len = %d) : ", PEMK_SIZE));
        for (int i=0 ; i < PEMK_SIZE; i++)
	       AAA_LOG((LM_DEBUG,"%3x",pac_ep_master_key[i])); 
        AAA_LOG((LM_DEBUG,"\n"));     

        m_Key.assign((const char *)pac_ep_master_key,PEMK_SIZE);
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

void PANA_IKE_Key::generate_key(pana_octetstring_t pemKey,
		  ACE_UINT32 keyId,
                  pana_octetstring_t mInfo,
                  ACE_UINT32 sessionId,   
                  pana_octetstring_t epdeviceId)
{
     
	std::string pac_ep_string = "IKE MSSK";
        ACE_UINT16 S_len;
        S_len=(ACE_UINT16)(pac_ep_string.length() + mInfo.size() + sizeof(ACE_UINT32) + sizeof(ACE_UINT32) + epdeviceId.size());
        //                  "PaC-EP master key" | Multicast Infor  |Session ID          | Key-ID-In            | EP-Device-Id

        
        ACE_Byte *S = new ACE_Byte [S_len];
        ACE_Byte *ike_key = new ACE_Byte [256];
	memset(ike_key,0,256);
 
        ACE_OS::memcpy(S,pac_ep_string.data(),pac_ep_string.length());
        ACE_Byte *aux = (ACE_Byte *)(S+pac_ep_string.length());

	ACE_OS::memcpy(aux,mInfo.data(),mInfo.size());
	aux = aux + mInfo.size();

        ACE_OS::memcpy(aux,&sessionId,sizeof(sessionId));
        aux = aux + sizeof(sessionId);
             
        ACE_OS::memcpy(aux,&keyId,sizeof(ACE_UINT32));
        aux = aux + sizeof(ACE_UINT32);

        ACE_OS::memcpy(aux,epdeviceId.data(),epdeviceId.size());
             
        PANA_PrfPlus prf_plus(PRF_HMAC_SHA1);
	int size = prf_plus.PRF((ACE_Byte *)pemKey.data(),pemKey.size(),S,S_len,ike_key);

        m_Key.assign((const char *)ike_key,size);
        delete [] S;
        delete [] ike_key;
}
