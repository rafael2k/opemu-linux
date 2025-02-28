//
//  bmi.c
//  opemu
//
//  Created by Meowthra on 2019/5/25.
//  Copyright © 2019 Meowthra. All rights reserved.
//  Made in Taiwan.

#include "bmi.h"

int bmi_instruction(struct pt_regs *regs,
                    uint8_t vexreg,
                    uint8_t opcode,
                    uint8_t *modrm,
                    uint8_t high_reg,
                    uint8_t high_index,
                    uint8_t high_base,
                    uint8_t operand_size,
                    uint8_t leading_opcode,
                    uint8_t simd_prefix,
                    uint8_t *bytep,
                    uint8_t ins_size,
                    uint8_t modbyte)
{
    
    uint8_t imm;
    //const char* opcodename;
    uint8_t modreg = (*modrm >> 3) & 0x7;
    uint8_t num_dst = (*modrm >> 3) & 0x7;
    uint8_t num_src = *modrm & 0x7;

    if (high_reg) num_dst += 8;
    if (high_base) num_src += 8;
    
    uint64_t rmaddrs = 0;

    if (is_saved_state64(regs)) {

        M64 m64src, m64vsrc, m64dst, m64res, m64dres;
        
        //X86-64 Register
        get_x64regs(modrm, high_reg, high_index, high_base, &m64src, &m64vsrc, &m64dst, vexreg, regs, modbyte, &rmaddrs);

        int consumed = get_consumed(modrm);
        imm = *((uint8_t*)&bytep[consumed]);
        ins_size += consumed;
        
        switch(opcode) {
                /*** BMI1/2 ***/
            case 0xF0: //rorx
                if (simd_prefix == 3) { //F2
                    if (leading_opcode == 3) {//0F3A
                        rorx64(m64src, &m64res, imm, operand_size);
                        ins_size++;
                        //opcodename = "rorx64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                }
                break;
                
            case 0xF2: //andn
                if (simd_prefix == 0) { //None
                    if (leading_opcode == 2) {//0F38
                        andn64(m64src, m64vsrc, &m64res, operand_size, regs);
                        //opcodename = "andn64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                }
                break;
                
            case 0xF3: //blsr/blsmsk/blsi
                if (simd_prefix == 0) { //None
                    if (leading_opcode == 2) {//0F38
                        if (modreg == 1) {
                            blsr64(m64src, &m64res, operand_size, regs);
                            //opcodename = "blsr64";
                            _load_m64(vexreg, &m64res, regs);
                        }
                        else if (modreg == 2) {
                            blsmsk64(m64src, &m64res, operand_size, regs);
                            //opcodename = "blsmsk64";
                            _load_m64(vexreg, &m64res, regs);
                        }
                        else if (modreg == 3) {
                            blsi64(m64src, &m64res, operand_size, regs);
                            //opcodename = "blsi64";
                            _load_m64(vexreg, &m64res, regs);
                        }
                    }
                }
                break;
                
            case 0xF5: //bzhi/pdep/pext
                if (leading_opcode == 2) {//0F38
                    if (simd_prefix == 0) { //None
                        bzhi64(m64src, m64vsrc, &m64res, operand_size, regs);
                        //opcodename = "bzhi64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                    else if (simd_prefix == 2) { //F3
                        pext64(m64src, m64vsrc, &m64res, operand_size);
                        //opcodename = "pext64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                    else if (simd_prefix == 3) { //F2
                        pdep64(m64src, m64vsrc, &m64res, operand_size);
                        //opcodename = "pdep64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                }
                break;
                
            case 0xF6: //mulx
                if (simd_prefix == 3) { //F2
                    if (leading_opcode == 2) {//0F38
                        mulx64(m64src, &m64res, &m64dres, regs, operand_size);
                        //opcodename = "mulx64";
                        _load_m64(num_dst, &m64res, regs);
                        _load_m64(vexreg, &m64dres, regs);
                    }
                }
                break;
                
            case 0xF7: //bextr/sarx/shrx/shlx
                if (leading_opcode == 2) { //0F38
                    if (simd_prefix == 0) { //None
                        m64res = m64dst;
                        bextr64(m64src, m64vsrc, &m64res, operand_size, regs);
                        //opcodename = "bextr64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                    else if (simd_prefix == 1) { //66
                        shlx64(m64src, m64vsrc, &m64res, operand_size);
                        //opcodename = "shlx64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                    else if (simd_prefix == 2) { //F3
                        sarx64(m64src, m64vsrc, &m64res, operand_size);
                        //opcodename = "sarx64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                    else if (simd_prefix == 3) { //F2
                        shrx64(m64src, m64vsrc, &m64res, operand_size);
                        //opcodename = "shrx64";
                        _load_m64(num_dst, &m64res, regs);
                    }
                }
                break;

            default: return 0;
        }
        
    } else {
        //32-bit
        M32 m32src, m32vsrc, m32dst, m32res, m32dres;
        //X86-64 Register
        get_x64regs(modrm, high_reg, high_index, high_base, &m32src, &m32vsrc, &m32dst, vexreg, regs, modbyte, &rmaddrs);
        
        int consumed = get_consumed(modrm);
        imm = *((uint8_t*)&bytep[consumed]);
        ins_size += consumed;
        
        
        switch(opcode) {
                /*** BMI1/2 ***/
            case 0xF0: //rorx
                if (simd_prefix == 3) { //F2
                    if (leading_opcode == 3) {//0F3A
                        rorx32(m32src, &m32res, imm);
                        ins_size++;
                        //opcodename = "rorx32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                }
                break;
                
            case 0xF2: //andn
                if (simd_prefix == 0) { //None
                    if (leading_opcode == 2) {//0F38
                        andn32(m32src, m32vsrc, &m32res, regs);
                        //opcodename = "andn32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                }
                break;
                
            case 0xF3: //blsr/blsmsk/blsi
                if (simd_prefix == 0) { //None
                    if (leading_opcode == 2) {//0F38
                        if (modreg == 1) {
                            blsr32(m32src, &m32res, regs);
                            //opcodename = "blsr32";
                            _load_m32(vexreg, &m32res, regs);
                        }
                        else if (modreg == 2) {
                            blsmsk32(m32src, &m32res, regs);
                            //opcodename = "blsmsk32";
                            _load_m32(vexreg, &m32res, regs);
                        }
                        else if (modreg == 3) {
                            blsi32(m32src, &m32res, regs);
                            //opcodename = "blsi32";
                            _load_m32(vexreg, &m32res, regs);
                        }
                    }
                }
                break;
                
            case 0xF5: //bzhi/pdep/pext
                if (leading_opcode == 2) {//0F38
                    if (simd_prefix == 0) { //None
                        bzhi32(m32src, m32vsrc, &m32res, regs);
                        //opcodename = "bzhi32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                    else if (simd_prefix == 2) { //F3
                        pext32(m32src, m32vsrc, &m32res);
                        //opcodename = "pext32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                    else if (simd_prefix == 3) { //F2
                        pdep32(m32src, m32vsrc, &m32res);
                        //opcodename = "pdep32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                }
                break;
                
            case 0xF6: //mulx
                if (simd_prefix == 3) { //F2
                    if (leading_opcode == 2) {//0F38
                        mulx32(m32src, &m32res, &m32dres, regs);
                        //opcodename = "mulx32";
                        _load_m32(num_dst, &m32res, regs);
                        _load_m32(vexreg, &m32dres, regs);
                    }
                }
                break;
                
            case 0xF7: //bextr/sarx/shrx/shlx
                if (leading_opcode == 2) { //0F38
                    if (simd_prefix == 0) { //None
                        m32res = m32dst;
                        bextr32(m32src, m32vsrc, &m32res, regs);
                        //opcodename = "bextr32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                    else if (simd_prefix == 1) { //66
                        shlx32(m32src, m32vsrc, &m32res);
                        //opcodename = "shlx32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                    else if (simd_prefix == 2) { //F3
                        sarx32(m32src, m32vsrc, &m32res);
                        //opcodename = "sarx32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                    else if (simd_prefix == 3) { //F2
                        shrx32(m32src, m32vsrc, &m32res);
                        //opcodename = "shrx32";
                        _load_m32(num_dst, &m32res, regs);
                    }
                }
                break;
                
            default: return 0;
        }
    }
    return ins_size;
}
