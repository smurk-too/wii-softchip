/*******************************************************************************
 * Input.cpp
 *
 * Copyright (c) 2008 SoftChip Team
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

#include <ogc/pad.h>
#include <wiiuse/wpad.h>
#include "Input.h"

Input::Input(){}
Input::~Input(){}

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
}

void Input::Terminate()
{
	WPAD_Shutdown();
}

void Input::Activate(Control* Command, bool Active)
{
	if (!Command) return;
	Command->Active = Active;
}

void Input::Scan()
{
	WPAD_ScanPads();
	PAD_ScanPads();

	int WPAD_Buttons = WPAD_ButtonsDown(0);
	int GC_Buttons	= PAD_ButtonsDown(0);

	if (WPAD_Buttons & Up.WPAD_Binding || GC_Buttons & Up.GC_Binding)
			Up.Active = true;
		else
			Up.Active = false;

	if (WPAD_Buttons & Down.WPAD_Binding || GC_Buttons & Down.GC_Binding)
				Down.Active = true;
			else
				Down.Active = false;

	if (WPAD_Buttons & Left.WPAD_Binding || GC_Buttons & Left.GC_Binding)
				Left.Active = true;
			else
				Left.Active = false;

	if (WPAD_Buttons & Right.WPAD_Binding || GC_Buttons & Right.GC_Binding)
				Right.Active = true;
			else
				Right.Active = false;

	if (WPAD_Buttons & Accept.WPAD_Binding || GC_Buttons & Accept.GC_Binding)
		Accept.Active = true;
	else
		Accept.Active = false;

	if (WPAD_Buttons & Cancel.WPAD_Binding || GC_Buttons & Cancel.GC_Binding)
			Cancel.Active = true;
		else
			Cancel.Active = false;

	if (WPAD_Buttons & Exit.WPAD_Binding || GC_Buttons & Exit.GC_Binding)
			Exit.Active = true;
		else
			Exit.Active = false;
}
