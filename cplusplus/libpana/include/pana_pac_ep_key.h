#ifndef __PANA_PAC_EP_BOOTSTRAP_H__
#define __PANA_PAC_EP_BOOTSTRAP_H__

#include "pana_exports.h"
#include "pana_parser.h"
#include "pana_psk_bootstrap.h"

class PANA_EXPORT PANA_PAC_EP_Key
{
    public:
        PANA_PAC_EP_Key(pana_octetstring_t &aaaKey,
			ACE_UINT32 keyId,
                        pana_octetstring_t &sessionId,   
                        pana_octetstring_t &epdeviceId) 
        {
            generate_key(aaaKey, keyId, sessionId, epdeviceId); 
        }
        virtual ~PANA_PAC_EP_Key() {}
        virtual pana_octetstring_t &Key() {
            return m_Key;
        }

    protected:
        virtual void generate_key(pana_octetstring_t &aaaKey,
                                  ACE_UINT32 keyId,
                    		  pana_octetstring_t &sessionId,   
                    		  pana_octetstring_t &epdeviceId);

    private:
        pana_octetstring_t m_Key;
};

typedef std::list< pana_octetstring_t > PANA_PACEPKeyList;
typedef PANA_PACEPKeyList::iterator PANA_PACEPKeyListIterator;

#endif

