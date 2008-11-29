/*******************************************************************************
 * Console.h
 *
 * Copyright (c) 2008 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to handle console output
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <string>
#include <vector>

#include "Memory_Map.h"
#include "Input.h"

using namespace std;

//--------------------------------------
// Colors

#define Color_Black		30
#define Color_Red		31
#define Color_Green		32
#define Color_Yellow	33
#define Color_Blue		34
#define Color_Magenta	35
#define Color_Cyan		36
#define Color_White		37

//--------------------------------------
// Console Class

class Console
{
public:
	struct Option
	{
		int		Index;
		int		Max;
		string	Message;
		string	*Options;
	};

protected:
	// Buffer Related
	string	Output;		// Console Buffer
	dword	MenuStart;	// Start Position of the Menu
	bool	Silent;		// Silent Option

	// Menu Related	
	vector<Option*> Menu;	// Vector
	int		iMenu;			// Menu Index
	bool	SavedPos;		// Is the position saved?

public:
	// Console Related
	void	Print(const char *Format, ...);		// Print Formatted
	void	PrintErr(const char *Format, ...);	// Print Formatted (Red)
	void	SetColor(int Color, bool Bright);	// Set Foreground Color
	void	SetSilent(bool Enable);				// Enable or Disable Silent Option
	void	Clear();							// Clear the Console
	
	// Menu Related
	void	StartMenu();						// Prepare Console for Menu
	Option*	CreateOption(string Message, string *Options, int Max, int Value);
	void	UpdateMenu(Input *Controls);		// Print Menu
	void	ClearMenu();						// Clear Menu

protected:
	Console();
	Console(const Console&);
	Console& operator= (const Console&);

	virtual ~Console();

public:
	inline static Console* Instance()
	{
		static Console instance;
		return &instance;
	}
};
