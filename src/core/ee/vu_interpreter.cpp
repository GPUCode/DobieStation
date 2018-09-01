#include <cstdio>
#include <cstdlib>
#include "vu_interpreter.hpp"
#include "../errors.hpp"

#define printf(fmt, ...)(0)

void VU_Interpreter::interpret(VectorUnit &vu, uint32_t upper_instr, uint32_t lower_instr)
{
    vu.decoder.vf_read0[0] = 0; vu.decoder.vf_read0[1] = 0;
    vu.decoder.vf_read0_field[0] = 0; vu.decoder.vf_read0_field[1] = 0;

    vu.decoder.vf_read1[0] = 0; vu.decoder.vf_read1[1] = 0;
    vu.decoder.vf_read1_field[0] = 0; vu.decoder.vf_read1_field[1] = 0;

    vu.decoder.vf_write[0] = 0; vu.decoder.vf_write[1] = 0;
    vu.decoder.vf_write_field[0] = 0; vu.decoder.vf_write_field[1] = 0;

    vu.decoder.vi_read0 = 0; vu.decoder.vi_read1 = 0;
    vu.decoder.vi_write = 0;

    bool swapops = check_swapops(vu, upper_instr, lower_instr);

    //WaitQ, DIV, RSQRT, SQRT
    if (((lower_instr & 0x800007FC) == 0x800003BC))
    {
        vu.waitq();
    }

    if (!swapops)
        upper(vu, upper_instr);

    //LOI
    if (upper_instr & (1 << 31))
        vu.set_I(lower_instr);
    else
        lower(vu, lower_instr);

    if (swapops)
        upper(vu, upper_instr);

    vu.check_for_FMAC_stall();

    if (upper_instr & (1 << 30))
        vu.end_execution();
}

bool VU_Interpreter::check_swapops(VectorUnit &vu, uint32_t upper_instr, uint32_t lower_instr)
{
    uint8_t upperfd, lowerfs, lowerft;

    upperfd = ((upper_instr >> 2) & 0xf) == 0xf ? ((upper_instr >> 16) & 0x1F) : ((upper_instr >> 6) & 0x1F);
    lowerfs = (upper_instr >> 11) & 0x1F;
    lowerft = (lower_instr >> 16) & 0x1F;

    if ((upperfd == lowerfs || upperfd == lowerft) && !(upper_instr & (1 << 31)))
        return true;
    else
        return false;
}

void VU_Interpreter::upper(VectorUnit &vu, uint32_t instr)
{
    uint8_t op = instr & 0x3F;
    switch (op)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            addbc(vu, instr);
            break;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            subbc(vu, instr);
            break;
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            maddbc(vu, instr);
            break;
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            msubbc(vu, instr);
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            maxbc(vu, instr);
            break;
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            minibc(vu, instr);
            break;
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            mulbc(vu, instr);
            break;
        case 0x1C:
            mulq(vu, instr);
            break;
        case 0x1D:
            maxi(vu, instr);
            break;
        case 0x1E:
            muli(vu, instr);
            break;
        case 0x1F:
            minii(vu, instr);
            break;
        case 0x20:
            addq(vu, instr);
            break;
        case 0x21:
            maddq(vu, instr);
            break;
        case 0x22:
            addi(vu, instr);
            break;
        case 0x23:
            maddi(vu, instr);
            break;
        case 0x24:
            subq(vu, instr);
            break;
        case 0x25:
            msubq(vu, instr);
            break;
        case 0x26:
            subi(vu, instr);
            break;
        case 0x27:
            msubi(vu, instr);
            break;
        case 0x28:
            add(vu, instr);
            break;
        case 0x29:
            madd(vu, instr);
            break;
        case 0x2A:
            mul(vu, instr);
            break;
        case 0x2B:
            max(vu, instr);
            break;
        case 0x2C:
            sub(vu, instr);
            break;
        case 0x2D:
            msub(vu, instr);
            break;
        case 0x2E:
            opmsub(vu, instr);
            break;
        case 0x2F:
            mini(vu, instr);
            break;
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            upper_special(vu, instr);
            break;
        default:
            unknown_op("upper", instr, op);
    }
}

