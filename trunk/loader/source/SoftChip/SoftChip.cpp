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

#include "SoftChip.h"

extern "C" void settime(u64 time);

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
    Controls					= Input::Instance();
	Out							= Console::Instance();
	Cfg							= Configuration::Instance();
    Standby_Flag				= false;
    Reset_Flag					= false;
    framebuffer					= 0;
    vmode						= 0;

    // Initialize subsystems
    VIDEO_Init();

	// Initialize FAT
    Logger::Instance()->Initialize();

	// Initialize Folders
	Cfg->CreateFolder(ConfigData::DefaultFolder);

	// Initialize Configuration	
	Cfg->Read(ConfigData::DefaultFile);
	Cfg->Save(ConfigData::DefaultFile); // Create

	// Release FAT
	Logger::Instance()->Release();
	
	// Load proper cIOS version
    IOS_Version = Cfg->Data.IOS;
    IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);
    if (!IOS_Loaded)
    {
        IOS_Version = 36;
        IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);
    }

	// Initialize FAT
    Logger::Instance()->Initialize();

	// Initialize Default Logger
	Logger::Instance()->Write("/SoftChip/Default.log",  "Starting SoftChip...\r\n");

	// Set Autoboot
	Out->SetSilent(Cfg->Data.AutoBoot);

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
	Controls->Initialize();

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
    Out->Print("Wii SoftChip v0.0.1-pre\n");
    Out->Print("This software is distributed under the terms\n");
    Out->Print("of the GNU General Public License (GPLv3)\n");
    Out->Print("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n\n");

    // TODO: Make the IOS version configurable
    if (IOS_Loaded)
    {
        Out->Print("[+] IOS %d Loaded\n", IOS_Version);
        if (IOS_Version != Cfg->Data.IOS)
        {
			Out->SetSilent(false);
            Out->Print("Error: SoftChip requires a Custom IOS with dip-module\n");
            Out->Print("installed as IOS %d.\n\n", Cfg->Data.IOS);
            SelectIOS();
        }
    }
    else
    {
		Out->SetSilent(false);
        Out->Print("[-] Error loading IOS.\n");
        Out->Print("SoftChip requires a Custom IOS with dip-module\n");
        Out->Print("installed as IOS %d.\n\n", Cfg->Data.IOS);
        SelectIOS();
    }

    if (!DI->Initialize())
    {
		Out->SetSilent(false);
        Out->Print("[-] Error initializing dip-module.\n\n");
        SelectIOS();
    }

    DI->Stop_Motor();
}

