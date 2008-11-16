/*******************************************************************************
 * SoftChip.cpp
 *
 * Copyright (c) 2008 Requiem (requiem@century-os.com)
 * Copyright (c) 2008 luccax
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
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>

#include "Memory_Map.h"
#include "WiiDisc.h"
#include "Apploader.h"
#include "cIOS.h"
#include "Logger.h"
#include "Input.h"

#include "SoftChip.h"

extern "C" int sdio_Deinitialize(void);

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
	Lang_Selected				= -1;
	System_VMode				= true;

    IOS_Loaded = false;
    IOS_Version = Target_IOS;

    // Initialize subsystems
    VIDEO_Init();

    // Load proper cIOS version
    IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);
    if (!IOS_Loaded)
    {
        IOS_Version = 36;
        IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);
    }

	// Initialize logger
    Logger::Instance()->Initialize();
	Logger::Instance()->Write("/softchip.log",  "Starting SoftChip...\n");

    // Initialize Video
    vmode = VIDEO_GetPreferredMode(0);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

    VIDEO_Configure(vmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

	// Set console parameters
    int x, y, w, h;
    x = 20;
    y = 20;

    w = vmode->fbWidth - (x * 2);
    h = vmode->xfbHeight - (y + 20);

    // Initialize the console - CON_InitEx works after VIDEO_ calls
	CON_InitEx(vmode, x, y, w, h);

	// Clear the garbage around the edges of the console
    VIDEO_ClearFrameBuffer(vmode, framebuffer, COLOR_BLACK);

    // Initialize Input
	Input::Init();

    // Set callback functions
    SYS_SetPowerCallback(Standby);
    SYS_SetResetCallback(Reboot);

    Initialize();
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
 * Initialize: Sets up subsystems
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Initialize()
{
    // TODO: Replace this with graphical banner (PNGU)
    printf("Wii SoftChip v0.0.1-pre\n");
    printf("This software is distributed under the terms\n");
    printf("of the GNU General Public License (GPLv3)\n");
    printf("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n\n");

    // TODO: Make the IOS version configurable
    if (IOS_Loaded)
    {
        printf("[+] IOS %d Loaded\n", IOS_Version);
        if (IOS_Version != Target_IOS)
        {
            printf("Error: SoftChip requires a Custom IOS with dip-module\n");
            printf("installed as IOS %d. Exiting...", Target_IOS);
            sleep(5);
            exit(0);
        }
    }
    else
    {
        printf("Error loading IOS.\n");
        printf("SoftChip requires a Custom IOS with dip-module\n");
        printf("installed as IOS %d. Exiting...", Target_IOS);
        sleep(5);
        exit(0);
    }

    if (!DI->Initialize())
    {
        printf("Error: Failed to initialize dip-module. Exiting...");
        sleep (5);
        exit(0);
    }

    DI->Stop_Motor();
}

/*******************************************************************************
 * Set_VideoMode: Forces the video mode based on current system settings
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Set_VideoMode(char Region)
{
    // TODO: Some exception handling is needed here
    unsigned int Video_Mode;

	if (System_VMode)
	{
		switch (CONF_GetVideo()) {
			case CONF_VIDEO_NTSC:
				Video_Mode = (unsigned int)Video::Modes::NTSC;
				break;

			case CONF_VIDEO_PAL:
				Video_Mode = (unsigned int)Video::Modes::PAL;
				break;

			case CONF_VIDEO_MPAL:
				Video_Mode = (unsigned int)Video::Modes::MPAL;
				break;

			default:
				Video_Mode = (unsigned int)Video::Modes::NTSC;
		}
	}
	else
	{
		switch (Region) {
			case Wii_Disc::Regions::PAL_Default:
			case Wii_Disc::Regions::PAL_France:
			case Wii_Disc::Regions::PAL_Germany:
			case Wii_Disc::Regions::Euro_X:
			case Wii_Disc::Regions::Euro_Y:
				Video_Mode = (unsigned int)Video::Modes::PAL;
				break;

			case Wii_Disc::Regions::NTSC_USA:
			case Wii_Disc::Regions::NTSC_Japan:
			default:
				Video_Mode = (unsigned int)Video::Modes::NTSC;
				break;
		}
	}

    // Set 0x800000cc based on system settings
    // For some reason, setting this before VIDEO_ calls has better PAL compatibility.
    *(unsigned int*)Memory::Video_Mode = Video_Mode;

    VIDEO_Configure(vmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

/*******************************************************************************
 * Set_GameLanguage: Patches Game's Language
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if patched
 *
 ******************************************************************************/

