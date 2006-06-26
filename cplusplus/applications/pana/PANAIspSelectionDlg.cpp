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

#include "stdafx.h"
#include "PANAPacApplication.h"
#include "PANAIspSelectionDlg.h"
#include ".\panaispselectiondlg.h"
#include "PanaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// #define  SUPPORT_INFO_TIP // Informational tips in ISP selection

CIspSelectionDlg::CIspSelectionDlg(PANA_CfgProviderList &list, 
                                   CWnd* pParent) : 
      CDialog(CIspSelectionDlg::IDD, pParent),
      m_Selection(list),
      m_Choice(NULL),
      m_pImageList(NULL),
      m_List(NULL)
{
}

CIspSelectionDlg::~CIspSelectionDlg() 
{
    delete m_pImageList;
}

void CIspSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIspSelectionDlg, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
    ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_ISP, OnLvnItemActivateListIsp)
    ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST_ISP, OnLvnGetInfoTipListIsp)
END_MESSAGE_MAP()

BOOL CIspSelectionDlg::OnInitDialog()
{
  	CDialog::OnInitDialog();

	m_List = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_LIST_ISP));

    // create, initialize, and hook up image list
    m_pImageList = new CImageList();
    ASSERT(m_pImageList != NULL);    // serious allocation failure checking
    m_pImageList->Create(48, 48, ILC_COLOR8, 0, 4);
    m_pImageList->Add(AfxGetApp()->LoadIcon(IDI_ICON1));
    m_pImageList->Add(AfxGetApp()->LoadIcon(IDI_ICON2));
    m_pImageList->Add(AfxGetApp()->LoadIcon(IDI_ICON3));
    m_pImageList->Add(AfxGetApp()->LoadIcon(IDI_ICON4));
    m_List->SetImageList(m_pImageList, LVSIL_NORMAL);

    CHAR szText[128];
    int ItemIndex = 0, ImageIndex = 0;
    PANA_CfgProviderList::iterator i;
    for (i = m_Selection.begin(); i != m_Selection.end(); i++) 
    {
        PANA_CfgProviderInfo *p = (*i);
        if (p->m_Name.length() > 0) {
            sprintf(szText, "%s, Id is %d\n",
                    p->m_Name.data(), p->m_Id);
#if defined(SUPPORT_INFO_TIP)
            CHAR *ptr = strchr(szText, ';');
            if (ptr) {
                *ptr = NULL;
            }
#endif
            m_List->InsertItem(ItemIndex, szText, ImageIndex);
            m_List->SetItemData(ItemIndex, (DWORD_PTR)p);
            ItemIndex ++;
            ImageIndex = (ImageIndex >= 3) ? 0 : ImageIndex + 1;
        }
    }
    m_List->SetSelectionMark(0);
    m_List->SetExtendedStyle(m_List->GetExtendedStyle()|LVS_EX_INFOTIP);

	m_Username = reinterpret_cast<CEdit*>(GetDlgItem(IDC_USERNAME));
    m_Password = reinterpret_cast<CEdit*>(GetDlgItem(IDC_PASSWORD));
    m_VlanName = reinterpret_cast<CEdit*>(GetDlgItem(IDC_VLAN));

    m_Username->SetWindowText(LPCTSTR(""));
    m_Password->SetWindowText(LPCTSTR(""));
    m_VlanName->SetWindowText(LPCTSTR(""));

    if (! PANA_CLIENT->Arg().m_Md5Only) {
        m_Password->EnableWindow(FALSE);
    }
    return TRUE;
}

