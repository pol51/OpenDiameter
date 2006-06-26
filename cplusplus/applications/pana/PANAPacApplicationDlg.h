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
// PANAPacApplicationDlg.h : header file
//

#pragma once

#include "ace/Log_Msg_Callback.h"
#include "ace/Log_Record.h"
#include "PanaClient.h"
#include "PANASetupDlg.h"
#include "PANAPaCNetworkSupport.h"

class CPANAPacApplicationDlg;
class PANA_AppClientEvent : public PANAClientEvent
{
public:
    /// Constructor
    PANA_AppClientEvent(CPANAPacApplicationDlg &dlg) : 
         m_AppDlg(dlg) { 
    }
    /// destructor
    virtual ~PANA_AppClientEvent() { 
    }

    void Success(std::string &ifname);
    virtual void Failure();
    virtual void Disconnect();

    virtual PANA_CfgProviderInfo *IspSelection(const PANA_CfgProviderList &list);

private:
    CPANAPacApplicationDlg &m_AppDlg;
};

class PANA_APMonitor : public PANA_MonitorAccessPoint
{
public:
    HRESULT Update(std::string &ifname) {
        std::string ssid;
        HRESULT hRes = GetSSId(ifname, ssid);
        if (SUCCEEDED(hRes)) {
            std::string mac;
            if (m_CurrentAP != ssid) {
                m_CurrentAP = ssid;
                if (GetMaC(ifname, mac) == S_OK) {
                    m_CurrentMaC = mac;
                }
                return hRes;
            }
        }
        return E_FAIL;
    }
    std::string &CurrentAP() {
        return m_CurrentAP;
    }
    std::string &CurrentMaC() {
        return m_CurrentMaC;
    }
private:
    std::string m_CurrentAP;
    std::string m_CurrentMaC;
};

// CPANAPacApplicationDlg dialog
class CPANAPacApplicationDlg : public CDialog
{
// Construction
public:
	CPANAPacApplicationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PANAPACAPPLICATION_DIALOG };

     typedef enum {
        BST_INITIAL,
        BST_IN_GETISP,
        BST_IN_PROGRESS,
        BST_IN_DHCP,
        BST_SUCCESS,
        BST_FAILED,
        BST_CANCEL_IN_PROGRESS,
        BST_CANCELED,
        BST_FINISH
    } BSTATES;
    void SetButtonStates(BSTATES st);

// Access methods
    CPANASetupDlg &Setup() {
        return m_SetupDlg;
    }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
    CStatic *m_Status;
    CButton *m_Next;
    CButton *m_Cancel;
    CButton *m_Finish;
	CProgressCtrl *m_ProgressCtrl;
	UINT_PTR m_nProgCtlTimerId;
    UINT_PTR m_nPingTimerId;
    UINT_PTR m_nAPMonitorTimerId;
    CPANASetupDlg m_SetupDlg;
    PANA_APMonitor m_APMonitor;
    DWORD dwSendUpdate;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
    afx_msg void OnBnClickedNext();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedFinish();
    afx_msg void OnBnClickedBrowse();
    afx_msg void OnBnClickedSetup();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
    PANA_AppClientEvent m_Event;

    typedef enum {
        TID_PROGRESS_CTL = 1,
        TID_PING,
        TID_AP_MONITOR
    }; 

    friend PANA_AppClientEvent;
};
