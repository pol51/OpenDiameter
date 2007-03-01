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

#include <ace/OS.h>
#include "pana_parser.h"

static AAADictionaryEntry PANA_CatchAllAvp(0, "AVP", AAA_AVP_DATA_TYPE, 0, 0);

static PANA_AvpValueParser* PANA_CreateAvpValueParser(AAAAvpDataType type)// throw(DiameterErrorCode)
{
    AAAErrorCode st;
    PANA_AvpType *t = (PANA_AvpType*)PANA_AvpTypeList::instance()->search(type);
    if (t == NULL) {
        AAA_LOG((LM_ERROR, "Specified avp type not found"));
        st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
        throw (st);
    }
    PANA_AvpValueParser *p = t->createParser();
    if (p == NULL) {
        AAA_LOG((LM_ERROR, "Avp value parser not found"));
        st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER);
        throw (st);
    }
    return p;
}

static int PANA_CheckFlags(PANA_AvpHeader::Flags flag, AAAAVPFlag flags)
{
    if (flag.mandatory == 0 && (flags & PANA_AVP_FLAG_MANDATORY)) {
        AAA_LOG((LM_ERROR, "M-flag must be set\n"));
        return -1;
    }
    if (flag.mandatory == 1 && (flags & PANA_AVP_FLAG_MANDATORY) == 0) {
        AAA_LOG((LM_ERROR, "M-flag must not be set\n"));
        return -1;
    }
    if (flag.vendor == 0 && (flags & PANA_AVP_FLAG_VENDOR_SPECIFIC)) {
        AAA_LOG((LM_ERROR, "V-flag needs to be set\n"));
        return -1;
    }
    if (flag.vendor == 1 && (flags & PANA_AVP_FLAG_VENDOR_SPECIFIC) == 0) {
        AAA_LOG((LM_ERROR, "V-flag must not be set\n"));
        return -1;
    }
    return 0;
}

PANA_AvpList_S::PANA_AvpList_S()
{
    this->add(&PANA_CatchAllAvp);
}

PANA_AvpList_S::~PANA_AvpList_S()
{
    pop_front(); // remove ANY AVP
}

void PANA_AvpHeaderList::create(PANA_MessageBuffer *aBuffer)
    throw (AAAErrorCode)
{
    PANA_AvpHeader h;
    AAAErrorCode st;
    char *start = aBuffer->rd_ptr();
    char *end = aBuffer->base()+aBuffer->size();

    for (char *cavp = start; cavp < end; cavp += adjust_word_boundary(h.m_Length)) {
        char *p = cavp;
        h.m_Code = ACE_NTOHS(*((ACE_UINT16*)p)); p += 2;
        h.m_Flags.vendor = (*((ACE_UINT16*)p) & PANA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0;
        h.m_Flags.mandatory = (*((ACE_UINT16*)p) & PANA_AVP_FLAG_MANDATORY) ? 1 : 0;
        p += 2;

        h.m_Length = ACE_NTOHS(*((ACE_UINT16*)p)); p += 4;
        if (h.m_Length == 0 || h.m_Length > (ACE_UINT32)(end-cavp)) {
            AAAErrorCode st;
            AAA_LOG((LM_ERROR, "invalid message length\n"));
            st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_MESSAGE_LENGTH);
            throw st;
        }

        if (h.m_Flags.vendor == 1) {
            h.m_Flags.vendor = ACE_NTOHL(*((ACE_UINT32*)p)); p+=4;
        }
        h.m_pValue = cavp;      // Store the pointer to the header head
        push_back(h);
    }

    aBuffer->rd_ptr(end);
}