void VU_Interpreter::addbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.addbc(instr);
}

void VU_Interpreter::subbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.subbc(instr);
}

void VU_Interpreter::maddbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.maddbc(instr);
}

void VU_Interpreter::msubbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.msubbc(instr);
}

void VU_Interpreter::maxbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.maxbc(instr);
}

void VU_Interpreter::minibc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.minibc(instr);
}

void VU_Interpreter::mulbc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read0[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.mulbc(instr);
}

void VU_Interpreter::mulq(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.mulq(instr);
}

void VU_Interpreter::maxi(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.maxi(instr);
}

void VU_Interpreter::muli(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.muli(instr);
}

void VU_Interpreter::minii(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.minii(instr);
}

void VU_Interpreter::addq(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.addq(instr);
}

void VU_Interpreter::maddq(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.maddq(instr);
}

void VU_Interpreter::addi(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.addi(instr);
}

void VU_Interpreter::maddi(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.maddi(instr);
}

void VU_Interpreter::subq(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.subq(instr);
}

void VU_Interpreter::msubq(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.msubq(instr);
}

void VU_Interpreter::subi(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.subi(instr);
}

void VU_Interpreter::msubi(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.msubi(instr);
}

void VU_Interpreter::add(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.add(instr);
}

void VU_Interpreter::madd(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.madd(instr);
}

void VU_Interpreter::mul(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.mul(instr);
}

void VU_Interpreter::max(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.max(instr);
}

void VU_Interpreter::sub(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.sub(instr);
}

void VU_Interpreter::msub(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0_field[0] = field;
    vu.decoder.vf_read1_field[0] = field;
    vu.msub(instr);
}

void VU_Interpreter::opmsub(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_write_field[0] = 0xE; //xyz
    vu.decoder.vf_read0_field[0] = 0xE;
    vu.decoder.vf_read1_field[0] = 0xE;
    vu.opmsub(instr);
}

void VU_Interpreter::mini(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.mini(instr);
}

void VU_Interpreter::upper_special(VectorUnit &vu, uint32_t instr)
{
    uint16_t op = (instr & 0x3) | ((instr >> 4) & 0x7C);
    switch (op)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            addabc(vu, instr);
            break;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            subabc(vu, instr);
            break;
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            maddabc(vu, instr);
            break;
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            msubabc(vu, instr);
            break;
        case 0x10:
            itof0(vu, instr);
            break;
        case 0x11:
            itof4(vu, instr);
            break;
        case 0x12:
            itof12(vu, instr);
            break;
        case 0x13:
            itof15(vu, instr);
            break;
        case 0x14:
            ftoi0(vu, instr);
            break;
        case 0x15:
            ftoi4(vu, instr);
            break;
        case 0x16:
            ftoi12(vu, instr);
            break;
        case 0x17:
            ftoi15(vu, instr);
            break;
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            mulabc(vu, instr);
            break;
        case 0x1C:
            mulaq(vu, instr);
            break;
        case 0x1D:
            abs(vu, instr);
            break;
        case 0x1E:
            mulai(vu, instr);
            break;
        case 0x1F:
            clip(vu, instr);
            break;
        case 0x21:
            maddaq(vu, instr);
            break;
        case 0x22:
            addai(vu, instr);
            break;
        case 0x23:
            maddai(vu, instr);
            break;
        case 0x25:
            msubaq(vu, instr);
            break;
        case 0x26:
            subai(vu, instr);
            break;
        case 0x27:
            msubai(vu, instr);
            break;
        case 0x28:
            adda(vu, instr);
            break;
        case 0x29:
            madda(vu, instr);
            break;
        case 0x2A:
            mula(vu, instr);
            break;
        case 0x2C:
            suba(vu, instr);
            break;
        case 0x2D:
            msuba(vu, instr);
            break;
        case 0x2E:
            opmula(vu, instr);
            break;
        case 0x2F:
        case 0x30:
            /**
              * NOP
              */
            break;
        default:
            unknown_op("upper special", instr, op);
    }
}

