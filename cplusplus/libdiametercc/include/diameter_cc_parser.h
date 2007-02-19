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

/* $Id: diameter_cc_parser.hxx $ */

#ifndef __DIAMETER_CC_PARSER_H__
#define __DIAMETER_CC_PARSER_H__

#include <utility>
#include <vector>
#include <cmath>
#include "diameter_parser.h"

const diameter_unsigned32_t CCApplicationId = 4;
const AAACommandCode CC_CommandCode = 272;

enum
  {
    CC_TYPE_INITIAL_REQUEST = 1,
    CC_TYPE_UPDATE_REQUEST,
    CC_TYPE_TERMINATION_REQUEST,
    CC_TYPE_EVENT_REQUEST
  };

enum
  {
    CC_ACTION_DIRECT_DEBITING = 0,
    CC_ACTION_REFUND_ACCOUNT,
    CC_ACTION_CHECK_BALANCE,
    CC_ACTION_PRICE_ENQUIRY
  };

template <typename T>
class CCDiameterNumberScholarAttribute :
  public DiameterScholarAttribute<T>
{
public:
  CCDiameterNumberScholarAttribute() {
  }
  CCDiameterNumberScholarAttribute(T val) :
    DiameterScholarAttribute<T>(val) {
  }
  
  virtual T& operator=(T v) {
    
    return DiameterScholarAttribute<T>::operator=(v);
  }
  
  virtual bool operator==(const CCDiameterNumberScholarAttribute<T>& b) const
  { 
    bool equal = false;
    if(AAAScholarAttribute<T>::isSet && b.isSet)
      {
        equal = AAAScholarAttribute<T>::value == b.value;
      }
    return equal;
  }

  virtual bool operator <(const CCDiameterNumberScholarAttribute<T>& b) const
  {
    bool less = false;
    if(AAAScholarAttribute<T>::isSet && b.isSet)
      {
        less = AAAScholarAttribute<T>::value < b.value;
      }
    return less;
  }

  virtual bool operator > (const diameter_unsigned64_t & b) const
  {
    bool greater = false;
    if(AAAScholarAttribute<T>::isSet)
      {
        greater = AAAScholarAttribute<T>::value > b;
      }
    return greater;
  }

  virtual CCDiameterNumberScholarAttribute<T> operator - (const CCDiameterNumberScholarAttribute<T>& b) const
  {
    CCDiameterNumberScholarAttribute<T> c;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = AAAScholarAttribute<T>::value;
      }
    if(c.isSet && b.isSet)
      {
        c.value = c.value - b.value;
      }
       return c;
  }

  virtual CCDiameterNumberScholarAttribute<T> operator + (const CCDiameterNumberScholarAttribute<T>& b) const
  {
    CCDiameterNumberScholarAttribute<T> c;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = AAAScholarAttribute<T>::value;
      }
    if(c.isSet && b.isSet)
      {
        c.value = c.value + b.value;
      }
    return c;  
  }

  virtual long double exponentValue (diameter_integer64_t base) const
  {
    long double c = 1;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = pow(base,AAAScholarAttribute<T>::value);
      }
    return c;
  }

  virtual long double operator * (const long double& b) const
  {
    long double c =0;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = AAAScholarAttribute<T>::value *b;
      }
    return c;
  }
};

template <typename T>
class CCDiameterStringScholarAttribute :
  public DiameterScholarAttribute<T>
{
public:
  CCDiameterStringScholarAttribute() {
  }
  CCDiameterStringScholarAttribute(T val) :
    DiameterScholarAttribute<T>(val) {
  }
  
  virtual T& operator=(T v) {
    return DiameterScholarAttribute<T>::operator=(v);
  }
  
  virtual bool operator==(const CCDiameterStringScholarAttribute<T>& b) const
  { 
    bool equal = false;
    if(AAAScholarAttribute<T>::isSet && b.isSet)
      {
        equal = AAAScholarAttribute<T>::value == b.value;
      }
    return equal;
  }

  virtual bool operator <(const CCDiameterStringScholarAttribute<T>& b) const
  {
    bool less = false;
    if(AAAScholarAttribute<T>::isSet && b.isSet)
      {
        less = AAAScholarAttribute<T>::value < b.value;
      }
    return less;
  }
};

