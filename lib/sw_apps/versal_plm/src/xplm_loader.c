/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_loader.c
*
* This file contains the wrapper functions for platform loader
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_loader.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XilPdi PdiInstance;

/*****************************************************************************/
/**
* It initializes the loader structures and registers  the CDO loader
* commands
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_LoaderInit()
{
	int Status;

	Status = XLoader_Init();

	return Status;
}

/*****************************************************************************/
/**
* It loads the boot PDI
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_LoadBootPdi(void *arg)
{
	int Status;
	u32 BootMode;

	XPlmi_PrintPlmTimeStamp();
	XPlmi_Printf(DEBUG_PRINT_PERF, "PLM Initialization Time \n\r");

	/**
	 * 1. Read Boot mode register and multiboot offset register
	 * 2. Load subsystem present in PDI
	 */
	XilPdi* PdiPtr = &PdiInstance;
	PdiPtr->SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
					PMC_TAP_SLR_TYPE_VAL_MASK;

	if ((PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR) ||
		(PdiPtr->SlrType == XLOADER_SSIT_MONOLITIC)) {
		BootMode = XLoader_GetBootMode();
	} else {
		BootMode = XLOADER_PDI_SRC_SBI;
	}

	/**
	 * In case of JTAG boot mode and jtag mode is not SBI,
	 * no PDI loading is present
	 */
	if ((BootMode == XLOADER_PDI_SRC_JTAG) &&
	    (!XLoader_IsJtagSbiMode()))
	{
		Status = XST_SUCCESS;
		goto END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Started***********\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	Status = XLoader_LoadPdi(PdiPtr, BootMode, 0U);
	if (Status != XST_SUCCESS)
	{
		goto ERR_END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "***********Boot PDI Load: Done*************\n\r");

	/** Print ROM time and PLM time stamp */
	XPlmi_PrintRomTime();
	XPlmi_PrintPlmTimeStamp();
	XPlmi_Printf(DEBUG_PRINT_PERF, "Total PLM Boot Time \n\r");
END:
	/**
	 * This is used to identify PLM has completed boot PDI
	 */
	XPlmi_SetBootPdiDone();

	XLoader_ClearIntrSbiDataRdy();
	/**
	 * Enable the SBI RDY interrupt to get the next PDI
	 */
	XPlmi_PlmIntrEnable(XPLMI_SBI_DATA_RDY);

ERR_END:
	return Status;
}
