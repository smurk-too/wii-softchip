/*******************************************************************************
 * DIP.cpp
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
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

#include <string.h>
//#include <ogcsys.h>
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
#include <ogc/dvd.h>

#include "DIP.h"
#include "Ioctl.h"
#include "Memory_Map.h"

//--------------------------------------
// DIP Class

unsigned int DIP::Mutex = 0;
unsigned int DIP::Command[8];
unsigned int DIP::Output[8];

/*******************************************************************************
 * DIP: Default Constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

DIP::DIP()
{
	this->Device_Handle = -1;

	memset(Command, 0, 0x20);
	memset(Output, 0, 0x20);
}

/*******************************************************************************
 * DIP: Default Destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

DIP::~DIP(){}

void DIP::Lock()
{
	while (LWP_MutexLock(Mutex));
}

void DIP::Unlock()
{
	LWP_MutexUnlock(Mutex);
}

/*******************************************************************************
 * Initialize: Open the /dev/di file descriptor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	true on success, false on error
 *
 ******************************************************************************/

bool DIP::Initialize()
{
	LWP_MutexInit(&Mutex, false);
	if (Device_Handle < 0)
	{
		Device_Handle = IOS_Open("/dev/di", 0);
	}
	return (Device_Handle >= 0) ? true : false;
}

/*******************************************************************************
 * Close: Closes the /dev/di handle
 * -----------------------------------------------------------------------------
 * Return Values:
 *	void
 *
 ******************************************************************************/


void DIP::Close()
{
	if (Device_Handle > 0)
	{
		IOS_Close(Device_Handle);
		Device_Handle = -1;
	}
}

/*******************************************************************************
 * Inquiry: Performs an Ioctl inquiry to fetch drive information
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Inquiry(void* Drive_ID)
{
	if (!Drive_ID) throw "Null DriveID pointer";

	Command[0] = Ioctl::DI_Inquiry << 24;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_Inquiry, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_Inquiry)";
	memcpy(Drive_ID, Output, 8);

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * ReadID: Read the disc identifier
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Read_DiscID(dvddiskid* Disc_ID)
{
	if (!Disc_ID) throw "Null Disc_ID pointer";
	
	Command[0] = Ioctl::DI_ReadID << 24;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_ReadID, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_ReadID)";
	memcpy(Disc_ID, Output, 0x20);
	
	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Read: Read from the disc into a buffer
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Read(void* Buffer, unsigned int size, unsigned int offset)
{
	if (!Buffer) return -1; //throw "Null Buffer";
	if (reinterpret_cast<unsigned int>(Buffer) & 0x1f) throw "Buffer alignment error";

	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_Read << 24;
	Command[1] = size;
	Command[2] = offset >> 2;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_Read, Command, 0x20, Buffer, size);
	
	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_Read)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Read_Unencrypted: Read from the disc into a buffer
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Read_Unencrypted(void* Buffer, unsigned int size, unsigned int offset)
{
	if (!Buffer) throw "Null Buffer";
	if (reinterpret_cast<unsigned int>(Buffer) & 0x1f) throw "Buffer alignment error";

	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_ReadUnencrypted << 24;
	Command[1] = size;
	Command[2] = offset >> 2;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_ReadUnencrypted, Command, 0x20, Buffer, size);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_ReadUnencrypted)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Wait_CoverClose: Waits for the drive to announce cover is closed
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Wait_CoverClose()
{
	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_WaitCoverClose << 24;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_WaitCoverClose, Command, 0x20, Output, 0x20);
	
	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_WaitCoverClose)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Verify_Cover: Verify if a disc is inserted
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Verify_Cover(bool *Inserted)
{
	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_VerifyCover << 24;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_VerifyCover, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_VerifyCover)";
	if (Ret == 1) *Inserted = !((bool)*Output);

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Reset: Resets the drive's hardware
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Reset()
{
	Command[0] = Ioctl::DI_Reset << 24;
	Command[1] = 1;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_Reset, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_Reset)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Enable_DVD: Enables the "DVD Video" mode on the drive
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Enable_DVD()
{
	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_EnableDVD << 24;
	Command[1] = 1;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_EnableDVD, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_EnableDVD)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Set_OffsetBase: Sets the base of read commands
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Set_OffsetBase(unsigned int Base)
{
	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_SetOffsetBase << 24;
	Command[1] = Base >> 2;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_SetOffsetBase, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_SetOffsetBase)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Set_OffsetBase: Sets the base of read commands
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Get_OffsetBase(unsigned int* Base)
{
	memset(Command, 0, 0x20);
	Command[0] = Ioctl::DI_GetOffsetBase << 24;

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_GetOffsetBase, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_GetOffsetBase)";
	if (Ret == 1) *Base = *((unsigned int*)Output);
	
	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Open_Partition: Opens a partition for reading
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Open_Partition(unsigned int Offset, void* Ticket, void* Certificate, unsigned int Cert_Len, void* Out)
{
	static ioctlv	Vectors[5]		__attribute__((aligned(0x20)));

	Command[0] = Ioctl::DI_OpenPartition << 24;
	Command[1] = Offset;

	Vectors[0].data		= Command;
	Vectors[0].len		= 0x20;
	Vectors[1].data		= (Ticket == NULL) ? 0 : Ticket;
	Vectors[1].len		= (Ticket == NULL) ? 0 : 0x2a4;
	Vectors[2].data		= (Certificate == NULL) ? 0 : Certificate;
	Vectors[2].len		= (Certificate == NULL) ? 0 : Cert_Len;
	Vectors[3].data		= Out;
	Vectors[3].len		= 0x49e4;
	Vectors[4].data		= Output;
	Vectors[4].len		= 0x20;

	Lock();

	int Ret = IOS_Ioctlv(Device_Handle, Ioctl::DI_OpenPartition, 3, 2, Vectors);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_OpenPartition)";

	return ((Ret == 1) ? 0 : -Ret);
}

/*******************************************************************************
 * Close_Partition: Closes the opened partition
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Close_Partition() 
{
	Lock();

	Command[0] = Ioctl::DI_ClosePartition << 24;

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_ClosePartition, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_ClosePartition)";

	return ((Ret == 1)? 0 : -Ret);
}


/*******************************************************************************
 * Stop_Motor: Stops the drives motor.  Will require a reset to resume operation
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns result of Ioctl command
 *
 ******************************************************************************/

int DIP::Stop_Motor()
{
	Command[0] = Ioctl::DI_StopMotor << 24;
	Command[1] = 0;		// Set this to 1 to eject the disc
	Command[2] = 0;		// This will temporarily kill the drive if set!!!

	Lock();

	int Ret = IOS_Ioctl(Device_Handle, Ioctl::DI_StopMotor, Command, 0x20, Output, 0x20);

	Unlock();

	if (Ret == 2) throw "Ioctl error (DI_StopMotor)";

	return ((Ret == 1) ? 0 : -Ret);
}
