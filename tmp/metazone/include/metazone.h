/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#ifndef _METAZONE_H_
#define _METAZONE_H_

#define MZ_RD_ONLY_IDX_START  0x0000
#define MZ_RD_ONLY_DWORD_NUM   0
#define MZ_RD_ONLY_BINARY_NUM  0


#define MZ_WR_IDX_START  0x10000U
#define MZ_WR_DWORD_NUM   200
#define MZ_WR_BINARY_NUM  10


/* For FS RW, now it is not enable, only used in WINCE */
#define MZ_FS_IDX_START  0x20000U
#define MZ_FS_DWORD_NUM   2000U
#define MZ_FS_BINARY_NUM  500U
#define MZ_FS_BINARY_SIZE  100U

#define MZ_MSDK_START  0x20000
#define MZ_MSDK_END    0x20

#define MZ_WC_DW_IDX_START  (MZ_WR_IDX_START)	/* Recording writing code of metazone */

/* Backlight index */
#define MZ_BACKLIGHT_DW_IDX_START  (MZ_WR_IDX_START + 1)	/* Backlight setting . */


/* Index for BT Transport Layer Driver */
#define MZ_BT_ADDR         MZ_WR_IDX_START	/* Index for BT Address in binary data */
#define MZ_BT_ADDR_DONE   (MZ_WR_IDX_START + 8)
#define MZ_BT_STATUS      (MZ_FS_IDX_START + 8)


/* Index for Touch panel */
#define MZ_TP_DW_IDX_START  (MZ_WR_IDX_START + 10)
#define MZ_TP_DW_IDX_PRIORITY     (MZ_TP_DW_IDX_START+1)
#define MZ_TP_DW_IDX_MAXERROR     (MZ_TP_DW_IDX_START+2)
#define MZ_TP_DW_IDX_SPL          (MZ_TP_DW_IDX_START+3)
#define MZ_TP_DW_IDX_RISTHRESHOLE (MZ_TP_DW_IDX_START+4)
#define MZ_TP_DW_IDX_END    (MZ_WR_IDX_START + 18)

#define MZ_TP_BIN_IDX_START (MZ_WR_IDX_START + 1)
#define MZ_TP_BIN_IDX_CALIBRATION (MZ_TP_BIN_IDX_START)
#define MZ_TP_BIN_IDX_END   (MZ_WR_IDX_START + 1)


/* Index for  Waveform Driver */
#define MZ_WAV_DW_IDX_START   (MZ_WR_IDX_START + 20)	/*  */
#define MZ_WAV_DW_IDX_END     (MZ_WR_IDX_START + 39)	/*  */
#define MZ_WAV_BIN_IDX_START   (MZ_WR_IDX_START + 2)	/*  */
#define MZ_WAV_BIN_IDX_END     (MZ_WR_IDX_START + 7)	/*  */


/* Index for USB Driver. */
#define MZ_USB_DW_IDX_START  (MZ_FS_IDX_START + 2)	/* Usb setting */
#define MZ_USB_BIN_IDX_START  (MZ_FS_IDX_START + 1)	/* Usb setting */


/* Index for SDK & MSDK */

#define MZ_SDK_DW_DEF_IDX_START  (MZ_WR_IDX_START + 200)
#define MZ_SDK_DW_DEF_IDX_END    (MZ_WR_IDX_START + 499)

#define MZ_SDK_DW_IDX_START      (MZ_FS_IDX_START + 100)
#define MZ_SDK_DW_IDX_END        (MZ_FS_IDX_START + 399)

#define MZ_SDK_BIN_DEF_IDX_START  (MZ_WR_IDX_START + 10)
#define MZ_SDK_BIN_DEF_IDX_END    (MZ_WR_IDX_START + 19)

#define MZ_SDK_BIN_IDX_START     (MZ_FS_IDX_START + 10)
#define MZ_SDK_BIN_IDX_END       (MZ_FS_IDX_START + 19)

#define MZ_DRM_INFO_IDX_START   (MZ_WR_IDX_START + 9)

#define  MZ_HDCPKEY_BIN_IDX_START      (MZ_WR_IDX_START + 150)
#define  MZ_HDCPKEY_BIN_IDX_END        (MZ_WR_IDX_START + 163)

#define  MZ_WIFI_MAC        (MZ_WR_IDX_START + 26)
#define  MZ_BT_MAC        (MZ_WR_IDX_START + 27)
#define  MZ_WIFI_TYPE        (MZ_WR_IDX_START + 28)
#define  MZ_GPS_TYPE        (MZ_WR_IDX_START + 29)
#define  MZ_BT_TYPE        (MZ_WR_IDX_START + 30)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*__cplusplus*/
/**
*	@brief  Initialize MetaZone API.  Please call this function first before calling other functions.
*
*      @param
*      @param
*
*
*	@return	   return MZ_SUCCESS if succeeds or MZ_FAILURE if failure.
*
*
*	@see
*     @note
**/

unsigned int MetaZone_Init(void);

/**
*	@brief  De-initialize MetaZone API. Call this function to release resource of MetaZone API.
*
*      @param
*      @param
*
*
*	@return	   return MZ_SUCCESS if succeeds or MZ_FAILURE if failure.
*
*
*	@see
*     @note
**/

unsigned int MetaZone_Deinit(void);

