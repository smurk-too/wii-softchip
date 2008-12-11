/*******************************************************************************
 * Storage.h
 *
 * Copyright (c) 2008 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to handle storage (FAT)
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <fat.h>
#include <dirent.h>

//--------------------------------------
// Storage Class

class Storage
{
public:
	void Initialize_FAT();
	void Release_FAT();

	FILE *OpenFile(const char *Path, const char *Mode);
	bool MakeDir(const char *Path);

protected:
	bool FatOk;

	Storage();
	Storage(const Storage&);
	Storage& operator= (const Storage&);

	virtual ~Storage();

public:
	inline static Storage* Instance()
	{
		static Storage instance;
		return &instance;
	}
};