template <typename T>
class CCDiameterGroupedScholarAttribute :
  public DiameterGroupedScholarAttribute<T>
{
public:
  CCDiameterGroupedScholarAttribute() {
  }
  CCDiameterGroupedScholarAttribute(T val) :
    DiameterGroupedScholarAttribute<T>(val) {
  }
  
  virtual T& operator=(T v) {
    return DiameterGroupedScholarAttribute<T>::operator=(v);
  }

  virtual bool operator > (const diameter_unsigned64_t & b) const
  {
    bool greater = false;
    if(AAAScholarAttribute<T>::isSet)
      {
        greater = AAAScholarAttribute<T>::value > b;
      }
    return greater;
  }

  virtual CCDiameterGroupedScholarAttribute<T> operator - (const CCDiameterGroupedScholarAttribute<T>& b) const
  {
    CCDiameterGroupedScholarAttribute<T> c;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = AAAScholarAttribute<T>::value;
      }
    if(c.isSet && b.isSet)
      {
        c.value = c.value - b.value;
      }
       return c;
  }

  virtual CCDiameterGroupedScholarAttribute<T> operator + (const CCDiameterGroupedScholarAttribute<T>& b) const
  {
    CCDiameterGroupedScholarAttribute<T> c;
    if(AAAScholarAttribute<T>::isSet)
      {
        c = AAAScholarAttribute<T>::value;
      }
    if(c.isSet && b.isSet)
      {
        c.value = c.value + b.value;
      }
    return c;  
  }
};

class subscriptionId_t
{
 public:

  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (SubscriptionIdType.IsSet())
      {
        c = cm.acquire("Subscription-Id-Type");
        SubscriptionIdType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    if (SubscriptionIdData.IsSet())
      {
        c = cm.acquire("Subscription-Id-Data");
        SubscriptionIdData.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
  }

  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
	  
    if ((c = cl.search("Subscription-Id-Type")))
      {
        SubscriptionIdType.CopyFrom(*c);
      }
    if ((c = cl.search("Subscription-Id-Data")))
      {
        SubscriptionIdData.CopyFrom(*c);
      } 
  }

  bool operator==(subscriptionId_t& a) 
  { 
    bool equal;
    equal = ( SubscriptionIdType == a.SubscriptionIdType() && 
              SubscriptionIdData == a.SubscriptionIdData());
    return equal;
  }

  bool operator <(const subscriptionId_t& b) const
  { 
    bool less = false;

    if(SubscriptionIdType < b.SubscriptionIdType)
      {
        less=true;
      }
    else if(SubscriptionIdType == b.SubscriptionIdType)
      {
        less = SubscriptionIdData < b.SubscriptionIdData;
      }
    else
      {
        less=false;
      }
    return less;
  }
  subscriptionId_t& operator = (const subscriptionId_t& value)
  {
    SubscriptionIdType = value.SubscriptionIdType;
    SubscriptionIdData = value.SubscriptionIdData;
    return *this;
  }

  subscriptionId_t(){}
  subscriptionId_t(diameter_enumerated_t subscriptionIdType,
                   diameter_utf8string_t subscriptionIdData)
    :SubscriptionIdType(subscriptionIdType), SubscriptionIdData(subscriptionIdData)
  {
  }

  // Required AVPs
  CCDiameterNumberScholarAttribute<diameter_enumerated_t> SubscriptionIdType;
  CCDiameterStringScholarAttribute<diameter_utf8string_t> SubscriptionIdData;
};