void VU_Interpreter::addabc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.addabc(instr);
}

void VU_Interpreter::subabc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.subabc(instr);
}

void VU_Interpreter::maddabc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.maddabc(instr);
}

void VU_Interpreter::msubabc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.msubabc(instr);
}

void VU_Interpreter::itof0(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.itof0(instr);
}

void VU_Interpreter::itof4(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.itof4(instr);
}

void VU_Interpreter::itof12(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.itof12(instr);
}

void VU_Interpreter::itof15(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.itof15(instr);
}

void VU_Interpreter::ftoi0(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.ftoi0(instr);
}

void VU_Interpreter::ftoi4(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.ftoi4(instr);
}

void VU_Interpreter::ftoi12(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.ftoi12(instr);
}

void VU_Interpreter::ftoi15(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.ftoi15(instr);
}

void VU_Interpreter::mulabc(VectorUnit &vu, uint32_t instr)
{
    uint8_t bc = instr & 0x3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t bc_reg = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = bc_reg;
    vu.decoder.vf_read1_field[0] = 1 << (3 - bc);
    vu.mulabc(instr);
}

void VU_Interpreter::mulaq(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.mulaq(instr);
}

void VU_Interpreter::abs(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.abs(instr);
}

void VU_Interpreter::mulai(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.mulai(instr);
}

void VU_Interpreter::clip(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = 0xE; //xyz

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = 0x1; //w
    vu.clip(instr);
}

void VU_Interpreter::addai(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.addai(instr);
}

void VU_Interpreter::maddaq(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.maddaq(instr);
}

void VU_Interpreter::maddai(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.maddai(instr);
}

void VU_Interpreter::msubaq(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.msubaq(instr);
}

void VU_Interpreter::subai(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.subai(instr);
}

void VU_Interpreter::msubai(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.msubai(instr);
}

void VU_Interpreter::mula(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.mula(instr);
}

void VU_Interpreter::adda(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.adda(instr);
}

void VU_Interpreter::suba(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.suba(instr);
}

void VU_Interpreter::madda(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.madda(instr);
}

void VU_Interpreter::opmula(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read0_field[0] = 0xE; //xyz
    vu.decoder.vf_read1_field[0] = 0xE; //xyz
    vu.opmula(instr);
}

void VU_Interpreter::msuba(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[0] = reg1;
    vu.decoder.vf_read0_field[0] = field;

    vu.decoder.vf_read1[0] = reg2;
    vu.decoder.vf_read1_field[0] = field;
    vu.msuba(instr);
}

void VU_Interpreter::lower(VectorUnit &vu, uint32_t instr)
{
    if (instr & (1 << 31))
        lower1(vu, instr);
    else
        lower2(vu, instr);
}

void VU_Interpreter::lower1(VectorUnit &vu, uint32_t instr)
{
    uint8_t op = instr & 0x3F;
    switch (op)
    {
        case 0x30:
            iadd(vu, instr);
            break;
        case 0x31:
            isub(vu, instr);
            break;
        case 0x32:
            iaddi(vu, instr);
            break;
        case 0x34:
            iand(vu, instr);
            break;
        case 0x35:
            ior(vu, instr);
            break;
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            lower1_special(vu, instr);
            break;
        default:
            unknown_op("lower1", instr, op);
    }
}

void VU_Interpreter::iadd(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.iadd(dest, reg1, reg2);
}

void VU_Interpreter::isub(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.isub(dest, reg1, reg2);
}

void VU_Interpreter::iaddi(VectorUnit &vu, uint32_t instr)
{
    int8_t imm = (instr >> 6) & 0x1F;
    imm = ((int8_t)(imm << 3)) >> 3;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;

    vu.iaddi(dest, source, imm);
}

void VU_Interpreter::iand(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.iand(dest, reg1, reg2);
}

void VU_Interpreter::ior(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 6) & 0x1F;
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    vu.ior(dest, reg1, reg2);
}