template<> void PANA_HeaderParser::parseRawToApp()
{
    PANA_MessageBuffer *aBuffer = getRawData();
    PANA_MsgHeader *h = reinterpret_cast<PANA_MsgHeader*>(getAppData());

    char *p = aBuffer->rd_ptr();

    // version
    h->version() = AAAUInt8(*((AAAUInt8*)(p)));
    p += sizeof(ACE_UINT16);

    // length
    h->length() = ACE_NTOHS(*((ACE_UINT16*)(p)));
    p += sizeof(ACE_UINT16);

    // flags
    h->flags().request = *((AAAUInt8*)(p)) & 0x80 ? 1 : 0;
    h->flags().reserved = 0;
    p += sizeof(ACE_UINT16);

    // type
    h->type() = ACE_NTOHS(*((ACE_UINT16*)(p)));
    p += sizeof(ACE_UINT16);

    // validate if type is present
    setDictData(PANA_CommandList::instance()->search(h->type(), h->flags()));
    if (getDictData() == NULL) {
        AAAErrorCode st;
        AAA_LOG((LM_ERROR, "Message type [%d] not present in the dictionary\n", h->type()));
        st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
        throw st;
    }

    // session identifier
    h->sessionId() = ACE_UINT32(ACE_NTOHL(*((ACE_UINT32*)(p))));
    p += sizeof(ACE_UINT32);

    // sequence number
    h->seq() = ACE_UINT32(ACE_NTOHL(*((ACE_UINT32*)(p))));
    p += sizeof(ACE_UINT32);

    // start of AVP's
    aBuffer->rd_ptr(p);
}

template<> void PANA_HeaderParser::parseAppToRaw()
{
    PANA_MessageBuffer *aBuffer = getRawData();
    PANA_MsgHeader *h = reinterpret_cast<PANA_MsgHeader*>
                                            (getAppData());

    // validate if type is present
    setDictData(PANA_CommandList::instance()->search(h->type(), h->flags()));
    if (getDictData() == NULL) {
        AAAErrorCode st;
        AAA_LOG((LM_ERROR, "Message type [%d] not present in the dictionary\n", h->type()));
        st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
        throw st;
    }

    aBuffer->wr_ptr(aBuffer->base());

    char *p = aBuffer->base();

    // version
    *((ACE_UINT16*)(p)) = 0;
    *((AAAUInt8*)(p)) = h->version();
    p += sizeof(ACE_UINT16);

    // length
    *((ACE_UINT16*)(p)) = ACE_HTONS(h->length());
    p += sizeof(ACE_UINT16);

    // flags
    *((ACE_UINT16*)(p)) = 0;
    *((AAAUInt8*)(p)) |= h->flags().request ? 0x80 : 0x0;
    p += sizeof(ACE_UINT16);

    // type
    *((ACE_UINT16*)(p)) = ACE_HTONS(h->type());
    p += sizeof(ACE_UINT16);

    // session identifier
    *((ACE_UINT32*)(p)) = ACE_UINT32(ACE_HTONL(h->sessionId()));
    p += sizeof(ACE_UINT32);

    // seq number
    *((ACE_UINT32*)(p)) = ACE_UINT32(ACE_HTONL(h->seq()));
    p += sizeof(ACE_UINT32);

    aBuffer->wr_ptr(p);
}

