/*******************************************************************************
 * Console.h
 *
 * Copyright (c) 2008 SoftChip Team
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains definition of a class to handle console output
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <string>
#include <vector>

#include "Memory_Map.h"
#include "Input.h"

//--------------------------------------
// Console Class

class Console
{
public:
	void	Print(const char *Format, ...);
	void	SetSilent(bool Value);
	void	Clear();

	struct Option
	{
		int			Index;
		dword		Max;
		std::string	Message;
		std::string	*Options;
	};

	void	StartMenu();
	Option*	CreateOption(std::string Message, std::string *Options, dword Max, dword Value);
	void	UpdateMenu(Input *Controls);
	void	ClearMenu();
	void	ReStartMenu();

protected:
	std::string	Output;
	bool		Silent;

	std::vector<Option*> Menu;
	dword iMenu;

	Console();
	Console(const Console&);
	Console& operator= (const Console&);

	virtual ~Console();

public:
	inline static Console* Instance()
	{
		static Console instance;
		return &instance;
	}
};
