/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2011/09/08.
 *
 *----------------------------------------------------------------------
 */

/*
 *	config.c
 *
 *       system-related processing / system configuration information
 *
 *       target: T-Engine/EM1D-512
 */

#include "sysdepend.h"
#include <arm/em1d512.h>

/* used device driver */
IMPORT	ER	initSIO_ns16550(SIOCB *, const CFGSIO *, W speed);
IMPORT	ER	initMemDisk(DISKCB *, const CFGDISK *);

/* memory region definition */
EXPORT	MEMSEG	MemSeg[] = {
	// Bank1/2/3
	{0x10000000, 0x30000000, MSA_IO,	PGA_RW|PGA_D |PGA_S|PGA_XN},
	// DDR2 SDRAM, 64Mbyte
	{0x30000000, 0x40000000, MSA_RAM,	PGA_RW|PGA_C},
        // EM1 internal device (1)
	{0x40000000, 0x70000000, MSA_IO,	PGA_RW|PGA_D |PGA_S|PGA_XN},
	// Bank0
	{0x70000000, 0x72000000, MSA_FROM,	PGA_RO|PGA_C |0x90000000},
        // EM1 internal SRAM
	{0xa0000000, 0xb0000000, MSA_SRAM,	PGA_RW|PGA_NC},
        // EM1 internal device (2)
	{0xb0000000, 0xd0000000, MSA_IO,	PGA_RW|PGA_D |PGA_S|PGA_XN},
        // EM1 internal Boot ROM
	{0xf0000000, 0xffffffff, MSA_ROM,	PGA_RO|PGA_NC},

	{0x70000000, 0x70020000, MSA_MON,	0},
	{0x70030000, 0x72000000, MSA_RDA,	0},
	{0x30006000, 0x34000000, MSA_OS,	0},
};

EXPORT	W	N_MemSeg = sizeof(MemSeg) / sizeof(MEMSEG);

/* unused memory region definition */
EXPORT	MEMSEG	NoMemSeg[] = {
	{0x00000000, 0x10000000, 0,		0},
	{0x72000000, 0xa0000000, 0,		0},
	{0xd0000000, 0xf0000000, 0,		0},
};

EXPORT	W	N_NoMemSeg = sizeof(NoMemSeg) / sizeof(MEMSEG);

/*
 * serial port configuration definition
 *       list in the order of port number
 */
EXPORT	const	CFGSIO	ConfigSIO[] = {
	{initSIO_ns16550, 0},
};

EXPORT	const W	N_ConfigSIO = sizeof(ConfigSIO) / sizeof(CFGSIO);


/*
 * disk drive configuration definition
 *	list in the order of port number
 */
EXPORT	const CFGDISK	ConfigDisk[] = {
	{"rda",	DA_RONLY,	initMemDisk,	0},	// FlashROM
};

EXPORT	const W	N_ConfigDisk = sizeof(ConfigDisk) / sizeof(CFGDISK);

/* boot information */
EXPORT	const UH	BootSignature = 0xe382;		// signature
EXPORT	UB *	const PBootAddr = (UB *)0x30200000;	// primary boot loader address
 
/* ------------------------------------------------------------------------ */

#define	IICC_IICE	(1 << 7)
#define	IICC_WREL	(1 << 5)
#define	IICC_WTIM	(1 << 3)
#define	IICC_ACKE	(1 << 2)
#define	IICC_STT	(1 << 1)
#define	IICC_SPT	(1 << 0)

#define	IICCL_SMC	(1 << 3)
#define	IICCL_DFC	(1 << 2)

#define	IICSE_MSTS	(1 << 15)
#define	IICSE_ALD	(1 << 14)
#define	IICSE_ACKD	(1 << 10)
#define	IICSE_SPD	(1 << 8)

#define	IICF_IICBSY	(1 << 6)
#define	IICF_STCEN	(1 << 1)
#define	IICF_IICRSV	(1 << 0)

#define	IIC_TOPDATA	(1 << 11)
#define	IIC_LASTDATA	(1 << 10)

#define	TIMEOUT		1000000	// microsec

#define	IIC2_IRQ	39
#define	IRQbit(x)	(1 << ((x) % 32))

