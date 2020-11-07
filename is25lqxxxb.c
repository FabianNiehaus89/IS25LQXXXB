/*
 * 		Created on: 01.04.2020
 *      Author: Fabian Niehaus
 *      Mail:	fabian.niehaus@tuhh.de
 *
 *      STM32 flash memory driver - IS25LQXXXB
 *
 */

//Includes
#include "is25lq040b_ext_mem.h"

//Private variables
IS25mem_Identification	memory_ident		= {0};
IS25mem_MemorySpace		memory_space		= {0};
QSPI_HandleTypeDef 		*qspi_h				=  0;


//function prototypes
void (*autoPollingCallback)(void) 	= 0;
void (*pEraseDoneCallback) 			= 0;


//Function to register the Callback in Callback routine
void IS25mem_registerCallback(void (*functPtr)(void)){
	autoPollingCallback = functPtr;
}

/**
 * setEraseDoneCallbackFct(void (*fct)
 *
 * @brief
 *
 * @Parameter
 * 		void (*fct)  -	address of function which will be called when eraseDoneCallback is fired.
 *
 * @return
 * 		falsh_err	- error code of memory functions
 **/
void setEraseDoneCallbackFct(void (*fct)){
	pEraseDoneCallback = fct;
}

/**
 * Init IS24lq flash driver
 *
 * @Brief
 * 		Use this function to init. the driver in main. As Parameter the address of the QSPI-Handler is needed.
 *
 * @Parameter
 * 		QSPI_HandleTypeDef * - QSPI Handler
 *
 * @return
 * 		flash_err
 *
 */
flash_err IS25mem_Init(QSPI_HandleTypeDef *qSPIHandler){

	if(qSPIHandler != 0){
		qspi_h = qSPIHandler;
	}

	IS25mem_readProductId(&memory_ident);
	switch(memory_ident.Capacity){
		case 0x13:	memory_space.blocks64 	= 8;
					memory_space.blocks32 	= 16;
					memory_space.sectors	= 128;
					return MEMORY_OK;
		case 0x12:	memory_space.blocks64 	= 4;
					memory_space.blocks32 	= 8;
					memory_space.sectors	= 64;
					return MEMORY_OK;
		case 0x11:	memory_space.blocks64 	= 2;
					memory_space.blocks32 	= 4;
					memory_space.sectors	= 32;
					return MEMORY_OK;
		case 0x10:	memory_space.blocks64 	= 0;
					memory_space.blocks32 	= 2;
					memory_space.sectors	= 16;
					return MEMORY_OK;
		case 0x09:	memory_space.blocks64 	= 0;
					memory_space.blocks32 	= 1;
					memory_space.sectors	= 8;
					return MEMORY_OK;
		default: return MEMORY_WRONG_CPACITY_ERR;
	}
}

/**
 * READ DATA OPERATION (RD, 03h)
 *
 * @Brief The Read Data (RD) instruction is used to read memory contents of the device at a maximum frequency of 33MHz.
 * 			If a Read Data instruction is issued while an Erase, Program or Write cycle is in process (WIP=1) the instruction
 * 			is ignored and will not have any effects on the current cycle.
 * @Parameter		uint8_t *		- bufferPointer
 * 					mem_address 	- memory Address
 * 					uint8_t			- size
 * @Return value 	flash_err		- flash memory return value (MEMORY_ERROR or MEMORY_OK)
 */

