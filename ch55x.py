#!/usr/bin/env python3

from ch55x import ffi, lib


@ffi.def_extern()
def em8051exception_callback(aCPU, aCode):
    if aCode == -1:
        raise Exception("Breakpoint reached") # TODO: call callback or something here
    elif aCode == lib.EXCEPTION_STACK:
        raise Exception("SP exception: stack address > 127 with no upper memory, or SP roll over.")
    elif aCode == lib.EXCEPTION_ACC_TO_A:
        raise Exception("Invalid operation: acc-to-a move operation")
    elif aCode == lib.EXCEPTION_IRET_PSW_MISMATCH:
        raise Exception("PSW not preserved over interrupt call")
    elif aCode == lib.EXCEPTION_IRET_SP_MISMATCH:
        raise Exception("SP not preserved over interrupt call")
    elif aCode == lib.EXCEPTION_IRET_ACC_MISMATCH:
        raise Exception("ACC not preserved over interrupt call")
    elif aCode == lib.EXCEPTION_ILLEGAL_OPCODE:
        raise Exception("Invalid opcode: 0xA5 encountered")
    else:
        raise Exception("Unknown exception")

@ffi.def_extern()
def em8051sfrread_callback(aCPU, aRegister):
    print('SFR read %s' % hex(aRegister))
    return 0

@ffi.def_extern()
def em8051sfrwrite_callback(aCPU, aRegister):
    pass

@ffi.def_extern()
def em8051xwrite_callback(aCPU, aAddress, aValue):
    pass

@ffi.def_extern()
def em8051xread_callback(aCPU, aAddress):
    return 0



class EmuCH55X:

    def __init__(self, type = 2):
        types = { 
            1: (10, 0.5),
            2: (16, 1),
            4: (16, 1),
            5: (32, 1),
            7: (64, 4),
            8: (40, 4),
            9: (64, 6),
        }
        if type not in types:
            raise ParameterError("unknown type '%s'." % type)
        type = types[type]

        self.emu = ffi.new("struct em8051 *")

        self.emu.mCodeMemSize = type[0]*1024
        self.emu.mExtDataSize = type[1]*1024

        self.emu.mCodeMem = self.code_memory = ffi.new("char[%d]" % self.emu.mCodeMemSize)
        self.emu.mExtData = self.ext_memory  = ffi.new("char[%d]" % self.emu.mExtDataSize)

        self.emu.mLowerData = self.lower_data = ffi.new("char[128]")
        self.emu.mUpperData = self.upper_data = ffi.new("char[128]")
        self.emu.mSFR       = self.SFR        = ffi.new("char[128]")

        self.emu.except_cb = lib.em8051exception_callback
        # no callback = default behaviour = read from internal memory
        self.emu.sfrread   = lib.em8051sfrread_callback

        lib.reset(self.emu, 1)

    def reset(self, wipeMemory = False):
        if wipeMemory:
            lib.reset(self.emu, 1)
        else:
            lib.reset(self.emu, 0)


    def loadHEX(self, filename):
        result = lib.load_obj(self.emu, filename.encode('ascii'));

        if result == -1:
            raise IOError("File not found.")
        elif result == -2:
            raise IOError("Bad file format.")
        elif result == -3:
            raise IOError("Unsupported HEX file version.")
        elif result == -4:
            raise IOError("Checksum failure.")
        elif result == -5:
            raise IOError("No end of data marker found.")

    def decode(self, position):
        cstring = ffi.new("char[64]")
        lib.decode(self.emu, position, cstring)
        return ffi.string(cstring)


    def tick(self):
        return lib.tick(self.emu)

    def step(self, instructions=1):
        while instructions > 0:
            if self.tick() > 0:
                instructions -= 1

    def PC(self):
        return self.emu.mPC

    def ACC(self):
        return self.SFR[lib.REG_ACC]

    def PSW(self):
        return self.SFR[lib.REG_PSW]

    # convenience function to access current register bank
    def r(self, n):

        if n < 0 or n > 7:
            raise ValueError("Invalid register index: " + str(n))

        RX_ADDRESS = n + 8 * ((int.from_bytes(self.PSW(), byteorder='little') & (lib.PSWMASK_RS0|lib.PSWMASK_RS1))>>lib.PSW_RS0)
        return self.lower_data[RX_ADDRESS]



if __name__ == "__main__":
    import sys
    e = EmuCH55X()
    e.loadHEX(sys.argv[1])
    
    print(lib.REG_ACC)

    while e.lower_data[0x60] != b'\xFA':
        #print(hex(e.PC()), e.ACC(), e.r(0), e.r(1), e.r(2))
        e.step()

    print(hex(ord(e.lower_data[0x60])))

    print(lib.REG_ACC)
