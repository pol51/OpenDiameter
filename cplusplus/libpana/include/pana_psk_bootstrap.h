#ifndef __PANA_PSK_BOOTSTRAP_H__
#define __PANA_PSK_BOOTSTRAP_H__

#include "pana_parser.h"
#include "pana_exports.h"
#include "pana_prf_plus.h"

//Transform PSK to STR
#define PSK2STR(a) (unsigned char)(a)[0], (unsigned char)(a)[1], (unsigned char)(a)[2], (unsigned char)(a)[3], (unsigned char)(a)[4], (unsigned char)(a)[5], (unsigned char)(a)[6], (unsigned char)(a)[7], (unsigned char)(a)[8], (unsigned char)(a)[9], (unsigned char)(a)[10], (unsigned char)(a)[11], (unsigned char)(a)[12], (unsigned char)(a)[13], (unsigned char)(a)[14], (unsigned char)(a)[15], (unsigned char)(a)[16], (unsigned char)(a)[17], (unsigned char)(a)[18], (unsigned char)(a)[19], (unsigned char)(a)[20], (unsigned char)(a)[21], (unsigned char)(a)[22], (unsigned char)(a)[23], (unsigned char)(a)[24], (unsigned char)(a)[25], (unsigned char)(a)[26], (unsigned char)(a)[27], (unsigned char)(a)[28], (unsigned char)(a)[29], (unsigned char)(a)[30], (unsigned char)(a)[31]
//Transform PSK to STR
#define PSKSTR "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"

#define MAC2STR(a) (unsigned char)(a)[0], (unsigned char)(a)[1], (unsigned char)(a)[2], (unsigned char)(a)[3], (unsigned char)(a)[4], (unsigned char)(a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACSTRNO "%02x%02x%02x%02x%02x%02x"

class PANA_EXPORT PANA_PSKKey
{

    public:
        PANA_PSKKey(pana_octetstring_t &pac_epkey,
                    pana_octetstring_t chain = "IEEE802.11i PSK") {
            
            generate_key(pac_epkey,chain); 
        }
        virtual ~PANA_PSKKey() {
        }
        virtual pana_octetstring_t &Key() {
            return m_Key;
        }

    protected:
        virtual void generate_key(pana_octetstring_t &pac_epkey,
                    		  pana_octetstring_t &chain);

    private:
        pana_octetstring_t m_Key;


};


typedef std::list<pana_octetstring_t> PANA_PSKKeyList;
typedef PANA_PSKKeyList::iterator PAMA_PSKKeyListIterator;


#endif /* __PANA_PSK_BOOTSTRAP_H__ */
