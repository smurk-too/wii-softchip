/*******************************************************************************
 * Logger.h
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to log program output
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include "Storage.h"

//--------------------------------------
// Logger Class

class Logger
{
public:
	bool OpenLog(const char* Filename);
	void CloseLog();
	void Write(const char* Message, ...);

	bool ShowTime;

protected:
	FILE *LogFile;

	Logger();
	Logger(const Logger&);
	Logger& operator= (const Logger&);

	virtual ~Logger();

public:
	inline static Logger* Instance()
	{
		static Logger instance;
		return &instance;
	}
};

