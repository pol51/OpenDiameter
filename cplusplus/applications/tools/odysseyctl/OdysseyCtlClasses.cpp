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

#include "OdysseyCtlClasses.h"

static BOOL 
SetText(HWND parent, int id, LPCTSTR text)
{
    HWND hEdt = NULL;
    TCHAR szBuf[1024];

    if (parent == NULL) {
        STDOUT << "Null parent" << STDENDL;
        return FALSE;
    }

    hEdt = GetDlgItem(parent, id);
    memset(szBuf, 0, sizeof(szBuf));
    strncpy(szBuf, text, sizeof(szBuf));

    if (! SendMessage(hEdt, WM_SETTEXT, 0, (LPARAM)szBuf)) {
        STDOUT << "WM_SETTEXT failed when setting" << text << STDENDL;
        return FALSE;
    }

    STDOUT << text << "Ok" << STDENDL;
    return TRUE;
}

static BOOL
SetTextUsingKB(HWND edit, LPCTSTR text)
{
    SHORT   key  = 0;
    UINT    code = 0;

    for (UINT ndx = 0; ndx < strlen(text); ndx++) {
        key = VkKeyScan(text[ndx]);
        code = MapVirtualKey(key, 0);
        if (code == 0) {
            code = OemKeyScan(text[ndx]);
        }
        code = (code << 16) + 1;
        PostMessage(edit, WM_KEYDOWN, key, code);
        //PostMessage(edit, WM_CHAR, text[ndx], code);
        //PostMessage(edit, WM_KEYUP, key, (0xC0000000) && code);
    }

    STDOUT << "Ok [" << text << "]" << STDENDL;
    return TRUE;
}

static BOOL
PostButton(HWND hwnd, UINT msg, WPARAM wparam, RECT *rc)
{
	long cx, cy;
	cx = (rc->left + rc->right) / 2;
	cy = (rc->top + rc->bottom) / 2;
    if (PostMessage(hwnd, msg, wparam, MAKELONG(cx, cy))) {
        STDOUT << "Ok" << STDENDL;
        return TRUE;
    }
    else {
        STDOUT << "Failed to post " << msg << STDENDL;
        return FALSE;
    }
}

BOOL
static GetItemText(CSharedMem &shm, HWND hwnd, 
                   int index, LPSTR text, UINT tsize)
{
    LVITEM *pItem = (LVITEM *)shm.Local();
    pItem->mask = LVIF_TEXT;
    pItem->iItem = index;
    pItem->iSubItem = 0;
    pItem->pszText = (LPTSTR)((LVITEM *)shm.Peer() + 1);
    pItem->cchTextMax = ODYSSEY_DEFAULT_SHMEMSIZE - sizeof(*pItem);

	shm.WriteMemory();
    if (SendMessage(hwnd, LVM_GETITEMTEXT, index, (LPARAM)shm.Peer()) <= 0) {
        STDOUT << "Failed to get item text" << STDENDL;
        return FALSE;
    }
	shm.ReadMemory();

	LPCTSTR rcText = (LPCTSTR)((LVITEM *)shm.Local() + 1);
    strncpy(text, rcText, tsize);

    STDOUT << "Ok, item text is " << text << STDENDL;
    return TRUE;
}

static BOOL CALLBACK
EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	INT                            len;
	TCHAR                          wt[1024];
    Odyssey_WindowLookupTopLevel  *lookup;

	len = GetWindowText(hwnd, wt, sizeof(wt)/sizeof(TCHAR));
    if (len <= 0) {
		wt[0] = '\0';
    }
    lookup = (Odyssey_WindowLookupTopLevel*)lParam;
    return lookup->Test(wt, hwnd);
}

BOOL 
Odyssey_WindowLookupTopLevel::Lookup(LPARAM lParam,
                                     int maxSec)
{
    for (int retry = 0; retry < (maxSec*2); retry ++) {
        if (EnumWindows(EnumWindowsProc, lParam)) {
            SleepEx(1000, FALSE);
            continue;
        }
        return TRUE;
    }

    STDOUT << "Lookup failed" << STDENDL;
    return FALSE;
}

BOOL 
Odyssey_WindowLookupChildren::Lookup(HWND parent, 
                                     LPARAM lParam)
{
    HWND hCurrent                         = NULL;
    Odyssey_WindowLookupChildren *lookup  = NULL;

    lookup = (Odyssey_WindowLookupChildren*)lParam;
    while ((hCurrent = FindWindowEx(parent, hCurrent, 
                                    NULL, NULL))) {
        if (lookup->Test(hCurrent)) {
            return TRUE;
        }
    }

    STDOUT << "Lookup failed" << STDENDL;
    return FALSE;
}

