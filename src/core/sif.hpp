#pragma once
#include <cstdint>
#include <fstream>
#include <functional>
#include <list>
#include <queue>
#include <util/int128.hpp>

namespace ee
{
    class DMAC;
    class EmotionEngine;
}

namespace iop
{
    class DMA;
}

namespace core
{
    struct SifRpcServer
    {
        std::string name;
        uint32_t module_id;
        uint32_t client_ptr;
        std::function<void(SifRpcServer& server, uint32_t fno, uint32_t buff, uint32_t buff_size)> rpc_func;
    };

    class SubsystemInterface
    {
    private:
        ee::EmotionEngine* ee;
        iop::DMA* iop_dma;
        ee::DMAC* dmac;
        uint32_t mscom;
        uint32_t smcom;
        uint32_t msflag;
        uint32_t smflag;
        uint32_t control; //???

        uint32_t oldest_SIF0_data[4];

        std::queue<uint32_t> SIF0_FIFO;
        std::queue<uint32_t> SIF1_FIFO;

        std::list<SifRpcServer> rpc_servers;

        bool sifrpc_bind(uint32_t module, uint32_t client);
        void sifrpc_register_server(std::string name, uint32_t module_id,
            std::function<void(SifRpcServer& server,
                uint32_t fno, uint32_t buff, uint32_t buff_size)>);
    public:
        constexpr static int MAX_FIFO_SIZE = 32;
        SubsystemInterface(ee::EmotionEngine* ee, iop::DMA* iop_dma, ee::DMAC* dmac);

        void reset();
        void register_system_servers();
        int get_SIF0_size();
        int get_SIF1_size();

        void write_SIF0(uint32_t word);
        void send_SIF0_junk(int count);
        void write_SIF1(uint128_t quad);
        uint32_t read_SIF0();
        uint32_t read_SIF1();

        uint32_t get_mscom();
        uint32_t get_smcom();
        uint32_t get_msflag();
        uint32_t get_smflag();
        uint32_t get_control();
        void set_mscom(uint32_t value);
        void set_smcom(uint32_t value);
        void set_msflag(uint32_t value);
        void reset_msflag(uint32_t value);
        void set_smflag(uint32_t value);
        void reset_smflag(uint32_t value);

        void set_control_EE(uint32_t value);
        void set_control_IOP(uint32_t value);

        void ee_log_sifrpc(uint32_t transfer_ptr, int len);

        void load_state(std::ifstream& state);
        void save_state(std::ofstream& state);
    };

    inline int SubsystemInterface::get_SIF0_size()
    {
        return SIF0_FIFO.size();
    }

    inline int SubsystemInterface::get_SIF1_size()
    {
        return SIF1_FIFO.size();
    }
}