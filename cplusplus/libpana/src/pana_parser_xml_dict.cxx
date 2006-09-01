/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
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
/* $Id: xml_parsing.cxx,v 1.1 2006/05/31 18:02:13 vfajardo Exp $ */

#include <string>
#include <map>
#include "ACEXML/common/FileCharStream.h"
#include "ACEXML/parser/parser/Parser.h"
#include "ACEXML/common/DefaultHandler.h"
#include "aaa_comlist.h"
#include "aaa_avplist.h"
#include "aaa_q_avplist.h"
#include "aaa_g_avplist.h"
#include "diameter_parser.h"

class PANAXML_Element;
typedef std::list<PANAXML_Element*> PANAXML_ElementStack;

class PANAXML_Element
{
  public:
     PANAXML_Element(char *name, 
                    PANAXML_ElementStack &stack) :
        m_inProcess(false), 
        m_isSkipped(false),
        m_name(name),
        m_callStack(stack),
        m_parent(NULL) {
     }
     virtual ~PANAXML_Element() {
     }
     std::string &Name() {
        return m_name;
     }
     virtual bool startElement(ACEXML_Attributes *atts) {
        if (m_inProcess) {
            ACE_DEBUG ((LM_ERROR, "Error: element %s already in process\n",
                        m_name.data()));
            return false;
        }
        m_inProcess = true;
        if (! m_callStack.empty()) {
            m_parent = m_callStack.front();
        }
        m_callStack.push_front(this);
        return true;
     }
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length ACEXML_ENV_ARG_DECL) {
        if (! m_inProcess) {
            ACE_DEBUG ((LM_ERROR, "Error: element %s not in process\n",
                        m_name.data()));
            return false;
        }
        return true;
     }
     virtual bool endElement() {
        if (! m_inProcess) {
            ACE_DEBUG ((LM_ERROR, "Error: element %s not in process\n",
                       m_name.data()));
            return false;
        }
        m_inProcess = false;
        m_callStack.pop_front();
        m_parent = NULL;
        return true;
     }
     PANAXML_ElementStack &CallStack() {
        return m_callStack;
     }
     PANAXML_Element *Parent() {
        return m_parent;
     }
     bool &IsSkipped() {
        return m_isSkipped;
     }

  private:
     bool m_inProcess;
     bool m_isSkipped;
     std::string m_name;
     PANAXML_ElementStack &m_callStack;
     PANAXML_Element *m_parent;
};

class PANAXML_DictionaryElement :
  public PANAXML_Element
{
  public:
     PANAXML_DictionaryElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("dictionary", stack) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }
        return true;
     }
     virtual bool endElement() {
        return PANAXML_Element::endElement();
     }
};

class PANAXML_CommandElement :
  public PANAXML_Element
{
  public:
     PANAXML_CommandElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("command", stack),
        m_code(0), m_appId(0), 
        m_pBit(0), m_eBit(0) {
     }
     virtual bool startElement(ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }
        if (alist != 0) {
            for (size_t i = 0; i < alist->getLength (); ++i) {
               if (! ACE_OS::strcmp(alist->getQName(i), "name")) {
                   m_name = alist->getValue(i);
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "code")) {
                   m_code = ACE_OS::atoi(alist->getValue(i));
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "application-id")) {
                   m_appId = ACE_OS::atoi(alist->getValue(i));
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "pbit")) {
                   m_pBit = ACE_OS::atoi(alist->getValue(i));
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "ebit")) {
                   m_eBit = ACE_OS::atoi(alist->getValue(i));
               }
            }
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Command [ name = %s, code = %d\n", 
	           m_name.c_str(), m_code));
#endif
        }
        else {
            ACE_DEBUG((LM_ERROR, 
               "Command code does not have attributes !!!\n"));
            throw;
        }

        // application id over-rides
        if (Parent()->Name() == std::string("base")) {
            m_appId = 0;
        }
        else if (Parent()->Name() == std::string("application")) {
            PANAXML_ApplicationElement *appElm = 
                (PANAXML_ApplicationElement*)Parent();
            m_appId = appElm->AppId();
        }
        else {
            ACE_DEBUG((LM_ERROR, 
                 "Command has an invalid parent !!!\n"));
            throw;
        }
        return true;
     }
     std::string &Name() {
        return m_name;
     }
     int Code() {
        return m_code;
     }
     int AppId() {
        return m_appId;
     }
     int pBit() {
        return m_pBit;
     }
     int eBit() {
        return m_eBit;
     }
     virtual bool endElement() {
        m_name = "";
        m_code = m_appId = m_pBit = m_eBit = 0;
        return PANAXML_Element::endElement();
     }

  private:
     std::string m_name;
     int m_code;
     int m_appId;
     int m_pBit;
     int m_eBit;
};

class PANAXML_RequestRulesElement :
  public PANAXML_Element
{
  public:
     PANAXML_RequestRulesElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("requestrules", stack),
        m_command(NULL) {
     }
     virtual bool startElement(ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }

        PANAXML_CommandElement *cmdElm = 
            (PANAXML_CommandElement*)Parent();

        m_command = new DiameterCommand;
        m_command->avp_f = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_FIXED_HEAD);
        m_command->avp_r = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_REQUIRED);
        m_command->avp_o = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_OPTIONAL);

        m_command->name = cmdElm->Name() + std::string("-Request");
        m_command->code = cmdElm->Code();
        m_command->appId = cmdElm->AppId();
        m_command->flags.p = cmdElm->pBit();
        m_command->flags.e = cmdElm->eBit();
        m_command->flags.r = 1;
        DiameterCommandList::instance()->add(m_command);

#if PANAXML_DEBUG
        ACE_DEBUG((LM_DEBUG, " Request [name = %s, code = %d, applid = %d]\n", 
	           m_command->name.c_str(), m_command->code, m_command->appId));
#endif
        return true;
     }
     virtual bool endElement() {
        m_command = NULL;
        return PANAXML_Element::endElement();
     }
     DiameterCommand *Cmd() {
        return m_command;
     }
 
  private:
     DiameterCommand *m_command;
};

class PANAXML_AnswerRulesElement :
  public PANAXML_Element
{
  public:
     PANAXML_AnswerRulesElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("answerrules", stack),
        m_command(NULL) {
     }
     virtual bool startElement(ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }

        m_command = new DiameterCommand;
        m_command->avp_f = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_FIXED_HEAD);
        m_command->avp_r = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_REQUIRED);
        m_command->avp_o = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_OPTIONAL);

        PANAXML_CommandElement *cmdElm = 
            (PANAXML_CommandElement*)Parent();

        m_command->name = cmdElm->Name() + std::string("-Answer");
        m_command->code = cmdElm->Code();
        m_command->appId = cmdElm->AppId();
        m_command->flags.p = cmdElm->pBit();
        m_command->flags.e = cmdElm->eBit();
        m_command->flags.r = 0;
        DiameterCommandList::instance()->add(m_command);

#if PANAXML_DEBUG
        ACE_DEBUG((LM_DEBUG, " Answer [name = %s, code = %d, applid = %d]\n", 
	           m_command->name.c_str(), m_command->code, m_command->appId));
#endif
        return true;
     }
     virtual bool endElement() {
        m_command = NULL;
        return PANAXML_Element::endElement();
     }
     DiameterCommand *Cmd() {
        return m_command;
     }
 
  private:
     DiameterCommand *m_command;
};

class PANAXML_TypedefElement : 
  public PANAXML_Element
{
  public:
     PANAXML_TypedefElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("typedefn", stack) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }
#if PANAXML_DEBUG
        if (alist != 0) {
            for (size_t i = 0; i < alist->getLength (); ++i) {
               ACE_DEBUG ((LM_INFO,
                          ACE_TEXT (" Type   [%s = \"%s\"]\n"),
               alist->getQName (i), alist->getValue (i)));
            }
        }
#endif
        return true;
     }
};

class PANAXML_AvpElement :
  public PANAXML_Element
{
  public:
     PANAXML_AvpElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("avp", stack),
        m_avp(NULL) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }
        if (alist != 0) {
            int code = 0, vendorId = 0;
            std::string name, mandatory;

            for (size_t i = 0; i < alist->getLength (); ++i) {
               if (! ACE_OS::strcmp(alist->getQName(i), "name")) {
                   name = alist->getValue(i);
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "code")) {
                   code = ACE_OS::atoi(alist->getValue(i));
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "mandatory")) {
                   mandatory = alist->getValue(i);
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "vendor-id")) {
                   vendorId = ACE_OS::atoi(alist->getValue(i));
               }
            }

            m_avp = new AAADictionaryEntry(code,
                                           name.c_str(),
                                           AAA_AVP_DATA_TYPE,
                                           vendorId,
                                           PANA_AVP_FLAG_NONE);
            m_avp->flags = PANA_AVP_FLAG_NONE;
            m_avp->flags |=
                (((mandatory == std::string("may")) ||
                  (mandatory == std::string("must"))) ?
                  PANA_AVP_FLAG_MANDATORY : 0);
            m_avp->flags |=
                ((vendorId == 0) ? 0 : PANA_AVP_FLAG_VENDOR_SPECIFIC);
            if (m_avp->avpCode != 0)  { // Do not add "AVP" AVP
                PANA_AvpList::instance()->add(m_avp);
            }
            else {
                delete m_avp;
                m_avp = NULL;
            }
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Avp [name = %s, code = %d]\n", 
                       name.c_str(), code));
