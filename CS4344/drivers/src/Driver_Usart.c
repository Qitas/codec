#include "stm32f10x.h"
#include "Hapc_BSP.h"
#include <stdarg.h>

u8 UsartRecData[USART_RECDATAMAX]		= {0};
u8 UsartRecFinish 				=  0;


void USART_Conf(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStruct;		//����ṹ�����
  // Configure USART1 /
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  //USART1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  // Configure USART1_Rx as input floating 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_InitStruct.USART_BaudRate = 115200;		//���ò�����
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//����Ӳ��������
  USART_InitStruct.USART_Mode =  USART_Mode_Rx |USART_Mode_Tx;	//����ģʽΪ���ܻ���
  USART_InitStruct.USART_Parity = USART_Parity_No;	//��������żУ��λ
  USART_InitStruct.USART_StopBits = USART_StopBits_1;	//ֹͣλ
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;	//�����ֳ�
  USART_Init(USART1, &USART_InitStruct);
 
  // ʹ�� USART1 �����жϣ��� USART1 �������ݼĴ����ǿգ��������ж� 
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  // Enable USART1 /
  USART_Cmd(USART1, ENABLE);
}

void Usart_Puts(const char * str)
{
  while(*str){
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ;//�ȴ��������
    USART_SendData(USART1,*str);
    str++;
  }
}

void Usart_TransmitData(char * str , u32 len)
{
	while(len--)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ;//�ȴ��������
		USART_SendData(USART1,*str);
		str ++ ;
	}
}

int SendChar (int ch)
{                /* Write character to Serial Port     */

  USART_SendData(USART1, (unsigned char) ch);
  while (!(USART1->SR & USART_FLAG_TXE));
  return (ch);
}
/*----------------------------------------------------------------------------
  GetKey
  Read character to Serial Port.
 *----------------------------------------------------------------------------*/
int GetKey (void)  
{
  while (!(USART1->SR & USART_FLAG_RXNE));
  return ((int)(USART1->DR & 0x1FF));
}


/******************************************************
		��ʽ�������������
        "\r"	�س���	   USART_OUT(USART1, "abcdefg\r")   
		"\n"	���з�	   USART_OUT(USART1, "abcdefg\r\n")
		"%s"	�ַ���	   USART_OUT(USART1, "�ַ����ǣ�%s","abcdefg")
		"%d"	ʮ����	   USART_OUT(USART1, "a=%d",10)
**********************************************************/
void PrintfU(USART_TypeDef* USARTx, uint8_t *Data,...)
{ 

	const char *s;
    int d;
   
    char buf[16];
    va_list ap;
    va_start(ap, Data);

	while(*Data!=0){				                          //�ж��Ƿ񵽴��ַ���������
		if(*Data==0x5c){									  //'\'
			switch (*++Data){
				case 'r':							          //�س���
					USART_SendData(USARTx, 0x0d);	   

					Data++;
					break;
				case 'n':							          //���з�
					USART_SendData(USARTx, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				    break;
			}
			
			 
		}
		else if(*Data=='%'){									  //
			switch (*++Data){				
				case 's':										  //�ַ���
                	s = va_arg(ap, const char *);
                	for ( ; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
            	case 'd':										  //ʮ����
                	d = va_arg(ap, int);
                	itoa(d, buf, 10);
                	for (s = buf; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				default:
					Data++;
				    break;
			}		 
		}
		else USART_SendData(USARTx, *Data++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}

/******************************************************
		��������ת�ַ�������
        char *itoa(int value, char *string, int radix)
		radix=10 ��ʾ��10����	��ʮ���ƣ�ת�����Ϊ0;  

	    ����d=-379;
		ִ��	itoa(d, buf, 10); ��
		
		buf="-379"							   			  
**********************************************************/
char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */
