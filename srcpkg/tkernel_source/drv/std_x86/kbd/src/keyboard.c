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

#include <tk/typedef.h>
#include <tk/kernel.h>
#include <stdint.h>

#include <device/port.h>
#include <device/std_x86/kbd.h>
#include <device/std_x86/pic.h>
#include <cpu/x86/cpu_insn.h>


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
static INLINE uint8_t read_kbd_ctl(void);
LOCAL ER write_kbd_ctl(uint8_t data);
LOCAL INLINE uint8_t read_kbd_enc(void);
LOCAL ER write_kbd_enc(uint8_t data);
LOCAL int get_kbd_queue_size(void);
LOCAL ER write_kbd_queue(uint8_t data);
LOCAL ER read_one_byte(void);

/*
----------------------------------------------------------------------------------
	scan code analysis functions
----------------------------------------------------------------------------------
*/
LOCAL uint8_t to_capital_code(uint8_t scode);
LOCAL void single_code_make(int code);
LOCAL void single_code_break(int code);
LOCAL void ex0_2nd_code(int code);
LOCAL void ex0_3rd_code(int code);
LOCAL void ex1_2nd_code(int code);

/*
----------------------------------------------------------------------------------
	keyboard interrupt handler
----------------------------------------------------------------------------------
*/
LOCAL void kbd_intterupt(struct ctx_reg *reg);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	KBD_MAX_RETRY		10000

/*
----------------------------------------------------------------------------------
	data queue of keyboard
----------------------------------------------------------------------------------
*/
#define	KBD_QUEUE_MAX	32

struct keyboard_queue {
	/* keyboard analyzed data */
	uint8_t		data[ KBD_QUEUE_MAX];
	/* queue write pointer */
	uint32_t	write;
	/* queue read pointer */
	uint32_t	read;
	/* queue data size */
	uint32_t	size;
	/* data over flow flag */
	int32_t		overflow;
};

/*
----------------------------------------------------------------------------------
	analyzing states of key scan
----------------------------------------------------------------------------------
*/
enum analyze_state {
	SINGLE_CODE_MAKE,
	SINGLE_CODE_BREAK,
	
	EX0_2ND_CODE,
	EX0_3RD_CODE,
	
	EX1_2ND_CODE,
	
	ANALYZED_STATE_NUM,
};

/*
----------------------------------------------------------------------------------
	extend scan code definitions
----------------------------------------------------------------------------------
*/
#define	KBD_SCODE_BREAK			0x80

#define	SCODE_MK_EX0			0xE0	/* E0 make code			*/
#define	SCODE_MK_EX1			0xE1	/* E1 make code			*/

#define	SCODE_BK_EX0			0xE0	/* E0 break code		*/
#define	SCODE_BK_EX1			0xE1	/* E1 break code		*/

#define	SCODE_MK_2_R_ALT		0x38	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_R_ALT		(SCODE_MK_2_R_ALT | KBD_SCODE_BREAK)

#define	SCODE_MK_2_R_CTRL		0x1D	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_R_CTRL		(SCODE_MK_2_R_CTRL | KBD_SCODE_BREAK)

#define	SCODE_MK_2_INSERT		0x52	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_INSERT		(SCODE_MK_2_INSERT | KBD_SCODE_BREAK)

#define	SCODE_MK_2_DELETE		0x53	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_DELETE		(SCODE_MK_2_DELETE | KBD_SCODE_BREAK)

#define	SCODE_MK_2_L_ARROW		0x4B	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_L_ARROW		(SCODE_MK_2_L_ARROW | KBD_SCODE_BREAK)

#define	SCODE_MK_2_HOME			0x47	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_HOME			(SCODE_MK_2_HOME | KBD_SCODE_BREAK)

#define	SCODE_MK_2_END			0x4F	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_END			(SCODE_MK_2_END | KBD_SCODE_BREAK)

#define	SCODE_MK_2_UP_ARROW		0x48	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_UP_ARROW		(SCODE_MK_2_UP_ARROW | KBD_SCODE_BREAK)

#define	SCODE_MK_2_DN_ARROW		0x50	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_DN_ARROW		(SCODE_MK_2_DN_ARROW | KBD_SCODE_BREAK)

#define	SCODE_MK_2_PAGE_UP		0x49	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_PAGE_UP		(SCODE_MK_2_PAGE_UP | KBD_SCODE_BREAK)

#define	SCODE_MK_2_PAGE_DN		0x51	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_PAGE_DN		(SCODE_MK_2_PAGE_DN | KBD_SCODE_BREAK)

