/*******************************************************************************
 * SoftChip.cpp
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
 * Copyright (c) 2009 luccax
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

#include <ogc/lwp_watchdog.h>

#include "Memory_Map.h"
#include "WiiDisc.h"
#include "Apploader.h"
#include "cIOS.h"

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
	// Classes
    DI						= DIP::Instance();
    Controls				= Input::Instance();
	Out						= Console::Instance();
	Cfg						= Configuration::Instance();
	Log						= Logger::Instance();
	SD						= Storage::Instance();

	// Flags
    Standby_Flag			= false;
    Reset_Flag				= false;
	Skip_AutoBoot			= false;
	Log->ShowTime			= true;

	// Video
    framebuffer				= 0;
    vmode					= 0;
	Video_Mode				= 0;

    // Initialize subsystems
    VIDEO_Init();

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
    int x = 20, y = 20, w, h;
    w = vmode->fbWidth - (x * 2);
    h = vmode->xfbHeight - (y + 20);

    // Initialize the console - CON_InitEx works after VIDEO_ calls
	CON_InitEx(vmode, x, y, w, h);

	// Clear the garbage around the edges of the console
    VIDEO_ClearFrameBuffer(vmode, framebuffer, COLOR_BLACK);

    // Set callback functions
    SYS_SetPowerCallback(Standby);
    SYS_SetResetCallback(Reboot);

	// Start Silent
	Out->SetSilent(true);

	// Banner (TODO: Change to an image)
	Out->Print("Wii SoftChip v0.0.1-pre\n");
    Out->Print("This software is distributed under the terms\n");
    Out->Print("of the GNU General Public License (GPLv3)\n");
    Out->Print("See http://www.gnu.org/licenses/gpl-3.0.txt for more info.\n\n");

	// IOS Notice
	Out->PrintErr("SoftChip uses by default IOS %d, which doesn't load backups!\n", Default_IOS);
	Out->PrintErr("Don't forget to change it before playing them.\n\n");

	// Initialize FAT
	SD->Initialize_FAT();

	// Verify SoftChip Folder
	SD->MakeDir(ConfigData::SoftChip_Folder);

	// Initialize Configuration	
	Out->Print("Reading Configuration File...\n");
	if (!Cfg->Read(ConfigData::Default_ConfigFile))
	{
		Out->Print("Using Defaults.\n\n");
		Cfg->Save(ConfigData::Default_ConfigFile);
	}
	else
	{
		Out->Print("Configuration Loaded.\n\n");
	}

	// Save IOS Position
	Cursor_IOS = Out->Save_Cursor();

	// Load IOS
	NextPhase = Phase_IOS;
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
 * Run: Heart of SoftChip's Logic
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Run()
{
	while (true)
	{
		if (NextPhase == Phase_IOS)
		{
			// Load IOS
			Load_IOS();
		}

		if (NextPhase == Phase_Menu)
		{
			// Handle Autoboot
			if (Skip_AutoBoot || !Cfg->Data.AutoBoot || Reset_Flag || Controls->Wait_ButtonPress(&Controls->Menu, 2))
			{
				Show_Menu();
			}
			else
			{
				NextPhase = Phase_Play;
			}

			// AutoBoot Done
			Skip_AutoBoot = true;
			Reset_Flag = false;
		}

		if (NextPhase == Phase_SelectIOS)
		{
			// Select another IOS
			Show_IOSMenu();
		}

		if (NextPhase == Phase_Play)
		{
			// Handle Silent
			Out->SetSilent(Cfg->Data.Silent);

			// Initialize Default Logger
			Log->OpenLog(ConfigData::Default_LogFile);
			Log->Write("---------------------\r\n");
			Log->Write("Loading Disc...\r\n");

			// Run Game
			Load_Disc();
		}
	}

	Exit_Loader();
}

/*******************************************************************************
 * Load_IOS: Load the IOS
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Load_IOS()
{
	// Release FAT and Wiimotes
	SD->Release_FAT();
	Controls->Terminate();

	// Restore Console Position
	Out->Restore_Cursor(Cursor_IOS);

	try
	{
		// Close it or the game will hang after the second Load_IOS()
		DI->Close();

		IOS_Version = Cfg->Data.IOS;
		IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);

		if (!IOS_Loaded)
		{
			Out->PrintErr("Error Loading IOS %d!\n", Cfg->Data.IOS);
			Out->PrintErr("Trying Default IOS %d...\n", Default_IOS);

			IOS_Version = Default_IOS;
			IOS_Loaded = !(IOS_ReloadIOS(IOS_Version) < 0);
		}

		if (IOS_Loaded)
		{
			Out->SetColor(Color_Green, true);
			Out->Print("[+] IOS %d (Rev %d) Loaded\n", IOS_Version, IOS_GetRevision());
		}
		else
		{
			Out->PrintErr("[-] Error Loading Default IOS\n");
			throw "Error Loading IOS";
		}

		if (!DI->Initialize())
		{
			Out->PrintErr("[-] Error Initializing DIP-Module.\n");
			throw "Error Initializing DIP";
		}

		// Stop Motor
		DI->Stop_Motor();

		// Continue
		NextPhase = Phase_Menu;
	}
	catch (const char* Message)
    {
		// Re-Select IOS
		NextPhase = Phase_SelectIOS;
	}

	// Re-Init FAT and Wiimotes
	SD->Initialize_FAT();
	Controls->Initialize();

	// Verify FAT
	if (!SD->Verify_FAT())
	{
		Out->SetColor(Color_Yellow, false);
		Out->Print("[Warning] No Storage Device was found. Configuration won't be saved.\n");
	}

	// Reset Color and Save Menu Position
	Out->Print("\n");
	Out->SetColor(Color_White, false);
	Cursor_Menu = Out->Save_Cursor();
}

/*******************************************************************************
 * Show_Menu: SoftChip Main Menu
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Show_Menu()
{
	std::string Languages[]	= { "System Default", "Japanese", "English", "German", "French", "Spanish", "Italian", "Dutch", "S. Chinese", "T. Chinese", "Korean" };
	std::string VModes[] = { "System Settings", "Disc Region" };
	std::string BoolOption[] = { "Disabled", "Enabled" };

	// Restore Menu Position
	Out->Restore_Cursor(Cursor_Menu);
	Out->SetSilent(false);

	Out->SetColor(Color_Yellow, false);
	Out->Print("--- Main Menu ---\n");
	Out->Print("> The changes you make here are saved to a Default Configuration File\n");
	Out->Print("Press the (A) button to continue.\n");
	Out->Print("Press the (+) button to select IOS.\n");
	Out->Print("Use the D-Pad to change settings.\n\n");
	Out->SetColor(Color_White, false);

	Out->CreateMenu();
	Console::Option *oLang = Out->CreateOption("Game's Language: ", Languages, 11, Cfg->Data.Language + 1);
	Console::Option *oMode = Out->CreateOption("Set Video Mode using: ", VModes, 2, !Cfg->Data.SysVMode);
	Console::Option *oBoot = Out->CreateOption("Autoboot: ", BoolOption, 2, Cfg->Data.AutoBoot);
	Console::Option *oSlnt = Out->CreateOption("Silent: ", BoolOption, 2, Cfg->Data.Silent);
	Console::Option *oLogg = Out->CreateOption("Logging: ", BoolOption, 2, Cfg->Data.Logging);
	Console::Option *o002  = Out->CreateOption("Remove 002 Protection: ", BoolOption, 2, Cfg->Data.Remove_002);

    while (true)
    {
        // Input
		Controls->Scan();

		// Return to HBC
        if (Controls->Exit.Active)
		{
			Out->Print("Returning...\n");
			Cfg->Save(ConfigData::Default_ConfigFile);
            Exit_Loader();
		}

		// Run Game
        if (Controls->Accept.Active)
        {
			Cfg->Save(ConfigData::Default_ConfigFile);
            NextPhase = Phase_Play;
			return;
        }

		// Select IOS
		if (Controls->Plus.Active)
		{
			NextPhase = Phase_SelectIOS;
			return;
		}

		// Update Menu
		Out->UpdateMenu(Controls);
		Cfg->Data.Language = oLang->Index - 1;
		Cfg->Data.SysVMode = !oMode->Index;
		Cfg->Data.AutoBoot = oBoot->Index;
		Cfg->Data.Silent = oSlnt->Index;
		Cfg->Data.Logging = oLogg->Index;
		Cfg->Data.Remove_002 = o002->Index;
		
		VerifyFlags();
        VIDEO_WaitVSync();
    }
}

/*******************************************************************************
 * Show_IOSMenu: Show Menu for changing IOS
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Show_IOSMenu()
{
	dword i, Count = 0;
	dword Cfg_IOS = 0;

	// Get a list of Valid IOSes
	cIOS *IOS = cIOS::Instance();
	IOS->List_SysTitles();

	// Declare Lists
	string List[IOS->SysTitles.size()];
	char nList[IOS->SysTitles.size()];

	// Fill the List
	for (i = 0; i < IOS->SysTitles.size(); i++)
	{
		switch (IOS->SysTitles[i])
		{
			case 1:		// BOOT2
			case 2:		// System Menu
			case 0x100:	// BC
			case 0x101:	// MIOS
				break;

			default:	// Valid IOS
				char Buffer[32];
				sprintf(Buffer, "IOS%u", IOS->SysTitles[i]);

				if (IOS->SysTitles[i] == Cfg->Data.IOS) Cfg_IOS = Count;
				nList[Count] = IOS->SysTitles[i];
				List[Count++] = string(Buffer);
		}
	}

	// Restore Menu Position
	Out->Restore_Cursor(Cursor_Menu);
	Out->SetSilent(false);
	
	Out->SetColor(Color_Yellow, false);
	Out->Print("--- IOS Menu ---\n");
	Out->Print("> SoftChip will try to load the selected IOS on startup.\n");
	Out->Print("> If a problem occurs, it'll load the Default IOS %d.\n", Default_IOS);
	Out->Print("Use the D-Pad to change IOS number.\n");
	Out->Print("Press the (Up) and (Down) Buttons to go faster.\n");
	Out->Print("Press the (A) Button to change IOS.\n\n");
	Out->SetColor(Color_White, false);

	Out->CreateMenu();
	Console::Option *oIOS = Out->CreateOption("SoftChip will use: ", List, Count, Cfg_IOS);

	while (true)
	{
		// Update Input
		Controls->Scan();

		// Load IOS
		if (Controls->Accept.Active)
		{
			Cfg->Save(ConfigData::Default_ConfigFile);
            NextPhase = Phase_IOS;
			return;
		}

		// Update Menu
		Out->UpdateMenu(Controls);
		Cfg->Data.IOS = nList[oIOS->Index];

		VerifyFlags();
        VIDEO_WaitVSync();
    }
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

	// Set Clock
	settime(secs_to_ticks(time(NULL) - 946684800));

    try
    {
		bool Disc_Inserted = false;
		if (DI->Verify_Cover(&Disc_Inserted) < 0)
		{
			throw "Verify_Cover failed";
		} else
		{
			if (!Disc_Inserted)
			{
				Out->SetSilent(false);
				Out->Print("Please insert a Disc.\n");
				DI->Wait_CoverClose();
			}
		}

		Out->Print("Loading Game...\n");
        DI->Reset();

        memset(reinterpret_cast<void*>(Memory::Disc_ID), 0, 6);
        DI->Read_DiscID(reinterpret_cast<qword*>(Memory::Disc_ID));

		if ( *(dword*)(Memory::Disc_ID) == 0x10001 || *(dword*)(Memory::Disc_ID) == 0x10000)
		{
			Out->PrintErr("Decrypted discs are not supported.\n\n");
            throw "Disc is decrypted";
		}
		
		//Determine the video mode to use
        Determine_VideoMode(*(char*)Memory::Disc_Region);
		
		// Read header & process info
        DI->Read_Unencrypted(&Header, sizeof(Wii_Disc::Header), 0);

        char Disc_ID[8];
        memset(Disc_ID, 0, sizeof(Disc_ID));
        memcpy(Disc_ID, &Header.ID, sizeof(Header.ID));
        Out->Print("Disc ID: %s\n", Disc_ID);
        Out->Print("Magic Number: 0x%x\n", Header.Magic);
        Out->Print("Disc Title: %s\n", Header.Title);
		
		// Log
		Log->Write("Disc ID: %s\r\n", Disc_ID);
		Log->Write("Disc Title: %s\r\n", Header.Title);

        // Read partition descriptor and get offset to partition info
        dword Offset = Wii_Disc::Offsets::Descriptor;
        DI->Read_Unencrypted(&Descriptor, sizeof(Wii_Disc::Partition_Descriptor), Offset);

        Offset = Descriptor.Primary_Offset << 2;
        Out->Print("Partition Info is at: 0x%x\n", Offset);

        // TODO: Support for additional partition types (secondary, tertiary, quaternary)
		// TODO: Support for selecting which partition load
		dword PartSize = sizeof(Wii_Disc::Partition_Info);
		dword BufferLen = Descriptor.Primary_Count * PartSize;

		// Length must be multiple of 0x20
		BufferLen += 0x20 - (BufferLen % 0x20);
		byte *PartBuffer = (byte*)memalign(0x20, BufferLen);

		memset(PartBuffer, 0, BufferLen);
		DI->Read_Unencrypted(PartBuffer, BufferLen, Offset);

		Wii_Disc::Partition_Info *Partitions = (Wii_Disc::Partition_Info*)PartBuffer;

        for (dword i = 0; i < Descriptor.Primary_Count; i++)
        {
			Out->Print("Found Partition (Type %u): 0x%x\n", Partitions[i].Type, Partitions[i].Offset << 2);

			if (Partitions[i].Type == 0)
			{
				memcpy(&Partition_Info, PartBuffer + (i * PartSize), PartSize);
				break;
			}
        }

		Offset = Partition_Info.Offset << 2;
		free(PartBuffer);

        if (!Offset)
		{
			Out->PrintErr("[-] No boot partition found.\n\n");
            throw "Wrong Offset";
		}

        Out->Print("Boot Partition is located at: 0x%x\n", Offset);

        // Set Offset Base to start of partition
        DI->Set_OffsetBase(Offset);
        Offset = 0;

        signed_blob* Certs		= 0;
        signed_blob* Ticket		= 0;
        signed_blob* Tmd		= 0;

        unsigned int C_Length	= 0;
        unsigned int T_Length	= 0;
        unsigned int MD_Length	= 0;

        static byte	Ticket_Buffer[0x800] __attribute__((aligned(0x20)));
        static byte	Tmd_Buffer[0x49e4] __attribute__((aligned(0x20)));

        // Get certificates from the cIOS
        cIOS::Instance()->GetCerts(&Certs, &C_Length);

        Out->Print("System certificates at: 0x%x\n", reinterpret_cast<dword>(Certs));

        // Read the ticket buffer
        DI->Read_Unencrypted(Ticket_Buffer, 0x800, Partition_Info.Offset << 2);

        // Get the ticket pointer
        Ticket		= reinterpret_cast<signed_blob*>(Ticket_Buffer);
        T_Length	= SIGNED_TIK_SIZE(Ticket);

        Out->Print("Ticket at: 0x%x\n", reinterpret_cast<dword>(Ticket));

        // Open Partition and get the TMD buffer
        if (DI->Open_Partition(Partition_Info.Offset, 0,0,0, Tmd_Buffer) < 0)
        {
            Out->PrintErr("[-] Error opening partition.\n\n");
			throw "Open Error";
        }

        // Get the TMD pointer
        Tmd = reinterpret_cast<signed_blob*>(Tmd_Buffer);
        MD_Length = SIGNED_TMD_SIZE(Tmd);

        Out->Print("Tmd at: 0x%x\n", reinterpret_cast<dword>(Tmd));
		
		Out->Print("[+] Partition opened successfully.\n");
		Out->Print("IOS requested by the game: %u\n", Tmd_Buffer[0x18b]);
		Log->Write("IOS requested by the game: %u\r\n", Tmd_Buffer[0x18b]);


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
        DI->Read((void*)Memory::Apploader, Loader.Size + Loader.Trailer_Size, Wii_Disc::Offsets::Apploader + 0x20);
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
		bool	Removed_002 = (!Cfg->Data.Remove_002);
		bool	Videomode_found = false;

        Out->Print("Loading.\t\t\n");
        while(Load(&Address, &Section_Size, &Partition_Offset))
        {
            Out->Print(".");

            if (!Address) throw ("Null pointer from apploader");

            DI->Read(Address, Section_Size, Partition_Offset << 2);
            DCFlushRange(Address, Section_Size);
            // NOTE: This would be the ideal time to patch main.dol
			if (!Lang_Patched) Lang_Patched = Set_GameLanguage(Address, Section_Size);
			if (!Removed_002) Removed_002 = Remove_002_Protection(Address, Section_Size);
			if (!Videomode_found) Videomode_found = Check_Video_Mode(Address, Section_Size);
        }
		
		if (!Videomode_found)
		{
			Log->Write("Error: The used video mode was not found in the main.dol\r\n");
		}
		if ((Cfg->Data.Language != -1) && (!Lang_Patched))
		{
			Log->Write("Error: Did not patch the language, pattern not found\r\n");
		}
		if (Cfg->Data.Remove_002)
		{
			if (Removed_002)
			{
				Log->Write("002 error removed\r\n");
			} else
			{
				Log->Write("002 error pattern not found\r\n");
			}
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

        // Set Video Mode based on Configuration
        Set_VideoMode();

        // Flush application memory range
        DCFlushRange((void*)0x80000000, 0x17fffff);	// TODO: Remove these hardcoded values

		// Close the logfile
		Log->CloseLog();
		
		// Release FAT
		SD->Release_FAT();

        // Cleanup loader information
        DI->Close();

		// Identify as the game
		if (IS_VALID_SIGNATURE(Certs) 	&& IS_VALID_SIGNATURE(Tmd) 	&& IS_VALID_SIGNATURE(Ticket) 
		&&  C_Length > 0 				&& MD_Length > 0 			&& T_Length > 0)
		{
			int ret;
			ret = ES_Identify(Certs, C_Length, Tmd, MD_Length, Ticket, T_Length, NULL);
			if (ret < 0)
			{
				Out->PrintErr("Error: ES_Identify returned %d\n", ret);
			} else
			{
				Out->Print("ES_Identify successful\n");
			}			
		}
		
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
		Out->SetSilent(false);
        Out->PrintErr("Exception: %s\n\n", Message);
		Log->Write("Exception: %s\r\n", Message);

		// Stop Drive
		DI->Stop_Motor();

		// Return to Menu
		NextPhase = Phase_Menu;

		// Wait User
		Controls->Press_AnyKey("Press Any Key to Continue...\n\n");
    }
}

/*******************************************************************************
 * Determine_VideoMode: Determines which video mode to use based on current system settings
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Determine_VideoMode(char Region)
{
    // TODO: Some exception handling is needed here
    vmode = VIDEO_GetPreferredMode(0);

	if (Cfg->Data.SysVMode)
	{
		Video_Mode = (vmode->viTVMode) >> 2;
	}
	else
	{
		switch (Region) 
		{
			case Wii_Disc::Regions::PAL_Default:
			case Wii_Disc::Regions::PAL_France:
			case Wii_Disc::Regions::PAL_Germany:
			case Wii_Disc::Regions::Euro_X:
			case Wii_Disc::Regions::Euro_Y:
				if ((vmode->viTVMode) >> 2 == Video::Modes::PAL60 || (vmode->viTVMode) >> 2 == Video::Modes::PAL)
				{
					Video_Mode = (vmode->viTVMode) >> 2;
				} else
				{
					Video_Mode = (unsigned int)Video::Modes::PAL60;
					if (vmode->xfbMode == VI_XFBMODE_SF)
					{
						vmode = &TVEurgb60Hz480Prog;		// PAL 480p
					} else
					{
						vmode = &TVEurgb60Hz480IntDf;		// PAL 480i
					}				
				}
				break;

			case Wii_Disc::Regions::NTSC_USA:
			case Wii_Disc::Regions::NTSC_Japan:
			default:
				Video_Mode = (unsigned int)Video::Modes::NTSC;
				if ((vmode->viTVMode) >> 2 != Video::Modes::NTSC)
				{
					if (vmode->xfbMode == VI_XFBMODE_SF)
					{
						vmode = &TVNtsc480Prog;			// NTSC 480p
					} else
					{
						vmode = &TVNtsc480IntDf;		// NTSC 480i
					}				
				}
				break;
		}		
	}
}


/*******************************************************************************
 * Set_VideoMode: Sets to the video mode determined in: Determine_VideoMode
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Set_VideoMode()
{
    // TODO: Some exception handling is needed here
 
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
 * Check_Video_Mode: Checks if the used video mode is found inside the main.dol
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if video mode is found
 *
 ******************************************************************************/

