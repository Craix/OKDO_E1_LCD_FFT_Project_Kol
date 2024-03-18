#include <stdio.h>

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC55S69_cm33_core0.h"
#include "fsl_debug_console.h"

#include "lcd.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define NFFT 512
#define N2FFT (2*NFFT)

float x[N2FFT]={0};
q31_t y[N2FFT]={0};
float f=0;

void drawPlot(float *data, uint16_t size, uint16_t color)
{
	static float y=0, y1=0;
	y=64-63*(data[0]);

	for(int x=0; x<size-1; x++)
	{
		y1=y;
		y=64-63*(data[x*2]);

		if(y>=LCD_HEIGHT)
			y=LCD_HEIGHT-1;

		if(y<0)
			y=0;

		LCD_Draw_Line(x, y1, x+1, y, color);
	}
}

void drawBars(float *data, uint16_t size, uint16_t color)
{
	static float y=0;
	for(int x=0; x<size; x++)
	{
		y=127-127*(data[x]);

		if(y>=LCD_HEIGHT)
			y=LCD_HEIGHT-1;

		if(y<0)
			y=0;

		LCD_Draw_Line(x, 127, x, y, color);
	}
}
/*
 * @brief Application entry point.
 */
int main(void)
{
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();

	#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
	#endif

	LCD_Init(FLEXCOMM8_PERIPHERAL);

	f=1;
	while(1)
	{
		for(int i=0;i<NFFT;i++)
		{
			x[2*i] = arm_sin_f32(2*PI*i*f/NFFT);
			x[2*i+1] = 0;
		}

		f+=0.02;

		if(f>=160)
			f=1;

		LCD_Clear(0x0000);
		drawPlot(x, 160, 0x0FF0);
		arm_float_to_q31(x, y, N2FFT);
		arm_cfft_q31(&arm_cfft_sR_q31_len512, y, 0, 1);
		arm_cmplx_mag_q31(y, y, NFFT);
		arm_scale_q31 (y, 0x7FFFFFFF, 2, y, NFFT);
		arm_q31_to_float(y, x, N2FFT);
		drawBars(x, 160, 0xF800);
		LCD_GramRefresh();
	}

	return 0 ;
}
