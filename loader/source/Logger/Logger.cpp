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
#include <ogcsys.h>

#include "Logger.h"

extern "C" bool sdio_Startup(void);
extern "C" bool sdio_Deinitialize(void);

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
 * Initialize: Init Fat System
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::Initialize_FAT()
{
    // Mount the file system
    if (!fatInitDefault())
    {
		FatOk = false;
        return;
    }
	FatOk = true;
}

/*******************************************************************************
 * Release: Unmount SD
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::Release_FAT()
{
	// Unmount FAT
    /*if (!fatUnmount(PI_INTERNAL_SD)) {
        fatUnsafeUnmount(PI_INTERNAL_SD);
    }*/

	// Shutdown sdio
	sdio_Startup();
	sdio_Deinitialize();
	FatOk = false;
}

/*******************************************************************************
 * Write: Log a line to the specified file
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::Write(const char* Filename, const char* Message, ...)
{
	// Avoid Errors
	if (!FatOk) return;

    va_list argp;
    FILE *fp;

    // Open existing file
    fp = fopen(Filename, "ab");
    if (fp == NULL)
    {
        fp = fopen(Filename, "wb");
        if (fp == NULL)
		{
			return;
		}
    }

	// Write Time Tag
	if (ShowTime)
	{
		time_t NowTime;
		struct tm FTime;

		time(&NowTime);
		localtime_r(&NowTime, &FTime);

		fprintf(fp, "[%02d/%02d %02d:%02d:%02d] ", FTime.tm_mday, FTime.tm_mon, FTime.tm_hour, FTime.tm_min, FTime.tm_sec);
	}

	// Write the formatted message    
    va_start(argp, Message);
    vfprintf(fp, Message, argp);
    va_end(argp);

    // Cleanup
    fclose(fp);
}
