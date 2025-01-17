#include <iop/iop.hpp>
#include <iop/interpreter/iop_interpreter.hpp>
#include <ee/interpreter/emotiondisasm.hpp>
#include <emulator.hpp>
#include <util/errors.hpp>
#include <algorithm>
#include <cstring>
#include <fmt/core.h>

namespace iop
{
    IOP::IOP(core::Emulator* e) : 
        e(e)
    {
        ram = new uint8_t[2 * 1024 * 1024];
    }

    IOP::~IOP()
    {
        delete[] ram;
    }

    void IOP::reset()
    {
        cop0.reset();
        PC = 0xBFC00000;
        std::memset(icache, 0, sizeof(icache));
        gpr[0] = 0;
        branch_delay = 0;
        will_branch = false;
        can_disassemble = false;
        wait_for_IRQ = false;
        muldiv_delay = 0;
        cycles_to_run = 0;

        /* HLE method to zero out IOP memory */
        std::memset(ram, 0, 2 * 1024 * 1024);
        scratchpad_start = 0x1F800000;
    }

    uint32_t IOP::translate_addr(uint32_t addr)
    {
        //KSEG0
        if (addr >= 0x80000000 && addr < 0xA0000000)
            return addr - 0x80000000;
        //KSEG1
        if (addr >= 0xA0000000 && addr < 0xC0000000)
            return addr - 0xA0000000;

        //KUSEG, KSEG2
        return addr;
    }

    void IOP::run(int cycles)
    {
        if (!wait_for_IRQ)
        {
            cycles_to_run += cycles;
            while (cycles_to_run > 0)
            {
                cycles_to_run--;
                if (muldiv_delay > 0)
                    muldiv_delay--;
                uint32_t instr = read_instr(PC);
                if (can_disassemble)
                {
                    fmt::print("[IOP] [{:#x}] {:#x} - {}\n", PC, instr, ee::interpreter::disasm_instr(instr, PC).c_str());
                    //print_state();
                }
                
                interpreter::interpret(*this, instr);

                /* Detect calls to the putc function and handle them */
                if (PC == 0x00012C48 || PC == 0x0001420C || PC == 0x0001430C)
                {
                    e->iop_puts();
                }

                PC += 4;

                if (will_branch)
                {
                    if (!branch_delay)
                    {
                        will_branch = false;
                        PC = new_PC;
                        if (PC & 0x3)
                        {
                            Errors::die("[IOP] Invalid PC address $%08X!\n", PC);
                        }
                    }
                    else
                        branch_delay--;
                }
            }
        }
        else if (muldiv_delay)
            muldiv_delay--;

        if (cop0.status.IEc && (cop0.status.Im & cop0.cause.int_pending))
            interrupt();
    }

    void IOP::print_state()
    {
        fmt::print("pc:{:#x}\n", PC);
        for (int i = 1; i < 32; i++)
        {
            fmt::print("{}:{:#x}", REG[i], get_gpr(i));
            if (i % 4 == 3)
                fmt::print("\n");
            else
                fmt::print("\t");
        }
        
        fmt::print("lo:{:#x}\thi:{:#x}\n", LO, HI);
    }

    void IOP::set_disassembly(bool dis)
    {
        can_disassemble = dis;
    }

    void IOP::jp(uint32_t addr)
    {
        if (!will_branch)
        {
            new_PC = addr;
            will_branch = true;
            branch_delay = 1;
        }
    }

    void IOP::branch(bool condition, int32_t offset)
    {
        if (condition)
            jp(PC + offset + 4);
    }

    void IOP::handle_exception(uint32_t addr, uint8_t cause)
    {
        cop0.cause.code = cause;
        if (will_branch)
        {
            cop0.EPC = PC - 4;
            cop0.cause.bd = true;
        }
        else
        {
            cop0.EPC = PC;
            cop0.cause.bd = false;
        }
        cop0.status.IEo = cop0.status.IEp;
        cop0.status.IEp = cop0.status.IEc;
        cop0.status.IEc = false;

        /* We do this to offset PC being incremented */
        PC = addr - 4;
        branch_delay = 0;
        will_branch = false;
    }

    void IOP::syscall_exception()
    {
        handle_exception(0x80000080, 0x08);
    }

    void IOP::interrupt_check(bool i_pass)
    {
        if (i_pass)
            cop0.cause.int_pending |= 0x4;
        else
            cop0.cause.int_pending &= ~0x4;
    }

