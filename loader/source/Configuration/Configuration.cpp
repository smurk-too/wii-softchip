/*******************************************************************************
 * Configuration.cpp
 *
 * Copyright (c) 2008 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to load config from SD
 *
 ******************************************************************************/

//--------------------------------------
// Includes

#include <string.h>
#include <stdlib.h>

#include "Configuration.h"

//--------------------------------------
// Configuration Class

/*******************************************************************************
 * Configuration: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Configuration::Configuration() {}

/*******************************************************************************
 * ~Configuration: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Configuration::~Configuration() {}

/*******************************************************************************
 * CreateFolder: Create a folder
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if created
 *
 ******************************************************************************/

bool Configuration::CreateFolder(const char* Path)
{
	DIR_ITER* dir = diropen(Path);

	if (dir == NULL)
	{
		// Create
		mode_t mode = 0777;
		mkdir(Path, mode);

		// Verify
		dirclose(dir);
		dir = diropen(Path);
		if (dir == NULL) return false;
	}

	dirclose(dir);
	return true;
}

/*******************************************************************************
 * Read: Read a Config File
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Configuration::Read(const char *Path)
{
	Configuration *Parser = NULL;
	byte Buffer[64];
	FILE *fp;

	// Open File
    fp = fopen(Path, "rb");
    if (fp == NULL)
    {
		Parse(0); // Use Default Settings
        return;
    }

	try
	{
		// Read Header
		if (fread(Buffer, 1, 16, fp) != 16)
		{
			throw "Read Error";
		}

		// Verify Signature
		if (memcmp(Buffer, ConfigData::Signature, 15) != 0)
		{
			throw "Invalid Signature";
		}

		// Get File Version		
		switch (Buffer[15]) 
		{
			case 1:		// Version 1
				Parser = new ConfigVer1();
				break;

			default:	// Unknown Version
				throw "Unknow File Version";
		}
		
		// Parse
		Parser->Parse(fp);

		// Copy Settings
		memcpy(&Data, &Parser->Data, sizeof(Data));
	}
	catch (const char* Message)
    {
		// Use Default Settings
		Parse(0);
	}

	// Close
	if (Parser) delete Parser;
	if (fp) fclose(fp);
}

/*******************************************************************************
 * Save: Save a Config File
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if saved successfully
 *
 ******************************************************************************/

bool Configuration::Save(const char* Path)
{
	FILE *fp;

	// Open File
    fp = fopen(Path, "wb");
    if (fp == NULL)
    {
        return false;
    }

	// Write Signature and Version
	fwrite(ConfigData::Signature, 1, 15, fp);
	fwrite(&ConfigData::LastVersion, 1, 1, fp);

	// Write Body
	fwrite(&Data, 1, sizeof(Data), fp);

    // Close
    fclose(fp);
	return true;
}

/*******************************************************************************
 * Parse: Parse the file
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Configuration::Parse(FILE *fp)	// Default Settings
{
	Data.IOS = Default_IOS;
	Data.Language = -1;
	Data.SysVMode = true;
	Data.AutoBoot = false;
}

void ConfigVer1::Parse(FILE *fp)	// Ver1 Settings
{
	// Read Default
	Configuration::Parse(0);

	// Get File Data
	ConfigData::Ver1 Temp;
	if (fread(&Temp, 1, sizeof(Temp), fp) != sizeof(Temp))
	{
		return;
	}

	// Convert
	Data.IOS = Temp.IOS;
	Data.Language = Temp.Language;
	Data.AutoBoot = Temp.AutoBoot;
	Data.SysVMode = Temp.SysVMode;
}
