/*******************************************************************************
 * cIOS.h
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
 *
 * Special thanks to Waninkoko.
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Contains an implementation of Waninkoko's libcIOS;
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <vector>
#include <malloc.h>
//#include <gccore.h>
#include <ogc/ios.h>
#include <ogc/isfs.h>
#include <ogc/video.h>
#include <ogc/system.h>
#include <ogc/consol.h>
#include <ogc/ios.h>
#include <ogc/es.h>
#include <ogc/ipc.h>
#include <ogc/stm.h>
#include <ogc/color.h>
#include <ogc/cache.h>
#include <ogc/conf.h>
#include <ogc/consol.h>


using namespace std;

#define TITLEID(x)	(0x100000000ULL | x)
#define CERTS_SIZE	0xA00
#define MAX_TITLES	256

//--------------------------------------
// cIOS Class

class cIOS
{
public:
	s32 Load(u32 Version);
	s32 Version();
	s32 Identify();
	s32 List_SysTitles();

	vector<u32> SysTitles;

protected:
	inline cIOS(){}
	inline cIOS(const cIOS&){}
	cIOS& operator= (const cIOS&);
	inline virtual ~cIOS(){}

public:
	s32 GetCerts(signed_blob** Certs, u32* Length);
	s32 GenerateTicket(signed_blob** Ticket, u32* Length);
	s32 GetTMD(u64 TicketID, signed_blob** TMD, u32* Length);

public:
	inline static cIOS* Instance()
	{
		static cIOS instance;
		return &instance;
	}
};
