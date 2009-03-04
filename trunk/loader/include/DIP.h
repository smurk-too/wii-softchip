/*******************************************************************************
 * DIP.h
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Interface library to access /dev/di using the cIOS Ioctl commands.
 *
 ******************************************************************************/

#pragma once

//--------------------------------------
// Includes

#include <ogc/mutex.h>

//--------------------------------------
// DIP Class

class DIP
{
public:

	bool Initialize();

	//----------------------------------
	// Ioctl Commands

	int Inquiry(void* Drive_ID);
	int	Read_DiscID(dvddiskid* Disc_ID);
	int Read(void* Buffer, unsigned int size, unsigned int offset);
	int	Wait_CoverClose();
	int Verify_Cover(bool *Inserted);
	int Reset();
	int Read_Unencrypted(void* Buffer, unsigned int size, unsigned int offset);
	int Enable_DVD();
	int Set_OffsetBase(unsigned int Base);
	int Get_OffsetBase(unsigned int* Base);
	int	Open_Partition(unsigned int Offset, void* Ticket, void* Certificate, unsigned int Cert_Len, void* Out);
	int Stop_Motor();
	void Close();

private:

	static unsigned int Mutex;
	static unsigned int Command[8];
	static unsigned int Output[8];

	void Lock();
	void Unlock();

protected:
	DIP();
	DIP(const DIP&);
	DIP& operator= (const DIP&);

	virtual ~DIP();

	int	Device_Handle;

public:
	inline static DIP* Instance()
	{
		static DIP instance;
		return &instance;
	}
};
