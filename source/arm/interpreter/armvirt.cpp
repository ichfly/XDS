/*  armvirt.c -- ARMulator virtual memory interace:  ARM6 Instruction Emulator.
    Copyright (C) 1994 Advanced RISC Machines Ltd.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

/* This file contains a complete ARMulator memory model, modelling a
"virtual memory" system. A much simpler model can be found in armfast.c,
and that model goes faster too, but has a fixed amount of memory. This
model's memory has 64K pages, allocated on demand from a 64K entry page
table. The routines PutWord and GetWord implement this. Pages are never
freed as they might be needed again. A single area of memory may be
defined to generate aborts. */

#include "Common.h"
#include "Kernel.h"
#include "arm/skyeye_common/armdefs.h"
#include "arm/skyeye_common/armemu.h"

#include "arm/memory.h"


#define dumpstack 1
#define dumpstacksize 0x10
#define maxdmupaddr 0x0033a850

/*ARMword ARMul_GetCPSR (ARMul_State * state) {
return 0;
}
ARMword ARMul_GetSPSR (ARMul_State * state, ARMword mode) {
return 0;
}
void ARMul_SetCPSR (ARMul_State * state, ARMword value) {

}
void ARMul_SetSPSR (ARMul_State * state, ARMword mode, ARMword value) {

}*/

void ARMul_Icycles(ARMul_State * state, unsigned number, ARMword address) {
}

void ARMul_Ccycles(ARMul_State * state, unsigned number, ARMword address) {
}

ARMword ARMul_LoadInstrS(ARMul_State * state, ARMword address, ARMword isize) {
    state->NumScycles++;

#ifdef HOURGLASS
    if ((state->NumScycles & HOURGLASS_RATE) == 0) {
        HOURGLASS;
    }
#endif
    if (isize == 2)
    {
        u16 data;
        if (unlikely(state->m_MemoryMap->Read16(address, data) != Success))
        {
            XDSERROR("size 2 error reading from %08x", address);
        }
        return data;
    }
    else
    {
        u32 data;
        if (unlikely(state->m_MemoryMap->Read32(address, data) != Success))
        {
            XDSERROR("size 4 error reading from %08x", address);
        }
        return data;
    }
}

ARMword ARMul_LoadInstrN(ARMul_State * state, ARMword address, ARMword isize) {
    state->NumNcycles++;

    if (isize == 2)
    {
        u16 data;
        if (unlikely(state->m_MemoryMap->Read16(address, data) != Success))
        {
            XDSERROR("size 2 error reading from %08x", address);
        }
        return data;
    }
    else
    {
        u32 data;
        if (unlikely(state->m_MemoryMap->Read32(address, data) != Success))
        {
            XDSERROR("size 4 error reading from %08x", address);
        }
        return data;
    }
}

ARMword ARMul_ReLoadInstr(ARMul_State * state, ARMword address, ARMword isize) {
    ARMword data;

    if ((isize == 2) && (address & 0x2)) {
        u16 data;
        if (unlikely(state->m_MemoryMap->Read16(address, data) != Success))
        {
            XDSERROR("error reading from %08x", address);
        }
        return data & 0xFFFF;
    }
    if (unlikely(state->m_MemoryMap->Read32(address, data) != Success))
    {
        XDSERROR("size 4 error reading from %08x", address);
    }
    return data;
}

ARMword ARMul_ReadWord(ARMul_State * state, ARMword address) {
    u32 data;
    if (unlikely(state->m_MemoryMap->Read32(address, data) != Success))
    {
		XDSERROR("error %s thread %u reading from %08x", state->m_currentThread->m_owner->GetName(), state->m_currentThread->m_thread_id, address);
    }
    return data;
}

ARMword ARMul_LoadWordS(ARMul_State * state, ARMword address) {
    state->NumScycles++;
    return ARMul_ReadWord(state, address);
}

ARMword ARMul_LoadWordN(ARMul_State * state, ARMword address) {
    state->NumNcycles++;
    return ARMul_ReadWord(state, address);
}

ARMword ARMul_LoadHalfWord(ARMul_State * state, ARMword address) {
    state->NumNcycles++;
    u16 data;
    if (unlikely(state->m_MemoryMap->Read16(address, data) != Success))
    {
		XDSERROR("error %s thread %u error reading from hword from %08x", state->m_currentThread->m_owner->GetName(), state->m_currentThread->m_thread_id, address);
    }
    return data;
}

ARMword ARMul_ReadByte(ARMul_State * state, ARMword address) {
    u8 data;
    if (unlikely(state->m_MemoryMap->Read8(address, data) != Success))
    {
		XDSERROR("error %s thread %u reading byte from %08x", state->m_currentThread->m_owner->GetName(), state->m_currentThread->m_thread_id, address);
		data = 0x10;
	}
    return data;
}

ARMword ARMul_LoadByte(ARMul_State * state, ARMword address) {
    state->NumNcycles++;
    return ARMul_ReadByte(state, address);
}

void ARMul_StoreHalfWord(ARMul_State * state, ARMword address, ARMword data) {
    state->NumNcycles++;
    if (unlikely(state->m_MemoryMap->Write16(address, data) != Success))
    {
		XDSERROR("error %s thread %u writing %04x to %08x", state->m_currentThread->m_owner->GetName(), state->m_currentThread->m_thread_id, data, address);
    }
}

void ARMul_StoreByte(ARMul_State * state, ARMword address, ARMword data) {
    state->NumNcycles++;
    ARMul_WriteByte(state, address, data);
}

ARMword ARMul_SwapWord(ARMul_State * state, ARMword address, ARMword data) {
    ARMword temp;
    state->NumNcycles++;
    temp = ARMul_ReadWord(state, address);
    state->NumNcycles++;
    if (unlikely(state->m_MemoryMap->Write32(address, data) != Success))
    {
        XDSERROR("error writing to %08x", address);
    }
    return temp;
}

ARMword ARMul_SwapByte(ARMul_State * state, ARMword address, ARMword data) {
    ARMword temp;
    temp = ARMul_LoadByte(state, address);
    if (unlikely(state->m_MemoryMap->Write8(address, data) != Success))
    {
        XDSERROR("error writing to %08x", address);
    }
    return temp;
}

void ARMul_WriteWord(ARMul_State * state, ARMword address, ARMword data) {
    if (unlikely(state->m_MemoryMap->Write32(address, data) != Success))
    {
        XDSERROR("error writing to %08x data %08x", address,data);
    }
}

void ARMul_WriteByte(ARMul_State * state, ARMword address, ARMword data)
{
    if (unlikely(state->m_MemoryMap->Write8(address, data) != Success))
    {
		XDSERROR("error %s thread %u writing %02x to %08x", state->m_currentThread->m_owner->GetName(), state->m_currentThread->m_thread_id, data, address);
    }
}
void ARMul_WriteDouble(ARMul_State* state, ARMword address, u64 data) {
    ARMul_WriteWord(state, address, (u32)(data >> 0));
    ARMul_WriteWord(state, address + 4, (u32)(data >> 32));
}


void ARMul_StoreWordS(ARMul_State * state, ARMword address, ARMword data)
{
    state->NumScycles++;
    ARMul_WriteWord(state, address, data);
}

void ARMul_StoreWordN(ARMul_State * state, ARMword address, ARMword data)
{
    state->NumNcycles++;
    ARMul_WriteWord(state, address, data);
}
