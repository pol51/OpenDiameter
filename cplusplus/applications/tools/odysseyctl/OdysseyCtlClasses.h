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

#ifndef __PANA_PAC_ODESSEY_CTL__
#define __PANA_PAC_ODESSEY_CTL__

#include "stdafx.h"
#include "SharedMem.h"

#define ODESSEY_CLIENT_EXECPATH _T("C:\\Program Files\\Funk Software\\Odyssey Client\\odClientMgr.exe")
#define ODYSSEY_DEFAULT_SHMEMSIZE  4000
#define STDOUT  std::cout << __FUNCTION__ << ": "
#define STDENDL std::endl;
#define RETRY_COUNT  5

class Odyssey_RetryGuard
{
public:
    Odyssey_RetryGuard() :
        m_LastTry(0) {
    }
    BOOL Test(int sec) {
        time_t current = time(NULL);
        if ((current - m_LastTry) < sec) {
		    return FALSE;// TRUE;
        }
	    m_LastTry = current;
        return FALSE;
    }

private:
    time_t m_LastTry;    
};

class Odyssey_WindowLookupTopLevel
{
public:
    virtual BOOL Test(LPCTSTR name, 
                      HWND hwnd) = 0;
    static BOOL Lookup(LPARAM lParam, 
                       int maxSec = 3);
};

class Odyssey_WindowLookupChildren
{
public:
    virtual BOOL Test(HWND hwnd) = 0;
    static BOOL Lookup(HWND parent, 
                       LPARAM lParam);
};

class Odyssey_WindowLookupChildrenByName :
    public Odyssey_WindowLookupChildren
{
public:
    Odyssey_WindowLookupChildrenByName() :
       m_hWnd(NULL) {
    }
    HWND Lookup(HWND parent, LPCTSTR name);

protected:
    virtual BOOL Test(HWND hwnd);

private:
    TCHAR m_Name[1024];
    HWND m_hWnd;
};

class Odyssey_ClientMgr : 
    public Odyssey_WindowLookupTopLevel
{
public:
    Odyssey_ClientMgr();
    HWND Handle();

protected:
    BOOL StartApplication();
    BOOL Test(LPCTSTR name, HWND hwnd);

private:
    HWND m_hWnd;
};

class Odyssey_SelectAction
{
public:
    Odyssey_SelectAction(Odyssey_ClientMgr &mgr);
    BOOL Select(LPCTSTR name);

private:
    Odyssey_ClientMgr &m_Mgr;
    Odyssey_RetryGuard m_Guard;
};

class Odyssey_NetworksWindow : 
    public Odyssey_WindowLookupChildren
{
private:
    class NetworkProperties : 
        public Odyssey_WindowLookupTopLevel {
    public:
        typedef enum {
            ID_SSID_EDIT     = 0x0000012B,
            ID_DESC_EDIT     = 0x0000012C,
            ID_ASSOC_CMB     = 0x00000282,
            ID_KEY_GROUP     = 0x00000281
        };
        typedef enum {
            MODE_OPEN = 0, // index to combo control
            MODE_WPA = 2,  // index to combo control
        } MODE;
    public:
        NetworkProperties();
        HWND Handle(BOOL checked = TRUE);
        BOOL Test(LPCTSTR name, 
                  HWND hwnd);
        BOOL Done(BOOL ok = TRUE);
        BOOL SetSSId(LPCTSTR ssid);
        BOOL SetDescription(LPCTSTR desc);
        BOOL SetAssociationMode(MODE mode);
        BOOL SetWPAKey(LPCTSTR key);
    private:
        HWND m_hWnd;
    };

public:
    typedef enum {
        ID_NETWORK_LABEL = 0x00000125,
        ID_ADD_BUTTON    = 0x000000CD,
        ID_DEL_BUTTON    = 0x000000CE,
        ID_PROP_BUTTON   = 0x000000CF
    };

public:
    Odyssey_NetworksWindow(Odyssey_ClientMgr &mgr);
    BOOL Add(LPCTSTR name, LPCTSTR ssid);
    BOOL Add(LPCTSTR name, LPCTSTR ssid, LPCTSTR key);
    BOOL Remove(LPCTSTR name, LPCTSTR ssid);

protected:
    BOOL Test(HWND hwnd);
    BOOL ShowNetworkProperties(int id);

private:
    HWND m_hWnd;
    Odyssey_ClientMgr &m_Mgr;
    Odyssey_RetryGuard m_Guard;
    NetworkProperties m_NetworkProperties;
};

class Odyssey_ConnectionWindow : 
    public Odyssey_WindowLookupChildren
{
public:
    typedef enum {
        ID_CONNECTION_LABEL = 0x00000125,
        ID_CONNECT_BTN      = 0x00000132,
        ID_CONNECT_CMB      = 0x00000134,
        ID_RECONNECT_BTN    = 0x0000011C,
        ID_REAUTH_BTN       = 0x0000011B,
        ID_STATUS_LBL       = 0x0000010F,
        ID_DEVICE_CMB       = 0x00000109
    };

public:
    Odyssey_ConnectionWindow(Odyssey_ClientMgr &mgr);
    BOOL SetConnection(BOOL bConnect);
    BOOL SelectNetwork(LPCTSTR name, LPCTSTR ssid);
    BOOL SelectDevice(LPCTSTR name);
    BOOL ReConnect();
    BOOL ReAuthenticate();
    BOOL Status(LPSTR text, UINT tsize);
    BOOL GetConnection(LPSTR text, UINT tsize);

protected:
    BOOL Test(HWND hwnd);

private:
    HWND m_hWnd;
    Odyssey_ClientMgr &m_Mgr;
    Odyssey_RetryGuard m_Guard;
};

#endif