template<> void PANA_PayloadParser::parseRawToApp()// throw(DiameterErrorCode)
{
    PANA_MessageBuffer *aBuffer = getRawData();
    AAAAvpContainerList *acl = getAppData();
    PANA_Dictionary *dict = (PANA_Dictionary*)getDictData();
    PANA_QualifiedAvpList::iterator i;
    AAAAvpContainerMngr cm;
    AAAAvpContainer *c;
    AAAAvpParseType pt;
    AAAQualifiedAVP *qavp;
    AAAErrorCode st;

    int type;
    unsigned int min, max;
    const char *name;

    PANA_AvpHeaderList ahl;
    try {
        ahl.create(aBuffer);
    }
    catch (AAAErrorCode &st) {
        throw st;
    }

    PANA_QualifiedAvpList *qavp_l[3] = { dict->m_Fixed,
                                         dict->m_Required,
                                         dict->m_Optional
                                       };
    for (int j=0; j<3; j++) {
        for (i = qavp_l[j]->begin(); i != qavp_l[j]->end(); i++) {
            pt = qavp_l[j]->getParseType();
            qavp = *i;
            min = qavp->qual.min;
            max = qavp->qual.max;
            name = qavp->avp->avpName.c_str();
            type = qavp->avp->avpType;

            c = cm.acquire(name);
            c->ParseType() = pt;

            do {
                PANA_AvpParser ap;
                PANA_AvpRawData rawData;
                rawData.ahl = &ahl;
                ap.setRawData(&rawData);
                ap.setAppData(c);
                ap.setDictData(qavp->avp);
                try {
                    ap.parseRawToApp();
                }
                catch (AAAErrorCode &st) {
                    AAA_PARSE_ERROR_TYPE type;
                    int code;
                    st.get(type, code);
                    if (type == AAA_PARSE_ERROR_TYPE_NORMAL && code == AAA_MISSING_AVP) {
                        // AVP was not found
                        c->releaseEntries();
                        cm.release(c);
                        if (0 == min) {
                            continue;
                        }
                        if (pt == AAA_PARSE_TYPE_OPTIONAL) {
                            continue;
                        }
                        AAA_LOG((LM_ERROR, "missing %s avp.\n", name));
                        throw;
                    }
                    else {
                        // Parse error 
                        AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
                        cm.release(c);
                        throw;
                    }
                }
                // Check number of containers
                if (c->size() < min) {
                    AAA_LOG((LM_ERROR, "at lease min %s avp needed.\n", name));
                    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP);
                    c->releaseEntries();
                    cm.release(c);
                    throw st;
                }
                if (c->size() > max) {
                    AAA_LOG((LM_ERROR, "at most max[%d] %s avp allowed.\n", max, name));
                    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_OCCURS_TOO_MANY_TIMES);
                    c->releaseEntries();
                    cm.release(c);
                    throw st;
                }
                acl->add(c);
            } while (0);
        }
    }
}

template<> void PANA_PayloadParser::parseAppToRaw()// throw(DiameterErrorCode)
{
    AAAAvpParseType pt;
    AAAQualifiedAVP *qavp;
    AAAAvpContainer *c;
    AAAErrorCode st;
    PANA_QualifiedAvpList::iterator i;

    int type;
    unsigned int min, max;
    const char *name;

    PANA_Dictionary *dict = (PANA_Dictionary*)getDictData();
    AAAAvpContainerList *acl = getAppData();
    PANA_QualifiedAvpList *qavp_l[3] = { dict->m_Fixed,
                                         dict->m_Required,
                                         dict->m_Optional
                                       };
    for (int j=0; j < 3; j++) {
        for (i = qavp_l[j]->begin(); i != qavp_l[j]->end(); i++) {
            pt = qavp_l[j]->getParseType();
            qavp = *i;
            min = qavp->qual.min;
            max = qavp->qual.max;
            name = qavp->avp->avpName.c_str();
            type = qavp->avp->avpType;

            if ((c = acl->search(qavp->avp->avpName.c_str())) == NULL) {
                if (min > 0 && max > 0) {
                    AAA_LOG((LM_ERROR, "missing avp %s in container.\n", name));
                    st.set(AAA_PARSE_ERROR_TYPE_BUG,
                           AAA_PARSE_ERROR_MISSING_CONTAINER);
                    throw st;
                }
                continue;
            }

            if (max == 0) {
                AAA_LOG((LM_ERROR, "%s must not appear in container.\n", name));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                       AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
                throw st;
            }
            if (c->size() < min) {
                AAA_LOG((LM_ERROR, "less than min entries for the AVP.\n"));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                       AAA_PARSE_ERROR_TOO_LESS_AVP_ENTRIES);
                throw st;
            }
            if (c->size() > max) {
                AAA_LOG((LM_ERROR, "more than max entries for the AVP.\n"));
                st.set(AAA_PARSE_ERROR_TYPE_BUG,
                        AAA_PARSE_ERROR_TOO_MUCH_AVP_ENTRIES);
                throw st;
            }
            if (c->size() == 0) {
                AAA_LOG((LM_INFO, "container is empty.\n"));
                continue;
            }

            c->ParseType() = pt;
            PANA_AvpParser ap;
            PANA_AvpRawData rawData;
            rawData.msg = getRawData();
            ap.setRawData(&rawData);
            ap.setAppData(c);
            ap.setDictData(qavp->avp);

            try {
                ap.parseAppToRaw();
            }
            catch (AAAErrorCode &st)
                {
                AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
                throw;
                }
            }
        }
}