#define	SCODE_MK_2_R_ARROW		0x4D	/* 2nd code followed by E0	*/
#define	SCODE_BK_2_R_ARROW		(SCODE_MK_2_R_ARROW | KBD_SCODE_BREAK)

#define	SCODE_MK_2_NUM_ON_FINAL_KEY	0x2A	/* final key on num-lock on	*/
#define	SCODE_BK_2_NUM_ON_FINAL_KEY	(SCODE_MK_2_NUM_ON_FINAL_KEY | KBD_SCODE_BREAK)

#define	SCODE_BK_2_NUM_OFF_LSHIFT	0x2A
#define	SCODE_MK_2_NUM_OFF_LSHIFT	(SCODE_BK_2_NUM_OFF_LSHIFT | KBD_SCODE_BREAK)

#define	SCODE_BK_2_NUM_OFF_RSHIFT	0x36
#define	SCODE_MK_2_NUM_OFF_RSHIFT	(SCODE_BK_2_NUM_OFF_RSHIFT | KBD_SCODE_BREAK)


#define	SCODE_MK_3_NUM_OFF_L_SHIFT	0x12	/* 3nd code when num-lock off	*/
#define	SCODE_MK_3_NUM_OFF_R_SHIFT	0x59	/* 3nd code when num-lock off	*/


#define	SCODE_MK_2_NUMERIC_DIVIDE	0x35
#define	SCODE_BK_2_NUMERIC_DIVIDE	(SCODE_MK_2_NUMERIC_DIVIDE | KBD_SCODE_BREAK)

#define	SCODE_MK_2_PRTSCREEN		0x2A
#define	SCODE_BK_2_PRTSCREEN		(SCODE_MK_2_PRTSCREEN | KBD_SCODE_BREAK)
#define	SCODE_MK_4_PRTSCREEN		0x37
#define	SCODE_BK_4_PRTSCREEN		(SCODE_MK_4_PRTSCREEN | KBD_SCODE_BREAK)

#define	SCODE_MK_2_LR_CTRL		0x46
#define	SCODE_MK_4_LR_CTRL		0xC6

/*
----------------------------------------------------------------------------------
	states of keyboard
----------------------------------------------------------------------------------
*/
struct keyboard_state{
	/* capslock state							*/
	uint32_t		capslock:1;
	/* alt state								*/
	uint32_t		alt:1;
	/* shift state								*/
	uint32_t		shift:1;
	/* control state							*/
	uint32_t		control:1;
	/* numlock state							*/
	uint32_t		numlock:1;
	/* scrolllock state							*/
	uint32_t		scrolllock:1;
	/* windows key state							*/
	uint32_t		win:1;
	/* insert key state							*/
	uint32_t		insert:1;
	/* katakana key state							*/
	uint32_t		katakana:1;
	/* analyzed state							*/
	enum analyze_state	analyze_state;
	ID			flgid;
};

#define	KBD_IN_EVENT		0x00000001

/*
----------------------------------------------------------------------------------
	ascii definitions for single key table
----------------------------------------------------------------------------------
*/
#define KEY_SPACE		' '
#define KEY_0			'0'
#define KEY_1			'1'
#define KEY_2			'2'
#define KEY_3			'3'
#define KEY_4			'4'
#define KEY_5			'5'
#define KEY_6			'6'
#define KEY_7			'7'
#define KEY_8			'8'
#define KEY_9			'9'

#define KEY_A			'a'
#define KEY_B			'b'
#define KEY_C			'c'
#define KEY_D			'd'
#define KEY_E			'e'
#define KEY_F			'f'
#define KEY_G			'g'
#define KEY_H			'h'
#define KEY_I			'i'
#define KEY_J			'j'
#define KEY_K			'k'
#define KEY_L			'l'
#define KEY_M			'm'
#define KEY_N			'n'
#define KEY_O			'o'
#define KEY_P			'p'
#define KEY_Q			'q'
#define KEY_R			'r'
#define KEY_S			's'
#define KEY_T			't'
#define KEY_U			'u'
#define KEY_V			'v'
#define KEY_W			'w'
#define KEY_X			'x'
#define KEY_Y			'y'
#define KEY_Z			'z'

#define KEY_RETURN		'\n'
#define KEY_ESCAPE		0x1B
#define KEY_BACKSPACE		'\b'
#define	KEY_DELETE		0x10

