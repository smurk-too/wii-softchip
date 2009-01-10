/*******************************************************************************
 * cIOS.cpp
 *
 * Copyright (c) 2009 Requiem (requiem@century-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	This is a port/encapsulation of Waninkoko's libcios.
 *
 ******************************************************************************/

//--------------------------------------
// Includes

#include "cIOS.h"

//--------------------------------------
// Typedefs

typedef struct
{
	sig_rsa2048	signature;
	tik			tik_data;
} ATTRIBUTE_PACKED signed_tik;

//--------------------------------------
// Globals

static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";

/*******************************************************************************
 * s32 Load: Loads the IOS specified in Version
 * -----------------------------------------------------------------------------
 * Return Values:
 *
 * Returns the result of IOS_ReloadIOS
 *
 ******************************************************************************/

s32 cIOS::Load(u32 Version)
{
	return IOS_ReloadIOS(Version);
}

/*******************************************************************************
 * s32 Version: Wrapper around IOS_GetVersion
 * -----------------------------------------------------------------------------
 * Return Values:
 *
 * Returns the result of IOS_GetVersion
 *
 ******************************************************************************/

s32 cIOS::Version()
{
	return IOS_GetVersion();
}

/*******************************************************************************
 * s32 Load: Loads the IOS specified in Version
 * -----------------------------------------------------------------------------
 * Return Values:
 *
 * Returns the result of IOS_ReloadIOS
 *
 ******************************************************************************/

s32 cIOS::Identify()
{
	signed_blob* Certs	= NULL;
	signed_blob* Ticket	= NULL;
	signed_blob* TMD	= NULL;

	u32	Cert_Length;
	u32	Ticket_Length;
	u32	TMD_Length;

	s32 ret = GetCerts(&Certs, &Cert_Length);
	if (ret < 0)
	{
		if (Certs)	free(Certs);
		if (Ticket)	free(Ticket);
		if (TMD)	free(TMD);
		return ret;
	}

	ret = GenerateTicket(&Ticket, &Ticket_Length);
	if (ret < 0)
	{
		if (Certs)	free(Certs);
		if (Ticket)	free(Ticket);
		if (TMD)	free(TMD);
		return ret;
	}

	ret = GetTMD(TITLEID(Version()), &TMD, &TMD_Length);
	if (ret < 0)
	{
		if (Certs)	free(Certs);
		if (Ticket)	free(Ticket);
		if (TMD)	free(TMD);
		return ret;
	}

	return ES_Identify(Certs, Cert_Length, TMD, TMD_Length, Ticket, Ticket_Length, NULL);
}


s32 cIOS::GetCerts(signed_blob** Certs, u32* Length)
{
	static unsigned char		Cert[CERTS_SIZE] __attribute__((aligned(0x20)));
	memset(Cert, 0, CERTS_SIZE);
	s32				fd, ret;

	fd = IOS_Open(certs_fs, ISFS_OPEN_READ);
	if (fd < 0) return fd;

	ret = IOS_Read(fd, Cert, CERTS_SIZE);
	if (ret < 0)
	{
		if (fd >0) IOS_Close(fd);
		return ret;
	}

	*Certs = reinterpret_cast<signed_blob*>(Cert);
	*Length = CERTS_SIZE;

	if (fd > 0) IOS_Close(fd);

	return 0;
}

s32 cIOS::GetTMD(u64 TicketID, signed_blob **Output, u32 *Length)
{
	signed_blob* TMD = NULL;

	u32 TMD_Length;
	s32 ret;

	/* Retrieve TMD length */
	ret = ES_GetStoredTMDSize(TicketID, &TMD_Length);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	TMD = (signed_blob*)memalign(32, TMD_Length);
	if (!TMD)
		return IPC_ENOMEM;

	/* Retrieve TMD */
	ret = ES_GetStoredTMD(TicketID, TMD, TMD_Length);
	if (ret < 0)
	{
		free(TMD);
		return ret;
	}

	/* Set values */
	*Output = TMD;
	*Length = TMD_Length;

	return 0;
}

s32	cIOS::GenerateTicket(signed_blob** Output, u32* Length)
{
	signed_tik* Ticket = NULL;

	/* Allocate memory */
	Ticket = (signed_tik*)memalign(32, sizeof(signed_tik));
	if (!Ticket)
		return IPC_ENOMEM;

	/* Clean ticket */
	memset(Ticket, 0, sizeof(signed_tik));

	/* Generate signature */
	Ticket->signature.type = ES_SIG_RSA2048;

	/* Generate ticket */
	memset(Ticket->tik_data.cidx_mask, 0xFF, 32);
	strcpy(Ticket->tik_data.issuer, "Root-CA00000001-XS00000003");

	/* Set values */
	*Output = (signed_blob *)Ticket;
	*Length = sizeof(signed_tik);

	return 0;
}

/*******************************************************************************
 * List_IOS: Fill a buffer with the list of IOS
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns < 0 (Error) or 0 (Success)
 *
 ******************************************************************************/

s32 cIOS::List_SysTitles()
{
	static u64 Titles[MAX_TITLES] ATTRIBUTE_ALIGN(32);
	u32 Count = 0, i;

	// Get Count
	s32 Result = ES_GetNumTitles(&Count);

	// Verify
	if (Result < 0) return Result;
	if (Result > MAX_TITLES) return -1;

	// Get Data
	Result = ES_GetTitles(Titles, Count);
	if (Result < 0) return Result;

	// Clear SysTitles
	SysTitles.clear();

	for (i = 0; i < Count; i++)
	{
		// Look for System Titles (Type == 1)
		if (Titles[i] >> 32 == 1)
		{
			// Get Title
			SysTitles.push_back((u32)(Titles[i] & 0xFFFFFFFF));
		}
	}

	// Sort
	sort(SysTitles.begin(), SysTitles.end());

	// Success
	return 0;
}