    void IOP::interrupt()
    {
        fmt::print("[IOP] Processing interrupt!\n");
        handle_exception(0x80000084, 0x00);
        unhalt();
    }

    void IOP::mfc(int cop_id, int cop_reg, int reg)
    {
        switch (cop_id)
        {
        case 0:
            set_gpr(reg, cop0.mfc(cop_reg));
            break;
        default:
            Errors::die("\n[IOP] MFC: Unknown COP%d", cop_id);
        }
    }

    void IOP::mtc(int cop_id, int cop_reg, int reg)
    {
        uint32_t bark = get_gpr(reg);
        switch (cop_id)
        {
        case 0:
            cop0.mtc(cop_reg, bark);
            break;
        default:
            Errors::die("\n[IOP] MTC: Unknown COP%d", cop_id);
        }
    }

    void IOP::rfe()
    {
        cop0.status.KUc = cop0.status.KUp;
        cop0.status.KUp = cop0.status.KUo;

        cop0.status.IEc = cop0.status.IEp;
        cop0.status.IEp = cop0.status.IEo;
    }

    uint8_t IOP::read8(uint32_t addr)
    {
        return e->iop_read8(translate_addr(addr));
    }

    uint16_t IOP::read16(uint32_t addr)
    {
        if (addr & 0x1)
        {
            Errors::die("[IOP] Invalid read16 from $%08X!\n", addr);
        }
        return e->iop_read16(translate_addr(addr));
    }

    uint32_t IOP::read32(uint32_t addr)
    {
        if (addr & 0x3)
        {
            Errors::die("[IOP] Invalid read32 from $%08X!\n", addr);
        }
        if (addr == 0xFFFE0130)
            return cache_control;
        return e->iop_read32(translate_addr(addr));
    }

    uint32_t IOP::read_instr(uint32_t addr)
    {
        /* Uncached RAM waitstate. In the future might be good idea to do BIOS as well */
        if (addr >= 0xA0000000 || !(cache_control & (1 << 11)))
        {
            cycles_to_run -= 4;
            muldiv_delay = std::max(muldiv_delay - 4, 0);
        }

        //This is supposed to be icache handling code.
        //Either due to a misunderstanding of the icache, the lack of cache emulation on the EE, or some other problems,
        //this exposes race conditions in several games (Shadow Hearts, Devil May Cry) and makes them not work.
        //This code should remain commented out until we have a better understanding of PS2 cache timings.
        /*else
        {
            int index = (addr >> 4) & 0xFF;
            uint32_t tag = (addr & 0x1FFFFFFF) >> 12;
            if (!icache[index].valid || icache[index].tag != tag)
            {
                //Cache miss: load 4 words
                //I don't know what the exact count should be. 16 (4 * 4) breaks Fatal Frame 2.
                //Current theory is 4 cycles for the first load + 1 * 3 cycles for sequential loads
                //printf("[IOP] i$ miss $%08X\n", addr);
                cycles_to_run -= 7;
                muldiv_delay = std::max(muldiv_delay - 7, 0);
                icache[index].valid = true;
                icache[index].tag = tag;
            }
        }*/
        return e->iop_read32(addr & 0x1FFFFFFF);
    }

    void IOP::write8(uint32_t addr, uint8_t value)
    {
        if (cop0.status.IsC)
            return;
        e->iop_write8(translate_addr(addr), value);
    }

    void IOP::write16(uint32_t addr, uint16_t value)
    {
        if (cop0.status.IsC)
            return;
        if (addr & 0x1)
        {
            Errors::die("[IOP] Invalid write16 to $%08X!\n", addr);
        }
        e->iop_write16(translate_addr(addr), value);
    }

    void IOP::write32(uint32_t addr, uint32_t value)
    {
        if (cop0.status.IsC)
        {
            //printf("Clearing IOP cache ($%08X)\n", addr);
            icache[(addr >> 4) & 0xFF].valid = false;
            return;
        }
        if (addr & 0x3)
        {
            Errors::die("[IOP] Invalid write32 to $%08X!\n", addr);
        }
        //Check for cache control here, as it's used internally by the IOP
        if (addr == 0xFFFE0130)
            cache_control = value;
        e->iop_write32(translate_addr(addr), value);
    }
}