// PANASetupDlg.cpp : implementation file
//

#include <direct.h>
#include "stdafx.h"
#include "PANAPacApplication.h"
#include "PANASetupDlg.h"
#include "PANAPacNetworkSupport.h"
#include ".\panasetupdlg.h"


// CPANASetupDlg dialog

IMPLEMENT_DYNAMIC(CPANASetupDlg, CDialog)
CPANASetupDlg::CPANASetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPANASetupDlg::IDD, pParent),
    m_NetSupport(NULL)
{
    CPANA_PaCRegistry registry;
    registry.Default(m_CfgData);
    registry.Load(m_CfgData);
}

CPANASetupDlg::~CPANASetupDlg()
{
}

void CPANASetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPANASetupDlg, CDialog)
    ON_BN_CLICKED(IDC_SCRIPT_BROWSE, OnBnClickedScriptBrowse)
    ON_BN_CLICKED(IDC_CFG_BROWSE, OnBnClickedCfgBrowse)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BROWSE_SHAREDSECRET, OnBnClickedBrowseSharedsecret)
    ON_BN_CLICKED(IDC_AP_MONITOR, OnBnClickedApMonitor)
END_MESSAGE_MAP()

BOOL CPANASetupDlg::OnInitDialog()
{
    char szBuf[32];
	CDialog::OnInitDialog();

    m_AuthType = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_COMBO_AUTHTYPE));
    m_CfgEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_CFGFILE));
    m_ScriptEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_SCRIPTFILE));
    m_SharedSecretEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_SHAREDSECRET));
    m_AttemptTimeoutEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_ATTEMPTTIMEOUT));
    m_PingIntervalEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_PINGINTERVAL));
    m_EapAuthPeriodEditBox = reinterpret_cast<CEdit*>(GetDlgItem(IDC_EAPAUTHPERIOD));
    m_RenewIPCheckBox = reinterpret_cast<CButton*>(GetDlgItem(IDC_RENEW_IP));
    m_EnablePingCheckBox = reinterpret_cast<CButton*>(GetDlgItem(IDC_PING));
    m_EnableAPMonCheckBox = reinterpret_cast<CButton*>(GetDlgItem(IDC_AP_MONITOR));
    m_AdapterNames = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_ADAPTER_LIST));

    m_CfgEditBox->SetWindowText((LPCTSTR)m_CfgData.m_strCfgFilename);
    m_ScriptEditBox->SetWindowText((LPCTSTR)m_CfgData.m_strScriptFilename);
    m_SharedSecretEditBox->SetWindowText((LPCTSTR)m_CfgData.m_strSharedSecretFilename);
    m_RenewIPCheckBox->SetCheck(m_CfgData.m_dwRenewIPAddress ? BST_CHECKED : BST_UNCHECKED);
    m_EnablePingCheckBox->SetCheck(m_CfgData.m_dwEnablePing ? BST_CHECKED : BST_UNCHECKED);
    m_EnableAPMonCheckBox->SetCheck(m_CfgData.m_dwEnableAPMon ? BST_CHECKED : BST_UNCHECKED);
    sprintf(szBuf, "%d", m_CfgData.m_dwAttemptTimeout);
    m_AttemptTimeoutEditBox->SetWindowText((LPCTSTR)szBuf);
    sprintf(szBuf, "%d", m_CfgData.m_dwPingInterval);
    m_PingIntervalEditBox->SetWindowText((LPCTSTR)szBuf);
    sprintf(szBuf, "%d", m_CfgData.m_dwEapAuthPeriod);
    m_EapAuthPeriodEditBox->SetWindowText((LPCTSTR)szBuf);

    // auth type setup
    SetDropDownSize(*m_AuthType, 2);
    m_AuthType->AddString(LPCTSTR("MD5")); 
    m_AuthType->AddString(LPCTSTR("Archie")); // 0 index
    m_AuthType->SetCurSel(m_CfgData.m_dwUseArchie ? 0 : 1);

    // ssid monitor
    if (m_NetSupport) {
        int selection = 0, index = 0;
        SetDropDownSize(*m_AdapterNames, 5);
        PANA_SSID_LIST SsidList;
        m_NetSupport->EnumerateDevices(SsidList);
        for (PANA_SSID_LIST::iterator i = SsidList.begin();
             i != SsidList.end(); i ++, index ++) {
             m_AdapterNames->AddString((*i).data());
             CString lookup((*i).data());
             if (lookup == m_CfgData.m_strAdaptername) {
                 selection = index;
             }
        }
        m_AdapterNames->SetCurSel(selection);
        if (! (m_EnableAPMonCheckBox->GetState() & 0x0003)) {
            m_AdapterNames->EnableWindow(FALSE);
        }
    }
    else {
        m_CfgData.m_strAdaptername = "";
        m_CfgData.m_dwEnableAPMon = FALSE;
        m_EnableAPMonCheckBox->EnableWindow(FALSE);
        m_AdapterNames->EnableWindow(FALSE);
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// CPANASetupDlg message handlers

void CPANASetupDlg::OnBnClickedScriptBrowse()
{
    static char BASED_CODE szFilter[] = "PANA Auth Script (*.bat)|*.bat|Batch Files (*.bat)|All Files (*.*)|*.*||";
    CFileDialog BrowseDlg(TRUE, 
                          NULL, 
                          NULL, 
                          OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                          szFilter);
    if (BrowseDlg.DoModal() == IDOK) {
        m_ScriptEditBox->SetWindowText((LPCTSTR)BrowseDlg.GetPathName());
    }
}

void CPANASetupDlg::OnBnClickedCfgBrowse()
{
    static char BASED_CODE szFilter[] = "PANA Cfg Files (*.xml)|*.xml|XML Files (*.xml)|All Files (*.*)|*.*||";
    CFileDialog BrowseDlg(TRUE, 
                          NULL, 
                          NULL, 
                          OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                          szFilter);
    if (BrowseDlg.DoModal() == IDOK) {
        m_CfgEditBox->SetWindowText((LPCTSTR)BrowseDlg.GetPathName());
    }
}

void CPANASetupDlg::OnBnClickedBrowseSharedsecret()
{
    static char BASED_CODE szFilter[] = "Binary File (*.bin)|*.bin|BIN Files (*.xml)|All Files (*.*)|*.*||";
    CFileDialog BrowseDlg(TRUE, 
                          NULL, 
                          NULL, 
                          OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                          szFilter);
    if (BrowseDlg.DoModal() == IDOK) {
        m_SharedSecretEditBox->SetWindowText((LPCTSTR)BrowseDlg.GetPathName());
    }
}

void CPANASetupDlg::SetDropDownSize(CComboBox& box, UINT LinesToDisplay)
{
    CRect cbSize;
    int Height;

    box.GetClientRect(cbSize);
    Height = box.GetItemHeight(-1);
    Height += box.GetItemHeight(0) * LinesToDisplay;

    Height += GetSystemMetrics(SM_CYEDGE) * 2;
    Height += GetSystemMetrics(SM_CYEDGE) * 2;

    box.SetWindowPos(NULL,
        0, 0,
        cbSize.right, Height,
        SWP_NOMOVE | SWP_NOZORDER
        );
}


void CPANASetupDlg::OnBnClickedOk()
{
    char szBuf[32];
    m_CfgEditBox->GetWindowText(m_CfgData.m_strCfgFilename);
    m_ScriptEditBox->GetWindowText(m_CfgData.m_strScriptFilename);
    m_SharedSecretEditBox->GetWindowText(m_CfgData.m_strSharedSecretFilename);
    m_AdapterNames->GetWindowText(m_CfgData.m_strAdaptername);
    m_CfgData.m_dwUseArchie = (m_AuthType->GetCurSel() == 0) ? 1 :0;
    m_CfgData.m_dwRenewIPAddress = (m_RenewIPCheckBox->GetState() & 0x0003) ? 1 :0;
    m_CfgData.m_dwEnablePing = (m_EnablePingCheckBox->GetState() & 0x0003) ? 1 :0;
    m_CfgData.m_dwEnableAPMon = (m_EnableAPMonCheckBox->GetState() & 0x0003) ? 1 :0;
    m_AttemptTimeoutEditBox->GetWindowText(szBuf, sizeof(szBuf));
    m_CfgData.m_dwAttemptTimeout = atol(szBuf);
    m_PingIntervalEditBox->GetWindowText(szBuf, sizeof(szBuf));
    m_CfgData.m_dwPingInterval = atol(szBuf);
    m_EapAuthPeriodEditBox->GetWindowText(szBuf, sizeof(szBuf));
    m_CfgData.m_dwEapAuthPeriod = atol(szBuf);    

    CPANA_PaCRegistry registry;
    registry.Save(m_CfgData);

    OnOK();
}



void CPANASetupDlg::OnBnClickedApMonitor()
{
    BOOL bEnabled = (m_EnableAPMonCheckBox->GetState() & 0x0003) ? 
                     TRUE : FALSE;
    m_AdapterNames->EnableWindow(bEnabled);
}