#define KEY_DOT			'.'
#define KEY_COMMA		','
#define KEY_COLON		':'
#define KEY_SEMICOLON		';'
#define KEY_SLASH		'/'
#define KEY_BACKSLASH		'\\'
#define KEY_PLUS		'+'
#define KEY_MINUS		'-'
#define KEY_ASTERISK		'*'
#define KEY_EXCLAMATION		'!'
#define KEY_QUESTION		'\?'
#define KEY_QUOTEDOUBLE		'\"'
#define KEY_QUOTE		'\''
#define KEY_EQUAL		'='
#define KEY_HASH		'#'
#define KEY_PERCENT		'%'
#define KEY_AMPERSAND		'&'
#define KEY_UNDERSCORE		'_'
#define KEY_LEFTPARENTHESIS	'('
#define KEY_RIGHTPARENTHESIS	')'
#define KEY_LEFTBRACKET		'['
#define KEY_RIGHTBRACKET	']'
#define KEY_LEFTCURL		'{'
#define KEY_RIGHTCURL		'}'
#define KEY_DOLLAR		'$'
#define KEY_LESS		'<'
#define KEY_GREATER		'>'
#define KEY_BAR			'|'
#define KEY_GRAVE		'`'
#define KEY_TILDE		'~'
#define KEY_AT			'@'
#define KEY_CARRET		'^'

#define KEY_NUM_0		'0'
#define KEY_NUM_1		'1'
#define KEY_NUM_2		'2'
#define KEY_NUM_3		'3'
#define KEY_NUM_4		'4'
#define KEY_NUM_5		'5'
#define KEY_NUM_6		'6'
#define KEY_NUM_7		'7'
#define KEY_NUM_8		'8'
#define KEY_NUM_9		'9'
#define KEY_NUM_PLUS		'+'
#define KEY_NUM_MINUS		'-'
#define KEY_NUM_DECIMAL		'.'
#define KEY_NUM_DIVIDE		'/'
#define KEY_NUM_ASTERISK	'*'
#define	KEY_NUM_ENTER		'\n'

#define KEY_TAB			'\t'

#define	KEY_ASCII_MAX		0x80

#define	KEY_F1			0xF1
#define	KEY_F2			0xF2
#define	KEY_F3			0xF3
#define	KEY_F4			0xF4
#define	KEY_F5			0xF5
#define	KEY_F6			0xF6
#define	KEY_F7			0xF7
#define	KEY_F8			0xF8
#define	KEY_F9			0xF9
#define	KEY_F10			0xFA
#define	KEY_F11			0xFB
#define	KEY_F12			0xFC


#define	KEY_CAPS_LOCK		0x90
#define	KEY_ALT			0x91
#define	KEY_L_ALT		KEY_ALT
#define	KEY_R_ALT		KEY_ALT
#define	KEY_SHIFT		0x92
#define	KEY_L_SHIFT		KEY_SHIFT
#define	KEY_R_SHIFT		KEY_SHIFT
#define	KEY_CTRL		0x93
#define	KEY_L_CTRL		KEY_CTRL
#define	KEY_R_CTRL		KEY_CTRL
#define	KEY_NUM_LOCK		0x94
#define	KEY_SCROLL_LOCK		0x95
#define	KEY_WIN			0x96
#define	KEY_LEFT_WIN		KEY_WIN
#define	KEY_RIGHT_WIN		KEY_WIN
#define	KEY_KATAKANA		0x97
#define	KEY_CONVERT		NULL
#define	KEY_NONCONVERT		NULL

#define KEY_UNKNOWN		NULL
#define	KEY_DO_NOT_USE		NULL
#define	KEY_ACPI_WAKE		NULL
#define	KEY_PAUSE		NULL
#define	KEY_APPLICATION		NULL
#define	KEY_ACPI_POWER		NULL
#define	KEY_SBCSCHAR		NULL



/*
==================================================================================

	Description :Keyboard controller register
	
	bit number	value	description
	0		0	Output buffer empty 
			1	Output buffer full
	1		0	Input buffer empty
			1	Input buffer full
	2		0	Set after power on reset 
			1	Set after successfull completion of 
				the keyboard controllers self-test 
	3		0	Last write to input buffer was data (via port 0x60) 
			1	Last write to input buffer was a command (via port 0x64)
	4		0	Locked 
			1	Not locked 
	5		0	PS/2 Systems: Determins if read from port 0x60 is 
				valid If valid, 0=Keyboard data 
			1	PS/2 Systems: Mouse data, only if you can read 
				from port 0x60 
	6		0	OK flag 
			1	PS/2 Systems: General Timeout
	7		0	OK flag, no error 
			1	Parity error with last byte 

==================================================================================
*/
#define	KBD_MASK_OUT_BUF	0x01
#define	KBD_MASK_IN_BUF		0x02
#define	KBD_MASK_SYS_FLG	0x04
#define	KBD_MASK_COM_DAT	0x08
#define	KBD_MASK_LOCKED		0x10
#define	KBD_MASK_AUX_BUF	0x20
#define	KBD_MASK_TOUT		0x40
#define	KBD_MASK_PARITY		0x80