#endif
        }
        return true;
     }
     virtual bool endElement() {
        m_avp = NULL;
        return PANAXML_Element::endElement();
     }
     AAADictionaryEntry *Avp() {
        return m_avp;
     }

  private:
     AAADictionaryEntry *m_avp;
};

class PANAXML_TypeElement :
  public PANAXML_Element
{
  public:
     PANAXML_TypeElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("type", stack) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }

        std::string tname;
        if (alist != 0) {
            for (size_t i = 0; i < alist->getLength (); ++i) {
               if (! ACE_OS::strcmp(alist->getQName(i), "type-name")) {
                   tname = alist->getValue(i);
                   break;
               }
            }
        }
        if (tname.length() == 0) {
            ACE_DEBUG((LM_ERROR, "AVP Type not specified\n"));
            throw;
        }
 
        ++++++++++++++++++++++++ TBD +++++++++++++++++++++
#if 0
        // check for validity of type
        DiameterAvpType *avpt = (DiameterAvpType*)DiameterAvpTypeList::instance()->search(tname.c_str());
        if (avpt == NULL) {
            ACE_DEBUG((LM_ERROR, 
                 "Unknown AVP type %s.\n", tname.c_str()));
            throw;
        }

        if (avpt->getType()) {
           PANAXML_AvpElement *avpElm = (PANAXML_AvpElement*)Parent();
           avpElm->Avp()->avpType = avpt->getType();
        }
#endif
        return true;
     }
     virtual bool endElement() {
        return PANAXML_Element::endElement();
     }
};

class PANAXML_GroupedElement :
  public PANAXML_Element
{
  public:
     PANAXML_GroupedElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("grouped", stack),
        m_grpAvp(NULL) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }

        PANAXML_AvpElement *avpElm = (PANAXML_AvpElement*)Parent();
        m_grpAvp = new DiameterGroupedAVP;
        m_grpAvp->code = avpElm->Avp()->avpCode;
        m_grpAvp->vendorId = avpElm->Avp()->vendorId;
        m_grpAvp->avp_f = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_FIXED_HEAD);
        m_grpAvp->avp_r = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_REQUIRED);
        m_grpAvp->avp_o = new DiameterQualifiedAvpList(AAA_PARSE_TYPE_OPTIONAL);

        DiameterGroupedAvpList::instance()->add(m_grpAvp);  // to be revisited
						       // after parsing
                                                       // all avps.
        avpElm->Avp()->avpType = AAA_AVP_GROUPED_TYPE;
#if PANAXML_DEBUG
        ACE_DEBUG((LM_DEBUG, " Grouped [code = %d]\n", 
                   m_grpAvp->code));
#endif
        return true;
     }
     DiameterGroupedAVP *Avp() {
        return m_grpAvp;
     }
     virtual bool endElement() {
        m_grpAvp = NULL;
        return PANAXML_Element::endElement();
     }

  private:
     DiameterGroupedAVP *m_grpAvp;
};

class PANAXML_PositionElement :
  public PANAXML_Element
{
  public:
     PANAXML_PositionElement(char *name,
                            PANAXML_ElementStack &stack) :
        PANAXML_Element(name, stack),
        m_qAvpList(NULL) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }
        if (Parent()->Name() == std::string("requestrules")) {
            PANAXML_RequestRulesElement *reqElm = 
                (PANAXML_RequestRulesElement*)(Parent());
            m_qAvpList = ResolveAvpList(reqElm->Cmd(), Name());
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Request [%s] definition\n", 
                       Name().c_str()));
