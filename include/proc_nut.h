/*
 $Id$
 Copyright 1995, 2003, 2004, 2005 Eric L. Smith <eric@brouhaha.com>
 
 Nonpareil is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that I am not
 granting permission to redistribute or modify Nonpareil under the
 terms of any later version of the General Public License.
 
 Nonpareil is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING"); if not, write to the
 Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 MA 02111, USA.
 */

//
// changes for mac os x by Maciej Bartosiak
// changes for espnut by shezik
//

#pragma once

#include <Arduino.h>
#include "digit_ops.h"
#include "proc.h"
#include "display.h"
#include "voyager_lcd.h"

#include "util.h"
#include <LittleFS.h>

#define WSIZE 14
#define EXPSIZE 3  // two exponent and one exponent sign digit

typedef digit_t reg_t [WSIZE];


#define SSIZE 14


#define EXT_FLAG_SIZE 14

#define EF_PRINTER_BUSY     0  // 82143A printer
#define EF_CARD_READER      1  // 82104A card reader
#define EF_WAND_DATA_AVAIL  2  // 82153A bar code wand
#define EF_BLINKY_EDAV      5  // 88242A IR printer module
#define EF_HPIL_IFCR        6  // 82160A HP-IL module
#define EF_HPIL_SRQR        7
#define EF_HPIL_FRAV        8
#define EF_HPIL_FRNS        9
#define EF_HPIL_ORAV       10
#define EF_TIMER           12  // 82182A Time Module (built into 41CX)
#define EF_SERVICE_REQUEST 13  // shared general service request
							   // Flags 3, 4, and 11 are apparently not used by any standard peripherals


#define STACK_DEPTH 4

//#undef PAGE_SIZE
#define N_PAGE_SIZE 4096
#define MAX_PAGE 16
#define MAX_BANK 4


typedef enum
{
	KB_IDLE,
	KB_PRESSED,
	KB_RELEASED,
	KB_WAIT_CHK,
	KB_WAIT_CYC,
	KB_STATE_MAX  // must be last
} keyboard_state_t;


enum
{
	event_periph_select = first_arch_event,
	event_ram_select
};


typedef uint16_t rom_addr_t;


typedef enum
{
    norm,
    long_branch,
    cxisa,
    ldi,
    selprf         // "smart" peripheral selected (NPIC, PIL)
} inst_state_t;


struct nut_reg_t;

typedef void ram_access_fn_t (struct nut_reg_t *nut_reg, int addr, reg_t *reg);

