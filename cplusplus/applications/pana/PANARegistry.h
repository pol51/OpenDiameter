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

#ifndef __PACD_REGISTRY__
#define __PACD_REGISTRY__

#define CPANA_CONFIG_SUBKEY "Software\\Toshiba Research\\PANAClient"
#define CPANA_CONFIG_DEFAULT_CFG "config\\pac.xml"
#define CPANA_CONFIG_DEFAULT_AUTH_SCRIPT "config\\pana_auth.bat"
#define CPANA_CONFIG_DEFAULT_SHARED_SECRET "config\\shared_secret.bin"
#define CPANA_CONFIG_DEFAULT_USE_ARCHIE true
#define CPANA_CONFIG_DEFAULT_RENEW_IP false
#define CPANA_CONFIG_DEFAULT_ATTEMPT_TIMEOUT 30
#define CPANA_CONFIG_DEFAULT_PING_INTERVAL 3
#define CPANA_CONFIG_DEFAULT_ENABLE_PING true
#define CPANA_CONFIG_DEFAULT_EAP_AUTH_PERIOD 900
#define CPANA_CONFIG_DEFAULT_ENABLE_AP_MONITORING false

class CPANA_Registry 
{
   public:
       CPANA_Registry() :
          m_hkResult(0) {
       }
       virtual ~CPANA_Registry() {
           Close();
       }
       LONG Open(LPCTSTR lpSubKey) {
            return RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  lpSubKey,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &m_hkResult,
                                  NULL);
       }
       LONG Get(LPCTSTR lpValueName, LPBYTE lpData, LPDWORD lpcbData) {
            return RegQueryValueEx(m_hkResult,
                                   lpValueName,
                                   NULL,
                                   NULL,
                                   lpData,
                                   lpcbData);
       }
       LONG Set(LPCTSTR lpValueName, DWORD dwType, 
                const BYTE* lpData, DWORD cbData) {
            return RegSetValueEx(m_hkResult,
                                 lpValueName,
                                 NULL,
                                 dwType,
                                 lpData,
                                 cbData);
       }
       void Close() {
           if (m_hkResult) {
               RegCloseKey(m_hkResult);
               m_hkResult = 0;
           }
       }

   private:
       HKEY m_hkResult;
};

typedef struct {
    CString m_strCfgFilename;
    CString m_strScriptFilename;
    CString m_strSharedSecretFilename;
    CString m_strAdaptername;
    DWORD m_dwUseArchie;
    DWORD m_dwRenewIPAddress;
    DWORD m_dwAttemptTimeout;
    DWORD m_dwPingInterval;
    DWORD m_dwEnablePing;
    DWORD m_dwEapAuthPeriod;
    DWORD m_dwEnableAPMon;
} PaCConfig;