flash_err IS25mem_readData(uint8_t *readBuffer,mem_address address, uint16_t size){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RD;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= size;


	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, readBuffer, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * READ DATA OPERATION (FR, 0Bh)
 *
 * @Brief The Fast Read instruction is used to read memory data at up to a 104MHZ clock.
 * 			If a FAst Read Data instruction is issued while an Erase, Program or Write cycle is in process (WIP=1) the instruction
 * 			is ignored and will not have any effects on the current cycle.
 * @Parameter		uint8_t *		- bufferPointer
 * 					mem_address 	- memory Address
 * 					uint8_t			- size
 * @Return value 	flash_err		- flash memory return value (MEMORY_ERROR or MEMORY_OK)
 */
flash_err IS25mem_fastReadData(uint8_t *readBuffer,mem_address address, uint16_t size){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= FR;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= size;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, readBuffer, 200) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * FAST READ DUAL I/O OPERATION (FRDIO, BBh)
 *
 * @Brief 	The FRDIO instruction allows the address bits to be input two bits at a time. This may allow for code to be executed
 * 			directly from the SPI in some applications.
 *
 * 			If a FRDIO instruction is issued while an Erase, Program or Write cycle is in process (WIP=1) the instruction
 * 			is ignored and will not have any effects on the current cycle.
 *
 * @Parameter		uint8_t *		- bufferPointer
 * 					mem_address 	- memory Address
 * 					uint8_t			- size
 * @Return value 	flash_err		- flash memory return value (MEMORY_ERROR or MEMORY_OK)
 */
flash_err IS25mem_DualFastReadData(uint8_t *readBuffer,mem_address address, uint8_t size){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= FR;
	memCmd.AddressMode 			= QSPI_ADDRESS_2_LINES;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_2_LINES;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= size;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, readBuffer, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * FAST READ QUAD OUTPUT (FRQO, 6Bh)
 *
 * @Brief 	The FRQO instruction is used to read memory data on four output pins each at up to a 104 MHz clock.
 *
 * 			If a FRQO instruction is issued while an Erase, Program or Write cycle is in process (WIP=1) the instruction
 * 			is ignored and will not have any effects on the current cycle.
 *
 * @Parameter		uint8_t *		- bufferPointer
 * 					mem_address 	- memory Address
 * 					uint8_t			- size
 * @Return value 	flash_err		- flash memory return value (MEMORY_ERROR or MEMORY_OK)
 */
flash_err IS25mem_QuadFastReadData(uint8_t *readBuffer,mem_address address, uint8_t size){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= FR;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_4_LINES;
	memCmd.DummyCycles 			= 8;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= size;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, readBuffer, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * @Brief        	READ PRODUCT IDENTIFICATION (RDID, ABh)
 * @Description
	The Release from Power-down/Read Device ID instruction is a multi-purpose instruction. It can support both SPI
	and Multi-IO mode. The Read Product Identification (RDID) instruction is for reading out the old style of 8-bit
	Electronic Signature, whose values are shown as table of Product Identification.
	The RDID instruction code is followed by three dummy bytes, each bit being latched-in on SI during the rising
	SCK edge. Then the Device ID is shifted out on SO with the MSB first, each bit been shifted out during the falling
	edge of SCK. The RDID instruction is ended by CE# going high. The Device ID (ID7-ID0) outputs repeatedly if
	additional clock cycles are continuously sent on SCK while CE# is at low.
 *
 * @Parameter     deviceID *
 * @Return value  flash_err
 */
flash_err IS25mem_readID(IS25mem_deviceID *id){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDID;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, id, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * @Brief
 * @Description
 * The JEDEC ID READ instruction allows the user to read the Manufacturer and Product ID of devices. Refer to
	Table 8.4 Product Identification for Manufacturer ID and Device ID. After the JEDEC ID READ command is input,
	the Manufacturer ID is shifted out on SO with the MSB first, followed by the Memory Type and Capacity ID15-ID0.
	Each bit is shifted out during the falling edge of SCK. If CE# stays low after the last bit of the Device ID is shifted
	out, the Manufacturer ID and Device ID (Type/Capacity) will loop until CE# is pulled high.
 * @Parameter     IS25mem_Identification *
 * @Return value  flash_err
 */
flash_err IS25mem_readProductId(IS25mem_Identification *productId){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDJDID;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 3;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, (uint8_t *)productId, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * @Brief		RELEASE DEEP POWER DOWN (RDPD, ABh)
 *
 * @Description
 *
 * The Release from Power-down/Read Device ID instruction is a multi-purpose instruction. To release the device
	from the deep power-down mode, the instruction is issued by driving the CE# pin low, shifting the instruction code
	“ABh” and driving CE# high.
	Release from power-down will take the time duration of tRES1 before the device will resume normal operation and
	other instructions are accepted. The CE# pin must remain high during the tRES1 time duration.
	If the Release from Power-down/RDID instruction is issued while an Erase, Program or Write cycle is in process
	(when WIP equals 1) the instruction is ignored and will not have any effects on the current cycle.

 * @Return Value	flash_err
 */
flash_err IS25mem_releasePowerDown(void){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDPD;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	//Time needed for the memory to power up
	HAL_Delay(5);

	return MEMORY_OK;
}

/**
 * @Brief		DEEP POWER DOWN (DP, B9h)
 *
 * @Description
 *
 * The Deep Power-down (DP) instruction is for setting the device on the minimizing the power consumption (enter
	into Power-down mode), and the standby current is reduced from Isb1 to Isb2. During the Power-down mode, the
	device is not active and all Write/Program/Erase instructions are ignored. The instruction is initiated by driving the
	CE# pin low and shifting the instruction code into the device. The CE# pin must be driven high after the instruction
	has been latched. If this is not done the Power-Down will not be executed. After CE# pin driven high, the powerdown
	state will be entered within the time duration of tDP. While in the power-down state only the Release from
	Power-down/RDID instruction, which restores the device to normal operation, will be recognized. All other
	instructions are ignored. This includes the Read Status Register instruction, which is always available during
	normal operation. Ignoring all but one instruction makes the Power Down state a useful condition for securing
	maximum write protection. It can support in SPI and Multi-IO mode.

 * @Return Value	flash_err
 */
flash_err IS25mem_DeepPowerDown(void){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= DP;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	//Time needed for the memory to power up
	HAL_Delay(5);

	return MEMORY_OK;
}

/**
 * WRITE FUNCTION REGISTER OPERATION (WRFR, 42h)
 * Information Row Lock bits (IRL3~IRL0) can be set to “1” individually by WRFR instruction in order to lock
 * Information Row. Since IRL bits are OTP, once it is set to “1”, it cannot set back to “0” again.
 *
 * @Parameter 		extFlash_func
 * @Return Value	falsh_err
 */
flash_err IS25mem_writeFctReg(extFlash_func *statFctVal){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= 0x42;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Transmit(qspi_h, statFctVal, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * READ FUNCTION REGISTER OPERATION (RDFR, 48h)
 * The Read Function Register (RDFR) instruction provides access to the Function Register. Refer to Table 6.6
 * Function Register Bit Definition for more detail.
 *
 * @Parameter 		extFlash_func
 * @Return Value	falsh_err
 */
flash_err IS25mem_readFctReg(extFlash_func *fctReg){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDFR;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Receive(qspi_h, fctReg, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}



/**
 * WRITE ENABLE OPERATION (WREN, 06h)
	The Write Enable (WREN) instruction is used to set the Write Enable Latch (WEL) bit. The WEL bit is reset to the
	write-protected state after power-up. The WEL bit must be write enabled before any write operation, including
	Sector Erase, Block Erase, Chip Erase, Page Program, Write Status Register, and Write Function Register
	operations. The WEL bit will be reset to the write-protected state automatically upon completion of a write
	operation. The WREN instruction is required before any above operation is executed.
 */
flash_err IS25mem_writeEnable(void){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= WREN;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_NONE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.DdrHoldHalfCycle		= QSPI_DDR_HHC_ANALOG_DELAY;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * WRITE DISABLE OPERATION (WRDI, 04h)
 * The Write Disable (WRDI) instruction resets the WEL bit and disables all write instructions. The WRDI instruction
 * is not required after the execution of a write instruction, since the WEL bit is automatically reset.
 */
flash_err IS25mem_writeDisable(void){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= WRDI;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_NONE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.DdrHoldHalfCycle		= QSPI_DDR_HHC_ANALOG_DELAY;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}


flash_err IS25mem_pageProgramm(uint8_t *writeBuffer,mem_address address, uint8_t size){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= PP;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= size;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	if(HAL_QSPI_Transmit(qspi_h, writeBuffer, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	//@TODO at the moment a workaround.
	HAL_Delay(500);
//	if (IS25mem_AutoPollingMemReady() != MEMORY_OK) {
//		return MEMORY_ERROR;
//	}

	return MEMORY_OK;
}

/**
 * WRITE STATUS REGISTER OPERATION (WRSR, 01h)
 * The Write Status Register (WRSR) instruction allows the user to enable or disable the block protection and Status
 * Register write protection features by writing “0”s or “1”s into the non-volatile BP3, BP2, BP1, BP0, and SRWD
 * bits. Also WRSR instruction allows the user to disable or enable quad operation by writing “0” or “1” into the nonvolatile
 * QE bit.
 *
 * 	@Parameter 		extFlash_stat
 * 	@Return Value	flash_err
 */
flash_err IS25mem_writeStatReg(extFlash_stat *statRegVal){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= WRSR;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	if(HAL_QSPI_Transmit(qspi_h, statRegVal, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

/**
 * READ STATUS REGISTER OPERATION (RDSR, 05h)
 * The Read Status Register (RDSR) instruction provides access to the Status Register. During the execution of a
 * program, erase or write Status Register operation, all other instructions will be ignored except the RDSR
 * instruction, which can be used to check the progress or completion of an operation by reading the WIP bit of
 * Status Register.
 *
 * 	@Parameter 		extFlash_stat
 * 	@Return Value	flash_err
 */
flash_err IS25mem_readStatusReg(extFlash_stat *statReg){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDSR;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	HAL_StatusTypeDef spi_status = HAL_QSPI_Command(qspi_h, &memCmd, 100);
	if(spi_status != HAL_OK){
		return MEMORY_ERROR;
	}
	HAL_Delay(5);
	if(HAL_QSPI_Receive(qspi_h, statReg, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}

flash_err IS25mem_reset(){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RSTEN;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 1;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	memCmd.Instruction 			= RST;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;

}

flash_err IS25mem_sectorErase(mem_address address){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= 0xD7;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_NONE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	if (IS25mem_AutoPollingMemReady() != MEMORY_OK) {
		return MEMORY_ERROR;
	}

	IS25mem_registerCallback(pEraseDoneCallback);

	return MEMORY_OK;
}

/**
 * BLOCK ERASE OPERATION (BER32K:52h, BER64K:D8h)
 *
 * 	@Brief
 * 		A Block Erase (BER) instruction erases a 32/64Kbyte block. Before the execution of a BER instruction, the Write
 * 		Enable Latch (WEL) must be set via a Write Enable (WREN) instruction. The WEL is reset automatically after the
 * 		completion of a block erase operation.
 *
 * 	@Parameter 		mem_address
 * 	@Return Value	flash_err
 */
flash_err IS25mem_blockErase(mem_address address){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= BER64;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_NONE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= address.val;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	if (IS25mem_AutoPollingMemReady() != MEMORY_OK) {
		return MEMORY_ERROR;
	}

	IS25mem_registerCallback(pEraseDoneCallback);

	return MEMORY_OK;
}

/**
 * CHIP ERASE OPERATION (CER, C7h/60h)
 *
 * @Brief
 * 		A Chip Erase (CER) instruction erases the entire memory array. Before the execution of CER instruction, the Write
 * 		Enable Latch (WEL) must be set via a Write Enable (WREN) instruction. The WEL is reset automatically after
 * 		completion of a chip erase operation.
 * 	@Parameter 		mem_address
 * 	@Return Value	flash_err
 */
flash_err IS25mem_chipErase(mem_address address){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= 0xC7;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_NONE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	uint8_t testVal = HAL_QSPI_Command(qspi_h, &memCmd, 100);
	if(testVal != HAL_OK){
		return MEMORY_ERROR;
	}

	if (IS25mem_AutoPollingMemReady() != MEMORY_OK) {
		return MEMORY_ERROR;
	}

	IS25mem_registerCallback(pEraseDoneCallback);

	return MEMORY_OK;
}

/**
 * READ UNIQUE ID NUMBER (RDUID, 4Bh)
 *
 * @Brief
 * 		A Chip Erase (CER) instruction erases the entire memory array. Before the execution of CER instruction, the Write
 * 		Enable Latch (WEL) must be set via a Write Enable (WREN) instruction. The WEL is reset automatically after
 * 		completion of a chip erase operation.
 * 	@Parameter 		mem_address
 * 	@Return Value	flash_err
 */
flash_err IS25mem_readUid(uint8_t *UID){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDUID;
	memCmd.AddressMode 			= QSPI_ADDRESS_1_LINE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 8;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 16;

	if(HAL_QSPI_Command(qspi_h, &memCmd, 100) != HAL_OK){
		return MEMORY_ERROR;
	}

	if(HAL_QSPI_Receive(qspi_h, UID, 100) != HAL_OK){
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}

/**
 * IS25mem_AutoPollingMemReady(void)
 *
 * @Brief
 * 		Internal function so set up auto polling mode on the QSPI controller to check when memory becomes ready again.
 *
 * 	@Parameter 		(void)
 * 	@Return Value	flash_err
 */
flash_err IS25mem_AutoPollingMemReady(void){
	//Set QSPI CMD
	QSPI_CommandTypeDef memCmd	= {0};
	memCmd.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;
	memCmd.Instruction 			= RDSR;
	memCmd.AddressMode 			= QSPI_ADDRESS_NONE;
	memCmd.AddressSize 			= QSPI_ADDRESS_24_BITS;
	memCmd.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
	memCmd.DataMode 			= QSPI_DATA_1_LINE;
	memCmd.DummyCycles 			= 0;
	memCmd.Address 				= 0;
	memCmd.DdrMode 				= QSPI_DDR_MODE_DISABLE;
	memCmd.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD;
	memCmd.NbData 				= 0;

	QSPI_AutoPollingTypeDef s_config = {0};
	s_config.Match           = 0;
	s_config.Mask            = 0x01;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;


	if (HAL_QSPI_AutoPolling_IT(qspi_h, &memCmd, &s_config) != HAL_OK){
		return MEMORY_ERROR;
	}

	return MEMORY_OK;
}




