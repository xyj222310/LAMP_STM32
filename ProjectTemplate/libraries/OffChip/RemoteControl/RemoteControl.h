/**
  ******************************************************************************
  * @file    RemoteControl.h 
  * @author  
  * @version V3.0 测试版
  * @date    11/22/2015
  * @brief   增加了校准功能，更改了很多方式
			 		 		
  ******************************************************************************
*/
#ifndef __REMOTECONTROL_H
#define __REMOTECONTROL_H

/* Includes ------------------------------------------------------*/
#include "InputCapture.h"

/*  define ------------------------------------------------------------*/
#define REMOTECONTROL_LOSE 0x00
#define REMOTECONTROL_LOCK 0x10
#define REMOTECONTROL_UNLOCK 0x11
#define REMOTECONTROL_CALIBRATION 0x12  


#define LOWERRIGHT_LOWERLEFT  1	//内八字
#define LOWERLEFT_LOWERRIGHT  2	//外八字
#define LOWERLEFT_UPPERROGHT  3 //左下+右上
#define LOWERLEFT_LOWERLEFT   4 //左下+左下
#define LOWERRIGHT_LOWERRIGHT 5 //右下+右下
#define LOWERLEFT_ALLCAN	  6 //左下+任意
#define LOWERRIGHT_ALLCAN     7 //右下+任意
#define UPPER_LOWER			  8 //上+下

//futaba RemoteControl
#define FUTABA_ORIGINAL_MAX	1900
#define FUTABA_ORIGINAL_MIN	1100

//default output limit
#define RC_DEFAULT_LIMIT_MIN	0
#define RC_DEFAULT_LIMIT_MAX	1000

//Choose way is to use a percentage of the output data output function using fixed scope of data output
//选择输出数据的方式  是使用百分比输出 函数使用规定好的数据范围输出
#define USE_PERCENTAGE_OUTPUT
//#define USE_THE_RANGE_GIVEN_OUTPUT


/* assist  -------------------------------------------------------------*/
//typedef enum {Normal,Lose,Lock,Unlock,ACC_OFFSET,GYRO_OFFSET}Rc_State;
//typedef struct{unsigned int Max;unsigned int Min;}MaxMin;



class RemoteControl{
	private:
			
	//Save the data after eight channel limits(保存8个通道处理后的值)
		uint16_t mPITCH;
		uint16_t mTHROTTLE;
		uint16_t mYAW;
		uint16_t mROLL;

		uint16_t mModel;
		uint16_t mAdditional6;
		uint16_t mAdditional7;
		uint16_t mAdditional8;
	
	//Save the hunter and channel number(保存捕获者和通道号)
		InputCapture *mHunter1;
		u8 mPitchCh1,mThrottleCh2,mYawCh3,mRollCh4;
	
		InputCapture *mHunter2;
		u8 mModelCh5,mCh6,mCh7,mCh8;
	
	
	//Keep the original data and scope(保存原始值的范围)
	
		
		uint16_t OriginalModelMax;
		uint16_t OriginalModelMin;
	
		uint16_t OriginalAdd6Max;
		uint16_t OriginalAdd6Min;

		uint16_t OriginalAdd7Max;
		uint16_t OriginalAdd7Min;

		uint16_t OriginalAdd8Max;
		uint16_t OriginalAdd8Min;
		
		
	//Save the output range(保存输出数据范围)
		uint16_t Limit_MIN;
		uint16_t Limit_MAX;
		
	//The remote control posture(遥控器判断姿态)
		u8  LockPosture;				//上锁
		u8  UnlockPosture;				//解锁
		u8  LostPosture;    			//上电未检测到
		u8	CalibrationPosture;			//校准
		
	//Scope of transformation(将从硬件捕获得到的值（油门、偏航、横滚、俯仰）转换为上层可以理解的值)
		uint16_t LimitsConversion(uint16_t OriginalData, uint16_t OriginalMax,uint16_t OriginalMin);
	

	//Select a state to judgment(选择一种状态进行判断)
		u8 StateSelection(u8 state);
	
	//Perform calibration, save the scope(校准函数，在updata中调用)
		void Calibration();

	public:
	//暂时放在这儿，用于测试		
		uint16_t OriginalPitMax;
		uint16_t OriginalPitMin;
		
		uint16_t OriginalThrMax;
		uint16_t OriginalThrMin;

		uint16_t OriginalYawMax;
		uint16_t OriginalYawMin;

		uint16_t OriginalRolMax;
		uint16_t OriginalRolMin;
	
	//Save State
		u8	State;


	////////////////////////////////////////////////////
	///@brief constructor
    ///@param hunter1 传入一个捕获类的对象
	///@param Pit/Thr/Yaw/Rol 捕获对象对应姿态的通道号
	/////////////////////////////////////////////////////
		RemoteControl(InputCapture *hunter1,u8 Pit,u8 Thr,u8 Yaw,u8 Rol);
	    RemoteControl(InputCapture *hunter1,u8 Pit,u8 Thr,u8 Yaw,u8 Rol,InputCapture *hunter2,u8 Ch5,u8 Ch6,u8 Ch7,u8 Ch8);
		~RemoteControl();
			
	////////////////////////////////////////////////////
	///@brief Update the data at a time, and state judgment(更新一次数据，并状态判断)
    ///@param time_ms -每两次之间进本函数的间隔
	///@param Time_Of_Duration 触发命令需要保持舵量为特定值持续的时长
	///@retval LOCK -上锁命令 UNLOCK -解锁命令 LOSE -丢失  CALIBRATION
	/////////////////////////////////////////////////////
		u8 Updata(u16 time_ms,u16 Time_Of_Duration_ms);
	   
	//Set to the upper range transmission amount of rudder is worth(设置向上层传输舵量值得范围)
		void Set_Limit_MAX_MIN(u16 max,u16 min);
	
	//return  channel value, unit: us or percentage.
	#ifdef 	USE_PERCENTAGE_OUTPUT
		double operator[](u8 chNum); 
	#endif

	#ifdef 	USE_THE_RANGE_GIVEN_OUTPUT
		uint16_t operator[](u8 chNum); 
	#endif
		
	////Remove the calibration mark(结束校准)
		void EndCalibration();
				
	//Output the original value ,us(输出原始值)
		uint16_t GetOriginalValue(u8 ChNum);

	
	//Update the original value(修改原始值范围)
		void SetOriginalPit(uint16_t min,uint16_t max);
		void SetOriginalThr(uint16_t min,uint16_t max);
		void SetOriginalYaw(uint16_t min,uint16_t max);
		void SetOriginalRol(uint16_t min,uint16_t max);
		void SetOriginalAdd5(uint16_t min,uint16_t max);
		void SetOriginalAdd6(uint16_t min,uint16_t max);
		void SetOriginalAdd7(uint16_t min,uint16_t max);
		void SetOriginalAdd8(uint16_t min,uint16_t max);
		
	//Modify the remote control posture judgment(修改遥控器姿态判断)
		void SetLockPosture(u8 Choose);
		void SetUnlockPosture(u8 Choose);
		void SetLostPosture(u8 Choose);
		void SetCalibrationPosture(u8 Choose);
		
	//(Enter the calibration mode)通过函数进入校准模式
		void StartCalibration();

};

			
#endif
