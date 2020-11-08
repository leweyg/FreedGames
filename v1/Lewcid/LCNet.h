
//
// The Lewcid Networking system
//
// This is the first rewrite and it should change drastically
//

#include "gennet.h"

bool AskHostOrClient(bool* dohost)
{
	int res = MessageBox(0, 
		"Do you wish to host a game?\n\nYes for Host\nNo for Connect to Host", 
		"Connect Option", MB_YESNOCANCEL);

	if (res == IDYES)
	{
		*dohost = true;
		return true;
	}
	if (res == IDNO)
	{
		*dohost = false;
		return true;
	}

	return false;
}

char LN_Buffer[400];
bool LN_IsHosting;
Net_Transport* LN_Trans;
HWND LN_Dlg;

void ln_HostConnected(Net_Transport* trans, void* extra)
{
	SendMessage(LN_Dlg, WM_COMMAND, IDOK, 0);
}

LRESULT CALLBACK LN_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			{
				LN_Dlg = hDlg;
				if (!LN_IsHosting)
				{
					SetWindowText(hDlg, "Connecting");
					SetDlgItemText(hDlg, IDC_Host_IP, "");
					SetDlgItemText(hDlg, IDC_Host_Info, "Enter the host IP above and hit 'Connect'");
					//SetDlgItemText(hDlg, IDC_Host_Top, "Enter the host IP below:");
					EnableWindow(GetDlgItem(hDlg,IDC_Host_LanIP),false);
				}
				else
				{
					//EnableWindow(GetDlgItem(hDlg,IDC_Host_IP),false);
					Net_GetMyIP(LN_Buffer);
					SetDlgItemText(hDlg, IDC_Host_IP, LN_Buffer);
					EnableWindow(GetDlgItem(hDlg,IDC_Host_Connect),false);

					LN_Trans->Host_Setup();
					LN_Trans->Host_WaitForClient_Begin(ln_HostConnected, 0);
				}
			}
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_Host_Connect:
				{
					GetDlgItemText(hDlg, IDC_Host_IP, LN_Buffer, 400);
					if (!LN_Trans->Client_Setup(LN_Buffer))
					{
						MessageBox(hDlg, "Couldn't connect", "Net Error", MB_OK);
					}
					else
					{
						//MessageBox(0, "Client Connected", "Woot", MB_OK);
						EndDialog(hDlg, IDOK);
					}
				}
				break;
			case IDC_Host_LanIP:
				{
					Net_GetMyLANIP(LN_Buffer);
					SetDlgItemText(hDlg, IDC_Host_IP, LN_Buffer);
				}
				break;
			case IDCANCEL:
				if (LN_IsHosting)
				{
					LN_Trans->Host_WaitForClient_Cancel();
				}
			case IDOK:
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
			}
			break;
	}
    return FALSE;
}

bool SetupRemoting(Net_Transport* transport)
{
	if (transport->mOpen)
	{
		ShowMessage("Already Connected", "Network Error");
		return false;
	}
	if (!AskHostOrClient(&LN_IsHosting))
		return false;

	LN_Trans = transport;
	int res = DialogBox(0, (LPCTSTR)IDD_Host_Hosting, 0, (DLGPROC)LN_DlgProc);
	if (res==IDOK)
		return true;

	return false;
}