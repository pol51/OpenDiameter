#ifndef __PANA_PAC_EP_BOOTSTRAP_H__
#define __PANA_PAC_EP_BOOTSTRAP_H__

#include "pana_exports.h"
#include "diameter_parser_api.h"
#include "pana_psk_bootstrap.h"

class PANA_EXPORT PANA_PAC_EP_Key
{
    public:
        PANA_PAC_EP_Key(diameter_octetstring_t &aaaKey,
			ACE_UINT32 keyId,
                        diameter_octetstring_t &sessionId,   
                        diameter_octetstring_t &epdeviceId) 
        {
            generate_key(aaaKey, keyId, sessionId, epdeviceId); 
        }
        virtual ~PANA_PAC_EP_Key() {}
        virtual diameter_octetstring_t &Key() {
            return m_Key;
        }

    protected:
        virtual void generate_key(diameter_octetstring_t &aaaKey,
                                  ACE_UINT32 keyId,
                    		  diameter_octetstring_t &sessionId,   
                    		  diameter_octetstring_t &epdeviceId);

    private:
        diameter_octetstring_t m_Key;
};

typedef std::list< diameter_octetstring_t > PANA_PACEPKeyList;
typedef PANA_PACEPKeyList::iterator PANA_PACEPKeyListIterator;

#endif

