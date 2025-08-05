/*

█▀▄▀█ ▄▀█ █▀ ▀█▀ █▀▀ █▀█   ▀█▀ █▀█ ▄▀█ █▄░█ █▀ █▀▄▀█ █ ▀█▀ ▀█▀ █▀▀ █▀█ ▄▄ █▀ █░░ ▄▀█ █░█ █▀▀   █▀█ █▀▀ █▀▀ █▀▀ █ █░█ █▀▀ █▀█
█░▀░█ █▀█ ▄█ ░█░ ██▄ █▀▄   ░█░ █▀▄ █▀█ █░▀█ ▄█ █░▀░█ █ ░█░ ░█░ ██▄ █▀▄ ░░ ▄█ █▄▄ █▀█ ▀▄▀ ██▄   █▀▄ ██▄ █▄▄ ██▄ █ ▀▄▀ ██▄ █▀▄

▄▀ █▀█ █▄░█ █▀▀   █▄▄ █▄█ ▀█▀ █▀▀ ▀▄
▀▄ █▄█ █░▀█ ██▄   █▄█ ░█░ ░█░ ██▄ ▄▀
*/
//indexes (pointers) for rx/tx buffers
volatile uint8_t txCounterI2C1 = 0;
volatile uint8_t txCounterI2C2 = 0;
volatile uint8_t rxCounterI2C1 = 0;
volatile uint8_t rxCounterI2C2 = 0;
//the buffers 
volatile unsigned char rxBufferI2C1[16];
volatile unsigned char rxBufferI2C2[16];
volatile unsigned char txBufferI2C1[16];
volatile unsigned char txBufferI2C2[16];
//the semaphore
volatile uint16_t states_I2C;
//when communication in progress - these values can be assigned to a semaphore
#define I2C1_PACKET 0x01
#define I2C2_PACKET 0x02

int MasterWriteOne(uint8_t addr, uint8_t *dataPtr)
{
	/*
	there should be set -
		 (1)STOP (set STOPIE ). The interrupt procedure clears the semaphore 'states_I2C'
	       and terminate while() loop.
	*/
	   //1) Add the delays SDADEL, SCLDEL in TIMINGR (optionally)
	   I2C2->TIMINGR |=  (0x0F << 16) |  //SDA delay
			             (0x0F << 20); //SCL delay
	    //2) Initialize global buffer address
	     i2c2TxDataPtr = dataPtr;
		//3) set auto-end, amount of data, hardware control (number of transmitted data)
		I2C2->CR2 |= (I2C_CR2_AUTOEND |  //auto end
				  (addr << 1) |  //address
				  (1 << 16) );  //one byte
		//4)Write data
		I2C2->TXDR = *dataPtr;
		//5) Start transmission, reset state semaphore firstly:
		states_I2C |= I2C2_PACKET;
		//6)Start transaction
		I2C2->CR2 |= I2C_CR2_START;
		//7)Waiting: the semaphore will has been cleared in the ISR (STOP flag)
		while (states_I2C &  I2C2_PACKET );

		if (I2C2->ISR &  I2C_ISR_NACKF) {
			//when NACK = return  error
			return -1;
		} else {
			return 0;
		}
}

void slaveRxOneInit(uint8_t address, uint8_t* buffer) {
 /**NOTE: this should be done before:
	     (1)Load own address into  OAR1 and set here OA1EN
		 (2)Enable RXNE interrupt.The incoming data is copied into a buffer by the pointer. Increment the pointer.
		 (3)Enable STOP interrupt. Clear the pointer to 0.
   		 (4)Enable ADDR interrupt (set ADDRIE).Clear this flag in the interrupt
		     with the ADDRCF flag in the ICR.
		 **/
	//1) Init RAM buffer for slave
  i2c1RxDataPtr = buffer;
    //2)assign address
  I2C1->OAR1 |= (address << 1) | I2C_OAR1_OA1EN;
  ///all the work to receive data be done in the ISR

}

