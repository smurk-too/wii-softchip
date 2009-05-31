/*******************************************************************************
 * Console.cpp
 *
 * Copyright (c) 2009 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to handle console output
 *
 ******************************************************************************/

//--------------------------------------
// Includes

#include "Console.h"

//--------------------------------------
// Console Class

/*******************************************************************************
 * Console: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Console::Console() {}

/*******************************************************************************
 * ~Console: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Console::~Console() {}

/*******************************************************************************
 * Print: printf
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Print(const char *Format, ...)
{
	char Buffer[1024];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);
	Output += string(Buffer);

	if (!Silent) printf(Buffer);
}

/*******************************************************************************
 * PrintErr: Red printf
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::PrintErr(const char *Format, ...)
{
	SetColor(Color_Red, true);

	char Buffer[1024];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);
	Output += string(Buffer);

	if (!Silent) printf(Buffer);

	SetColor(Color_White, false);
}

/*******************************************************************************
 * Clear: Clear Screen
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Clear()
{
	// Clear Output Buffer
	Output.clear();

	// Clear Console
	printf("\x1b[J");
}

/*******************************************************************************
 * Print_Disclaimer: Print the Disclaimer
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Print_Disclaimer()
{
	// Clear Console
	printf("\x1b[J");

	SetColor(Color_White, true);
	printf("Wii SoftChip v0.0.1-pre\n");

	SetColor(Color_White, false);
    printf("This software is distributed under the terms\n");
    printf("of the GNU General Public License (GPLv3)\n");
    printf("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n");
	
	SetColor(Color_Red, true);
	printf("This software is for free, if you paid for it, you got ripped off!\n");
	printf("\n");

	SetColor(Color_White, true);
	printf("Official Homepage: http://www.softchip-mod.com/\n");
	printf("Sourcecode available at: http://code.google.com/p/wii-softchip/\n");
	printf("Official irc chatroom: irc://irc.freenode.org/SoftChip\n");
	printf("\n");
}


/*******************************************************************************
 * Print_Help: Print the Help Screen
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Print_Help()
{
  //printf("123456789012345678901234567890123456789012345678901234567890123456789012345"); // Test line to see how much space can be used
	// Clear Console
	printf("\x1b[J");

	SetColor(Color_White, false);
	printf("What is required to play backups on Wiis wihout hardware modification?\n");
	printf("To play backups on these Wiis, you need to have an IOS installed that\n");
	printf("allows to read from DVD-Rs as if they were retail discs. One of Waninkoko's\n");
	printf("cIOS for exampe. DVD+Rs need to be booktyped to dvd-rom in order to work.\n");
	printf("\n");

	printf("Which IOS to use:\n");
	printf("To play retail discs or backups with hardware modification, use the\n");
	printf("'Load requested IOS' function to always use the correct IOS.\n");
	printf("To play backups without hardware modification, use IOS249(the cIOS).\n");
	printf("\n");

	printf("What does 'Fake IOS version' do:\n");
	printf("If enabled, it's written into the memory that the IOS requested by the game\n");
	printf("is loaded. If disabled, the correct values are written into the memory.\n");
	printf("Enabling this option removes the 002 error in a better than the\n");
	printf("'Remove 002 Protection' option.\n");
	printf("\n");
	
	printf("What's 'Autoboot'?\n");
	printf("If enabled, SoftChip automatically starts the insterted disc on startup. If\n");
	printf("the 'Silent' option is also enabled, the screen stays black while starting\n");
	printf("the game. The autoboot can be canceled by tapping the '1' button.\n");
	printf("\n");

	printf("What's 'Patch Country Strings'?\n");
	printf("This is an option for import games only. When encountering problems with an\n");
	printf("import game, this option might be required. Mostly japanese users need\n");
	printf("this to play certain PAL and US games.\n");
	printf("\n");
}

/*******************************************************************************
 * Reprint: Print the complete Output again
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Reprint()
{
	// Clear Console
	printf("\x1b[J");

	if (!Silent)
	{
		printf(Output.c_str()); 
	}
}

/*******************************************************************************
 * SetColor: Change Foreground Color
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::SetColor(int Color, bool Bright)
{
	Print("\x1b[%u;%um", Color, Bright);
}

/*******************************************************************************
 * SetSilent: Activate Silent Status
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::SetSilent(bool Enable)
{
	// Avoid Clearing
	if (Silent == Enable) return;

	// Set
	Silent = Enable;

	Reprint();
}

/*******************************************************************************
 * Save_Cursor: Saves Console Position
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns Saved Position
 *
 ******************************************************************************/

