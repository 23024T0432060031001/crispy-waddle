#include "sys.h"
#include "usart.h"
#include "stdio.h"
#include "delay.h"
#include "lcd.h"

u8 USART1_RX_BUFF0[1024];
u8 USART1_RX_BUFF1[1024];

void uart_init(u32 baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
 
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 and GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

  
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//WordLength: 8bits
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//StopBit number: 1
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
	
	USART1_DMA_Init((u32)USART1_RX_BUFF0, (u32)USART1_RX_BUFF1);
	
	
	
  USART_Cmd(USART1, ENABLE);
}


void USART1_DMA_Init(u32 DMA_Memory0BaseAddr, u32 DMA_Memory1BaseAddr)
{ 
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	DMA_DeInit(DMA2_Stream2);
	while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//Peripheral pointer no increase
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//Peripheral size: Byte
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	
	DMA_InitStructure.DMA_Memory0BaseAddr = DMA_Memory0BaseAddr;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//Memory pointer increase
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;	//Memory size: Byte
	
	DMA_InitStructure.DMA_BufferSize = 1024;	//counter
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//Circular Mode
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable; //FIFO enable  
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;//Full FIFO 
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC16;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
		
	//Double Buffer Mode
	DMA_MemoryTargetConfig(DMA2_Stream2, DMA_Memory1BaseAddr, DMA_Memory_1);
	DMA_DoubleBufferModeCmd(DMA2_Stream2,ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	
	DMA_Cmd(DMA2_Stream2, ENABLE);

}

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif

