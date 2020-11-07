# IS25LQXXX_Memory
C code driver for the ISSI-IS25LQXXXB SPI memory. 

# Usage

Add: 
```c
IS25mem_Init(&hqspi)
```
in <b>main.c</b> to make sure driver can use the qspi handler.

Add: 
```c
/* USER CODE BEGIN 4 */
//Flashmemory Autopolling Match Callback
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi){
	if(autoPollingCallback != 0){
		autoPollingCallback();
		autoPollingCallback = 0;
	}else{
		printf("NO REGISTERD CALLBACK\n");
	}
}
/* USER CODE END 4 */
```

If you want to call a user specific function after a erase process is done. Register the function via: 
```c
setEraseDoneCallbackFct(&Userfunction)
```

# Functions

```c
extern void (*autoPollingCallback)(void);

//Functions to register the Callback fct for users erase done action.
void setEraseDoneCallbackFct(void (*fct));

flash_err IS25mem_Init(QSPI_HandleTypeDef *qSPIHandler);
flash_err IS25mem_reset(void);
flash_err IS25mem_writeEnable(void);
flash_err IS25mem_sectorErase(mem_address address);
flash_err IS25mem_readID(IS25mem_deviceID *id);
flash_err IS25mem_readProductId(IS25mem_Identification *productId);
flash_err IS25mem_readUid(uint8_t *UID);
flash_err IS25mem_AutoPollingMemReady(void);
flash_err IS25mem_writeFctReg(extFlash_func *statFctVal);
flash_err IS25mem_readFctReg(extFlash_func *fctReg);
flash_err IS25mem_readStatusReg(extFlash_stat *statReg);
flash_err IS25mem_writeStatReg(extFlash_stat *statRegVal);
flash_err IS25mem_readData(uint8_t *readBuffer,mem_address address, uint16_t size);
flash_err IS25mem_fastReadData(uint8_t *readBuffer,mem_address address, uint16_t size);
flash_err IS25mem_QuadFastReadData(uint8_t *readBuffer,mem_address address, uint8_t size);
flash_err IS25mem_pageProgramm(uint8_t *writeBuffer,mem_address address, uint8_t size);
flash_err IS25mem_chipErase(mem_address address);
```