#endif
        }
        else if (Parent()->Name() == std::string("answerrules")) {
            PANAXML_AnswerRulesElement *ansrElm = 
                (PANAXML_AnswerRulesElement*)(Parent());
            m_qAvpList = ResolveAvpList(ansrElm->Cmd(), Name());
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Answer [%s] definition\n", 
                       Name().c_str()));
#endif
        }
        else if (Parent()->Name() == std::string("grouped")) {
            PANAXML_GroupedElement *grpElm = (PANAXML_GroupedElement*)Parent();
            m_qAvpList = ResolveAvpList(grpElm->Avp(), Name());
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Grouped [%s] definition\n", 
                       Name().c_str()));
#endif
        }
        else {
            IsSkipped() = true;
#if PANAXML_DEBUG
            ACE_DEBUG((LM_DEBUG, " Skipped [%s] definition\n", 
                       Name().c_str()));
#endif
        }
        return true;
     }
     virtual bool endElement() {
        IsSkipped() = false;
        m_qAvpList = NULL;
        return PANAXML_Element::endElement();
     }
     DiameterQualifiedAvpList* AvpList() {
        return m_qAvpList;
     }
     DiameterQualifiedAvpList *ResolveAvpList(DiameterDictionary *dict, 
                                         std::string &position) {
        if (dict == NULL) {
            ACE_DEBUG((LM_ERROR, "Command not allocated !!!\n"));
            throw;
        }
        if (position == std::string("fixed")) {
            return dict->avp_f;
        }
        else if (position == std::string("required")) {
            return dict->avp_r;
        }
        else if (position == std::string("optional")) {
            return dict->avp_o;
        }
        else {
            ACE_DEBUG((LM_ERROR, "Grouped AVP not allocated !!!\n"));
            throw;
        }
     }

  private:
     DiameterQualifiedAvpList* m_qAvpList;
};

class PANAXML_FixedElement : 
  public PANAXML_PositionElement
{
  public:
     PANAXML_FixedElement(PANAXML_ElementStack &stack) :
        PANAXML_PositionElement("fixed", stack) {
     }
};

class PANAXML_RequiredElement : 
  public PANAXML_PositionElement
{
  public:
     PANAXML_RequiredElement(PANAXML_ElementStack &stack) :
        PANAXML_PositionElement("required", stack) {
     }
};

class PANAXML_OptionalElement : 
  public PANAXML_PositionElement
{
  public:
     PANAXML_OptionalElement(PANAXML_ElementStack &stack) :
        PANAXML_PositionElement("optional", stack) {
     }
};

class PANAXML_AvpRuleElement :
  public PANAXML_Element
{
  public:
     PANAXML_AvpRuleElement(PANAXML_ElementStack &stack) :
        PANAXML_Element("avprule", stack) {
     }
     virtual bool startElement (ACEXML_Attributes *alist) {
        if (! PANAXML_Element::startElement(alist)) {
            return false;
        }

        PANAXML_PositionElement *posElm = (PANAXML_PositionElement*)Parent();
        if (posElm->IsSkipped()) {
            return true;
        }

        AAAQualifiedAVP *qavp;
        AAADictionaryEntry *avp;

        std::string avpName, sMax = "", sMin = "";
        int vendorId = 0;
        if (alist != 0) {
            for (size_t i = 0; i < alist->getLength (); ++i) {
               if (! ACE_OS::strcmp(alist->getQName(i), "name")) {
                   avpName = alist->getValue(i);
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "vendor-id")) {
                   vendorId = ACE_OS::atoi(alist->getValue(i));
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "minimum")) {
                   sMin = alist->getValue(i);
               }
               else if (! ACE_OS::strcmp(alist->getQName(i), "maximum")) {
                   sMax = alist->getValue(i);
               }
            }
        }
        else {
            AAA_LOG(LM_ERROR, "AVP rule does not have attributes\n");
            throw;
        }

        if ((avp = DiameterAvpList::instance()->search(avpName)) == NULL) {
            AAA_LOG(LM_ERROR, "*** Cannot find AVP named %s ***\n\
  If %s is included inside a grouped avp, \n\
  make sure it's <avp> definition comes before \n\
  the <grouped> definition using it. If it is\n\
  not in a group, make sure it spelled properly\n\
  and there are no white-spaces.\n", 
            avpName.c_str(), avpName.c_str());
            throw;
        }

        int min = (sMin == std::string("")) ? 0 :
                   ACE_OS::atoi(sMin.c_str()); 
        int max = ((sMax == std::string("none")) ||
                   (sMax == std::string(""))) ? 
		   AAA_QUALIFIER_INFINITY : ACE_OS::atoi(sMax.c_str());

        qavp = new AAAQualifiedAVP;
        qavp->avp = avp; 
        qavp->qual.min = min; 
        qavp->qual.max = max; 
        posElm->AvpList()->add(qavp);
