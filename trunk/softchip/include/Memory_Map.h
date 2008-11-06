/*******************************************************************************
 * Memory_Map.h
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
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
typedef unsigned int		dword;
typedef unsigned long long	qword;

namespace Memory
{
	// TODO: Replace these with pointers to help type-safety
	enum
	{
		Disc_ID			= 0x80000000,
		Disc_Region		= 0x80000003,
		Disc_Magic		= 0x80000018,
		Sys_Magic		= 0x80000020,
		Version			= 0x80000024,
		Mem_Size		= 0x80000028,
		Board_Model		= 0x8000002c,
		Arena_L			= 0x80000030,
		Arena_H			= 0x80000034,
		FST				= 0x80000038,
		Max_FST			= 0x8000003c,
		Video_Mode		= 0x800000cc,
		Simulated_Mem	= 0x800000f0,
		BI2				= 0x800000f4,
		Bus_Speed		= 0x800000f8,
		CPU_Speed		= 0x800000fc,
		Dol_Params		= 0x800030f0,
		Offline_Check	= 0x80003180,
		Apploader		= 0x81200000
	};
}

namespace Video
{
	namespace Modes
	{
		enum
		{
			NTSC		= 0x00,
			PAL			= 0x01,
			Debug		= 0x02,
			Debug_PAL	= 0x03,
			MPAL		= 0x04,
			PAL60		= 0x05
		};
	}
}
