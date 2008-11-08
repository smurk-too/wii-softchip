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
#include <stdlib.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>

#include <wiiuse/wpad.h>

#include "Memory_Map.h"
#include "WiiDisc.h"
#include "Apploader.h"
#include "cIOS.h"

#include "SoftChip.h"

// static void Silent_Report(const char* Args, ...){}		// Blank apploader reporting function

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
	Standby_Flag				= false;
	Reset_Flag					= false;
	framebuffer					= 0;
	vmode						= 0;

	int			IOS_Version		= 249;
	bool		IOS_Loaded		= false;

	// Initialize subsystems
	VIDEO_Init();

	// Load proper cIOS version
	IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);

	// Initialize Video
	Set_VideoMode(false);

	// Initialize Input
	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	// Set callback functions
	SYS_SetPowerCallback(Standby);
	SYS_SetResetCallback(Reboot);

	// TODO: Replace this with graphical banner (PNGU)
	printf("\x1b[1;0H"); // Jump a line
	printf("Wii SoftChip v0.0.1-pre\n");
	printf("This software is distributed under the terms\n");
	printf("of the GNU General Public License (GPLv3)\n");
	printf("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n\n");

	// TODO: Make the IOS version configurable
	if (IOS_Loaded) printf("[+] IOS %d Loaded\n", IOS_Version);
	DI->Initialize();
}

