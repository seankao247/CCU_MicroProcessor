/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 15/04/10 2:05p $
 * @brief    Change duty cycle and period of output waveform by PWM Double Buffer function.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC100Series.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define PLLCON_SETTING      CLK_PLLCON_50MHz_HXT
#define PLL_CLOCK           50000000

#define PWM_Prescaler				48
#define PWM_Period					1999

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t 	g_u8Forward=1;
volatile uint32_t g_u32BreathingCount=0;
float dt;
float deg;
float cnr;
float cmr;
int val;
/**
 * @brief       PWMA IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWMA interrupt event
 */
void PWMA_IRQHandler(void)
{	 
	static int toggle = 0;
	val=90;
	dt=SystemCoreClock/50/256/8;
	deg=(val-0)*(12-3)/(180-0)+3;
	cnr=dt-1;
	cmr=dt*deg/100-1;
	
	
	if(g_u8Forward==1)
	{
			if(g_u32BreathingCount<PWM_Period)
					g_u32BreathingCount++;
			else
					g_u8Forward=0;
	}
	else
	{
					if(g_u32BreathingCount>0)
						g_u32BreathingCount--;
					else
						g_u8Forward=1;
	}
	PWM_SET_CMR(PWMA,PWM_CH0,g_u32BreathingCount);
	
	PWM_ClearPeriodIntFlag(PWMA, 0);
	
	
	/*
    static int toggle = 0;
    // Update PWMA channel 0 period and duty
    if(toggle == 0)
    {
        PWM_SET_CNR(PWMA, PWM_CH0, 99);
        PWM_SET_CMR(PWMA, PWM_CH0, 39);
    }
    else
    {
        PWM_SET_CNR(PWMA, PWM_CH0, 199);
        PWM_SET_CMR(PWMA, PWM_CH0, 99);
    }
    toggle ^= 1;
    // Clear channel 0 period interrupt flag
    PWM_ClearPeriodIntFlag(PWMA, 0);
		*/
		
 }

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for IRC22M clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external 12MHz XTAL, internal 22.1184MHz */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk | CLK_PWRCON_OSC22M_EN_Msk);

    /* Enable PLL and Set PLL frequency */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_PLL_STB_Msk | CLK_CLKSTATUS_XTL12M_STB_Msk | CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to PLL, STCLK to HCLK/2 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_PLL, CLK_CLKDIV_HCLK(2));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable PWM module clock */
    CLK_EnableModuleClock(PWM01_MODULE);
    CLK_EnableModuleClock(PWM23_MODULE);

    /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));

    /* Select PWM module clock source */
    CLK_SetModuleClock(PWM01_MODULE, CLK_CLKSEL1_PWM01_S_HXT, 0);
    CLK_SetModuleClock(PWM23_MODULE, CLK_CLKSEL1_PWM23_S_HXT, 0);

    /* Reset PWMA channel0~channel3 */
    SYS_ResetModule(PWM03_RST);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);
    /* Set GPA multi-function pins for PWMA Channel0 */
    SYS->GPA_MFP = SYS_GPA_MFP_PA12_PWM0;
}




void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}

/*
uint8_t setservo(uint8_t pin, float val)
{
	if(val>180) val=180;
	dt=SystemCoreClock/50/256/8;
	deg=(val-0)*(12-3)/(180-0)+3;
	cnr=dt-1;
	cmr=dt*deg/100-1;
	switch(pin)
	{
		case 12:
			PWMA->CNR0 = cnr;
			PWMA->CMR0 = cmr;
			PWMA->CAPENR = 0;
		  PWMA->POE=PWMA->POE|PWM_POE_POE0_Msk;
			//PWMA->POE.PWM0 = 1;
		PWMA->PCR=PWMA->PCR|PWM_PCR_CH3EN_Msk;
			//PWMA->PCR.CH0EN = 1;
	}
	return 0;
}*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    printf("\nSystem clock rate: %d Hz\n", SystemCoreClock);

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will use PWMA channel 0 to output waveform\n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0(PA.12)\n");
    printf("\nUse double buffer feature.\n");

    /*
        PWMA channel 0 waveform of this sample shown below:

        |<-        CNR + 1  clk     ->|  CNR + 1 = 199 + 1 CLKs
                       |<-CMR+1 clk ->|  CMR + 1 = 99 + 1 CLKs
                                      |<-   CNR + 1  ->|  CNR + 1 = 99 + 1 CLKs
                                               |<CMR+1>|  CMR + 1 = 39 + 1 CLKs

      __                ______________          _______
        |_____ 100_____|     100      |___60___|  40   |_     PWM waveform

    */


    /*
      Configure PWMA channel 0 init period and duty.
      Period is __HXT / (prescaler * clock divider * (CNR + 1))
      Duty ratio = (CMR + 1) / (CNR + 1)
      Period = 12 MHz / (2 * 1 * (199 + 1)) =  30000 Hz
      Duty ratio = (99 + 1) / (199 + 1) = 50%
    */
		
    /* set PWMA channel 0 output configuration */
    PWM_ConfigOutputChannel(PWMA, PWM_CH0, 3000, 50); /*(PWM_T *pwm,
																												 uint32_t u32ChannelNum,
																												 uint32_t u32Frequency,
																												 uint32_t u32DutyCycle)
																											 */
		

    /* Enable PWM Output path for PWMA channel 0 */
    PWM_EnableOutput(PWMA, 0x1);

    // Enable PWM channel 0 period interrupt
    PWMA->PIER = PWM_PIER_PWMIE0_Msk;
    NVIC_EnableIRQ(PWMA_IRQn);
		
    // Start
    PWM_Start(PWMA, 0x1);
    while(1);


}




