#pragma once

#include "PANARegistry.h"
#include "PANAPacNetworkSupport.h"

// CPANASetupDlg dialog

class CPANASetupDlg : public CDialog
{
	DECLARE_DYNAMIC(CPANASetupDlg)

public:
	CPANASetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPANASetupDlg();

// Dialog Data
	enum { IDD = IDD_SETUP };

// Access method
    CString &ConfigFilename() {
        return m_CfgData.m_strCfgFilename;
    }
    CString &ScriptFilename() {
        return m_CfgData.m_strScriptFilename;
    }
    CString &SharedSecret() {
        return m_CfgData.m_strSharedSecretFilename;
    }
    CString &AdapterName() {
        return m_CfgData.m_strAdaptername;
    }
    bool UseArchie() {
        return m_CfgData.m_dwUseArchie ? true : false;
    }
    bool RenewIPAddress() {
        return m_CfgData.m_dwRenewIPAddress ? true : false;
    }
    bool EnablePing() {
        return m_CfgData.m_dwEnablePing ? true : false;
    }
    bool EnableAPMonitor() {
        return m_CfgData.m_dwEnableAPMon ? true : false;
    }
    DWORD &AttemptTimeout() {
        return m_CfgData.m_dwAttemptTimeout;
    }
    DWORD &PingInterval() {
        return m_CfgData.m_dwPingInterval;
    }
    DWORD &EapAuthPeriod() {
        return m_CfgData.m_dwEapAuthPeriod;
    }
    PANA_MonitorAccessPoint *&NetSupport() {
        return m_NetSupport;
    }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedScriptBrowse();
    afx_msg void OnBnClickedCfgBrowse();
    afx_msg void OnBnClickedBrowseSharedsecret();
    afx_msg void OnBnClickedOk();
private:
    CEdit *m_CfgEditBox;
    CEdit *m_ScriptEditBox;
    CEdit *m_SharedSecretEditBox;
    CEdit *m_AttemptTimeoutEditBox;
    CEdit *m_PingIntervalEditBox;
    CEdit *m_EapAuthPeriodEditBox;
    CComboBox *m_AuthType;
    CButton *m_RenewIPCheckBox;
    CButton *m_EnablePingCheckBox;
    CButton *m_EnableAPMonCheckBox;
    CComboBox *m_AdapterNames;
    PaCConfig m_CfgData;
    PANA_MonitorAccessPoint *m_NetSupport;
private:
    void SetDropDownSize(CComboBox& box, UINT LinesToDisplay);
public:
    afx_msg void OnBnClickedApMonitor();
};