/// Definition for Proxy-Info AVP internal structure.
class proxyInfo_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (ProxyHost.IsSet())
      {
        c = cm.acquire("Proxy-Host");
        ProxyHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
        cl.add(c);
      }
    if (ProxyState.IsSet())
      {
        c = cm.acquire("Proxy-State");
        ProxyState.CopyTo(*c, AAA_AVP_STRING_TYPE);
        cl.add(c);
      }
    if (Avp.IsSet())
      {
        c = cm.acquire("AVP");
        Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Proxy-Host")))
      {
        ProxyHost.CopyFrom(*c);
      }
    if ((c = cl.search("Proxy-State")))
      {
        ProxyState.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
        Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_identity_t> ProxyHost;
  DiameterScholarAttribute<diameter_octetstring_t> ProxyState;
  // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

/// Definition for Unit-Value AVP internal structure.
class unitValue_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (ValueDigits.IsSet())
      {
        c = cm.acquire("Value-Digits");
        ValueDigits.CopyTo(*c, AAA_AVP_INTEGER64_TYPE);
        cl.add(c);
      }
    if (Exponent.IsSet())
      {
        c = cm.acquire("Exponent");
        Exponent.CopyTo(*c, AAA_AVP_INTEGER32_TYPE);
        cl.add(c);
      }
  }

  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Value-Digits")))
      {
        ValueDigits.CopyFrom(*c);
      }
    if ((c = cl.search("Exponent")))
      {
        Exponent.CopyFrom(*c);
      }
  }

  unitValue_t(){}
  unitValue_t(const diameter_integer64_t& valueDigits,
            const diameter_integer32_t& exponent)
    :ValueDigits(valueDigits),
     Exponent(exponent)
  {}

  unitValue_t operator - (const unitValue_t& b) const
  {
    unitValue_t c;
    long double ldValue1 = ValueDigits * Exponent.exponentValue(10);
    long double ldValue2 = b.ValueDigits * b.Exponent.exponentValue(10);
    long double ldValue = ldValue1 - ldValue2;
    c.Exponent = (diameter_integer32_t)log10(ldValue);
    if(c.Exponent() < 0)
      {
        c.ValueDigits = (diameter_integer64_t) (ldValue * pow(10, -c.Exponent()));
      }
    else
      {
        c.ValueDigits = (diameter_integer64_t) ldValue;
        c.Exponent() = 0;
      }
    return c;
  }

  unitValue_t operator + (const unitValue_t& b) const
  {
    unitValue_t c;
    long double ldValue1 = ValueDigits * Exponent.exponentValue(10);
    long double ldValue2 = b.ValueDigits * b.Exponent.exponentValue(10);
    long double ldValue = ldValue1 + ldValue2;
    c.Exponent = (diameter_integer32_t)log10(ldValue);
    if(c.Exponent() < 0)
      {
        c.ValueDigits = (diameter_integer64_t) (ldValue * pow(10, -c.Exponent()));
      }
    else
      {
        c.ValueDigits = (diameter_integer64_t) ldValue;
        c.Exponent() = 0;
      }
    return c;
  }

  bool operator > ( const diameter_unsigned64_t& b) const
  {
    bool greater = false;
    greater = (ValueDigits * Exponent.exponentValue(10)) > b;
    return greater;
  }

  unitValue_t& operator = (const unitValue_t& value)
  {
    ValueDigits = value.ValueDigits;
    Exponent = value.Exponent;
    return *this;
  }
  void Clear()
  {
    ValueDigits.Clear();
    Exponent.Clear();
  }
  
  bool IsSet()
  {
    if(ValueDigits.IsSet() || Exponent.IsSet())
      {
        return true;
      }
    return false;
  }
  // Required AVPs
  CCDiameterNumberScholarAttribute<diameter_integer64_t> ValueDigits;

  // Optional AVPs
  CCDiameterNumberScholarAttribute<diameter_integer32_t> Exponent;
 
};

class costInformation_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (UnitValue.IsSet())
      {
        c = cm.acquire("Unit-Value");
        UnitValue.CopyTo(*c);
        cl.add(c);
      }
    if (CurrencyCode.IsSet())
      {
        c = cm.acquire("Currency-Code");
        CurrencyCode.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
    if (CostUnit.IsSet())
      {
        c = cm.acquire("Cost-Unit");
        CostUnit.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Unit-Value")))
      {
        UnitValue.CopyFrom(*c);
      }
    if ((c = cl.search("Current-Code")))
      {
        CurrencyCode.CopyFrom(*c);
      }
    if ((c = cl.search("Cost-Unit")))
      {
        CostUnit.CopyFrom(*c);
      }
  }
  // Required AVPs
  DiameterGroupedScholarAttribute<unitValue_t> UnitValue;
  DiameterScholarAttribute<diameter_unsigned32_t> CurrencyCode;

  // Optional AVPs
  DiameterScholarAttribute<diameter_utf8string_t> CostUnit;
};

/// Definition for Money AVP internal structure.
class ccMoney_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (UnitValue.IsSet())
      {
        c = cm.acquire("Unit-Value");
        UnitValue.CopyTo(*c);
        cl.add(c);
      }
    if (CurrencyCode.IsSet())
      {
        c = cm.acquire("Currency-Code");
        CurrencyCode.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Unit-Value")))
      {
        UnitValue.CopyFrom(*c);
      }
    if ((c = cl.search("Currency-Code")))
      {
        CurrencyCode.CopyFrom(*c);
      }
  }
  ccMoney_t(){}
  ccMoney_t(const unitValue_t& unitValue,
           const diameter_unsigned32_t& currencyCode)
    :CurrencyCode(currencyCode)
  {
    UnitValue=unitValue;
  }

  ccMoney_t operator - (const ccMoney_t& b) const
  {
    ccMoney_t c;
    c.UnitValue = UnitValue - b.UnitValue;
    c.CurrencyCode = CurrencyCode;
    return c;
  }

  ccMoney_t operator + (const ccMoney_t& b) const
  {
    ccMoney_t c;
    c.UnitValue = UnitValue + b.UnitValue;
    c.CurrencyCode = CurrencyCode;
    return c;
  }

  bool operator > (const diameter_unsigned64_t& b) const
  {
    if( UnitValue > b)
      {
        return true;
      }
    return false;
  }

  ccMoney_t& operator = (const ccMoney_t& value)
  {
    UnitValue = value.UnitValue;
    CurrencyCode = value.CurrencyCode;
    return *this;
  }

  void Clear()
  {
    UnitValue.Clear();
    CurrencyCode.Clear();
  }

  bool IsSet()
  {
    if (UnitValue.IsSet())
    {
      return true;
    }
    return false;
  }

  // Required AVPs
  CCDiameterGroupedScholarAttribute<unitValue_t> UnitValue;
  
  // Optional AVPs
  CCDiameterNumberScholarAttribute<diameter_unsigned32_t> CurrencyCode;
 
};

