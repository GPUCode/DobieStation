#pragma once
#include <cstdint>
#include <fstream>
#include <functional>
#include <list>
#include <ee/cop0.hpp>
#include <ee/cop1.hpp>
#include <util/int128.hpp>
#include <memory>

namespace core
{
    class Emulator;
    class SubsystemInterface;
}

namespace vu
{
    class VectorUnit;
}

namespace ee
{
    class EmotionEngine;
    namespace jit
    {
        class EE_JIT64;
    }
}

extern "C" uint8_t * exec_block_ee(ee::jit::EE_JIT64& jit, ee::EmotionEngine& ee);

namespace ee
{
    //Handler used for Deci2Call (syscall 0x7C)
    struct Deci2Handler
    {
        bool active;
        uint32_t device;
        uint32_t addr;
    };

    struct EE_ICacheLine
    {
        bool lfu[2];
        uint32_t tag[2];
    };

    /* Taken from PS2SDK */
    struct EE_OsdConfigParam
    {
        /* 0 = enabled, 1 = disabled */
        uint32_t spdifMode : 1;
        /* 0 = 4:3, 1 = fullscreen, 2 = 16:9 */
        uint32_t screenType : 2;
        /* 0 = rgb(scart), 1 = component */
        uint32_t videoOutput : 1;
        /* 0 = japanese, 1 = english(non-japanese) */
        uint32_t japLanguage : 1;
        /* Playstation driver settings. */
        uint32_t ps1drvConfig : 8;
        /* 0 = early Japanese OSD, 1 = OSD2, 2 = OSD2 with extended languages.
         * Early kernels cannot retain the value set in this field (Hence always 0). */
        uint32_t version : 3;
        /* LANGUAGE_??? value */
        uint32_t language : 5;
        /* timezone minutes offset from gmt */
        uint32_t timezoneOffset : 11;
    };

    /* EE register */
    static const char* REG[] =
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

    class EmotionEngine
    {
    public:
        core::Emulator* e;

        uint64_t cycle_count;
        int32_t cycles_to_run;
        uint64_t run_event;

        std::unique_ptr<Cop0> cp0;
        std::unique_ptr<Cop1> fpu;

        uint8_t** tlb_map;

        EE_OsdConfigParam osd_config_param;

        uint8_t* rdram = nullptr;
        uint8_t scratchpad[16 * 1024] = {};

        //Each register is 128-bit
        alignas(16) uint8_t gpr[32 * sizeof(uint64_t) * 2];
        
        /* Using this we can load LO and HI in the same AVX register thus
           improving performance */
        union
        {
            uint128_t LO_HI[2];
            struct
            {
                uint128_t LO, HI;
            };
        };
        
        uint32_t PC, new_PC;
        uint64_t SA;

        /*
           Property used by the JIT for COP2 sync purposes
           Updated upon every COP2 instruction, necessary as a COP2 instruction in a branch delay slot
           may otherwise mutate PC after a branch.
        */
        uint32_t PC_now;

        EE_ICacheLine icache[128];

        bool wait_for_IRQ, wait_for_VU0, wait_for_interlock;
        bool branch_on;
        bool can_disassemble;
        int delay_slot;

        Deci2Handler deci2handlers[128];
        int deci2size;

        bool flush_jit_cache;

        std::function<void(EmotionEngine&)> run_func;

        uint32_t get_paddr(uint32_t vaddr);
        void handle_exception(uint32_t new_addr, uint8_t code);
        void deci2call(uint32_t func, uint32_t param);

        void log_sifrpc(uint32_t dma_struct_ptr, int len);
    
    public:
        EmotionEngine(core::Emulator* e);
        ~EmotionEngine();

        static const char* SYSCALL(int id);
        
        void reset();
        void init_tlb();
        void run(int cycles);
        void run_interpreter();
        void run_jit();
        uint64_t get_cycle_count();
        uint64_t get_cycle_count_goal();
        void set_cycle_count(uint64_t value);
        void halt();
        void unhalt();
        void print_state();
        void set_disassembly(bool dis);
        void set_run_func(std::function<void(EmotionEngine&)> func);

        template <typename T> T get_gpr(int id, int offset = 0);
        template <typename T> T get_LO(int offset = 0);
        template <typename T> void set_gpr(int id, T value, int offset = 0);
        template <typename T> void set_LO(int id, T value, int offset = 0);
        
