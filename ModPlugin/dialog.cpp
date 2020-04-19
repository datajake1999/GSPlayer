#include "ModPlugin.h"
#include "commctrl.h"
#include "Resource.h"
#include <commdlg.h>
#ifdef _WIN32_WCE
#include "wce\wcehelper.h"
#endif

extern HINSTANCE M_hInst;
/*extern BOOL g_fUpdateConfig;*/

extern BOOL M_CalculateLength;
extern int Save_BitsPerSample;
extern int Save_SampleRate;
extern int Save_Channels;

//Tuning Parameters:
extern BOOL M_Loop; //Looping
extern int M_Ramp; //Ramping : 0 - off, 1 - normal, 2 - sensitive
extern BOOL M_NonInter; //Non-Interpolated Mixing
extern int M_ModMode; //Mod Playback Mode.  0 - Normal, 1 - ProTracker 1, 2 - FastTracker 2
extern BOOL M_PosReset; //Stop all notes when moving position (using BASSMOD_MusicSetPosition or BASSMOD_MusicPlayEx).  
extern int M_Surround; //Surround Sound.  0 - Off, 1 - Mode 1, 2 = Mode 2
extern BOOL M_StopBack; //Stop the music when a backward jump effect is played. This stops musics that never 
						 //reach the end from going into endless loops. Some MOD musics are designed to jump all 
						 //over the place, so this flag would cause those to be stopped prematurely. If this flag is 
						 //used together with the BASS_MUSIC_LOOP flag, then the music would not be stopped but any 
						 //BASS_SYNC_END sync would be called.  


//Sampling Rate Constants
const int s_nSampleRate[] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 0};

//Initialisation Handler
BOOL ConfigDialogOnInitDialog(HWND hwndDlg)
{
	//Temporary Variables:
	int i, n;
	TCHAR sz[MAX_PATH];

	//Load up the Sampling Rate Combobox
	for (i = 0; s_nSampleRate[i]; i++) 
	{
		//Convert the number to a string:
		wsprintf(sz, _T("%d"), s_nSampleRate[i]);
		//Add to the combobox:
		n = SendMessage(GetDlgItem(hwndDlg, IDC_SAMPLERATE), CB_ADDSTRING, 0, (LPARAM)sz);
		//If we find the selected sampling rate, select it
		if (s_nSampleRate[i] == Save_SampleRate)
			SendMessage(GetDlgItem(hwndDlg, IDC_SAMPLERATE), CB_SETCURSEL, n, 0);
	}
	//Add Ramping Options:
	for (i=0; i<3; i++)
	{
		switch(i)
		{
			case 0:
				_tcscpy(sz, _T("No Ramping"));
				break;
			case 1:
				_tcscpy(sz, _T("Normal Ramping"));
				break;
			case 2:
				_tcscpy(sz, _T("Sensitive Ramping"));
				break;
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_RAMPING), CB_ADDSTRING, 0, (LPARAM)sz);
	}

	//Set the ramping selection:
	SendMessage(GetDlgItem(hwndDlg, IDC_RAMPING), CB_SETCURSEL, M_Ramp, 0);

	//Add Surround Sound Options:
	for (i=0; i<3; i++)
	{
		switch(i)
		{
			case 0:
				_tcscpy(sz, _T("Off"));
				break;
			case 1:
				_tcscpy(sz, _T("Mode 1"));
				break;
			case 2:
				_tcscpy(sz, _T("Mode 2"));
				break;
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_SURROUND), CB_ADDSTRING, 0, (LPARAM)sz);
	}

	//Set the Surround Sound selection:
	SendMessage(GetDlgItem(hwndDlg, IDC_SURROUND), CB_SETCURSEL, M_Surround, 0);

	//Add Mod Playback Mode Options:
	for (i=0; i<3; i++)
	{
		switch(i)
		{
			case 0:
				_tcscpy(sz, _T("Normal"));
				break;
			case 1:
				_tcscpy(sz, _T("ProTracker Mode"));
				break;
			case 2:
				_tcscpy(sz, _T("FastTracker 2 Mode"));
				break;
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_MODMODE), CB_ADDSTRING, 0, (LPARAM)sz);
	}

	//Set the Mod Playback Mode selection:
	SendMessage(GetDlgItem(hwndDlg, IDC_MODMODE), CB_SETCURSEL, M_ModMode, 0);

	//Set Checkboxes:
	if (Save_BitsPerSample == 16)
		SendMessage(GetDlgItem(hwndDlg, IDC_16BIT), BM_SETCHECK, 1, 0);
	
	if (Save_Channels == 2)
		SendMessage(GetDlgItem(hwndDlg, IDC_STEREO), BM_SETCHECK, 1, 0);

	if (!M_NonInter)
		SendMessage(GetDlgItem(hwndDlg, IDC_INTERPOLATED), BM_SETCHECK, 1, 0);	

	if (M_Loop)
		SendMessage(GetDlgItem(hwndDlg, IDC_LOOP), BM_SETCHECK, 1, 0);	

	if (M_StopBack)
		SendMessage(GetDlgItem(hwndDlg, IDC_JUMPBACK), BM_SETCHECK, 1, 0);	

	if (M_PosReset)
		SendMessage(GetDlgItem(hwndDlg, IDC_POSRESET), BM_SETCHECK, 1, 0);	

	//Indicate that initialisation was successful
	return TRUE;
}