dword Console::Save_Cursor()
{
	// Save Output Length
	return Output.length();
}

/*******************************************************************************
 * Restore_Cursor: Restore Console Position
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::Restore_Cursor(dword Position)
{
	// Verify
	if (Position > Output.length());

	// Restore
	printf("\x1b[J");
	Print(Output.substr(0, Position).c_str());
	Output.erase(Position);
}

/*******************************************************************************
 * CreateMenu: Prepare Console for Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::CreateMenu()
{
	// Clear Previous Menu
	ClearMenu();
	SavedPos = false;

	// Set Menu Position
	MenuStart = Save_Cursor();
}

/*******************************************************************************
 * CreateOption: Create an Option for a Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns a pointer to the Option created
 *
 ******************************************************************************/

Console::Option *Console::CreateOption(string Message, string *Options, int Max, int Value)
{
	Option *Op = new Option();

	Op->Index = Value;
	Op->Max = Max;
	Op->Message = Message;
	Op->Options = Options;

	Menu.push_back(Op);
	return Op;
}

/*******************************************************************************
 * PrintMenu: Print Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::UpdateMenu(Input *Controls)
{
	if (Menu.size() == 0) return;
	int Cols = 0, i = 0;

	// Get Console Metrics
	CON_GetMetrics(&Cols, &i);

	// Change Option
	iMenu += (Controls->Down.Active) - (Controls->Up.Active);

	if (iMenu < 0) iMenu += Menu.size();
	if (iMenu >= (int)Menu.size()) iMenu -= Menu.size();

	// Change Value
	Menu[iMenu]->Index += (Controls->Right.Active) - (Controls->Left.Active);

	if (Menu[iMenu]->Index < 0) Menu[iMenu]->Index += Menu[iMenu]->Max;
	if (Menu[iMenu]->Index >= (int)Menu[iMenu]->Max) Menu[iMenu]->Index -= Menu[iMenu]->Max;

	// Erase Previous Menu
	Output.erase(MenuStart);

	// Return and Erase
	if (!Silent && SavedPos)
	{
		printf("\x1b[u");
		printf("\x1b[%uA", Menu.size() + 1);

		for (i = 0; i < Cols * (int)Menu.size(); i++) printf(" "); // Clear Lines
		printf("\r\x1b[%uA", Menu.size());
	}

	for (i = 0; i < (int)Menu.size(); i++)
	{
		Print(Menu[i]->Message.c_str());				// printf Message
		if (iMenu == i) SetColor(Color_Green, false);	// Set Color

		if (Menu[i]->Options)
			Print("%s\n", Menu[i]->Options[Menu[i]->Index].c_str());
		else
			Print("%d\n", Menu[i]->Index);

		SetColor(Color_White, false);
	}

	// End of Menu
	Print("\n");

	// Save Position
	if (!Silent) 
	{
		printf("\x1b[s");
		SavedPos = true;
	}
}

/*******************************************************************************
 * ClearMenu: Clear Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::ClearMenu()
{
	iMenu = 0;

	if (Menu.size() > 0)
	{
		// Free every Option pointer
		for (dword i = 0; i < Menu.size(); i++)
			delete Menu[i];

		// Clear Vector
		Menu.clear();
	}	
}
