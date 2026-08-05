#ifndef LDR_H_
#define LDR_H_

#include "main.h"

/*=====================LDR Return Value===========================*/
// Ambient Light Value: 600 - 2500
// Finger Cover Value: 	Below 500
// As long as it is below 500 it can be consider as closed
/*================================================================*/
/**
 *
 * 12-bit ADC value (0 to 4095)
 */
uint16_t Read_LDR_PB1_LDR2(void);

/**
 *
 * 12-bit ADC value (0 to 4095)
 */
uint16_t Read_LDR_PC4_LDR1(void);

#endif /* LDR_H_ */
