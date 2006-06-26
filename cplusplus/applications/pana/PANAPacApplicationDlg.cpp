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
// PANAPacApplicationDlg.cpp : implementation file
//

#include <fstream>
#include <stdlib.h>
#include "stdafx.h"
#include "Iphlpapi.h"
#include "PANAPacApplication.h"
#include "PANAPacApplicationDlg.h"
#include "PANAIspSelectionDlg.h"
#include "PANAPacNetworkSupport.h"
#include ".\panapacapplicationdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void PANA_AppClientEvent::Success(std::string &ifname) 
{
    std::string errMsg = "Unknown Error";
    try {
        if (m_AppDlg.Setup().RenewIPAddress()) {
            PANA_DhcpRnewal dhcp;
            PANA_AdapterProperties adapter;
            if (! adapter.Get(ifname)) {
                errMsg = "Failed to resolve interface name";
                throw FALSE;
            }
            m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_IN_DHCP);
            if (! dhcp.Get(adapter.AdapterName(), errMsg, TRUE)) {
                throw FALSE;
            }
            else if (! adapter.Get(ifname)) {
                errMsg = "Failed to retrieve new IP address";
                throw FALSE;
            }
            std::string note = "Address change";
            PANA_CLIENT->UpdateAddress(adapter.CurrentAddress(), note);
        }
        m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_SUCCESS);
    }
    catch (...) {
        MessageBox(NULL, errMsg.data(), 
                   "PANA Error", 
                   MB_ICONHAND | MB_ICONSTOP | MB_ICONERROR);
        m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_FAILED);
    }
}

void PANA_AppClientEvent::Failure() 
{
    m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_FAILED);
}

void PANA_AppClientEvent::Disconnect() 
{
    m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_CANCELED);
}

PANA_CfgProviderInfo *PANA_AppClientEvent::IspSelection(const PANA_CfgProviderList &list)
{
    CIspSelectionDlg selectISP(const_cast<PANA_CfgProviderList&>(list));
	if (selectISP.DoModal() == IDOK) {
        m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_IN_PROGRESS);
	}
    else {
        m_AppDlg.SetButtonStates(CPANAPacApplicationDlg::BST_CANCELED);
    }
    return selectISP.Choice();
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPANAPacApplicationDlg dialog

CPANAPacApplicationDlg::CPANAPacApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPANAPacApplicationDlg::IDD, pParent),
      m_Event(*this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
}

void CPANAPacApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPANAPacApplicationDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_NEXT, OnBnClickedNext)
    ON_BN_CLICKED(IDC_CANCEL, OnBnClickedCancel)
    ON_BN_CLICKED(IDC_FINISH, OnBnClickedFinish)
    ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
    ON_BN_CLICKED(IDC_SETUP, OnBnClickedSetup)
END_MESSAGE_MAP()


// CPANAPacApplicationDlg message handlers

BOOL CPANAPacApplicationDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
	    CString strAboutMenu;
	    strAboutMenu.LoadString(IDS_ABOUTBOX);
	    if (!strAboutMenu.IsEmpty())
	    {
		    pSysMenu->AppendMenu(MF_SEPARATOR);
		    pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	    }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // Button references
    m_ProgressCtrl = reinterpret_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS));
    m_Status = reinterpret_cast<CStatic*>(GetDlgItem(IDC_STATUS));
    m_Next = reinterpret_cast<CButton*>(GetDlgItem(IDC_NEXT));
    m_Cancel = reinterpret_cast<CButton*>(GetDlgItem(IDC_CANCEL));
    m_Finish = reinterpret_cast<CButton*>(GetDlgItem(IDC_FINISH));

    // progress control setup
	m_nProgCtlTimerId = 0;
    m_nPingTimerId = 0;

    // set to inital state
    SetButtonStates(CPANAPacApplicationDlg::BST_INITIAL);

    // set net mon instance
    m_SetupDlg.NetSupport() = &m_APMonitor;

    // set update flag
    dwSendUpdate = 0;

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPANAPacApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPANAPacApplicationDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPANAPacApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPANAPacApplicationDlg::OnBnClickedNext()
{
    char bSecretBuf[64];
    ifstream fSharedSecret;
    fSharedSecret.open((LPCTSTR)m_SetupDlg.SharedSecret(),
                        ios::binary | ios::in);
    if (fSharedSecret.is_open()) {  // ios::operator void*()
        fSharedSecret.read(bSecretBuf, sizeof(bSecretBuf));
    }
    else {
        MessageBox("Can't open shared secret file", 
                   "PANA Error", 
                   MB_ICONHAND | MB_ICONSTOP | MB_ICONERROR);
        return;
    }
    fSharedSecret.close();

    PANA_CLIENT->Arg().m_SharedSecret.assign(bSecretBuf, sizeof(bSecretBuf));
    PANA_CLIENT->Arg().m_Md5Only = !m_SetupDlg.UseArchie();
    PANA_CLIENT->Arg().m_Timeout = m_SetupDlg.EapAuthPeriod();
    PANA_CLIENT->Arg().m_ThreadCount = PANA_CLIENT_DEFAULT_TCOUNT;
    PANA_CLIENT->Arg().m_PanaCfgFile = (LPCTSTR)m_SetupDlg.ConfigFilename();
    PANA_CLIENT->Arg().m_AuthScript = m_SetupDlg.ScriptFilename();

    if (PANA_CLIENT->Arg().m_PanaCfgFile.length() == 0) {
        MessageBox("No configuration file specified", 
                   "PANA Error", 
                   MB_ICONHAND | MB_ICONSTOP | MB_ICONERROR);
        return;
    }

    try {
        PANA_CLIENT->RegisterEvent(m_Event);
        SetButtonStates(CPANAPacApplicationDlg::BST_IN_GETISP);
        PANA_CLIENT->Start();
    }
    catch (...) {
        MessageBox("Failed to start PANA", 
                   "PANA Error", 
                   MB_ICONHAND | MB_ICONSTOP | MB_ICONERROR);
        SetButtonStates(CPANAPacApplicationDlg::BST_FAILED);
    }
}

