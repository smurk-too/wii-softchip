/*******************************************************************************
 * Input.cpp
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 * Copyright (c) 2008 luccax
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains implementation of a class to handle input
 *
 ******************************************************************************/

//--------------------------------------
// Includes

#include "Input.h"

//--------------------------------------
// Input Class

/*******************************************************************************
 * Input: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Input::Input()
{
	Buttons	= 0;
	GC_Buttons	= 0;
}

/*******************************************************************************
 * ~Input: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

Input::~Input(){}

/*******************************************************************************
 * Init: Init Input System
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Init()
{
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
}

/*******************************************************************************
 * Update: Updates input records
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void Input::Update()
{
	WPAD_ScanPads();
	PAD_ScanPads();
	// Only ButtonsDown will be useful
	Buttons	= WPAD_ButtonsDown(0);
	GC_Buttons	= PAD_ButtonsDown(0);
}

/*******************************************************************************
 * GetInput: Checks for input
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if any of the buttons was pressed
 *
 ******************************************************************************/

bool Input::GetInput(unsigned int Wiimote, unsigned int GC, unsigned int Classic)
{
	return ((Buttons & Wiimote) || (GC_Buttons & GC) || (Buttons & Classic));
}
