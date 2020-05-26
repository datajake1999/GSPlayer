// gspflac.cpp
// November 28, 2005
//
// by Jeff Huang <keltusflac@gmail.com>
// AIM: keltuskeltus
// ICQ: 46531632
// MSN: keltuskeltus@hotmail.com


#include "filedecoder.h"

MAP_DEC_PLUGIN plugin;

FileDecoder* g_fileDecoder;

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

BOOL InitFile(LPCTSTR pszFile)
{
	if (g_fileDecoder != NULL)
		return TRUE;

	if (lstrlen(pszFile) < 5 || lstrcmpi(pszFile + lstrlen(pszFile) - 5, _T(".flac")) != 0)
		return FALSE;
	
	char szFile[MAX_PATH];
#ifdef _UNICODE
	WideCharToMultiByte(CP_ACP, 0, pszFile, -1, szFile, MAX_PATH, 0, 0);
#else
	strcpy(szFile, pszFile);
#endif
	
	g_fileDecoder = new FileDecoder();

	if (!g_fileDecoder->set_filename(szFile))
		return FALSE;

	g_fileDecoder->set_metadata_ignore_all();
	g_fileDecoder->set_metadata_respond(FLAC__METADATA_TYPE_STREAMINFO);
	g_fileDecoder->set_metadata_respond(FLAC__METADATA_TYPE_VORBIS_COMMENT);

	if (g_fileDecoder->init() != ::FLAC__FILE_DECODER_OK)
		return FALSE;

	g_fileDecoder->FileSize = FileSize(pszFile);

	if (!g_fileDecoder->process_until_end_of_metadata())
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////
// export functions

void WINAPI flacInit()
{
	g_fileDecoder = NULL;
}

void WINAPI flacQuit()
{
}

DWORD WINAPI flacGetFunc()
{
	return PLUGIN_FUNC_DECFILE | PLUGIN_FUNC_SEEKFILE;
}

BOOL WINAPI flacGetPluginName(LPTSTR pszName)
{
	_tcscpy(pszName, _T("GSPlayer FLAC plugin by Jeff Huang"));
	return TRUE;
}

int WINAPI flacGetFileExtCount()
{
	return 1;
}

BOOL WINAPI flacGetFileExt(int nIndex, LPTSTR pszExt, LPTSTR pszExtDesc)
{
	if (nIndex != 0)
		return FALSE;

	_tcscpy(pszExt, _T("flac"));
	_tcscpy(pszExtDesc, _T("FLAC Files (*.flac)"));
	return TRUE;
}

BOOL WINAPI flacSetEqualizer(MAP_PLUGIN_EQ* pEQ)
{
	return FALSE;
}

BOOL WINAPI flacIsValidFile(LPCTSTR pszFile)
{
	return TRUE;
}

void WINAPI flacShowConfigDlg(HWND hwndParent)
{
	MessageBox(hwndParent, _T("No configurations yet!"), _T("Sorry"), MB_OK);
}

BOOL WINAPI flacOpen(LPCTSTR pszFile, MAP_PLUGIN_FILE_INFO* pInfo)
{
	if (InitFile(pszFile) == FALSE)
		return FALSE;

	pInfo->nAvgBitrate = g_fileDecoder->MapPluginFileInfo.nAvgBitrate;
	pInfo->nBitsPerSample = g_fileDecoder->MapPluginFileInfo.nBitsPerSample;
	pInfo->nChannels = g_fileDecoder->MapPluginFileInfo.nChannels;
	pInfo->nDuration = g_fileDecoder->MapPluginFileInfo.nDuration;
	pInfo->nSampleRate = g_fileDecoder->MapPluginFileInfo.nSampleRate;

	return TRUE;
}

long WINAPI flacSeek(long lTime)
{
	FLAC__uint64 target_sample = (FLAC__uint64)g_fileDecoder->TotalSamples * lTime / g_fileDecoder->MapPluginFileInfo.nDuration;
	g_fileDecoder->seek_absolute(target_sample);
	g_fileDecoder->InternalBufferSize = 0;
	return (long)(target_sample * 1000 / g_fileDecoder->MapPluginFileInfo.nSampleRate);
}

BOOL WINAPI flacStart()
{
	return TRUE;
}

int  WINAPI flacDecode(WAVEHDR* pHdr)
{
	//pHdr->dwBufferLength is usually 16384

	int buffer_to_fill = pHdr->dwBufferLength;

	if (g_fileDecoder->InternalBufferSize == 0)
		g_fileDecoder->process_single();

	if (g_fileDecoder->InternalBufferSize == 0)
		return PLUGIN_RET_EOF;
	
	if (g_fileDecoder->InternalBufferSize >= buffer_to_fill)
	{
		memcpy(pHdr->lpData, g_fileDecoder->InternalBuffer, buffer_to_fill);
		pHdr->dwBytesRecorded = pHdr->dwBufferLength;
		g_fileDecoder->InternalBufferSize -= buffer_to_fill;
		g_fileDecoder->InternalBuffer += buffer_to_fill / sizeof(FLAC__int32);
	}
	else
	{
		memcpy(pHdr->lpData, g_fileDecoder->InternalBuffer, g_fileDecoder->InternalBufferSize);
		pHdr->dwBytesRecorded = g_fileDecoder->InternalBufferSize;
		g_fileDecoder->InternalBufferSize = 0;
	}

	return PLUGIN_RET_SUCCESS;
}

void WINAPI flacStop()
{
	g_fileDecoder->CleanUp();

	g_fileDecoder->seek_absolute(0);
}

void WINAPI flacClose()
{
	g_fileDecoder->finish();

	if (g_fileDecoder)
		delete g_fileDecoder;
	g_fileDecoder = NULL;
}

BOOL WINAPI flacGetTag(MAP_PLUGIN_FILETAG* pTag)
{
	return FALSE;
}

BOOL WINAPI flacGetFileTag(LPCTSTR pszFile, MAP_PLUGIN_FILETAG* pTag)
{
	/* This seems like it should work but it doesn't. Bug in GSPlayer?
	   To "enable" this,
	       comment in this section and add the PLUGIN_FUNC_FILETAG to flacGetFunc
		   and comment in the vorbis comment section in filedecoder.cpp/metadata_callback()

	if (InitFile(pszFile) == FALSE)
		return FALSE;

	pTag->nTrackNum = g_fileDecoder->FileTag.nTrackNum;
	pTag->nYear = g_fileDecoder->FileTag.nYear;
	wcsncpy(pTag->szAlbum, g_fileDecoder->FileTag.szAlbum, MAX_PLUGIN_TAG_STR);
	wcsncpy(pTag->szArtist, g_fileDecoder->FileTag.szArtist, MAX_PLUGIN_TAG_STR);
	wcsncpy(pTag->szComment, g_fileDecoder->FileTag.szComment, MAX_PLUGIN_TAG_STR);
	wcsncpy(pTag->szGenre, g_fileDecoder->FileTag.szGenre, MAX_PLUGIN_TAG_STR);
	wcsncpy(pTag->szTrack, g_fileDecoder->FileTag.szTrack, MAX_PLUGIN_TAG_STR);

	return TRUE;
	*/

	return FALSE;
}

BOOL WINAPI flacOpenStreaming(LPBYTE pbBuf, DWORD cbBuf, MAP_PLUGIN_STREMING_INFO* pInfo)
{
	return FALSE;
}

int WINAPI flacDecodeStreaming(LPBYTE pbInBuf, DWORD cbInBuf, DWORD* pcbInUsed, WAVEHDR* pHdr)
{
	return PLUGIN_RET_ERROR;
}

void WINAPI flacCloseStreaming()
{
}

MAP_DEC_PLUGIN* WINAPI mapGetDecoder()
{
	plugin.dwVersion = PLUGIN_DEC_VER;
	plugin.dwChar = sizeof(TCHAR);
	plugin.Init = flacInit;
	plugin.Quit = flacQuit;
	plugin.GetFunc = flacGetFunc;
	plugin.GetPluginName = flacGetPluginName;
	plugin.GetFileExtCount = flacGetFileExtCount;
	plugin.GetFileExt = flacGetFileExt;
	plugin.SetEqualizer = flacSetEqualizer;
	plugin.IsValidFile = flacIsValidFile;
	plugin.ShowConfigDlg = flacShowConfigDlg;
	plugin.OpenFile = flacOpen;
	plugin.SeekFile = flacSeek;
	plugin.StartDecodeFile = flacStart;
	plugin.DecodeFile = flacDecode;
	plugin.StopDecodeFile = flacStop;
	plugin.CloseFile = flacClose;
	plugin.OpenStreaming = flacOpenStreaming;
	plugin.DecodeStreaming = flacDecodeStreaming;
	plugin.CloseStreaming = flacCloseStreaming;
	plugin.GetTag = flacGetTag;
	plugin.GetFileTag = flacGetFileTag;

	return &plugin;
}

