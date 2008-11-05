/*******************************************************************************
 * SoftChip.cpp
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

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>

#include <wiiuse/wpad.h>

#include "Memory_Map.h"
#include "WiiDisc.h"
#include "cIOS.h"

#include "SoftChip.h"

namespace Apploader
{
	const dword Header	= 0x2440;		// Offset into the partition to apploader header
	const dword Payload	= 0x2460;		// Offset into the partition to apploader payload

	typedef void 	(*Report)	(const char*, ...);
	typedef void 	(*Enter)	(Report);
	typedef int 	(*Load)		(void** Dest, int* Size, int* Offset);
	typedef void* 	(*Exit)		();
	typedef void 	(*Start)	(Enter*,Load*,Exit*);
}

// static void Report(const char* Args, ...){}	// Commented while we use printf for this
extern "C" void __exception_closeall();

//--------------------------------------
// SoftChip Class

/*******************************************************************************
 * SoftChip: Default constructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

SoftChip::SoftChip()
{
	DI							= DIP::Instance();

	void		*framebuffer	= 0;
	GXRModeObj	*vmode			= 0;
	int			IOS_Version		= 249;
	bool		IOS_Loaded		= false;

	// Initialize subsystems
	VIDEO_Init();

	// Load proper cIOS version
	IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);

	// Initialize Video
	vmode		= VIDEO_GetPreferredMode(0);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

	// Set console paramaters
	int x, y, w, h;
	x = 40;
	y = 40;

	w = vmode->fbWidth		- (x * 2);		// Center the console
	h = vmode->xfbHeight	- (y + 20);		// Offset the bottom

	CON_InitEx(vmode, x, y, w, h);			// Initialize the console

	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

	// Clear the garbage around the edges of the console
	VIDEO_ClearFrameBuffer(vmode, framebuffer, COLOR_BLACK);

	// Initialize Input
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	// Set callback functions
	SYS_SetPowerCallback(Standby);
	SYS_SetResetCallback(Reboot);

	// TODO: Replace this with graphical banner (PNGU)

	printf("Wii SoftChip v0.0.1-pre\n");
	printf("This software is distributed under the terms\n");
	printf("of the GNU General Public License (GPLv3)\n");
	printf("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n\n");

	// TODO: Make the IOS version configurable
	if (IOS_Loaded) printf("[+] IOS %d Loaded\n", IOS_Version);
	DI->Initialize();
}

SoftChip::~SoftChip(){}

/*******************************************************************************
 * Standby: Put the console in standby mode
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Standby()
{
	STM_ShutdownToStandby();
}

/*******************************************************************************
 * Reboot: Return to the System Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Reboot()
{
	STM_RebootSystem();
}


/*******************************************************************************
 * Run: SoftChip entry
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Run()
{
	while (true)
	{
		// TODO: Wrap this into an input class
		WPAD_ScanPads();
		PAD_ScanPads();

		unsigned int Buttons	= WPAD_ButtonsDown(0);
		unsigned int GC_Buttons	= PAD_ButtonsDown(0);

		if ((Buttons & WPAD_BUTTON_HOME) || (GC_Buttons & PAD_BUTTON_START))
			exit(0);

		if ((Buttons & WPAD_BUTTON_A) || (GC_Buttons & PAD_BUTTON_A))
		{
			Load();
		}

		VIDEO_WaitVSync();
	}
	exit(0);
}

/*******************************************************************************
 * Load: Loads a Wii Disc
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/
void SoftChip::Load()
{
	static Wii_Disc::Header					Header			__attribute__((aligned(0x20)));
	static Wii_Disc::Partition_Descriptor	Descriptor		__attribute__((aligned(0x20)));
	static Wii_Disc::Partition_Info			Partition_Info	__attribute__((aligned(0x20)));

	memset(&Header, 0, sizeof(Wii_Disc::Header));
	memset(&Descriptor, 0, sizeof(Wii_Disc::Partition_Descriptor));
	memset(&Partition_Info, 0, sizeof(Wii_Disc::Partition_Info));

	try
	{
		DI->Wait_CoverClose();
		DI->Reset();

		memset((char*)0x80000000, 0, 6);
		DI->Read_DiscID((unsigned long long*)0x80000000);

		// Read header & process info
		DI->Read_Unencrypted(&Header, 0x800, 0);

		char Disc_ID[8];
		memset(Disc_ID, 0, 8);
		memcpy(Disc_ID, &Header.ID, sizeof(Header.ID));
		printf("Disc ID: %s\n",Disc_ID);
		printf("Magic Number: 0x%x\n", Header.Magic);
		printf("Disc Title: %s\n", Header.Title);

		// Read partition descriptor and get offset to partition info
		dword Offset = 0x00040000;
		DI->Read_Unencrypted(&Descriptor, sizeof(Wii_Disc::Partition_Descriptor), Offset);

		Offset = Descriptor.Primary_Offset << 2;
		printf("Partition Info is at: 0x%x\n", Offset);

		// TODO: Support for additional partition types (secondary, tertiary, quaternary)

		for (unsigned long i = 0; i < Descriptor.Primary_Count; i++)
		{
			Offset += (i * sizeof(Wii_Disc::Partition_Info));
			DI->Read_Unencrypted(&Partition_Info, sizeof(Wii_Disc::Partition_Info),Offset);

			if (Partition_Info.Type == 0)
				break;
		}

		Offset = Partition_Info.Offset << 2;
		printf("Partition is located at: 0x%x\n", Offset);

		// Set Offset Base to start of partition
		DI->Set_OffsetBase(Offset);
		Offset = 0;

		// Get Certificates, Ticket, and Ticket Metadata

		static byte Buffer[0x800] __attribute__((aligned(0x20)));

		signed_blob* Certs		= 0;
		signed_blob* Ticket		= 0;
		signed_blob* Tmd		= 0;

		unsigned int C_Length	= 0;
		unsigned int T_Length	= 0;
		unsigned int MD_Length	= 0;

		// Get certificates from the cIOS
		cIOS::Instance()->GetCerts(&Certs, &C_Length);

		printf("System certificates at: 0x%x\n", reinterpret_cast<dword>(Certs));

		// Read the buffer
		DI->Read_Unencrypted(Buffer, 0x800, Offset);

		// Get the ticket pointer
		Ticket		= reinterpret_cast<signed_blob*>(Buffer);
		T_Length	= SIGNED_TIK_SIZE(Ticket);

		printf("Ticket at: 0x%x\n", reinterpret_cast<dword>(Ticket));

		// Get the TMD pointer

		Tmd = reinterpret_cast<signed_blob*>(Buffer + 0x2c0);
		MD_Length = SIGNED_TMD_SIZE(Tmd);

		printf("Tmd at: 0x%x\n", reinterpret_cast<dword>(Tmd));

		static byte	Tmd_Buffer[0x49e4] __attribute__((aligned(0x20)));

		// Open Partition
		int ret = DI->Open_Partition(Partition_Info.Offset, 0,0,0, Tmd_Buffer);
		if (ret < 0)
		{
			printf("[-] Error opening partition.\n\n");
			return;
		}

		printf("[+] Partition opened successfully.\n");

		// Read apploader.bin header from 0x2440
		byte* Header = reinterpret_cast<byte*>(Memory::Apploader_Head);

		printf("Reading apploader... ");
		DI->Read(Header, 0x20, Apploader::Header);
		DCFlushRange(Header, 0x20);

		// Read apploader.bin
		unsigned int Payload_Length = ((*(unsigned int*)(Header + 0x14) + 0x1f) & ~0x1f);
		printf("Payload size: 0x%x bytes.\n",Payload_Length);

		printf("Reading payload.\n");
		byte* Payload = reinterpret_cast<byte*>(Memory::Apploader);
		DI->Read(Payload, Payload_Length, Apploader::Payload);
		DCFlushRange(Payload, Payload_Length);

		// Get entry address from memory

		Apploader::Enter	Enter;
		Apploader::Load		Load;
		Apploader::Exit		Exit;

		Apploader::Start Start = (*(Apploader::Start*)((int)Header + 0x10));
		Start(&Enter, &Load, &Exit);
		printf("Apploader pointers set.\n");

		// Run Enter to set reporting callback
		Apploader::Report Print = (Apploader::Report)&printf;
		Enter(Print);
		printf("Report callback set.  Loading system files... \n\n");

		void* Dest = 0;
		int Size = 0;
		int Loader_Offset = 0;

		// Read information from disc
		while (Load(&Dest, &Size, &Loader_Offset))
		{
			if (!Dest) throw ("Null pointer from apploader.");
			if (DI->Read(Dest, Size, Loader_Offset << 2) < 0) throw ("Disc read error");
			DCFlushRange(Dest, Size);

			// This would be a good time to patch video / region / language
		}

		printf("done.\n");

		// TODO: Patch in missing info from reads

		// Retrieve entry point
		dword Entry = (dword)Exit();

		// Cleanup loader information
		DI->Close();

		if (false)	// This code is disabled until we fix the apploader issue.
		{
			// TODO: Evaluate this call, as per waninkoko's advice
			// SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

			WPAD_Shutdown();
			IRQ_Disable();
			__IOS_ShutdownSubsystems();
			__exception_closeall();

			// Branch to Application entry point

			__asm__ __volatile__
			(
					"mtlr %0;"			// Move the entry point into link register
					"blr"				// Branch to address in link register
					:					// No output registers
					:	"r" (Entry)		// Input register
					:
			);
		}
	}
	catch (const char* Message)
	{
		printf("Exception: %s", Message);
		return;
	}
}