/*******************************************************************************
 * SelectIOS: Select the IOS
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::SelectIOS()
{
    Out->SetSilent(false);
	Out->Print("SoftChip will try to load the selected IOS on startup.\n");
	Out->Print("Press the (A) Button to save and reboot.\n\n");

	Out->StartMenu();
	Console::Option *oIOS = Out->CreateOption("Use IOS: ", 0, 256, Cfg->Data.IOS);

	while (true)
	{
		// Update Input
		Controls->Scan();

		// Faster Selecting
		if (Controls->Up.Active) oIOS->Index += 20;
		if (Controls->Down.Active) oIOS->Index -= 20;

		if (Controls->Accept.Active)
		{
			Out->Print("Saving and Returning...\n");
			Cfg->Save(ConfigData::DefaultFile);
            exit(0);
		}

		if (Standby_Flag)
        {
            Controls->Terminate();
            STM_ShutdownToStandby();
        }

        else if (Reset_Flag)
        {
            STM_RebootSystem();
        }

		// Update Menu
		Out->UpdateMenu(Controls);
		Cfg->Data.IOS = oIOS->Index;

        VIDEO_WaitVSync();
    }
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

	if (Cfg->Data.SysVMode)
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
				if (vmode != &TVNtsc480Prog) vmode = &TVNtsc480IntDf; // Force NTSC Display (576i fix)
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
				*Addr = (unsigned int)(0x38600000 | Cfg->Data.Language);
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
	if (Cfg->Data.AutoBoot)
	{
		// Wait 2 seconds for Button Press
		if (!Controls->Wait_ButtonPress(&Controls->Menu, 2))
		{
			Load_Disc();
		}

		// Cancel Autoboot
		Out->SetSilent(false);
	}

	std::string Languages[]	= { "System Default", "Japanese", "English", "German", "French", "Spanish", "Italian", "Dutch", "S. Chinese", "T. Chinese", "Korean" };
	std::string VModes[] = { "System Settings", "Disc Region" };
	std::string BoolOption[] = { "Disabled", "Enabled" };

	Out->Print("Press the (A) button to continue.\n");
	Out->Print("Press the (+) button to select IOS.\n");
	Out->Print("Use the D-Pad to change settings.\n\n");

	Out->StartMenu();
	Console::Option *oLang = Out->CreateOption("Game's Language: ", Languages, 11, Cfg->Data.Language + 1);
	Console::Option *oMode = Out->CreateOption("Set Video Mode using: ", VModes, 2, !Cfg->Data.SysVMode);
	Console::Option *oBoot = Out->CreateOption("Autoboot: ", BoolOption, 2, Cfg->Data.AutoBoot);

	settime(secs_to_ticks(time(NULL) - 946684800));

    while (true)
    {
        // Input
		Controls->Scan();

        if (Controls->Exit.Active)
		{
			Out->Print("Returning...\n");
			Cfg->Save(ConfigData::DefaultFile);
            exit(0);
		}

        if (Controls->Accept.Active)
        {
			Cfg->Save(ConfigData::DefaultFile);
            Load_Disc();
			Out->ReStartMenu();
        }

		if (Controls->Plus.Active)
		{
			SelectIOS();
		}

        if (Standby_Flag)
        {
            Controls->Terminate();
            STM_ShutdownToStandby();
        }

        else if (Reset_Flag)
        {
            STM_RebootSystem();
        }

		// Update Menu
		Out->UpdateMenu(Controls);
		Cfg->Data.Language = oLang->Index - 1;
		Cfg->Data.SysVMode = !oMode->Index;
		Cfg->Data.AutoBoot = oBoot->Index;

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
        Out->Print("Loading Game...\n");

        DI->Wait_CoverClose();
        DI->Reset();

        memset(reinterpret_cast<void*>(Memory::Disc_ID), 0, 6);
        DI->Read_DiscID(reinterpret_cast<qword*>(Memory::Disc_ID));

        // Read header & process info
        DI->Read_Unencrypted(&Header, sizeof(Wii_Disc::Header), 0);

        char Disc_ID[8];
        memset(Disc_ID, 0, sizeof(Disc_ID));
        memcpy(Disc_ID, &Header.ID, sizeof(Header.ID));
        Out->Print("Disc ID: %s\n",Disc_ID);
        Out->Print("Magic Number: 0x%x\n", Header.Magic);
        Out->Print("Disc Title: %s\n", Header.Title);

        // Read partition descriptor and get offset to partition info
        dword Offset = Wii_Disc::Offsets::Descriptor;
        DI->Read_Unencrypted(&Descriptor, sizeof(Wii_Disc::Partition_Descriptor), Offset);

        Offset = Descriptor.Primary_Offset << 2;
        Out->Print("Partition Info is at: 0x%x\n", Offset);

        // TODO: Support for additional partition types (secondary, tertiary, quaternary)
		// TODO: Fix hardcoded values, ugly handling of 4+ partitions
		dword BufferLen = 0x20;
		if (Descriptor.Primary_Count > 4) BufferLen += (Descriptor.Primary_Count - 4) * 8;
		byte *PartBuffer = (byte*)memalign(0x20, BufferLen);

		memset(PartBuffer, 0, BufferLen);
		DI->Read_Unencrypted(PartBuffer, BufferLen, Offset);

		Wii_Disc::Partition_Info *Partitions = (Wii_Disc::Partition_Info*)PartBuffer;

        for (dword i = 0; i < Descriptor.Primary_Count; i++)
        {
			if (Partitions[i].Type == 0)
			{
				memcpy(&Partition_Info, &Partitions[i], sizeof(Wii_Disc::Partition_Info));
				break;
			}
        }

		Offset = Partition_Info.Offset << 2;
		free(PartBuffer);

        if (!Offset)
		{
			Out->Print("[-] No boot partition found.\n\n");
            return;
		}

        Out->Print("Partition is located at: 0x%x\n", Offset);

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

        Out->Print("System certificates at: 0x%x\n", reinterpret_cast<dword>(Certs));

        // Read the buffer
        DI->Read_Unencrypted(Buffer, 0x800, Offset);

        // Get the ticket pointer
        Ticket		= reinterpret_cast<signed_blob*>(Buffer);
        T_Length	= SIGNED_TIK_SIZE(Ticket);

        Out->Print("Ticket at: 0x%x\n", reinterpret_cast<dword>(Ticket));

        // Get the TMD pointer

        Tmd = reinterpret_cast<signed_blob*>(Buffer + 0x2c0);
        MD_Length = SIGNED_TMD_SIZE(Tmd);

        Out->Print("Tmd at: 0x%x\n", reinterpret_cast<dword>(Tmd));

        static byte	Tmd_Buffer[0x49e4] __attribute__((aligned(0x20)));

        // Open Partition
        if (DI->Open_Partition(Partition_Info.Offset, 0,0,0, Tmd_Buffer) < 0)
        {
            Out->Print("[-] Error opening partition.\n\n");
            return;
        }

        Out->Print("[+] Partition opened successfully.\n");

        // Read apploader header from 0x2440

        static Apploader::Header Loader __attribute__((aligned(0x20)));
        Out->Print("Reading apploader header.\n");
        DI->Read(&Loader, sizeof(Apploader::Header), Wii_Disc::Offsets::Apploader);
        DCFlushRange(&Loader, 0x20);

        Out->Print("Payload Information:\n");
        Out->Print("\tRevision:\t%s\n", Loader.Revision);
        Out->Print("\tEntry:\t0x%x\n", (int)Loader.Entry_Point);
        Out->Print("\tSize:\t%d bytes\n", Loader.Size);
        Out->Print("\tTrailer:\t%d bytes\n\n", Loader.Trailer_Size);

        Out->Print("Reading payload.\n");

        // Read apploader.bin
        DI->Read((void*)Memory::Apploader,Loader.Size + Loader.Trailer_Size, Wii_Disc::Offsets::Apploader + 0x20);
        DCFlushRange((void*)(((int)&Loader) + 0x20),Loader.Size + Loader.Trailer_Size);

        // Set up loader function pointers
        Apploader::Start	Start	= Loader.Entry_Point;
        Apploader::Enter	Enter	= 0;
        Apploader::Load		Load	= 0;
        Apploader::Exit		Exit	= 0;

        // Grab function pointers from apploader
        Out->Print("Retrieving function pointers from apploader.\n");
        Start(&Enter, &Load, &Exit);

        /*
         * This is what's causing the apploader errors.
         * We should be able to report, but it isn't working.
         * Probably an alignment issue.
         *

        // Set reporting callback
        Out->Print("Setting reporting callback.\n");
        Apploader::Report Report = (Apploader::Report)printf;
        Enter(Report);

        */

        // Read fst, bi2, and main.dol information from disc

        void*	Address = 0;
        int		Section_Size;
        int		Partition_Offset;
		bool	Lang_Patched = (Cfg->Data.Language == -1);

        Out->Print("Loading.\t\t\n");
        while(Load(&Address, &Section_Size, &Partition_Offset))
        {
            Out->Print(".");

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

        Out->Print("Launching Application!\n\n");

        // Set video mode for discs native region
        Set_VideoMode(*(char*)Memory::Disc_Region);

        // Flush application memory range
        DCFlushRange((void*)0x80000000,0x17fffff);	// TODO: Remove these hardcoded values

		Logger::Instance()->Release();

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
        Out->Print("Exception: %s\n\n", Message);
        return;
    }
}
