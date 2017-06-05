#include "W25QXX.h"

u16 W25QXX_TYPE = W25Q32;	//Ĭ����W25Q32

//��ʼ��SPI FLASH��IO��
W25QXX::W25QXX(SPI& SPIx, GPIO_TypeDef* CSPortx, u16 CSPinx) :mSPI(SPIx)
{
	mCSPort = CSPortx;
	mCSPin = CSPinx;
	GPIO_InitTypeDef  GPIO_InitStructure;
	if (mCSPort == GPIOA)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
	if (mCSPort == GPIOB)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
	if (mCSPort == GPIOC)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
	if (mCSPort == GPIOD)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//ʹ��GPIODʱ��
	if (mCSPort == GPIOE)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOEʱ��
	if (mCSPort == GPIOF)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��

	GPIO_InitStructure.GPIO_Pin = mCSPin;									//PB0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;					//���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;					//����
	GPIO_Init(mCSPort, &GPIO_InitStructure);							//��ʼ��

	GPIO_SetBits(mCSPort, mCSPin);
	mSPI.SetSpeed(SPI_BaudRatePrescaler_2);
	W25QXX_TYPE = W25QXX_ReadID();												//��ȡFLASH ID.
}

//��ȡW25QXX��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 W25QXX::W25QXX_ReadSR(void)
{
	u8 byte = 0;
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   
	mSPI.SPI_RW(W25X_ReadStatusReg); 				 //���Ͷ�ȡ״̬�Ĵ�������  
	byte = mSPI.SPI_RW(0xFF);         			 //��ȡһ���ֽ� 
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ  
	return byte;
}
//дW25QXX״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void W25QXX::W25QXX_Write_SR(u8 sr)
{
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   
	mSPI.SPI_RW(W25X_WriteStatusReg);  			 //����дȡ״̬�Ĵ�������    
	mSPI.SPI_RW(sr);               					 //д��һ���ֽ�  
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ  
}
//W25QXXдʹ��	
//��WEL��λ   
void W25QXX::W25QXX_Write_Enable(void)
{
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   
	mSPI.SPI_RW(W25X_WriteEnable);     			 //����дʹ��
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ  	      
}
//W25QXXд��ֹ	
//��WEL����  
void W25QXX::W25QXX_Write_Disable(void)
{
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   
	mSPI.SPI_RW(W25X_WriteDisable);    			 //����д��ָֹ��
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ    	      
}
//��ȡоƬID
//����ֵ����:				   
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
u16 W25QXX::W25QXX_ReadID(void)
{
	u16 Temp = 0;
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   			    
	mSPI.SPI_RW(0x90);//���Ͷ�ȡID����	    
	mSPI.SPI_RW(0x00);
	mSPI.SPI_RW(0x00);
	mSPI.SPI_RW(0x00);
	Temp |= mSPI.SPI_RW(0xFF) << 8;
	Temp |= mSPI.SPI_RW(0xFF);
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ  			    
	return Temp;
}
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void W25QXX::W25QXX_Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������   
	mSPI.SPI_RW(W25X_ReadData);         		 //���Ͷ�ȡ����   
	mSPI.SPI_RW((u8)((ReadAddr) >> 16)); 		 //����24bit��ַ    
	mSPI.SPI_RW((u8)((ReadAddr) >> 8));
	mSPI.SPI_RW((u8)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = mSPI.SPI_RW(0XFF);  			 //ѭ������  
	}
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ  				    	      
}
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void W25QXX::W25QXX_Write_Page(const u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i;
	W25QXX_Write_Enable();                  //SET WEL
	GPIO_ResetBits(mCSPort, mCSPin);        //ʹ������  
	mSPI.SPI_RW(W25X_PageProgram);      		//����дҳ����   
	mSPI.SPI_RW((u8)((WriteAddr) >> 16));   //����24bit��ַ
	mSPI.SPI_RW((u8)((WriteAddr) >> 8));
	mSPI.SPI_RW((u8)WriteAddr);
	for (i = 0; i < NumByteToWrite; i++)
		mSPI.SPI_RW(pBuffer[i]);							//ѭ��д��
	GPIO_SetBits(mCSPort, mCSPin);					//ȡ��Ƭѡ  	
	W25QXX_Wait_Busy();					  				  //�ȴ�д�����
}
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void W25QXX::W25QXX_Write_NoCheck(const u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 pageremain;
	pageremain = 256 - WriteAddr % 256;							//��ҳʣ����ֽ���		 	    
	if (NumByteToWrite <= pageremain)
		pageremain = NumByteToWrite;									//������256���ֽ�
	while (1)
	{
		W25QXX_Write_Page(pBuffer, WriteAddr, pageremain);
		if (NumByteToWrite == pageremain)
			break;											 								//д�������
		else																				  //NumByteToWrite>pageremain
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;
			NumByteToWrite -= pageremain;			  				//��ȥ�Ѿ�д���˵��ֽ���
			if (NumByteToWrite > 256)
				pageremain = 256; 											  //һ�ο���д��256���ֽ�
			else
				pageremain = NumByteToWrite; 	 			 		  //����256���ֽ���
		}
	};
}
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535)   
u8 W25QXX_BUFFER[4096];
void W25QXX::W25QXX_Write(const u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u32 secpos;
	u16 secoff;
	u16 secremain;
	u16 i;
	u8 * W25QXX_BUF;
	W25QXX_BUF = W25QXX_BUFFER;
	secpos = WriteAddr / 4096;													//������ַ  
	secoff = WriteAddr % 4096;													//�������ڵ�ƫ��
	secremain = 4096 - secoff;													//����ʣ��ռ��С   
	if (NumByteToWrite <= secremain)
		secremain = NumByteToWrite;												//������4096���ֽ�
	while (1)
	{
		W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096);								 //������������������
		for (i = 0; i < secremain; i++)															 //У������
		{
			if (W25QXX_BUF[secoff + i] != 0XFF)
				break;									 																 //��Ҫ����  	  
		}
		if (i < secremain)																					 //��Ҫ����
		{
			W25QXX_Erase_Sector(secpos);										    			 //�����������
			for (i = 0; i < secremain; i++)	   												 //����
			{
				W25QXX_BUF[i + secoff] = pBuffer[i];
			}
			W25QXX_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096);	   //д����������  
		}
		else
			W25QXX_Write_NoCheck(pBuffer, WriteAddr, secremain);			 //д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if (NumByteToWrite == secremain)
			break;																										 //д�������
		else																												 //д��δ����
		{
			secpos++;																	 //������ַ��1
			secoff = 0;																 //ƫ��λ��Ϊ0 	 
			pBuffer += secremain;  										 //ָ��ƫ��
			WriteAddr += secremain;										 //д��ַƫ��	   
			NumByteToWrite -= secremain;							 //�ֽ����ݼ�
			if (NumByteToWrite > 4096)
				secremain = 4096;											   //��һ����������д����
			else
				secremain = NumByteToWrite;					 		 //��һ����������д����
		}
	};
}
//��������оƬ		  
//�ȴ�ʱ�䳬��...
void W25QXX::W25QXX_Erase_Chip(void)
{
	W25QXX_Write_Enable();                   //SET WEL 
	W25QXX_Wait_Busy();
	GPIO_ResetBits(mCSPort, mCSPin);         //ʹ������  
	mSPI.SPI_RW(W25X_ChipErase);     				 //����Ƭ��������  
	GPIO_SetBits(mCSPort, mCSPin);					 //ȡ��Ƭѡ     	      
	W25QXX_Wait_Busy();   				  				 //�ȴ�оƬ��������
}
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void W25QXX::W25QXX_Erase_Sector(u32 Dst_Addr)
{
	//����falsh�������,������    
	Dst_Addr *= 4096;
	W25QXX_Write_Enable();                	   //SET WEL 	 
	W25QXX_Wait_Busy();
	GPIO_ResetBits(mCSPort, mCSPin);        	 //ʹ������  
	mSPI.SPI_RW(W25X_SectorErase);     				 //������������ָ�� 
	mSPI.SPI_RW((u8)((Dst_Addr) >> 16));  		 //����24bit��ַ    
	mSPI.SPI_RW((u8)((Dst_Addr) >> 8));
	mSPI.SPI_RW((u8)Dst_Addr);
	GPIO_SetBits(mCSPort, mCSPin);						 //ȡ��Ƭѡ  
	W25QXX_Wait_Busy();   				  					 //�ȴ��������
}
//�ȴ�����
void W25QXX::W25QXX_Wait_Busy(void)
{
	while ((W25QXX_ReadSR() & 0x01) == 0x01);  // �ȴ�BUSYλ���
}
//�������ģʽ
void W25QXX::W25QXX_PowerDown(void)
{
	GPIO_ResetBits(mCSPort, mCSPin);        	 //ʹ������   
	mSPI.SPI_RW(W25X_PowerDown);      	 	  	 //���͵�������
	GPIO_SetBits(mCSPort, mCSPin);						 //ȡ��Ƭѡ
}
//����
void W25QXX::W25QXX_WAKEUP(void)
{
	GPIO_ResetBits(mCSPort, mCSPin);        	 //ʹ������
	mSPI.SPI_RW(W25X_ReleasePowerDown);  	  	 //send W25X_PowerDown command 0xAB
	GPIO_SetBits(mCSPort, mCSPin);						 //ȡ��Ƭѡ
}

//************ Ӧ������ŷ��
void W25QXX::SaveTagData(TagData * data, u8 num)
{
	u8 i;
	for (i = 0; i < num; i++)
	{
		W25QXX_Write((u8 *)data[i].TrainNum, WriAdd, sizeof(TagData) - 1);
		WriAdd += sizeof(TagData);
	}
}
void W25QXX::SetWriAdd(u32 WriAddre)
{
	WriAdd = WriAddre;
}
//************ Ӧ������ŷ��
