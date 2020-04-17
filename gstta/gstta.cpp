/********************************************************************
 * gstta.cpp
 *
 * Description: TTA plugin fo gsplayer ppc and winCE
 *
 * Copyleft (c) 2008 ioctl.tistory.com
 *
 ********************************************************************/

/********************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Please see the file COPYING in this directory for full copyright
 * information.
 *********************************************************************/

#include <windows.h>
#include <tchar.h>
#ifndef _WIN32_WCE
#include <mmsystem.h>
#endif
#include "mapplugin.h"
#include "TTAlib.h"
#include "TTAReader.h"
#include "gstta.h"

#define THREAD_TYPE 1

MAP_DEC_PLUGIN plugin = {0};
TTA_FILE_INFO g_Info = {0};
CRITICAL_SECTION g_csec;

CRITICAL_SECTION g_csec_buf;

void WINAPI Init()
{
	InitializeCriticalSection(&g_csec);
	InitializeCriticalSection(&g_csec_buf);
}

void WINAPI Quit()
{
	DeleteCriticalSection(&g_csec);
	DeleteCriticalSection(&g_csec_buf);
}

DWORD WINAPI GetFunc()
{
	return PLUGIN_FUNC_DECFILE | PLUGIN_FUNC_SEEKFILE | PLUGIN_FUNC_FILETAG;// | PLUGIN_FUNC_DECSTREAMING;
}

BOOL WINAPI GetPluginName(LPTSTR pszName)
{
	_tcscpy(pszName, _T("TTA codec Plug-in"));
	return FALSE;
}

BOOL WINAPI SetEqualizer(MAP_PLUGIN_EQ* pEQ)
{
	return FALSE;
}

void WINAPI ShowConfigDlg(HWND hwndParent)
{
	MessageBox(hwndParent, _T("TTA Codec Plug-in\nCopyleft(C) 2008 ioctl.tistory.com"), _T("About"), MB_ICONINFORMATION);
}

int WINAPI GetFileExtCount()
{
	return 1;
}

BOOL WINAPI GetFileExt(int nIndex, LPTSTR pszExt, LPTSTR pszExtDesc)
{
	switch (nIndex) {
	case 0:
		_tcscpy(pszExt, _T("tta"));
		_tcscpy(pszExtDesc, _T("TTA Files (*.tta)"));
		return TRUE;
	}
	return FALSE;
}

BOOL IsTtaFile(TCHAR* pszFile)
{
    unsigned char *buf1;
	HANDLE hFile;
	BOOL bRet = FALSE;
	DWORD dwRead = 0;

    buf1 = (unsigned char *)malloc(24);
	if(buf1 == NULL)
		return FALSE;
	
	hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (!ReadFile(hFile, (void *)buf1, 3, &dwRead, NULL) || dwRead < 3)
	{
		free(buf1);
		CloseHandle(hFile);
		return FALSE;
	}

	// Search for audio start code TTA   
	if ((buf1[0] == 'T') &&  
	(buf1[1] == 'T') &&
	(buf1[2] == 'A') )
	{
		bRet = TRUE;
	}
	else 
	{
        bRet = FALSE;
	}
    free(buf1);
	CloseHandle(hFile);
	return bRet;
}

using namespace TTALib;
static tta_info tta_file_info;

BOOL WINAPI IsValidFile(LPCTSTR pszFile)
{
	return IsTtaFile((TCHAR*)pszFile);
}

#ifdef _WIN32
	#pragma pack(1)
	#define __ATTRIBUTE_PACKED__
	typedef unsigned __int64 uint64;
#else
	#define __ATTRIBUTE_PACKED__	__attribute__((packed))
	typedef unsigned long long uint64;
#endif