//*****************************interrupts***************************
void I2C2_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_IRQn 0 */
///means-------- M A S T E R --------

	if (I2C2->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C2->ICR |= I2C_ICR_STOPCF;
	        // Set your flag/semaphore
	        states_I2C &= ~I2C2_PACKET;
	    }

  /* USER CODE END I2C2_IRQn 1 */
}

void I2C1_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_IRQn 0 */
   ///means --------S L A V E-----------
	if (I2C1->ISR & I2C_ISR_RXNE) {
		//read data into a buffer
		i2c1RxDataPtr[rxCounterI2C1++] = I2C1->RXDR;
	}

	if (I2C1->ISR & I2C_ISR_ADDR){
		//clear address match. Without processing this interrupt the I2C get in stuck (SCL-low, SDA-high)
		I2C1->ICR = I2C_ICR_ADDRCF;
	}

	if (I2C1->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C1->ICR |= I2C_ICR_STOPCF;
            //clear index of the pointer
	        rxCounterI2C1 = 0;
	        // Set your flag/semaphore.Optional.Not needs here.
	        states_I2C &= ~I2C1_PACKET;
	    }

  /* USER CODE END I2C1_IRQn 1 */
}

/*
█▀▄▀█ ▄▀█ █▀ ▀█▀ █▀▀ █▀█   ▀█▀ █▀█ ▄▀█ █▄░█ █▀ █▀▄▀█ █ ▀█▀ ▀█▀ █▀▀ █▀█ ▄▄ █▀ █░░ ▄▀█ █░█ █▀▀   █▀█ █▀▀ █▀▀ █▀▀ █ █░█ █▀▀ █▀█
█░▀░█ █▀█ ▄█ ░█░ ██▄ █▀▄   ░█░ █▀▄ █▀█ █░▀█ ▄█ █░▀░█ █ ░█░ ░█░ ██▄ █▀▄ ░░ ▄█ █▄▄ █▀█ ▀▄▀ ██▄   █▀▄ ██▄ █▄▄ ██▄ █ ▀▄▀ ██▄ █▀▄

▄▀ █▀█ ▄▀█ █▀▀ █▄▀ █▀▀ ▀█▀ ▀▄
▀▄ █▀▀ █▀█ █▄▄ █░█ ██▄ ░█░ ▄▀
*/
int MasterWritePacket(uint8_t addr, uint8_t size, uint8_t* dataPtr)
{
	/*
	 the next interrupts should be enabled:
	 (1)STOP (set STOPIE ). The interrupt procedure clears the semaphore 'states_I2C'
	       and terminate while() loop.
	  (2)TXIS (set TXIE).This interrupt appears when numBytes > 1.
	       So the last transmitted byte NOT call the interrupt.
	 * */
	/*
	 INTERRUPTS:
	   TXIS: write new data into the TXDR register using the pointer, increment this pointer.
	   STOP: clear the pointer to 0, clear the semaphore
	 * */

    i2c2TxDataPtr = dataPtr;
    txCounterI2C2 = 0;

    // Set CR2 with correct size
    I2C2->CR2 = (I2C2->CR2 & ~(I2C_CR2_SADD | I2C_CR2_NBYTES)) |
                (addr << 1) |
                (size << 16) |
                I2C_CR2_AUTOEND |
                I2C_CR2_START;

    states_I2C |= I2C2_PACKET;

    // Wait until STOPF clears the packet semaphore
    while (states_I2C & I2C2_PACKET);

    if (I2C2->ISR & I2C_ISR_NACKF)
        return -1;
    else
        return 0;
}




void slaveRxOneInit(uint8_t address, uint8_t* buffer) {
 /**NOTE: there should be done before the following:
	     (1)Load own address into  OAR1 and set here OA!EN
		 (2)Enable RXNE interrupt.There incoming data copy into a buffer by the pointer.Increment the pointer.
		 (3)Enable STOP interrupt. Clear the pointer to 0.
   		 (4)Enable ADDR interrupt (set ADDRIE).Clear this flag in the interrupt
		     with the ADDRCF flag in the ICR.
		 **/
	//1) Init RAM buffer for slave
  i2c1RxDataPtr = buffer;
    //2)assign address
  I2C1->OAR1 |= (address << 1) | I2C_OAR1_OA1EN;
  ///all the work to receive data be done in the ISR

}

