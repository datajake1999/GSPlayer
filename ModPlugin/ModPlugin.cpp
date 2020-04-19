// ModPlugin.cpp : Defines the initialization routines for the DLL.
//

#include "ModPlugin.h"
#include "bassmod.h"
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MAP_DEC_PLUGIN plugin;

void ReadRegistory();
void WriteRegistory();
void ShowConfigDialog(HWND hwndParent);

HINSTANCE M_hInst = NULL;

//Settings:
BOOL M_CalculateLength = TRUE;
int M_BitsPerSample = 16;
int M_SampleRate = 44100;
int M_Channels = 2;

//Separate Save Settings (they get messed up if we change them on the fly)
int Save_BitsPerSample = 16;
int Save_SampleRate = 44100;
int Save_Channels = 2;

//Tuning Parameters:
BOOL M_Loop = FALSE; //Looping
int M_Ramp = 2; //Ramping : 0 - off, 1 - normal, 2 - sensitive
BOOL M_NonInter = FALSE; //Non-Interpolated Mixing
int M_ModMode = 0; //Mod Playback Mode.  0 - Normal, 1 - ProTracker 1, 2 - FastTracker 2
BOOL M_PosReset = TRUE; //Stop all notes when moving position (using BASSMOD_MusicSetPosition or BASSMOD_MusicPlayEx).  
int M_Surround = 2; //Surround Sound.  0 - Off, 1 - Mode 1, 2 = Mode 2
BOOL M_StopBack = TRUE; //Stop the music when a backward jump effect is played. This stops musics that never 
						 //reach the end from going into endless loops. Some MOD musics are designed to jump all 
						 //over the place, so this flag would cause those to be stopped prematurely. If this flag is 
						 //used together with the BASS_MUSIC_LOOP flag, then the music would not be stopped but any 
						 //BASS_SYNC_END sync would be called.  

// Initialization status (to prevent Bassmod snafus)
BOOL M_InitStatus = FALSE;

//API Entry point?
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	M_hInst = (HINSTANCE)hModule;
    return TRUE;
}


//Sound Settings
DWORD GetFileSettings()
{
	DWORD Flags = 0;
	//Looping
	if (M_Loop)	Flags += BASS_MUSIC_LOOP;
	//Calculating Length
	if (M_CalculateLength) Flags += BASS_MUSIC_CALCLEN;
	//Surround Mode
	if (M_Surround == 1) Flags += BASS_MUSIC_SURROUND;
	else if (M_Surround == 2) Flags += BASS_MUSIC_SURROUND2;
	//Ramping Mode
	if (M_Ramp == 1) Flags += BASS_MUSIC_RAMP;
	else if (M_Ramp == 2) Flags += BASS_MUSIC_RAMPS;
	//Mod Playback Mode
	if (M_ModMode == 1) Flags += BASS_MUSIC_PT1MOD;
	else if (M_ModMode == 2) Flags += BASS_MUSIC_FT2MOD;
	//Stop Jump Backs
	if (M_StopBack) Flags += BASS_MUSIC_STOPBACK;
	//Position Change Reset:
	if (M_PosReset) Flags += BASS_MUSIC_POSRESET;
	//Non-Interpolated Mixing:
	if (M_NonInter) Flags += BASS_MUSIC_NONINTER;

	return Flags;
}

