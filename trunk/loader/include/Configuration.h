/*******************************************************************************
 * Configuration.h
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

#pragma once

//--------------------------------------
// Includes

#include <stdio.h>

#include "Memory_Map.h"
#include "Storage.h"

#define Default_IOS 36

//--------------------------------------
// ConfigData

namespace ConfigData
{
	const byte LastVersion = 4;
	const char Signature[] = "B5662343D78AD6D";
	const char SoftChip_Folder[] = "sd:/SoftChip";
	const char Default_ConfigFile[] = "sd:/SoftChip/Default.cfg";
	const char Default_LogFile[] = "sd:/SoftChip/Default.log";
	
	struct Ver4
	{
		char		IOS;
		signed char	Language;
		bool		SysVMode;
		bool		AutoBoot;
		bool		Silent;
		bool		Logging;
		bool		Remove_002;
		bool		Fake_IOS_Version;
	} __attribute__((packed));

	struct Ver3
	{
		char		IOS;
		signed char	Language;
		bool		SysVMode;
		bool		AutoBoot;
		bool		Silent;
		bool		Logging;
		bool		Remove_002;
	} __attribute__((packed));

	struct Ver2
	{
		char		IOS;
		signed char	Language;
		bool		SysVMode;
		bool		AutoBoot;
		bool		Silent;
		bool		Logging;
	} __attribute__((packed));

	struct Ver1
	{
		char		IOS;
		signed char	Language;
		bool		SysVMode;
		bool		AutoBoot;
	} __attribute__((packed));
}

//--------------------------------------
// Default Configuration Class

class Configuration
{
public:
	bool Read(const char* Path);
	bool Save(const char* Path);

	ConfigData::Ver4 Data;

protected:
	virtual bool Parse(FILE *fp);

	Configuration();
	Configuration(const Configuration&);
	Configuration& operator= (const Configuration&);

	virtual ~Configuration();

public:
	inline static Configuration* Instance()
	{
		static Configuration instance;
		return &instance;
	}
};

//--------------------------------------
// Derived Configurations

class ConfigVer4 : public Configuration
{
protected:
	bool Parse(FILE *fp);
};

class ConfigVer3 : public Configuration
{
protected:
	bool Parse(FILE *fp);
};

class ConfigVer2 : public Configuration
{
protected:
	bool Parse(FILE *fp);
};

class ConfigVer1 : public Configuration
{
protected:
	bool Parse(FILE *fp);
};