/* wait for register state information */
LOCAL	ER	wait_state(UW addr, UW mask, UW value)
{
	W	i;

	for (i = TIMEOUT; i > 0; i--) {
		waitUsec(1);
		if ((in_w(addr) & mask) == value) break;
	}

	return i ? E_OK : E_TMOUT;
}

/* interrupt Raw status / clear */
LOCAL	void	clear_int(void)
{
	out_w(IT0_IIR, IRQbit(IIC2_IRQ));	// IRQ39 clear
	return;
}

/* interrupt Raw status / useable */
LOCAL	void	setup_int(void)
{
	out_w(IT_PINV_CLR1, IRQbit(IIC2_IRQ));
	out_w(IT0_IENS1, IRQbit(IIC2_IRQ));
	clear_int();
	return;
}

/* wait for interrupt Raw status */
LOCAL	ER	wait_int(void)
{
	ER	er;

	er = wait_state(IT0_RAW1, IRQbit(IIC2_IRQ), IRQbit(IIC2_IRQ));
	clear_int();

	return er;
}

/* start / restart */
LOCAL	ER	send_start(UB addr)
{
	ER	er;
	UW	sts;

        /* generate start condition */
	out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) & ~IICC_ACKE);
	out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) |  IICC_STT);

        /* wait for reserving a master */
	er = wait_state(IIC_IICSE(IIC2), IICSE_MSTS, IICSE_MSTS);
	if (er < E_OK) goto fin0;

        /* slave address / communication mode transmission */
	out_w(IIC_IIC(IIC2), addr);
	er = wait_int();
	if (er < E_OK) goto fin0;

        /* error check */
	sts = in_w(IIC_IICSE(IIC2));
	if ((sts & IICSE_ALD) || !(sts & IICSE_ACKD)) {
		er = E_IO;
		goto fin0;
	}

	er = E_OK;
fin0:
	return er;
}

/* stop */
LOCAL	ER	send_stop(void)
{
	ER	er;

        /* generate stop condition */
	out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) | IICC_SPT);

        /* wait for sending STOP bit(s) */
	er = wait_state(IIC_IICSE(IIC2), IICSE_SPD, IICSE_SPD);

	return er;
}

/* data transmission */
LOCAL	ER	send_data(UB data)
{
	ER	er;
	UW	sts;

        /* data transmission */
	out_w(IIC_IIC(IIC2), data);
	er = wait_int();
	if (er < E_OK) goto fin0;

        /* NAK check */
	sts = in_w(IIC_IICSE(IIC2));
	if (!(sts & IICSE_ACKD)) {
		er = E_IO;
		goto fin0;
	}

	er = E_OK;
fin0:
	return er;
}

/* data receive */
LOCAL	W	recv_data(W attr)
{
	W	er;

        /* when the first data is received, switch to receive mode */
	if (attr & IIC_TOPDATA) {
		out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) & ~IICC_WTIM);
		out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) |  IICC_ACKE);
	}

        /* instruct the reception of data */
	out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) | IICC_WREL);
	er = wait_int();
	if (er < E_OK) goto fin0;

        /* read data */
	er = in_w(IIC_IIC(IIC2)) & 0xff;
fin0:
        /* when an error occurs, or the last byte is seen, then perform the post processing */
	if ((attr & IIC_LASTDATA) || er < E_OK) {
		out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) |  IICC_WTIM);
		out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) & ~IICC_ACKE);
		out_w(IIC_IICC(IIC2), in_w(IIC_IICC(IIC2)) | IICC_WREL);
		wait_int();
	}

	return er;
}

/* start IIC send/receive */
LOCAL	ER	iic_start(void)
{
	ER	er;

        /* initialization default */
	out_w(IIC_IICC(IIC2), 0);			// stop completely
	out_w(IIC_IICCL(IIC2), IICCL_SMC | IICCL_DFC);	// fast mode + filter
	out_w(IIC_IICF(IIC2), IICF_STCEN | IICF_IICRSV);// forcibly start transmission
	out_w(IIC_IICC(IIC2), IICC_IICE | IICC_WTIM);	// IIC mode, 9bit mode
	clear_int();

        /* wait for bus to become available (since there is only one master, the bus is supposed to be unoccupied) */
	er = wait_state(IIC_IICF(IIC2), IICF_IICBSY, 0);

	return er;
}

