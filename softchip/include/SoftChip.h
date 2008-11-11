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

#define Target_IOS 249 // Just for now

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
	int			IOS_Version;
	bool		IOS_Loaded;
	int			Lang_Selected;				// Language Selected
	bool		System_VMode;				// Use System Video Mode

public:
	void Initialize();					// Initializer method
	void Run();							// Main function

private:
	static void Standby();				// Put the console into standby
	static void Reboot();				// Return to system menu

	void 	Load_Disc();								// Loads the disc
	void	Set_VideoMode(char Region);					// Set Video Mode
	bool	Set_GameLanguage(void *Address, int Size);	// Patch Game's Language

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
