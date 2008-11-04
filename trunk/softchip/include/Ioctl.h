/*******************************************************************************
 * Ioctl.h
 *
 * Copyright (c) 2008 Requiem (requiem@cenury-os.com)
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *	Interface library to access /dev/di using the cIOS Ioctl commands.
 *
 ******************************************************************************/


//--------------------------------------
// Includes

//--------------------------------------
// Ioctl Namespace

namespace Ioctl
{
	enum
	{
		DI_Inquiry				= 0x12,
		DI_ReadID				= 0x70,
		DI_Read					= 0x71,
		DI_WaitCoverClose		= 0x79,
		DI_ResetNotify			= 0x7e,
		DI_GetCover				= 0x88,
		DI_Reset				= 0x8a,
		DI_OpenPartition		= 0x8b,
		DI_ClosePartition		= 0x8c,
		DI_ReadUnencrypted		= 0x8d,
		DI_Seek					= 0xab,
		DI_ReadDVD				= 0xd0,
		DI_StopLaser			= 0xd2,
		DI_Offset				= 0xd9,
		DI_RequestError			= 0xe0,
		DI_StopMotor			= 0xe3,
		DI_Streaming			= 0xe4,
		DI_EnableDVD			= 0xf0,
		DI_SetOffsetBase		= 0xf1,
		DI_GetOffsetBase		= 0xf2,
		DI_CustomCommand		= 0xff
	};
}
