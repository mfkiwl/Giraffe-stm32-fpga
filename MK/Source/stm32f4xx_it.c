/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides all exceptions handler and peripherals interrupt
  *          service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "stdbool.h"
#include "fsmc_fpga.h"
#include "config.h"

extern void Timer2_ISR(void);

char bufer_console[SIZE_CONS_IN]; 
vu8 index_buf_cons;
extern SemaphoreHandle_t xMutexFSMC;
extern SemaphoreHandle_t xSemaphoreEXTI;
extern SemaphoreHandle_t xSemaphoreFSMCDMA;
extern SemaphoreHandle_t xSemaphoreSPIDMA;
extern SemaphoreHandle_t xSemaphoreCONSOLE;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/*************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    :  This function handles EXT NWAIT for FPGA .
*************************************************************/
void EXTI9_5_IRQHandler(void)
{
	long xHigherPriorityTaskWoken = pdFALSE;
	EXTI_ClearITPendingBit( EXTI_Line6 );

	xSemaphoreGiveFromISR( xSemaphoreEXTI, &xHigherPriorityTaskWoken );
	if( xHigherPriorityTaskWoken == pdTRUE )
  {
     portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}
/*********************************************************************
* Function Name  : DMA2_Stream0_IRQHandler
* Description    :  This function handles end translate data from fpga.
**********************************************************************/
void DMA2_Stream0_IRQHandler(void)
{
	long xHigherPriorityTaskWoken = pdFALSE;
  DMA_ClearITPendingBit(DMA_FSMC_STREAM,DMA_FSMC_IT_TCIF_CHECK);

  xSemaphoreGiveFromISR( xSemaphoreFSMCDMA, &xHigherPriorityTaskWoken );
	if( xHigherPriorityTaskWoken == pdTRUE )
  {
     portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}
/*********************************************************************
* Function Name  : USARTx_IRQHandler
* Description    : This function handles byte from host
**********************************************************************/

void USART1_IRQHandler(void)
{
	long xHigherPriorityTaskWoken = pdFALSE;
	char input_data; 
	
		USART_ClearITPendingBit(USART, USART_IT_RXNE);
		input_data= USART_ReceiveData(USART);
		USART_SendData(USART,input_data);
		if (index_buf_cons<SIZE_CONS_IN) 
		{
			bufer_console[index_buf_cons]=input_data;
			if (bufer_console[index_buf_cons]=='\r') 
			{
				xSemaphoreGiveFromISR( xSemaphoreCONSOLE, &xHigherPriorityTaskWoken );
				index_buf_cons=0;
			}
			else index_buf_cons++;
		}
		else xSemaphoreGiveFromISR( xSemaphoreCONSOLE, &xHigherPriorityTaskWoken );
		
		if( xHigherPriorityTaskWoken == pdTRUE )
		{
			 portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
		

	
}
/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		Timer2_ISR();
		
	}
}
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/*
void SVC_Handler(void)
{
}
*/
/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */

/*
void PendSV_Handler(void)
{
}
*/
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*
void SysTick_Handler(void)
{
 
}
*/


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