TTALib::TTAError CopyId3Header_l (HANDLE hInFile, void *tag_info)
{
	struct {
		unsigned char id[3];
		unsigned short version;
		unsigned char flags;
		unsigned char size[4];
	} __ATTRIBUTE_PACKED__ id3v2;
	unsigned long data_len, offset = 0,	result;

	if (!ReadFile (hInFile, &id3v2, sizeof(id3v2), &result, NULL))
		return TTALib::READ_ERROR;

	if (id3v2.id[0] == 'I' && 
		id3v2.id[1] == 'D' && 
		id3v2.id[2] == '3') {
			char buffer[512];

			if (id3v2.size[0] & 0x80) 
				return TTALib::FILE_ERROR;

			offset = (id3v2.size[0] & 0x7f);
			offset = (offset << 7) | (id3v2.size[1] & 0x7f);
			offset = (offset << 7) | (id3v2.size[2] & 0x7f);
			offset = (offset << 7) | (id3v2.size[3] & 0x7f);
			if (id3v2.flags & (1 << 4)) offset += 10;
			data_len = offset, offset += 10;

			while (data_len > 0) {
				unsigned long len = (data_len > sizeof(buffer))? sizeof(buffer):data_len;
				if (!ReadFile (hInFile, buffer, len, &result, NULL)) 
					return TTALib::READ_ERROR;
				data_len -= len;
			}
		} else SetFilePointer (hInFile, 0, NULL, FILE_BEGIN);
	return TTALib::TTA_NO_ERROR;
}

#define	ENDSWAP_INT32(x)	(x)
TTALib::TTADecoder *decoder;
unsigned long g_byte_size;
#ifndef THREAD_TYPE
unsigned char g_prePatched; //
char *prePatchBuf_read;
char *prePatchBuf_write;
char *prePatchBuf;
unsigned int  prePatchByteCount;
#endif
static unsigned int unpack_sint28 (const char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;
	value =  value       | (ptr[0] & 0x7f);
	value = (value << 7) | (ptr[1] & 0x7f);
	value = (value << 7) | (ptr[2] & 0x7f);
	value = (value << 7) | (ptr[3] & 0x7f);

	return value;
}
static unsigned int unpack_sint32 (const char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;
	value = (value << 8) | ptr[0];
	value = (value << 8) | ptr[1];
	value = (value << 8) | ptr[2];
	value = (value << 8) | ptr[3];

	return value;
}
static int get_frame_id (const char *id) {
	if (!memcmp(id, "TIT2", 4)) return TIT2;	// Title
	if (!memcmp(id, "TPE1", 4)) return TPE1;	// Artist
	if (!memcmp(id, "TALB", 4)) return TALB;	// Album
	if (!memcmp(id, "TRCK", 4)) return TRCK;	// Track
	if (!memcmp(id, "TYER", 4)) return TYER;	// Year
	if (!memcmp(id, "TCON", 4)) return TCON;	// Genre
	if (!memcmp(id, "COMM", 4)) return COMM;	// Comment
	return 0;
}
static void get_id3v1_tag (tta_info *ttainfo) {
	id3v1_tag id3v1;
	unsigned long result;

	SetFilePointer(ttainfo->HFILE, -(int) sizeof(id3v1_tag), NULL, FILE_END);
	if (ReadFile(ttainfo->HFILE, &id3v1, sizeof(id3v1_tag), &result, NULL) &&
		result == sizeof(id3v1_tag) && !memcmp(id3v1.id, "TAG", 3)) {
		memcpy(ttainfo->id3v1.title, id3v1.title, 30);
		memcpy(ttainfo->id3v1.artist, id3v1.artist, 30);
		memcpy(ttainfo->id3v1.album, id3v1.album, 30);
		memcpy(ttainfo->id3v1.year, id3v1.year, 4);
		memcpy(ttainfo->id3v1.comment, id3v1.comment, 28);
		ttainfo->id3v1.track = id3v1.track;
		ttainfo->id3v1.genre = id3v1.genre;
		ttainfo->id3v1.id3has = 1;
	}
	SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
}

