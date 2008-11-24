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

#include <stdlib.h>

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
	char Buffer[256];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);
	Output += std::string(Buffer);

	if (!Silent) printf(Buffer);	
}

/*******************************************************************************
 * SetSilent: Activate Silent Status
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::SetSilent(bool Value)
{
	if (Silent == Value) return;
	Silent = Value;

	printf("\x1b[J"); // Clear Screen
	if (!Value) printf(Output.c_str());
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
	Output.clear();
	printf("\x1b[J"); // Clear Screen
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
	ClearMenu();
	if (Silent) return;
	printf("\x1b[s");
}

/*******************************************************************************
 * CreateOption: Create an Option for a Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns a pointer to the Option created
 *
 ******************************************************************************/

Console::Option *Console::CreateOption(std::string Message, std::string *Options, dword Max, dword Value)
{
	if (Silent) return NULL;
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
	if (Silent || !Menu.size()) return;

	dword i		= 0;
	int Cols	= 0;
	CON_GetMetrics(&Cols, (int*)&i);

	// Change Option
	if (Controls->Up.Active)
		iMenu = ((!iMenu) ? Menu.size() : iMenu) - 1;
	else if (Controls->Down.Active)
		if (++iMenu >= Menu.size()) iMenu = 0;

	// Change Value
	if (Controls->Left.Active)
		--Menu[iMenu]->Index;
	else if (Controls->Right.Active)
		++Menu[iMenu]->Index;

	// Fix Position
	if (Menu[iMenu]->Index < 0) Menu[iMenu]->Index = Menu[iMenu]->Max + Menu[iMenu]->Index;
	if (Menu[iMenu]->Index >= (int)Menu[iMenu]->Max) Menu[iMenu]->Index = Menu[iMenu]->Index - Menu[iMenu]->Max;

	printf("\x1b[u"); // Return
	for (i = 0; i < Cols * Menu.size(); i++) printf(" "); // Clear Lines
	printf("\x1b[u"); // Return

	for (i = 0; i < Menu.size(); i++)
	{
		printf(Menu[i]->Message.c_str()); // Print Message
		printf("\x1b[%um", (iMenu == i) ? 32 : 37); // Set Color

		if (Menu[i]->Options)
			printf("%s\n", Menu[i]->Options[Menu[i]->Index].c_str());
		else
			printf("%d\n", Menu[i]->Index);

		printf("\x1b[37m"); // Reset Color
	}

	printf("\n"); // End of Menu
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

/*******************************************************************************
 * ReStartMenu: Re-Save Position
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Console::ReStartMenu()
{
	if (Silent) return;
	printf("\x1b[s");
}
