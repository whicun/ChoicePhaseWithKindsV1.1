/****************************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:MonitorCalculate.c
*文件标识:
*创建日期： 2015年1月25日
*摘要:
*2105/3/24: 改综合变量为结构体
*2015/1/31: 添加针对ADC数据的处理流程
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
*******************************************************/
#include "DSP28x_Project.h"
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "Header.h"
#include "RefParameter.h"
#include "Action.h"
/*=============================全局变量定义 Start=============================*/
//#define RFFT_STAGES 6
//#define RFFT_SIZE (1 << RFFT_STAGES)
/*FFTin1Buff section to 2*FFT_SIZE in the linker file*/

//FFT变换，变量内存分配
#pragma DATA_SECTION(RFFTin1Buff,"RFFTdata1");
float32 RFFTin1Buff[RFFT_SIZE];
#pragma DATA_SECTION(RFFToutBuff,"RFFTdata2");
float32 RFFToutBuff[RFFT_SIZE];
#pragma DATA_SECTION(RFFTmagBuff,"RFFTdata3");
float32 RFFTmagBuff[RFFT_SIZE/2+1];
#pragma DATA_SECTION(RFFTF32Coef,"RFFTdata4");
float32 RFFTF32Coef[RFFT_SIZE];

//#pragma DATA_SECTION(RFFphaseBuff,"RFFTdata4");
float32 RFFphaseBuff[RFFT_SIZE/2+1];
RFFT_F32_STRUCT rfft; //FFT 变换数据结构体

//正弦余弦系数
//float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
//float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
//float TwoDivideN = 0; //2/N


//struct  FreqCollect  g_SystemVoltageParameter.frequencyCollect; //监控频率


volatile struct  TimeParameteCall CalTimeMonitor; //计算时间过程
volatile struct  TimeParameteCall* pCalTimeMonitor;



volatile  struct  OrderParamCollect SetParam;// 设定参数

volatile Uint8 FirstTrig = 0;// 首次触发标志 0--不需要触发 非0--需要触发
/*=============================全局变量定义 End=============================*/

/*=============================引用变量 extern Start=============================*/


/*=============================引用变量 extern End=============================*/






/**************************************************
 *函数名：InitMonitorCalData ()
 *功能： 初始化FFT模块，初始化变量，RFFT
 *形参：void
 *返回值：void
 *调用函数：FFT_Init()
 *引用外部变量：
       RFFphaseBuff[i], RFFTmagBuff[i],RFFTF32Coef[i],
       RFFToutBuff[i], RFFTin1Buff[i]
       Cos1step, Sin1step
       g_SystemVoltageParameter.frequencyCollect, pg_SystemVoltageParameter.frequencyCollect
       CalTimeMonitor, pg_SystemVoltageParameter.frequencyCollect
       SetParam
****************************************************/
void InitMonitorCalData(void)
{
   Uint16 i =0;
  for(i=0;i<RFFT_SIZE;i++)
    {
        RFFphaseBuff[i] = 0;
        RFFTmagBuff[i] = 0;
        RFFTF32Coef[i] = 0;
        RFFToutBuff[i] = 0;
        RFFTin1Buff[i] = 0;
    }

  //Cos1step =cos(2.0*PI*1.0/ (float)RFFT_SIZE);//step = 1
  //Sin1step =sin(2.0*PI*1.0/ (float)RFFT_SIZE);//step = 1
  //TwoDivideN = 1;//TwoDivideN = 2.0 / (float)RFFT_SIZE; 消去
  FFT_Init();


  //pg_SystemVoltageParameter.frequencyCollect = &g_SystemVoltageParameter.frequencyCollect;

  CalTimeMonitor.CalTimeDiff = 0;
  CalTimeMonitor.CalTp = 0;
  CalTimeMonitor.CalT0 = 0;
  CalTimeMonitor.CalPhase = 0;
  pCalTimeMonitor = &CalTimeMonitor;
  FirstTrig = 0;

  SetParam.HezhaTime = 50000;
  SetParam.FenzhaTime = 20000;
  SetParam.SetPhase = 0; //此处以COS标准 弧度
  SetParam.SetPhaseTime = 20000 * 0.75f;
  SetParam.HeFenFlag = 0;//合闸

  RefParameterInit();
}
/**************************************************
 *函数名：FFT_Init()
 *功能： 初始化FFT 基本数据
 *形参：
 *返回值：void
 *调用函数： null
 *引用外部变量： rfft
****************************************************/
void FFT_Init(void)
{
  NOP();
  rfft.FFTSize = RFFT_SIZE;
  rfft.FFTStages = RFFT_STAGES;
  rfft.InBuf = &RFFTin1Buff[0]; /*Input buffer*/
  rfft.OutBuf = &RFFToutBuff[0]; /*Output buffer*/
  rfft.CosSinBuf = &RFFTF32Coef[0]; /*Twiddle factor buffer*/
  rfft.MagBuf = &RFFTmagBuff[0]; /*Magnitude buffer*/
  rfft.PhaseBuf  = &RFFphaseBuff[0];

}