static void get_id3v2_tag (tta_info *ttainfo) {
	HANDLE hMap;
	id3v2_tag id3v2;
	id3v2_frame frame_header;
	unsigned char *buffer, *ptr;
	unsigned long result; 
		int id3v2_size;

	if (!ReadFile(ttainfo->HFILE, &id3v2, sizeof(id3v2_tag), &result, NULL) ||
		result != sizeof(id3v2_tag) || memcmp(id3v2.id, "ID3", 3)) {
		SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
		return;
	}

	id3v2_size = unpack_sint28((char *)id3v2.size) + 10;

	if ((id3v2.flags & ID3_UNSYNCHRONISATION_FLAG) ||
		(id3v2.flags & ID3_EXPERIMENTALTAG_FLAG) ||
		(id3v2.version < 3)) goto done;

	hMap = CreateFileMapping(ttainfo->HFILE, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
	if (!hMap) goto done;

	buffer = (unsigned char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
	if (!buffer) {
		CloseHandle(hMap);
		goto done;
	}

	ptr = buffer + 10;
	// skip extended header if present
	if (id3v2.flags & ID3_EXTENDEDHEADER_FLAG) {
		int offset = (int) unpack_sint32((char *)ptr);
		ptr += offset;
	}

	// read id3v2 frames
	while (ptr - buffer < id3v2_size) {
		int data_size, frame_id;
		int size, comments = 0;
		unsigned char *data;

		// get frame header
		memcpy(&frame_header, ptr, sizeof(id3v2_frame));
		ptr += sizeof(id3v2_frame);
		data_size = unpack_sint32((char*)frame_header.size);

		if (!*frame_header.id) break;

		if (!(frame_id = get_frame_id((char *)frame_header.id))) {
			ptr += data_size;
			continue;
		}

		// skip unsupported frames
		if (frame_header.flags & FRAME_COMPRESSION_FLAG ||
			frame_header.flags & FRAME_ENCRYPTION_FLAG ||
			frame_header.flags & FRAME_UNSYNCHRONISATION_FLAG ||
			*ptr != FIELD_TEXT_ISO_8859_1) {
			ptr += data_size;
			continue;
		}

		ptr++; data_size--;
		switch (frame_id) {
		case TIT2:	data = ttainfo->id3v2.title;
					size = sizeof(ttainfo->id3v2.title) - 1; break;
		case TPE1:	data = ttainfo->id3v2.artist;
					size = sizeof(ttainfo->id3v2.artist) - 1; break;
		case TALB:	data = ttainfo->id3v2.album;
					size = sizeof(ttainfo->id3v2.album) - 1; break;
		case TRCK:	data = ttainfo->id3v2.track;
					size = sizeof(ttainfo->id3v2.track) - 1; break;
		case TYER:	data = ttainfo->id3v2.year;
					size = sizeof(ttainfo->id3v2.year) - 1; break;
		case TCON:	data = ttainfo->id3v2.genre;
					size = sizeof(ttainfo->id3v2.genre) - 1; break;
		case COMM:	if (comments++) goto next;
					data = ttainfo->id3v2.comment;
					size = sizeof(ttainfo->id3v2.comment) - 1;
					data_size -= 3; ptr += 3;
					// skip zero short description
					if (*ptr == 0) { data_size--; ptr++; }
					break;
		}
next:
		memcpy(data, ptr, (data_size <= size)? data_size:size);
		ptr += data_size;
	}

	UnmapViewOfFile((LPCVOID *) buffer);
	CloseHandle(hMap);

done:
	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
	SetFilePointer(ttainfo->HFILE, id3v2_size, NULL, FILE_BEGIN);
	ttainfo->id3v2.size = id3v2_size;

	ttainfo->id3v2.id3has = 1;
}

#define MAX_Q_SIZE 4
#define Q_BLOCK_SIZE 184320
#define TOTAL_Q_SIZE (Q_BLOCK_SIZE*MAX_Q_SIZE)
#define PRE_Q_NUM 2
int write_q_pos;
int read_q_pos;
int queued_byte;
int write_block_num; // 0 ~ max-1
int read_block_num;  // 0 ~ max-1
int free_q_block_num; // 1 ~ max
unsigned char * q_start_point;
static volatile int killDecoderThread = 0;
static DWORD WINAPI DecoderThread (void *p);
static HANDLE decoder_handle = NULL;
static int end_of_file=0;
static unsigned int start_file_pos;

int check_queue(void)
{
	if(queued_byte < PRE_Q_NUM*Q_BLOCK_SIZE) 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void put_queue(unsigned char * src, int size)
{
    if(size > TOTAL_Q_SIZE-write_q_pos)
	{
	    int splite = TOTAL_Q_SIZE-write_q_pos;
        memcpy(q_start_point+write_q_pos, src, splite);
		memcpy(q_start_point, src+splite, size-splite);
		write_q_pos = size-splite;
		queued_byte += size;
	}
	else
	{
        memcpy(q_start_point+write_q_pos, src, size);
		write_q_pos += size;
		queued_byte += size;
	}
}

int get_queue(unsigned char * dest, int size) 
{
    if(size > TOTAL_Q_SIZE - read_q_pos)
	{
	    int splite = TOTAL_Q_SIZE-read_q_pos;
		
        memcpy(dest, q_start_point+read_q_pos, splite);
		memcpy(dest+splite, q_start_point, size-splite);
		read_q_pos = size-splite;
		queued_byte -= size;
	}
	else
	{
        memcpy(dest, q_start_point+read_q_pos, size);
		read_q_pos += size;
		queued_byte -= size;
	}

	return size;
}


bool Transfer(long *data, long byte_size, long num_chan, unsigned long *len, WAVEHDR* pHdr)
{
    unsigned char *buffer, *dst;
	long *src = data;
	
	if (!(dst = buffer = (unsigned char *)calloc(*len * num_chan, byte_size)))
	{

		return 0;
	}

	switch (byte_size) {
	case 1: for (; src < data + (*len * num_chan); src++)
				*dst++ = (unsigned char) (*src + 0x80);
			break;
	case 2: for (; src < data + (*len * num_chan); src++) {
				*dst++ = (unsigned char) *src;
				*dst++ = (unsigned char) (*src >> 8);
			}
			break;
	case 3: for (; src < data + (*len * num_chan); src++) {
				*dst++ = (unsigned char) *src;
				*dst++ = (unsigned char) (*src >> 8);
				*dst++ = (unsigned char) (*src >> 16);
			}
			break;
	case 4: for (; src < data + (*len * num_chan * 2); src += 2) {
				*dst++ = (unsigned char) *src;
				*dst++ = (unsigned char) (*src >> 8);
				*dst++ = (unsigned char) (*src >> 16);
				*dst++ = (unsigned char) (*src >> 24);
			}
			break;
    }
	
#ifdef THREAD_TYPE
    put_queue(buffer, *len * num_chan * byte_size);
#else
	memcpy(prePatchBuf, buffer, *len * num_chan * byte_size);
	prePatchByteCount = *len * num_chan * byte_size;
#endif

    free(buffer);
    return true;
}



#define FRAME_TIME		1.04489795918367346939
#define SEEK_STEP		(int)(FRAME_TIME * 1000)
int set_position (unsigned long pos) {

	queued_byte = 0;
	memset(q_start_point, 0, TOTAL_Q_SIZE);
    write_q_pos = 0;
	read_q_pos = 0;
	write_block_num = 0; 
	read_block_num = 0;  
	free_q_block_num = MAX_Q_SIZE; 

	return 0;
}

static int seek_needed=-1;
unsigned long data_pos;			// currently playing frame index

static DWORD WINAPI DecoderThread (void *p) {
	int done = 0;
	unsigned long buflen;
	long *buf;
	while (!killDecoderThread) {
		if (seek_needed != -1) {
			data_pos = seek_needed / SEEK_STEP;
			set_position (data_pos);
			seek_needed = -1;
		}
		if(check_queue()==TRUE && end_of_file!=1)
		{  
		   if((buflen = decoder->GetBlock (&buf)) > 0)
		   {
		       if (!Transfer(buf, g_byte_size, decoder->GetNumChannels(), &buflen, NULL))//pHdr))
		       	{
		       	   
		       	}
			   else 
			   	{
			   	   
			   	}
		   }
		   else
		   	{
			    // set eof
			    end_of_file = 1;
		   	    //return 0; 
		   	}
	   	}
		else
		{
			Sleep(100);
		}
		
	}
	return 0;
}


BOOL WINAPI OpenFile(LPCTSTR pszFile, MAP_PLUGIN_FILE_INFO* pInfo)
{
    TTALib::TTAError err;
	unsigned long byte_size;
	#ifndef THREAD_TYPE
	int iii;
    #endif
	tta_hdr ttahdr;
	DWORD result;
	unsigned long datasize;
	unsigned long origsize;

    write_q_pos = 0;
	read_q_pos = 0;
    read_block_num = 0;
	free_q_block_num = MAX_Q_SIZE;
	q_start_point = NULL;
	killDecoderThread = FALSE;
	end_of_file = 0;
	queued_byte = 0;
	#ifndef THREAD_TYPE
	g_prePatched = FALSE;
	prePatchByteCount = 0;
	prePatchBuf = NULL;
	prePatchBuf_read = NULL;
	prePatchBuf_write = NULL;
	#endif
	if (!IsTtaFile((TCHAR*)pszFile))
		return FALSE;
    g_Info.hFile = NULL;
	if ((g_Info.hFile = CreateFile ((TCHAR*)pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
	{

		return FALSE; 
	}
	tta_file_info.HFILE = g_Info.hFile;
	tta_file_info.FILESIZE = GetFileSize(g_Info.hFile, NULL);

	// ID3V1 support
	get_id3v1_tag(&tta_file_info);

	// ID3V2 minimal support
	get_id3v2_tag(&tta_file_info);
    
	// read TTA header
	if (!ReadFile(g_Info.hFile, &ttahdr, sizeof(tta_hdr), &result, NULL) ||
		result != sizeof(tta_hdr)) {
		CloseHandle(g_Info.hFile);
		tta_file_info.STATE = READ_ERROR;
		return FALSE;
	}

	SetFilePointer(g_Info.hFile, 0, NULL, FILE_BEGIN);
	err = TTALib::TTA_NO_ERROR;
	decoder = new TTADecoder (g_Info.hFile); 
	byte_size = (decoder->GetBitsPerSample() + 7) / 8;
    g_byte_size = byte_size;

    #define FRAME_TIME		1.04489795918367346939
	// fill the File Info
	tta_file_info.NCH = ttahdr.NumChannels;
	tta_file_info.BPS = ttahdr.BitsPerSample;
	tta_file_info.BSIZE = (ttahdr.BitsPerSample + 7)/8;
	tta_file_info.FORMAT = ttahdr.AudioFormat;
	tta_file_info.SAMPLERATE = ttahdr.SampleRate;
	tta_file_info.DATALENGTH = ttahdr.DataLength;
	tta_file_info.FRAMELEN = (long) (FRAME_TIME * ttahdr.SampleRate);
	tta_file_info.LENGTH = ttahdr.DataLength / ttahdr.SampleRate * 1000;
	datasize = tta_file_info.FILESIZE - tta_file_info.id3v2.size;
	origsize = tta_file_info.DATALENGTH * tta_file_info.BSIZE * tta_file_info.NCH;
	tta_file_info.COMPRESS = (float) datasize / origsize;
	tta_file_info.BITRATE = (long) ((tta_file_info.COMPRESS *
	tta_file_info.SAMPLERATE * tta_file_info.NCH * tta_file_info.BPS) / 1000);
    pInfo->nAvgBitrate = tta_file_info.BITRATE;
	pInfo->nBitsPerSample = (unsigned short)decoder->GetBitsPerSample();
	pInfo->nChannels = (unsigned short)decoder->GetNumChannels();
	pInfo->nDuration = decoder->GetDataLength()/decoder->GetSampleRate() *1000 ; // <<--- ???
	pInfo->nSampleRate = decoder->GetSampleRate();
	#if 0
	RETAILMSG(TRUE, (TEXT("AvgBitrate = %d \r\n"),pInfo->nAvgBitrate));
	RETAILMSG(TRUE, (TEXT("BitsPerSample = %d \r\n"),pInfo->nBitsPerSample));
	RETAILMSG(TRUE, (TEXT("Channels = %d \r\n"), pInfo->nChannels));
	RETAILMSG(TRUE, (TEXT("Duration = %d \r\n"), pInfo->nDuration));
	RETAILMSG(TRUE, (TEXT("SampleRate = %d \r\n"), pInfo->nSampleRate));
    #endif

    // save start file pointer.
    start_file_pos = SetFilePointer(g_Info.hFile, 0, NULL, FILE_CURRENT);

#ifdef THREAD_TYPE
    q_start_point = (unsigned char*)malloc(TOTAL_Q_SIZE);//
    memset(q_start_point, 0, TOTAL_Q_SIZE);
    write_q_pos = 0;
	read_q_pos = 0;
	write_block_num = 0; // 0 ~ max-1
	read_block_num = 0;  // 0 ~ max-1
	free_q_block_num = MAX_Q_SIZE; // 1 ~ max
#else
	iii = 184320/16384 + 1; 
    prePatchBuf = (char *) malloc(iii*16384); 
	prePatchBuf_read = prePatchBuf;
	prePatchBuf_write = prePatchBuf;	
#endif	

	return TRUE;
}



long WINAPI SeekFile(long lTime)
{
	EnterCriticalSection(&g_csec);
	seek_needed = lTime;
	LeaveCriticalSection(&g_csec);
	Sleep(700);
	return lTime;
}

BOOL WINAPI StartDecodeFile()
{
    unsigned int dwNewPos;
#ifdef THREAD_TYPE
	unsigned long decoder_thread_id;
#endif
	
	if (!g_Info.hFile)
		return FALSE;
	#ifdef THREAD_TYPE
	write_q_pos = 0;
	read_q_pos = 0;
    read_block_num = 0;
	free_q_block_num = MAX_Q_SIZE;
	killDecoderThread = FALSE;
	end_of_file = 0;
	queued_byte = 0;
	write_q_pos = 0;
	read_q_pos = 0;
	write_block_num = 0; // 0 ~ max-1
	read_block_num = 0;  // 0 ~ max-1
	free_q_block_num = MAX_Q_SIZE; // 1 ~ max
	dwNewPos = SetFilePointer(g_Info.hFile, start_file_pos, NULL, FILE_BEGIN);
    decoder_handle = CreateThread(NULL, 0, DecoderThread, NULL, 0, &decoder_thread_id);
	if (!decoder_handle) 
		return FALSE;
	Sleep(1000); // get. prefill Queue time.
	#endif
	
	return TRUE; 
}

int DecodeTtaFile(TTA_FILE_INFO* pInfo, WAVEHDR* pHdr)
{
    #ifndef THREAD_TYPE
    long *buf;
	unsigned long buflen;
	#endif

	if (!pInfo->hFile)
	{
		return PLUGIN_RET_ERROR;
	}

#ifdef THREAD_TYPE
	if(end_of_file && queued_byte < 16384)
	{
		return PLUGIN_RET_EOF;
	}
    else
	{
        get_queue((LPBYTE)pHdr->lpData, 16384);
		pHdr->dwBytesRecorded += 16384;
		return PLUGIN_RET_SUCCESS;
	}
#else
    if(prePatchByteCount>16384)
   	{
   	    prePatchByteCount -= 16384;
        memcpy((LPBYTE)pHdr->lpData, prePatchBuf_read, 16384);
		prePatchBuf_read += 16384;
		pHdr->dwBytesRecorded += 16384;
		return PLUGIN_RET_SUCCESS;
   	}
    else if(prePatchByteCount>0)
   	{
        memcpy((LPBYTE)pHdr->lpData, prePatchBuf_read, prePatchByteCount);
		pHdr->dwBytesRecorded += prePatchByteCount;
		prePatchByteCount = 0;
		return PLUGIN_RET_SUCCESS;
   	}
	if(end_of_file)
	{
		return PLUGIN_RET_EOF;
	}
    else
   	{  
   	   prePatchByteCount = 0;
	   prePatchBuf_read = prePatchBuf;
	   
	   if((buflen = decoder->GetBlock (&buf)) > 0)
	   {
	       if (!Transfer(buf, g_byte_size, decoder->GetNumChannels(), &buflen, pHdr))
	   	       return PLUGIN_RET_ERROR; 
		   else 
		   	   return PLUGIN_RET_SUCCESS; 
	   }
	   else
	   {
	   	   return PLUGIN_RET_EOF;
	   }
   	}
#endif
	
return PLUGIN_RET_ERROR;

}


int WINAPI DecodeFile(WAVEHDR* pHdr)
{
	int nRet;
	EnterCriticalSection(&g_csec);
	nRet = DecodeTtaFile(&g_Info, pHdr);
	LeaveCriticalSection(&g_csec);
	return nRet;
}

void WINAPI StopDecodeFile()
{
    #ifdef THREAD_TYPE
	killDecoderThread = 1;
	Sleep(500);	// wait. <- 
	#else
	prePatchByteCount = 0;
	prePatchBuf_read = prePatchBuf;
	prePatchBuf_write = prePatchBuf;
    #endif
}

void WINAPI CloseFile()
{
	CloseHandle(g_Info.hFile);
	#ifndef THREAD_TYPE
	free(prePatchBuf);
	#endif
	delete decoder;
}

BOOL WINAPI GetTag(MAP_PLUGIN_FILETAG* pTag)
{
	if (!g_Info.hFile)
		return FALSE;
	*pTag = g_Info.FileTag;
	return TRUE;
}

BOOL WINAPI GetFileTag(LPCTSTR pszFile, MAP_PLUGIN_FILETAG* pTag)
{
	memset(pTag, 0, sizeof(MAP_PLUGIN_FILETAG));
	return FALSE; 
}

BOOL WINAPI OpenStreaming(LPBYTE pbBuf, DWORD cbBuf, MAP_PLUGIN_STREMING_INFO* pInfo)
{
	return FALSE;
}

int WINAPI DecodeStreaming(LPBYTE pbInBuf, DWORD cbInBuf, DWORD* pcbInUsed, WAVEHDR* pHdr)
{
	return -1;
}

void WINAPI CloseStreaming()
{

}

MAP_DEC_PLUGIN* WINAPI mapGetDecoder()
{
	plugin.dwVersion = PLUGIN_DEC_VER;
	plugin.dwChar = sizeof(TCHAR);
	plugin.Init = Init;
	plugin.Quit = Quit;
	plugin.GetFunc = GetFunc;
	plugin.GetPluginName = GetPluginName;
	plugin.GetFileExtCount = GetFileExtCount;
	plugin.GetFileExt = GetFileExt;
	plugin.SetEqualizer = SetEqualizer;
	plugin.IsValidFile = IsValidFile;
	plugin.ShowConfigDlg = ShowConfigDlg;
	plugin.OpenFile = OpenFile;
	plugin.SeekFile = SeekFile;
	plugin.StartDecodeFile = StartDecodeFile;
	plugin.DecodeFile = DecodeFile;
	plugin.StopDecodeFile = StopDecodeFile;
	plugin.CloseFile = CloseFile;
	plugin.OpenStreaming = OpenStreaming;
	plugin.DecodeStreaming = DecodeStreaming;
	plugin.CloseStreaming = CloseStreaming;
	plugin.GetTag = GetTag;
	plugin.GetFileTag = GetFileTag;

	return &plugin;
}