bool SoftChip::Check_Video_Mode(void *Address, int Size)
{
	u8 *Addr				= (u8 *)Address;

	/*
	VI_XFBMODE_DF seems to be an indicator for interlaced video mode
	VI_XFBMODE_SF seems to be an indicator for progressive video mode
	*/

	while (Size >= 15)
	{
		if (Addr[3] == 0x01 && Addr[4] == 0xE0 && Addr[5] == 0x01 && Addr[6] == 0xE0 && Addr[10] == 0x00 && Addr[13] == 0x01 && Addr[14] == 0xE0)
		{
			if (Addr[0] == 0x00) 
			{
				if (Video_Mode == Video::Modes::NTSC && vmode->xfbMode == VI_XFBMODE_DF) return true;
				//Log->Write("NTSC 480i found\r\n");					
			}
			if (Addr[0] == 0x02) 
			{
				if (Video_Mode == Video::Modes::NTSC && vmode->xfbMode == VI_XFBMODE_SF) return true;
				//Log->Write("NTSC 480p found\r\n");					
			}
			if (Addr[0] == 0x14) 
			{
				if (Video_Mode == Video::Modes::PAL && vmode->xfbMode == VI_XFBMODE_DF && vmode->viHeight == 480) return true;
				//Log->Write("PAL 480i found\r\n");					
			}
			if (Addr[0] == 0x16) 
			{
				if (Video_Mode == Video::Modes::PAL && vmode->xfbMode == VI_XFBMODE_SF && vmode->viHeight == 480) return true;
				//Log->Write("PAL 480p found\r\n");					
			}
			if (Addr[0] == 0x08) 
			{
				if (Video_Mode == Video::Modes::MPAL && vmode->xfbMode == VI_XFBMODE_DF) return true;
				//Log->Write("MPAL 480i found\r\n");					
			}
			if (Addr[0] == 0x0A) 
			{
				if (Video_Mode == Video::Modes::MPAL && vmode->xfbMode == VI_XFBMODE_SF) return true;
				//Log->Write("MPAL 480p found\r\n");					
			}
		}
		if (Addr[3] == 0x02 && Addr[4] == 0x10 && Addr[5] == 0x02 && Addr[6] == 0x10 && Addr[10] == 0x17 && Addr[13] == 0x02 && Addr[14] == 0x10)
		{
			if (Addr[0] == 0x04) 
			{
				if (Video_Mode == Video::Modes::PAL && vmode->xfbMode == VI_XFBMODE_DF && (vmode->viHeight == 524 || vmode->viHeight == 528 || vmode->viHeight == 574)) return true;
				//Log->Write("PAL 576i found\r\n");					
			}
			if (Addr[0] == 0x06) 
			{
				if (Video_Mode == Video::Modes::PAL && vmode->xfbMode == VI_XFBMODE_SF && (vmode->viHeight == 524 || vmode->viHeight == 528 || vmode->viHeight == 574)) return true;
				//Log->Write("PAL 576p found\r\n");					
			}
		}
		if (Addr[3] == 0x01 && Addr[4] == 0x08 && Addr[5] == 0x02 && Addr[6] == 0x0c && Addr[10] == 0x17 && Addr[13] == 0x02 && Addr[14] == 0x0c)
		{
			if (Addr[0] == 0x06) 
			{
				if (Video_Mode == Video::Modes::PAL && vmode->xfbMode == VI_XFBMODE_SF && (vmode->viHeight == 524 || vmode->viHeight == 528 || vmode->viHeight == 574)) return true;
				//Log->Write("PAL 576p* found\r\n");					
			}
		}
		
		Addr += 1;
		Size -= 1;
	}

	return false;
}

/*******************************************************************************
 * Remove_002_protection: Removes the 002 copy protection
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if patched
 *
 ******************************************************************************/

bool SoftChip::Remove_002_Protection(void *Address, int Size)
{
	unsigned int SearchPattern[3]	= { 0x2C000000, 0x40820214, 0x3C608000 };
	unsigned int PatchData[3]		= { 0x2C000000, 0x48000214, 0x3C608000 };
	unsigned int *Addr				= (unsigned int*)Address;

	while (Size >= 12)
	{
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2])
		{
			*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			return true;
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
 * VerifyFlags: Verify Standby and Reboot Flags
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::VerifyFlags()
{
    if (Standby_Flag)
    {
        Controls->Terminate();
        STM_ShutdownToStandby();
    }

    else if (Reset_Flag)
    {
        STM_RebootSystem();
    }
}

/*******************************************************************************
 * Exit_Loader: Exit
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns void
 *
 ******************************************************************************/

void SoftChip::Exit_Loader()
{
	// DOL Version
	if (*(dword*)Memory::Exit_Stub) exit(0);

	// Channel Version
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}
