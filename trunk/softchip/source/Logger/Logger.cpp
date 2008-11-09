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

//--------------------------------------
// Logger Class

/*******************************************************************************
 * Logger: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Logger::Logger()
{
	// TODO: Is there a way to create a directory with libfat?
}

/*******************************************************************************
 * ~Logger: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Logger::~Logger(){}

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
			return;
		}
	}

    // Write the formatted message
    va_start(argp, Message);
	vfprintf(fp, Message, argp);
    va_end(argp);

	// Cleanup
	if (fp != NULL)
	{
		fclose(fp);
	}
}
