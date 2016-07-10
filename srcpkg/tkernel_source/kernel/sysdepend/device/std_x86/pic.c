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

#include <stdint.h>
#include <tk/typedef.h>
#include <cpu.h>
#include <device/port.h>
#include <device/std_x86/pic.h>

#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE uint8_t maskIrqMaster(uint8_t irq_num);
LOCAL INLINE uint8_t unmaskIrqMaster(uint8_t irq_num);
LOCAL INLINE uint8_t maskIrqSlave(uint8_t irq_num);
LOCAL INLINE uint8_t unmaskIrqSlave(uint8_t irq_num);

/*
----------------------------------------------------------------------------------
	controls for master pic
----------------------------------------------------------------------------------
*/
LOCAL INLINE void sendCommandMaster(uint8_t command);
LOCAL INLINE uint8_t recvStatusMaster(void);
LOCAL INLINE void sendIrqMaskMaster(uint8_t irqmask);
LOCAL INLINE uint8_t recvIrqMaskMaster(void);
LOCAL INLINE void sendDataMaster(uint8_t data);
LOCAL INLINE uint8_t recvDataMaster(void);

/*
----------------------------------------------------------------------------------
	controls for slave pic
----------------------------------------------------------------------------------
*/
LOCAL INLINE void sendCommandSlave(uint8_t command);
LOCAL INLINE uint8_t recvStatusSlave(void);
LOCAL INLINE void sendIrqMaskSlave(uint8_t irqmask);
LOCAL INLINE uint8_t recvIrqMaskSlave(void);
LOCAL INLINE void sendDataSlave(uint8_t data);
LOCAL INLINE uint8_t recvDataSlave(void);

/*
==================================================================================

	DEFINE 

==================================================================================
*/


/*
----------------------------------------------------------------------------------

	PIC Initialization control words

----------------------------------------------------------------------------------
*/
/*
==================================================================================

	Description :initialization control word 1
	
	bit number	value
	0		IC4
	1		SNGL
	2		ADI
	3		LTIM
	4		1
	5...7		0

==================================================================================
*/
#define	ICW1_MST	0x11	/* ICW1 for master pic	*/
#define	ICW1_SLV	0x11	/* ICW1 for slave pic	*/

/*
==================================================================================

	Description :initialization control word 2
	
	bit number	value
	0-2		A8/A9/A10
	3-7		A11(T3)/
			A12(T4)/
			A13(T5)/
			A14(T6)/
			A15(T7)

==================================================================================
*/
#define	ICW2_MST	0x20	/* ICW2 for master pic (IRQ 0...7 ) 		*/
				/* IRQ 0 is mapped to interrupt number 0x20	*/
#define	ICW2_SLV	0x28	/* ICW2 for slave  pic (IRQ 8...15) 		*/
				/* IRQ 0 is mapped to interrupt number 0x28	*/

/*
==================================================================================

	Description :initialization control word 3
	
	bit number	value
	0-7		IRQ0-IRQ7

	Master IRQ2 conected to Slave IRQ2
==================================================================================
*/
#define	ICW3_MST	0x04	/* ICW3 for master pic				*/
#define	ICW3_SLV	0x02	/* ICW3 for slave pic				*/

/*
==================================================================================

	Description :initialization control word 4
	
	bit number	value
	0		uPM
	1		AEOI
	2		M/S
	3		BUF
	4		SFNM
	5...7		0

==================================================================================
*/
#define	ICW4_MST	0x01	/* ICW4 for master pic				*/
#define	ICW4_SLV	0x01	/* ICW4 for slave pic				*/


