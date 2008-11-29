/*******************************************************************************
 * Console.cpp
 *
 * Copyright (c) 2008 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class  to handle console output
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

	// Clear Console
	printf("\x1b[J");

	// Re-Print
	if (!Enable)
	{
		printf(Output.c_str()); 
	}
}

/*******************************************************************************
 * StartMenu: Prepare Console for Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::StartMenu()
{
	// Clear Previous Menu
	ClearMenu();
	SavedPos = false;
	MenuStart = Output.length();
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
	printf("\x1b[s");
	SavedPos = true;
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