//---------------interrupts--------------------------------------
void I2C1_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_IRQn 0 */
   ///means --------S L A V E-----------
	if (I2C1->ISR & I2C_ISR_RXNE) {

		//read data into a buffer
		i2c1RxDataPtr[rxCounterI2C1++] = I2C1->RXDR;

	}

	if (I2C1->ISR & I2C_ISR_ADDR){
		//clear address match
		I2C1->ICR = I2C_ICR_ADDRCF;
	}

	if (I2C1->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C1->ICR |= I2C_ICR_STOPCF;
            //clear index of the pointer
	        rxCounterI2C1 = 0;
	        // Set your flag/semaphore (optional, not need herre)
	        states_I2C &= ~I2C1_PACKET;
	    }

  /* USER CODE END I2C1_IRQn 1 */
}

/**
  * @brief This function handles I2C2 global interrupt.
  */
void I2C2_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_IRQn 0 */
///means-------- M A S T E R --------

	if (I2C2->ISR & I2C_ISR_TXIS) {
	    I2C2->TXDR = i2c2TxDataPtr[txCounterI2C2++];
	}

	if (I2C2->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C2->ICR |= I2C_ICR_STOPCF;
            //clear index of the pointer
	        txCounterI2C2 = 0;
	        // Set your flag/semaphore
	        states_I2C &= ~I2C2_PACKET;
	    }

  /* USER CODE END I2C2_IRQn 1 */
}


/*****99999999999999999999999999999999***/
int MasterReadOne(uint8_t addr, uint8_t *dataPtr)
{
	/*
	there should be set -
	(1)interrupts STOP (set STOPIE) ,
	     clear the semaphore 'states_I2C' in this interrupt handler
	(2)Interrupts RXNE. In the interrupts a new data stores into buffer
	 ******
	 INTERRUPT NOTE:
	 S T O P:  clear index of the array to 0, clear the semaphore
	 R X N E:  read new data into array by index
	*/
	   //1) Enable hardware slave control
	    I2C2->CR1 |= I2C_CR1_SBC;
	   //1) Add the delays SDADEL, SCLDEL in TIMINGR (optionally)
	   I2C2->TIMINGR |=  (0x0F << 16) |  //SDA delay
			             (0x0F << 20); //SCL delay
	    //2) Initialize global buffer address
	     i2c2RxDataPtr = dataPtr;
		//3) set auto-end, amount of data, hardware control (number of transmitted data)
		I2C2->CR2 |= (I2C_CR2_AUTOEND |  //auto end
				  (addr << 1) |  //address
				  (1 << 10) |   //  R E A D   operation
				  (1 << 16) );  //one byte
		//NOTE: In RXNE interrupt data stores in array, index++;
		//      In STOP index = 0
		//5) Start transmission, reset state semaphore firstly:
		states_I2C |= I2C2_PACKET;
		//6)Start transaction
		I2C2->CR2 |= I2C_CR2_START;
		//7)Waiting: the semaphore will has been cleared in the ISR (STOP flag)
		while (states_I2C &  I2C2_PACKET );

		if (I2C2->ISR &  I2C_ISR_NACKF) {
			//when NACK = return  error
			return -1;
		} else {
			return 0;
		}

}

void slaveTxOneInit(uint8_t address, uint8_t* buffer) {
 /**NOTE: there should be done before the following:
	     (1)Load own address into  OAR1 and set here OA!EN
		 (2)STOP (set STOPIE ). The interrupt procedure clears the semaphore 'states_I2C'
	       and terminate while() loop.
	     (3) TXIS (set TXIE).This interrupt appears when numBytes > 1.
	       So the last transmitted byte NOT call the interrupt.
	     (4) ADDR (set (ADDRIE).Load the first byte here. Clear this bit in the interrupt by the ADDRCF flag
	     ****************
	     INTERRUPTS:
	     S T O P: clears the semaphore 'states_I2C'.Clears index to 0
	     T X I S: write new data from the buffer-array, by the index, after this index++;
		 **/
	//1) Init RAM buffer for slave
  i2c1TxDataPtr = buffer;
    //2)assign address
  I2C1->OAR1 |= (address << 1) | I2C_OAR1_OA1EN;
  ///all the work to receive data be done in the ISR

}

