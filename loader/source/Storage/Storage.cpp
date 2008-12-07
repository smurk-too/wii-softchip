/*******************************************************************************
 * Storage.cpp
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

//--------------------------------------
// Includes

#include <stdio.h>
#include <stdarg.h>
#include <ogcsys.h>

#include "Storage.h"

extern "C" bool sdio_Startup(void);
extern "C" bool sdio_Deinitialize(void);

//--------------------------------------
// Storage Class

/*******************************************************************************
 * Storage: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Storage::Storage() {}

/*******************************************************************************
 * ~Storage: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Storage::~Storage() {}

/*******************************************************************************
 * Initialize_FAT: Init Fat System
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Storage::Initialize_FAT()
{
    // Mount the file system
	FatOk = fatInitDefault();
}

/*******************************************************************************
 * Release_FAT: Release FAT System
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Storage::Release_FAT()
{
	// Unmount FAT
    if (!fatUnmount(PI_INTERNAL_SD)) 
	{
        fatUnsafeUnmount(PI_INTERNAL_SD);
    }

	// Shutdown sdio
	sdio_Startup();
	sdio_Deinitialize();
	FatOk = false;
}

/*******************************************************************************
 * OpenFile: Open a file
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

FILE *Storage::OpenFile(const char *Path, const char *Mode)
{
	// Avoid Errors
	if (!FatOk) return NULL;

	// Open File
	return fopen(Path, Mode);
}

/*******************************************************************************
 * MakeDir: Create a Directory
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

bool Storage::MakeDir(const char *Path)
{
	// Avoid Errors
	if (!FatOk) return false;

	// Verify FAT
	DIR_ITER* dir = diropen("fat:/");
	if (dir == NULL)
	{
		// FAT Error (Avoid mkdir)
		return false;
	}

	// Open Target Folder
	dirclose(dir);
	dir = diropen(Path);

	// Already Exists?
	if (dir == NULL)
	{
		// Create
		mode_t Mode = 0777;
		mkdir(Path, Mode);

		// Re-Verify
		dirclose(dir);
		dir = diropen(Path);
		if (dir == NULL) return false;
	}

	// Success
	dirclose(dir);
	return true;
}