HWND 
Odyssey_WindowLookupChildrenByName::Lookup(HWND parent, 
                                           LPCTSTR name) 
{
    memset(m_Name, 0, sizeof(m_Name));
    strcpy(m_Name, name);
    if (! Odyssey_WindowLookupChildren::Lookup(parent, (LPARAM)this)) {
        STDOUT << "Lookup failed for " << name << STDENDL;
        m_hWnd = NULL;
    }
    return m_hWnd;
}

BOOL 
Odyssey_WindowLookupChildrenByName::Test(HWND hwnd)
{
    TCHAR text[1024];
    if (GetWindowText(hwnd, text, sizeof(text)) > 0) {
        if (! strcmp(text, m_Name)) {
            m_hWnd = hwnd;
            STDOUT << "Found child window " << m_Name << STDENDL;
            return TRUE;
        }
    }
    return FALSE;
}

Odyssey_ClientMgr::Odyssey_ClientMgr() :
    m_hWnd(NULL)
{
    m_hWnd = Handle();
}

HWND 
Odyssey_ClientMgr::Handle() 
{
    if (m_hWnd == NULL) {
        for (int retry = 0; retry < RETRY_COUNT; retry ++) {
            if (Odyssey_WindowLookupTopLevel::Lookup((LPARAM)this, retry + 1)) {            
                break; // m_hWnd is populated by Test()
            }
            STDOUT << "Application not found, starting ..." << STDENDL;
            if (! StartApplication()) {
                m_hWnd = NULL;
                SleepEx(1000, FALSE);
                continue;
            }
            break;
        }
    }
    return m_hWnd;
}

BOOL 
Odyssey_ClientMgr::StartApplication() 
{
	PROCESS_INFORMATION proc_info;
	STARTUPINFO sui;
	ZeroMemory(&sui, sizeof(sui));
	sui.cb = sizeof(STARTUPINFO);

	if (CreateProcess(ODESSEY_CLIENT_EXECPATH, NULL, NULL, NULL, FALSE,
					  (CREATE_NEW_PROCESS_GROUP),
					  NULL, NULL, &sui, &proc_info) == 0) {
        STDOUT << "Failed to start application" << STDENDL;
		return FALSE;
	}

	CloseHandle(proc_info.hProcess);
	CloseHandle(proc_info.hThread);

    STDOUT << "Application started" << STDENDL;
	return TRUE;
}

BOOL 
Odyssey_ClientMgr::Test(LPCTSTR name, HWND hwnd)
{
    if (!strcmp(name, "Odyssey Client Manager")) {
        m_hWnd = hwnd;

        STDOUT << "Found Odyssey Client Manager Window" << STDENDL;
        return FALSE; // found it
    }
    return TRUE; // continue
}

Odyssey_SelectAction::Odyssey_SelectAction
    (Odyssey_ClientMgr &mgr) : m_Mgr(mgr)
{
}

BOOL 
Odyssey_SelectAction::Select(LPCTSTR name)
{
    HWND mgrWnd     = NULL;
    HWND lstWnd     = NULL;
    int lCount      = 0;
    TCHAR szBuf[1024];

    if (m_Guard.Test(1)) {
        STDOUT << "Too many retries" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }

    if ((mgrWnd = m_Mgr.Handle()) == NULL) {
        STDOUT << "Failed to get Handle()" << STDENDL;
        return FALSE;
    }

    for (lCount = 0; lCount < RETRY_COUNT; lCount ++) {
        Odyssey_WindowLookupChildrenByName lookup;
        if ((lstWnd = lookup.Lookup(mgrWnd, "List1")) == NULL) {
            STDOUT << "Can't find child window List1" << STDENDL;
            SleepEx(1000, FALSE);
            continue;
        }
        break;
    }
    if (lCount == RETRY_COUNT) {
        STDOUT << "Can't find child window List1 giving up" << STDENDL;
        return FALSE;
    }

	CSharedMem shm(lstWnd, ODYSSEY_DEFAULT_SHMEMSIZE);

    lCount = ListView_GetItemCount(lstWnd);
    for (int lIndex = 0; lIndex < lCount; lIndex ++) {
        if (! GetItemText(shm, lstWnd, (int)lIndex, szBuf, sizeof(szBuf))) {
            STDOUT << "Can't get item text from list" << STDENDL;
            return FALSE;
        }
        if (strcmp(szBuf, name)) {
            continue;
        }

        STDOUT << "Found item " << name << " in list" << STDENDL;

        RECT *rc = (RECT*)shm.Local();    	
        rc->left = LVIR_BOUNDS;

        shm.WriteMemory();
	    if (! SendMessage(lstWnd, LVM_GETITEMRECT, lIndex,
					      (LPARAM)shm.Peer())) {
            STDOUT << "Failed to get item RECT" << STDENDL;
		    return FALSE;
	    }
	    shm.ReadMemory();

	    return (PostButton(lstWnd, WM_LBUTTONDOWN, MK_LBUTTON, rc)
			    && PostButton(lstWnd, WM_LBUTTONUP, 0, rc));
    }

    STDOUT << "Can't find item " << name << " in list" << STDENDL;
    return FALSE;
}

