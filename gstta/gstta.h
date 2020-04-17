/********************************************************************
 * gstta.h
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

#include "id3tag.h"
#include "id3genre.h"

typedef struct _TTA_FILE_INFO {
	HANDLE					hFile;
	MAP_PLUGIN_FILE_INFO	FileInfo;
	MAP_PLUGIN_FILETAG		FileTag;
	LPBYTE					pbReadBuf;
} TTA_FILE_INFO, *PTTA_FILE_INFO;

typedef struct {
	HANDLE			HFILE;		// file handle
	unsigned short	NCH;		// number of channels
	unsigned short	BPS;		// bits per sample
	unsigned short	BSIZE;		// byte size
	unsigned short	FORMAT;		// audio format
	unsigned long	SAMPLERATE;	// samplerate (sps)
	unsigned long	DATALENGTH;	// data length in samples
	unsigned long	FRAMELEN;	// frame length
	unsigned long	LENGTH;		// playback time (sec)
	unsigned long	FILESIZE;	// file size (byte)
	float			COMPRESS;	// compression ratio
	unsigned long	BITRATE;	// bitrate (kbps)
	unsigned long	STATE;		// return code
	unsigned char	filename[256];
	id3v1_data		id3v1;
	id3v2_data		id3v2;
} tta_info;

typedef struct {
    unsigned long TTAid;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned short BitsPerSample;
    unsigned long SampleRate;
    unsigned long DataLength;
    unsigned long CRC32;
} tta_hdr;
