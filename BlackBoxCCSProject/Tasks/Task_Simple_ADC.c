//*****************************************************************************
//
//	Set up ADC_0 Channel_0 to sample voltage and report.
//
//		Author: 		Gary J. Minden
//		Organization:	KU/EECS/EECS 388
//		Date:			2016-02-29 (B60229)
//		Version:		1.0
//
//		Description:	Sample ADC_0_Channel_0 every 0.5 seconds
//
//		Notes:
//
//*****************************************************************************
//

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <Tasks/globals.h>

#include "driverlib/sysctl.h"
#include "driverlib/adc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "stdio.h"

#include "Task_ReportData.h"

//
//	Gloabal subroutines and variables
//

/*
QueueHandle_t temp_qc;

QueueHandle_t ReportData_Queue;

ReportData_Item ADC_report;
*/


extern void Task_Simple_ADC0_Ch0( void *pvParameters ) {

	temp_qc = xQueueCreate(5, sizeof(float));

	ReportData_Queue = xQueueCreate( 10, sizeof( ReportData_Item ) );

	//
	//	Measured voltage value
	//
	uint32_t	ADC_Value;

	//
	//	Enable (power-on) ADC0
	//
	SysCtlPeripheralEnable( SYSCTL_PERIPH_ADC0 );

	//
	// Enable the first sample sequencer to capture the value of channel 0 when
	// the processor trigger occurs.
	//
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

	ADCSequenceStepConfigure( ADC0_BASE, 0, 0,
								ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0 );

	ADCSequenceEnable( ADC0_BASE, 0 );

//	printf( ">>>>ADC Initialized.\n");

	while ( 1 ) {

		//
		// Trigger the sample sequence.
		//
		ADCProcessorTrigger(ADC0_BASE, 0);

		//
		// Wait until the sample sequence has completed.
		//
		while( !ADCIntStatus( ADC0_BASE, 0, false )) {
		}

		//
		// Read the value from the ADC.
		//
		ADCSequenceDataGet(ADC0_BASE, 0, &ADC_Value);
		ADCIntClear( ADC0_BASE, 0 );

		//
		//	Print ADC_Value
		//
		ADC_report.TimeStamp = xPortSysTickCount;
		ADC_report.ReportName = 0;
		ADC_report.ReportValue_0 = ADC_Value;
		ADC_report.ReportValue_1 = 0;
		xQueueSend( ReportData_Queue, &ADC_report, 0 );

		float myf2 = 0.0;
		float myf = (ADC_Value/4095.0)*3.3;
		xQueueSend( temp_qc, &myf,0);

		//printf( ">>ADC_Value: %f\n>>INT: %d\n", myf,ADC_Value);
		xQueueReceive(temp_qc, (void*)&myf2, 10);
		printf("%f\n", 91.93 - 30.45*myf2);

		//
		//	Delay one (1) second.
		//
		vTaskDelay( (1000 * configTICK_RATE_HZ) / 1000 );
	}
}