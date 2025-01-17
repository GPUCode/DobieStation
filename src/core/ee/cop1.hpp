#pragma once
#include <cstdint>
#include <fstream>

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
    namespace jit
    {
        class EE_JitTranslator;
    }

    struct COP1_CONTROL
    {
        bool su;
        bool so;
        bool sd;
        bool si;
        bool u;
        bool o;
        bool d;
        bool i;
        bool condition;
    };

    union COP1_REG
    {
        float f;
        uint32_t u;
        int32_t s;
    };

    class Cop1
    {
    private:
        COP1_CONTROL control;
        COP1_REG gpr[32];
        COP1_REG accumulator;

        float convert(uint32_t value);
        float accurate_add_sub(uint32_t op1, uint32_t op2, bool isSub);
        void check_overflow(uint32_t& dest, bool set_flags);
        void check_underflow(uint32_t& dest, bool set_flags);
    public:
        Cop1() = default;

        void reset();
        bool get_condition();

        uint32_t get_gpr(int index);
        void mtc(int index, uint32_t value);
        uint32_t cfc(int index);
        void ctc(int index, uint32_t value);

        void cvt_s_w(int dest, int source);
        void cvt_w_s(int dest, int source);

        void add_s(int dest, int reg1, int reg2);
        void sub_s(int dest, int reg1, int reg2);
        void mul_s(int dest, int reg1, int reg2);
        void div_s(int dest, int reg1, int reg2);
        void sqrt_s(int dest, int source);
        void abs_s(int dest, int source);
        void mov_s(int dest, int source);
        void neg_s(int dest, int source);
        void rsqrt_s(int dest, int reg1, int reg2);
        void adda_s(int reg1, int reg2);
        void suba_s(int reg1, int reg2);
        void mula_s(int reg1, int reg2);
        void madd_s(int dest, int reg1, int reg2);
        void msub_s(int dest, int reg1, int reg2);
        void madda_s(int reg1, int reg2);
        void msuba_s(int reg1, int reg2);
        void max_s(int dest, int reg1, int reg2);
        void min_s(int dest, int reg1, int reg2);
        void c_f_s();
        void c_lt_s(int reg1, int reg2);
        void c_eq_s(int reg1, int reg2);
        void c_le_s(int reg1, int reg2);

        void load_state(std::ifstream& state);
        void save_state(std::ofstream& state);

        //Friends needed for JIT convenience
        friend class jit::EE_JIT64;
        friend class jit::EE_JitTranslator;

        friend uint8_t* exec_block_ee(jit::EE_JIT64& jit, EmotionEngine& ee);
    };
}