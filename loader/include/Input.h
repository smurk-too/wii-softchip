/*******************************************************************************
 * Input.h
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

#pragma once

//--------------------------------------
// Includes

#include <gccore.h>

//--------------------------------------
// Input Class

struct Control
{
	bool	Active;
	int		WPAD_Binding;
	int		GC_Binding;
};

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

public:
	void Initialize(void);
	void Terminate();
	void Scan();

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