/*******************************************************************************
 * ~SoftChip: Default destructor
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

SoftChip::~SoftChip(){}

/*******************************************************************************
 * Set_VideoMode: Sets the video mode based on the region of the disc
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Set_VideoMode(bool LoadingGame)
{
	// TODO: Some exception handling is needed here
	// The VideoMode is set in two phases, when starting SoftChip (LoadingGame == false)
	// and when booting the game (LoadingGame == true)

	if (!LoadingGame)
	{
		vmode = VIDEO_GetPreferredMode(0);
		framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

		// Set console parameters
		int x, y, w, h;
		x = 40;
		y = 40;

		w = vmode->fbWidth - (x * 2);
		h = vmode->xfbHeight - (y + 20);

		// Initialize the console - CON_InitEx was the problem with stev418
		CON_Init(framebuffer, x, y, w, h, vmode->fbWidth * VI_DISPLAY_PIX_SZ);
	}
	else if (CONF_GetProgressiveScan() > 0 && VIDEO_HaveComponentCable()) // 480p
	{
		vmode = &TVNtsc480Prog;
	}

	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

	if (!LoadingGame)
	{
		// Clear the garbage around the edges of the console
		VIDEO_ClearFrameBuffer(vmode, framebuffer, COLOR_BLACK);
	}
	else
	{
		// Set Video_Move based on system settings
		switch (VIDEO_GetCurrentTvMode())
		{
			case VI_NTSC:
				*(unsigned int*)Memory::Video_Mode = (unsigned int)Video::Modes::NTSC;
				break;

			case VI_PAL:
				*(unsigned int*)Memory::Video_Mode = (unsigned int)Video::Modes::PAL;
				break;

			case VI_MPAL:
				*(unsigned int*)Memory::Video_Mode = (unsigned int)Video::Modes::MPAL;
				break;

			case VI_EURGB60:
				*(unsigned int*)Memory::Video_Mode = (unsigned int)Video::Modes::PAL60;
				break;
		}
	}
}

/*******************************************************************************
 * Standby: Put the console in standby mode
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Standby()
{
	SoftChip::Instance()->Standby_Flag = true;
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
	SoftChip::Instance()->Reset_Flag = true;
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
	printf("Press the (A) button to continue.\n\n");
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
			Load_Disc();
		}

		if (Standby_Flag)
		{
			WPAD_Shutdown();
			STM_ShutdownToStandby();
		}

		else if (Reset_Flag)
		{
			STM_RebootSystem();
		}

		VIDEO_WaitVSync();
	}
	exit(0);
}

/*******************************************************************************
 * Load_Disc: Loads a Wii Disc
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/
void SoftChip::Load_Disc()
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

		memset(reinterpret_cast<void*>(Memory::Disc_ID), 0, 6);
		DI->Read_DiscID(reinterpret_cast<qword*>(Memory::Disc_ID));

		// Read header & process info
		DI->Read_Unencrypted(&Header, 0x800, 0);

		char Disc_ID[8];
		memset(Disc_ID, 0, 8);
		memcpy(Disc_ID, &Header.ID, sizeof(Header.ID));
		printf("Disc ID: %s\n",Disc_ID);
		printf("Magic Number: 0x%x\n", Header.Magic);
		printf("Disc Title: %s\n", Header.Title);

		// Read partition descriptor and get offset to partition info
		dword Offset = Wii_Disc::Offsets::Descriptor;
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
		if (DI->Open_Partition(Partition_Info.Offset, 0,0,0, Tmd_Buffer) < 0)
		{
			printf("[-] Error opening partition.\n\n");
			return;
		}

		printf("[+] Partition opened successfully.\n");

		// Read apploader header from 0x2440

		static Apploader::Header Loader __attribute__((aligned(0x20)));
		printf("Reading apploader header.\n");
		DI->Read(&Loader, sizeof(Apploader::Header), Wii_Disc::Offsets::Apploader);
		DCFlushRange(&Loader, 0x20);

		printf("Payload Information:\n");
		printf("\tRevision:\t%s\n", Loader.Revision);
		printf("\tEntry:\t0x%x\n", (int)Loader.Entry_Point);
		printf("\tSize:\t%d bytes\n", Loader.Size);
		printf("\tTrailer:\t%d bytes\n\n", Loader.Trailer_Size);

		printf("Reading payload.\n");

		// Read apploader.bin
		DI->Read((void*)Memory::Apploader,Loader.Size + Loader.Trailer_Size, Wii_Disc::Offsets::Apploader + 0x20);
		DCFlushRange((void*)(((int)&Loader) + 0x20),Loader.Size + Loader.Trailer_Size);

		// Set up loader function pointers
		Apploader::Start	Start	= Loader.Entry_Point;
		Apploader::Enter	Enter	= 0;
		Apploader::Load		Load	= 0;
		Apploader::Exit		Exit	= 0;

		// Grab function pointers from apploader
		printf("Retrieving function pointers from apploader.\n");
		Start(&Enter, &Load, &Exit);

		/*
		 * This is what's causing the apploader errors.
		 * We should be able to report, but it isn't working.
		 * Probably an alignment issue.
		 *

		// Set reporting callback
		printf("Setting reporting callback.\n");
		Apploader::Report Report = (Apploader::Report)printf;
		Enter(Report);

		*/

		// Read fst, bi2, and main.dol information from disc

		void*	Address = 0;
		int		Section_Size;
		int		Partition_Offset;

		printf("Loading.\t\t\n");
		while(Load(&Address, &Section_Size, &Partition_Offset))
		{
			printf(".");

			if (!Address) throw ("Null pointer from apploader");

			DI->Read(Address, Section_Size, Partition_Offset << 2);
			DCFlushRange(Address, Section_Size);
			// NOTE: This would be the ideal time to patch main.dol

		}

		// Patch in info missing from apploader reads

		*(dword*)Memory::Sys_Magic	= 0x0d15ea5e;
		*(dword*)Memory::Version	= 1;
		*(dword*)Memory::Arena_L	= 0x00000000;
		*(dword*)Memory::Bus_Speed	= 0x0E7BE2C0;
		*(dword*)Memory::CPU_Speed	= 0x2B73A840;

		// Retrieve application entry point
		void* Entry = Exit();

		printf("Launching Application!\n\n");

		// Set video mode for discs native region
		Set_VideoMode(true);

		// Flush application memory range
		DCFlushRange((void*)0x80000000,0x17fffff);	// TODO: Remove these hardcoded values

		// Cleanup loader information
		DI->Close();

		// Shutdown libogc services
		SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

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
	catch (const char* Message)
	{
		printf("Exception: %s\n", Message);
		return;
	}
}
