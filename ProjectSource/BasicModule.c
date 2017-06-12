/***************************************************************
*Copyright(c) 2013, Sojo
*保留所有权利
*文件名称:BasicModule.c
*文件标识:
*创建日期： 2013年3月18日
*摘要:  用于基本模块,此处包含ms与us延时
*当前版本:1.0
*作者: ZFREE
*取代版本:
*作者:
*完成时间:
************************************************************/
#include"BasicModule.h"

/******************************************************************************
 *函数名： DelayMs()
 *形参： Uint16 ms -- 延时ms数
 *返回值：void
 *功能：  ms延时
 *调用函数： DELAY_US()
 *引用外部变量： null
 *****************************************************************************/
void DelayMs(Uint16 ms)
{
  for( ; ms>0; ms--)
    {
      DELAY_US(1000);
      ServiceDog();
    }
}

/******************************************************************************
 *函数名： DelayUs()
 *形参： Uint16 us -- 延时ms数
 *返回值：void
 *功能：  us延时,
 *调用函数： DELAY_US()
 *引用外部变量： null
 *****************************************************************************/
void DelayUs(Uint16 us)
{
   DELAY_US(us);
}