template<> void PANA_AvpParser::parseRawToApp()// throw(DiameterErrorCode)
{
    PANA_AvpRawData* rawData = getRawData();
    AAAAvpContainer *c = getAppData();
    AAADictionaryEntry *avp = getDictData();

    AAAErrorCode st;
    PANA_MessageBuffer *aBuffer;

    for (int i=0; ; i++) {
        PANA_AvpContainerEntryManager em;
        PANA_AvpValueParser *vp;
        AAAAvpContainerEntry* e;
        PANA_AvpHeader h;
        h.m_ParseType = c->ParseType();

        /* header check */
        PANA_AvpHeaderParser hp;
        hp.setRawData(rawData);
        hp.setAppData(&h);
        hp.setDictData(avp);
        try {
            hp.parseRawToApp();
        }
        catch (AAAErrorCode &st) {
            int code;
            AAA_PARSE_ERROR_TYPE type;
            st.get(type, code);
            if (i > 0 &&
                type == AAA_PARSE_ERROR_TYPE_NORMAL &&
                code == AAA_MISSING_AVP) {
                // return if no more entry is found once after getting
                // at lease one entry received.
                return;
            }
            throw st;
        }

        /* payload check */
        e = em.acquire(avp->avpType);
        c->add(e);

        try {
            vp = PANA_CreateAvpValueParser(avp->avpType);
        }
        catch (AAAErrorCode &st) {
            throw st;
        }

        aBuffer =
            PANA_MessageBuffer::Acquire(h.m_pValue, h.m_Length-PANA_AVP_HEADER_LEN(avp));
        vp->setRawData(aBuffer);
        vp->setAppData(e);
        vp->setDictData(avp);

        try {
            vp->parseRawToApp();
        }
        catch (AAAErrorCode &st) {
            aBuffer->Release();
            delete vp;
            throw st;
        }
        delete vp;
        aBuffer->Release();
    }
}

template<> void PANA_AvpParser::parseAppToRaw()// throw(DiameterErrorCode)
{
    PANA_AvpRawData* rawData = getRawData();
    AAAAvpContainer *c = getAppData();
    AAADictionaryEntry *avp = getDictData();

    PANA_MessageBuffer *aBuffer = rawData->msg;
    PANA_AvpHeader h;
    AAAErrorCode st;
    if (! avp) {
        AAA_LOG((LM_ERROR, "AVP dictionary cannot be null."));
        st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
        throw st;
    }

    if (avp->avpType == AAA_AVP_DATA_TYPE) {
        /* Any AVP */
        for (unsigned int i=0; i<c->size(); i++) {
            PANA_AvpValueParser *vp;
            try {
                vp = PANA_CreateAvpValueParser(avp->avpType);
            }
            catch (AAAErrorCode &st) {
                throw st;
            }
            vp->setRawData(aBuffer);
            vp->setAppData((*c)[i]);
            vp->setDictData(avp);
            vp->parseAppToRaw();

            aBuffer->wr_ptr
                (aBuffer->base() +
                adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base()));

            delete vp;
        }
        return;
    }

    for (unsigned int i=0; i < c->size(); i++) {
        char *saved_p = aBuffer->wr_ptr();

        ACE_OS::memset(&h, 0, sizeof(h));
        h.m_Code = avp->avpCode;
        h.m_Flags.vendor = (avp->flags & PANA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0;
        h.m_Flags.mandatory = (avp->flags & PANA_AVP_FLAG_MANDATORY) ? 1 : 0;
        h.m_Vendor = avp->vendorId;

        PANA_AvpHeaderParser hp;
        hp.setRawData(rawData);
        hp.setAppData(&h);
        hp.setDictData(avp);
        hp.parseAppToRaw();

        PANA_AvpValueParser *vp = PANA_CreateAvpValueParser(avp->avpType);
        vp->setRawData(aBuffer);
        vp->setAppData((*c)[i]);
        vp->setDictData(avp);
        vp->parseAppToRaw();
        delete vp;

        // calculate the actual header length
        h.m_Length = ACE_UINT32(aBuffer->wr_ptr() - saved_p);

        // save the current write pointer
        saved_p = aBuffer->base() +
            adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base());

        // set the header again
        aBuffer->wr_ptr(aBuffer->wr_ptr() - h.m_Length);
        hp.parseAppToRaw();

        // restore the write pointer
        aBuffer->wr_ptr(saved_p);
    }
}