void VU_Interpreter::lower1_special(VectorUnit &vu, uint32_t instr)
{
    uint8_t op = (instr & 0x3) | ((instr >> 4) & 0x7C);
    switch (op)
    {
        case 0x30:
            move(vu, instr);
            break;
        case 0x31:
            mr32(vu, instr);
            break;
        case 0x34:
            lqi(vu, instr);
            break;
        case 0x35:
            sqi(vu, instr);
            break;
        case 0x36:
            lqd(vu, instr);
            break;
        case 0x37:
            sqd(vu, instr);
            break;
        case 0x38:
            div(vu, instr);
            break;
        case 0x39:
            vu_sqrt(vu, instr);
            break;
        case 0x3A:
            rsqrt(vu, instr);
            break;
        case 0x3B:
            waitq(vu, instr);
            break;
        case 0x3C:
            mtir(vu, instr);
            break;
        case 0x3D:
            mfir(vu, instr);
            break;
        case 0x3E:
            ilwr(vu, instr);
            break;
        case 0x3F:
            iswr(vu, instr);
            break;
        case 0x40:
            rnext(vu, instr);
            break;
        case 0x41:
            rget(vu, instr);
            break;
        case 0x42:
            rinit(vu, instr);
            break;
        case 0x43:
            rxor(vu, instr);
            break;
        case 0x64:
            mfp(vu, instr);
            break;
        case 0x68:
            xtop(vu, instr);
            break;
        case 0x69:
            xitop(vu, instr);
            break;
        case 0x6C:
            xgkick(vu, instr);
            break;
        case 0x72:
            eleng(vu, instr);
            break;
        case 0x73:
            erleng(vu, instr);
            break;
        case 0x78:
            esqrt(vu, instr);
            break;
        case 0x79:
            ersqrt(vu, instr);
            break;
        case 0x7B:
            //waitp should wait for the P pipeline to finish, which isn't emulated. For now, let's just flush the pipes.
            vu.flush_pipes();
            break;
        case 0x7E:
            eexp(vu, instr);
            break;
        default:
            unknown_op("lower1 special", instr, op);
    }
}

void VU_Interpreter::move(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.move(instr);
}

void VU_Interpreter::mr32(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;

    vu.decoder.vf_read0[0] = source;
    vu.decoder.vf_read0_field[0] = field;
    vu.mr32(instr);
}

void VU_Interpreter::lqi(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t ft = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.decoder.vf_write[1] = ft;
    vu.decoder.vf_write_field[1] = field;
    vu.lqi(instr);
}

void VU_Interpreter::sqi(VectorUnit& vu, uint32_t instr)
{
    uint32_t fs = (instr >> 11) & 0x1F;
    uint32_t it = (instr >> 16) & 0x1F;
    uint8_t dest_field = (instr >> 21) & 0xF;
    vu.decoder.vf_read0[1] = fs;
    vu.decoder.vf_read0_field[1] = dest_field;
    vu.sqi(instr);
}

void VU_Interpreter::lqd(VectorUnit &vu, uint32_t instr)
{
    uint32_t is = (instr >> 11) & 0x1F;
    uint32_t ft = (instr >> 16) & 0x1F;
    uint8_t dest_field = (instr >> 21) & 0xF;
    vu.decoder.vf_write[1] = ft;
    vu.decoder.vf_write_field[1] = dest_field;
    vu.lqd(instr);
}

void VU_Interpreter::sqd(VectorUnit &vu, uint32_t instr)
{
    uint32_t fs = (instr >> 11) & 0x1F;
    uint32_t it = (instr >> 16) & 0x1F;
    uint8_t dest_field = (instr >> 21) & 0xF;
    vu.decoder.vf_read0[1] = fs;
    vu.decoder.vf_read0_field[1] = dest_field;
    vu.sqd(instr);
}

void VU_Interpreter::div(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;
    uint8_t ftf = (instr >> 23) & 0x3;

    vu.decoder.vf_read0[1] = reg1;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);

    vu.decoder.vf_read1[1] = reg2;
    vu.decoder.vf_read1_field[1] = 1 << (3 - ftf);
    vu.div(instr);
}

void VU_Interpreter::vu_sqrt(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 16) & 0x1F;
    uint8_t ftf = (instr >> 23) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - ftf);
    vu.vu_sqrt(instr);
}