/**************************************************
 *函数名：FFT_Cal ()
 *功能： FFT计算
 *形参：float ADsample[] -- ADC采样值
 *返回值：void
 *调用函数：RFFT_f32_sincostable(), RFFT_f32()
 *引用外部变量： RFFTin1Buff[], rfft
****************************************************/
void FFT_Cal(float ADsample[])
{

	Uint16 j=0;
	//将数据复制与此
	for(j=0;j<RFFT_SIZE;j++) 
	{
	    RFFTin1Buff[j] = (float32)ADsample[j];
	}
	//rfft.FFTSize = RFFT_SIZE;
	//rfft.FFTStages = RFFT_STAGES;
	//rfft.InBuf = &RFFTin1Buff[0]; /*Input buffer*/
	//rfft.OutBuf = &RFFToutBuff[0]; /*Output buffer*/
	//rfft.CosSinBuf = &RFFTF32Coef[0]; /*Twiddle factor buffer*/
	//rfft.MagBuf = &RFFTmagBuff[0]; /*Magnitude buffer*/
	//rfft.PhaseBuf  = &RFFphaseBuff[0];
	RFFT_f32_sincostable(&rfft); /*5130 Calculate twiddle factor */
	NOP();
	RFFT_f32(&rfft); /*1278 Calculate output*/
	NOP();
	//RFFT_f32_mag(&rfft); /*695	Calculate magnitude	*/
	NOP();
	//RFFT_f32_phase(&rfft); /*1229	Calculate phase	*/
	NOP();

}

/**
 * 使用FFT值计算有效值
 */
void CalEffectiveValue(void)
{
	uint16_t j = 0;
	float sum = 0;
	float basic = RFFToutBuff[0]/RFFT_SIZE;
	 //计算实部与虚部平方和作为有效值 基波
	 // g_SystemVoltageParameter.voltageA = sqrt( RFFToutBuff[1]*RFFToutBuff[1] + RFFTmagBuff[1]*RFFTmagBuff[1]) * g_SystemCalibrationCoefficient.voltageCoefficient1;
	  for(j=0;j<RFFT_SIZE;j++)
	  {
		  sum += (RFFTin1Buff[j] -  basic) * (RFFTin1Buff[j] -  basic); //
	  }
	  g_SystemVoltageParameter.voltageA = sqrt(sum/RFFT_SIZE) *  g_SystemCalibrationCoefficient.voltageCoefficient1;


}




/********************************************************************
 * 函数名：GetOVD()
 * 参数：
 * 返回值：NULL
 * 功能：基于 FFT，根据数据获取过零时刻
 ********************************************************************/