BOOL ConfigDialogOnOK(HWND hwndDlg)
{
	//Temporary Variables:
	int n;
	TCHAR sz[MAX_PATH];

	//Extract the sampling rate:
	GetWindowText(GetDlgItem(hwndDlg, IDC_SAMPLERATE), sz, MAX_PATH);
	Save_SampleRate = _tcstol(sz, NULL, 10);

	//Extract the Ramping Mode:
	n = SendMessage(GetDlgItem(hwndDlg, IDC_RAMPING), CB_GETCURSEL, 0, 0);
	if (n!=CB_ERR) M_Ramp = n;
	else M_Ramp = 0;

	//Extract the Surround Sound Mode:
	n = SendMessage(GetDlgItem(hwndDlg, IDC_SURROUND), CB_GETCURSEL, 0, 0);
	if (n!=CB_ERR) M_Surround = n;
	else M_Surround = 0;

	//Extract the Mod Mode:
	n = SendMessage(GetDlgItem(hwndDlg, IDC_MODMODE), CB_GETCURSEL, 0, 0);
	if (n!=CB_ERR) M_ModMode = n;
	else M_ModMode = 0;

	//Extract Checkboxes:
	//Bits per Sample
	if (SendMessage(GetDlgItem(hwndDlg, IDC_16BIT), BM_GETCHECK, 0, 0))
		Save_BitsPerSample = 16;
	else
		Save_BitsPerSample = 8;

	//Num of Channels
	if (SendMessage(GetDlgItem(hwndDlg, IDC_STEREO), BM_GETCHECK, 0, 0))
		Save_Channels = 2;
	else
		Save_Channels = 1;

	//Interpolated Mixing
	if (SendMessage(GetDlgItem(hwndDlg, IDC_INTERPOLATED), BM_GETCHECK, 0, 0))
		M_NonInter = FALSE;
	else
		M_NonInter = TRUE;

	//Looping
	if (SendMessage(GetDlgItem(hwndDlg, IDC_LOOP), BM_GETCHECK, 0, 0))
		M_Loop = TRUE;
	else
		M_Loop = FALSE;

	//No Jumpback
	if (SendMessage(GetDlgItem(hwndDlg, IDC_JUMPBACK), BM_GETCHECK, 0, 0))
		M_StopBack = TRUE;
	else
		M_StopBack = FALSE;

	//Position Reset
	if (SendMessage(GetDlgItem(hwndDlg, IDC_POSRESET), BM_GETCHECK, 0, 0))
		M_PosReset = TRUE;
	else
		M_PosReset = FALSE;
	
	//Close the Dialogue Box
	EndDialog(hwndDlg, IDOK);

	return TRUE;
}


//Callback Function
BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_WCE
	static CWinceHelper* pHelper;
#endif
	//Callback message function, based on the following:
	switch (uMsg) 
	{
		case WM_INITDIALOG:
			//Perform initialisation:
#ifdef _WIN32_WCE
		pHelper = (CWinceHelper*)lParam;
			pHelper->SHInitDialog(hwndDlg);
#endif
			return ConfigDialogOnInitDialog(hwndDlg);
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDOK:
					//OK Button runs the actual OK procedure
					return ConfigDialogOnOK(hwndDlg);
				case IDC_CANCEL:
					//Cancel button does essentially nothing
					EndDialog(hwndDlg, LOWORD(wParam));
					return TRUE;
			}
#ifdef _WIN32_WCE
		case WM_PAINT:
			//Repaint the dialogue (using helper)
			return pHelper->DefDlgPaintProc(hwndDlg, wParam, lParam);
		case WM_CTLCOLORSTATIC:
			//Colour Status Process (using Helper)
			return pHelper->DefDlgCtlColorStaticProc(hwndDlg, wParam, lParam);
#endif

	}
	return FALSE;
}

void ShowConfigDialog(HWND hwndParent)
{
#ifdef _WIN32_WCE
	CWinceHelper helper;

	//Shows the Dialogue
	DialogBoxParam(M_hInst, MAKEINTRESOURCE(IDD_CONFIG), hwndParent, ConfigDialogProc, (LPARAM)&helper);
#else
	DialogBox(M_hInst, MAKEINTRESOURCE(IDD_CONFIG), hwndParent, ConfigDialogProc);
#endif
}