void VU_Interpreter::rsqrt(VectorUnit &vu, uint32_t instr)
{
    uint8_t reg1 = (instr >> 11) & 0x1F;
    uint8_t reg2 = (instr >> 16) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;
    uint8_t ftf = (instr >> 23) & 0x3;

    vu.decoder.vf_read0[1] = reg1;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);

    vu.decoder.vf_read1[1] = reg2;
    vu.decoder.vf_read1_field[1] = 1 << (3 - ftf);
    vu.rsqrt(instr);
}

void VU_Interpreter::waitq(VectorUnit &vu, uint32_t instr)
{
    vu.waitq();
}

void VU_Interpreter::mtir(VectorUnit &vu, uint32_t instr)
{
    uint32_t fs = (instr >> 11) & 0x1F;
    uint32_t it = (instr >> 16) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;
    vu.decoder.vf_read0[1] = fs;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.decoder.vi_write = it;
    vu.mtir(instr);
}

void VU_Interpreter::mfir(VectorUnit &vu, uint32_t instr)
{
    uint32_t is = (instr >> 11) & 0x1F;
    uint32_t ft = (instr >> 16) & 0x1F;
    uint8_t dest_field = (instr >> 21) & 0xF;
    vu.decoder.vi_read0 = is;
    vu.decoder.vf_write[1] = ft;
    vu.decoder.vf_write_field[1] = dest_field;
    vu.mfir(instr);
}

void VU_Interpreter::ilwr(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.ilwr(field, it, is);
}

void VU_Interpreter::iswr(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.iswr(field, it, is);
}

void VU_Interpreter::rnext(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[1] = dest;
    vu.decoder.vf_write_field[1] = field;
    vu.rnext(instr);
}

void VU_Interpreter::rget(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[1] = dest;
    vu.decoder.vf_write_field[1] = field;
    vu.rget(instr);
}

void VU_Interpreter::rinit(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.rinit(instr);
}

void VU_Interpreter::rxor(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.rxor(instr);
}

void VU_Interpreter::mfp(VectorUnit &vu, uint32_t instr)
{
    uint8_t dest = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[0] = dest;
    vu.decoder.vf_write_field[0] = field;
    vu.mfp(instr);
}

void VU_Interpreter::xtop(VectorUnit &vu, uint32_t instr)
{
    uint8_t it = (instr >> 16) & 0x1F;
    vu.xtop(it);
}

void VU_Interpreter::xitop(VectorUnit &vu, uint32_t instr)
{
    uint8_t it = (instr >> 16) & 0x1F;
    vu.xitop(it);
}

void VU_Interpreter::xgkick(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    vu.xgkick(is);
}

void VU_Interpreter::eleng(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 0xE; //xyz
    vu.eleng(instr);
}

void VU_Interpreter::erleng(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 0xE; //xyz
    vu.erleng(instr);
}

void VU_Interpreter::esqrt(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.esqrt(instr);
}

void VU_Interpreter::ersqrt(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.ersqrt(instr);
}

void VU_Interpreter::eexp(VectorUnit &vu, uint32_t instr)
{
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t fsf = (instr >> 21) & 0x3;

    vu.decoder.vf_read0[1] = source;
    vu.decoder.vf_read0_field[1] = 1 << (3 - fsf);
    vu.eexp(instr);
}

