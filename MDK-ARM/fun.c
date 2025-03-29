#include "headfile.h"

uint8_t view = 0;	//全局切屏变量

int  fre_a,fre_b;

typedef enum { T , F } mode ;
mode  display_mode = F ;//周期与频率模式选择

typedef enum { D ,H , X }parameter ;
parameter current_parameter = D ;//PD,PH,PX参数选择

uint16_t PD = 1000;
uint16_t PH = 5000;
int PX = 0;

uint8_t B3_pressd;
uint16_t B3_press_dration;

uint8_t NDA = 0 ;
uint8_t NDB = 0 ;
uint8_t NHA = 0 ;
uint8_t NHB = 0 ;

//led函数封装
void led_show(uint8_t led ,uint8_t mode )
{
	HAL_GPIO_WritePin (GPIOD,GPIO_PIN_2,GPIO_PIN_SET );
	if(mode )
	HAL_GPIO_WritePin (GPIOC,GPIO_PIN_8 << (led-1),GPIO_PIN_RESET );
	else 
	HAL_GPIO_WritePin (GPIOC,GPIO_PIN_8 << (led-1),GPIO_PIN_SET );
	HAL_GPIO_WritePin (GPIOD,GPIO_PIN_2,GPIO_PIN_RESET );
}

//频率捕获函数
uint16_t freA_capture ,freB_capture;
uint16_t capture_A ,capture_B;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim ->Instance == TIM2 )
	{
		capture_A = HAL_TIM_ReadCapturedValue (htim ,TIM_CHANNEL_1 );
		__HAL_TIM_SetCounter (htim , 0);
		freA_capture = 80000000/ (80*capture_A );
		HAL_TIM_IC_Start_IT (htim ,TIM_CHANNEL_1 );

	}
	else if(htim ->Instance == TIM3 )
	{
		capture_B = HAL_TIM_ReadCapturedValue (htim ,TIM_CHANNEL_1 );
		__HAL_TIM_SetCounter (htim , 0);
		freB_capture = 80000000/ (80*capture_B );
	HAL_TIM_IC_Start_IT (htim ,TIM_CHANNEL_1 );
	}

}

//频率超限与频率突变功能实现

uint16_t NHA_flag , NHB_flag ;

void change ()
{	//NHA 
	if (NHA_flag == 0)
	{
		if (fre_a > PH)
		{
			NHA ++;
			NHA_flag = 1;
		
		}
	}
	else 
		if (fre_a <= PH)	NHA_flag = 0 ;
	//NHB
		if (NHB_flag == 0)
	{
		if (fre_b > PH)
		{
			NHB ++;
			NHB_flag = 1;
		
		}
	}
	else 
		if (fre_b <= PH)	NHB_flag = 0 ;

}
	//频率突变
uint16_t timer_100ms ;
uint16_t timerA_3s;
uint16_t timerB_3s;

int freA_max ,freB_max;
int freA_min ,freB_min;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim -> Instance == TIM4 )
	{
		timer_100ms ++ ;
		if (timer_100ms == 10 )
		{
			 fre_a = PX + freA_capture ;
			 fre_b = PX + freB_capture ;
			timer_100ms = 0 ;
		}
	}
	
	//A的频率突变
	if (fre_a > 0)
	{
			if (timerA_3s == 0)
		{
			freA_max = fre_a ;
			freA_min = fre_a ;
			
		}
		else 
		{
			if (fre_a > freA_max )	freA_max = fre_a ;
			if (fre_a < freA_min )	freA_min = fre_a ;
		}
		timerA_3s ++;
		
		if (timerA_3s >= 300)//3s
		{
			timerA_3s = 0 ;
			if ((freA_max - freA_min )> PD)	NDA ++;
			freA_max = fre_a;
			freA_min = fre_a;
		}
	}
	//B的频率突变
	if (fre_b > 0 )
	{
		if (timerB_3s == 0)
		{
			freB_max = fre_b ;
			freB_min = fre_b ;
			
		}
		else 
		{
			if (fre_b > freB_max )	freB_max = fre_b ;
			if (fre_b < freB_min )	freB_min = fre_b ;
		}
		timerB_3s ++;
		
		if (timerB_3s >= 300)//3s
		{
			timerB_3s = 0 ;
			if ((freB_max - freB_min )> PD)	NDB ++;
			freB_max = fre_b;
			freB_min = fre_b;

		}
	}
	if (B3_pressd )
		B3_press_dration ++;
}

//key函数封装

uint8_t B1_state;
uint8_t B1_last_state =1;			//定义第0次按键未按下
uint8_t B2_state;
uint8_t B2_last_state =1;
uint8_t B3_state;
uint8_t B3_last_state =1;
uint8_t B4_state;
uint8_t B4_last_state =1;