Odyssey_NetworksWindow::NetworkProperties::NetworkProperties() :
    m_hWnd(NULL) 
{
}

HWND 
Odyssey_NetworksWindow::NetworkProperties::Handle(BOOL checked) 
{
    if ((m_hWnd == NULL) && checked) {
        for (int i = 0; i < RETRY_COUNT; i ++) {
            if (! Odyssey_WindowLookupTopLevel::Lookup((LPARAM)this)) {
                STDOUT << "Can't find Network Properties window" << STDENDL;
                SleepEx(1000, FALSE);
                continue;
            }
            break;
        }
    }
    return m_hWnd;
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::Test(LPCTSTR name, 
                                                HWND hwnd) 
{
    if ((!strcmp(name, "Add Network")) ||
        (!strcmp(name, "Network Properties"))) {
        m_hWnd = hwnd;
        STDOUT << "Found " << name << " window" << STDENDL;
        return FALSE;
    }
    return TRUE;
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::SetSSId(LPCTSTR ssid)
{
    STDOUT << "Setting SSID" << STDENDL;
    return SetText(m_hWnd, ID_SSID_EDIT, ssid);
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::SetDescription(LPCTSTR desc)
{
    STDOUT << "Setting Description" << STDENDL;
    return SetText(m_hWnd, ID_DESC_EDIT, desc);
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::SetAssociationMode(MODE mode)
{
    HWND          hCmbAssoc    = NULL;
    HWND          hEdtKey      = NULL;
    COMBOBOXINFO  cbInfo;
    RECT          itemRect;

    STDOUT << "Setting Association Mode" << STDENDL;

    if ((hCmbAssoc = GetDlgItem(m_hWnd, ID_ASSOC_CMB)) == NULL) {
        STDOUT << "Can't find Association Mode window" << STDENDL;
        return FALSE;
    }

    cbInfo.cbSize = sizeof(COMBOBOXINFO);
    if (! SendMessage(hCmbAssoc, CB_GETCOMBOBOXINFO, 
                      0, (LPARAM)&cbInfo)) {
        STDOUT << "Failed to get ComboBoxInfo from assocation dialog" << STDENDL;
		return FALSE;
	}

    if (! PostButton(hCmbAssoc, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(cbInfo.rcButton))
        || ! PostButton(hCmbAssoc, WM_LBUTTONUP, 0, &(cbInfo.rcButton))) {
        STDOUT << "Failed to post button click in association mode arrow button" << STDENDL;
        return FALSE;
    }

    if (SendMessage(cbInfo.hwndList, LB_GETITEMRECT, 
                    (int)mode, (LPARAM)&itemRect) == LB_ERR) {
        STDOUT << "Failed to get RECT from association mode item" << STDENDL;
		return FALSE;
	}

    if (! PostButton(cbInfo.hwndList, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(itemRect))
        || ! PostButton(cbInfo.hwndList, WM_LBUTTONUP, 0, &(itemRect))) {
        STDOUT << "Failed to post button click in association mode item" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::SetWPAKey(LPCTSTR key)
{
    class KeyEditLookup : 
        public Odyssey_WindowLookupChildren
    {
    public:
        typedef enum {
            ID_PASSPHRASE_LBL  = 0x000002F1,
            ID_PASSPHRASE_EDIT = 0x00000284
        };
    public:
        KeyEditLookup() :
            m_hWnd(NULL),
            m_hParent(NULL) {
        }
        HWND Handle() {
            return m_hWnd;
        }
        HWND Parent() {
            return m_hParent;
        }
        virtual BOOL Test(HWND hwnd) {
            HWND hLabel = NULL;
            HWND hEdit  = NULL;
            TCHAR text[1024];

            if (! (hLabel = GetDlgItem(hwnd, ID_PASSPHRASE_LBL))) {
                return FALSE;
            }
            if (GetWindowText(hLabel, text, sizeof(text)) > 0) {
                if (strcmp(text, "&Passphrase:")) {
                    return FALSE;
                }
                if (! (hEdit = GetDlgItem(hwnd, ID_PASSPHRASE_EDIT))) {
                    return FALSE;
                }

                STDOUT << "Found WPA key edit box" << STDENDL;
                m_hParent = hwnd;
                m_hWnd = hEdit;
                return TRUE;
            }
            return FALSE;
        }
    private:
        HWND m_hWnd;
        HWND m_hParent;
    };

    HWND           hGroup = NULL;
    HWND           hEdit  = NULL;
    KeyEditLookup  keyLookup;
    RECT           editRect;

    if ((hGroup = GetDlgItem(m_hWnd, ID_KEY_GROUP)) == NULL) {
        STDOUT << "Can't find static parent for WPA phassphrase" << STDENDL;
        return FALSE;
    }
    if (! Odyssey_WindowLookupChildren::Lookup(hGroup, 
                                               (LPARAM)&keyLookup)) {
        STDOUT << "Can't find WPA key edit window" << STDENDL;
        return FALSE;
    }
    if ((hEdit = keyLookup.Handle()) == NULL) {
        STDOUT << "Invalid handle for WPA key edit window" << STDENDL;
        return FALSE;
    }

    SendMessage(hEdit, EM_GETRECT, 0, (LPARAM)&editRect);
    if (! PostButton(hEdit, WM_LBUTTONDOWN, MK_LBUTTON, &(editRect))
        || ! PostButton(hEdit, WM_LBUTTONUP, 0, &(editRect))) {
        STDOUT << "Failed to send click event to WPA key edit" << STDENDL;
        return FALSE;
    }

    STDOUT << "Setting WPA key using KB events" << STDENDL;
    return SetTextUsingKB(hEdit, key);
}

BOOL 
Odyssey_NetworksWindow::NetworkProperties::Done(BOOL ok)
{
    HWND hBtn   = NULL;
    INT  idBtn  = (ok) ? IDOK : IDCANCEL;

    if (m_hWnd == NULL) {
        STDOUT << "Invalid Handle" << STDENDL;
        return FALSE;
    }

    // click the "OK" or "Cancel" button
    if ((hBtn = GetDlgItem(m_hWnd, idBtn)) == NULL) {
        STDOUT << "Can't find OK or CANCEL" << STDENDL;
        return FALSE;
    }
    if (! PostMessage(m_hWnd, WM_COMMAND,
                      MAKELONG(idBtn, BN_CLICKED), 
                      (LPARAM)hBtn)) {
        STDOUT << "Failed to post OK or CANCEL event" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    m_hWnd = NULL;
    return TRUE;
}

Odyssey_NetworksWindow::Odyssey_NetworksWindow(Odyssey_ClientMgr &mgr) :
    m_Mgr(mgr),
    m_hWnd(NULL)
{
    if (! Odyssey_WindowLookupChildren::Lookup(mgr.Handle(), 
                                               (LPARAM)this)) {
        STDOUT << "Can't find Networks window" << STDENDL;
        m_hWnd = NULL;
    }
}

BOOL 
Odyssey_NetworksWindow::Test(HWND hwnd)
{
    HWND  hLabel;
    TCHAR text[1024];

    if (! (hLabel = GetDlgItem(hwnd, ID_NETWORK_LABEL))) {
        return FALSE;
    }
    if (GetWindowText(hLabel, text, sizeof(text)) > 0) {
        if (! strcmp(text, "Networks")) {
            m_hWnd = hwnd;

            STDOUT << "Found Networks window" << STDENDL;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL 
Odyssey_NetworksWindow::ShowNetworkProperties(int id)
{
    HWND hBtn = NULL;

    if ((m_hWnd == NULL) || 
        (m_NetworkProperties.Handle(FALSE))) {
        STDOUT << "Invalid Handle or no network properties window" << STDENDL;
        return FALSE;
    }
    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }
    if ((hBtn = GetDlgItem(m_hWnd, id)) == NULL) {
        STDOUT << "Failed to get " << id << " window" << STDENDL;
        return FALSE;
    }
    if (! PostMessage(m_hWnd, WM_COMMAND,
                      MAKELONG(id, BN_CLICKED), 
                      (LPARAM)hBtn)) {
        STDOUT << "Failed to send click event to " << STDENDL;
        return FALSE;
    }
    return (m_NetworkProperties.Handle()) ? TRUE : FALSE;
}

BOOL 
Odyssey_NetworksWindow::Add(LPCTSTR name, LPCTSTR ssid)
{
    HWND hAddDlg  = NULL;
    HWND hBtn     = NULL;

    STDOUT << "Addig open network " << name << " for " << ssid << STDENDL;

    if (! ShowNetworkProperties(ID_ADD_BUTTON)) {
        STDOUT << "ShowNetworkProperties failed" << STDENDL;
        return FALSE;
    }
    for (int i = 0; i < RETRY_COUNT; i ++) {
        try {
            if (! m_NetworkProperties.SetSSId(ssid)) {
                STDOUT << "Faied to set SSID" << STDENDL;
                throw (FALSE);
            }
            if (! m_NetworkProperties.SetDescription(name)) {
                STDOUT << "Faied to set Description" << STDENDL;
                throw (FALSE);
            }
            if (! m_NetworkProperties.SetAssociationMode(NetworkProperties::MODE_OPEN)) {
                STDOUT << "Faied to set Association mode" << STDENDL;
                throw (FALSE);
            }
            throw (TRUE);
        }
        catch (BOOL bResult) {
            if (bResult == FALSE) {
                SleepEx(1000, FALSE);
                continue;
            }
            m_NetworkProperties.Done(bResult);
            STDOUT << "Done" << STDENDL;
            return bResult;
        }
    }
    m_NetworkProperties.Done(FALSE);
    STDOUT << "Failed to add after several retries" << STDENDL;
    return FALSE;
}

BOOL 
Odyssey_NetworksWindow::Add(LPCTSTR name, 
                            LPCTSTR ssid, 
                            LPCTSTR key)
{
    STDOUT << "Addig WPA network " << name << " for " << ssid << STDENDL;

    if (! ShowNetworkProperties(ID_ADD_BUTTON)) {
        return FALSE;
    }
    for (int i = 0; i < RETRY_COUNT; i ++) {
        try {
            if (! m_NetworkProperties.SetSSId(ssid)) {
                STDOUT << "Faied to set SSID" << STDENDL;
                throw (FALSE);
            }
            if (! m_NetworkProperties.SetDescription(name)) {
                STDOUT << "Faied to set Description" << STDENDL;
                throw (FALSE);
            }
            if (! m_NetworkProperties.SetAssociationMode(NetworkProperties::MODE_WPA)) {
                STDOUT << "Faied to set Association mode" << STDENDL;
                throw (FALSE);
            }
            if (! m_NetworkProperties.SetWPAKey(key)) {
                STDOUT << "Faied to set WPA key" << STDENDL;
                throw (FALSE);
            }
            throw (TRUE);
        }
        catch (BOOL bResult) {
            if (bResult == FALSE) {
                SleepEx(1000, FALSE);
                continue;
            }
            m_NetworkProperties.Done(bResult);
            STDOUT << "Done" << STDENDL;
            return bResult;
        }
    }
    m_NetworkProperties.Done(FALSE);
    STDOUT << "Failed to add after several retries" << STDENDL;
    return FALSE;
}

BOOL 
Odyssey_NetworksWindow::Remove(LPCTSTR name, LPCTSTR ssid)
{
    HWND hBtn       = NULL;
    HWND lstWnd     = NULL;
    int lCount      = 0;
    TCHAR szBuf[1024];
    TCHAR szItem[1024];

    STDOUT << "Removing network " << name << " for " << ssid << STDENDL;

    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }

    Odyssey_WindowLookupChildrenByName lookup;
    if ((lstWnd = lookup.Lookup(m_hWnd, "List1")) == NULL) {
        STDOUT << "Can't find List1 window" << STDENDL;
        return FALSE;
    }

    if (strlen(name)) {
        sprintf(szItem, "%s <%s>", name, ssid);
    }
    else {
        sprintf(szItem, "<%s>", ssid);
    }

	CSharedMem shm(lstWnd, ODYSSEY_DEFAULT_SHMEMSIZE);

    lCount = ListView_GetItemCount(lstWnd);
    for (int lIndex = 0; lIndex < lCount; lIndex ++) {
        if (! GetItemText(shm, lstWnd, (int)lIndex, szBuf, sizeof(szBuf))) {
            STDOUT << "Failed to get item text" << STDENDL;
            return FALSE;
        }
        if (strcmp(szBuf, szItem)) {
            continue;
        }

        STDOUT << "Found network " << szItem << STDENDL;

        RECT *rc = (RECT*)shm.Local();    	
        rc->left = LVIR_BOUNDS;

        shm.WriteMemory();
	    if (! SendMessage(lstWnd, LVM_GETITEMRECT, lIndex,
					      (LPARAM)shm.Peer())) {
            STDOUT << "Failed to get item RECT" << STDENDL;
		    return FALSE;
	    }
	    shm.ReadMemory();

	    if (PostButton(lstWnd, WM_LBUTTONDOWN, MK_LBUTTON, rc)
            && PostButton(lstWnd, WM_LBUTTONUP, 0, rc)) {
            if ((hBtn = GetDlgItem(m_hWnd, ID_DEL_BUTTON)) == NULL) {
                STDOUT << "Can't find Remove button" << STDENDL;
                return FALSE;
            }
            if (! PostMessage(m_hWnd, WM_COMMAND,
                            MAKELONG(ID_DEL_BUTTON, BN_CLICKED), 
                            (LPARAM)hBtn)) {
                STDOUT << "Failed to post click event on Remove button" << STDENDL;
                return FALSE;
            }
        }
        STDOUT << "Ok" << STDENDL;
        return TRUE;
    }
    STDOUT << name << " not found" << STDENDL;
    return FALSE;
}

Odyssey_ConnectionWindow::Odyssey_ConnectionWindow(Odyssey_ClientMgr &mgr) :
    m_Mgr(mgr),
    m_hWnd(NULL)
{
    int i;
    for (i = 0; i < RETRY_COUNT; i ++) {
        if (! Odyssey_WindowLookupChildren::Lookup(mgr.Handle(), 
                                                (LPARAM)this)) {
            STDOUT << "Can't find Odyssey Client Manager window" << STDENDL;
            m_hWnd = NULL;
            continue;
        }
        break;
    }
    if (i == RETRY_COUNT) {
        STDOUT << "Can't find Odyssey Client Manager window giving up" << STDENDL;
    }
}

BOOL 
Odyssey_ConnectionWindow::Test(HWND hwnd)
{
    HWND  hLabel;
    TCHAR text[1024];

    if (! (hLabel = GetDlgItem(hwnd, ID_CONNECTION_LABEL))) {
        STDOUT << "Can't get Connection label window" << STDENDL;
        return FALSE;
    }
    if (GetWindowText(hLabel, text, sizeof(text)) > 0) {
        if (! strcmp(text, "Connection")) {
            m_hWnd = hwnd;
            STDOUT << "Found Connection Label" << STDENDL;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL 
Odyssey_ConnectionWindow::SetConnection(BOOL bConnect)
{
    HWND    hBtn    = NULL;
    LRESULT lState  = 0;

    STDOUT << "Setting connection event to " << bConnect << STDENDL;

    if (m_hWnd == NULL) {
        STDOUT << "Invalid handle" << STDENDL;
        return FALSE;
    }
    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }
    if ((hBtn = GetDlgItem(m_hWnd, ID_CONNECT_BTN)) == NULL) {
        STDOUT << "Can't retrieve Connect button window" << STDENDL;
        return FALSE;
    }
    
    lState = SendMessage(hBtn, BM_GETCHECK, 0, 0);
    if ((lState == BST_CHECKED) && bConnect) {
        STDOUT << "Already connected" << STDENDL;
        return TRUE;
    }
    else if ((lState == BST_UNCHECKED) && !bConnect) {
        STDOUT << "Already disconnected" << STDENDL;
        return TRUE;
    }
    else if (lState == BST_INDETERMINATE) {
        STDOUT << "Unknow state for the button" << STDENDL;
        return FALSE;
    }

	CSharedMem shm(hBtn, ODYSSEY_DEFAULT_SHMEMSIZE);

    RECT *rc = (RECT*)shm.Local();    	

    shm.WriteMemory();
	if (! SendMessage(hBtn, BCM_GETTEXTMARGIN, 0,
                      (LPARAM)shm.Peer())) {
        STDOUT << "Failed to get RECT of Connect button" << STDENDL;
        return FALSE;
    }
    shm.ReadMemory();

    if (PostButton(hBtn, WM_LBUTTONDOWN, MK_LBUTTON, rc)
        && PostButton(hBtn, WM_LBUTTONUP, 0, rc)) {
        STDOUT << "Ok" << STDENDL;
        return TRUE;
    }

    STDOUT << "Failed to post click event to connect button" << STDENDL;
    return FALSE;
}

BOOL 
Odyssey_ConnectionWindow::SelectNetwork(LPCTSTR name, 
                                        LPCTSTR ssid)
{
    HWND          hCmbEx     = NULL;
    HWND          hCmb       = NULL;
    LRESULT       lCount     = 0;
    COMBOBOXINFO  cbInfo;
    RECT          itemRect;
    TCHAR szBuf[1024];

    STDOUT << "Selecting network " << name << " for " << ssid << STDENDL;

    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }

    if ((hCmbEx = GetDlgItem(m_hWnd, ID_CONNECT_CMB)) == NULL) {
        STDOUT << "Failed to get combobox window" << STDENDL;
        return FALSE;
    }

    if ((hCmb = (HWND)SendMessage(hCmbEx, CBEM_GETCOMBOCONTROL, 
                                  0, 0)) == NULL) {
        STDOUT << "Failed to get combobox control info" << STDENDL;
		return FALSE;
	}

    if ((lCount = SendMessage(hCmb, CB_GETCOUNT, 
                              0, 0)) == CB_ERR) {
        STDOUT << "Failed to get combobox item count" << STDENDL;
		return FALSE;
	}

    if (strlen(name)) {
        sprintf(szBuf, "%s <%s>", name, ssid);
    }
    else {
        sprintf(szBuf, "<%s>", ssid);
    }

	CSharedMem shm(hCmbEx, ODYSSEY_DEFAULT_SHMEMSIZE);

    for (int lIndex = 0; lIndex < lCount; lIndex ++) {

        COMBOBOXEXITEM *pItem = (COMBOBOXEXITEM*)shm.Local();
        pItem->mask = CBEIF_TEXT;
        pItem->iItem = lIndex;
        pItem->cchTextMax = ODYSSEY_DEFAULT_SHMEMSIZE - sizeof(COMBOBOXEXITEM);
        pItem->pszText = (LPSTR)shm.Peer() + 1;

        shm.WriteMemory();
        if (SendMessage(hCmbEx, CBEM_GETITEM, 0, 
                        (LPARAM)shm.Peer()) == CB_ERR) {
            STDOUT << "Failed to get combobox item info" << STDENDL;
            return FALSE;
        }
        shm.ReadMemory();

        LPSTR itemName = (LPSTR)shm.Local() + 1;
        if (!strcmp(szBuf, itemName)) {
            STDOUT << "Found combo box item " << szBuf << STDENDL;
            break;
        }
    }
    if (lIndex == lCount) {
        STDOUT << "Can't find combo box item " << szBuf << STDENDL;
        return FALSE;
    }

    cbInfo.cbSize = sizeof(COMBOBOXINFO);
    if (! SendMessage(hCmb, CB_GETCOMBOBOXINFO, 
                      0, (LPARAM)&cbInfo)) {
        STDOUT << "Failed to get combo box info" << STDENDL;
		return FALSE;
	}

    if (! PostButton(hCmb, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(cbInfo.rcButton))
        || ! PostButton(hCmb, WM_LBUTTONUP, 0, &(cbInfo.rcButton))) {
        STDOUT << "Failed to send click event to combo box down arrow button" << STDENDL;
        return FALSE;
    }

    if (SendMessage(cbInfo.hwndList, LB_GETITEMRECT, 
                    lIndex, (LPARAM)&itemRect) == LB_ERR) {
        STDOUT << "Failed to get RECT for combobox item" << STDENDL;
		return FALSE;
	}

    if (! PostButton(cbInfo.hwndList, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(itemRect))
        || ! PostButton(cbInfo.hwndList, WM_LBUTTONUP, 0, &(itemRect))) {
        STDOUT << "Failed to send click event to combo box item" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}

BOOL 
Odyssey_ConnectionWindow::SelectDevice(LPCTSTR name)
{
    HWND          hCmb       = NULL;
    LRESULT       lCount     = 0;
    COMBOBOXINFO  cbInfo;
    RECT          itemRect;
    TCHAR szBuf[1024];

    STDOUT << "Selecting device " << name << STDENDL;

    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }

    if ((hCmb = GetDlgItem(m_hWnd, ID_DEVICE_CMB)) == NULL) {
        STDOUT << "Failed to get combobox window" << STDENDL;
        return FALSE;
    }

    if ((lCount = SendMessage(hCmb, CB_GETCOUNT, 
                              0, 0)) == CB_ERR) {
        STDOUT << "Failed to get combobox item count" << STDENDL;
		return FALSE;
	}

    for (int lIndex = 0; lIndex < lCount; lIndex ++) {

        if (SendMessage(hCmb, CB_GETLBTEXT, lIndex, 
                        (LPARAM)szBuf) == CB_ERR) { 
            STDOUT << "Failed to get combobox item text" << STDENDL;
            return FALSE;
        }

        if (strstr(szBuf, name)) {
            STDOUT << "Found combo box item " << szBuf << STDENDL;
            break;
        }
    }
    if (lIndex == lCount) {
        STDOUT << "Can't find combo box item " << name << STDENDL;
        return FALSE;
    }

    cbInfo.cbSize = sizeof(COMBOBOXINFO);
    if (! SendMessage(hCmb, CB_GETCOMBOBOXINFO, 
                      0, (LPARAM)&cbInfo)) {
        STDOUT << "Failed to get combo box info" << STDENDL;
		return FALSE;
	}

    if (! PostButton(hCmb, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(cbInfo.rcButton))
        || ! PostButton(hCmb, WM_LBUTTONUP, 0, &(cbInfo.rcButton))) {
        STDOUT << "Failed to send click event to combo box down arrow button" << STDENDL;
        return FALSE;
    }

    if (SendMessage(cbInfo.hwndList, LB_GETITEMRECT, 
                    lIndex, (LPARAM)&itemRect) == LB_ERR) {
        STDOUT << "Failed to get RECT for combobox item" << STDENDL;
		return FALSE;
	}

    if (! PostButton(cbInfo.hwndList, WM_LBUTTONDOWN, 
                     MK_LBUTTON, &(itemRect))
        || ! PostButton(cbInfo.hwndList, WM_LBUTTONUP, 0, &(itemRect))) {
        STDOUT << "Failed to send click event to combo box item" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}

BOOL 
Odyssey_ConnectionWindow::ReConnect()
{
    HWND hBtn   = NULL;
    if (m_hWnd == NULL) {
        STDOUT << "Invalid handle" << STDENDL;
        return FALSE;
    }

    STDOUT << "Reconnecting" << STDENDL;

    // click the "ReConnect"
    if ((hBtn = GetDlgItem(m_hWnd, ID_RECONNECT_BTN)) == NULL) {
        STDOUT << "Failed to get ReConnect window" << STDENDL;
        return FALSE;
    }
    if (! PostMessage(m_hWnd, WM_COMMAND,
                      MAKELONG(ID_RECONNECT_BTN, BN_CLICKED), 
                      (LPARAM)hBtn)) {
        STDOUT << "Failed to post click event to ReConnect window" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}

BOOL 
Odyssey_ConnectionWindow::ReAuthenticate()
{
    HWND hBtn   = NULL;
    if (m_hWnd == NULL) {
        STDOUT << "Invalid handle" << STDENDL;
        return FALSE;
    }

    STDOUT << "ReAuthenticate" << STDENDL;

    // click the "ReAuth"
    if ((hBtn = GetDlgItem(m_hWnd, ID_REAUTH_BTN)) == NULL) {
        STDOUT << "Failed to get ReAuth window" << STDENDL;
        return FALSE;
    }
    if (! PostMessage(m_hWnd, WM_COMMAND,
                      MAKELONG(ID_REAUTH_BTN, BN_CLICKED), 
                      (LPARAM)hBtn)) {
        STDOUT << "Failed to post click event to ReAuth window" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}

BOOL
Odyssey_ConnectionWindow::Status(LPSTR text, UINT tsize)
{
    static TCHAR status[256];

    HWND hEdit   = NULL;
    if (m_hWnd == NULL) {
        STDOUT << "Invalid handle" << STDENDL;
        return FALSE;
    }

    STDOUT << "Getting status" << STDENDL;

    // Get status window
    if ((hEdit = GetDlgItem(m_hWnd, ID_STATUS_LBL)) == NULL) {
        STDOUT << "Failed to get status window" << STDENDL;
        return FALSE;
    }

    if (SendMessage(hEdit, WM_GETTEXT, tsize, (LPARAM)text) <= 0) {
        STDOUT << "Failed to get status window" << STDENDL;
        return FALSE;
    }

    STDOUT << "Ok: " << text << STDENDL;
    return TRUE;
}

BOOL 
Odyssey_ConnectionWindow::GetConnection(LPSTR text, UINT tsize)
{
    HWND     hCmbEx     = NULL;
    HWND     hCmb       = NULL;
    LRESULT  lIndex     = 0;

    STDOUT << "Getting current connection" << STDENDL;

    if (m_Guard.Test(1)) {
        STDOUT << "Excessive retry" << STDENDL;
        return FALSE; // suppressing excessive attempts
    }

    if ((hCmbEx = GetDlgItem(m_hWnd, ID_CONNECT_CMB)) == NULL) {
        STDOUT << "Failed to get combobox window" << STDENDL;
        return FALSE;
    }

    if ((hCmb = (HWND)SendMessage(hCmbEx, CBEM_GETCOMBOCONTROL, 
                                  0, 0)) == NULL) {
        STDOUT << "Failed to get combobox control info" << STDENDL;
		return FALSE;
	}

    if ((lIndex = SendMessage(hCmb, CB_GETCURSEL, 0, 0)) == CB_ERR) { 
        STDOUT << "Failed to get combobox current selection" << STDENDL;
        return FALSE;
    }

	CSharedMem shm(hCmbEx, ODYSSEY_DEFAULT_SHMEMSIZE);

    COMBOBOXEXITEM *pItem = (COMBOBOXEXITEM*)shm.Local();
    pItem->mask = CBEIF_TEXT;
    pItem->iItem = lIndex;
    pItem->cchTextMax = ODYSSEY_DEFAULT_SHMEMSIZE - sizeof(COMBOBOXEXITEM);
    pItem->pszText = (LPSTR)shm.Peer() + 1;

    shm.WriteMemory();
    if (SendMessage(hCmbEx, CBEM_GETITEM, 0, 
                    (LPARAM)shm.Peer()) == CB_ERR) {
        STDOUT << "Failed to get combobox item info" << STDENDL;
        return FALSE;
    }
    shm.ReadMemory();

    LPSTR itemName = (LPSTR)shm.Local() + 1;
    strncpy(text, itemName, tsize);

    STDOUT << "Ok" << STDENDL;
    return TRUE;
}