#if PANAXML_DEBUG
        ACE_DEBUG((LM_DEBUG, " Avp Rule [name = %s]\n", 
                  avpName.c_str()));
#endif
        return true;
     }
};

typedef std::map<std::string, PANAXML_Element*> PANAXML_ElementMap;
typedef std::pair<std::string, PANAXML_Element*> PANAXML_ElementPair;

class PANAXML_ParsingTables
{
  public:
     typedef enum {
         NUM_PARSING_PASSES = 2,
     };

  public:
     PANAXML_ParsingTables() :
        m_passNum(0),
        m_currentElement(NULL) {

        PANAXML_Element *e = NULL;

        // populate first pass table
        e = new PANAXML_DictionaryElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_TypedefElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_AvpElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_TypeElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_GroupedElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_FixedElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_RequiredElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_OptionalElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_AvpRuleElement(m_callStack);
        m_elementMaps[0].insert(PANAXML_ElementPair(e->Name(), e));

        // populate second pass table
        e = new PANAXML_DictionaryElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_CommandElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_RequestRulesElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_AnswerRulesElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_FixedElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_RequiredElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_OptionalElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));

        e = new PANAXML_AvpRuleElement(m_callStack);
        m_elementMaps[1].insert(PANAXML_ElementPair(e->Name(), e));
     }
     virtual ~PANAXML_ParsingTables() {
        for (int x = 0; x < NUM_PARSING_PASSES; x ++) {
            PANAXML_ElementMap::iterator i;
            for (i = m_elementMaps[x].begin(); 
                 i != m_elementMaps[x].end(); i++) {
                delete i->second;
            }
        }
     }

     // Interface to SAX handlers
     virtual void startDocument() {
     	m_passNum ++;
     }
     virtual void startElement(const ACEXML_Char *namespaceURI,
                               const ACEXML_Char *localName,
                               const ACEXML_Char *qName,
                               ACEXML_Attributes *atts) {
         PANAXML_ElementMap::iterator i = m_elementMaps[m_passNum-1].find(std::string(qName));
         if (i != m_elementMaps[m_passNum-1].end()) {
             i->second->startElement(atts);
             m_currentElement = i->second;
         }
     }
     virtual void characters (const ACEXML_Char *ch,
                              int start,
                              int length) {
         if (m_currentElement) {
             m_currentElement->characters(ch, start, length);
         }
     }
     virtual void endElement (const ACEXML_Char *namespaceURI,
                              const ACEXML_Char *localName,
                              const ACEXML_Char *qName) {
         PANAXML_ElementMap::iterator i = m_elementMaps[m_passNum-1].find(std::string(qName));
         if (i != m_elementMaps[m_passNum-1].end()) {
             i->second->endElement();
         }
         m_currentElement = NULL;
     }
     virtual void endDocument() {
     }

  private:
     ACE_UINT32 m_passNum;
     PANAXML_Element *m_currentElement;
     PANAXML_ElementMap m_elementMaps[NUM_PARSING_PASSES]; // Maps for each pass
     PANAXML_ElementStack m_callStack;
}; 

