/*******************************************************************************
 * Memory_Map.h
 *
 * Copyright (c) 2008 Requiem (requiem@cenury-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains addresses for all direct memory access
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

//--------------------------------------
// Type definitions

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;
typedef unsigned long long	qword;

namespace Memory
{
	enum
	{
		Disc_ID			= 0x80000000,
		Disc_Magic		= 0x80000018,
		Sys_Magic		= 0x80000020,
		Version			= 0x80000024,
		Arena_L			= 0x80000034,
		Arena_H			= 0x80000038,
		Max_FST			= 0x8000003c,
		BI2				= 0x800000f4,
		Bus_Speed		= 0x800000f8,
		CPU_Speed		= 0x800000fc,
		Apploader_Head	= 0x81000000,
		Apploader		= 0x81200000,
		Unk_Bus_Speed	= 0xcd00643c
	};
}
