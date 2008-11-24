/*******************************************************************************
 * Configuration.h
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

#pragma once

//--------------------------------------
// Includes

#include <stdio.h>
#include <sys/dir.h>

#include "Memory_Map.h"

#define Default_IOS 249

//--------------------------------------
// ConfigData

namespace ConfigData
{
	const byte LastVersion = 1;
	const char Signature[] = "B5662343D78AD6D";
	const char DefaultFolder[] = "/SoftChip";
	const char DefaultFile[] = "/SoftChip/Default.cfg";

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
	bool CreateFolder(const char* Path);
	void Read(const char* Path);
	bool Save(const char* Path);

	ConfigData::Ver1 Data;

protected:
	virtual void Parse(FILE *fp);

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

class ConfigVer1 : public Configuration
{
protected:
	void Parse(FILE *fp);
};

