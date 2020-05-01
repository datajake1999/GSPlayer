#include "ModPlugin.h"

extern BOOL M_CalculateLength;
extern int M_BitsPerSample;
extern int M_SampleRate;
extern int M_Channels;
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

#define REG_KEY_BASSMOD			_T("Software\\GreenSoftware\\GSPlayer\\Plug-ins\\BassMod")
#define REG_NAME_SAMPLERATE		_T("SampleRate")
#define REG_NAME_BITS			_T("Bits")
#define REG_NAME_STEREO			_T("Stereo")
#define REG_NAME_RAMPING		_T("Ramping")
#define REG_NAME_NONINTERP		_T("NonInterpolated")
#define REG_NAME_SURROUND		_T("SurroundMode")
#define REG_NAME_MODMODE		_T("ModMode")
#define REG_NAME_LOOP			_T("Loop")
#define REG_NAME_JUMPBACK		_T("StopBack")
#define REG_NAME_POSRESET		_T("PosReset")
#define REG_NAME_CALCLEN		_T("CalcLen")

void ReadRegistory()
{
	HKEY hKey;
	DWORD dwType, dwSize;

	//Open the Registry Key:
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_BASSMOD, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return;

	dwSize = sizeof(int);
	//Load Settings:
	RegQueryValueEx(hKey, REG_NAME_SAMPLERATE, 0, &dwType, (LPBYTE)&M_SampleRate, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_RAMPING, 0, &dwType, (LPBYTE)&M_Ramp, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_BITS, 0, &dwType, (LPBYTE)&M_BitsPerSample, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_STEREO, 0, &dwType, (LPBYTE)&M_Channels, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_NONINTERP, 0, &dwType, (LPBYTE)&M_NonInter, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_SURROUND, 0, &dwType, (LPBYTE)&M_Surround, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_MODMODE, 0, &dwType, (LPBYTE)&M_ModMode, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_LOOP, 0, &dwType, (LPBYTE)&M_Loop, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_JUMPBACK, 0, &dwType, (LPBYTE)&M_StopBack, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_POSRESET, 0, &dwType, (LPBYTE)&M_PosReset, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_CALCLEN, 0, &dwType, (LPBYTE)&M_CalculateLength, &dwSize);

	//Copy over the values we are going to save next time:
	Save_BitsPerSample = M_BitsPerSample;
	Save_SampleRate = M_SampleRate;
	Save_Channels = M_Channels;

	RegCloseKey(hKey);
}

void WriteRegistory()
{
	HKEY hKey;
	DWORD dwDispos;

	//Create the settings key:
	if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_BASSMOD, 
			0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDispos) != ERROR_SUCCESS)
		return;

	//Store Settings:
	RegSetValueEx(hKey, REG_NAME_SAMPLERATE, 0, REG_DWORD, (LPBYTE)&Save_SampleRate, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_BITS, 0, REG_DWORD, (LPBYTE)&Save_BitsPerSample, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_STEREO, 0, REG_DWORD, (LPBYTE)&Save_Channels, sizeof(DWORD));

	RegSetValueEx(hKey, REG_NAME_NONINTERP, 0, REG_DWORD, (LPBYTE)&M_NonInter, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_RAMPING, 0, REG_DWORD, (LPBYTE)&M_Ramp, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_SURROUND, 0, REG_DWORD, (LPBYTE)&M_Surround, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_MODMODE, 0, REG_DWORD, (LPBYTE)&M_ModMode, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_LOOP, 0, REG_DWORD, (LPBYTE)&M_Loop, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_JUMPBACK, 0, REG_DWORD, (LPBYTE)&M_StopBack, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_POSRESET, 0, REG_DWORD, (LPBYTE)&M_PosReset, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_CALCLEN, 0, REG_DWORD, (LPBYTE)&M_CalculateLength, sizeof(DWORD));
}