class CPANA_PaCRegistry : public CPANA_Registry
{
   public:
       CPANA_PaCRegistry() {
       }
       virtual ~CPANA_PaCRegistry() {
       }
       LONG Load(PaCConfig &cfg) {
           LONG err = Open(CPANA_CONFIG_SUBKEY);
           if (err == ERROR_SUCCESS) {
               Get("ConfigFile", cfg.m_strCfgFilename);
               Get("AuthScriptFile", cfg.m_strScriptFilename);
               Get("SharedSecretFile", cfg.m_strSharedSecretFilename);
               Get("AdapterName", cfg.m_strAdaptername);
               Get("UseArchie", cfg.m_dwUseArchie);
               Get("RenewIP", cfg.m_dwRenewIPAddress);
               Get("AttemptTimeout", cfg.m_dwAttemptTimeout);
               Get("PingInterval", cfg.m_dwPingInterval);
               Get("EnablePing", cfg.m_dwEnablePing);
               Get("EAPAuthPeriod", cfg.m_dwEapAuthPeriod);
               Get("EnableAPMonitoring", cfg.m_dwEnableAPMon);
               Close();
           }
           return err;
       }
       LONG Save(PaCConfig &cfg) {
           LONG err = Open(CPANA_CONFIG_SUBKEY);
           if (err == ERROR_SUCCESS) {
               Set("ConfigFile", cfg.m_strCfgFilename);
               Set("AuthScriptFile", cfg.m_strScriptFilename);
               Set("SharedSecretFile", cfg.m_strSharedSecretFilename);
               Set("AdapterName", cfg.m_strAdaptername);
               Set("UseArchie", cfg.m_dwUseArchie);
               Set("RenewIP", cfg.m_dwRenewIPAddress);
               Set("AttemptTimeout", cfg.m_dwAttemptTimeout);               
               Set("PingInterval", cfg.m_dwPingInterval);
               Set("EnablePing", cfg.m_dwEnablePing);
               Set("EAPAuthPeriod", cfg.m_dwEapAuthPeriod);
               Set("EnableAPMonitoring", cfg.m_dwEnableAPMon);
               Close();
           }
           return err;
       }
       void Default(PaCConfig &cfg) {
            char szCurrentDir[256];
            getcwd(szCurrentDir, sizeof(szCurrentDir));
            cfg.m_dwUseArchie = CPANA_CONFIG_DEFAULT_USE_ARCHIE;
            cfg.m_dwRenewIPAddress = CPANA_CONFIG_DEFAULT_RENEW_IP;
            cfg.m_strCfgFilename = szCurrentDir;
            cfg.m_strCfgFilename += "\\";
            cfg.m_strCfgFilename += CPANA_CONFIG_DEFAULT_CFG;
            cfg.m_strScriptFilename = szCurrentDir;
            cfg.m_strScriptFilename += "\\";
            cfg.m_strScriptFilename += CPANA_CONFIG_DEFAULT_AUTH_SCRIPT;
            cfg.m_strSharedSecretFilename = szCurrentDir;
            cfg.m_strSharedSecretFilename += "\\";
            cfg.m_strSharedSecretFilename += CPANA_CONFIG_DEFAULT_SHARED_SECRET;
            cfg.m_strAdaptername = "";
            cfg.m_dwAttemptTimeout = CPANA_CONFIG_DEFAULT_ATTEMPT_TIMEOUT;
            cfg.m_dwPingInterval = CPANA_CONFIG_DEFAULT_PING_INTERVAL;
            cfg.m_dwEnablePing = CPANA_CONFIG_DEFAULT_ENABLE_PING;
            cfg.m_dwEapAuthPeriod = CPANA_CONFIG_DEFAULT_EAP_AUTH_PERIOD;
            cfg.m_dwEnableAPMon = CPANA_CONFIG_DEFAULT_ENABLE_AP_MONITORING;
       }
   protected:
       LONG Get(LPCTSTR lpValueName, CString &cstrData) {
           CHAR szBuffer[256];
           DWORD dwSize = sizeof(szBuffer);
           memset(szBuffer, 0, sizeof(szBuffer));
           LONG err = CPANA_Registry::Get
               (lpValueName, (LPBYTE)szBuffer, &dwSize);
           if (err == ERROR_SUCCESS) {
               cstrData = szBuffer;
           }
           return err;
       }
       LONG Get(LPCTSTR lpValueName, DWORD &dwData) {
           DWORD dwSize = sizeof(DWORD);
           return CPANA_Registry::Get
                   (lpValueName, (LPBYTE)&dwData, &dwSize);
       }
       LONG Set(LPCTSTR lpValueName, CString &cstrData) {
           LPTSTR lpStr = cstrData.GetBuffer
               (cstrData.GetLength());
           return CPANA_Registry::Set(lpValueName, REG_SZ, 
                                      (LPBYTE)lpStr, cstrData.GetLength());
       }
       LONG Set(LPCTSTR lpValueName, DWORD dwData) {
           return CPANA_Registry::Set(lpValueName, REG_DWORD, 
                      (LPBYTE)&dwData, sizeof(DWORD));
       }
};

#endif
