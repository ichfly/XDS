#include "Kernel.h"
#include "arm/skyeye_common/armdefs.h"
#include "arm/skyeye_common/arm_regformat.h"

u32 ReadCP15Register(ARMul_State* cpu, u32 crn, u32 opcode_1, u32 crm, u32 opcode_2)
{
    // Unprivileged registers
    if (crn == 13 && opcode_1 == 0 && crm == 0)
    {
        if (opcode_2 == 2)
            return cpu->CP15[CP15(CP15_THREAD_UPRW)];

        // TODO: Whenever TLS is implemented, this should return
        // "cpu->CP15[CP15(CP15_THREAD_URO)];"
        // which contains the address of the 0x200-byte TLS
        if (opcode_2 == 3)
            return cpu->m_currentThread->m_TSL3DS;
    }

    LOG("MRC CRn=%u, CRm=%u, OP1=%u OP2=%u is not implemented. Returning zero.", crn, crm, opcode_1, opcode_2);
    return 0;
}

// Write to the CP15 registers. Used with implementation of the MCR instruction.
// Note that since the 3DS does not have the hypervisor extensions, these registers
// are not implemented.
void WriteCP15Register(ARMul_State* cpu, u32 value, u32 crn, u32 opcode_1, u32 crm, u32 opcode_2)
{
    // Unprivileged registers
    if (crn == 7 && opcode_1 == 0 && crm == 5 && opcode_2 == 4)
    {
        cpu->CP15[CP15(CP15_FLUSH_PREFETCH_BUFFER)] = value;
    }
    else if (crn == 7 && opcode_1 == 0 && crm == 10)
    {
        if (opcode_2 == 4)
            cpu->CP15[CP15(CP15_DATA_SYNC_BARRIER)] = value;
        else if (opcode_2 == 5)
            cpu->CP15[CP15(CP15_DATA_MEMORY_BARRIER)] = value;

    }
    else if (crn == 13 && opcode_1 == 0 && crm == 0 && opcode_2 == 2)
    {
        cpu->CP15[CP15(CP15_THREAD_UPRW)] = value;
    }
}
