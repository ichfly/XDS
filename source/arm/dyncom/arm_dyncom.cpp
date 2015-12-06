// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

//massive fixed by ichfly XDS/3dmoo team

#include "Kernel.h"

#include "arm/skyeye_common/armcpu.h"
#include "arm/skyeye_common/armemu.h"
#include "arm/skyeye_common/vfp/vfp.h"

#include "arm/dyncom/arm_dyncom.h"
#include "arm/dyncom/arm_dyncom_interpreter.h"
#include "arm/dyncom/arm_dyncom_run.h"

//#define CACHE_BUFFER_SIZE    (64 * 1024 * 2000)


const static cpu_config_t s_arm11_cpu_info = {
    "armv6", "arm11", 0x0007b000, 0x0007f000, NONCACHE
};

ARM_DynCom::ARM_DynCom() {
    state = (ARMul_State*)malloc(sizeof(ARMul_State));

    ARMul_NewState(state);
    ARMul_SelectProcessor(state, ARM_v6_Prop | ARM_v5_Prop | ARM_v5e_Prop);

    state->abort_model = 0;
    state->cpu = (cpu_config_t*)&s_arm11_cpu_info;

    state->bigendSig = LOW;
    state->lateabtSig = LOW;
    state->NirqSig = HIGH;

    // Reset the core to initial state
    ARMul_Reset(state);
    state->NextInstr = RESUME; // NOTE: This will be overwritten by LoadContext
    state->Emulate = RUN;

    // Switch to the desired privilege mode.
    switch_mode(state, 0);

    state->Reg[13] = 0x10000000; // Set stack pointer to the top of the stack
    state->Reg[15] = 0x00000000;
}

ARM_DynCom::~ARM_DynCom() {
}

void ARM_DynCom::SetPC(u32 pc) {
    state->Reg[15] = pc;
}

u32 ARM_DynCom::GetPC() const {
    return state->Reg[15];
}

u32 ARM_DynCom::GetReg(int index) const {
    return state->Reg[index];
}

void ARM_DynCom::SetReg(int index, u32 value) {
    state->Reg[index] = value;
}

u32 ARM_DynCom::GetCPSR() const {
    return state->Cpsr;
}

void ARM_DynCom::SetCPSR(u32 cpsr) {
    state->Cpsr = cpsr;
}

void ARM_DynCom::AddTicks(u64 ticks) {
    down_count -= ticks;
    //if (down_count < 0)
        //CoreTiming::Advance(); //ichfly todo
}

u64 ARM_DynCom::ExecuteInstructions(int num_instructions) {
    state->NumInstrsToExecute = num_instructions;

    // Dyncom only breaks on instruction dispatch. This only happens on every instruction when
    // executing one instruction at a time. Otherwise, if a block is being executed, more
    // instructions may actually be executed than specified.
	unsigned ticks_executed = InterpreterMainLoop(state);
    AddTicks(ticks_executed);
	return ticks_executed;
}

void ARM_DynCom::SaveContext(ThreadContext& ctx) {
    memcpy(ctx.cpu_registers, state->Reg, sizeof(ctx.cpu_registers));
    memcpy(ctx.fpu_registers, state->ExtReg, sizeof(ctx.fpu_registers));

    ctx.sp = state->Reg[13];
    ctx.lr = state->Reg[14];
    ctx.pc = state->pc;
    ctx.cpsr = state->Cpsr;

    ctx.fpscr = state->VFP[1];
    ctx.fpexc = state->VFP[2];

    ctx.reg_15 = state->Reg[15];
    ctx.mode = state->NextInstr;

    //Dyncore
	state->m_currentThread->m_owner->repretBuffersize = state->inst_buffsize;
	state->m_currentThread->m_owner->repretBuffertop = state->inst_bufftop;

    ctx.NFlag = state->NFlag;
    ctx.ZFlag = state->ZFlag;
    ctx.CFlag = state->CFlag;
    ctx.VFlag = state->VFlag;
    ctx.IFFlags = state->IFFlags;

}

void ARM_DynCom::LoadContext(const ThreadContext& ctx) {
    memcpy(state->Reg, ctx.cpu_registers, sizeof(ctx.cpu_registers));
    memcpy(state->ExtReg, ctx.fpu_registers, sizeof(ctx.fpu_registers));

    state->Reg[13] = ctx.sp;
    state->Reg[14] = ctx.lr;
    state->pc = ctx.pc;
    state->Cpsr = ctx.cpsr;
	state->Mode = ctx.cpsr&0x1F;

    state->VFP[1] = ctx.fpscr;
    state->VFP[2] = ctx.fpexc;

    state->Reg[15] = ctx.reg_15;
    state->NextInstr = ctx.mode;

    //Dyncore
	state->inst_buf = state->m_currentThread->m_owner->repretBuffer;
	state->inst_buffsize = state->m_currentThread->m_owner->repretBuffersize;
	state->inst_bufftop = state->m_currentThread->m_owner->repretBuffertop;
	state->CreamCache = state->m_currentThread->m_owner->CreamCache;

    state->NFlag = ctx.NFlag;
    state->ZFlag = ctx.ZFlag;
    state->CFlag = ctx.CFlag;
    state->VFlag = ctx.VFlag;
    state->IFFlags = ctx.IFFlags;
}

void ARM_DynCom::PrepareReschedule() {
    state->NumInstrsToExecute = 0;
}
u64 ARM_DynCom::GetTicks() const {
    return state->NumInstrs;
}