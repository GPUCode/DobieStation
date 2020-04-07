#ifndef SPU_HPP
#define SPU_HPP
#include <cstdint>
#include <fstream>
#include "spu_envelope.hpp"
#include "../../audio/utils.hpp"
#include "spu_adpcm.hpp"

struct stereo_sample
{
    int16_t left;
    int16_t right;

    void mix(stereo_sample two)
    {
       left += two.left;
       right += two.right;
    }
};

struct voice_mix
{
    bool dry_l;
    bool dry_r;
    bool wet_l;
    bool wet_r;

};

struct Voice
{
    Volume left_vol, right_vol;
    uint16_t pitch;
    uint32_t start_addr;
    uint32_t current_addr;
    uint32_t loop_addr;
    uint32_t crest_out_pos;
    bool loop_addr_specified;

    voice_mix mix_state;

    ADSR adsr;

    int key_switch_timeout;

    uint32_t counter;
    unsigned block_pos;
    int loop_code;

    ADPCM_decoder adpcm;

    int16_t old1 = 0;
    int16_t old2 = 0;
    int16_t old3 = 0;
    int16_t next_sample = 0;

    std::array<int16_t, 28> pcm;


    void set_envelope_stage(ADSR::Stage stage );
    void read_sweep(Envelope envelope, uint16_t val);

    void reset()
    {
        adsr = {};
        left_vol = {};
        right_vol = {};
        pitch = 0;
        start_addr = 0;
        current_addr = 0;
        loop_addr = 0;
        loop_addr_specified = false;
        counter = 0;
        block_pos = 0;
        loop_code = 0;
    }
};

struct core_mix
{
    bool sin_wet_r;
    bool sin_wet_l;
    bool sin_dry_r;
    bool sin_dry_l;
    bool memin_wet_r;
    bool memin_wet_l;
    bool memin_dry_r;
    bool memin_dry_l;
    bool voice_wet_r;
    bool voice_wet_l;
    bool voice_dry_r;
    bool voice_dry_l;

    void read(uint16_t val)
    {
        sin_wet_r = (val & (1 << 0));
        sin_wet_l = (val & (1 << 1));
        sin_dry_r = (val & (1 << 2));
        sin_dry_l = (val & (1 << 3));
        memin_wet_r = (val & (1 << 4));
        memin_wet_l = (val & (1 << 5));
        memin_dry_r = (val & (1 << 6));
        memin_dry_l = (val & (1 << 7));
        voice_wet_r = (val & (1 << 8));
        voice_wet_l = (val & (1 << 9));
        voice_dry_r = (val & (1 << 10));
        voice_dry_l = (val & (1 << 11));
    }

};

struct Reverb
{
    uint32_t effect_area_start;
    uint32_t effect_area_end;
    uint32_t effect_pos;
    uint8_t cycle;

    union
    {
        uint32_t regs[22];
        struct
        {
            uint32_t APF1_SIZE;
            uint32_t APF2_SIZE;
            uint32_t SAME_L_DST;
            uint32_t SAME_R_DST;
            uint32_t COMB1_L_SRC;
            uint32_t COMB1_R_SRC;
            uint32_t COMB2_L_SRC;
            uint32_t COMB2_R_SRC;
            uint32_t SAME_L_SRC;
            uint32_t SAME_R_SRC;
            uint32_t DIFF_L_DST;
            uint32_t DIFF_R_DST;
            uint32_t COMB3_L_SRC;
            uint32_t COMB3_R_SRC;
            uint32_t COMB4_L_SRC;
            uint32_t COMB4_R_SRC;
            uint32_t DIFF_L_SRC;
            uint32_t DIFF_R_SRC;
            uint32_t APF1_L_DST;
            uint32_t APF1_R_DST;
            uint32_t APF2_L_DST;
            uint32_t APF2_R_DST;
        };
    };

    int16_t IN_COEF_L;
    int16_t IN_COEF_R;
    int16_t APF1_VOL;
    int16_t APF2_VOL;
    int16_t IIR_VOL;
    int16_t WALL_VOL;
    int16_t COMB1_VOL;
    int16_t COMB2_VOL;
    int16_t COMB3_VOL;
    int16_t COMB4_VOL;
};

struct SPU_STAT
{
    bool DMA_ready;
    bool DMA_busy;
};

class IOP_INTC;
class IOP_DMA;

class SPU
{
    private:
        unsigned int id;
        IOP_INTC* intc;
        IOP_DMA* dma;

        uint16_t* RAM;
        Voice voices[24];
        static uint16_t core_att[2];
        SPU_STAT status;

        WAVWriter* coreout;
        std::vector<int16_t> left_out_pcm;
        std::vector<int16_t> right_out_pcm;

        static uint16_t spdif_irq;

        Reverb reverb;

        uint32_t transfer_addr;
        uint32_t current_addr;

        bool effect_enable;

        // ADMA volume
        int16_t data_input_volume_l;
        int16_t data_input_volume_r;

        int16_t effect_volume_l;
        int16_t effect_volume_r;

        // core0 to core1 input, only valid on 1
        int16_t core_volume_l;
        int16_t core_volume_r;

        core_mix mix_state;

        uint32_t voice_mixdry_left;
        uint32_t voice_mixdry_right;
        uint32_t voice_mixwet_left;
        uint32_t voice_mixwet_right;
        uint32_t voice_pitch_mod;
        uint32_t voice_noise_gen;
        //ADMA bullshit
        uint16_t autodma_ctrl;
        int ADMA_buf;
        bool buf_filled;
        int ADMA_progress;
        int input_pos;

        static uint32_t IRQA[2];
        uint32_t ENDX;
        uint32_t key_on;
        uint32_t key_off;

        stereo_sample voice_gen_sample(int voice_id);
        int16_t interpolate(int voice);

        void key_on_voice(int v);
        void key_off_voice(int v);

        void spu_check_irq(uint32_t address);
        void spu_irq(int index);

        stereo_sample read_memin();

        uint16_t read(uint32_t addr);
        void write(uint32_t addr, uint16_t data);

        stereo_sample run_reverb(stereo_sample wet);
        uint32_t translate_reverb_offset(uint32_t offset);
        uint16_t read_voice_reg(uint32_t addr);
        void write_voice_reg(uint32_t addr, uint16_t value);
        void write_reverb_reg32(uint32_t addr, uint16_t value);
        void write_voice_addr(uint32_t addr, uint16_t value);

        void clear_dma_req();
        void set_dma_req();
    public:
        SPU(int id, IOP_INTC* intc, IOP_DMA* dma);

        void dump_voice_data();

        bool running_ADMA();

        void reset(uint8_t* RAM);
        void gen_sample();

        void start_DMA(int size);
        void pause_DMA();
        void finish_DMA();

        uint16_t read_mem();
        uint32_t read_DMA();
        void write_DMA(uint32_t value);
        void write_ADMA(uint8_t* RAM);
        void write_mem(uint16_t value);

        uint16_t read16(uint32_t addr);
        void write16(uint32_t addr, uint16_t value);
        uint32_t get_memin_addr();

        void load_state(std::ifstream& state);
        void save_state(std::ofstream& state);

};

inline bool SPU::running_ADMA()
{
    return (autodma_ctrl & (1 << (id - 1)));
}

#endif // SPU_HPP
