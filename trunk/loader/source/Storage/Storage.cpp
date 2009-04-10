/*******************************************************************************
 * Storage.cpp
 *
 * Copyright (c) 2009 SoftChip Team
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

#include <sdcard/wiisd_io.h>
//#include <ogc/usbstorage.h>
#include <stdio.h>

#include "Storage.h"

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
	__io_wiisd.startup();
	FatOk = fatMountSimple("sd", &__io_wiisd);

    //__io_usbstorage.startup();
	//fatMountSimple("usb", &__io_usbstorage);
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
	fatUnmount("sd");
	__io_wiisd.shutdown();

	//fatUnmount("usb");
	//__io_usbstorage.shutdown();

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
	if (!Verify_FAT())
	{
		// FAT Error (Avoid mkdir)
		return false;
	}

	// Open Target Folder
	DIR* dir = opendir(Path);

	// Already Exists?
	if (dir == NULL)
	{
		// Create
		mode_t Mode = 0777;
		mkdir(Path, Mode);

		// Re-Verify
		closedir(dir);
		dir = opendir(Path);
		if (dir == NULL) return false;
	}

	// Success
	closedir(dir);
	return true;
}

/*******************************************************************************
 * Verify_FAT: Verify if FAT is working
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

bool Storage::Verify_FAT()
{
	// Verify FAT
	DIR* dir = opendir("sd:/");
	if (dir == NULL)
	{
		// FAT Error
		return false;
	}

	// FAT is Ok
	return true;
}
