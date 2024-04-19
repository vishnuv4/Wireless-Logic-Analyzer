/*
 * adc_spi.h
 *
 * Created: 4/19/2024 6:55:23 PM
 *  Author: vishn
 */ 


#ifndef ADC_SPI_H_
#define ADC_SPI_H_

#define ADC_SPI_TASK_SIZE 500
#define ADC_SPI_PRIORITY (configMAX_PRIORITIES - 2)

void vAdcSpiTask(void *pvParameters);



#endif /* ADC_SPI_H_ */