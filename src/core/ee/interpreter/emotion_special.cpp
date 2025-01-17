#include <cstdio>
#include <cstdlib>
#include "emotioninterpreter.hpp"

namespace ee
{
    namespace interpreter
    {
        void special(EE_InstrInfo& info, uint32_t instruction)
        {
            int op = instruction & 0x3F;
            switch (op)
            {
            case 0x00:
                info.interpreter_fn = &sll;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x02:
                info.interpreter_fn = &srl;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x03:
                info.interpreter_fn = &sra;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x04:
                info.interpreter_fn = &sllv;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x06:
                info.interpreter_fn = &srlv;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x07:
                info.interpreter_fn = &srav;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x08:
                info.interpreter_fn = &jr;
                info.pipeline = EE_InstrInfo::Pipeline::Branch;
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x09:
                info.interpreter_fn = &jalr;
                info.pipeline = EE_InstrInfo::Pipeline::Branch;
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x0A:
                info.interpreter_fn = &movz;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x0B:
                info.interpreter_fn = &movn;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x0C:
                info.interpreter_fn = &syscall_ee;
                info.pipeline = EE_InstrInfo::Pipeline::COP0;
                break;
            case 0x0D:
                info.interpreter_fn = &break_ee;
                info.pipeline = EE_InstrInfo::Pipeline::COP0;
                break;
            case 0x0F:
                // SYNC
                info.interpreter_fn = &nop;
                info.pipeline = EE_InstrInfo::Pipeline::SYNC;
                break;
            case 0x10:
                info.interpreter_fn = &mfhi;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (uint8_t)Registers::HI0);
                break;
            case 0x11:
                info.interpreter_fn = &mthi;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::HI0);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x12:
                info.interpreter_fn = &mflo;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (uint8_t)Registers::LO0);
                break;
            case 0x13:
                info.interpreter_fn = &mtlo;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::LO0);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x14:
                info.interpreter_fn = &dsllv;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x16:
                info.interpreter_fn = &dsrlv;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x17:
                info.interpreter_fn = &dsrav;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x18:
                info.interpreter_fn = &mult;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.throughput = 2;
                info.latency = 4;
                info.instruction_type = EE_InstrInfo::InstructionType::MULT;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::LO0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::HI0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x19:
                info.interpreter_fn = &multu;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.throughput = 2;
                info.latency = 4;
                info.instruction_type = EE_InstrInfo::InstructionType::MULT;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::LO0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::HI0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x1A:
                info.interpreter_fn = &div;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.throughput = 37;
                info.latency = 37;
                info.instruction_type = EE_InstrInfo::InstructionType::DIV;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::LO0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::HI0);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x1B:
                info.interpreter_fn = &divu;
                info.pipeline = EE_InstrInfo::Pipeline::MAC0;
                info.throughput = 37;
                info.latency = 37;
                info.instruction_type = EE_InstrInfo::InstructionType::DIV;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::LO0);
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::HI0);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                break;
            case 0x20:
                info.interpreter_fn = &add;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x21:
                info.interpreter_fn = &addu;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x22:
                info.interpreter_fn = &sub;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x23:
                info.interpreter_fn = &subu;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x24:
                info.interpreter_fn = &and_ee;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x25:
                info.interpreter_fn = &or_ee;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x26:
                info.interpreter_fn = &xor_ee;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x27:
                info.interpreter_fn = &nor;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x28:
                info.interpreter_fn = &mfsa;
                info.pipeline = EE_InstrInfo::Pipeline::SA;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (uint8_t)Registers::SA);
                break;
            case 0x29:
                info.interpreter_fn = &mtsa;
                info.pipeline = EE_InstrInfo::Pipeline::SA;
                info.add_dependency(DependencyType::Write, RegType::GPR, (uint8_t)Registers::SA);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2A:
                info.interpreter_fn = &slt;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2B:
                info.interpreter_fn = &sltu;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2C:
                info.interpreter_fn = &dadd;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2D:
                info.interpreter_fn = &daddu;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2E:
                info.interpreter_fn = &dsub;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x2F:
                info.interpreter_fn = &dsubu;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x34:
                info.interpreter_fn = &teq;
                info.pipeline = EE_InstrInfo::Pipeline::COP0;
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x38:
                info.interpreter_fn = &dsll;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x3A:
                info.interpreter_fn = &dsrl;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x3B:
                info.interpreter_fn = &dsra;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x3C:
                info.interpreter_fn = &dsll32;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x3E:
                info.interpreter_fn = &dsrl32;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            case 0x3F:
                info.interpreter_fn = &dsra32;
                info.pipeline = EE_InstrInfo::Pipeline::IntGeneric;
                info.add_dependency(DependencyType::Write, RegType::GPR, (instruction >> 11) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 16) & 0x1F);
                info.add_dependency(DependencyType::Read, RegType::GPR, (instruction >> 21) & 0x1F);
                break;
            default:
                unknown_op("special", instruction, op);
            }
        }

        void sll(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint32_t>(source);
            source <<= shift;
            cpu.set_gpr<int64_t>(dest, (int32_t)source);
        }

        void srl(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint32_t>(source);
            source >>= shift;
            cpu.set_gpr<int64_t>(dest, (int32_t)source);
        }

        void sra(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            int32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<int32_t>(source) >> shift;
            cpu.set_gpr<int64_t>(dest, source);
        }

        void sllv(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<uint32_t>(source);
            source <<= cpu.get_gpr<uint8_t>(shift) & 0x1F;
            cpu.set_gpr<int64_t>(dest, (int32_t)source);
        }

        void srlv(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<uint32_t>(source);
            source >>= cpu.get_gpr<uint8_t>(shift) & 0x1F;
            cpu.set_gpr<int64_t>(dest, (int32_t)source);
        }

        void srav(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<int32_t>(source);
            source >>= cpu.get_gpr<uint8_t>(shift) & 0x1F;
            cpu.set_gpr<int64_t>(dest, source);
        }

        void jr(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t address = (instruction >> 21) & 0x1F;
            cpu.jp(cpu.get_gpr<uint32_t>(address));
        }

        void jalr(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t new_addr = (instruction >> 21) & 0x1F;
            uint32_t return_reg = (instruction >> 11) & 0x1F;
            uint32_t return_addr = cpu.get_PC() + 8;
            cpu.jp(cpu.get_gpr<uint32_t>(new_addr));
            cpu.set_gpr<uint64_t>(return_reg, return_addr);
        }

        void movz(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 21) & 0x1F;
            uint64_t test = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            if (!cpu.get_gpr<uint64_t>(test))
                cpu.set_gpr<uint64_t>(dest, source);
        }

        void movn(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 21) & 0x1F;
            uint64_t test = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            if (cpu.get_gpr<uint64_t>(test))
                cpu.set_gpr<uint64_t>(dest, source);
        }

        void syscall_ee(EmotionEngine& cpu, uint32_t instruction)
        {
            cpu.syscall_exception();
        }

        void break_ee(EmotionEngine& cpu, uint32_t instruction)
        {
            cpu.break_exception();
        }

        void mfhi(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t dest = (instruction >> 11) & 0x1F;
            cpu.mfhi(dest);
        }

        void mthi(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t source = (instruction >> 21) & 0x1F;
            cpu.mthi(source);
        }

        void mflo(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t dest = (instruction >> 11) & 0x1F;
            cpu.mflo(dest);
        }

        void mtlo(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t source = (instruction >> 21) & 0x1F;
            cpu.mtlo(source);
        }

        void dsllv(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source <<= cpu.get_gpr<uint8_t>(shift) & 0x3F;
            cpu.set_gpr<uint64_t>(dest, source);
        }

        void dsrlv(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source >>= cpu.get_gpr<uint8_t>(shift) & 0x3F;
            cpu.set_gpr<int64_t>(dest, source);
        }

        void dsrav(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 21) & 0x1F;
            source = cpu.get_gpr<int64_t>(source);
            source >>= cpu.get_gpr<uint8_t>(shift) & 0x3F;
            cpu.set_gpr<int64_t>(dest, source);
        }

        void mult(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int32_t>(op1);
            op2 = cpu.get_gpr<int32_t>(op2);
            int64_t temp = (int64_t)op1 * op2;
            cpu.set_LO_HI((int64_t)(int32_t)(temp & 0xFFFFFFFF), (int64_t)(int32_t)(temp >> 32));
            cpu.mflo(dest);
        }

        void multu(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t op1 = (instruction >> 21) & 0x1F;
            uint32_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint32_t>(op1);
            op2 = cpu.get_gpr<uint32_t>(op2);
            uint64_t temp = (uint64_t)op1 * op2;
            cpu.set_LO_HI((int64_t)(int32_t)(temp & 0xFFFFFFFF), (int64_t)(int32_t)(temp >> 32));
            cpu.mflo(dest);
        }

        void div(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            op1 = cpu.get_gpr<int32_t>(op1);
            op2 = cpu.get_gpr<int32_t>(op2);
            if (op1 == 0x80000000 && op2 == 0xFFFFFFFF)
            {
                cpu.set_LO_HI((int64_t)(int32_t)0x80000000, 0);
            }
            else if (op2)
            {
                cpu.set_LO_HI((int64_t)(int32_t)(op1 / op2), (int64_t)(int32_t)(op1 % op2));
            }
            else
            {
                int64_t lo;
                if (op1 >= 0)
                    lo = -1;
                else
                    lo = 1;
                cpu.set_LO_HI(lo, (int64_t)(int32_t)op1);
            }
        }

        void divu(EmotionEngine& cpu, uint32_t instruction)
        {
            uint32_t op1 = (instruction >> 21) & 0x1F;
            uint32_t op2 = (instruction >> 16) & 0x1F;
            op1 = cpu.get_gpr<uint32_t>(op1);
            op2 = cpu.get_gpr<uint32_t>(op2);
            if (op2)
            {
                cpu.set_LO_HI((int64_t)(int32_t)(op1 / op2), (int64_t)(int32_t)(op1 % op2));
            }
            else
            {
                cpu.set_LO_HI((int64_t)-1, (int64_t)(int32_t)op1);
            }
        }

        void add(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            int32_t result = op1 + op2;
            cpu.set_gpr<int64_t>(dest, result);
        }

        void addu(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            int32_t result = op1 + op2;
            cpu.set_gpr<int64_t>(dest, result);
        }

        void sub(EmotionEngine& cpu, uint32_t instruction)
        {
            //TODO: overflow
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            int64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int32_t>(op1);
            op2 = cpu.get_gpr<int32_t>(op2);
            int32_t result = op1 - op2;
            cpu.set_gpr<int64_t>(dest, result);
        }

        void subu(EmotionEngine& cpu, uint32_t instruction)
        {
            int32_t op1 = (instruction >> 21) & 0x1F;
            int32_t op2 = (instruction >> 16) & 0x1F;
            int64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int32_t>(op1);
            op2 = cpu.get_gpr<int32_t>(op2);
            int32_t result = op1 - op2;
            cpu.set_gpr<int64_t>(dest, result);
        }

        void and_ee(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint64_t>(op1);
            op2 = cpu.get_gpr<uint64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 & op2);
        }

        void or_ee(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint64_t>(op1);
            op2 = cpu.get_gpr<uint64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 | op2);
        }

        void xor_ee(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint64_t>(op1);
            op2 = cpu.get_gpr<uint64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 ^ op2);
        }

        void nor(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint64_t>(op1);
            op2 = cpu.get_gpr<uint64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, ~(op1 | op2));
        }

        void mfsa(EmotionEngine& cpu, uint32_t instruction)
        {
            int dest = (instruction >> 11) & 0x1F;
            cpu.mfsa(dest);
        }

        void mtsa(EmotionEngine& cpu, uint32_t instruction)
        {
            int source = (instruction >> 21) & 0x1F;
            cpu.mtsa(source);
        }

        void slt(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t op1 = (instruction >> 21) & 0x1F;
            int64_t op2 = (instruction >> 16) & 0x1F;
            int64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 < op2);
        }

        void sltu(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<uint64_t>(op1);
            op2 = cpu.get_gpr<uint64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 < op2);
        }

        void dadd(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t op1 = (instruction >> 21) & 0x1F;
            int64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 + op2);
        }

        void daddu(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t op1 = (instruction >> 21) & 0x1F;
            int64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 + op2);
        }

        void dsub(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t op1 = (instruction >> 21) & 0x1F;
            int64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 - op2);
        }

        void dsubu(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t op1 = (instruction >> 21) & 0x1F;
            int64_t op2 = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            op1 = cpu.get_gpr<int64_t>(op1);
            op2 = cpu.get_gpr<int64_t>(op2);
            cpu.set_gpr<uint64_t>(dest, op1 - op2);
        }

        void teq(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t op1 = (instruction >> 21) & 0x1F;
            uint64_t op2 = (instruction >> 16) & 0x1F;
            if (cpu.get_gpr<uint64_t>(op1) == cpu.get_gpr<uint64_t>(op2))
                cpu.trap_exception();
        }

        void dsll(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source <<= shift;
            cpu.set_gpr<uint64_t>(dest, source);
        }

        void dsrl(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source >>= shift;
            cpu.set_gpr<uint64_t>(dest, source);
        }

        void dsra(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<int64_t>(source);
            source >>= shift;
            cpu.set_gpr<int64_t>(dest, source);
        }

        void dsll32(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source <<= (shift + 32);
            cpu.set_gpr<uint64_t>(dest, source);
        }

        void dsrl32(EmotionEngine& cpu, uint32_t instruction)
        {
            uint64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<uint64_t>(source);
            source >>= (shift + 32);
            cpu.set_gpr<uint64_t>(dest, source);
        }

        void dsra32(EmotionEngine& cpu, uint32_t instruction)
        {
            int64_t source = (instruction >> 16) & 0x1F;
            uint64_t dest = (instruction >> 11) & 0x1F;
            uint32_t shift = (instruction >> 6) & 0x1F;
            source = cpu.get_gpr<int64_t>(source);
            source >>= (shift + 32);
            cpu.set_gpr<int64_t>(dest, source);
        }
    }
}