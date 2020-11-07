/*
 * 		Created on: 01.04.2020
 *      Author: Fabian Niehaus
 *      Mail:	fabian.niehaus@tuhh.de
 *
 *      STM32 flash memory driver - IS25LQXXXB
 *
 */

#ifndef INC_IS25LQ040B_EXT_MEM_H_
#define INC_IS25LQ040B_EXT_MEM_H_

#include "stm32l4xx_hal.h"

/**
 * 						ISSI flash memory instruction set
 */

#define RD 						0x03						// Read Data Bytes from Memory at Normal Read Mode
#define FR 						0x0B						// Read Data Bytes from Memory at Fast Read Mode
#define FRDIO					0xBB						// Fast Read Dual I/O
#define FRDO					0x3B						// Fast Read Dual Output
#define FRQIO					0xEB						// Fast Read Quad I/O
#define FRQO					0x6B						// Fast Read Quad Output
#define PP						0x02						// Single page program operation
#define PPQ						0x38						// Quad input page program operation
#define SER						0x20						// Sector erase 4Kb
#define BER32					0x52						// Block erase 32Kb
#define BER64					0xD8						// Block erase 64Kb
#define CER						0x60						// Chip Erase
#define WREN					0x06						// Write Enable
#define WRDI					0x04						// Write Disable
#define RDSR					0x05						// Read Status Register
#define WRSR					0x01						// Write Status Register
#define RDFR					0x48						// Read Function Register
#define WRFR					0x42						// Write Function Register
#define PERSUS					0xB0						// Suspend during the Program/Erase
#define PERRSM					0x30						// Resume Program/Erase
#define DP						0xB9						// Deep Power Down Mode
#define RDID					0xAB						// Read Manufacturer and Product ID/Release Deep Power Down
#define RDPD					0xAB						// Read Unique ID Number
#define RDUID					0x4B						// Read Manufacturer and Product ID by JEDEC ID Command
#define RDJDID					0x9F						// Read Manufacturer and Device ID
#define RDMDID					0x90						// SFDP Read
#define RDSFDP					0x5A						// Software Reset Enable
#define RSTEN					0x66						// Software Reset Enable
#define RST						0x99						// Reset
#define IRP						0x62						// Program Information Row
#define IRRD					0x68						// Read Information Row
#define SECUNLOCK				0x26						// Sector Unlock
#define SECLOCK					0x24						// Sector Lock

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
  * @brief  MEMORY status structures definition
  */
typedef enum
{
  MEMORY_OK    				= 0x00,
  MEMORY_ERROR    			= 0x01,
  MEMORY_BUSY     			= 0x02,
  MEMORY_TIMEOUT  			= 0x03,
  MEMORY_WRONG_CPACITY_ERR 	= 0x04
} flash_err;



/* External Flash Memory Status Register
 *
 *BIT 	7   6	5	4	3	2	1	0
 * 	 |SRWD|QE |BP3|BP2|BP1|BP0|WEL|WIP|
 */
typedef uint8_t extFlash_stat;

#define WIP						( extFlash_stat & 0x01 )	//<- Write in Progress "0" (default) device is ready / "1" write cycle in progress
#define WEL						( extFlash_stat & 0x02 )	//<- Write Enable Latch "0" (default) device is not write enable
#define BP0						( extFlash_stat & 0x04 )	//<- Block Protection Bit "0" - not write protected
#define BP1						( extFlash_stat & 0x08 )	//<- Block Protection Bit "0" - not write protected
#define BP2						( extFlash_stat & 0x10 )	//<- Block Protection Bit "0" - not write protected
#define BP3						( extFlash_stat & 0x20 )	//<- Block Protection Bit "0" - not write protected
#define QE						( extFlash_stat & 0x40 )	//<- Quad Enable Bit "0" - Quad output function disable (default)
#define SRWD					( extFlash_stat & 0x80 )	//<- Status Register Write Disable "0" - Status Register not write-protected (default)

#define getBlockWriteProtectionBits(status)			(status & 0x60)
/**
 * 	Block Write Protection
 */
typedef uint8_t flash_bwp_t;

#define NO_B_PROTECTED			0x00		// No Blocks Protected