/* stop IIC send/receive */
LOCAL	void	iic_finish(void)
{
	out_w(IIC_IICC(IIC2), 0);	// stop completely
	return;
}

/* read IIC-GPIO */
LOCAL	W	IICGPIORead(W addr)
{
	W	dat;

	setup_int();

	iic_start();
	send_start(addr);
	dat = recv_data(IIC_TOPDATA | IIC_LASTDATA);
	send_stop();
	iic_finish();

	clear_int();

	return dat;
}

/* IIC-GPIO write */
LOCAL	void	IICGPIOWrite(W addr, W dat)
{
	setup_int();

	iic_start();
	send_start(addr);
	send_data(dat);
	send_stop();
	iic_finish();

	clear_int();

	return;
}

/* ------------------------------------------------------------------------ */

IMPORT	W	pmicRead(W reg);
IMPORT	W	pmicWrite(W reg, W dat);
#define	pmicDelay(x)	waitUsec(4)	// about 16msec
#define	USBPowerOn	0xe0		// GPIO13(OD), High * power is supplied to A connector only
#define	USBPowerOff	0xe0		// GPIO13(OD), High

/* obtain DipSw status */
EXPORT	UW	DipSwStatus(void)
{
	UW	d;

        /* read data from read port */
	d = IICGPIORead(0xd9);

        /* unnecessary bits are masked and then invert logic. */
	d = (d ^ SW_MON) & SW_MON;

        /* check abort switch */
	if (in_w(GIO_I(GIO_L)) & 0x00000100) d |= SW_ABT;

	return d;
}

/* USB power control */
EXPORT	void	usbPower(BOOL power)
{
	pmicWrite(27, (pmicRead(27) & 0x0f) |
		  		(power ? USBPowerOn : USBPowerOff));
	pmicDelay();
}

/* power off */
EXPORT	void	powerOff(void)
{
	W	i;

	for (i = 10; i < 14; i++) pmicWrite(i, 0xff);	// IRQ_MASK_A-D (mask)
	pmicDelay();

	for (i = 5 ; i < 9; i++) pmicWrite(i, 0xff);	// EVENT_A-D (clear)
	pmicDelay();

	while (1) {
		pmicWrite(15, 0x60);	// DEEP_SLEEP
		pmicDelay();
	}
}

/* reset start*/
EXPORT	void	resetStart(void)
{
	while (1) {
                /* reset */
		pmicWrite(15, 0xac);		// SHUTDOWN
		pmicDelay();
	}
}

/* initialize hardware peripherals (executed only during reset) */
EXPORT	void	initHardware(void)
{
        /* enable abort switch interrupt */
	out_w(GIO_IDT1(GIO_L), 0x00000008);	// asynchronous leading-edge high interrupt
	out_w(GIO_IIR(GIO_L), 0x00000100);
	out_w(GIO_IIA(GIO_L), 0x00000100);
	out_w(GIO_IEN(GIO_L), 0x00000100);

	return;
}

/* LED on/off */
EXPORT	void	cpuLED(UW v)
{
	UB	m, d, r, c;

	m = ~((v >> 16) | 0xf0);	// mask (0:unmodified 1:modify)
	d = ~((v >>  0) | 0xf0);	// set value (0:on 1:off)
	r = IICGPIORead(0xb9);
	c = (r ^ d) & m;		// modify flag (0:unmodified 1:modify)
	IICGPIOWrite(0xb8, r ^ c);
}

/*
 * machine-dependent interrupt processing
 *       vec     interrupt vector number
 *       return value    0: unsupported target
 *               1: for the supported target, processing was performed. (monitor still continues)
 *               2: for the supported target, proceesing was performed (interrupt hander is exited)
 */
EXPORT	W	procHwInt(UW vec)
{
        /* only abort switch (GPIO(P8)) is supported */
	if (vec != EIT_GPIO(8)) return 0;

        /* clear interrupt */
	out_w(GIO_IIR(GIO_L), 0x00000100);

	DSP_S("Abort Switch (SW1) Pressed");
	return 1;
}

/* ------------------------------------------------------------------------ */

