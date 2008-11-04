/*******************************************************************************
 * WiiDisc.h
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains structures to define Wii Disc information
 *
 ******************************************************************************/


//--------------------------------------
// Includes

#include "Memory_Map.h"

//--------------------------------------
// WiiDisc Structures

// TODO: Write structures to define Wii Disc information

namespace Wii_Disc
{
struct Header
{
	dword	ID;
	word	Publisher;
	word	Version;
	byte	Audio_Stream;
	byte	Buffer_Size;
	byte	Unused[14];
	dword	Magic;					// 0x5d1c9ea3 or disc is invalid
	byte	Unused2[4];
	byte	Title[60];
	byte	Disable_Verify;			// Will fail if set
	byte	Disable_Encryption;		// Will fail if set
	byte	Padding[0x7a2];
} __attribute__((__packed__));

struct Partition_Descriptor
{
	dword	Primary_Count;
	dword	Primary_Offset;
	dword	Secondary_Count;
	dword	Secondary_Offset;
	dword	Tertiary_Count;
	dword	Tertiary_Offset;
	dword	Quaternary_Count;
	dword	Quaternary_Offset;
} __attribute__((__packed__));

struct Partition_Info
{
	dword	Offset;
	dword	Type;
};

}