void CPANAPacApplicationDlg::OnBnClickedCancel()
{
    SetButtonStates(CPANAPacApplicationDlg::BST_CANCEL_IN_PROGRESS);
    PANA_CLIENT->Stop();
    SetButtonStates(CPANAPacApplicationDlg::BST_CANCELED);
}

void CPANAPacApplicationDlg::OnBnClickedFinish()
{
    std::string errMsg;
    SetButtonStates(CPANAPacApplicationDlg::BST_FINISH);
    PANA_CLIENT->Stop();
    OnCancel();
}

void CPANAPacApplicationDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent) {
        case TID_PROGRESS_CTL:
	        m_ProgressCtrl->StepIt();
	        if (m_ProgressCtrl->GetPos() == m_SetupDlg.AttemptTimeout()) {
                SetButtonStates(CPANAPacApplicationDlg::BST_FAILED);
	        }
            break;
        case TID_PING:
            PANA_CLIENT->SendPing();
            break;
        case TID_AP_MONITOR:
            // check for AP changes
            std::string ifname = (LPCTSTR)m_SetupDlg.AdapterName();
            if (m_APMonitor.Update(ifname) == S_OK) {
                dwSendUpdate |= 0x0001;
                ACE_DEBUG((LM_ERROR, "(%P|%t) Adapter change detected: %s\n",
                           m_APMonitor.CurrentAP().data()));
            }

            // make sure there's an address before sending
            ACE_UINT32 ipAddr;
            std::string errMsg;
            PANA_AdapterProperties adapter;
            if (dwSendUpdate &&
                adapter.Get(ifname) &&
                ((ipAddr = adapter.CurrentAddress().get_ip_address()) != 0)) {
                char buf[32];
                adapter.CurrentAddress().addr_to_string(buf, sizeof(buf));
                ACE_DEBUG((LM_ERROR, "(%P|%t) IP address detected: %s\n", buf));
                // check for microsoft 169.254.x.x private address
                if ((ipAddr & 0xA9FE0000) == 0xA9FE0000) {
                    ACE_DEBUG((LM_ERROR, "(%P|%t) Win32 private address\n"));
                }
                else {
                    ACE_DEBUG((LM_ERROR, "(%P|%t) Valid address\n"));
                    dwSendUpdate |= 0x0002;
                }
            }

            // attempt to send
            if ((dwSendUpdate & 0x0003) == 0x0003) {
                std::string message = m_APMonitor.CurrentAP() + ":" +
                                      m_APMonitor.CurrentMaC();
                dwSendUpdate = 0;

                // wait to stabilize
                ACE_Time_Value tout(3, 0);
                ACE_OS::sleep(tout);

                // sending address update and notification
                ACE_DEBUG((LM_ERROR, "(%P|%t) Sending notification\n"));
                // PANA_CLIENT->UpdateAddress(adapter.CurrentAddress(),
                //                            message);

                // also send using sendudp 
                std::string sysCmd = "\"" + PANA_CLIENT->Arg().m_AuthScript + "\" ";
                sysCmd += " update NOTIFICATION: ";
                sysCmd += message;
                ACE_DEBUG((LM_INFO, "(%P|%t) Executing: %s\n",
                           sysCmd.data()));
                system(sysCmd.data());
            }
            break;
    }
}