        uint32_t get_PC();
        uint32_t get_PC_now();
        uint64_t get_LO();
        uint64_t get_LO1();
        uint64_t get_HI();
        uint64_t get_HI1();
        uint64_t get_SA();
        Cop1& get_FPU();
        vu::VectorUnit& get_VU0();
        bool check_interlock();
        void clear_interlock();
        bool vu0_wait();

        uint32_t read_instr(uint32_t address);

        uint8_t read8(uint32_t address);
        uint16_t read16(uint32_t address);
        uint32_t read32(uint32_t address);
        uint64_t read64(uint32_t address);
        uint128_t read128(uint32_t address);

        void set_PC(uint32_t addr);
        void write8(uint32_t address, uint8_t value);
        void write16(uint32_t address, uint16_t value);
        void write32(uint32_t address, uint32_t value);
        void write64(uint32_t address, uint64_t value);
        void write128(uint32_t address, uint128_t value);

        void jp(uint32_t new_addr);
        void branch(bool condition, int offset);
        void branch_likely(bool condition, int offset);
        void cfc(int cop_id, int reg, int cop_reg, uint32_t instruction);
        void ctc(int cop_id, int reg, int cop_reg, uint32_t instruction);
        void mfc(int cop_id, int reg, int cop_reg);
        void mtc(int cop_id, int reg, int cop_reg);
        void lwc1(uint32_t addr, int index);
        void lqc2(uint32_t addr, int index);
        void swc1(uint32_t addr, int index);
        void sqc2(uint32_t addr, int index);

        void invalidate_icache_indexed(uint32_t addr);

        void mfhi(int index);
        void mthi(int index);
        void mflo(int index);
        void mtlo(int index);
        void mfhi1(int index);
        void mthi1(int index);
        void mflo1(int index);
        void mtlo1(int index);
        void mfsa(int index);
        void mtsa(int index);
        void pmfhi(int index);
        void pmflo(int index);
        void pmthi(int index);
        void pmtlo(int index);
        void set_SA(uint64_t value);
        void set_LO_HI(uint64_t a, uint64_t b, bool hi = false);

        void hle_syscall();
        void syscall_exception();
        void break_exception();
        void trap_exception();
        void int0();
        void int1();
        void int_timer();
        void set_int0_signal(bool value);
        void set_int1_signal(bool value);

        void tlbr();
        void tlbwi();
        void tlbp();
        void eret();
        void ei();
        void di();
        void cp0_bc0(int32_t offset, bool test_true, bool likely);
        void mtps(int reg);
        void mtpc(int pc_reg, int reg);
        void mfps(int reg);
        void mfpc(int pc_reg, int reg);

        void fpu_bc1(int32_t offset, bool test_true, bool likely);
        void cop2_bc2(int32_t offset, bool test_true, bool likely);

        void qmfc2(int dest, int cop_reg);
        void qmtc2(int source, int cop_reg);
        void cop2_updatevu0();

        void load_state(std::ifstream& state);
        void save_state(std::ofstream& state);
    };

    template <typename T>
    inline T EmotionEngine::get_gpr(int id, int offset)
    {
        return *(T*)&gpr[(id * sizeof(uint64_t) * 2) + (offset * sizeof(T))];
    }

    template <typename T>
    inline void EmotionEngine::set_gpr(int id, T value, int offset)
    {
        if (id)
            *(T*)&gpr[(id * sizeof(uint64_t) * 2) + (offset * sizeof(T))] = value;
    }

    // Returns the current cycle count at a given moment
    inline uint64_t EmotionEngine::get_cycle_count()
    {
        return cycle_count;
    }

    // Return how many cycles the EE should be running until
    inline uint64_t EmotionEngine::get_cycle_count_goal()
    {
        return cycle_count + cycles_to_run;
    }

    inline void EmotionEngine::set_cycle_count(uint64_t value)
    {
        cycle_count = value;
    }

    inline void EmotionEngine::halt()
    {
        wait_for_IRQ = true;
        cycles_to_run = 0;
    }

    inline void EmotionEngine::unhalt()
    {
        wait_for_IRQ = false;
        if (cycles_to_run < 0)
            cycles_to_run = 0;
    }
}