class requestedServiceUnit_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (CCTime.IsSet())
      {
        c = cm.acquire("CC-Time");
        CCTime.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
    if (CCMoney.IsSet())
      {
        c = cm.acquire("CC-Money");
        CCMoney.CopyTo(*c);
        cl.add(c);
      }
    if (CCTotalOctets.IsSet())
      {
        c = cm.acquire("CC-Total-Octets");
        CCTotalOctets.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
        cl.add(c);
      }
    if (CCInputOctets.IsSet())
      {
        c = cm.acquire("CC-INPUT-Octets");
        CCInputOctets.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
        cl.add(c);
      }
    if (CCOutputOctets.IsSet())
      {
        c = cm.acquire("CC-OUTPUT-Octets");
        CCOutputOctets.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
        cl.add(c);
      }
    if (CCServiceSpecificUnits.IsSet())
      {
        c = cm.acquire("Service-Specific-Units");
        CCServiceSpecificUnits.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
        cl.add(c);
      }
    if (Avp.IsSet())
      {
        c = cm.acquire("AVP");
        Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("CC-Time")))
      {
        CCTime.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Money")))
      {
        CCMoney.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Total-Octets")))
      {
        CCTotalOctets.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Input-Octets")))
      {
        CCInputOctets.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Output-Octets")))
      {
        CCOutputOctets.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Service-Specific-Units")))
      {
        CCServiceSpecificUnits.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
        Avp.CopyFrom(*c);
      }
  }	  
  requestedServiceUnit_t(){}
  requestedServiceUnit_t(const diameter_unsigned32_t& ccTime = 0,
                      const ccMoney_t& ccMoney,
                      const diameter_unsigned64_t& ccTotalOctets = 0,
                      const diameter_unsigned64_t& ccInputOctets = 0,
                      const diameter_unsigned64_t& ccOutputOctets = 0,
                      const diameter_unsigned64_t& ccServiceSpecificUnits =0
                         )
    :CCTime(ccTime),
     CCTotalOctets(ccTotalOctets),
     CCInputOctets(ccInputOctets),
     CCOutputOctets(ccOutputOctets),
     CCServiceSpecificUnits(ccServiceSpecificUnits)
  {
    CCMoney =ccMoney;
  }

  bool operator > (const diameter_unsigned64_t& b) const
  {
    if (CCTime > b ||
        CCMoney > b ||
        CCTotalOctets > b ||
        CCInputOctets > b ||
        CCOutputOctets > b ||
        CCServiceSpecificUnits > b)
      {
        return true;
      }
    else
      {
        return false;
      }
  }

  requestedServiceUnit_t operator - (const requestedServiceUnit_t& b) const
  {
    requestedServiceUnit_t c;
   
    c.CCTime = CCTime - b.CCTime;
    c.CCMoney = CCMoney - b.CCMoney;
    c.CCTotalOctets = CCTotalOctets - b.CCTotalOctets;
    c.CCInputOctets = CCInputOctets - b.CCInputOctets;
    c.CCOutputOctets = CCOutputOctets - b.CCOutputOctets;
    c.CCServiceSpecificUnits = CCServiceSpecificUnits -  b.CCServiceSpecificUnits;
    return c;
  }

  requestedServiceUnit_t operator + (const requestedServiceUnit_t& b) const
  {
    requestedServiceUnit_t c;

    c.CCTime = CCTime + b.CCTime;
    c.CCMoney = CCMoney + b.CCMoney;
    c.CCTotalOctets = CCTotalOctets + b.CCTotalOctets;
    c.CCInputOctets = CCInputOctets +  b.CCInputOctets;
    c.CCOutputOctets = CCOutputOctets + b.CCOutputOctets;
    c.CCServiceSpecificUnits = CCServiceSpecificUnits + b.CCServiceSpecificUnits;
    return c;
  }

  requestedServiceUnit_t& operator = (const requestedServiceUnit_t& value)
  {
    CCTime = value.CCTime;
    CCMoney =  value.CCMoney;
    CCTotalOctets =  value.CCTotalOctets;
    CCInputOctets =  value.CCInputOctets;
    CCOutputOctets =  value.CCOutputOctets;
    CCServiceSpecificUnits =  value.CCServiceSpecificUnits;
    return *this;
  }

  void Clear()
  {
    CCTime.Clear();
    CCMoney.Clear();
    CCTotalOctets.Clear();
    CCInputOctets.Clear();
    CCOutputOctets.Clear();
    CCServiceSpecificUnits.Clear();
  }

  bool IsSet()
  {
    if(CCTime.IsSet() ||
       CCMoney.IsSet() ||
       CCTotalOctets.IsSet() ||
       CCInputOctets.IsSet() ||
       CCServiceSpecificUnits.IsSet())
      return true;
    return false;
  }

  CCDiameterNumberScholarAttribute<diameter_unsigned32_t> CCTime;
  CCDiameterGroupedScholarAttribute<ccMoney_t> CCMoney;
  CCDiameterNumberScholarAttribute<diameter_unsigned64_t> CCTotalOctets;
  CCDiameterNumberScholarAttribute<diameter_unsigned64_t> CCInputOctets;
  CCDiameterNumberScholarAttribute<diameter_unsigned64_t> CCOutputOctets;
  CCDiameterNumberScholarAttribute<diameter_unsigned64_t> CCServiceSpecificUnits;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

class grantedServiceUnit_t: public requestedServiceUnit_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (TariffTimeChange.IsSet())
      {
        c = cm.acquire("Tariff-Time-Change");
        TariffTimeChange.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    requestedServiceUnit_t::CopyTo(cl);
  }
  
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Tariff-Time-Change")))
      {
        TariffTimeChange.CopyFrom(*c);
      }
    requestedServiceUnit_t::CopyFrom(cl);
  }

  grantedServiceUnit_t& operator = (const requestedServiceUnit_t& value)
  {
    requestedServiceUnit_t::operator = (value);
    return *this;
  }

  grantedServiceUnit_t(){}
  grantedServiceUnit_t(const diameter_time_t& tariffTimeChange = 0,
                    const diameter_unsigned32_t& ccTime = 0,
                    const ccMoney_t& ccMoney,
                    const diameter_unsigned64_t& ccTotalOctets = 0,
                    const diameter_unsigned64_t& ccInputOctets = 0,
                    const diameter_unsigned64_t& ccOutputOctets = 0,
                    const diameter_unsigned64_t& ccServiceSpecificUnits =0
                       )    
    : requestedServiceUnit_t(ccTime,
                             ccMoney,
                             ccTotalOctets,
                             ccInputOctets,
                             ccOutputOctets,
                             ccServiceSpecificUnits),
      TariffTimeChange(tariffTimeChange)
  {
  }
  grantedServiceUnit_t(const requestedServiceUnit_t& requestedServiceUnit)
    :requestedServiceUnit_t(requestedServiceUnit)
  {}

  CCDiameterNumberScholarAttribute<diameter_enumerated_t> TariffTimeChange;
};

class usedServiceUnit_t: public requestedServiceUnit_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (TariffChangeUsage.IsSet())
      {
        c = cm.acquire("Tariff-Change-Usage");
        TariffChangeUsage.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    requestedServiceUnit_t::CopyTo(cl);
  }
  
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Tariff-Change-Usage")))
      {
        TariffChangeUsage.CopyFrom(*c);
      }
    requestedServiceUnit_t::CopyFrom(cl);
  }
  usedServiceUnit_t(){}
  usedServiceUnit_t(const diameter_enumerated_t& tariffChangeUsage = 0,
                  const diameter_unsigned32_t& ccTime = 0,
                  const ccMoney_t& ccMoney,
                  const diameter_unsigned64_t& ccTotalOctets = 0,
                  const diameter_unsigned64_t& ccInputOctets = 0,
                  const diameter_unsigned64_t& ccOutputOctets = 0,
                  const diameter_unsigned64_t& ccServiceSpecificUnits =0
                )    
    : requestedServiceUnit_t(ccTime,
                             ccMoney,
                             ccTotalOctets,
                             ccInputOctets,
                             ccOutputOctets,
                             ccServiceSpecificUnits),
      TariffChangeUsage(tariffChangeUsage)
  {
  }

  CCDiameterNumberScholarAttribute<diameter_enumerated_t> TariffChangeUsage;
};

