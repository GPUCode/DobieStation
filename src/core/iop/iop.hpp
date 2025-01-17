#pragma once
#include <iop/cop0.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <fstream>

namespace core
{
    class Emulator;
}

namespace iop
{
    constexpr const char* REG[] =
    {
        "zero", "at", "v0", "v1",
        "a0", "a1", "a2", "a3",
        "t0", "t1", "t2", "t3",
        "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3",
        "s4", "s5", "s6", "s7",
        "t8", "t9", "k0", "k1",
        "gp", "sp", "fp", "ra"
    };

    struct IOP_ICacheLine
    {
        bool valid;
        uint32_t tag;
    };

    class IOP
    {
    public:
        core::Emulator* e;
        iop::Cop0 cop0;
        uint32_t gpr[32];
        uint32_t PC;
        uint32_t LO, HI;

        /* IOP memory caches */
        uint8_t* ram = nullptr;
        uint8_t scratchpad[1024] = {};
        uint32_t scratchpad_start = 0x1F800000;

        //4 KB bytes / 16 bytes per line = 256 cache lines
        IOP_ICacheLine icache[256];

        uint32_t new_PC;
        uint32_t cache_control;
        int branch_delay;
        bool can_disassemble;
        bool will_branch;
        bool wait_for_IRQ;

        int muldiv_delay;
        int cycles_to_run;

        uint32_t translate_addr(uint32_t addr);
    
    public:
        IOP(core::Emulator* e);
        ~IOP();

        void reset();
        void run(int cycles);
        void halt();
        void unhalt();
        void print_state();
        void set_disassembly(bool dis);
        void set_muldiv_delay(int delay);

        void jp(uint32_t addr);
        void branch(bool condition, int32_t offset);

        void handle_exception(uint32_t addr, uint8_t cause);
        void syscall_exception();
        void interrupt_check(bool i_pass);
        void interrupt();
        void rfe();

        void mfc(int cop_id, int cop_reg, int reg);
        void mtc(int cop_id, int cop_reg, int reg);
        void mtc();

        uint32_t get_PC();
        uint32_t get_gpr(int index);
        uint32_t get_LO();
        uint32_t get_HI();
        void set_PC(uint32_t value);
        void set_gpr(int index, uint32_t value);
        void set_LO(uint32_t value);
        void set_HI(uint32_t value);

        uint8_t read8(uint32_t addr);
        uint16_t read16(uint32_t addr);
        uint32_t read32(uint32_t addr);
        uint32_t read_instr(uint32_t addr);
        void write8(uint32_t addr, uint8_t value);
        void write16(uint32_t addr, uint16_t value);
        void write32(uint32_t addr, uint32_t value);

        void load_state(std::ifstream& state);
        void save_state(std::ofstream& state);
    };

    inline void IOP::halt()
    {
        wait_for_IRQ = true;
    }

    inline void IOP::unhalt()
    {
        wait_for_IRQ = false;
    }

    inline uint32_t IOP::get_PC()
    {
        return PC;
    }

    inline uint32_t IOP::get_gpr(int index)
    {
        return gpr[index];
    }

    inline uint32_t IOP::get_LO()
    {
        if (muldiv_delay)
        {
            cycles_to_run -= muldiv_delay;
            muldiv_delay = 0;
        }
        return LO;
    }

    inline uint32_t IOP::get_HI()
    {
        if (muldiv_delay)
        {
            cycles_to_run -= muldiv_delay;
            muldiv_delay = 0;
        }
        return HI;
    }

    inline void IOP::set_PC(uint32_t value)
    {
        PC = value;
    }

    inline void IOP::set_gpr(int index, uint32_t value)
    {
        if (index)
            gpr[index] = value;
    }

    inline void IOP::set_LO(uint32_t value)
    {
        LO = value;
    }

    inline void IOP::set_HI(uint32_t value)
    {
        HI = value;
    }

    inline void IOP::set_muldiv_delay(int delay)
    {
        if (muldiv_delay)
            cycles_to_run -= muldiv_delay;
        
        muldiv_delay = delay;
    }
}