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
#include "Input.h"
#include "Console.h"
#include "Configuration.h"

//--------------------------------------
// SoftChip Class

class SoftChip
{
protected:
	DIP*			DI;						// DIP interface
	Input*			Controls;
	Console*		Out;
	Configuration*	Cfg;

	bool			Standby_Flag;			// Flag is set when power button is pressed
	bool			Reset_Flag;				// Flag is set when reset button is pressed
	GXRModeObj*		vmode;
	void*			framebuffer;
	int				IOS_Version;
	bool			IOS_Loaded;

public:
	void Initialize();					// Initializer method
	void Run();							// Main function

private:
	static void Standby();				// Put the console into standby
	static void Reboot();				// Return to system menu

	void 	Load_Disc();								// Loads the disc
	void	SelectIOS();								// Select the IOS
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
