/*******************************************************************************
 * SoftChip.h
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <ogcsys.h>
#include "DIP.h"

//--------------------------------------
// SoftChip Class

class SoftChip
{
protected:
	DIP*		DI;							// DIP interface
	bool		Standby_Flag;				// Flag is set when power button is pressed
	bool		Reset_Flag;					// Flag is set when reset button is pressed
	GXRModeObj* vmode;
	void*		framebuffer;

public:
	void Run();							// Main function

private:
	static void Standby();				// Put the console into standby
	static void Reboot();				// Return to system menu

	void 	Load_Disc();					// Loads the disc
	void	Set_VideoMode();

protected:
	SoftChip();
	SoftChip(const SoftChip&);
	SoftChip& operator= (const SoftChip&);

	virtual ~SoftChip();

public:
	inline static SoftChip* Instance()
	{
		static SoftChip instance;
		return &instance;
	}
};
