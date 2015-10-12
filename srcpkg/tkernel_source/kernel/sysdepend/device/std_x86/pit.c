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

#include <device/port.h>
#include <tk/errno.h>
#include <tk/timer.h>

#include <device/std_x86/port.h>
#include <device/std_x86/pic.h>
#include <device/std_x86/pit.h>
#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE void sendCommand(uint8_t command);
LOCAL INLINE void sendCounter0(uint8_t counter);
LOCAL INLINE void sendCounter1(uint8_t counter);
LOCAL INLINE void sendCounter2(uint8_t counter);

LOCAL INLINE uint8_t recvData(void);
LOCAL INLINE uint8_t recvCounter0(void);
LOCAL INLINE uint8_t recvCounter1(void);
LOCAL INLINE uint8_t recvCounter2(void);

LOCAL void pit_intterupt(struct ctx_reg *reg);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	PIT_CLOCK		1193182		/* 1193181.666666... Hz		*/

/*
==================================================================================

	Description :control word register
	
	bit number	value	description
	0		BCD	Binary Counter
				0:Binary
				1:Binary Coded Decimal
	1...3		Mode	000:interrupt or terminal count
				001:programmable one-shot
				010:rate generator
				011:square wave generator
				100:software triggered strobe
				101:hardware triggered strobe
				110:undefined
				111:undefined
	4,5		RL	Read/Load Mode
				00:counter value is latched into an internal
				   control register at the time of the i/o write
				   operation
				01:read or load least significant byte only
				10:read or load most significatn byte only
				11:read or load lsb first then msb
	6,7		SC	select counter
				00:counter 0
				01:counter 1
				02:counter 2
				11:ilegal value	
==================================================================================
*/
#define	PIT_COM_MASK_BINCOUNT		0x01
#define	PIT_COM_MASK_MODE		0x0E
#define	PIT_COM_MASK_RL			0x30
#define	PIT_COM_MASK_COUNTER		0xC0

/* binary count									*/
#define	PIT_COM_BINCOUNT_BIN		0x00
#define	PIT_COM_BINCOUNT_BCD		0x01

/* data transfer								*/
#define	PIT_COM_RL_LATCH		0x00
#define	PIT_COM_RL_LSBONLY		0x10
#define	PIT_COM_RL_MSBONLY		0x20
#define	PIT_COM_RL_DATA			0x30

