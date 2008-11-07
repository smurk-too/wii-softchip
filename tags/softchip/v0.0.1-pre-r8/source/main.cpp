/*******************************************************************************
 * main.cpp
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


//--------------------------------------
// Includes

#include "SoftChip.h"

/*******************************************************************************
 * int main: Application Entry Point
 * -----------------------------------------------------------------------------
 * Return Values:
 *
 * 0 = Success
 ******************************************************************************/

int main(int argc, char* argv[])
{
	SoftChip* Application = SoftChip::Instance();
	Application->Run();

	return 0;
}