void CPANAPacApplicationDlg::SetButtonStates(BSTATES st)
{
    typedef enum {
        POS_DISC = 1,
        POS_RESET = 0,
        POS_IGNORE = -1,
        POS_INIT = -2,
        POS_CONNECTED = -3,
    };

    typedef struct {
        CHAR *m_Text;
        BOOL m_Cancel;
        BOOL m_Next;
        BOOL m_Finish;
        INT m_Position;
    } ButtonStatus;

    ButtonStatus BtnStatus[BST_FINISH+1] = {
        { "Status: Not Connected", FALSE, TRUE, TRUE, POS_RESET }, // BST_INITIAL
        { "Status: Retrieving ISP information ...", TRUE, FALSE, FALSE, POS_DISC }, // BST_GETISP
        { "Status: Attempting to Connect ...", TRUE, FALSE, FALSE, POS_INIT }, // BST_IN_PROGRESS
        { "Status: Authenticated, Renewing DHCP ...", FALSE, FALSE, FALSE, POS_IGNORE }, // BST_IN_DHCP
        { "Status: Successfully Authenticated", TRUE, FALSE, TRUE, POS_CONNECTED }, // BST_SUCCESS
        { "Status: Connection attempt failed", FALSE, TRUE, TRUE, POS_RESET }, // BST_FAILED
        { "Status: Disconnecting ... pls. wait", FALSE, FALSE, FALSE, POS_RESET }, // BST_CANCEL_IN_PROGRESS
        { "Status: Disconnected", FALSE, TRUE, TRUE, POS_RESET }, // BST_CANCELED
        { "Status: Exiting ... pls. wait", FALSE, FALSE, FALSE, POS_RESET } // BST_FINISH
    };

    m_Status->SetWindowText(BtnStatus[st].m_Text);
    m_Cancel->EnableWindow(BtnStatus[st].m_Cancel);
    m_Next->EnableWindow(BtnStatus[st].m_Next);
    m_Finish->EnableWindow(BtnStatus[st].m_Finish);

    if (BtnStatus[st].m_Position == POS_INIT) {
        m_nProgCtlTimerId = SetTimer(TID_PROGRESS_CTL, 1000, 0);
        m_ProgressCtrl->SetRange(1, (short)m_SetupDlg.AttemptTimeout());
        m_ProgressCtrl->SetStep(1);
    }
    else if ((BtnStatus[st].m_Position == POS_RESET) ||
             (BtnStatus[st].m_Position == POS_CONNECTED)) {
        int pos = (BtnStatus[st].m_Position == POS_CONNECTED) ?
                   m_SetupDlg.AttemptTimeout() : 0;
        m_ProgressCtrl->SetPos(pos);
        KillTimer(m_nProgCtlTimerId);
    }
    else {
        m_ProgressCtrl->SetPos(BtnStatus[st].m_Position);
    }

    if (m_SetupDlg.EnablePing()) {
        if (st == BST_SUCCESS) {
            m_nPingTimerId = SetTimer(TID_PING, 
                    m_SetupDlg.PingInterval() * 1000, 0);
        }
        else if ((st == BST_CANCELED) ||
                (st == BST_FINISH)) {
            KillTimer(m_nPingTimerId);
        }
    }

    if (m_SetupDlg.EnableAPMonitor()) {
        if (st == BST_SUCCESS) {
            m_nAPMonitorTimerId = SetTimer(TID_AP_MONITOR, 500, 0);
        }
        else if ((st == BST_CANCELED) ||
                (st == BST_FINISH)) {
            KillTimer(m_nAPMonitorTimerId);
        }
    }
}

void CPANAPacApplicationDlg::OnBnClickedBrowse()
{
    static char BASED_CODE szFilter[] = "PANA Cfg Files (*.xml)|*.xml|XML Files (*.xml)|All Files (*.*)|*.*||";
    CFileDialog BrowseDlg(TRUE, 
                          NULL, 
                          NULL, 
                          OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                          szFilter);
    if (BrowseDlg.DoModal() == IDOK) {
       CString FullName = BrowseDlg.GetPathName();
    }
}


void CPANAPacApplicationDlg::OnBnClickedSetup()
{
    m_SetupDlg.DoModal();
}