void key_scan()
{	
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

	if(B1_state == 0 && B1_last_state == 1 && view == 1)
	{
		switch (current_parameter )
		{
			case D :
				PD += 100;
			if (PD >= 999)	PD = 1000;
				break ;
			case H :
				PH += 100;
			if (PH >= 9999)	PH = 10000;
				break ;
			case X :
				PX += 100;
			if (PX >= 999)		PX = 1000;
				break ;
		}
		
	}
	if(B2_state == 0 && B2_last_state == 1 && view == 1)
	{
				switch (current_parameter )
		{
			case D :
				PD -= 100;
			if (PD <= 99)	PD = 100;
				break ;
			case H :
				PH -= 100;
			if (PH <= 999)	PH = 1000;
				break ;
			case X :
				PX -= 100;
			if (PX <= -999)	PX = -1000;
				break ;
			
		}

	}
	if(B3_state == 0 && B3_last_state == 1)
	{
		if (view == 0 )
		display_mode =! display_mode ;
		else if (view == 1)
		{
			current_parameter ++;
			current_parameter %=3;
		}
		else if (view == 2)
		{
			B3_pressd = 1;
			B3_press_dration = 0;
		}
	}
	else if (B3_state == 1 && B3_last_state == 0)
	{
		if (B3_press_dration > 100)
		{
			NHA = 0;
			NHB = 0;
			NDA = 0 ;
			NDB = 0 ;
			
		}
	}
	if(B4_state == 0 && B4_last_state == 1)
	{
		view ++;
		view %=3;
		LCD_Clear (Black );
		if (view == 1)
		current_parameter = D;
		if (view == 0)
		display_mode = F;
	}
	B1_last_state = B1_state;	
	B2_last_state = B2_state;
	B3_last_state = B3_state;	
	B4_last_state = B4_state;
	
}

//lcd函数
char text[20];
void lcd_show()
{
	if(view == 0)
	{
		if(display_mode == F)					//频率模式
			{
		sprintf (text ,"        DATA        ");
		LCD_DisplayStringLine(Line1 , (uint8_t *)text );
		if(fre_a < 0)
			{
		sprintf (text ,"     A=NULL        ");
		LCD_DisplayStringLine(Line3 , (uint8_t *)text );
			}
		else if(fre_a >0 && fre_a <1000)
			{
		sprintf (text ,"     A=%dHz        ",fre_a );
		LCD_DisplayStringLine(Line3 , (uint8_t *)text );
			}
		else if(fre_a > 1000)
			{
		sprintf (text ,"     A=%.2fKHz        ",(float )fre_a/1000 );
		LCD_DisplayStringLine(Line3 , (uint8_t *)text );
			}
		
		
		if(fre_b < 0)
			{
		sprintf (text ,"     B=NULL        ");
		LCD_DisplayStringLine(Line4 , (uint8_t *)text );
			}
		else if(fre_b >0 && fre_b <1000)
			{
		sprintf (text ,"     B=%dHz        ",fre_b );
		LCD_DisplayStringLine(Line4 , (uint8_t *)text );
			}
		else if(fre_b > 1000)
			{
		sprintf (text ,"     B=%.2fKHz        ",(float )fre_b/1000 );
		LCD_DisplayStringLine(Line4 , (uint8_t *)text );

			}
		}
		if (display_mode == T)					//周期模式
		{
			float fre__A = 1.0/ fre_a * 1000000;
			float fre__B = 1.0/ fre_b * 1000000;
			sprintf (text ,"        DATA        ");
			LCD_DisplayStringLine(Line1 , (uint8_t *)text );
			if(fre__A < 0)
				{
			sprintf (text ,"     A=NULL        ");
			LCD_DisplayStringLine(Line3 , (uint8_t *)text );
				}
			else if(fre__A >0 && fre__A <1000)
				{
			sprintf (text ,"     A=%duS        ",(uint16_t )fre__A );
			LCD_DisplayStringLine(Line3 , (uint8_t *)text );
				}
			else if(fre__A > 1000)
				{
			sprintf (text ,"     A=%.2fmS        ",fre__A/1000 );
			LCD_DisplayStringLine(Line3 , (uint8_t *)text );
				}
			
			
			if(fre__B < 0)
				{
			sprintf (text ,"     B=NULL        ");
			LCD_DisplayStringLine(Line4 , (uint8_t *)text );
				}
			else if(fre__B >0 && fre__B <1000)
				{
			sprintf (text ,"     B=%duS        ",(uint16_t )fre__B );
			LCD_DisplayStringLine(Line4 , (uint8_t *)text );
				}
			else if(fre__B > 1000)
				{
			sprintf (text ,"     B=%.2fmS        ",fre__B/1000 );
			LCD_DisplayStringLine(Line4 , (uint8_t *)text );

				}

		}
	}
	
	if (view == 1)
	{
			sprintf (text ,"        PARA        ");
			LCD_DisplayStringLine(Line1 , (uint8_t *)text );
			sprintf (text ,"     PD=%dHz        ",PD);
			LCD_DisplayStringLine(Line3 , (uint8_t *)text );
			sprintf (text ,"     PH=%dHz        ",PH);
			LCD_DisplayStringLine(Line4 , (uint8_t *)text );
			sprintf (text ,"     PX=%dHz        ",PX);
			LCD_DisplayStringLine(Line5 , (uint8_t *)text );

	}
		if (view == 2)
	{
			sprintf (text ,"        RECD        ");
			LCD_DisplayStringLine(Line1 , (uint8_t *)text );
			sprintf (text ,"     NDA=%d        ",NDA );
			LCD_DisplayStringLine(Line3 , (uint8_t *)text );
			sprintf (text ,"     NDB=%d        ",NDB );
			LCD_DisplayStringLine(Line4 , (uint8_t *)text );
			sprintf (text ,"     NHA=%d        ",NHA );
			LCD_DisplayStringLine(Line5 , (uint8_t *)text );
			sprintf (text ,"     NHB=%d        ",NHB );
			LCD_DisplayStringLine(Line6 , (uint8_t *)text );
	}

}

//LED 功能实现

void led_pro()
{
	if (view == 0)
		led_show (1,1);
	else 	led_show (1,0);
	if (fre_a > PH )
		led_show (2,1);
	else led_show (2,0);
	if (fre_b > PH )
		led_show (3,1);
	else led_show (3,0);
	if (NDA >=3 || NDB >=3)
		led_show (8,1 );
	else led_show (8, 0 );
}