/**
*	@brief  Read MetaZone configuration inforation..
*
*      @param  prInfo -- Pointer to structure METAZONE_INFO_T to receive data
*      @param
*
*
*	@return	   return MZ_SUCCESS if succeeds or MZ_FAILURE if failure.
*
*
*	@see
*     @note
**/
/* unsigned int MetaZone_ReadInfo(METAZONE_INFO_T *prInfo); */


/**
*	@brief  Read a DWORD value of given index.
*
*      @param u4Idx -- index of DWORD to be read.
*      @param  pu4Data -- Pointer to receive the value.
*
*
*	@return	   return 4 ( 4 bytes, a dword) if succeeds or 0 if failure.
*
*
*	@see
*   @note  Index of Read only section from 0x0000 to u4RdValueNum -1.
*		The u4RdValueNum is return by MetaZone_ReadInfo.
*          Index of writable section from 0x10000 to 0x10000 + u4WrValueNum -1.
*		The u4WrValueNum return by MetaZone_ReadInfo.
*          Index of file section from 0x20000 to 0x20000 + u4FsValueNum -1 .
*		The u4FsValueNum return by MetaZone_ReadInfo.
**/

unsigned int MetaZone_Read(unsigned int u4Idx, unsigned int *pu4Data);

/**
*	@brief  Write a DWORD value of given index.
*
*      @param u4Idx -- index of DWORD to be written.
*      @param  u4Data -- Data to be written.
*
*
*	@return	   return MZ_SUCCESS if succeeds or other values if failure.
*
*
*	@see
*   @note  Index of Read only section from 0x0000 to u4RdValueNum -1.
*		The u4RdValueNum is return by MetaZone_ReadInfo.
*          Index of writable section from 0x10000 to 0x10000 + u4WrValueNum -1.
*		The u4WrValueNum return by MetaZone_ReadInfo.
*          Index of file section from 0x20000 to 0x20000 + u4FsValueNum -1 .
*		The u4FsValueNum return by MetaZone_ReadInfo.
**/

unsigned int MetaZone_Write(unsigned int u4Idx, unsigned int u4Data);

/**
*	@brief  Read binary value of given index.
*
*      @param u4Idx   -- index of binary data.
*      @param  pbData -- buffer to receive data.
*      @param  u4Size -- size of buffer (in bytes).
*
*
*	@return	 reture the read size of binary data if succeeds or return value large than 0x8000000.
*
*
*	@see
*   @note  Index of Read only section from 0x0000 to u4RdBinarySize -1.
*		The u4RdBinarySize is return by MetaZone_ReadInfo.
*          Max size of binary data in read only section is specified by u4RdBinarySize
*		which is return by MetaZone_ReadInfo.
*          Index of writable section from 0x10000 to 0x10000 + u4WrBinaryNum -1.
*		The u4WrBinaryNum return by MetaZone_ReadInfo.
*          Max size of binary data in writable section is specified by u4WrBinarySize
*		which is return by MetaZone_ReadInfo.
*          Index of file section from 0x20000 to 0x20000 + u4FsBinaryNum -1 .
*		The u4FsBinaryNum return by MetaZone_ReadInfo.
*          Max size of binary data in file section is specified by u4FsBinarySize
*		which is return by MetaZone_ReadInfo.
**/

unsigned int MetaZone_ReadBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size);

/**
*	 @brief  Write binary value of given index.
*
*      @param u4Idx   -- index of binary data.
*      @param  pbData -- pointer to data to be written..
*      @param  u4Size -- size of data (in bytes).
*
*
*	@return	   return MZ_SUCCESS if succeeds or other values if failure.
*
*
*	@see
*   @note  Index of Read only section from 0x0000 to u4RdValueNum -1.
*		The u4RdValueNum is return by MetaZone_ReadInfo.
*          Index of writable section from 0x10000 to 0x10000 + u4WrValueNum -1.
*		The u4WrValueNum return by MetaZone_ReadInfo.
*          Index of file section from 0x20000 to 0x20000 + u4FsValueNum -1 .
*		The u4FsValueNum return by MetaZone_ReadInfo.
**/
unsigned int MetaZone_WriteBinary(unsigned int u4Idx, const char *pbData, unsigned int u4Size);

/**
*	@brief  Read all reserved data.
*      @param  pbData -- Buffer to received data.
*      @param  u4Size -- size of buffer (in bytes).
*
*	@return	 reture the read size of reserved data if succeeds or return value large than 0x8000000.
*
*
*	@see
*   @note  Reserved data in writable section is reserved for customer.
**/

unsigned int MetaZone_ReadReserved(char *pbData, unsigned int u4Size);

/**
*	@brief  Write reserved data.
*
*      @param  pbData -- Buffer to reserved data.
*      @param  u4Size -- size of data (in bytes).
*
*	@return	 return MZ_SUCCESS if succeeds or other values if failure.
*
*
*	@see
*     @note
**/

unsigned int MetaZone_WriteReserved(char *pbData, unsigned int u4Size);

/**
*	@brief  Write data to storage..
*
*	@return	 return MZ_SUCCESS if succeeds or other values if failure.
*
*
*	@see
*     @note
**/
unsigned int MetaZone_Flush(int fgSync);

unsigned int MetaZone_WriteLogo(char *pbData, unsigned int u4Size);

#ifdef __cplusplus
#if __cplusplus
}	// End extern "C"
#endif
#endif /*__cplusplus*/

#endif				/* _METAZONE_H_ */