class gsuPoolReference_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (GSUPoolIdentifier.IsSet())
      {
        c = cm.acquire("G-S-U-Pool-Identifier");
        GSUPoolIdentifier.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
    if (CCUnitType.IsSet())
      {
        c = cm.acquire("CC-Unit-Type");
        CCUnitType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    if (UnitValue.IsSet())
      {
        c = cm.acquire("Unit-Value");
        UnitValue.CopyTo(*c);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("G-S-U-Pool-Identifier")))
      {
        GSUPoolIdentifier.CopyFrom(*c);
      }
    if ((c = cl.search("CC-Unit-Type")))
      {
        CCUnitType.CopyFrom(*c);
      }
    if ((c = cl.search("Unit-Value")))
      {
        UnitValue.CopyFrom(*c);
      }
  }

  DiameterScholarAttribute<diameter_unsigned32_t> GSUPoolIdentifier;
  DiameterScholarAttribute<diameter_enumerated_t> CCUnitType;
  DiameterGroupedScholarAttribute<unitValue_t> UnitValue;
};

class redirectServer_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (RedirectAddressType.IsSet())
      {
        c = cm.acquire("Redirect-Address-Type");
        RedirectAddressType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    if (RedirectServerAddress.IsSet())
      {
        c = cm.acquire("Redirect-Server-Address");
        RedirectServerAddress.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Redirect-Address-Type")))
      {
        RedirectAddressType.CopyFrom(*c);
      }
    if ((c = cl.search("Redirect-Server-Address")))
      {
        RedirectServerAddress.CopyFrom(*c);
      }
  }
  DiameterScholarAttribute<diameter_enumerated_t> RedirectAddressType;
  DiameterScholarAttribute<diameter_utf8string_t> RedirectServerAddress;
};

