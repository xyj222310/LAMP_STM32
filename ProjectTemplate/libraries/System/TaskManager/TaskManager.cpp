#include "TaskManager.h"
#include "USART.h"

double TaskManager::_it_time = 0;
u16 TaskManager::_lose_tick_cnt = 0;
double TaskManager::_old_time = 0;

//Constructor
TaskManager::TaskManager()
{
	
	SysTick->CTRL &= 0xFFFFFFF8; //Clock div 8 = 9M, disable INT, disable count
	SysTick->LOAD = 16200000;     //1.8s
	SysTick->VAL = 0;
	SysTick->CTRL |= 0x00000003; //ENABLE count + INT
	for(volatile int i=0; i<1000;i++);
}	
//get current time since power on
double TaskManager::Time()
{
	double curTime = _it_time + 1.8 - SysTick->VAL/9000000.0; //update current time
		
	double dt = curTime - _old_time;
	
	if(dt < -1) //not INT, VAL reload(-1.8s)
	{
		curTime += 1.8;
	}
	else if(dt>1.79)
	{
		curTime -= 1.8;
	}
	_old_time = curTime;            //update old time
	return curTime;
}

void TaskManager::Update()
{
	_it_time += 1.8;

	double dt = _it_time - Time();
	if(dt<-1.79)
	{
		_lose_tick_cnt++;
		_it_time += 1.8;
	}
}

void TaskManager::DelayUs(u16 nus)
{
	double OldT = Time();
	while ((Time() - OldT) < double(nus) / 1000000.0);
}

void TaskManager::DelayMs(u16 nms)
{
	double OldT = Time();
	while ((Time() - OldT) < double(nms) / 1000.0);
}

void TaskManager::DelayS(u16 ns)
{
	double OldT = Time();
	while ((Time() - OldT) < double(ns));
}

//SysTick interrupt IRQ handler
extern "C"
{
	void SysTick_Handler(void)
	{
		TaskManager::Update();
	}
}


