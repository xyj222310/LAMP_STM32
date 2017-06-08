#include "stm32f10x.h"

/***** System *****/
#include "Configuration.h"
#include "TaskManager.h"

/***** OnChip *****/
#include "USART.h"
#include "I2C.h"
//#include "Timer.h"
#include "ADC.h"
#include "PWM.h"
#include "InputCapture_TIM.h"
#include "InputCapture_EXIT.h"

/***** OffChip*****/
#include "LED.h"
#include "DHT12.h"
#include "SHARP_PM2_5.h"
#include "Socket_esp8266.h"
#include "Remoter_PWM_TIM.h"
#include "Remoter_PWM_EXIT.h"

/***** ToolBox*****/
#include "Mathtool.h"
#include "GidLink.h"

union DataConvert
{
	int byInt;
	float byFloat;
	u8  byByte[4];
} dataConvert;

union DoubleDataConvert
{
	double byDouble;
	u8  byByte[8];
} doubledataConvert;

/***************************/
TaskManager tskmgr;

USART com1(USART1, 15200);
//Sharp_PM_2_5 mPM25(com1);

ADC MQ_AO(5);

USART com2(USART2, 115200);
Socket_esp8266 mWifi(com2);

bool Work = false;

//////////////////////////////////////////////////////////////////////////////
//////////////////            Global Variable       //////////////////////////
//////////////////////////////////////////////////////////////////////////////

void WIFI_Init()
{
	mWifi.Restore();
	mWifi.Init();
	mWifi.SetIP((char *)"8ge0", (char *)"00000000", (char *)"60.205.219.43", (char *)"9999");
}

void LED_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;										//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;       //
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;  
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
}

void Set_LED(bool IsTurnOn)
{
	IsTurnOn ? GPIO_SetBits(GPIOA, GPIO_Pin_6) : GPIO_ResetBits(GPIOA, GPIO_Pin_6);
}

void wifiRec()
{
	char data1[100] = {0};
//	char * indexStatus;
	if((mWifi.mWifiStatus == WIFI_FREE) && (mWifi.mWIFIInitStep == WIFIInit_FINISH))
	{
		if(com2.RxSize() > 2)
		{
			if(mWifi.Read(data1))
			{
//				indexStatus = strstr(data, "status:");
//				indexStatus += sizeof("status");
//				if(indexStatus[0] == '1'){
				if(strcmp(data1, "1")==0) {
					Work = true;
					Set_LED(true);
//					ADC MQ_AO(5);
//					GPIO_InitTypeDef GPIO_InitStructure;//
//					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//					GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;       //
//					GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;  
//					GPIO_Init(GPIOA, &GPIO_InitStructure); 
				}
//				else if(indexStatus[0] == '0'){
				else if((strcmp(data1, "0")==0) || (strcmp(data1, "-1")==0)) {
					Work = false;
					Set_LED(false);
//					GPIO_InitTypeDef GPIO_InitStructure;//
//					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//					GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;       //
//					GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;  
//					GPIO_Init(GPIOA, &GPIO_InitStructure); 
//					GPIO_ResetBits(GPIOA, GPIO_Pin_5);
				}
			}
		}
	}
}


float votage = 0;
static double oldTime = 0, curTime = 0;


void CheckADCStatus(void)
{
	curTime = tskmgr.Time();
	votage = MQ_AO[5];
	if(votage > 2.0)
	{
		if(Work) 
			Set_LED(true);

	} else {

		Set_LED(false);
	}
}

void WifiSend()
{
	char data[100] = {0};
	strcpy(data, "{\"socketId\":\"113030102500\",\"socketName\":\"tseSocket\",\"ownerId\":\"-1\",\"status\":\"-1\"}");
	if((mWifi.mWifiStatus == WIFI_FREE) && (mWifi.mWIFIInitStep == WIFIInit_FINISH))
		mWifi.Write(data, 81);
}

int main()
{
	uint16_t loop500Hzcnt=0,loop200HzCnt=0,loop50HzCnt=0 , loop600HzCnt=0,loop100HzCnt=0, loop20HzCnt=0 , loop10HzCnt=0, loop1HzCnt=0;
	while (tskmgr.Time() < 1);

//	double curTime = 0, oldTime = 0;
	int cnt = 0;
	
	WIFI_Init();
	LED_Init();
	
	while (1)
	{
		curTime = tskmgr.Time();
		float deltaT = curTime - oldTime;
		mWifi.CheckStatus();
		wifiRec();
		if (deltaT >= 0.002)
		{
//			Set_LED(true);
			oldTime = curTime;
			loop500Hzcnt++;
			CheckADCStatus();    //检测是否有人,控制开关状态
			if(loop500Hzcnt >= 5)//100HZ 控制
			{
//				com1 << MQ_AO[5]*0.033;
				loop500Hzcnt = 0;
				loop100HzCnt++;
				if(loop100HzCnt >= 2)//50HZ 
				{
					loop100HzCnt = 0;
					loop10HzCnt++;
					if(loop10HzCnt >= 5)//10HZ
					{
						loop10HzCnt = 0;
						loop1HzCnt++;
						if(loop1HzCnt>=10){
							loop1HzCnt=0;
							if(++cnt >= 3)
							{
								WifiSend();
								cnt = 0;
							}
						}
					}
				}
			}
		}
	}
}