template<>
void PANA_AvpHeaderParser::parseRawToApp()
{
    PANA_AvpRawData *rawData = getRawData();
    PANA_AvpHeader *h = getAppData();
    AAADictionaryEntry *avp = getDictData();

    AAAErrorCode st;
    PANA_AvpHeaderList *ahl = rawData->ahl;
    PANA_AvpHeaderList::iterator i;
    AAAAvpParseType parseType = h->m_ParseType;

    if (! avp) {
        AAA_LOG((LM_ERROR, "AVP dictionary cannot be null."));
        st.set(AAA_PARSE_ERROR_TYPE_BUG,
            AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
        throw st;
    }
    if (ahl->empty()) {
        st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP);
        throw st;
    }

    if (parseType == AAA_PARSE_TYPE_FIXED_HEAD) {
        if (avp->avpCode == 0) {
            AAA_LOG((LM_ERROR, "Wildcard AVP cannot be a fixed AVP."));
            st.set(AAA_PARSE_ERROR_TYPE_BUG,
                AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM);
            throw st;
        }
        i = ahl->begin();
        if ((*i).m_Code == avp->avpCode) {
            *h = *i; ahl->erase(i);
            h->m_pValue += PANA_AVP_HEADER_LEN(avp);  // restore original value_p
            return;
        }
    } else {
        for (i=ahl->begin(); i!=ahl->end(); i++) {
            // Wildcard AVPs match any AVPs.
            if (avp->avpCode != 0) {
                // For non-wildcard AVPs, strict checking on v-flag, Vencor-Id
                // and AVP code is performed.
                if ((*i).m_Flags.vendor !=
                    ((avp->flags & PANA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0)) {
                    continue;
                }
                if ((*i).m_Vendor != avp->vendorId) {
                    continue;
                }
                if ((*i).m_Code != avp->avpCode) {
                    continue;
                }
            }

            *h = *i; ahl->erase(i);
            if (avp->avpCode > 0 && PANA_CheckFlags(h->m_Flags, avp->flags) != 0) {
                st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_BITS);
                throw st;
            }
            h->m_pValue += PANA_AVP_HEADER_LEN(avp);  // restore original value_p
            return;
        }
    }

    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP);
    throw st;
}

template<>
void PANA_AvpHeaderParser::parseAppToRaw()
{
    PANA_AvpRawData *data = getRawData();
    char *p = data->msg->wr_ptr();
    PANA_AvpHeader *h = getAppData();

    *((ACE_UINT16*)p) = ACE_NTOHS(h->m_Code);
    p+=2;
    /* initialize this field to prepare for bit OR operation */
    *((ACE_UINT16*)p) = 0;
    if (h->m_Flags.vendor) {
        *((ACE_UINT16*)p)|=PANA_AVP_FLAG_VENDOR_SPECIFIC;
    }
    if (h->m_Flags.mandatory) {
        *((ACE_UINT16*)p)|=PANA_AVP_FLAG_MANDATORY;
    }
    p+=2;

    *((ACE_UINT16*)p) = ACE_NTOHS(h->m_Length);
    p+=2;
    *((ACE_UINT16*)p) = 0;
    p+=2;

    if (h->m_Flags.vendor) {
        *((ACE_UINT32*)p) = ACE_NTOHL(h->m_Vendor);
        p+=4;
    }
    data->msg->wr_ptr(p);
}


