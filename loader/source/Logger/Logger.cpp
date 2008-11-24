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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

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

void Logger::Initialize()
{
    // Mount the file system
    if (!fatInitDefault())
    {
        return;
    }
}

/*******************************************************************************
 * Release: Unmount SD
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Logger::Release()
{
	// Unmount FAT
    if (!fatUnmount(PI_INTERNAL_SD)) {
        fatUnsafeUnmount(PI_INTERNAL_SD);
    }

	// Shutdown sdio
	sdio_Startup();
	sdio_Deinitialize();
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
    va_list argp;
    FILE *fp;

    // Open existing file
    fp = fopen(Filename, "ab");
    if (fp == NULL)
    {
        fp = fopen(Filename, "wb");
        if (fp == NULL)
        {
            // If you are here, it's mostly because libfat init got whacked by ios_reload
			// or there's no SD card inserted
            return;
        }
    }

    // Write the formatted message
    va_start(argp, Message);
    vfprintf(fp, Message, argp);
    va_end(argp);

    // Cleanup
    fclose(fp);
}
