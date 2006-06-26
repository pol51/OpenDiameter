#include "pana_prf_plus.h"

PANA_PrfPlus::PANA_PrfPlus(PANA_TypeHash hash)
{
#ifndef WIN32
   switch (hash) {
     case PRF_HMAC_SHA1:
        this->prf_function =(EVP_MD*) EVP_sha1();
        this->prf_size=EVP_MD_size(this->prf_function);
        break;
     case PRF_HMAC_MD5:
        this->prf_function =(EVP_MD*) EVP_md5();
	break;
     default:
        this->prf_function=NULL;
   }
   if (this->prf_function != NULL) {
       this->prf_size=EVP_MD_size(this->prf_function);
   }
#else
// TBD: need windows version
#endif
}

void PANA_PrfPlus::PRF(ACE_Byte * key, 
                       ACE_UINT16 key_length, 
                       ACE_Byte * sequence, 
                       ACE_UINT16 sequence_length, 
                       ACE_Byte * result) 
{
#ifndef WIN32
    ACE_UINT32 size;
    HMAC(prf_function, key, key_length, sequence, sequence_length, result, &size);
#else
// TBD: need windows version
#endif
}

void PANA_PrfPlus::PRF_plus(ACE_Byte iter, 
                            ACE_Byte * key, 
                            ACE_UINT16 key_length, 
                            ACE_Byte * sequence, 
                            ACE_UINT16 sequence_length, 
                            ACE_Byte * result)
{
#ifndef WIN32
    ACE_Byte *temp = new ACE_Byte [this->prf_size];
    ACE_Byte *new_sequence = new ACE_Byte [this->prf_size + sequence_length + 1];

    ACE_Byte current_iter = 1;

    // New sequence = S | 0x01
    memcpy(new_sequence, sequence, sequence_length);
    new_sequence[sequence_length] = current_iter;
    
	// Calculate T1 = prf(K, S | 0x01) = prf(K, new_sequence)
    PRF(key, key_length, new_sequence, sequence_length + 1, temp);
    
	// Insert into result
    memcpy(result, temp, prf_size);


    for (current_iter = 2; current_iter <= iter; current_iter++) {
        // New sequence = T | S | iter
        memcpy(new_sequence, temp, prf_size);
        memcpy(&new_sequence[prf_size], sequence, sequence_length);
        new_sequence[prf_size + sequence_length] = current_iter;

        // Calculate T1 = prf(K, S | 0x01) = prf(K, new_sequence)
        PRF(key, key_length, new_sequence, sequence_length + prf_size + 1, temp);
        // Insert into result
        memcpy(&result[ (current_iter - 1) * prf_size], temp, prf_size);
    }

    delete[] temp;
    delete[] new_sequence;
#else
// TBD: need windows version
#endif
}

PANA_PrfPlus::~PANA_PrfPlus() {
}
