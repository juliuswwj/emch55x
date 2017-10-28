#!/usr/bin/env python3

# requires cffi >= 1.4 (install with pip to get a up-to-date version!)

from cffi import FFI
builder = FFI()

builder.set_source("ch55x", """
#include "emu8051.h"
""", sources=["core.c", "opcodes.c", "disasm.c"])

with open("emu8051.h", 'r') as f:
    text = f.read()
    text += """
// Callbacks into python

// Callback: some exceptional situation occurred. See EM8051_EXCEPTION enum, below
extern "Python" void em8051exception_callback(struct em8051 *aCPU, int aCode);

// Callback: an SFR register is about to be read (not called for 'a' ops nor psw changes)
// Default is to return the value in the SFR register. Ports may act differently.
extern "Python" int em8051sfrread_callback(struct em8051 *aCPU, int aRegister);

// Callback: an SFR register has changed (not called for 'a' ops)
// Default is to do nothing
extern "Python" void em8051sfrwrite_callback(struct em8051 *aCPU, int aRegister);

// Callback: writing to external memory
// Default is to update external memory
// (can be used to control some peripherals)
extern "Python" void em8051xwrite_callback(struct em8051 *aCPU, int aAddress, int aValue);

// Callback: reading from external memory
// Default is to return the value in external memory 
// (can be used to control some peripherals)
extern "Python" int em8051xread_callback(struct em8051 *aCPU, int aAddress);

"""
    builder.cdef(text)

if __name__ == "__main__":
    builder.compile(verbose=True)
