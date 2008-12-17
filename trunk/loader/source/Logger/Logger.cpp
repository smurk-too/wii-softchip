/*******************************************************************************
 * Logger.cpp
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains implementation of a class to log program output
 *
 ******************************************************************************/

//--------------------------------------
// Includes

#include <stdio.h>
#include <stdarg.h>

#include "Logger.h"
#include "Configuration.h"

//--------------------------------------
// Logger Class

/*******************************************************************************
 * Logger: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Logger::Logger() {}

/*******************************************************************************
 * ~Logger: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Logger::~Logger(){}

/*******************************************************************************
 * OpenLog: Set Log File
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns bool
 *
 ******************************************************************************/

bool Logger::OpenLog(const char* Filename)
{
	// Logging Activated?
	if (!Configuration::Instance()->Data.Logging) return false;

	// Close Previous
	if (LogFile) fclose(LogFile);

    // Verify File
	LogFile = Storage::Instance()->OpenFile(Filename, "ab");
    if (LogFile == NULL)
    {
        LogFile = Storage::Instance()->OpenFile(Filename, "wb");
        if (LogFile == NULL) return false;
    }

	// Ok
	return true;
}

/*******************************************************************************
 * CloseLog: Closes Log File
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::CloseLog()
{
	if (LogFile) fclose(LogFile);
}

/*******************************************************************************
 * Write: Log a line to the specified file
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::Write(const char* Message, ...)
{
	// Verify File
	if (LogFile == NULL) return;

	// Write Time Tag
	if (ShowTime)
	{
		time_t NowTime;
		struct tm FTime;

		time(&NowTime);
		localtime_r(&NowTime, &FTime);

		fprintf(LogFile, "[%02d/%02d %02d:%02d:%02d] ", FTime.tm_mday, FTime.tm_mon, FTime.tm_hour, FTime.tm_min, FTime.tm_sec);
	}

	// Write the formatted message  
	va_list argp;
    va_start(argp, Message);
    vfprintf(LogFile, Message, argp);
    va_end(argp);
}