float phase = 0.0,a = 0, b = 0;
float tp = 0,tph = 0, tp4 = 0,tq = 0, t0 = 0, t1 = 0, sumt = 0;//相位时间差
float tnow = 0; //当前时间折算
Uint16 xiang = 0;
Uint16 count = 0;
void GetOVD(float* pData)
{

	FFT_Cal(pData);   //傅里叶变化计算相角 每个点间隔为312.5us
	a = RFFToutBuff[1]; //实部
	b = RFFToutBuff[RFFT_SIZE - 1]; //虚部
	phase = atan( b/a ); //求取相位
	//相位判断
	if (phase >= -0.00001 ) //认为在第1,3象限   浮点数与零判断问题,是否需要特殊处理？
	{
		if (a >= -0.00001)
		{
			xiang = 1; //在第一象限

		}
		else
		{
			xiang = 3; //在第三象限
		}
	}
	else
	{
		if (a >= 0)
		{
			xiang = 4; //在第四象限
		}
		else
		{
			xiang = 2; //在第二象限
		}
	}
	//象限变换
	switch (xiang)
	{
	case 1:
	{
		phase = phase;
		break;
	}
	case 2:
	{
		phase += PI;
		break;
	}
	case 3:
	{
		phase += PI;
		break;
	}
	case 4:
	{
		phase += 2*PI;
	}
	}
	//超阈值处理
	tp = 1e6 / g_SystemVoltageParameter.frequencyCollect.FreqReal; //真实周期
	tph = tp * 0.5f;  //半个周期


	//phase 为cos值 cos
	/*
	 if (phase < PI3_2)// 时间t0; 距离下一个周期循环正过零点时间
	 {
	      //t0 = (3_2PI - phase)* tp * D2PI; //  1/2pi
		 t0 = (0.75f - phase * D2PI) * tp;
	 }
	 else
	 {
	      //t0 = (3_2PI - phase + 2*PI)* tp * D2PI;
		 t0 = (1.75f - phase * D2PI) * tp;
	 }
	*/
	//t0 = (2*pi - phase) * tp *D2PI;
	//t0 = (1.0f - phase * D2PI) * tp;

	//按设定合闸求取
	SetParam.SetPhaseTime = SetParam.SetPhase * tp * D2PI; //求取对应相角时间
	tnow = phase * tp * D2PI;
	sumt =   SetParam.SetPhaseTime - tnow - SetParam.HezhaTime;
	count = 2;
	do
	{
        t0 = count * tp + sumt;
        count ++;
	}
	while(t0 < 0);






#if WITH_FFT == 1

	// t1 = 10; //内部延时 //修正采样调理电路延时，应该补偿于此。
	t1 = 1713 + 10;
#elif WITH_ZVD == 1
	 t1 = 230;// %调入本函数开始进程   通过仿真获取指令周期数
#endif

	 if (t0 >= t1)
	{
		tq = t0 - t1 ;
	}
	else
	{
		tq =   t0 - t1 + tp; //%延迟一个周期
	}

	if (tq >= tp)
	{
		tq = tq - tp;
	}

	CalTimeMonitor.CalTimeDiff = tq; //赋值给时间差。

	TOGGLE_LED1; //采样输出标志


	DELAY_US(tq); //延时代替
#if WITH_FFT == 1

	DELAY_US(93); //1.修正固有延时，滞后时间 。 2.对于传输同步延时，应该在此处减去.140->80

#elif WITH_ZVD == 1


#endif


	//延时为到实际过零点开始计算
	//此目的为补偿实际计算等误差

	FirstTrig = 0xff;// 准备首次触发
	InitCpuTimers();


	ConfigCpuTimer(&CpuTimer0, 80, tph);
	//启动定时器
	//SET_YONGCI_ACTION(); //同步合闸定时器
	TOGGLE_LED2; //首次触发跳变
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	//赋值以备调用
	CalTimeMonitor.CalTp = tp;
	CalTimeMonitor.CalT0 = t0;
	CalTimeMonitor.CalPhase = phase;
	pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //置为非满标志

}

/**
 * 同步触发器，利用采样数据计算同步触发动作时刻，发出触发命令,以A相为例
 *
 * @param  *pData   指向采样数据的指针
 * @brief  计算触发时刻，发布触发命令
 */