//Checks the BASS for an error and spits it out
void CheckError(LPCTSTR Operation)
{
	TCHAR ErrorMessage[256];

	switch(BASSMOD_ErrorGetCode())
	{
		case BASS_OK:
			return;
		case BASS_ERROR_MEM:
			_tcscpy(ErrorMessage, _T("memory error"));
			break;
		case BASS_ERROR_FILEOPEN:
			_tcscpy(ErrorMessage, _T("can't open the file"));
			break;
		case BASS_ERROR_DRIVER:
			_tcscpy(ErrorMessage, _T("can't find a free/valid driver"));
			break;
		case BASS_ERROR_HANDLE:
			_tcscpy(ErrorMessage, _T("invalid handle"));
			break;
		case BASS_ERROR_FORMAT:
			_tcscpy(ErrorMessage, _T("unsupported format"));
			break;
		case BASS_ERROR_POSITION:
			_tcscpy(ErrorMessage, _T("invalid playback position"));
			break;
		case BASS_ERROR_INIT:
			_tcscpy(ErrorMessage, _T("BASS_Init has not been successfully called"));
			break;
		case BASS_ERROR_ALREADY:
			_tcscpy(ErrorMessage, _T("already initialized/loaded"));
			break;
		case BASS_ERROR_ILLTYPE:
			_tcscpy(ErrorMessage, _T("an illegal type was specified"));
			break;
		case BASS_ERROR_ILLPARAM:
			_tcscpy(ErrorMessage, _T("an illegal parameter was specified"));
			break;
		case BASS_ERROR_DEVICE:
			_tcscpy(ErrorMessage, _T("illegal device number"));
			break;
		case BASS_ERROR_NOPLAY:
			_tcscpy(ErrorMessage, _T("not playing"));
			break;
		case BASS_ERROR_NOMUSIC:
			_tcscpy(ErrorMessage, _T("no MOD music has been loaded"));
			break;
/*		case BASS_ERROR_NOSYNC:
			_tcscpy(ErrorMessage, _T("synchronizers have been disabled"));
			break; - Disabled in Current Version of BASS*/
		case BASS_ERROR_NOTAVAIL:
			_tcscpy(ErrorMessage, _T("requested data is not available"));
			break;
		case BASS_ERROR_DECODE:
			_tcscpy(ErrorMessage, _T("the channel is a decoding channel"));
			break;
		case BASS_ERROR_FILEFORM:
			_tcscpy(ErrorMessage, _T("unsupported file format"));
			break;
		case BASS_ERROR_UNKNOWN:
			_tcscpy(ErrorMessage, _T("unknown error"));
			break;
	}

	MessageBox(NULL,ErrorMessage,Operation,MB_OK | MB_ICONERROR);
}

BOOL CheckModFormat(LPCTSTR FileName)
{
	TCHAR Extension[20];
	int i, j, periodPos = -1;
	//First isolate the extension by finding the last period
	for(i = 0; i<(int)_tcslen(FileName); i++)
	{
		if (FileName[i] == _T('.')) periodPos = i;
	}
	//Abort if no extension was found
	if (periodPos < 0) return FALSE;

	//Copy the extension out of the FileName
	i = periodPos + 1;
	j = 0;
	while(i<(int)_tcslen(FileName) && j<20 && FileName[i] != 0)
	{
		Extension[j] = FileName[i];
		i++;
		j++;
	}
	//Terminate the string
	Extension[j] = 0;

	//Check each extension:
	if ((_tcsicmp(Extension,_T("mod")) == 0) || 
		(_tcsicmp(Extension,_T("xm")) == 0) || 
		(_tcsicmp(Extension,_T("s3m")) == 0) || 
		(_tcsicmp(Extension,_T("it")) == 0) || 
		(_tcsicmp(Extension,_T("mtm")) == 0) || 
		(_tcsicmp(Extension,_T("stm")) == 0) || 
		(_tcsicmp(Extension,_T("669")) == 0) || 
		(_tcsicmp(Extension,_T("umx")) == 0))
		return TRUE;
	else return FALSE;
}

////////////////////////////////////////////////////////////////////
// export functions

//Initialise
void WINAPI ModInit()
{
	if (!M_InitStatus)
	{
		//Load the saved settings:
		ReadRegistory();

		//Set flags based on settings:
		DWORD Flags = BASS_DEVICE_NOSYNC;
		if (M_BitsPerSample < 16) Flags += BASS_DEVICE_8BITS;
		if (M_Channels == 1) Flags += BASS_DEVICE_MONO;
		
		//Initialise the device
		BASSMOD_Init(-3,M_SampleRate,Flags);
		CheckError(_T("Error Initialising"));

		M_InitStatus = TRUE;
	}
}

//Quit
void WINAPI ModQuit()
{
	if (M_InitStatus)
	{
		//Free all BASSMod resources
		BASSMOD_Free();
		CheckError(_T("Error Freeing BASS"));

		//Save settings:
		WriteRegistory();

		M_InitStatus = FALSE;
	}
}

//Appears to return what functions this plugin is capable of.  Let's just say we can
//Only do filetags
DWORD WINAPI ModGetFunc()
{
	return PLUGIN_FUNC_FILETAG | PLUGIN_FUNC_DECFILE | PLUGIN_FUNC_SEEKFILE;
}

//Returns the name of this plugin
BOOL WINAPI ModGetPluginName(LPTSTR pszName)
{
	//MessageBox(NULL,_T("Query Name"),_T("Error"),MB_OK);
	_tcscpy(pszName, _T("BASS Mod plug-in"));
	return TRUE;
}

