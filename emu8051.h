/* 8051 emulator core
 * Copyright 2006 Jari Komppa
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the 
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject 
 * to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE. 
 *
 * (i.e. the MIT License)
 *
 * emu8051.h
 * Emulator core header file
 */

struct em8051;

// Operation: returns number of ticks the operation should take
typedef int (*em8051operation)(struct em8051 *aCPU); 

// Decodes opcode at position, and fills the buffer with the assembler code. 
// Returns how many bytes the opcode takes.
typedef int (*em8051decoder)(struct em8051 *aCPU, int aPosition, char *aBuffer);

// Callback: some exceptional situation occurred. See EM8051_EXCEPTION enum, below
typedef void (*em8051exception)(struct em8051 *aCPU, int aCode);

// Callback: an SFR register is about to be read (not called for 'a' ops nor psw changes)
// Default is to return the value in the SFR register. Ports may act differently.
typedef int (*em8051sfrread)(struct em8051 *aCPU, int aRegister);

// Callback: an SFR register has changed (not called for 'a' ops)
// Default is to do nothing
typedef void (*em8051sfrwrite)(struct em8051 *aCPU, int aRegister);

// Callback: writing to external memory
// Default is to update external memory
// (can be used to control some peripherals)
typedef void (*em8051xwrite)(struct em8051 *aCPU, int aAddress, int aValue);

// Callback: reading from external memory
// Default is to return the value in external memory 
// (can be used to control some peripherals)
typedef int (*em8051xread)(struct em8051 *aCPU, int aAddress);


struct em8051
{
    unsigned char *mCodeMem; // 1k - 64k, must be power of 2
    int mCodeMemSize; 
    unsigned char *mExtData; // 0 - 64k, must be power of 2
    int mExtDataSize;
    unsigned char *mLowerData; // 128 bytes
    unsigned char *mUpperData; // 0 or 128 bytes; leave to NULL if none
    unsigned char *mSFR; // 128 bytes; (special function registers)
    int mPC; // Program Counter; outside memory area
    int mTickDelay; // How many ticks should we delay before continuing
    em8051operation op[256]; // function pointers to opcode handlers
    em8051decoder dec[256]; // opcode-to-string decoder handlers    
    em8051exception except_cb; // callback: exceptional situation occurred
    em8051sfrread sfrread; // callback: SFR register being read
    em8051sfrwrite sfrwrite; // callback: SFR register written
    em8051xread xread; // callback: external memory being read
    em8051xwrite xwrite; // callback: external memory being written

    // Internal values for interrupt services etc.
    int mInterruptActive;
    // Stored register values for interrupts (exception checking)
    int int_a[2];
    int int_psw[2];
    int int_sp[2];
};

// set the emulator into reset state. Must be called before tick(), as
// it also initializes the function pointers. aWipe tells whether to reset
// all memory to zero.
void reset(struct em8051 *aCPU, int aWipe);

// run one emulator tick, or 12 hardware clock cycles.
// returns 1 if a new operation was executed.
int tick(struct em8051 *aCPU);

// decode the next operation as character string.
// buffer must be big enough (64 bytes is very safe). 
// Returns length of opcode.
int decode(struct em8051 *aCPU, int aPosition, char *aBuffer);

// Load an intel hex format object file. Returns negative for errors.
int load_obj(struct em8051 *aCPU, char *aFilename);

// Alternate way to execute an opcode (switch-structure instead of function pointers)
int do_op(struct em8051 *aCPU);

// Internal: Pushes a value into stack
void push_to_stack(struct em8051 *aCPU, int aValue);


// SFR register locations
enum SFR_REGS
{
    REG_ACC = 0xE0 - 0x80,
    REG_B   = 0xF0 - 0x80,
    REG_PSW = 0xD0 - 0x80,
    REG_SP  = 0x81 - 0x80,
    REG_DPL = 0x82 - 0x80,
    REG_DPH = 0x83 - 0x80,
    REG_P0  = 0x80 - 0x80,
    REG_P1  = 0x90 - 0x80,
    REG_P2  = 0xA0 - 0x80,
    REG_P3  = 0xB0 - 0x80,
    REG_IP  = 0xB8 - 0x80,
    REG_IE  = 0xA8 - 0x80,
    REG_TMOD = 0x89 - 0x80,
    REG_TCON = 0x88 - 0x80,
    REG_TH0 = 0x8C - 0x80,
    REG_TL0 = 0x8A - 0x80,
    REG_TH1 = 0x8D - 0x80,
    REG_TL1 = 0x8B - 0x80,
    REG_SCON = 0x98 - 0x80,
    REG_PCON = 0x87 - 0x80,
    