/* setting counter								*/
#define PIT_COM_COUNTER0		0x00
#define	PIT_COM_COUNTER1		0x40
#define	PIT_COM_COUNTER2		0xC0

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
EXPORT ER initPit(void)
{
	ER err;
	uint32_t fperiod;
	
	/* -------------------------------------------------------------------- */
	/* register timer interrupt for irq 0					*/
	/* -------------------------------------------------------------------- */
	err = register_int_handler(INT_IRQ0, pit_intterupt);

	if (err) {
		vd_printf("err[%d]:cannot register pit interrupt handler\n", err);
		return(err);
	}

	if (PIT_CLOCK < TIMER_PERIOD) {
		fperiod = PIT_CLOCK;
	} else {
		fperiod = (uint32_t)(1000 / TIMER_PERIOD);
	}

	/* -------------------------------------------------------------------- */
	/* set 10 ms interval							*/
	/* -------------------------------------------------------------------- */
	setPitCounter(fperiod, PIT_COUNTER0, PIT_COM_MODE_SQUAREWAVE);

	/* -------------------------------------------------------------------- */
	/* enable irq 0								*/
	/* -------------------------------------------------------------------- */
	reqEnableIrq(INT_IRQ0);

	return(E_OK);
}

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
EXPORT void setPitCounter(uint32_t freq, enum PIT_COUNTER_TYPE type, uint8_t mode)
{
	uint16_t counter;
	uint8_t command;
	
	if (!freq) {
		freq = 1;
	}

	counter = (uint16_t)(PIT_CLOCK / freq);
	
	if (mode == PIT_COM_MODE_SQUAREWAVE) {
		counter &=  (uint16_t)-1 << 1;
	}

	command = mode;
	command |= PIT_COM_RL_DATA;

	switch (type) {
	case	PIT_COUNTER0:
		command |= PIT_COM_COUNTER0;
		sendCommand(command);
		sendCounter0((uint8_t)(counter & 0xFF));
		sendCounter0((uint8_t)(counter >> 8));
		break;
	case	PIT_COUNTER1:
		command |= PIT_COM_COUNTER1;
		sendCounter1((uint8_t)(counter & 0xFF));
		sendCounter1((uint8_t)(counter >> 8));
		break;
	case	PIT_COUNTER2:
		command |= PIT_COM_COUNTER2;
		sendCounter2((uint8_t)(counter & 0xFF));
		sendCounter2((uint8_t)(counter >> 8));
		break;
	default:
		command |= PIT_COM_COUNTER0;
		sendCounter0((uint8_t)(counter & 0xFF));
		sendCounter0((uint8_t)(counter >> 8));
		break;
	}
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:sendCommand
 Input		:uint8_t command
 		 < command to send to a pit >
 Output		:void
 Return		:void
 Description	:senda a command to a pit
==================================================================================
*/
LOCAL INLINE void sendCommand(uint8_t command)
{
	out_b(PIT_REG_CONTROL, command);
}

/*
==================================================================================
 Funtion	:sendCounter0
 Input		:uint8_t counter
 		 < counter to send to a pit >
 Output		:void
 Return		:void
 Description	:send value of counter 0 to a pit
==================================================================================
*/
LOCAL INLINE void sendCounter0(uint8_t counter)
{
	out_b(PIT_REG_COUNTER0, counter);
}

/*
==================================================================================
 Funtion	:sendCounter1
 Input		:uint8_t counter
 		 < counter to send to a pit >
 Output		:void
 Return		:void
 Description	:send value of counter 1 to a pit
==================================================================================
*/
LOCAL INLINE void sendCounter1(uint8_t counter)
{
	out_b(PIT_REG_COUNTER1, counter);
}

/*
==================================================================================
 Funtion	:sendCounter2
 Input		:uint8_t counter
 		 < counter to send to a pit >
 Output		:void
 Return		:void
 Description	:send value of counter 2 to a pit
==================================================================================
*/
LOCAL INLINE void sendCounter2(uint8_t counter)
{
	out_b(PIT_REG_COUNTER2, counter);
}

/*
==================================================================================
 Funtion	:recvData
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received data >
 Description	:receive a data from a pit
==================================================================================
*/
LOCAL INLINE uint8_t recvData(void)
{
	return(in_b(PIT_REG_CONTROL));
}

/*
==================================================================================
 Funtion	:recvCounter0
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received a value of counter 0 >
 Description	:receive a value of counter 0 from a pit
==================================================================================
*/
LOCAL INLINE uint8_t recvCounter0(void)
{
	return(in_b(PIT_REG_COUNTER0));
}

/*
==================================================================================
 Funtion	:recvCounter1
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received a value of counter 1 >
 Description	:receive a value of counter 1 from a pit
==================================================================================
*/
LOCAL INLINE uint8_t recvCounter1(void)
{
	return(in_b(PIT_REG_COUNTER1));
}

/*
==================================================================================
 Funtion	:recvCounter2
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received a value of counter 2 >
 Description	:receive a value of counter 2 from a pit
==================================================================================
*/
LOCAL INLINE uint8_t recvCounter2(void)
{
	return(in_b(PIT_REG_COUNTER2));
}

/*
==================================================================================
 Funtion	:pit_intterupt
 Input		:struct ctx_reg *reg
 		 < saved context >
 Output		:void
 Return		:void
 Description	:interrupt for a timer interval of a pit
==================================================================================
*/
LOCAL void pit_intterupt(struct ctx_reg *reg)
{
	timer_handler();
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
