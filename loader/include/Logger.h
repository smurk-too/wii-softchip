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
#include <fat.h>

//--------------------------------------
// Logger Class

class Logger
{
public:
	void Initialize();
	void Release();
	void Write(const char* Filename, const char* Message, ...);

protected:
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