bool SoftChip::Set_GameLanguage(void *Address, int Size)
{
	unsigned int PatchData[3]	= { 0x7C600775, 0x40820010, 0x38000000 };
	unsigned int *Addr			= (unsigned int*)Address;
	bool SearchTarget			= false;

	while (Size >= 16)
	{
		if (SearchTarget)
		{
			if (*Addr == 0x88610008)
			{
				*Addr = (unsigned int)(0x38600000 | Lang_Selected);
				return true;
			}
		}
		else if (Addr[0] == PatchData[0] && Addr[1] == PatchData[1] && Addr[2] == PatchData[2])
		{
			SearchTarget = true;
		}

		Addr += 1;
		Size -= 4;
	}

	return false;
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
	std::string Languages[]	= { "Japanese", "English", "German", "French", "Spanish", "Italian", "Dutch", "S. Chinese", "T. Chinese", "Korean" };
	Input *In				= Input::Instance();
	int Index				= 0;
	int Cols				= 0;
	int Rows				= 0;

	CON_GetMetrics(&Cols, &Rows);
	printf("Press the (A) button to continue.\n");
	printf("Use the Arrows to change settings.\n\n");
	printf("\x1b[s"); // Save Position

    while (true)
    {
        // Input
		In->Update();

        if (In->GetSimpleInput(HOME, START, HOME))
		{
            exit(0);
		}

        if (In->GetSimpleInput(A, A, A))
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

		if (In->GetSimpleInput(UP, UP, UP) || In->GetSimpleInput(DOWN, DOWN, DOWN))
		{
			Index = !Index;
		}

		else if (In->GetSimpleInput(RIGHT, RIGHT, RIGHT))
		{
			if (Index == 0 && ++Lang_Selected > 0x09) Lang_Selected = -1;
			if (Index == 1) System_VMode = !System_VMode;
		}

		else if (In->GetSimpleInput(LEFT, LEFT, LEFT))
		{
			if (Index == 0 && --Lang_Selected < -1) Lang_Selected = 0x09;
			if (Index == 1) System_VMode = !System_VMode;
		}

		// Print Settings
		printf("\x1b[u"); // Return
		for (Rows = 0; Rows < Cols * 2; Rows++) printf(" "); // Clear Lines

		printf("\x1b[u\x1b[%um", (Index == 0) ? 32 : 37); // Return and Set Color
		printf("Game's Language: %s\n", (Lang_Selected == -1) ? "System Default" : Languages[Lang_Selected].c_str());

		printf("\x1b[%um", (Index == 1) ? 32 : 37); // Set Color
		printf("Set Video Mode using: %s\n\n", (System_VMode) ? "System Settings" : "Disc Settings");
		printf("\x1b[37m"); // Reset Color

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
        printf("Loading Game...\n");

        DI->Wait_CoverClose();
        DI->Reset();

        memset(reinterpret_cast<void*>(Memory::Disc_ID), 0, 6);
        DI->Read_DiscID(reinterpret_cast<qword*>(Memory::Disc_ID));

        // Read header & process info
        DI->Read_Unencrypted(&Header, sizeof(Wii_Disc::Header), 0);

        char Disc_ID[8];
        memset(Disc_ID, 0, sizeof(Disc_ID));
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
        	// TODO: Fix hardcoded values
        	static byte Buffer[0x20] __attribute__((aligned(0x20)));
        	memset(Buffer, 0, 0x20);

            DI->Read_Unencrypted(Buffer, 0x20, Offset);

            memcpy(&Partition_Info, Buffer, sizeof(Wii_Disc::Partition_Info));

            if (Partition_Info.Type != 1)
                break;

            Offset += sizeof(Wii_Disc::Partition_Info);
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
		bool	Lang_Patched = (Lang_Selected == -1);

        printf("Loading.\t\t\n");
        while(Load(&Address, &Section_Size, &Partition_Offset))
        {
            printf(".");

            if (!Address) throw ("Null pointer from apploader");

            DI->Read(Address, Section_Size, Partition_Offset << 2);
            DCFlushRange(Address, Section_Size);
            // NOTE: This would be the ideal time to patch main.dol
			if (!Lang_Patched) Lang_Patched = Set_GameLanguage(Address, Section_Size);

        }

        // Patch in info missing from apploader reads

        *(dword*)Memory::Sys_Magic	= 0x0d15ea5e;
        *(dword*)Memory::Version	= 1;
        *(dword*)Memory::Arena_L	= 0x00000000;
        *(dword*)Memory::Bus_Speed	= 0x0E7BE2C0;
        *(dword*)Memory::CPU_Speed	= 0x2B73A840;

        // Enable online mode in games
        memcpy((dword*)Memory::Online_Check, (dword*)Memory::Disc_ID, 4);

        // Retrieve application entry point
        void* Entry = Exit();

        printf("Launching Application!\n\n");

        // Set video mode for discs native region
        Set_VideoMode(*(char*)Memory::Disc_Region);

        // Flush application memory range
        DCFlushRange((void*)0x80000000,0x17fffff);	// TODO: Remove these hardcoded values

		Logger::Instance()->Release();

        // Cleanup loader information
        DI->Close();

		// Shutdown sdio
		sdio_Deinitialize();

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