class finalUnitIndication_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (FinalUnitAction.IsSet())
      {
        c = cm.acquire("Final-Unit-Action");
        FinalUnitAction.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    if (RestrictionFilterRule.IsSet())
      {
        c = cm.acquire("Restriction-Filter-Rule");
        RestrictionFilterRule.CopyTo(*c, AAA_AVP_IPFILTER_RULE_TYPE);
        cl.add(c);
      }
    if (FilterId.IsSet())
      {
        c = cm.acquire("Filter-Id");
        FilterId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
    if (RedirectServer.IsSet())
      {
        c = cm.acquire("Redirect-Server");
        RedirectServer.CopyTo(*c);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("FinalUnitAction")))
      {
        FinalUnitAction.CopyFrom(*c);
      }
    if ((c = cl.search("Restriction-Filter-Rule")))
      {
        RestrictionFilterRule.CopyFrom(*c);
      }
    if ((c = cl.search("Filter-Id")))
      {
        FilterId.CopyFrom(*c);
      }
    if ((c = cl.search("Redirect-Server")))
      {
        RedirectServer.CopyFrom(*c);
      }
  }

  DiameterScholarAttribute<diameter_enumerated_t> FinalUnitAction;
  DiameterVectorAttribute<diameter_ipfilter_rule_t> RestrictionFilterRule;
  DiameterVectorAttribute<diameter_utf8string_t> FilterId;
  DiameterGroupedScholarAttribute<redirectServer_t> RedirectServer;
};
 