typedef struct nut_reg_t  // By commenting 'constant' I mean the value is not changing when provided with the same ROM file and RAM size. Implies 'no need to save'.
						  // The ones commented with 'SAVE' appear in nonpareil XML save file and surely should be saved, but others I'm not so sure of.
{
	reg_t a;  // !! SAVE
	reg_t b;  // !! SAVE
	reg_t c;  // !! SAVE
	reg_t m;  // !! SAVE
	reg_t n;  // !! SAVE
	digit_t g [2];  // !! SAVE
	
	digit_t p;  // !! SAVE
	digit_t q;  // !! SAVE
	bool q_sel;  // true if q is the selected pointer, false for p  // !! SAVE
	
	uint8_t fo;  /* flag output regiters, 8 bits, used to drive bender */  // !! SAVE
	
	bool decimal;  // true for arithmetic radix 10, false for 16  // !! SAVE
	
	bool carry;       // carry being generated in current instruction  // !! SAVE
	bool prev_carry;  // carry that resulted from previous instruction  // !! I'll save it.
	
	int prev_tef_last;  // last digit of field of previous arith. instruction  // !! This one too.
						// used to simulate bug in logical or and and
	
	bool s [SSIZE];  // !! SAVE
	
	rom_addr_t pc;  // !! SAVE
	rom_addr_t prev_pc;  // !! Mm hmm.
	
	rom_addr_t stack [STACK_DEPTH];  // !! SAVE
	
	rom_addr_t cxisa_addr;  // !! I'll save this one.
	
	inst_state_t inst_state;  // !! yup.
	
	rom_word_t first_word;   /* long branch: remember first word */  // !! yup
	bool long_branch_carry;  /* and carry */  // !! yup
	
	// I'll save the following 4 variables for now. However if they interfere with state saving/loading they'll be removed.
	bool key_down;      /* true while a key is down */
	keyboard_state_t kb_state;
	int kb_debounce_cycle_counter;
	int key_buf;        /* most recently pressed key */
	
	bool awake;  // !! SAVE
	
	void (* op_fcn [1024])(struct nut_reg_t *nut_reg, int opcode);  // Constant
	
	// ROM:
	int bank_group [MAX_PAGE];  // defines which pages bank switch together  // Not even used. Used in Nonpareil when loading MOD1 roms which just is not happening here.
	uint8_t active_bank [MAX_PAGE];  // bank number from 0..MAX_BANK-1  // !! SAVE
	//bool rom_writeable [MAX_PAGE][MAX_BANK];
	rom_word_t *rom [MAX_PAGE][MAX_BANK];  // Initializes in nut_new_rom_addr_space()  // Constant (if you load the same rom)
	//bool *rom_breakpoint [MAX_PAGE][MAX_BANK];
	// source_code_line_info_t *source_code_line_info [MAX_PAGE][MAX_BANK];
	
	// RAM:
	uint16_t ram_addr;  // selected RAM address  // !! SAVE
	bool *ram_exists;  // Constant value given the same 'ram_size'
	reg_t *ram;  // !! SAVE
	ram_access_fn_t **ram_read_fn;  // Constant
	ram_access_fn_t **ram_write_fn;  // Constant
	
	// Peripherals:
	uint16_t max_pf;  // Constant
	uint8_t pf_addr;  // selected peripheral address  // !! SAVE
	bool *pf_exists;  // Constant
	
	bool ext_flag [EXT_FLAG_SIZE];  // Constant, nut_set_ext_flag() is yet to be implemented
	
	// Peripheral I/O functions return true if the peripheral responded.
	bool (* rd_n_fcn [256])(struct nut_reg_t *nut_reg, int n);  // Constant
	bool (* wr_n_fcn [256])(struct nut_reg_t *nut_reg, int n);  // Constant
	bool (* wr_fcn   [256])(struct nut_reg_t *nut_reg);  // Constant
	
	uint8_t selprf;  // selected "smart peripheral" number  // !! Save?
	
	// Function to call for "smart peripheral" to handle opcodes after
	// a selprf instruction:
	bool (* selprf_fcn [16])(struct nut_reg_t *nut_reg, rom_word_t opcode);  // Constant
	
	//chip_t *display_chip;  // opaque
	//chip_t *phineas_chip;  // opaque
	//chip_t *helios_chip;   // opaque
	
	voyager_display_reg_t *display_chip;  // 'Constant'
	
	//sim_t *sim;
	
	int display_digits;  // Constant
	segment_bitmap_t display_segments [MAX_DIGIT_POSITION];  // !! Better SAVE it
	//bool need_redraw;
	void *display;  // 'Constant'
	
	uint64_t cycle_count;  // !! Better SAVE it
	
	uint16_t   max_ram;  // Constant
	
	int max_rom;  // Constant, useless
	int max_bank;  // Constant, useless
} nut_reg_t;

nut_reg_t * nut_new_processor (int ram_size, void *display);
bool nut_read_object_file (nut_reg_t *nut_reg, const char *fn);
void nut_press_key (nut_reg_t *nut_reg, int keycode);
void nut_release_key (nut_reg_t *nut_reg);
bool nut_execute_instruction (nut_reg_t *nut_reg);
void nut_free_processor (nut_reg_t *nut_reg);
void do_event(nut_reg_t *nut_reg, event_t event);  // Maybe the other identical 'do_event_int' stands for 'internal do event'...