void VU_Interpreter::lower2(VectorUnit &vu, uint32_t instr)
{
    uint8_t op = (instr >> 25) & 0x7F;
    switch (op)
    {
        case 0x00:
            lq(vu, instr);
            break;
        case 0x01:
            sq(vu, instr);
            break;
        case 0x04:
            ilw(vu, instr);
            break;
        case 0x05:
            isw(vu, instr);
            break;
        case 0x08:
            iaddiu(vu, instr);
            break;
        case 0x09:
            isubiu(vu, instr);
            break;
        case 0x11:
            fcset(vu, instr);
            break;
        case 0x12:
            fcand(vu, instr);
            break;
        case 0x13:
            fcor(vu, instr);
            break;
        case 0x15:
            fsset(vu, instr);
            break;
        case 0x16:
            fsand(vu, instr);
            break;
        case 0x18:
            fmeq(vu, instr);
            break;
        case 0x1A:
            fmand(vu, instr);
            break;
        case 0x1B:
            fmor(vu, instr);
            break;
        case 0x1C:
            fcget(vu, instr);
            break;
        case 0x20:
            b(vu, instr);
            break;
        case 0x21:
            bal(vu, instr);
            break;
        case 0x24:
            jr(vu, instr);
            break;
        case 0x25:
            jalr(vu, instr);
            break;
        case 0x28:
            ibeq(vu, instr);
            break;
        case 0x29:
            ibne(vu, instr);
            break;
        case 0x2C:
            ibltz(vu, instr);
            break;
        case 0x2D:
            ibgtz(vu, instr);
            break;
        case 0x2E:
            iblez(vu, instr);
            break;
        case 0x2F:
            ibgez(vu, instr);
            break;
        default:
            unknown_op("lower2", instr, op);
    }
}

void VU_Interpreter::lq(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t ft = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_write[1] = ft;
    vu.decoder.vf_write_field[1] = field;
    vu.lq(instr);
}

void VU_Interpreter::sq(VectorUnit &vu, uint32_t instr)
{
    uint8_t fs = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;

    vu.decoder.vf_read0[1] = fs;
    vu.decoder.vf_read0_field[1] = field;
    vu.sq(instr);
}

void VU_Interpreter::ilw(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 16;
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.ilw(field, it, is, imm);
}

void VU_Interpreter::isw(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 16;
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    uint8_t field = (instr >> 21) & 0xF;
    vu.isw(field, it, is, imm);
}

void VU_Interpreter::iaddiu(VectorUnit &vu, uint32_t instr)
{
    uint16_t imm = instr & 0x7FF;
    imm |= ((instr >> 21) & 0xF) << 11;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    vu.iaddiu(dest, source, imm);
}

void VU_Interpreter::isubiu(VectorUnit &vu, uint32_t instr)
{
    uint16_t imm = instr & 0x7FF;
    imm |= ((instr >> 21) & 0xF) << 11;
    uint8_t source = (instr >> 11) & 0x1F;
    uint8_t dest = (instr >> 16) & 0x1F;
    vu.isubiu(dest, source, imm);
}

void VU_Interpreter::fcset(VectorUnit &vu, uint32_t instr)
{
    vu.fcset(instr & 0xFFFFFF);
}

void VU_Interpreter::fcand(VectorUnit &vu, uint32_t instr)
{
    vu.fcand(instr & 0xFFFFFF);
}

void VU_Interpreter::fcor(VectorUnit &vu, uint32_t instr)
{
    vu.fcor(instr & 0xFFFFFF);
}

void VU_Interpreter::fsset(VectorUnit &vu, uint32_t instr)
{
    uint32_t imm = ((instr >> 10) & 0x800) | (instr & 0x7FF);
    vu.fsset(imm);
}

void VU_Interpreter::fsand(VectorUnit &vu, uint32_t instr)
{
    uint32_t imm = ((instr >> 10) & 0x800) | (instr & 0x7FF);
    uint8_t dest = (instr >> 16) & 0x1F;
    vu.fsand(dest, imm);
}

void VU_Interpreter::fmeq(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    vu.fmeq(it, is);
}

void VU_Interpreter::fmand(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    vu.fmand(it, is);
}

void VU_Interpreter::fmor(VectorUnit &vu, uint32_t instr)
{
    uint8_t is = (instr >> 11) & 0x1F;
    uint8_t it = (instr >> 16) & 0x1F;
    vu.fmor(it, is);
}

void VU_Interpreter::fcget(VectorUnit &vu, uint32_t instr)
{
    uint8_t it = (instr >> 16) & 0x1F;
    vu.fcget(it);
}

void VU_Interpreter::b(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;
    printf("[VU] B $%x (Imm $%x)\n", vu.get_PC() + 16 + imm, imm);
    vu.branch(true, imm, false);
}