/*
==================================================================================

	Management 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	single key table
----------------------------------------------------------------------------------
*/
LOCAL uint8_t single_key_table[ ] = {
	/* key */		/* scan code */
	KEY_UNKNOWN,		/* 0x00 */	KEY_ESCAPE,		/* 0x01 */
	KEY_1,			/* 0x02 */	KEY_2,			/* 0x03 */
	KEY_3,			/* 0x04 */	KEY_4,			/* 0x05 */
	KEY_5,			/* 0x06 */	KEY_6,			/* 0x07 */
	KEY_7,			/* 0x08 */	KEY_8,			/* 0x09 */
	KEY_9,			/* 0x0A */	KEY_0,			/* 0x0B */
	KEY_MINUS,		/* 0x0C */	KEY_EQUAL,		/* 0x0D */
	KEY_BACKSPACE,		/* 0x0E */	KEY_TAB,		/* 0x0F */
	KEY_Q,			/* 0x10 */	KEY_W,			/* 0x11 */
	KEY_E,			/* 0x12 */	KEY_R,			/* 0x13 */
	KEY_T,			/* 0x14 */	KEY_Y,			/* 0x15 */
	KEY_U,			/* 0x16 */	KEY_I,			/* 0x17 */
	KEY_O,			/* 0x18 */	KEY_P,			/* 0x19 */
	KEY_LEFTBRACKET,	/* 0x1A */	KEY_RIGHTBRACKET,	/* 0x1B */
	KEY_RETURN,		/* 0x1C */	KEY_L_CTRL,		/* 0x1D */
	KEY_A,			/* 0x1E */	KEY_S,			/* 0x1F */
	KEY_D,			/* 0x20 */	KEY_F,			/* 0x21 */
	KEY_G,			/* 0x22 */	KEY_H,			/* 0x23 */
	KEY_J,			/* 0x24 */	KEY_K,			/* 0x25 */
	KEY_L,			/* 0x26 */	KEY_SEMICOLON,		/* 0x27 */
	KEY_QUOTE,		/* 0x28 */	KEY_GRAVE,		/* 0x29 */
	KEY_L_SHIFT,		/* 0x2A */	KEY_BACKSLASH,		/* 0x2B */
	KEY_Z,			/* 0x2C */	KEY_X,			/* 0x2D */
	KEY_C,			/* 0x2E */	KEY_V,			/* 0x2F */
	KEY_B,			/* 0x30 */	KEY_N,			/* 0x31 */
	KEY_M,			/* 0x32 */	KEY_COMMA,		/* 0x33 */
	KEY_DOT,		/* 0x34 */	KEY_SLASH,		/* 0x35 */
	KEY_R_SHIFT,		/* 0x36 */	KEY_NUM_ASTERISK,	/* 0x37 */
	KEY_L_ALT,		/* 0x38 */	KEY_SPACE,		/* 0x39 */
	KEY_CAPS_LOCK,		/* 0x3A */	KEY_F1,			/* 0x3B */
	KEY_F2,			/* 0x3C */	KEY_F3,			/* 0x3D */
	KEY_F4,			/* 0x3E */	KEY_F5,			/* 0x3F */
	KEY_F6,			/* 0x40 */	KEY_F7,			/* 0x41 */
	KEY_F8,			/* 0x42 */	KEY_F9,			/* 0x43 */
	KEY_F10,		/* 0x44 */	KEY_NUM_LOCK,		/* 0x45 */
	KEY_SCROLL_LOCK,	/* 0x46 */	KEY_NUM_7,		/* 0x47 */
	KEY_NUM_8,		/* 0x48 */	KEY_NUM_9,		/* 0x49 */
	KEY_NUM_MINUS,		/* 0x4A */	KEY_NUM_4,		/* 0x4B */
	KEY_NUM_5,		/* 0x4C */	KEY_NUM_6,		/* 0x4D */
	KEY_NUM_PLUS,		/* 0x4E */	KEY_NUM_1,		/* 0x4F */
	KEY_NUM_2,		/* 0x50 */	KEY_NUM_3,		/* 0x51 */
	KEY_NUM_0,		/* 0x52 */	KEY_NUM_DECIMAL,	/* 0x53 */
	KEY_UNKNOWN,		/* 0x54 */	KEY_UNKNOWN,		/* 0x55 */
	KEY_UNKNOWN,		/* 0x56 */	KEY_F11,		/* 0x57 */
	KEY_F12			/* 0x58 */
};

