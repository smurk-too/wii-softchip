/*******************************************************************************
 * Input.cpp
 *
 * Copyright (c) 2009 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *
 ******************************************************************************/


//--------------------------------------
// Includes

#include <string>

#include <ogc/pad.h>
#include <wiiuse/wpad.h>

#include "Input.h"
#include "Console.h"

//--------------------------------------
// Input Class

/*******************************************************************************
 * Input: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Input::Input(){}

/*******************************************************************************
 * ~Input: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Input::~Input(){}

/*******************************************************************************
 * Initialize: Initialize Input Class
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Initialize()
{
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	Up.Active		= false;
	Down.Active		= false;
	Left.Active		= false;
	Right.Active	= false;
	Accept.Active	= false;
	Cancel.Active	= false;
	Exit.Active		= false;
	Menu.Active		= false;
	Plus.Active		= false;
	Minus.Active	= false;
	Info.Active		= false;
	Any.Active		= false;

	Up.WPAD_Binding		= WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP;
	Up.GC_Binding		= PAD_BUTTON_UP;

	Down.WPAD_Binding	= WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN;
	Down.GC_Binding		= PAD_BUTTON_DOWN;

	Left.WPAD_Binding	= WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT;
	Left.GC_Binding		= PAD_BUTTON_LEFT;

	Right.WPAD_Binding	= WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT;
	Right.GC_Binding	= PAD_BUTTON_RIGHT;

	Accept.WPAD_Binding = WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A;
	Accept.GC_Binding	= PAD_BUTTON_A;

	Cancel.WPAD_Binding	= WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B;
	Cancel.GC_Binding	= PAD_BUTTON_B;

	Exit.WPAD_Binding	= WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME;
	Exit.GC_Binding		= PAD_BUTTON_START;

	Menu.WPAD_Binding	= WPAD_BUTTON_1;
	Menu.GC_Binding		= 0;

	Plus.WPAD_Binding	= WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS;
	Plus.GC_Binding		= PAD_BUTTON_X;

	Minus.WPAD_Binding	= WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS;
	Minus.GC_Binding	= PAD_BUTTON_Y;

	Info.WPAD_Binding	= WPAD_BUTTON_2;
	Info.GC_Binding		= PAD_TRIGGER_Z;
}

/*******************************************************************************
 * Terminate: Terminate Input Class
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Terminate()
{
	WPAD_Shutdown();
}

/*******************************************************************************
 * Activate: Change Control Status
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Activate(Control* Command, bool Active)
{
	if (!Command) return;
	Command->Active = Active;
}

/*******************************************************************************
 * Scan: Update Controls
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Scan()
{
	WPAD_ScanPads();
	PAD_ScanPads();

	int WPAD_Buttons = WPAD_ButtonsDown(0);
	int GC_Buttons	= PAD_ButtonsDown(0);

	Activate(&Up, (WPAD_Buttons & Up.WPAD_Binding || GC_Buttons & Up.GC_Binding));
	Activate(&Down, (WPAD_Buttons & Down.WPAD_Binding || GC_Buttons & Down.GC_Binding));
	Activate(&Left, (WPAD_Buttons & Left.WPAD_Binding || GC_Buttons & Left.GC_Binding));
	Activate(&Right, (WPAD_Buttons & Right.WPAD_Binding || GC_Buttons & Right.GC_Binding));
	Activate(&Accept, (WPAD_Buttons & Accept.WPAD_Binding || GC_Buttons & Accept.GC_Binding));
	Activate(&Cancel, (WPAD_Buttons & Cancel.WPAD_Binding || GC_Buttons & Cancel.GC_Binding));
	Activate(&Exit, (WPAD_Buttons & Exit.WPAD_Binding || GC_Buttons & Exit.GC_Binding));
	Activate(&Menu, (WPAD_Buttons & Menu.WPAD_Binding || GC_Buttons & Menu.GC_Binding));
	Activate(&Plus, (WPAD_Buttons & Plus.WPAD_Binding || GC_Buttons & Plus.GC_Binding));
	Activate(&Minus, (WPAD_Buttons & Minus.WPAD_Binding || GC_Buttons & Minus.GC_Binding));
	Activate(&Info, (WPAD_Buttons & Info.WPAD_Binding || GC_Buttons & Info.GC_Binding));
	Activate(&Any, (WPAD_Buttons || GC_Buttons));
}

/*******************************************************************************
 * Wait_ButtonPress: Wait for Input
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if pressed
 *
 ******************************************************************************/

bool Input::Wait_ButtonPress(Control *Button, unsigned int Timeout)
{
	time_t Sec = time(NULL) + Timeout;

	while (!Timeout || time(NULL) < Sec)
	{
		Scan();

		if (Button->Active)
			return true;
	}

	return false;
}

/*******************************************************************************
 * Press_AnyKey: Helper for "Press Any Key to..." 
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Press_AnyKey(const char *Message)
{
	Console::Instance()->Print(Message);
	Wait_ButtonPress(&Any, 0);	
}