void SynchronizTrigger(float* pData)
{
	float time = 0, difftime = 0;
	FFT_Cal(pData);   //傅里叶变化计算相角 每个点间隔
	a = RFFToutBuff[1]; //实部
	b = RFFToutBuff[RFFT_SIZE - 1]; //虚部
	phase = atan( b/a ); //求取相位(-pi/2 pi/2)


	//相位判断
		if (phase >= 0 ) //认为在第1,3象限   浮点数与零判断问题,是否需要特殊处理？
		{
			if (a >= 0)
			{
				xiang = 1; //在第一象限

			}
			else
			{
				xiang = 3; //在第三象限
			}
		}
		else
		{
			if (a >= 0)
			{
				xiang = 4; //在第四象限
			}
			else
			{
				xiang = 2; //在第二象限
			}
		}
		phase += PID2;//cos转化为sin
		if (phase > PI2)//大于2PI
		{
			phase = phase - PI2;
		}

		//象限变换
		switch (xiang)
		{
		case 1:
		{
			phase = phase;
			break;
		}
		case 2:
		{
			phase += PI;
			break;
		}
		case 3:
		{
			phase += PI;
			break;
		}
		case 4:
		{
			phase += 2*PI;
		}
		}
		//计算开始时间
	    g_PhaseActionRad[0].startTime = g_SystemVoltageParameter.period* phase* D2PI;
	    //此处相乘，为了保证使用最新的周期
	    g_PhaseActionRad[0].realTime =  g_SystemVoltageParameter.period * g_PhaseActionRad[0].realRatio;
	    //计算延时之和
	    g_ProcessDelayTime[PHASE_A].sumDelay = g_ProcessDelayTime[PHASE_A].sampleDelay +
	    		g_ProcessDelayTime[PHASE_A].transmitDelay +g_ProcessDelayTime[PHASE_A].actionDelay;
	    //总的时间和
	    time =  g_ProcessDelayTime[PHASE_A].sumDelay +
	    		g_PhaseActionRad[0].startTime - g_PhaseActionRad[0].realTime;

	    count = 1;
	    do
	    {
	    	difftime = count *  g_SystemVoltageParameter.period - time;
	    	//判断时间差是否大于0，若大于则跳出
	    	if(difftime > 0)
	    	{
	    		break;
	    	}
	    	count++;

	    }while(count < 100); //TODO:添加异常判断




	    //停止
	  	    CpuTimer1Regs.TCR.bit.TSS = 1;//停止定时器，防止打断中断
	  	    if (CpuTimer1Regs.TIM.all >= CpuTimer1Regs.TIM.all)
	  	    {
	  	    	 g_ProcessDelayTime[PHASE_A].innerDelay = (Uint16)(0.0125f *(CpuTimer1Regs.PRD.all - CpuTimer1Regs.TIM.all));

	  	    }
	  	    else
	  	    {
	  	    	g_ProcessDelayTime[PHASE_A].innerDelay = 88;
	  	    }



		if (difftime > 0)
		{


			//若时间之和大于等于0，则正常补偿；否则添加一个周期的延时
			time = difftime - g_ProcessDelayTime[PHASE_A].innerDelay + g_ProcessDelayTime[PHASE_A].compensationTime;
			if ( time >= 0 )
			{
				g_ProcessDelayTime[PHASE_A].calDelay =  (Uint16)time;
			}
			else
			{
				time = g_SystemVoltageParameter.period + time;
				if (time >= 0)
				{
					g_ProcessDelayTime[PHASE_A].calDelay =  (Uint16)time;
				}
				else
				{
					g_ProcessDelayTime[PHASE_A].calDelay = difftime;//超限后不进行补偿。
				}


			}
			DELAY_US(g_ProcessDelayTime[PHASE_A].calDelay);

			//产生序列动作，可以考虑在之前进行删去。
			//输出动作
			//
			SET_OUTB4_H;
			SET_OUTB4_H;
			SET_OUTB4_H;
			DELAY_US(50);
			SET_OUTB4_H;
			DELAY_US(50);
			SET_OUTB4_H;
			DELAY_US(50);
			SET_OUTB4_H;
			DELAY_US(50);
			SET_OUTB4_L;
			SET_OUTB4_L;
			SET_OUTB4_L;
			SendMultiFrame(&g_NetSendFrame);
		}
		else
		{
			//TODO: 异常处理
		}




}




