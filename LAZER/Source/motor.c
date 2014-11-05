
#include "motor.h"
#include "eeprom.h"
MOTOR_STATE motor[NUMBERS_MOTOR];


bool I2CWaitEventMotor( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
bool senser_pressed;

void motorInitGpio(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd( INT_PCA9539_1_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd( INT_PCA9539_2_RCC, ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
	GPIO_InitStructure.GPIO_Pin = INT_PCA9539_1_GPIO;
  GPIO_Init(INT_PCA9539_1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = INT_PCA9539_2_GPIO;
  GPIO_Init(INT_PCA9539_2_PORT, &GPIO_InitStructure);
	
  SYSCFG_EXTILineConfig(INT_PCA9539_1_PortSource, INT_PCA9539_1_PinSource);
  SYSCFG_EXTILineConfig(INT_PCA9539_2_PortSource, INT_PCA9539_2_PinSource);

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  
	EXTI_InitStructure.EXTI_Line = INT_PCA9539_1_IRQLine;
  EXTI_Init(&EXTI_InitStructure);

  EXTI_InitStructure.EXTI_Line = INT_PCA9539_2_IRQLine;
	EXTI_Init(&EXTI_InitStructure);
  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
	NVIC_InitStructure.NVIC_IRQChannel = INT_PCA9539_1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = INT_PCA9539_2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_Init(&NVIC_InitStructure);
	
}
void motorSenser(void)
{
	senser_pressed=true;
}	

void motorInit(void)
{	
	int i;
	for(i = 0 ; i < NUMBERS_MOTOR ; i++)
	{
			if ((i==0) || (i==1)) 	motor[i].address_i2c=ADDRES_DRIVER_0;
			else										motor[i].address_i2c=ADDRES_DRIVER_1;
			
			if ((i==0) || (i==2)) 	motor[i].port=0;				
			else										motor[i].port=1;
			
			if ((i==0) || (i==3))		motor[i].mask_enable=0xC0;
			else										motor[i].mask_enable=0x03;
			
			if ((i==0) || (i==3))		motor[i].mask_senser=0x03;
			else										motor[i].mask_senser=0xC0;
		
			motor[i].current_faza=0;
			motor[i].senser_open = false;
			motor[i].senser_close = false;
  		motorSendI2C(motor[i].address_i2c,CONFPORT+motor[i].port,motor[i].mask_senser);
			motorSendI2C(motor[i].address_i2c,OUTPORT+motor[i].port,0x00);
		
	}
}

int motorTurnOnPercent(int motor_number , int percent)
{
	int numbers_tick;
	if ((motor_number>(NUMBERS_MOTOR-1))||(motor_number<0)) return -1;
	if (motor[motor_number].max_count_tick!=0)
	{	
		if (percent>motor[motor_number].current_percent)
		{
			numbers_tick = (int)((motor[motor_number].max_count_tick*(percent-motor[motor_number].current_percent))/100);
			motorTurn(motor_number,CLOSE,numbers_tick,false);
		}
		else
		{
			numbers_tick = (int)((motor[motor_number].max_count_tick*(motor[motor_number].current_percent-percent))/100);
			motorTurn(motor_number,OPEN,numbers_tick,false);
		}
	}
	else return -1;
		
	return numbers_tick; 
}
int motorTurn(int number, bool direction,int tick ,bool turn_to_senser)
{

	int tick_numb = 0;
	int j = 0;
	uint8 value =  motor[number].mask_enable;
	uint8 address = motor[number].address_i2c;
	uint8 outport = OUTPORT+motor[number].port;
	bool work_motor = true;

	motorSendI2C(address,outport, value); //ENABLE
	
	while (work_motor)
	{
		if (!senser_pressed) 
		{
			switch (motor[number].current_faza)
			{
				case 0: 
					motor[number].current_faza=(direction) ?1 : 3; 
					value=(direction) ? FAZA1 : FAZA3;
				break;
				case 1: 
					motor[number].current_faza=(direction) ?2 : 0; 
					value=(direction) ? FAZA2 : FAZA0;
				break;
				case 2: 
					motor[number].current_faza=(direction) ?3 : 1; 
					value=(direction) ? FAZA3 : FAZA1;
				break;
				case 3: 
					motor[number].current_faza=(direction) ?0 : 2; 
					value=(direction) ? FAZA0 : FAZA2;
				break;
				default:
					motor[number].current_faza=0; 
					value= FAZA0;
				break;
			}
			value|=motor[number].mask_enable;
			motorSendI2C(address , outport , value);
			for (j=0;j<2000000;j++){};
			tick_numb++;
			if ((!turn_to_senser)&&(tick_numb==tick)) work_motor = false;
		}
		else 
		{
			senser_pressed = false;
			work_motor = false;
		}
		
	}
	motorSendI2C(address,outport,0x00); //DISABLE
	return tick_numb;
}

int open_hole(int motor_number)
{
	motorTurn(motor_number,OPEN,0,true);
}

int close_hole(int motor_number)
{
	motorTurn(motor_number,CLOSE,0,true);
}
	
void motorSettings(void)
{
	int motor_number;
	int mount_tick;
	for (motor_number = 0 ; motor_number<NUMBERS_MOTOR ; motor_number++)
	{
		open_hole(motor_number);
		mount_tick = close_hole(motor_number);
		mount_tick += open_hole(motor_number);
		motor[motor_number].max_count_tick=(int)(mount_tick/2);
		motor[motor_number].current_percent=0;
	}
}

void motorTest(void)
{
	int index;
	for (index = 0 ; index<NUMBERS_MOTOR ; index++)
	{
		motorTurn(index,true,1,false);
	}
}

/****************************************
* Name:Send message to motor driver 
* Desc: address cmd data 
*****************************************/
bool motorSendI2C(uint8 address, uint8 cmd ,uint8 data )
{
	/*
	//Generate START
  I2C_GenerateSTART(I2C, ENABLE);
	if(I2CWaitEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT)==EE_ERROR ) return EE_ERROR;
 	
	//Send Address
	I2C_Send7bitAddress(I2C, address, I2C_Direction_Transmitter); 
  if(I2CWaitEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==EE_ERROR ) return EE_ERROR;
  
	//Send command
	I2C_SendData(I2C,cmd);
  if(I2CWaitEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)==EE_ERROR ) return EE_ERROR;
  
	//Send data 8 bit
	I2C_SendData(I2C,data); 
  if(I2CWaitEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)==EE_ERROR ) return EE_ERROR;
  
	//Generate Stop
  I2C_GenerateSTOP(I2C, ENABLE);
	
	return 1;
	*/
	 /* Send STRAT condition */
	 
	I2C_GenerateSTART(I2C, ENABLE);

		
  /* Test on EV5 and clear it */
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT)){};
	
  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2C, address, I2C_Direction_Transmitter);
  
  /* Test on EV6 and clear it */
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){};
	
  /* Send the byte to be written */
  I2C_SendData(I2C,cmd); 
   
  /* Test on EV8 and clear it */
	
  while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){};
  
	 /* Send the byte to be written */
  I2C_SendData(I2C,data); 
   
  /* Test on EV8 and clear it */
	
  while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){};
 /* Send the byte to be written */
 // I2C_SendData(I2C,port1); 
   
  /* Test on EV8 and clear it */
	
 // while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){};
 
  /* Send STOP condition */
	I2C_GenerateSTOP(I2C, ENABLE);
	
	return NO_ERROR;
}