//Returns the number of extensions
//If you have more, it appears that it uses this to call the GetFileExt function multiple times
int WINAPI ModGetFileExtCount()
{
	return 6;
}

//Returns valid file extensions
//Need to implement a case statement to get multiple extensions
//(One index for each valid type of mod file)
BOOL WINAPI ModGetFileExt(int nIndex, LPTSTR pszExt, LPTSTR pszExtDesc)
{
	switch(nIndex)
	{
		case 0:
			_tcscpy(pszExt, _T("mod"));
			_tcscpy(pszExtDesc, _T("ProTracker Module (*.MOD)"));			
			break;
		case 1:
			_tcscpy(pszExt, _T("s3m"));
			_tcscpy(pszExtDesc, _T("ScreamTracker III Module (*.S3M)"));			
			break;
		case 2:
			_tcscpy(pszExt, _T("xm"));
			_tcscpy(pszExtDesc, _T("FastTracker II Module (*.XM)"));			
			break;
		case 3:
			_tcscpy(pszExt, _T("it"));
			_tcscpy(pszExtDesc, _T("Impulse Tracker Module (*.IT)"));			
			break;
		case 4:
			_tcscpy(pszExt, _T("mtm"));
			_tcscpy(pszExtDesc, _T("MultiTracker Module (*.MTM)"));			
			break;
		case 5:
			_tcscpy(pszExt, _T("umx"));
			_tcscpy(pszExtDesc, _T("Unreal Music Module (*.UMX)"));			
			break;
		default:
			//Any other index does not match a valid file
			return FALSE;
	}
	//If we didn't hit the default case, this should be a valid file entry
	return TRUE; 
}

//Sets the equalizer (nothing yet)
BOOL WINAPI ModSetEqualizer(MAP_PLUGIN_EQ* pEQ)
{
	return FALSE; 
}

//Checks if the specified file is valid (just verify by extension)
BOOL WINAPI ModIsValidFile(LPCTSTR pszFile)
{
	return CheckModFormat(pszFile);
}

void WINAPI ModShowConfigDlg(HWND hwndParent)
{
	//Show the config box:
	ShowConfigDialog(hwndParent);
}

//Opens the mod file
BOOL WINAPI ModOpen(LPCTSTR pszFile, MAP_PLUGIN_FILE_INFO* pInfo)
{
	DWORD len;
	double CalcLen;
	char szFile[MAX_PATH];

	// Run a sanity check on the initialization:
	if (!M_InitStatus)
		ModInit();

	//Check the filename first:
	if (!CheckModFormat(pszFile)) return FALSE;

	//Copy the passed file name
#ifdef _UNICODE
	WideCharToMultiByte(CP_ACP, 0, pszFile, -1, szFile, MAX_PATH, 0, 0);
#else
	_tcscpy(szFile, pszFile);
#endif

	//Open the file
	if (!BASSMOD_MusicLoad(FALSE,szFile,0,0,GetFileSettings()))
	{
		CheckError(_T("Error Loading"));
		return FALSE;
	}
	//Get the length
	if (M_CalculateLength) len = BASSMOD_MusicGetLength(TRUE);
	else len = 0;

	//Calculate the length based on current sound settings
	CalcLen = (double)len * 1000 / (M_SampleRate * M_Channels * (M_BitsPerSample/8));

	//Set the file information
	pInfo->nBitsPerSample = M_BitsPerSample;
	pInfo->nChannels = M_Channels;
	pInfo->nSampleRate = M_SampleRate;
	pInfo->nAvgBitrate = 64;
	pInfo->nDuration = (long)CalcLen;

	return TRUE;
}

//Seeks to a specific position in the file.
long WINAPI ModSeek(long lTime)
{
	//Convert the MS position to seconds
	DWORD Seconds = (DWORD)(lTime/1000);

	//Mask the high order bits with the appropriate pattern.
	Seconds = Seconds | 0xFFFF0000;
	BASSMOD_MusicSetPosition(Seconds);
	CheckError(_T("Error Seeking"));

	//Return the number passed in
	return lTime;
}

//Start playing the mod file
BOOL WINAPI ModStart()
{
	//Decoding doesn't need an offical start
	return true;
}