void VU_Interpreter::bal(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    uint8_t link_reg = (instr >> 16) & 0x1F;
    printf("[VU] BAL $%x (Imm $%x)\n", vu.get_PC() + 16 + imm, imm);
    vu.branch(true, imm, true, link_reg);
}

void VU_Interpreter::jr(VectorUnit &vu, uint32_t instr)
{
    uint8_t addr_reg = (instr >> 11) & 0x1F;
    uint16_t addr = vu.get_int(addr_reg) * 8;
    printf("[VU] JR vi%d ($%x)\n", addr_reg, addr);
    vu.jp(addr, false);
}

void VU_Interpreter::jalr(VectorUnit &vu, uint32_t instr)
{
    uint8_t addr_reg = (instr >> 11) & 0x1F;
    uint16_t addr = vu.get_int(addr_reg) * 8;

    uint8_t link_reg = (instr >> 16) & 0x1F;
    printf("[VU] JR vi%d ($%x) link vi%d\n", addr_reg, addr, link_reg);
    vu.jp(addr, true, link_reg);
}

void VU_Interpreter::ibeq(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    uint16_t is = (instr >> 11) & 0x1F;
    uint16_t it = (instr >> 16) & 0x1F;
    is = vu.get_int(is);
    it = vu.get_int(it);
    printf("[VU] IBEQ vi%d($%08X)==vi%d($%08X) $%x (Imm $%x)\n", (instr >> 11) & 0x1F, is, (instr >> 16) & 0x1F, it, vu.get_PC() + 16 + imm, imm);
    vu.branch(is == it, imm, false);
}

void VU_Interpreter::ibne(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    uint16_t is = (instr >> 11) & 0x1F;
    uint16_t it = (instr >> 16) & 0x1F;
    is = vu.get_int(is);
    it = vu.get_int(it);
    printf("[VU] IBNE vi%d($%08X)!=vi%d($%08X) $%x (Imm $%x)\n", (instr >> 11) & 0x1F, is, (instr >> 16) & 0x1F, it, vu.get_PC() + 16 + imm, imm);
    vu.branch(is != it, imm, false);
}

void VU_Interpreter::ibltz(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    int16_t reg = (int16_t)vu.get_int((instr >> 11) & 0x1F);
    printf("[VU] IBLTZ vi%d($%08X) < 0 $%x (Imm $%x)\n", (instr >> 11) & 0x1F, reg, vu.get_PC() + 16 + imm, imm);
    vu.branch(reg < 0, imm, false);
}

void VU_Interpreter::ibgtz(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    int16_t reg = (int16_t)vu.get_int((instr >> 11) & 0x1F);
    printf("[VU] IBGTZ vi%d($%08X) > 0 $%x (Imm $%x)\n", (instr >> 11) & 0x1F, reg, vu.get_PC() + 16 + imm, imm);
    vu.branch(reg > 0, imm, false);
}

void VU_Interpreter::iblez(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    int16_t reg = (int16_t)vu.get_int((instr >> 11) & 0x1F);
    printf("[VU] IBLEZ vi%d($%08X) <= 0 $%x (Imm $%x)\n", (instr >> 11) & 0x1F, reg, vu.get_PC() + 16 + imm, imm);
    vu.branch(reg <= 0, imm, false);
}

void VU_Interpreter::ibgez(VectorUnit &vu, uint32_t instr)
{
    int16_t imm = instr & 0x7FF;
    imm = ((int16_t)(imm << 5)) >> 5;
    imm *= 8;

    int16_t reg = (int16_t)vu.get_int((instr >> 11) & 0x1F);
    printf("[VU] IBGEZ vi%d($%08X) >= 0 $%x (Imm $%x)\n", (instr >> 11) & 0x1F, reg, vu.get_PC() + 16 + imm, imm);
    vu.branch(reg >= 0, imm, false);
}

void VU_Interpreter::unknown_op(const char *type, uint32_t instruction, uint16_t op)
{
    Errors::die("[VU_Interpreter] Unknown %s op $%02X: $%08X\n", type, op, instruction);
}