/*
----------------------------------------------------------------------------------
	keyboard state
----------------------------------------------------------------------------------
*/
volatile LOCAL struct keyboard_state kbd_state;

/*
----------------------------------------------------------------------------------
	keyboard data queue
----------------------------------------------------------------------------------
*/
volatile LOCAL struct keyboard_queue kbd_queue;
volatile CONST T_CFLG kbd_cflg = {
	.exinf		= NULL,
	.flgatr		= TA_TFIFO | TA_WSGL | TWF_BITCLR,
	.iflgptn	= 0,
	.dsname		= "kbdeflg",
};

/*
----------------------------------------------------------------------------------
	scan code analysis functions
----------------------------------------------------------------------------------
*/
LOCAL void (*scan_code_func[ANALYZED_STATE_NUM])(int) = {
	[SINGLE_CODE_MAKE]	= single_code_make,
	[SINGLE_CODE_BREAK]	= single_code_break,
	[EX0_2ND_CODE]		= ex0_2nd_code,
	[EX0_3RD_CODE]		= ex0_3rd_code,
	[EX1_2ND_CODE]		= ex1_2nd_code,
};


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initKeyboard
 Input		:void
 Output		:void
 Return		:ER
		 <error code>
 Description	:initialize keyboard driver
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER initKeyboard(void)
{
	ER err;
	
	memset((void*)&kbd_queue, 0x00, sizeof(struct keyboard_queue));
	kbd_queue.size = KBD_QUEUE_MAX;
	kbd_queue.overflow = FALSE;
	
	memset((void*)&kbd_state, 0x00, sizeof(struct keyboard_state));
	kbd_state.analyze_state = SINGLE_CODE_MAKE;
	
	kbd_state.flgid = tk_cre_flg((CONST T_CFLG*)&kbd_cflg);
	
	if (kbd_state.flgid < 0) {
		vd_printf("err:cannot create event flag for keyboard\n");
		return(kbd_state.flgid);
	}
	
	/* -------------------------------------------------------------------- */
	/* register keyboard interrupt handler for irq 1			*/
	/* -------------------------------------------------------------------- */
	err = register_int_handler(INT_IRQ1, kbd_intterupt);

	if (err) {
		vd_printf("err[%d]:cannot register pit interrupt handler\n", err);
		return(err);
	}
	
	/* -------------------------------------------------------------------- */
	/* enable irq 1								*/
	/* -------------------------------------------------------------------- */
	reqEnableIrq(INT_IRQ1);
	
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kbd_in
 Input		:void
 Output		:void
 Return		:ER
		 <read length or error code>
 Description	:get ascii which is read from keyboard and converted to
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER kbd_in(B *buf, UW len)
{
	ER err;
	UW i;
	int read_len = 0;
	
	/* Check the address */
#if 0
	/* check is done at read system call */
	if (ChkSpaceRW((void*)buf, len)) {
		return 0;
	}
#endif
	//return(read_one_byte());
	
	
	for (i = 0;i < len;i++) {
		err = read_one_byte();
		
		if (err == '\r' || err == '\n') {
			*(buf + i) = '\n';
			read_len++;
			//*(buf + i + 1) = '\0';
			//read_len++;
			return(read_len);
		}
		
		if (0 <= err) {
			*(buf + i) = (B)err;
			read_len++;
			
		} else {
			return(err);
		}
		if (!get_kbd_queue_size()) {
			return(read_len);
		}
	}
	
	//vd_printf("%s\n", buf);
	
	return(read_len);
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:read_kbd_ctl
 Input		:void
 Output		:void
 Return		:uint8_t
		 <read data>
 Description	:read from keyboard controller
==================================================================================
*/
LOCAL INLINE uint8_t read_kbd_ctl(void)
{
	return(in_b(KBD_CTL_READ_STS));
}

/*
==================================================================================
 Funtion	:write_kbd_ctl
 Input		:uint8_t
		 <write data>
 Output		:void
 Return		:ER
		 <error code>
 Description	:write to keyboard controller
==================================================================================
*/
LOCAL ER write_kbd_ctl(uint8_t data)
{
	int i;
	
	for (i = 0;i < KBD_MAX_RETRY;i++) {
		if(!(read_kbd_ctl() & KBD_MASK_OUT_BUF)) {
			out_b(KBD_CTL_WRITE_REG, data);
			return(E_OK);
		}
	}
	
	return(E_BUSY);
}

/*
==================================================================================
 Funtion	:read_kbd_enc
 Input		:void
 Output		:void
 Return		:uint8_t
		 <read data>
 Description	:read from keyboard encoder
==================================================================================
*/
LOCAL INLINE uint8_t read_kbd_enc(void)
{
	return(in_b(KBD_ENC_READ_BUF));
}

/*
==================================================================================
 Funtion	:write_kbd_enc
 Input		:uint8_t data
		 <write data>
 Output		:void
 Return		:ER
		 <error code>
 Description	:write data to keyboard encoder
==================================================================================
*/
LOCAL ER write_kbd_enc(uint8_t data)
{
	int i;
	
	for (i = 0;i < KBD_MAX_RETRY;i++) {
		if (!(read_kbd_ctl() & KBD_MASK_OUT_BUF)) {
			out_b(KBD_ENC_WRITE_BUF, data);
			return(E_OK);
		}
	}
	
	return(E_BUSY);
}

/*
==================================================================================
 Funtion	:get_kbd_queue_size
 Input		:void
 Output		:void
 Return		:int
		 <size of current queue>
 Description	:get current queue size
==================================================================================
*/
LOCAL int get_kbd_queue_size(void)
{
	if (kbd_queue.read <= kbd_queue.write) {
		return(kbd_queue.write - kbd_queue.read);
	}
	
	return(kbd_queue.read - kbd_queue.write);
}

/*
==================================================================================
 Funtion	:write_kbd_queue
 Input		:uint8_t data
		 <data to write>
 Output		:void
 Return		:ER
		 <error code>
 Description	:write data to kbd queue
==================================================================================
*/
LOCAL ER write_kbd_queue(uint8_t data)
{
	ER err;
	
	if (kbd_queue.write + 1 == kbd_queue.read) {
		return(E_QOVR);
	}
	
	kbd_queue.data[kbd_queue.write++] = data;
	
	if (kbd_queue.size <= kbd_queue.write) {
		kbd_queue.write = 0;
	}
	
	//err = _tk_set_flg(kbd_state.flgid, KBD_IN_EVENT);
	err = tk_set_flg(kbd_state.flgid, KBD_IN_EVENT);
	
	return(err);
}

/*
==================================================================================
 Funtion	:read_one_byte
 Input		:void
 Output		:void
 Return		:ER
		 <read data or error code>
 Description	:read a byte from a queue
==================================================================================
*/
LOCAL ER read_one_byte(void)
{
	UINT flgptn;
	ER err;
	
	if (get_kbd_queue_size()) {
		err = (ER)kbd_queue.data[kbd_queue.read++];
		if (kbd_queue.size <= kbd_queue.read) {
			kbd_queue.read = 0;
		}
		return(err);
	}
	
	/* wait for a data							*/
	err = tk_wai_flg(kbd_state.flgid, KBD_IN_EVENT,
			TWF_ANDW | TWF_BITCLR, &flgptn, TMO_FEVR);
	
	if (err) {
		return(err);
	}
	
	if (get_kbd_queue_size()) {
		err = (ER)kbd_queue.data[kbd_queue.read++];
		if (kbd_queue.size <= kbd_queue.read) {
			kbd_queue.read = 0;
		}
		return(err);
	}
	
	return(E_OBJ);
}

/*
----------------------------------------------------------------------------------
	scan code analysis functions
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:to_capital_code
 Input		:uint8_t ascii
		 <ascii to convert to capital>
 Output		:void
 Return		:uint8_t
		 <capital ascii>
 Description	:convert an ascii to a capital ascii
==================================================================================
*/
LOCAL uint8_t to_capital_code(uint8_t ascii)
{
	int code = (int)ascii;
	
	if ('a' <= code && code <= 'z') {
		return((uint8_t)(code - ('a' - 'A')));
	}
	
	if (kbd_state.capslock) {
		return((uint8_t)code);
	}
	
	switch (code) {
	case KEY_1:		return(KEY_EXCLAMATION);
	case KEY_3:		return(KEY_HASH);
	case KEY_4:		return(KEY_DOLLAR);
	case KEY_5:		return(KEY_PERCENT);
	case KEY_2:		return(KEY_AT);
	case KEY_6:		return(KEY_CARRET);
 	case KEY_7:		return(KEY_AMPERSAND);
	case KEY_8:		return(KEY_NUM_ASTERISK);
 	case KEY_9:		return(KEY_LEFTPARENTHESIS);
	case KEY_0:		return(KEY_RIGHTPARENTHESIS);
	case KEY_GRAVE:		return(KEY_TILDE);
	case KEY_MINUS:		return(KEY_UNDERSCORE);
	case KEY_EQUAL:		return(KEY_PLUS);
	case KEY_LEFTBRACKET:	return(KEY_LEFTCURL);
	case KEY_RIGHTBRACKET:	return(KEY_RIGHTCURL);
	case KEY_BACKSLASH:	return(KEY_BAR);
	case KEY_SEMICOLON:	return(KEY_COLON);
	case KEY_SLASH:		return(KEY_QUESTION);
	case KEY_DOT:		return(KEY_GREATER);
	case KEY_COMMA:		return(KEY_LESS);
	}
	
	return((uint8_t)code);
}

/*
==================================================================================
 Funtion	:single_code_make
 Input		:int code
		 <scan code>
 Output		:void
 Return		:void
 Description	:analyze single make code
==================================================================================
*/
LOCAL void single_code_make(int code)
{
	int ascii;

	if (code == SCODE_MK_EX0) {
		/*--------------------------------------------------------------*/
		/* transition to analyze extended code EX0			*/
		/*--------------------------------------------------------------*/
		kbd_state.analyze_state = EX0_2ND_CODE;
	} if (code == SCODE_MK_EX1) {
		/*--------------------------------------------------------------*/
		/* transition to analyze extended code EX1			*/
		/*--------------------------------------------------------------*/
		kbd_state.analyze_state = EX1_2ND_CODE;
	} else 	if (code & KBD_SCODE_BREAK) {
		/*--------------------------------------------------------------*/
		/* transition to analyze break code				*/
		/*--------------------------------------------------------------*/
		single_code_break(code);
		kbd_state.analyze_state = SINGLE_CODE_MAKE;
	} else {
		/*--------------------------------------------------------------*/
		/* procedure standard scan code 				*/
		/*--------------------------------------------------------------*/
		if (code < sizeof(single_key_table)) {
			ascii = single_key_table[ code ];
			/* if code is not equal ascii				*/
			switch (ascii) {
			case	KEY_ALT:	kbd_state.alt = TRUE;		break;
			case	KEY_SHIFT:	kbd_state.shift = TRUE;		break;
			case	KEY_CTRL:	kbd_state.control = TRUE;	break;
			case	KEY_CAPS_LOCK:	kbd_state.capslock = TRUE;	break;
			case	KEY_WIN:	kbd_state.win = TRUE;		break;
			case	KEY_KATAKANA:	kbd_state.katakana = TRUE;	break;
			case	KEY_UNKNOWN: break;
			default:
				if (kbd_state.shift || kbd_state.capslock) {
					write_kbd_queue(to_capital_code(ascii));
				} else {
					write_kbd_queue(ascii);
				}
				break;
			}
		}
	}
}

/*
==================================================================================
 Funtion	:single_code_break
 Input		:int code
		 <scan code>
 Output		:void
 Return		:void
 Description	:analyze single break code
==================================================================================
*/
LOCAL void single_code_break(int code)
{
	int break_code;
	int ascii;
	
	break_code = code & ~KBD_SCODE_BREAK;
	
	if (break_code < sizeof(single_key_table)) {
		ascii = single_key_table[ break_code ];
		/* if code is not equal ascii					*/
		switch (ascii) {
		case	KEY_ALT:	kbd_state.alt = FALSE;		break;
		case	KEY_SHIFT:	kbd_state.shift = FALSE;	break;
		case	KEY_CTRL:	kbd_state.control = FALSE;	break;
		case	KEY_CAPS_LOCK:	kbd_state.capslock = FALSE;	break;
		case	KEY_WIN:	kbd_state.win = FALSE;		break;
		case	KEY_KATAKANA:	kbd_state.katakana = FALSE;	break;
		default: break;
		}
	}
	
	/*----------------------------------------------------------------------*/
	/* state is back to analyze single code make 				*/
	/*----------------------------------------------------------------------*/
	kbd_state.analyze_state = SINGLE_CODE_MAKE;
	return;
}

/*
==================================================================================
 Funtion	:ex0_2nd_code
 Input		:int code
		 <scan code>
 Output		:void
 Return		:void
 Description	:analyze ex0 2nd code
==================================================================================
*/
LOCAL void ex0_2nd_code(int code)
{
	if (kbd_state.numlock) {
		switch (code) {
		case	SCODE_MK_2_NUM_ON_FINAL_KEY:
		case	SCODE_BK_2_NUM_ON_FINAL_KEY:
		case	SCODE_MK_2_LR_CTRL:
		case	SCODE_MK_4_LR_CTRL:
			/*------------------------------------------------------*/
			/* state is back to analyze single code make	 	*/
			/*------------------------------------------------------*/
			kbd_state.analyze_state = SINGLE_CODE_MAKE;
			return;
		default:
			break;
		}
	} else {
		switch (code) {
		case	SCODE_MK_2_NUM_OFF_LSHIFT:
		case	SCODE_MK_2_NUM_OFF_RSHIFT:
			kbd_state.shift = TRUE;
			/*------------------------------------------------------*/
			/* state is back to analyze single code make	 	*/
			/*------------------------------------------------------*/
			kbd_state.analyze_state = SINGLE_CODE_MAKE;
			return;
		case	SCODE_BK_2_NUM_OFF_LSHIFT:
		case	SCODE_BK_2_NUM_OFF_RSHIFT:
			kbd_state.shift = FALSE;
			/*------------------------------------------------------*/
			/* state is back to analyze single code make	 	*/
			/*------------------------------------------------------*/
			kbd_state.analyze_state = SINGLE_CODE_MAKE;
			return;
		default:
			break;
		}
	}
	
	switch (code) {
	case	SCODE_MK_2_R_ALT:
		kbd_state.alt = TRUE;
		break;
	case	SCODE_MK_2_R_CTRL:
		kbd_state.control = TRUE;
		break;
	case	SCODE_MK_2_END:
		break;
	case	SCODE_MK_2_L_ARROW:
		break;
	case	SCODE_MK_2_HOME:
		break;
	case	SCODE_MK_2_INSERT:
		kbd_state.insert = TRUE;
		break;
	case	SCODE_MK_2_DELETE:
		write_kbd_queue(KEY_DELETE);
		break;
	case	SCODE_MK_2_DN_ARROW:
		break;
	case	SCODE_MK_2_R_ARROW:
		break;
	case	SCODE_MK_2_UP_ARROW:
		break;
	case	SCODE_MK_2_NUMERIC_DIVIDE:
		write_kbd_queue(KEY_NUM_DIVIDE);
		break;
	case	SCODE_MK_4_PRTSCREEN:
		break;
	}
	
	/*----------------------------------------------------------------------*/
	/* state is back to analyze single code make 				*/
	/*----------------------------------------------------------------------*/
	kbd_state.analyze_state = SINGLE_CODE_MAKE;
}

/*
==================================================================================
 Funtion	:ex0_3rd_code
 Input		:int code
		 <scan code>
 Output		:void
 Return		:void
 Description	:analyze ex0 3rd break code
==================================================================================
*/
LOCAL void ex0_3rd_code(int code)
{
	/*----------------------------------------------------------------------*/
	/* state is back to analyze single code make 				*/
	/*----------------------------------------------------------------------*/
	kbd_state.analyze_state = SINGLE_CODE_MAKE;
}

/*
==================================================================================
 Funtion	:ex1_2nd_code
 Input		:int code
		 <scan code>
 Output		:void
 Return		:void
 Description	:analyze ex1 2nd or later break code
==================================================================================
*/
LOCAL void ex1_2nd_code(int code)
{
	switch (code) {
	case	0xC5:
		/*--------------------------------------------------------------*/
		/* state is back to analyze single code make 			*/
		/*--------------------------------------------------------------*/
		kbd_state.analyze_state = SINGLE_CODE_MAKE;
		break;
	case	0xE1:
	case	0x1D:
	case	0x45:
	case	0x9D:
		break;
	default:
		/*--------------------------------------------------------------*/
		/* state is back to analyze single code make 			*/
		/*--------------------------------------------------------------*/
		kbd_state.analyze_state = SINGLE_CODE_MAKE;
		break;
	}
}

/*
==================================================================================
 Funtion	:kbd_interrupt
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
LOCAL void kbd_intterupt(struct ctx_reg *reg)
{
	int scan_code;
	
	scan_code = (int)read_kbd_enc();
	
	if (scan_code_func[kbd_state.analyze_state]) {
		scan_code_func[kbd_state.analyze_state](scan_code);
	} else {
		kbd_state.analyze_state = SINGLE_CODE_MAKE;
	}
	
	EndOfInterrupt1();
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
