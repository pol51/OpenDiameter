#ifndef __PANA_PAC_EP_BOOTSTRAP_H__
#define __PANA_PAC_EP_BOOTSTRAP_H__

#include "pana_exports.h"
#include "pana_parser.h"
#include "pana_psk_bootstrap.h"

#define PEMK_SIZE	64

class PANA_EXPORT PANA_PAC_EP_Key
{
    public:
        PANA_PAC_EP_Key(pana_octetstring_t &aaaKey,
			ACE_UINT32 keyId,
                        ACE_UINT32 sessionId,   
                        pana_octetstring_t epdeviceId) 
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
                    		  ACE_UINT32 sessionId,   
                    		  pana_octetstring_t epdeviceId);

    private:
        pana_octetstring_t m_Key;
};

typedef std::list< pana_octetstring_t > PANA_PACEPKeyList;
typedef PANA_PACEPKeyList::iterator PANA_PACEPKeyListIterator;


class PANA_EXPORT PANA_IKE_Key
{
    public:
        PANA_IKE_Key(pana_octetstring_t pemKey,
			ACE_UINT32 keyIdIn,ACE_UINT32 keyIdOut,
			pana_octetstring_t mInfoIn,pana_octetstring_t mInfoOut,
                        ACE_UINT32 sessionId,   
                        pana_octetstring_t epdeviceId) 
        {
            generate_key(pemKey, keyIdIn, mInfoIn, sessionId, epdeviceId); 
	    set_key_in();
	    generate_key(pemKey, keyIdOut, mInfoOut, sessionId, epdeviceId); 
	    set_key_out();
        }
        virtual ~PANA_IKE_Key() {}
        virtual pana_octetstring_t &Key_In() {
            return m_Key_In;
        }
virtual pana_octetstring_t &Key_Out() {
            return m_Key_Out;
        }

    protected:
        virtual void generate_key(pana_octetstring_t pemKey,
                                  ACE_UINT32 keyId, pana_octetstring_t mInfo,
                    		  ACE_UINT32 sessionId,   
                    		  pana_octetstring_t epdeviceId);
	void set_key_in(){m_Key_In.assign((const char *)m_Key.data(),m_Key.size());}
	void set_key_out(){m_Key_Out.assign((const char *)m_Key.data(),m_Key.size());}
	
    private:
	pana_octetstring_t m_Key;
        pana_octetstring_t m_Key_In;
	pana_octetstring_t m_Key_Out;
};

typedef std::list< pana_octetstring_t > PANA_PACEPKeyList;
typedef PANA_PACEPKeyList::iterator PANA_PACEPKeyListIterator;

#endif