/// Definition for Unit-Value AVP internal structure.
class multipleServicesCreditControl_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (GrantedServiceUnit.IsSet())
      {
        c = cm.acquire("Granted-Service-Unit");
        GrantedServiceUnit.CopyTo(*c);
        cl.add(c);
      }
    if (RequestedServiceUnit.IsSet())
      {
        c = cm.acquire("Requested-Service-Unit");
        RequestedServiceUnit.CopyTo(*c);
        cl.add(c);
      }
    if (UsedServiceUnit.IsSet())
      {
        c = cm.acquire("Used-Service-Unit");
        UsedServiceUnit.CopyTo(*c);
        cl.add(c);
        }
  if (TariffChangeUsage.IsSet())
      {
        c = cm.acquire("Tariff-Change-Usage");
        TariffChangeUsage.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
 if (ServiceIdentifier.IsSet())
      {
        c = cm.acquire("Service-Identifier");
        ServiceIdentifier.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
  if (RatingGroup.IsSet())
      {
        c = cm.acquire("Rating-Group");
        RatingGroup.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
  if (GSUPoolReference.IsSet())
    {
      c = cm.acquire("G-S-U-Pool-Reference");
      GSUPoolReference.CopyTo(*c);
      cl.add(c);
    }
  if (ValidityTime.IsSet())
    {
        c = cm.acquire("Validity-Time");
        ValidityTime.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
  if (ResultCode.IsSet())
      {
        c = cm.acquire("Result-Code");
        ResultCode.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
  if (FinalUnitIndication.IsSet())
      {
        c = cm.acquire("Final-Unit-Indication");
        FinalUnitIndication.CopyTo(*c);
        cl.add(c);
        }
      }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Granted-Service-Unit")))
      {
        GrantedServiceUnit.CopyFrom(*c);
      }
      if ((c = cl.search("Requested-Service-Unit")))
      {
        RequestedServiceUnit.CopyFrom(*c);
      }
      if ((c = cl.search("Used-Service-Unit")))
      {
        UsedServiceUnit.CopyFrom(*c);
        }
    if ((c = cl.search("Tariff-Change-Usage")))
      {
        TariffChangeUsage.CopyFrom(*c);
      }
    if ((c = cl.search("Service-Identifier")))
      {
        ServiceIdentifier.CopyFrom(*c);
      }
    if ((c = cl.search("Rating-Group")))
      {
        RatingGroup.CopyFrom(*c);
      }
    if ((c = cl.search("G-S-U-Pool-Reference")))
      {
        GSUPoolReference.CopyFrom(*c);
      }
    if ((c = cl.search("Validity-Time")))
      {
        ValidityTime.CopyFrom(*c);
      }
    if ((c = cl.search("Result-Code")))
      {
        ResultCode.CopyFrom(*c);
      }
    if ((c = cl.search("Final-Unit-Indication")))
      {
        FinalUnitIndication.CopyFrom(*c);
        }
  }
  // Optional AVPs
  DiameterGroupedScholarAttribute<grantedServiceUnit_t> GrantedServiceUnit;
  DiameterGroupedScholarAttribute<requestedServiceUnit_t> RequestedServiceUnit;
  DiameterGroupedVectorAttribute<usedServiceUnit_t> UsedServiceUnit;
  DiameterScholarAttribute<diameter_enumerated_t> TariffChangeUsage;
  DiameterVectorAttribute<diameter_unsigned32_t> ServiceIdentifier;
  DiameterScholarAttribute<diameter_unsigned32_t> RatingGroup;
  DiameterGroupedVectorAttribute<gsuPoolReference_t> GSUPoolReference;
  DiameterScholarAttribute<diameter_unsigned32_t> ValidityTime;
  DiameterScholarAttribute<diameter_unsigned32_t> ResultCode;
  DiameterGroupedScholarAttribute<finalUnitIndication_t> FinalUnitIndication;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

class serviceParameterInfo_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (ServiceParameterType.IsSet())
      {
        c = cm.acquire("Service-Parameter-Type");
        ServiceParameterType.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
        cl.add(c);
      }
    if (ServiceParameterValue.IsSet())
      {
        c = cm.acquire("Service-Parameter-Value");
        ServiceParameterValue.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Service-Parameter-Type")))
      {
        ServiceParameterType.CopyFrom(*c);
      }
    if ((c = cl.search("Service-Parameter-Value")))
      {
        ServiceParameterValue.CopyFrom(*c);
      }
  }
  DiameterScholarAttribute<diameter_unsigned32_t> ServiceParameterType;
  DiameterScholarAttribute<diameter_octetstring_t> ServiceParameterValue;
};

class userEquipmentInfo_t
{
public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (UserEquipmentInfoType.IsSet())
      {
        c = cm.acquire("User-Equipment-Info-Type");
        UserEquipmentInfoType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
        cl.add(c);
      }
    if (UserEquipmentInfoValue.IsSet())
      {
        c = cm.acquire("User-Equipment-Info-Value");
        UserEquipmentInfoValue.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
        cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("User-Equipment-Info-Type")))
      {
        UserEquipmentInfoType.CopyFrom(*c);
      }
    if ((c = cl.search("User-Equipment-Info-Value")))
      {
        UserEquipmentInfoValue.CopyFrom(*c);
      }
  }
  DiameterScholarAttribute<diameter_enumerated_t> UserEquipmentInfoType;
  DiameterScholarAttribute<diameter_octetstring_t> UserEquipmentInfoValue;
};


/// Definition for AA-Request message contents internal structure.
class CCR_Data
{
 public:
  /// Constructor.
  CCR_Data() {}

  /// Return a pointer to the instance.
  CCR_Data* Self() { return this; }

  /// Clear the contents of the instance.
  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    OriginHost.Clear(); 
    OriginRealm.Clear();
    DestinationRealm.Clear();
    ServiceContextId.Clear();
    CCRequestType.Clear();
    CCRequestNumber.Clear();
    DestinationHost.Clear();
    UserName.Clear();
    CCSubSessionId.Clear();
    AcctMultiSessionId.Clear();
    OriginStateId.Clear();
    EventTimestamp.Clear();
    SubscriptionId.Clear();
    ServiceIdentifier.Clear();
    TerminationCause.Clear();
    RequestedServiceUnit.Clear();
    RequestedAction.Clear();
    UsedServiceUnit.Clear();
    MultipleServicesIndicator.Clear();
    MultipleServicesCreditControl.Clear();
    ServiceParameterInfo.Clear();
    ServiceParameterInfo.Clear();
    CCCorrelationId.Clear();
    UserEquipmentInfo.Clear();
    ProxyInfo.Clear();
    RouteRecord.Clear();
    Avp.Clear();
  }

  /// AA-Request AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;
  DiameterScholarAttribute<diameter_identity_t>  DestinationRealm;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_utf8string_t> ServiceContextId;
  DiameterScholarAttribute<diameter_enumerated_t> CCRequestType;
  DiameterScholarAttribute<diameter_unsigned32_t> CCRequestNumber;
  DiameterScholarAttribute<diameter_identity_t>  DestinationHost;
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_unsigned64_t> CCSubSessionId;
  DiameterScholarAttribute<diameter_utf8string_t> AcctMultiSessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterScholarAttribute<diameter_time_t> EventTimestamp;
  DiameterGroupedVectorAttribute<subscriptionId_t> SubscriptionId; 
  DiameterScholarAttribute<diameter_unsigned32_t> ServiceIdentifier;
  DiameterScholarAttribute<diameter_enumerated_t> TerminationCause;
  DiameterGroupedScholarAttribute<requestedServiceUnit_t> RequestedServiceUnit;
  DiameterScholarAttribute<diameter_enumerated_t> RequestedAction;
  DiameterGroupedVectorAttribute<usedServiceUnit_t> UsedServiceUnit;
  DiameterScholarAttribute<diameter_enumerated_t> MultipleServicesIndicator;
  DiameterGroupedVectorAttribute<multipleServicesCreditControl_t> MultipleServicesCreditControl;
  DiameterGroupedVectorAttribute<serviceParameterInfo_t> ServiceParameterInfo;  
  DiameterScholarAttribute<diameter_octetstring_t> CCCorrelationId;
  DiameterGroupedScholarAttribute<userEquipmentInfo_t> UserEquipmentInfo;
  DiameterGroupedVectorAttribute<proxyInfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_identity_t> RouteRecord;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