/****************************************
* Name:Recv message from motor driver 
* Desc: data - pointer on the data  
*****************************************/
bool motorRecvI2C(uint8 address, uint8 cmd ,uint8 *data )
{
	uint8 *read_data=data;
	//Generate start
	I2C_GenerateSTART(I2C, ENABLE);
	if(I2CWaitEventMotor(I2C, I2C_EVENT_MASTER_MODE_SELECT)==EE_ERROR ) return EE_ERROR;
 	
	//Send Address
	I2C_Send7bitAddress(I2C, address, I2C_Direction_Transmitter);
  if(I2CWaitEventMotor(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==EE_ERROR ) return EE_ERROR;
  
	//Send command
	I2C_SendData(I2C,cmd); 
  if(I2CWaitEventMotor(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)==EE_ERROR ) return EE_ERROR;
  
	//Read data 
	*read_data=I2C_ReceiveData(I2C); 
  if(I2CWaitEventMotor(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)==EE_ERROR ) return EE_ERROR;
  
	//Generate Stop
  I2C_GenerateSTOP(I2C, ENABLE);
	
	return 1;
}


/*******************************************************************************
* Function Name  : I2C_WaitEvent
* Description    : wait event from i2c
*******************************************************************************/
bool I2CWaitEventMotor( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	int eetime=WAYT_REQUEST_EEPROM;
	
  while(!I2C_CheckEvent(I2C, I2C_EVENT))
   {
    if((eetime--)==0) return EE_ERROR;
		
  }
	 
	return NO_ERROR;
	
}

