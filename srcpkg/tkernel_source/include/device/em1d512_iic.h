/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	em1d512_iic.h
 *
        IIC I/O interface definitions for EM1-D512
 */

#ifndef __DEVICE_EM1D512_IIC_H__
#define __DEVICE_EM1D512_IIC_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
        IIC I/O

IMPORT	ER	em1d512_iicxfer(W ch, UH *cmddat, W words);

        send/receive data to/from IIC device

                ch:     IIC Channel Number (0, 1)
                cmddat: pointer to the area to store command and sent/received data
                words:  word counts of command / data

                return value:    E_OK or errors (E_IO, E_TMOUT, E_PAR)

        this should not be called from interrupt handler, or from state where interrupt is disabled.


        command / data is described as follows.

        example 1)     one byte data is sent
		{
			IIC_START | (address << 1) | 0x00,	// R/W# = 0
			IIC_SEND  | IIC_TOPDATA  | IIC_LASTDATA | txdata0,
			IIC_STOP,
		}

                data to be sent is stored in the lower 8 bits of the word that specifies IIC_SEND.


        example 2) multiple bytes are sent
		{
			IIC_START | (address << 1) | 0x00,	// R/W# = 0
			IIC_SEND  | IIC_TOPDATA  | txdata0,
			IIC_SEND  |                txdata1
				:
			IIC_SEND  | IIC_LASTDATA | txdataN,
			IIC_STOP,
		}

        example 3) send and receive is switched
		{
			IIC_START | (address << 1) | 0x00,	// R/W# = 0
			IIC_SEND  | IIC_TOPDATA  | IIC_LASTDATA | txdata0,
			IIC_START | (address << 1) | 0x01,	// R/W# = 1
			IIC_RECV  | IIC_TOPDATA,		// rxdata0
			IIC_RECV,				// rxdata1
				:
			IIC_RECV  | IIC_LASTDATA,		// rxdataN
			IIC_STOP,
		}

                received data is stored into the lower 8 bits of the word that specifies IIC_RECV.
                (upper 8 bits remain the same.)
*/
/*
        SPI I/O

IMPORT	ER	em1d512_spixfer(W cs, UB *xmit, UB *recv, W len);

        perform send and/or receive to devices connected to SPI.
        sending and receiving proceed concurrently.

                cs:     communication target device (0 - 3)
                        used SPI channel is specified by ORing the following values.

				SP0: 0x0000
				SP1: 0x0100
				SP2: 0x0200
                xmit:   pointer to the area which stores the data to be sent
                recv:   pointer to the area that stores the received data.
                len:    length of sent/received data (byte)

                return value:    E_OK or errors (E_IO, E_TMOUT, E_PAR)

        this should not be called from interrupt handler, or from state where interrupt is disabled.

*/

/*
        definitions for automatic generation of interface library (mkiflib)
*/
/*** DEFINE_IFLIB
[INCLUDE FILE]
<device/em1d512_iic.h>

[PREFIX]
H8IO
***/

/*
        service functions for IIC I/O
*/
/* [BEGIN SYSCALLS] */

/* ORIGIN_NO 0x10 */
IMPORT	ER	em1d512_iicxfer(W ch, UH *cmddat, W words);
IMPORT	ER	em1d512_spixfer(W cs, UB *xmit, UB *recv, W len);
/* [END SYSCALLS] */

#define	IIC_START	(1 << 15)	// send START condition
#define	IIC_STOP	(1 << 14)	// send STOP condition
#define	IIC_SEND	(1 << 13)	// send data
#define	IIC_RECV	(1 << 12)	// receive data
#define	IIC_TOPDATA	(1 << 11)	// beginning of sent/received data
#define	IIC_LASTDATA	(1 << 10)	// end of sent/received data

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_EM1D512_IIC_H__ */