//Decode the mod file
int WINAPI ModDecode(WAVEHDR* pHdr)
{
	//Decode the info from BASS:
	BASSMOD_MusicDecode(pHdr->lpData,pHdr->dwBufferLength);
	if (BASSMOD_ErrorGetCode() == BASS_ERROR_NOPLAY) return PLUGIN_RET_EOF;
	else 
	{
		//Always "encode" the buffer length
		pHdr->dwBytesRecorded += pHdr->dwBufferLength;
		
		//Indicate that all is good:
		return PLUGIN_RET_SUCCESS;
	}
}

//Stop playing the mod file
void WINAPI ModStop()
{
	BASSMOD_MusicStop();
	CheckError(_T("Error Stopping"));
}

//Close the mod file
void WINAPI ModClose()
{
	//Free the current mod file
	BASSMOD_MusicFree();
	CheckError(_T("Error Freeing MOD"));
}

//Gets the track name for the current file?  Read this from BASS
BOOL WINAPI ModGetTag(MAP_PLUGIN_FILETAG* pTag)
{
	//MessageBox(NULL,_T("Tag Call"),_T("Error"),MB_OK);
	char * szTrack;
	
	//Copy the track name from the current BASS track
	szTrack = BASSMOD_MusicGetName();
	CheckError(_T("Error Getting Name"));

	//Set all other tag information to zero/null
	pTag->nTrackNum = 0;
	pTag->nYear = 0;
	*pTag->szAlbum = NULL;
	*pTag->szArtist = NULL;
	*pTag->szGenre = NULL;
	*pTag->szComment = NULL;

	//Convert the track name to Unicode or directly copy it.
#ifdef _UNICODE
	if (strlen(szTrack))
		MultiByteToWideChar(CP_ACP, 0, szTrack, -1, pTag->szTrack, MAX_PLUGIN_TAG_STR);
#else
	_tcscpy(pTag->szTrack, szTrack);
#endif

	return TRUE;
}

//Obtains the track name from a specified file.
//Just return false, we won't support this exact function (gets too tricky with
//the current file system of BASS)
BOOL WINAPI ModGetFileTag(LPCTSTR pszFile, MAP_PLUGIN_FILETAG* pTag)
{
	return FALSE;
}

//Open Streaming?  Not sure..
BOOL WINAPI ModOpenStreaming(LPBYTE pbBuf, DWORD cbBuf, MAP_PLUGIN_STREMING_INFO* pInfo)
{
	//MessageBox(NULL,_T("Open Streaming?"),_T("Error"),MB_OK);
	return FALSE;
}

//Streaming Decode?  Same deal.
int WINAPI ModDecodeStreaming(LPBYTE pbInBuf, DWORD cbInBuf, DWORD* pcbInUsed, WAVEHDR* pHdr)
{
	//MessageBox(NULL,_T("Decode Streaming?"),_T("Error"),MB_OK);
	return PLUGIN_RET_ERROR;
}

//Close Streaming?
void WINAPI ModCloseStreaming()
{
	//MessageBox(NULL,_T("Close Streaming?"),_T("Error"),MB_OK);
}

//The only exported function.  This returns a function that decodes
//All local functions and maps them to the plugin paths.
MAP_DEC_PLUGIN* WINAPI mapGetDecoder()
{
	plugin.dwVersion = PLUGIN_DEC_VER;
	plugin.dwChar = sizeof(TCHAR);
	plugin.Init = ModInit;
	plugin.Quit = ModQuit;
	plugin.GetFunc = ModGetFunc;
	plugin.GetPluginName = ModGetPluginName;
	plugin.GetFileExtCount = ModGetFileExtCount;
	plugin.GetFileExt = ModGetFileExt;
	plugin.SetEqualizer = ModSetEqualizer;
	plugin.IsValidFile = ModIsValidFile;
	plugin.ShowConfigDlg = ModShowConfigDlg;
	plugin.OpenFile = ModOpen;
	plugin.SeekFile = ModSeek;
	plugin.StartDecodeFile = ModStart;
	plugin.DecodeFile = ModDecode;
	plugin.StopDecodeFile = ModStop;
	plugin.CloseFile = ModClose;
	plugin.OpenStreaming = ModOpenStreaming;
	plugin.DecodeStreaming = ModDecodeStreaming;
	plugin.CloseStreaming = ModCloseStreaming;
	plugin.GetTag = ModGetTag;
	plugin.GetFileTag = ModGetFileTag;

	return &plugin;
}