    // CH55X
    REG_DPLX = 0xAE - 0x80,
    REG_DPHX = 0xAF - 0x80,
    REG_SAFE_MOD = 0xA1 - 0x80,
    REG_XBUS_AUX = 0xA2 - 0x80,
    
    
    REG_P1_MOD_OC  = 0x92 - 0x80,
    REG_P1_DIR_PU  = 0x93 - 0x80,
    REG_P3_MOD_OC  = 0x96 - 0x80,
    REG_P3_DIR_PU = 0x97 - 0x80,

    REG_SBUF  = 0x99 - 0x80,

    REG_ADC_CFG  = 0x9A - 0x80,
    REG_ADC_DATA  = 0x9F - 0x80,

    REG_PWM_DATA2  = 0x9B - 0x80,
    REG_PWM_DATA1  = 0x9C - 0x80,
    REG_PWM_CTRL  = 0x9D - 0x80,
    REG_PWM_CK_SE  = 0x9E - 0x80,
    
    REG_WAKE_CTRL = 0xA9 - 0x80,
    REG_GLOBAL_CFG = 0xB1 - 0x80,
    REG_CLOCK_CFG = 0xB9 - 0x80,
    REG_SCON1  = 0xC0 - 0x80,
    REG_SBUF1  = 0xC1 - 0x80,
    REG_SBAUD1  = 0xC2 - 0x80,
    
    REG_TKEY_CTRL  = 0xC3 - 0x80,
    REG_TKEY_DATL  = 0xC4 - 0x80,
    REG_TKEY_DATH  = 0xC5 - 0x80,
    REG_PIN_FUNC  = 0xC6 - 0x80,
    REG_GPIO_IE  = 0xC7 - 0x80,
    
    REG_T2CON  = 0xC8 - 0x80,
    REG_T2MOD  = 0xC9 - 0x80,
    REG_RCAP2L  = 0xCA - 0x80,
    REG_RCAP2H  = 0xCB - 0x80,
    REG_TL2  = 0xCC - 0x80,
    REG_TH2  = 0xCD - 0x80,
    REG_T2CAP1L  = 0xCE - 0x80,
    REG_T2CAP1H  = 0xCF - 0x80,
    
    REG_USB_C_CTRL = 0x91 - 0x80,
    REG_UDEV_CTRL  = 0xD1 - 0x80,
    REG_UEP1_CTRL  = 0xD2 - 0x80,
    REG_UEP1_T_LEN  = 0xD3 - 0x80,
    REG_UEP2_CTRL  = 0xD4 - 0x80,
    REG_UEP2_T_LEN  = 0xD5 - 0x80,
    REG_UEP3_CTRL  = 0xD6 - 0x80,
    REG_UEP3_T_LEN = 0xD7 - 0x80,
    REG_USB_INT_FG  = 0xD8 - 0x80,
    REG_USB_INT_ST  = 0xD9 - 0x80,
    REG_USB_MIS_ST  = 0xDA - 0x80,
    REG_USB_RX_LEN  = 0xDB - 0x80,
    REG_UEP0_CTRL  = 0xDC - 0x80,
    REG_UEP0_T_LEN  = 0xDD - 0x80,
    REG_UEP4_CTRL  = 0xDE - 0x80,
    REG_UEP4_T_LEN = 0xDF - 0x80,
    REG_USB_INT_EN  = 0xE1 - 0x80,
    REG_USB_CTRL  = 0xE2 - 0x80,
    REG_USB_DEV_AD  = 0xE3 - 0x80,
    REG_UEP2_DMA_L  = 0xE4 - 0x80,
    REG_UEP2_DMA_H  = 0xE5 - 0x80,
    REG_UEP3_DMA_L  = 0xE6 - 0x80,
    REG_UEP3_DMA_H = 0xE7 - 0x80,
    