class PANAXML_SAXHandler : 
    public ACEXML_DefaultHandler
{
  public:
     PANAXML_SAXHandler (const ACEXML_Char* name) :
        m_errorCount(0),
        m_fatalError(false),
        m_fileName(ACE::strnew (name)), 
        m_locator(NULL) {
     }
     virtual ~PANAXML_SAXHandler (void) {
        delete [] m_fileName;
     }

     // Methods inherit from ACEXML_ContentHandler.
     virtual void characters (const ACEXML_Char *ch,
                              int start,
                              int length ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parsingTables.characters(ch, start, length);
     }
     virtual void endDocument (ACEXML_ENV_SINGLE_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parsingTables.endDocument();
     }
     virtual void endElement (const ACEXML_Char *namespaceURI,
                              const ACEXML_Char *localName,
                              const ACEXML_Char *qName ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parsingTables.endElement(namespaceURI, localName, qName);
     }
     virtual void endPrefixMapping (const ACEXML_Char *prefix ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }  
     virtual void ignorableWhitespace (const ACEXML_Char *ch,
                                       int start,
                                       int length ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void processingInstruction (const ACEXML_Char *target,
                                         const ACEXML_Char *data ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void setDocumentLocator (ACEXML_Locator *locator) {
         m_locator = locator;
     }
     virtual void skippedEntity (const ACEXML_Char *name ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void startDocument (ACEXML_ENV_SINGLE_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parsingTables.startDocument();
     }
     virtual void startElement (const ACEXML_Char *namespaceURI,
                                const ACEXML_Char *localName,
                                const ACEXML_Char *qName,
                                ACEXML_Attributes *atts ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) { 
         m_parsingTables.startElement(namespaceURI, localName, qName, atts);
     }
     virtual void startPrefixMapping (const ACEXML_Char *prefix,
                                      const ACEXML_Char *uri ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }

     // *** Methods inherit from ACEXML_DTDHandler.
     virtual void notationDecl (const ACEXML_Char *name,
                                const ACEXML_Char *publicId,
                                const ACEXML_Char *systemId ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void unparsedEntityDecl (const ACEXML_Char *name,
                                      const ACEXML_Char *publicId,
                                      const ACEXML_Char *systemId,
                                      const ACEXML_Char *notationName ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }

     // Methods inherit from ACEXML_EnitityResolver.
     virtual ACEXML_InputSource *resolveEntity (const ACEXML_Char *publicId,
                                                const ACEXML_Char *systemId ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         return (NULL);
     }

     // Methods inherit from ACEXML_ErrorHandler.
     virtual void error (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         ACE_DEBUG ((LM_INFO, "Error %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
         m_errorCount ++;
     }
     virtual void fatalError (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         ACE_DEBUG ((LM_INFO, "Fatal error %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
         m_fatalError = true;
     }
     virtual void warning (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         ACE_DEBUG ((LM_INFO, "Warning %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
     }

     // local methods
     int ErrorCount() {
         return m_errorCount;
     }
     bool FatalError() {
         return m_fatalError;
     }

  private:
     int m_errorCount;
     bool m_fatalError;
     ACEXML_Char* m_fileName;
     ACEXML_Locator* m_locator;
     PANAXML_ParsingTables m_parsingTables;
};

void PANA_LoadXMLDictionary(char* xmlFile)
{
   ACEXML_FileCharStream *fstm = NULL;
   try {
       if (xmlFile == NULL) {
           ACE_DEBUG((LM_ERROR, 
                      "No dictionary file specified\n"));
           throw;
       }

       fstm = new ACEXML_FileCharStream;
       if (fstm == NULL) {
           ACE_DEBUG((LM_ERROR, 
                      "Allocation failure\n"));
           throw;
       }
   
       if (fstm->open (xmlFile) != 0) {
           ACE_DEBUG((LM_ERROR, 
                      "Failed to open XML file: %s\n", xmlFile));
           throw;
       }

       auto_ptr<ACEXML_DefaultHandler> handler
           (new PANAXML_SAXHandler(xmlFile));
       if (handler.get() == NULL) {
           ACE_DEBUG((LM_ERROR, 
                      "Allocation failure\n"));
           throw;
      }

      ACEXML_Parser parser;
      ACEXML_InputSource input (fstm);

      parser.setContentHandler (handler.get());
      parser.setDTDHandler (handler.get());
      parser.setErrorHandler (handler.get());
      parser.setEntityResolver (handler.get());

      try {
          for (int passes = 0; 
              passes < PANAXML_ParsingTables::NUM_PARSING_PASSES;
              passes ++) {
              parser.parse (&input ACEXML_ENV_ARG_NOT_USED);
          }
      }
      catch (ACEXML_SAXException &ex) {
          ex.print();
          ACE_DEBUG ((LM_ERROR, ACE_TEXT ("Exception occurred. Exiting...\n")));
          throw;
      }

      PANAXML_SAXHandler *h = static_cast<PANAXML_SAXHandler*>(handler.get());
      if (h->FatalError() || (h->ErrorCount() > 0)) {
          ACE_DEBUG ((LM_ERROR, ACE_TEXT ("Exception occurred. Exiting...\n")));
          throw;
      }       
   }
   catch (...) {
      exit(1);
   }
}