/*
        configure GPIO pin multiplexer

                * : used functions
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	GIO_P0      GIO_P0*
	GIO_P1      GIO_P1*       USB_WAKEUP    USB_PWR_FAULT
	GIO_P2      GIO_P2*
	GIO_P3      GIO_P3*
	GIO_P4      GIO_P4*                     NAND_RB1
	GIO_P5      GIO_P5                      NAND_RB2      CAM_SCLK*
	GIO_P6      GIO_P6*                     NAND_RB3
	GIO_P7      GIO_P7*                     NAND_CE0
	GIO_P8      GIO_P8*                     NAND_CE1
	GIO_P9      GIO_P9*                     NAND_CE2
	GIO_P10     GIO_P10*                    NAND_CE3
	AB0_CLK     GIO_P11       AB0_CLK*      NTS_CLK
	AB0_AD0     GIO_P12       AB0_AD0*
	AB0_AD1     GIO_P13       AB0_AD1*
	AB0_AD2     GIO_P14       AB0_AD2*
	AB0_AD3     GIO_P15       AB0_AD3*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	AB0_AD4     GIO_P16       AB0_AD4*
	AB0_AD5     GIO_P17       AB0_AD5*
	AB0_AD6     GIO_P18       AB0_AD6*
	AB0_AD7     GIO_P19       AB0_AD7*
	AB0_AD8     GIO_P20       AB0_AD8*
	AB0_AD9     GIO_P21       AB0_AD9*
	AB0_AD10    GIO_P22       AB0_AD10*
	AB0_AD11    GIO_P23       AB0_AD11*
	AB0_AD12    GIO_P24       AB0_AD12*
	AB0_AD13    GIO_P25       AB0_AD13*
	AB0_AD14    GIO_P26       AB0_AD14*
	AB0_AD15    GIO_P27       AB0_AD15*
	AB0_A17     GIO_P28       AB0_A17*
	AB0_A18     GIO_P29       AB0_A18*
	AB0_A19     GIO_P30       AB0_A19*
	AB0_A20     GIO_P31       AB0_A20*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	AB0_A21     GIO_P32       AB0_A21*
	AB0_A22     GIO_P33       AB0_A22*
	AB0_A23     GIO_P34       AB0_A23*
	AB0_A24     GIO_P35       AB0_A24*
	AB0_A25     GIO_P36*      AB0_A25
	AB0_A26     GIO_P37*      AB0_A26
	AB0_ADV     GIO_P38       AB0_ADV*
	AB0_RDB     GIO_P39       AB0_RDB*      NTS_DATA3
	AB0_WRB     GIO_P40       AB0_WRB*      NTS_DATA4
	AB0_WAIT    GIO_P41       AB0_WAIT*     NTS_DATA5
	AB0_CSB0    GIO_P42       AB0_CSB0*     NTS_DATA6
	AB0_CSB1    GIO_P43       AB0_CSB1*     NTS_DATA7
	AB0_CSB2    GIO_P44*      AB0_CSB2      NTS_VS
	AB0_CSB3    GIO_P45       AB0_CSB3*     NTS_HS
	AB0_BEN0    GIO_P46       AB0_BEN0*
	AB0_BEN1    GIO_P47       AB0_BEN1*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	SP0_CS1     GIO_P48       SP0_CS1*
	SP0_CS2     GIO_P49       SP0_CS2*
	LCD_PXCLK   GIO_P50       LCD_PXCLK*
	LCD_R0      GIO_P51       LCD_R0*
	LCD_R1      GIO_P52       LCD_R1*
	LCD_R2      GIO_P53       LCD_R2*
	LCD_R3      GIO_P54       LCD_R3*
	LCD_R4      GIO_P55       LCD_R4*
	LCD_R5      GIO_P56       LCD_R5*
	LCD_G0      GIO_P57       LCD_G0*
	LCD_G1      GIO_P58       LCD_G1*
	LCD_G2      GIO_P59       LCD_G2*
	LCD_G3      GIO_P60       LCD_G3*
	LCD_G4      GIO_P61       LCD_G4*
	LCD_G5      GIO_P62       LCD_G5*
	LCD_B0      GIO_P63       LCD_B0*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	LCD_B1      GIO_P64       LCD_B1*
	LCD_B2      GIO_P65       LCD_B2*
	LCD_B3      GIO_P66       LCD_B3*
	LCD_B4      GIO_P67       LCD_B4*
	LCD_B5      GIO_P68       LCD_B5*
	LCD_HSYNC   GIO_P69       LCD_HSYNC*
	LCD_VSYNC   GIO_P70       LCD_VSYNC*
	LCD_ENABLE  GIO_P71       LCD_ENABLE*
	NTS_CLK     GIO_P72*      NTS_CLK                     PM1_CLK
	NTS_VS      GIO_P73*      NTS_VS        SP1_CLK
	NTS_HS      GIO_P74*      NTS_HS        SP1_SI
	NTS_DATA0   GIO_P75       NTS_DATA0     SP1_SO        CAM_YUV0*
	NTS_DATA1   GIO_P76       NTS_DATA1     SP1_CS0       CAM_YUV1*
	NTS_DATA2   GIO_P77       NTS_DATA2     SP1_CS1       CAM_YUV2*
	NTS_DATA3   GIO_P78       NTS_DATA3     SP1_CS2       CAM_YUV3*
	NTS_DATA4   GIO_P79       NTS_DATA4     SP1_CS3       CAM_YUV4*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	NTS_DATA5   GIO_P80*      NTS_DATA5     SP1_CS4       PM1_SEN
	NTS_DATA6   GIO_P81*      NTS_DATA6     SP1_CS5       PM1_SI
	NTS_DATA7   GIO_P82*      NTS_DATA7                   PM1_SO
	IIC_SCL     GIO_P83       IIC_SCL*
	IIC_SDA     GIO_P84       IIC_SDA*
	URT0_CTSB   GIO_P85       URT0_CTSB     URT1_SRIN*
	URT0_RTSB   GIO_P86       URT0_RTSB     URT1_SOUT*
	PM0_SI      GIO_P87       PM0_SI*
	SD0_DATA1   GIO_P88       SD0_DATA1*
	SD0_DATA2   GIO_P89       SD0_DATA2*
	SD0_DATA3   GIO_P90       SD0_DATA3*
	SD0_CKI     GIO_P91       SD0_CKI*
	SD1_CKI     GIO_P92       SD1_CKI       CAM_CLKI*
	SD2_CKI     GIO_P93       SD2_CKI*      NAND_OE
	PWM0        GIO_P94*      PWM0
	PWM1        GIO_P95*      PWM1
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	USB_CLK     GIO_P96       USB_CLK*
	USB_DATA0   GIO_P97       USB_DATA0*
	USB_DATA1   GIO_P98       USB_DATA1*
	USB_DATA2   GIO_P99       USB_DATA2*
	USB_DATA3   GIO_P100      USB_DATA3*
	USB_DATA4   GIO_P101      USB_DATA4*
	USB_DATA5   GIO_P102      USB_DATA5*
	USB_DATA6   GIO_P103      USB_DATA6*
	USB_DATA7   GIO_P104      USB_DATA7*
	USB_DIR     GIO_P105      USB_DIR*
	USB_STP     GIO_P106      USB_STP*
	USB_NXT     GIO_P107      USB_NXT*
	URT2_SRIN   GIO_P108      URT2_SRIN*
	URT2_SOUT   GIO_P109      URT2_SOUT*
	URT2_CTSB   GIO_P110      URT2_CTSB*
	URT2_RTSB   GIO_P111      URT2_RTSB*
	
        pin name      function 0(00)     function1(01)     function2(10)     function3(11)
	SD2_CKO     GIO_P112      SD2_CKO*       NAND_D2
	SD2_CMD     GIO_P113      SD2_CMD*       NAND_D3
	SD2_DATA0   GIO_P114      SD2_DATA0*     NAND_D4
	SD2_DATA1   GIO_P115      SD2_DATA1*     NAND_D5
	SD2_DATA2   GIO_P116      SD2_DATA2*     NAND_D6
	SD2_DATA3   GIO_P117      SD2_DATA3*     NAND_D7
*/
EXPORT	const UW	GPIOConfig[] __attribute__((section(".startup"))) = {
	CHG_PINSEL_G(0),
	0x55400C00,		// AB0_CLK,AB0_AD3-0,CAM_SCLK
	CHG_PINSEL_G(16),
	0x55555555,		// AB0_AD15-4,AB0_A20-17
	CHG_PINSEL_G(32),
	0x54555055,		// AB0_BEN1-0,AB0_CSB3,AB0_CSB1-0,
				// AB0_WAIT,AB0_WRB,AB0_RDB,AB0_ADV,
				// AB0_A24-21

	CHG_CTRL_AB0_BOOT,	// AB0(AsyncBus0) pin:
	0x00000001,		// 	configured by PINSEL

	CHG_PINSEL_G(48),
	0x55555555,		// LCD,SP0_CS2-1
	CHG_PINSEL_G(64),
	0xffc05555,		// CAM_YUV4-0,LCD
	CHG_PINSEL_G(80),
	0x06556940,		// SD2_CKI,CAM_CLKI,SD0_CKI,SD0_DATA3-1,
				// PM0,URT1,IIC
	CHG_PINSEL_G(96),
	0x55555555,		// URT2,USB
	CHG_PINSEL_G(112),
	0x00000555,		// SD2
	CHG_PINSEL_SP0,
	0x00000000,
	CHG_PINSEL_DTV,
	0x00000001,
	CHG_PINSEL_SD0,
	0x00000000,
	CHG_PINSEL_SD1,
	0x00000002,
	CHG_PINSEL_IIC2,
	0x00000000,
	CHG_PULL_G(0),
	0x55055005,		// P7,P6,P4,P3,P0: IN, pull-up/down dis
	CHG_PULL_G(8),
	0x00000005,		// P8: IN, pull-up/down dis
	CHG_PULL_G(16),
	0x00000000,		// (default)
	CHG_PULL_G(24),
	0x00000000,		// (default)
	CHG_PULL_G(32),
	0x00550000,		// P37,36: IN, pull-up/down dis
	CHG_PULL_G(40),
	0x00050000,		// P44: IN, pull-up/down dis
	CHG_PULL_G(48),
	0x11111111,		// (default)
	CHG_PULL_G(56),
	0x11111111,		// (default)
	CHG_PULL_G(64),
	0x11111111,		// (default)
	CHG_PULL_G(72),
	0x00000005,		// P72: IN, pull-up/down dis
	CHG_PULL_G(80),
	0x00400050,		// P81: IN, pull-up/down dis
				// URT1_SRIN: IN, pull-down
	CHG_PULL_G(88),
	0x55000444,		// P95,94: IN, pull-up/down dis
				// SD0_DATA3-1: IN, pull-down
	CHG_PULL_G(96),
	0x44444444,		// USB signals: IN, pull-down
	CHG_PULL_G(104),
	0x04044444,		// USB signals: IN, pull-down
				// URT2_CTSB,URT2_SRIN: IN, pull-down
	CHG_PULL_G(112),
	0x00000000,		// (default)
	CHG_PULL_G(120),
	0x00000000,		// (default)

	CHG_PULL(0),
	0x50000004,		// URT0_SRIN: IN, pull-up/down dis
				// DEBUG_EN: IN, pull-down
	CHG_PULL(1),
	0x15110600,		// SP0_SO: OUT, pull-up/down dis
				// SP0_SI: IN, pull-up/down dis
				// SP0_CS: OUT, pull-up/down dis
				// SP0_CK: OUT, pull-up/down dis
				// JT0C: IN, pull-up
				// JT0B: OUT, pull-down
				// JT0A: OUT, pull-down
	CHG_PULL(2),
	0x60000661,		// PM0_SEN: IN, pull-up
				// SD0_DAT: IN, pull-up
				// SD1_CMD: IN, pull-up
				// SD0_CLK: OUT, pull-up/down dis
	CHG_PULL(3),
	0x00000000,		// (default)

	GIO_E0(GIO_L),
	0x000001d9,		// P8,P7,P6,P4,P3,P0: IN
	GIO_E1(GIO_L),
	0x00000604,		// P10,P9,P2: OUT
	GIO_E0(GIO_H),
	0x00001030,		// P44,P37,P36: IN
	GIO_E1(GIO_H),
	0x00000000,		// (default)
	GIO_E0(GIO_HH),
	0xc0020100,		// P95,P94,P81,P72:IN
	GIO_E1(GIO_HH),
	0x00040200,		// P82,P73: OUT
	GIO_OL(GIO_L),
	0x06040000,		// P10,P9,P2=0
	GIO_OL(GIO_HH),
	0x02000000,		// P73=0
	GIO_OH(GIO_HH),
	0x00040000,		// P82=0

	0x00000000,		// (terminate)
	0x00000000,
};
