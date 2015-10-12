/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#ifndef	__PIT_H__
#define	__PIT_H__

#include <cpu.h>
#include <stdint.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	PIT_FREQ_10_MSEC	100		/* for 10 msec			*/
#define	PIT_FREQ_1_MSEC		1000		/* for 1 msec			*/

/* counter mode									*/
#define	PIT_COM_MODE_TERMINAL	0x00
#define	PIT_COM_MODE_PROGONE	0x02
#define	PIT_COM_MODE_RATEGEN	0x04
#define	PIT_COM_MODE_SQUAREWAVE	0x06
#define	PIT_COM_MODE_SOFTTRIG	0x08
#define	PIT_COM_MODE_HARDTRIG	0x0A

enum PIT_COUNTER_TYPE {
	PIT_COUNTER0,
	PIT_COUNTER1,
	PIT_COUNTER2,
};

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initPit
 Input		:void
 Output		:void
 Return		:ER
 		 < error status >
 Description	:intialize a programmable interval timer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER initPit(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setPitCounter
 Input		:uint32_t freq
 		 < frequencies to set to a pit >
 		 enum PIT_COUNTER_TYPE type
 		 < type of counter >
 		 uint8_t mode
 		 < mode of pit behavior >
 Output		:void
 Return		:void
 Description	:set counter to a pit
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setPitCounter(uint32_t freq, enum PIT_COUNTER_TYPE type, uint8_t mode);


#endif	// __PIT_H__
