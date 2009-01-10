/*******************************************************************************
 * Apploader.h
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains structures to define Apploader information
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include "Memory_Map.h"

//--------------------------------------
// Apploader information

namespace Apploader
{
	typedef void 	(*Report)	(const char*, ...);
	typedef void 	(*Enter)	(Report);
	typedef int 	(*Load)		(void** Dest, int* Size, int* Offset);
	typedef void* 	(*Exit)		();
	typedef void 	(*Start)	(Enter*,Load*,Exit*);

	struct Header
	{
		char	Revision[16];		// Apploader compile date
		Start	Entry_Point;		// Pointer to entry point
		int		Size;				// Size of Apploader
		int		Trailer_Size;		// Size of trailer
		byte	Padding[4];			// Padded with zeroes
	} __attribute__((packed));

}
