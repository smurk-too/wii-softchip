/*******************************************************************************
 * Input.h
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

#pragma once

//--------------------------------------
// Includes

#include <gccore.h>

//--------------------------------------
// Struct Control

struct Control
{
	bool	Active;
	int		WPAD_Binding;
	int		GC_Binding;
};

//--------------------------------------
// Input Class

class Input
{
public:
	Control Up;
	Control Down;
	Control Left;
	Control Right;
	Control	Accept;
	Control Cancel;
	Control Exit;
	Control Menu;
	Control Plus;
	Control Minus;
	Control Any;

public:
	void Initialize(void);
	void Terminate();
	void Scan();

	bool Wait_ButtonPress(Control *Button, unsigned int Timeout);
	void Press_AnyKey(const char *Message);

protected:
	void Activate(Control* Command, bool Active);

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
