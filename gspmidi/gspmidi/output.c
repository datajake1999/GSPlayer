/*

TiMidity -- Experimental MIDI to WAVE converter
Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

output.c

Audio output (to file / device) functions.
*/

#include "config.h"
#include "output.h"
#include "tables.h"

extern PlayMode gsp_play_mode;

PlayMode *play_mode_list[] = {
    &gsp_play_mode,
    0
};

PlayMode *play_mode = &gsp_play_mode;

/*****************************************************************/
/* Some functions to convert signed 32-bit data to other formats */

void s32tos8(int32 *lp, int32 c)
{
    int8 *cp=(int8 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-8-GUARD_BITS);
        if (l>127) l=127;
        else if (l<-128) l=-128;
        *cp++ = (int8) (l);
    }
}

void s32tou8(int32 *lp, int32 c)
{
    uint8 *cp=(uint8 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-8-GUARD_BITS);
        if (l>127) l=127;
        else if (l<-128) l=-128;
        *cp++ = 0x80 ^ ((uint8) l);
    }
}

void s32tos16(int32 *lp, int32 c)
{
    int16 *sp=(int16 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-16-GUARD_BITS);
        if (l > 32767) l=32767;
        else if (l<-32768) l=-32768;
        *sp++ = (int16)(l);
    }
}

void s32tou16(int32 *lp, int32 c)
{
    uint16 *sp=(uint16 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-16-GUARD_BITS);
        if (l > 32767) l=32767;
        else if (l<-32768) l=-32768;
        *sp++ = 0x8000 ^ (uint16)(l);
    }
}

void s32tos16x(int32 *lp, int32 c)
{
    int16 *sp=(int16 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-16-GUARD_BITS);
        if (l > 32767) l=32767;
        else if (l<-32768) l=-32768;
        *sp++ = XCHG_SHORT((int16)(l));
    }
}

void s32tou16x(int32 *lp, int32 c)
{
    uint16 *sp=(uint16 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-16-GUARD_BITS);
        if (l > 32767) l=32767;
        else if (l<-32768) l=-32768;
        *sp++ = XCHG_SHORT(0x8000 ^ (uint16)(l));
    }
}

void s32toulaw(int32 *lp, int32 c)
{
    uint8 *up=(uint8 *)(lp);
    int32 l;
    while (c--)
    {
        l=(*lp++)>>(32-13-GUARD_BITS);
        if (l > 4095) l=4095;
        else if (l<-4096) l=-4096;
        *up++ = _l2u[l];
    }
}