    REG_IE_EX  = 0xE8 - 0x80,
    REG_IP_EX  = 0xE9 - 0x80,
    REG_UEP4_1_MOD  = 0xEA - 0x80,
    REG_UEP2_3_MOD  = 0xEB - 0x80,
    REG_UEP0_DMA_L  = 0xEC - 0x80,
    REG_UEP0_DMA_H  = 0xED - 0x80,
    REG_UEP1_DMA_L  = 0xEE - 0x80,
    REG_UEP1_DMA_H = 0xEF - 0x80,
    
    
    REG_ROM_ADDR_L = 0x84 - 0x80,
    REG_ROM_ADDR_H = 0x85 - 0x80,
    REG_ROM_CTRL  = 0x86 - 0x80,
    REG_ROM_DATA_L = 0x8E - 0x80,
    REG_ROM_DATA_H = 0x8F - 0x80,
    
    
    REG_SPI0_STAT  = 0xF8 - 0x80,
    REG_SPI0_DATA  = 0xF9 - 0x80,
    REG_SPI0_CTRL = 0xFA - 0x80,
    REG_SPI0_CK_SE = 0xFB - 0x80,
    REG_SPI0_SETUP  = 0xFC - 0x80,
    
    REG_RESET_KEEP  = 0xFE - 0x80,
    REG_WDOG_COUNT  = 0xFF - 0x80,
};

enum PSW_BITS
{
    PSW_P = 0,
    PSW_F1 = 1,
    PSW_OV = 2,
    PSW_RS0 = 3,
    PSW_RS1 = 4,
    PSW_F0 = 5,
    PSW_AC = 6,
    PSW_C = 7
};

enum PSW_MASKS
{
    PSWMASK_P = 0x01,
    PSWMASK_UNUSED = 0x02,
    PSWMASK_OV = 0x04,
    PSWMASK_RS0 = 0x08,
    PSWMASK_RS1 = 0x10,
    PSWMASK_F0 = 0x20,
    PSWMASK_AC = 0x40,
    PSWMASK_C = 0x80
};

enum IE_MASKS
{
    IEMASK_EX0 = 0x01,
    IEMASK_ET0 = 0x02,
    IEMASK_EX1 = 0x04,
    IEMASK_ET1 = 0x08,
    IEMASK_ES  = 0x10,
    IEMASK_ET2 = 0x20,
    IEMASK_UNUSED = 0x40,
    IEMASK_EA  = 0x80
};

enum PT_MASKS
{
    PTMASK_PX0 = 0x01,
    PTMASK_PT0 = 0x02,
    PTMASK_PX1 = 0x04,
    PTMASK_PT1 = 0x08,
    PTMASK_PS  = 0x10,
    PTMASK_PT2 = 0x20,
    PTMASK_UNUSED1 = 0x40,
    PTMASK_UNUSED2 = 0x80
};

enum TCON_MASKS
{
    TCONMASK_IT0 = 0x01,
    TCONMASK_IE0 = 0x02,
    TCONMASK_IT1 = 0x04,
    TCONMASK_IE1 = 0x08,
    TCONMASK_TR0 = 0x10,
    TCONMASK_TF0 = 0x20,
    TCONMASK_TR1 = 0x40,
    TCONMASK_TF1 = 0x80
};

enum TMOD_MASKS
{
    TMODMASK_M0_0 = 0x01,
    TMODMASK_M1_0 = 0x02,
    TMODMASK_CT_0 = 0x04,
    TMODMASK_GATE_0 = 0x08,
    TMODMASK_M0_1 = 0x10,
    TMODMASK_M1_1 = 0x20,
    TMODMASK_CT_1 = 0x40,
    TMODMASK_GATE_1 = 0x80
};

enum IP_MASKS
{
    IPMASK_PX0 = 0x01,
    IPMASK_PT0 = 0x02,
    IPMASK_PX1 = 0x04,
    IPMASK_PT1 = 0x08,
    IPMASK_PS  = 0x10,
    IPMASK_PT2 = 0x20
};

enum EM8051_EXCEPTION
{
    EXCEPTION_STACK,  // stack address > 127 with no upper memory, or roll over
    EXCEPTION_ACC_TO_A, // acc-to-a move operation; illegal (acc-to-acc is ok, a-to-acc is ok..)
    EXCEPTION_IRET_PSW_MISMATCH, // psw not preserved over interrupt call (doesn't care about P, F0 or UNUSED)
    EXCEPTION_IRET_SP_MISMATCH,  // sp not preserved over interrupt call
    EXCEPTION_IRET_ACC_MISMATCH, // acc not preserved over interrupt call
    EXCEPTION_ILLEGAL_OPCODE     // for the single 'reserved' opcode in the architecture
};

