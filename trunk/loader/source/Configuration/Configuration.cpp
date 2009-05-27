/*******************************************************************************
 * Configuration.cpp
 *
 * Copyright (c) 2009 SoftChip Team
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

#include <string>

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
 * Read: Read a Config File
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns bool
 *
 ******************************************************************************/

bool Configuration::Read(const char *Path)
{
	Configuration *Parser = NULL;
	bool Result = false;
	byte Buffer[64];
	FILE *fp = NULL;

	try
	{
		// Open File
		fp = Storage::Instance()->OpenFile(Path, "rb");
		if (fp == NULL)
		{
			throw "Open Error";
		}
		
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
			case 5:		// Version 5
				Parser = new ConfigVer5();
				break;

			case 4:		// Version 4
				Parser = new ConfigVer4();
				break;

			case 3:		// Version 3
				Parser = new ConfigVer3();
				break;

			case 2:		// Version 2
				Parser = new ConfigVer2();
				break;

			case 1:		// Version 1
				Parser = new ConfigVer1();
				break;

			default:	// Unknown Version
				throw "Unknow File Version";
		}
		
		// Parse
		if (!Parser->Parse(fp))
		{
			throw "Parse Error";
		}

		// Copy Settings
		memcpy(&Data, &Parser->Data, sizeof(Data));

		// Success
		Result = true;
	}
	catch (const char* Message)
    {
		// Use Default Settings
		Parse(0);
	}

	// Close
	if (Parser) delete Parser;
	if (fp) fclose(fp);
	return Result;
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
	// Open File
    FILE *fp = Storage::Instance()->OpenFile(Path, "wb");
    if (fp == NULL)
    {
        return false;
    }

	// Write Signature and Version
	if (fwrite(ConfigData::Signature, 1, 15, fp) != 15) return false;
	if (fwrite(&ConfigData::LastVersion, 1, 1, fp) != 1) return false;

	// Write Body
	if (fwrite(&Data, 1, sizeof(Data), fp) != sizeof(Data))
	{
		return false;
	}

    // Close
    fclose(fp);
	return true;
}

/*******************************************************************************
 * Parse: Parse the file
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns bool
 *
 ******************************************************************************/

bool Configuration::Parse(FILE *fp)	// Default Settings
{
	Data.IOS = Default_IOS;
	Data.Language = -2;
	Data.SysVMode = false;
	Data.AutoBoot = false;
	Data.Silent = false;
	Data.Logging = false;
	Data.Remove_002 = false;
	Data.Fake_IOS_Version = false;
	Data.Load_requested_IOS = false;
	Data.Country_String_Patching = false;
	
	return true;
}

bool ConfigVer5::Parse(FILE *fp)	// Ver5 Settings
{
	// Get File Data
	if (fread(&Data, 1, sizeof(Data), fp) != sizeof(Data))
		return false;
	else
		return true;
}

bool ConfigVer4::Parse(FILE *fp)	// Ver4 Settings
{
	// Get File Data
	ConfigData::Ver4 Temp;
	if (fread(&Temp, 1, sizeof(Data), fp) != sizeof(Data))
		return false;

	// Convert
	Configuration::Parse(0);
	Data.IOS = Temp.IOS;
	Data.Language = Temp.Language;
	Data.AutoBoot = Temp.AutoBoot;
	Data.SysVMode = Temp.SysVMode;
	Data.Silent = Temp.Silent;
	Data.Logging = Temp.Logging;
	Data.Remove_002 = Temp.Remove_002;
	Data.Fake_IOS_Version = Temp.Fake_IOS_Version;

	return true;
}

bool ConfigVer3::Parse(FILE *fp)	// Ver3 Settings
{
	// Get File Data
	ConfigData::Ver3 Temp;
	if (fread(&Temp, 1, sizeof(Temp), fp) != sizeof(Temp))
		return false;
	
	// Convert
	Configuration::Parse(0);
	Data.IOS = Temp.IOS;
	Data.Language = Temp.Language;
	Data.AutoBoot = Temp.AutoBoot;
	Data.SysVMode = Temp.SysVMode;
	Data.Silent = Temp.Silent;
	Data.Logging = Temp.Logging;
	Data.Remove_002 = Temp.Remove_002;
	return true;
}

bool ConfigVer2::Parse(FILE *fp)	// Ver2 Settings
{
	// Get File Data
	ConfigData::Ver2 Temp;
	if (fread(&Temp, 1, sizeof(Temp), fp) != sizeof(Temp))
		return false;

	// Convert
	Configuration::Parse(0);
	Data.IOS = Temp.IOS;
	Data.Language = Temp.Language;
	Data.AutoBoot = Temp.AutoBoot;
	Data.SysVMode = Temp.SysVMode;
	Data.Silent = Temp.Silent;
	Data.Logging = Temp.Logging;
	return true;
}

bool ConfigVer1::Parse(FILE *fp)	// Ver1 Settings
{
	// Get File Data
	ConfigData::Ver1 Temp;
	if (fread(&Temp, 1, sizeof(Temp), fp) != sizeof(Temp))
		return false;

	// Convert
	Configuration::Parse(0);
	Data.IOS = Temp.IOS;
	Data.Language = Temp.Language;
	Data.AutoBoot = Temp.AutoBoot;
	Data.SysVMode = Temp.SysVMode;
	return true;
}