/*NOTE!! The first byte must be written before transmission manually! Code has a bug - because in ADDR 
  Interrupt of slave transmitter data doesn`t used (slave got data after this interrupt ***/
int MasterReadPacket(uint8_t addr, uint8_t size, uint8_t* dataPtr)
{
	/*
	there should be set -
	(1)interrupts STOP (set STOPIE) ,
	     clear the semaphore 'states_I2C' in this interrupt handler
	(2)Interrupts RXNE. In the interrupts a new data stores into buffer
	 ******
	 INTERRUPT NOTE:
	 S T O P:  clear index of the array to 0, clear the semaphore
	 R X N E:  read new data into array by index
	*/
	//clear error flags
	I2C2->ICR = I2C_ICR_NACKCF |  I2C_ICR_BERRCF |  I2C_ICR_ARLOCF;

	   //1) Enable hardware slave control
	    I2C2->CR1 |= I2C_CR1_SBC;
	   //1) Add the delays SDADEL, SCLDEL in TIMINGR (optionally)
	   I2C2->TIMINGR |=  (0x0F << 16) |  //SDA delay
			             (0x0F << 20); //SCL delay
	    //2) Initialize global buffer address
	     i2c2RxDataPtr = dataPtr;
		//3) set auto-end, amount of data, hardware control (number of transmitted data)
		I2C2->CR2 |= (I2C_CR2_AUTOEND |  //auto end
				  (addr << 1) |  //address
				  (1 << 10) |   //  R E A D   operation
				  (size << 16) );  //amount of data
		//NOTE: In RXNE interrupt data stores in array, index++;
		//      In STOP index = 0
		//5) Start transmission, reset state semaphore firstly:
		states_I2C |= I2C2_PACKET;
		//6)Start transaction
		I2C2->CR2 |= I2C_CR2_START;
		//7)Waiting: the semaphore will has been cleared in the ISR (STOP flag)
		while (states_I2C &  I2C2_PACKET );


    if (I2C2->ISR & (I2C_ISR_NACKF |  I2C_ISR_BERR| I2C_ISR_ARLO ))
        return -1;
    else
        return 0;
}

//--------------interrupts-------------------------
void I2C1_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_IRQn 0 */
   ///------T X   m u s t  b e   w r i t t e n-----
	if (I2C1->ISR & I2C_ISR_TXIS) {
		//read data from a buffer into the transmitter
		I2C1->TXDR = i2c1TxDataPtr[txCounterI2C1++];
	}
     //----a d d r e s s---m a t c h---
	if (I2C1->ISR & I2C_ISR_ADDR) {
		//load the first byte
		I2C1->TXDR = i2c1TxDataPtr[txCounterI2C1++];
		//clear address match
		I2C1->ICR = I2C_ICR_ADDRCF;
	}
   ///---------s t o p-------
	if (I2C1->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C1->ICR |= I2C_ICR_STOPCF;
            //clear index of the pointer
	        txCounterI2C1 = 0;
	        // Set your flag/semaphore
	        states_I2C &= ~I2C1_PACKET;
	 }


  /* USER CODE END I2C1_IRQn 1 */
}

/**
  * @brief This function handles I2C2 global interrupt.
  */
void I2C2_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_IRQn 0 */
///means-------- M A S T E R --------

    //---------RX not empty
	if (I2C2->ISR & I2C_ISR_RXNE) {
	      i2c2RxDataPtr[rxCounterI2C2++] = I2C2->RXDR;
	}

	if (I2C2->ISR & I2C_ISR_STOPF) {
	        // Clear STOPF
	        I2C2->ICR |= I2C_ICR_STOPCF;
            //clear index of the pointer
	        rxCounterI2C2 = 0;
	        // Set your flag/semaphore
	        states_I2C &= ~I2C2_PACKET;
	 }

  /* USER CODE END I2C2_IRQn 1 */
}