/// Definition for CCA message contents internal structure.
class CCA_Data
{
 public:
  /// Constructor.
  CCA_Data() {}

  /// Return a pointer to the instance.
  CCA_Data* Self() { return this; }

  /// Clear the contents of the instance.
  void Clear()
  {
    SessionId.Clear();
    ResultCode.Clear();
    OriginHost.Clear();
    OriginRealm.Clear();
    AuthApplicationId.Clear();
    CCRequestType.Clear();
    CCRequestNumber.Clear();
    UserName.Clear();
    CCSessionFailover.Clear();
    AcctMultiSessionId.Clear();
    OriginStateId.Clear();
    EventTimestamp.Clear();
    GrantedServiceUnit.Clear();
    MultipleServicesCreditControl.Clear();
    CostInformation.Clear();
    FinalUnitIndication.Clear();
    CheckBalanceResult.Clear();
    CreditControlFailureHandling.Clear();
    DirectDebitingFailureHandling.Clear();
    ValidityTime.Clear();
    RedirectHost.Clear();
    RedirectHostUsage.Clear();
    RedirectMaxCacheTime.Clear();
    ProxyInfo.Clear();
    RouteRecord.Clear();
    FailedAvp.Clear();
    Avp.Clear();    
  }

  /// CCA AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> ResultCode;
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_enumerated_t> CCRequestType;
  DiameterScholarAttribute<diameter_unsigned32_t> CCRequestNumber;
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_enumerated_t> CCSessionFailover;
  DiameterScholarAttribute<diameter_unsigned64_t> CCSubSessionId;
  DiameterScholarAttribute<diameter_utf8string_t> AcctMultiSessionId;
  DiameterScholarAttribute<diameter_enumerated_t> AuthRequestType;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterScholarAttribute<diameter_time_t> EventTimestamp;
  DiameterGroupedScholarAttribute<grantedServiceUnit_t> GrantedServiceUnit;
  DiameterGroupedVectorAttribute<multipleServicesCreditControl_t> MultipleServicesCreditControl;
  DiameterGroupedScholarAttribute<costInformation_t> CostInformation;
  DiameterGroupedScholarAttribute<finalUnitIndication_t> FinalUnitIndication;
  DiameterScholarAttribute<diameter_enumerated_t> CheckBalanceResult;
  DiameterScholarAttribute<diameter_enumerated_t> CreditControlFailureHandling;
  DiameterScholarAttribute<diameter_enumerated_t> DirectDebitingFailureHandling;
  DiameterScholarAttribute<diameter_unsigned32_t> ValidityTime;
  DiameterVectorAttribute<diameter_identity_t>  RedirectHost;
  DiameterScholarAttribute<diameter_enumerated_t> RedirectHostUsage;
  DiameterScholarAttribute<diameter_unsigned32_t> RedirectMaxCacheTime;
  DiameterGroupedVectorAttribute<proxyInfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_identity_t> RouteRecord;
  DiameterVectorAttribute<diameter_avp_t> FailedAvp;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

typedef AAAParser<DiameterMsg*, CCR_Data*> CCR_Parser;
typedef AAAParser<DiameterMsg*, CCA_Data*> CCA_Parser;

template<> void CCR_Parser::parseRawToApp();
template<> void CCR_Parser::parseAppToRaw();

template<> void CCA_Parser::parseRawToApp();
template<> void CCA_Parser::parseAppToRaw();

#endif //__DIAMETER_CC_PARSER_H__
