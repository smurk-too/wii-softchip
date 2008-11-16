/*******************************************************************************
 * Input.h
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 * Copyright (c) 2008 luccax
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to handle input
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <ogc/pad.h>
#include <wiiuse/wpad.h>

//--------------------------------------
// Input Class

class Input
{
public:
	static void Init();
	void Update();
	bool GetInput(unsigned int Wiimote, unsigned int GC, unsigned int Classic);

	#define GetSimpleInput(Wiimote, GC, Classic) \
		GetInput(WPAD_BUTTON_##Wiimote, PAD_BUTTON_##GC, WPAD_CLASSIC_BUTTON_##Classic)

private:
	unsigned int Buttons;
	unsigned int GC_Buttons;

protected:
	Input();
	Input(const Input&);
	Input& operator= (const Input&);

	virtual ~Input();

public:
	inline static Input* Instance()
	{
		static Input instance;
		return &instance;
	}
};