void CIspSelectionDlg::OnBnClickedOk()
{
    char StringBuf[128];

    int ItemIndex = m_List->GetSelectionMark();
    if (ItemIndex >= 0) 
    {
        m_Choice = (PANA_CfgProviderInfo*)m_List->GetItemData(ItemIndex);
    }

    m_Username->GetWindowText(StringBuf, 128);
    PANA_CLIENT->Arg().m_Username = StringBuf;

#if (USE_DOMAIN_NAME)
    m_VlanName->GetWindowText(StringBuf, 128);
    if (strlen(StringBuf) > 0) {
        PANA_CLIENT->Arg().m_Username += "@";
        PANA_CLIENT->Arg().m_Username += StringBuf;
    }
#endif

    m_Password->GetWindowText(StringBuf, 128);
    PANA_CLIENT->Arg().m_Password = StringBuf;

    OnOK();
}

void CIspSelectionDlg::OnBnClickedCancel()
{
    m_Choice = NULL;
    OnCancel();
}

void CIspSelectionDlg::OnLvnItemActivateListIsp(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int ItemIndex = m_List->GetSelectionMark();
    if (ItemIndex >= 0) 
    {
        m_Choice = (PANA_CfgProviderInfo*)m_List->GetItemData(ItemIndex);
		PopulateControls(m_Choice);
    }
	else {
		m_Choice = NULL;
	}
#if defined(AUTO_EXIT_ON_SELECT)
    CIspSelectionDlg::OnBnClickedOk();
#endif
    *pResult = 0;
}

void CIspSelectionDlg::OnLvnGetInfoTipListIsp(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
    PANA_CfgProviderInfo *item = (PANA_CfgProviderInfo*)m_List->GetItemData(pGetInfoTip->iItem);
    if (item) {
#if defined(SUPPORT_INFO_TIP)
        CHAR StringBuf[128];
        strcpy(StringBuf, item->m_Name.data());
        CHAR Seps[] = ";";
        CHAR *Tk = strtok(StringBuf, Seps);
        int skips = 0;  // skip description and info tip
        do {
            if (skips == 1) {
                break;
            }
            skips ++;
            Tk = strtok(NULL, Seps);
        } while (Tk);
        if (Tk) {
            if (strlen(Tk) > (unsigned)pGetInfoTip->cchTextMax) {
                if (pGetInfoTip->dwFlags) {
                    strncpy(pGetInfoTip->pszText, Tk, pGetInfoTip->cchTextMax - 1);
                }
                else {
                    strncat(pGetInfoTip->pszText, Tk, pGetInfoTip->cchTextMax - 1);
                }
            }
            else {
                if (pGetInfoTip->dwFlags) {
                    strcpy(pGetInfoTip->pszText, Tk);
                }
                else {
                    strcat(pGetInfoTip->pszText, Tk);
                }
            }
        }
#else
        sprintf(pGetInfoTip->pszText, "%s\n", 
                item->m_Name.data());
#endif
		PopulateControls(item);
    }
    *pResult = 0;
}

void CIspSelectionDlg::PopulateControls(PANA_CfgProviderInfo *info)
{
    CHAR StringBuf[128];
#if defined(SUPPORT_INFO_TIP)
    INT i = 0;
    CHAR Seps[] = ";";
    CHAR *Tokens[] = { 0, 0, 0 };
    CEdit *Boxes[] = { m_Username, m_Password, m_VlanName };

    strcpy(StringBuf, info->m_Name.data());
    CHAR *Tk = strtok(StringBuf, Seps);
    int skips = 0;  // skip description and info tip
    do {
        if (skips == 1) {
            break;
        }
        skips ++;
        Tk = strtok(NULL, Seps);
    } while (Tk);

    if (Tk) {
        Tk = strtok(NULL, Seps); 
        while (Tk && i < sizeof(Tokens)/sizeof(CHAR*)) {
            Tokens[i++] = Tk;
            Tk = strtok(NULL, Seps);
        }
        for (i = 0; i < sizeof(Boxes)/sizeof(CEdit*); i++) {
            if (Tokens[i]) {
               Boxes[i]->SetWindowText(Tokens[i]);
            }
        }
    }
#else
    strcpy(StringBuf, info->m_Name.data());
    CHAR *Tk = strtok(StringBuf, "-");
    m_VlanName->SetWindowText(StringBuf);
#endif
}