/*
----------------------------------------------------------------------------------

	PIC operation command words

----------------------------------------------------------------------------------
*/
/*
==================================================================================

	Description :operation command word 2
	
	bit number	value
	0-2		L0/L1/L2
	3-4		0
	5		EOI
	6		SL
	7		R

==================================================================================
*/
#define	OCW2_INT_MASK_LV0	0x00
#define	OCW2_INT_MASK_LV1	0x01
#define	OCW2_INT_MASK_LV2	0x02
#define	OCW2_INT_MASK_LV3	0x04
#define	OCW2_MASK_EOI		0x20
#define	OCW2_MASK_SL		0x40
#define	OCW2_MASK_R		0x80

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
 Funtion	:initPic
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize programmable intterupt controllers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void initPic(void)
{
	/* -------------------------------------------------------------------- */
	/* disable all interrupt requests					*/
	/* -------------------------------------------------------------------- */
	reqDisableIrq(PIC_INT_IRQ_ALL);
	/* -------------------------------------------------------------------- */
	/* send ICW1 to pics							*/
	/* -------------------------------------------------------------------- */
	sendCommandMaster(ICW1_MST);
	sendCommandSlave(ICW1_SLV);
	/* -------------------------------------------------------------------- */
	/* send ICW2 to pics							*/
	/* -------------------------------------------------------------------- */
	sendDataMaster(ICW2_MST);
	sendDataSlave(ICW2_SLV);
	/* -------------------------------------------------------------------- */
	/* send ICW3 to pics							*/
	/* -------------------------------------------------------------------- */
	sendDataMaster(ICW3_MST);
	sendDataSlave(ICW3_SLV);
	/* -------------------------------------------------------------------- */
	/* send ICW4 to pics							*/
	/* -------------------------------------------------------------------- */
	sendDataMaster(ICW4_MST);
	sendDataSlave(ICW4_SLV);
	/* -------------------------------------------------------------------- */
	/* cascade a slave to a master						*/
	/* -------------------------------------------------------------------- */
	reqDisableIrq(PIC_INT_IRQ_ALL);
	
	reqEnableIrq(INT_IRQ2);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:reqEnableIrq
 Input		:uint8_t irq_num
 		 < irq number to enable >
 Output		:void
 Return		:uint16_t
 		 < old flags : low byte is from master, high byte is from slave >
 Description	:request to enable specified irq
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT uint16_t reqEnableIrq(uint8_t irq_num)
{
	uint16_t old_mask = 0;
	
	if (irq_num < INT_IRQ0 || INT_IRQ15 < irq_num) {
		if (irq_num != PIC_INT_IRQ_ALL) {
			return(old_mask);
		}
	}
	
	//temp = recvIrqMaskMaster();
	
	if (irq_num <= INT_IRQ7) {
		/* ------------------------------------------------------------ */
		/* mask irq 0 - irq 7 for a master				*/
		/* ------------------------------------------------------------ */
		old_mask = unmaskIrqMaster(irq_num);
		old_mask |= recvIrqMaskSlave() << 8;
	} else if(irq_num <= INT_IRQ15) {
		/* ------------------------------------------------------------ */
		/* mask irq 8 - irq 15 for a slave				*/
		/* ------------------------------------------------------------ */
		old_mask = unmaskIrqSlave(irq_num) << 8;
		old_mask |= recvIrqMaskMaster();
	} else {
		uint8_t irqmask;
		/* ------------------------------------------------------------ */
		/* mask all for initialization use				*/
		/* ------------------------------------------------------------ */
		irqmask = (uint8_t)~PIC_INT_IRQ_ALL;
		old_mask = recvIrqMaskMaster();
		old_mask |= recvIrqMaskSlave() << 8;
		sendIrqMaskMaster(irqmask);
		sendIrqMaskSlave(irqmask);
	}
	//temp = recvIrqMaskMaster();

	return(old_mask);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:reqDisableIrq
 Input		:uint8_t irq_num
 		 < irq number to enable >
 Output		:void
 Return		:uint16_t
 		 < old flags : low byte is from master, high byte is from slave >
 Description	:request to unenable specified irq
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT uint8_t reqDisableIrq(uint8_t irq_num)
{
	uint16_t old_mask = 0;
	
	if (irq_num < INT_IRQ0 || INT_IRQ15 < irq_num) {
		if (irq_num != PIC_INT_IRQ_ALL) {
			return(old_mask);
		}
	} 

	if (irq_num <= INT_IRQ7) {
		/* ------------------------------------------------------------ */
		/* mask irq 0 - irq 7 for a master				*/
		/* ------------------------------------------------------------ */
		old_mask = maskIrqMaster(irq_num);
		old_mask |= recvIrqMaskSlave() << 8;
	} else if(irq_num <= INT_IRQ15) {
		/* ------------------------------------------------------------ */
		/* mask irq 8 - irq 15 for a slave				*/
		/* ------------------------------------------------------------ */
		old_mask = maskIrqSlave(irq_num) << 8;
		old_mask |= recvIrqMaskMaster();
	} else {
		uint8_t irqmask;
		/* ------------------------------------------------------------ */
		/* mask all for initialization use				*/
		/* ------------------------------------------------------------ */
		irqmask = PIC_INT_IRQ_ALL;
		old_mask = recvIrqMaskMaster();
		old_mask |= recvIrqMaskSlave() << 8;
		sendIrqMaskMaster(irqmask);
		sendIrqMaskSlave(irqmask);
	}

	return(old_mask);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:EndOfIntterrupt1
 Input		:void
 Output		:void
 Return		:void
 Description	:send eoi to only master for use of irq 0 to 7
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void EndOfInterrupt1(void)
{
	sendCommandMaster(OCW2_MASK_EOI);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:EndOfIntterrupt2
 Input		:void
 Output		:void
 Return		:void
 Description	:send eoi to master and slave for use of irq 8 to 15
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void EndOfInterrupt2(void)
{
	sendCommandMaster(OCW2_MASK_EOI);
	sendCommandSlave(OCW2_MASK_EOI);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:maskIrqMaster
 Input		:uint8_t irq_num
 		 < irq number to mask >
 Output		:void
 Return		:uint8_t
 		 < old mask >
 Description	:mask irq of master
==================================================================================
*/
LOCAL INLINE uint8_t maskIrqMaster(uint8_t irq_num)
{
	uint8_t irqmask;
	uint8_t old_mask;

	old_mask = recvIrqMaskMaster();
	irqmask = 1 << (irq_num - INT_IRQ0);
	irqmask |= old_mask;
	sendIrqMaskMaster(irqmask);

	return(old_mask);
}

/*
==================================================================================
 Funtion	:unmaskIrqMaster
 Input		:uint8_t irq_num
 		 < irq number to mask >
 Output		:void
 Return		:uint8_t
 		 < old mask >
 Description	:unmask irq of master
==================================================================================
*/
LOCAL INLINE uint8_t unmaskIrqMaster(uint8_t irq_num)
{
	uint8_t irqmask;
	uint8_t old_mask;

	old_mask = recvIrqMaskMaster();
	irqmask = 1 << (irq_num - INT_IRQ0);
	irqmask = old_mask & ~irqmask;
	sendIrqMaskMaster(irqmask);

	return(old_mask);
}

/*
==================================================================================
 Funtion	:maskIrqSlave
 Input		:uint8_t irq_num
 		 < irq number to mask >
 Output		:void
 Return		:uint8_t
 		 < old mask >
 Description	:mask irq of slave
==================================================================================
*/
LOCAL INLINE uint8_t maskIrqSlave(uint8_t irq_num)
{
	uint8_t irqmask;
	uint8_t old_mask;

	old_mask = recvIrqMaskSlave();
	irqmask = 1 << (irq_num - INT_IRQ8);
	irqmask |= old_mask;
	sendIrqMaskSlave(irqmask);


	return(old_mask);
}

/*
==================================================================================
 Funtion	:unmaskIrqSlave
 Input		:uint8_t irq_num
 		 < irq number to mask >
 Output		:void
 Return		:uint8_t
 		 < old mask >
 Description	:unmask irq of slave
==================================================================================
*/
LOCAL INLINE uint8_t unmaskIrqSlave(uint8_t irq_num)
{
	uint8_t irqmask;
	uint8_t old_mask;
	
	old_mask = recvIrqMaskSlave();
	irqmask = 1 << (irq_num - INT_IRQ8);
	irqmask = old_mask & ~irqmask;
	sendIrqMaskSlave(irqmask);

	return(old_mask);
}

/*
----------------------------------------------------------------------------------
	controls for master pic
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:sendCommandMaster
 Input		:uint8_t command
 		 < command data to send to a pic >
 Output		:void
 Return		:void
 Description	:send a command to a pic master
==================================================================================
*/
LOCAL INLINE void sendCommandMaster(uint8_t command)
{
	out_b(PIC_MST_COM_REG, command);
}

/*
==================================================================================
 Funtion	:recvStatusMaster
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received status >
 Description	:receive a status from a pic master
==================================================================================
*/
LOCAL INLINE uint8_t recvStatusMaster(void)
{
	return(in_b(PIC_MST_STS_REG));
}

/*
==================================================================================
 Funtion	:sendIrqMaskMaster
 Input		:uint8_t irqmask
 		 < irq mask flags to send to a pic >
 Output		:void
 Return		:void
 Description	:send a irq mask reques to a pic master
==================================================================================
*/
LOCAL INLINE void sendIrqMaskMaster(uint8_t irqmask)
{
	out_b(PIC_MST_IMR_REG, irqmask);
}

/*
==================================================================================
 Funtion	:recvIrqMaskMaster
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received data >
 Description	:receive mask data from a pic master
==================================================================================
*/
LOCAL INLINE uint8_t recvIrqMaskMaster(void)
{
	return(in_b(PIC_MST_IMR_REG));
}

/*
==================================================================================
 Funtion	:sendDataMaster
 Input		:uint8_t data
 		 < data to send to a pic >
 Output		:void
 Return		:void
 Description	:send data to a pic master
==================================================================================
*/
LOCAL INLINE void sendDataMaster(uint8_t data)
{
	out_b(PIC_MST_DAT_REG, data);
}

/*
==================================================================================
 Funtion	:recvDataMaster
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received data >
 Description	:receive data from a pic master
==================================================================================
*/
LOCAL INLINE uint8_t recvDataMaster(void)
{
	return(in_b(PIC_MST_DAT_REG));
}

/*
----------------------------------------------------------------------------------
	controls for slave pic
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:sendCommandSlave
 Input		:uint8_t command
 		 < command data to send to a pic >
 Output		:void
 Return		:void
 Description	:senda a command to a pic slave
==================================================================================
*/
LOCAL INLINE void sendCommandSlave(uint8_t command)
{
	out_b(PIC_SLV_COM_REG, command);
}

/*
==================================================================================
 Funtion	:recvStatusSlave
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received status >
 Description	:receive a status from a pic slave
==================================================================================
*/
LOCAL INLINE uint8_t recvStatusSlave(void)
{
	return(in_b(PIC_SLV_STS_REG));
}

/*
==================================================================================
 Funtion	:sendIrqMaskSlave
 Input		:uint8_t irqmask
 		 < irq mask flags to send to a pic >
 Output		:void
 Return		:void
 Description	:send a irq mask reques to a pic slave
==================================================================================
*/
LOCAL INLINE void sendIrqMaskSlave(uint8_t irqmask)
{
	out_b(PIC_SLV_IMR_REG, irqmask);
}

/*
==================================================================================
 Funtion	:recvIrqMaskSlave
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received data >
 Description	:receive mask data from a pic slave
==================================================================================
*/
LOCAL INLINE uint8_t recvIrqMaskSlave(void)
{
	return(in_b(PIC_SLV_IMR_REG));
}

/*
==================================================================================
 Funtion	:sendDataSlave
 Input		:uint8_t data
 		 < data to send to a pic >
 Output		:void
 Return		:void
 Description	:send data to a pic slave
==================================================================================
*/
LOCAL INLINE void sendDataSlave(uint8_t data)
{
	out_b(PIC_SLV_DAT_REG, data);
}

/*
==================================================================================
 Funtion	:recvDataSlave
 Input		:void
 Output		:void
 Return		:uint8_t
 		 < received data >
 Description	:receive data from a pic slave
==================================================================================
*/
LOCAL INLINE uint8_t recvDataSlave(void)
{
	return(in_b(PIC_SLV_DAT_REG));
}