#define BU1_PROTECTED			0x01		// Block 7 	 Protected
#define BU2_PROTECTED			0x02		// Block 6-7 Protected
#define BU4_PROTECTED			0x04		// Block 4-7 Protected

#define BL4_PROTECTED			0x40		// Block 0-3 Protected
#define BL2_PROTECTED			0x20		// Block 0-1 Protected
#define BL1_PROTECTED			0x10		// Block 0	 Protected

#define ALL_B_PROTECTED			0xFF		// All Blocks Protected


/* External Flash Memory Function Register
 *
 * BIT		7		 6			5		4		3	 2		1		0
 * 		|IR Lock3|IR Lock2|IR Lock1|IR Lock0| ESUS| PSUS|	R	|	R	|	R=Reserved
 * 			 |		 |			|		|		|	 |-> 	Program suspend bit
 * 			 |		 |			|		|		|------>	Erase suspend bit
 * 			 |		 |			|		|--------------> 	Lock the Information Row 0
 * 			 |		 |			|----------------------> 	Lock the Information Row 1
 * 			 |		 |---------------------------------> 	Lock the Information Row 2
 * 			 |-----------------------------------------> 	Lock the Information Row 3
 */
typedef uint8_t	extFlash_func;

/**
 * Manufacturer ID 		|									(MF7-MF0)									|
 * ISSI Serial Flash 	| 									   9Dh										|
 * Instruction 			|		ABh			|		90h			|					9Fh					|
 * Device 				| 		Density Device ID (ID7-ID0)		|	Device Type + Capacity (ID15-ID0)	|
 * 4Mb 					|					12h 				|					4013h				|
 * 2Mb 					|					11h 				|					4012h				|
 * 1Mb 					|					10h 				|					4011h				|
 * 512K 				|					05h 				|					4010h				|
 * 256K 				|					02h 				|					4009h				|
 *
 */
typedef uint8_t IS25mem_ManfID;
typedef uint8_t IS25mem_deviceID;
typedef uint8_t IS25mem_Capacity;

typedef struct{
	IS25mem_ManfID		ManufacturerID;
	IS25mem_deviceID	DeviceType;
	IS25mem_Capacity	Capacity;
}IS25mem_Identification;

typedef union{
	struct{
		uint32_t	sectorBytes:12;		// Bytes inside a sector
		uint32_t	sector:8;			// Memory Sector	(4  kByte)
		uint32_t 	undefined:4;		// Not used
		};
	uint32_t val;				// address value
}mem_address;

typedef struct{
	uint8_t		blocks64;
	uint8_t		blocks32;
	uint8_t		sectors;
}IS25mem_MemorySpace;


//External function declaration

extern void (*autoPollingCallback)(void);

//Functions to register the Callbacks.
extern void setEraseDoneCallbackFct(void (*fct));

extern flash_err IS25mem_Init(QSPI_HandleTypeDef *qSPIHandler);
extern flash_err IS25mem_reset(void);
extern flash_err IS25mem_writeEnable(void);
extern flash_err IS25mem_sectorErase(mem_address address);
extern flash_err IS25mem_readID(IS25mem_deviceID *id);
extern flash_err IS25mem_readProductId(IS25mem_Identification *productId);
extern flash_err IS25mem_readUid(uint8_t *UID);
extern flash_err IS25mem_AutoPollingMemReady(void);
extern flash_err IS25mem_writeFctReg(extFlash_func *statFctVal);
extern flash_err IS25mem_readFctReg(extFlash_func *fctReg);
extern flash_err IS25mem_readStatusReg(extFlash_stat *statReg);
extern flash_err IS25mem_writeStatReg(extFlash_stat *statRegVal);
extern flash_err IS25mem_readData(uint8_t *readBuffer,mem_address address, uint16_t size);
extern flash_err IS25mem_fastReadData(uint8_t *readBuffer,mem_address address, uint16_t size);
extern flash_err IS25mem_QuadFastReadData(uint8_t *readBuffer,mem_address address, uint8_t size);
extern flash_err IS25mem_pageProgramm(uint8_t *writeBuffer,mem_address address, uint8_t size);
extern flash_err IS25mem_chipErase(mem_address address);

#endif /* INC_IS25LQ040B_EXT_MEM_H_ */
