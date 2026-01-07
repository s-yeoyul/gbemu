#include "gb/cpu.hpp"
#include "gb/bus.hpp"

#include <iostream>

namespace gb {
	CPU::CPU(Bus& bus) : bus_(bus) {}

	void CPU::reset() {
		regs.a = 0;
		regs.b = 0;
		regs.c = 0;
		regs.d = 0;
		regs.e = 0;
		regs.f = 0;
		regs.h = 0;
		regs.l = 0;

		regs.pc = 0x0000;
		regs.sp = 0xFFFE;

		flags.z = 0;
		flags.n = 0;
		flags.h = 0;
		flags.c = 0;

		halted_ = false;
	}

	int CPU::step() {
		if(halted_) return 4; // HALT

		u8 opcode = bus_.read8(regs.pc);
		std::cout << "current opcode=0x" << std::hex << (int)opcode << ", pc=0x" << (int)regs.pc << std::endl;
		regs.pc++;

		if((opcode & 0xC0) == 0x40) { // LD r8, r8; 0x40 ~ 0x7F
			if(opcode == 0x76) { // HALT
				halted_ = true;
				return 4;
			}
			u8 src = opcode & 0x07;
			u8 dest = (opcode & 0x38) >> 3;
			u8 src_val = 0;
			switch(src) {
				case 0:
					src_val = regs.b;
					break;
				case 1:
					src_val = regs.c;
					break;
				case 2:
					src_val = regs.d;
					break;
				case 3:
					src_val = regs.e;
					break;
				case 4:
					src_val = regs.h;
					break;
				case 5:
					src_val = regs.l;
					break;
				case 6:
					{
						u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
						src_val = bus_.read8(hl);
						break;
					}
				case 7:
					src_val = regs.a;
					break;
			}
			switch(dest) {
				case 0:
					regs.b = src_val;
					break;
				case 1:
					regs.c = src_val;
					break;
				case 2:
					regs.d = src_val;
					break;
				case 3:
					regs.e = src_val;
					break;
				case 4:
					regs.h = src_val;
					break;
				case 5:
					regs.l = src_val;
					break;
				case 6:
					{
						u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
						bus_.write8(hl, src_val);
						break;
					}
				case 7:
					regs.a = src_val;
					break;
			}
			if(src == 6 || dest == 6) return 8; // HL included
			else return 4; // HL Not included
		} 

		if((opcode & 0xC0) == 0x80) { // 8-bit Arithmetic & Logical Operations
			u8 reg = (opcode & 0x07);
			u8 reg_val = 0;
			switch(reg) {
				case 0:
					reg_val = regs.b;
					break;
				case 1:
					reg_val = regs.c;
					break;
				case 2:
					reg_val = regs.d;
					break;
				case 3:
					reg_val = regs.e;
					break;
				case 4:
					reg_val = regs.h;
					break;
				case 5:
					reg_val = regs.l;
					break;
				case 6: 
					{
						u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
						reg_val = bus_.read8(hl);
						break;
					}
				case 7:
					reg_val = regs.a;
					break;
			}
			u8 op = (opcode & 0x38) >> 3;
			switch(op) {
				case 0: // ADD A, r8
					{
						u16 temp = regs.a + reg_val;
						flags.z = ((temp & 0xFF) == 0) ? 1 : 0;
						flags.n = 0;
						flags.c = (temp > 0xFF) ? 1 : 0;
						flags.h = ((((regs.a & 0x0F) + (reg_val & 0x0F)) & 0x10) == 0x10) ? 1 : 0;
						regs.a = temp & 0xFF;
						return (reg == 6) ? 8 : 4;
					}
				case 1: // ADC A, r8
					{
						u8 carry = (flags.c) ? 1 : 0;
						u16 temp = regs.a + reg_val + carry;
						flags.z = ((temp & 0xFF) == 0) ? 1 : 0;
						flags.n = 0;
						flags.c = (temp > 0xFF) ? 1 : 0;
						flags.h = ((((regs.a & 0x0F) + (reg_val & 0x0F) + carry) & 0x10) == 0x10) ? 1 : 0;
						regs.a = temp & 0xFF;
						return (reg == 6) ? 8 : 4;
					}
				case 2: // SUB A, r8
				case 7: // CP A, r8
					{
						u16 temp = regs.a - reg_val;
						flags.z = ((temp & 0xFF) == 0) ? 1 : 0;
						flags.n = 1;
						flags.h = ((regs.a & 0x0F) < (reg_val & 0x0F)) ? 1 : 0;
						flags.c = (regs.a < reg_val) ? 1 : 0;
						if(op == 2) regs.a = temp & 0xFF;
						return (reg == 6) ? 8 : 4;
					}
				case 3: // SBC A, r8
					{
						u8 carry = (flags.c) ? 1 : 0;
						u16 temp = regs.a - (reg_val + carry);
						flags.z = ((temp & 0xFF) == 0) ? 1 : 0;
						flags.n = 1;
						flags.c = (regs.a < reg_val + carry) ? 1 : 0;
						flags.h = ((regs.a & 0x0F) < ((reg_val & 0x0F) + carry)) ? 1 : 0;
						regs.a = temp & 0xFF;
						return (reg == 6) ? 8 : 4;
					}
				case 4: // AND A, r8
					{
						u8 result = regs.a & reg_val;
						flags.z = (result == 0) ? 1 : 0;
						flags.n = 0;
						flags.h = 1;
						flags.c = 0;
						regs.a = result;
						return (reg == 6) ? 8 : 4;
					}
				case 5: // XOR A, r8
					{
						u8 result = regs.a ^ reg_val;
						flags.z = (result == 0) ? 1 : 0;
						flags.n = 0;
						flags.h = 0;
						flags.c = 0;
						regs.a = result;
						return (reg == 6) ? 8 : 4;
					}
				case 6: // OR A, r8
					{
						u8 result = regs.a | reg_val;
						flags.z = (result == 0) ? 1 : 0;
						flags.n = 0;
						flags.h = 0;
						flags.c = 0;
						regs.a = result;
						return (reg == 6) ? 8 : 4;
					}
			}
		}

		if(opcode == 0xCB) { // CB prefix
			opcode = bus_.read8(regs.pc++);
			u8 op = (opcode >> 6) & 0x03;
			u8 bit = (opcode >> 3) & 0x07;
			u8 reg = opcode & 0x07;

			u8 reg_val = 0;

			switch(reg) {
				case 0:
					reg_val = regs.b;
					break;
				case 1:
					reg_val = regs.c;
					break;
				case 2:
					reg_val = regs.d;
					break;
				case 3:
					reg_val = regs.e;
					break;
				case 4:
					reg_val = regs.h;
					break;
				case 5:
					reg_val = regs.l;
					break;
				case 6: 
					{
						u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
						reg_val = bus_.read8(hl);
						break;
					}
				case 7:
					reg_val = regs.a;
					break;
			}

			if(op == 0) {
				// 1. Do execution
				switch(bit) {
					case 0: // RLC r8
						{
							u8 hi = (reg_val & 0x80) >> 7;
							reg_val = static_cast<u8>((reg_val << 1) | hi);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (hi == 1) ? 1 : 0;
							break;
						}
					case 1: // RRC r8
						{
							u8 lo = (reg_val & 0x01);
							reg_val = static_cast<u8>((reg_val >> 1) | (lo << 7));
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (lo == 1) ? 1 : 0;
							break;
						}
					case 2: // RL r8
						{
							u8 hi = (reg_val & 0x80) >> 7;
							u8 carry = (flags.c) ? 1 : 0;
							reg_val = static_cast<u8>((reg_val << 1) | carry);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (hi == 1) ? 1 : 0;
							break;
						}
					case 3: // RR r8
						{
							u8 lo = (reg_val & 0x01);
							u8 carry = (flags.c) ? 1 : 0;
							reg_val = static_cast<u8>((reg_val >> 1) | (carry << 7));
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (lo == 1) ? 1 : 0;
							break;
						}
					case 4: // SLA r8
						{
							u8 hi = (reg_val & 0x80) >> 7;
							reg_val = static_cast<u8>((reg_val << 1) & 0xFE);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (hi == 1) ? 1 : 0;
							break;
						}
					case 5: // SRA r8
						{
							u8 lo = (reg_val & 0x01);
							u8 hi = (reg_val & 0x80);
							reg_val = static_cast<u8>((reg_val >> 1) | hi);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (lo == 1) ? 1 : 0;
							break;
						}
					case 6: // SWAP r8
						{
							u8 lo = (reg_val & 0x0F) << 4;
							u8 hi = (reg_val & 0xF0) >> 4; 
							reg_val = static_cast<u8>(lo | hi);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = 0;
							break;
						}
					case 7: // SRL r8
						{
							u8 lo = (reg_val & 0x01);
							reg_val = static_cast<u8>((reg_val >> 1) & 0x7F);
							flags.z = (reg_val == 0) ? 1 : 0;
							flags.n = 0;
							flags.h = 0;
							flags.c = (lo == 1) ? 1 : 0;
							break;
						}
				}
				// 2. Update registers
				switch(reg) {
					case 0:
						regs.b = reg_val;
						break;
					case 1:
						regs.c = reg_val;
						break;
					case 2:
						regs.d = reg_val;
						break;
					case 3:
						regs.e = reg_val;
						break;
					case 4:
						regs.h = reg_val;
						break;
					case 5:
						regs.l = reg_val;
						break;
					case 6: 
						{
							u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
							bus_.write8(hl, reg_val);
							break;
						}
					case 7:
						regs.a = reg_val;
						break;
				}
				
				// 3. Return cycle count
				return (reg == 6) ? 16 : 8;
			}

			int idx = static_cast<int>(bit);
			if(op == 1) { // BIT u3, r8
				u8 test = (reg_val >> idx) & 0x01;
				flags.z = (test == 0) ? 1 : 0;
				flags.n = 0;
				flags.h = 1;
				return (reg == 6) ? 12 : 8;
			}
			if(op == 2) {
				reg_val &= ~(0x01 << idx);
			}
			if(op == 3) {
				reg_val |= (0x01 << idx);
			}
			switch(reg) {
				case 0:
					regs.b = reg_val;
					break;
				case 1:
					regs.c = reg_val;
					break;
				case 2:
					regs.d = reg_val;
					break;
				case 3:
					regs.e = reg_val;
					break;
				case 4:
					regs.h = reg_val;
					break;
				case 5:
					regs.l = reg_val;
					break;
				case 6: 
					{
						u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
						bus_.write8(hl, reg_val);
						break;
					}
				case 7:
					regs.a = reg_val;
					break;
			}
			return (reg == 6) ? 16 : 8;
		}

		switch(opcode) {
			case 0x00: // NOP
				return 4;
			case 0x01: // LD BC, n16
				{
					u8 c = bus_.read8(regs.pc++);
					u8 b = bus_.read8(regs.pc++);
					regs.c = c;
					regs.b = b;
					return 12;
				}
			case 0x02: // LD [BC], A
				{
					u16 bc = (static_cast<u16>(regs.b) << 8) | regs.c;
					bus_.write8(bc, regs.a);	
					return 8;
				}
			case 0x03: // INC BC
				{
					u16 bc = (static_cast<u16>(regs.b) << 8) | regs.c;
					bc++;
					regs.b = static_cast<u8>(bc >> 8);
					regs.c = static_cast<u8>(bc & 0xFF);
					return 8;
				}
			case 0x04: // INC B
				{
					u16 tmp = static_cast<u16>(regs.b + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.b = static_cast<u8>(tmp);
					return 4;
				}
			case 0x05: // DEC B
				{
					u8 tmp = regs.b - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.b = tmp;
					return 4;
				}
			case 0x06: // LD B, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.b = imm;
					return 8;
				}
			case 0x07: // RLCA
				{
					u8 hi = (regs.a & 0x80) >> 7;
					regs.a = (regs.a << 1) | hi;
					flags.z = flags.n = flags.h = 0;
					flags.c = hi;
					return 4;
				}
			case 0x0A: // LD A, [BC]
				{
					u16 bc = (static_cast<u16>(regs.b) << 8) | regs.c;
					regs.a = bus_.read8(bc);
					return 8;
				}
			case 0x0B: // DEC BC
				{
					u16 bc = (static_cast<u16>(regs.b) << 8) | regs.c;
					bc--;
					regs.b = static_cast<u8>(bc >> 8);
					regs.c = static_cast<u8>(bc & 0xFF);
					return 8;
				}
			case 0x0C: // INC C
				{
					u16 tmp = static_cast<u16>(regs.c + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.c = static_cast<u8>(tmp);
					return 4;
				}
			case 0x0D: // DEC C
				{
					u8 tmp = regs.c - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.c = tmp;
					return 4;
				}
			case 0x0E: // LD C, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.c = imm;
					return 8;
				}
			case 0x11: // LD DE, n16
				{
					u8 e = bus_.read8(regs.pc++);
					u8 d = bus_.read8(regs.pc++);
					regs.e = e;
					regs.d = d;
					return 12;
				}
			case 0x12: // LD [DE], A
				{
					u16 de = (static_cast<u16>(regs.d) << 8) | regs.e;
					bus_.write8(de, regs.a);	
					return 8;
				}
			case 0x13: // INC DE
				{
					u16 de = (static_cast<u16>(regs.d) << 8) | regs.e;
					de++;
					regs.d = static_cast<u8>(de >> 8);
					regs.e = static_cast<u8>(de & 0xFF);
					return 8;
				}
			case 0x14: // INC D
				{
					u16 tmp = static_cast<u16>(regs.d + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.d = static_cast<u8>(tmp);
					return 4;
				}
			case 0x15: // DEC D
				{
					u8 tmp = regs.d - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.d = tmp;
					return 4;
				}
			case 0x16: // LD D, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.d = imm;
					return 8;
				}
			case 0x17: // RLA
				{
					u8 hi = (regs.a & 0x80) >> 7;
					regs.a = (regs.a << 1) | static_cast<u8>(flags.c);
					flags.z = flags.h = flags.n = 0;
					flags.c = (hi == 1) ? 1 : 0;
					return 4;
				}
			case 0x18: // JR e8
				{
					int8_t offset = static_cast<int8_t>(bus_.read8(regs.pc++));
					regs.pc += offset;
					return 12;
				}
			case 0x1A: // LD A, [DE]
				{
					u16 de = (static_cast<u16>(regs.d) << 8) | regs.e;
					regs.a = bus_.read8(de);
					return 8;
				}
			case 0x1B: // DEC DE
				{
					u16 de = (static_cast<u16>(regs.d) << 8) | regs.e;
					de--;
					regs.d = static_cast<u8>(de >> 8);
					regs.e = static_cast<u8>(de & 0xFF);
					return 8;
				}
			case 0x1C: // INC E
				{
					u16 tmp = static_cast<u16>(regs.e + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.e = static_cast<u8>(tmp);
					return 4;
				}
			case 0x1D: // DEC E
				{
					u8 tmp = regs.e - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.e = tmp;
					return 4;
				}
			case 0x1E: // LD E, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.e = imm;
					return 8;
				}
			case 0x20: // JR NZ, e8
				{
					int8_t offset = static_cast<int8_t>(bus_.read8(regs.pc++));
					if(!flags.z) {
						regs.pc = static_cast<u16>(regs.pc + offset);
						return 12;
					}
					else return 8;
				}
			case 0x21: // LD HL, n16
				{
					u8 l = bus_.read8(regs.pc++);
					u8 h = bus_.read8(regs.pc++);
					regs.l = l;
					regs.h = h;
					return 12;
				}
			case 0x22: // LD [HL+], A
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					bus_.write8(hl, regs.a);	
					hl++;
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x23: // INC HL
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					hl++;
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x24: // INC H
				{
					u16 tmp = static_cast<u16>(regs.h + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.h = static_cast<u8>(tmp);
					return 4;
				}
			case 0x25: // DEC H
				{
					u8 tmp = regs.h - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.h = tmp;
					return 4;
				}
			case 0x26: // LD H, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.h = imm;
					return 8;
				}
			case 0x28: // JR Z, e8
				{
					int8_t offset = static_cast<int8_t>(bus_.read8(regs.pc++));
					if(flags.z) {
						regs.pc += offset;
						return 12;
					}
					return 8;
				}
			case 0x2A: // LD A, [HL+]
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					regs.a = bus_.read8(hl++);
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x2B: // DEC HL
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					hl--;
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x2C: // INC L
				{
					u16 tmp = static_cast<u16>(regs.l + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.l = static_cast<u8>(tmp);
					return 4;
				}
			case 0x2D: // DEC L
				{
					u8 tmp = regs.l - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.l = tmp;
					return 4;
				}
			case 0x2E: // LD L, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.l = imm;
					return 8;
				}
			case 0x30: // JR NC, e8
				{
					int8_t offset = static_cast<int8_t>(bus_.read8(regs.pc++));
					if(!flags.c) {
						regs.pc = static_cast<u16>(regs.pc + offset);
						return 12;
					}
					else return 8;
				}
			case 0x31: // LD SP, n16
				{
					u16 lo = bus_.read8(regs.pc++);
					u16 hi = bus_.read8(regs.pc++);
					regs.sp = lo | (hi << 8);
					return 12;
				}
			case 0x32: // LD [HL-], A
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					bus_.write8(hl, regs.a);	
					hl--;
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x33: // INC SP
				{
					regs.sp++;
					return 8;
				}
			case 0x34: // INC [HL]
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					u16 tmp = static_cast<u16>(bus_.read8(hl) + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					bus_.write8(hl, static_cast<u8>(tmp));
					return 12;
				}
			case 0x35: // DEC [HL]
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					u8 tmp = bus_.read8(hl) - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					bus_.write8(hl, tmp);
					return 12;
				}
			case 0x36: // LD [HL], n8
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | static_cast<u16>(regs.l);
					u8 imm = bus_.read8(regs.pc++);
					bus_.write8(hl, imm);
					return 12;
				}
			case 0x38: // JR C, e8
				{
					int8_t offset = static_cast<int8_t>(bus_.read8(regs.pc++));
					if(flags.c) {
						regs.pc += offset;
						return 12;
					}
					return 8;
				}
			case 0x3A: // LD A, [HL-]
				{
					u16 hl = (static_cast<u16>(regs.h) << 8) | regs.l;
					regs.a = bus_.read8(hl--);
					regs.h = static_cast<u8>(hl >> 8);
					regs.l = static_cast<u8>(hl & 0xFF);
					return 8;
				}
			case 0x3B: // DEC SP
				{
					regs.sp--;
					return 8;
				}
			case 0x3C: // INC A
				{
					u16 tmp = static_cast<u16>(regs.a + 1);
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = ((tmp & 0x0F) == 0) ? 1 : 0;
					regs.a = static_cast<u8>(tmp);
					return 4;
				}
			case 0x3D: // DEC A
				{
					u8 tmp = regs.a - 1;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((tmp & 0x0F) == 0x0F) ? 1 : 0;
					regs.a = tmp;
					return 4;
				}
			case 0x3E: // LD A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.a = imm;
					return 8;
				}
			case 0xC1: // POP BC
				{
					u8 c = bus_.read8(regs.sp++);
					u8 b = bus_.read8(regs.sp++);
					regs.c = c;
					regs.b = b;
					return 12;
				}
			case 0xC5: // PUSH BC
				{
					bus_.write8(--regs.sp, regs.b);
					bus_.write8(--regs.sp, regs.c);
					return 16;
				}
			case 0xC6: // ADD A, n8
				{
					u16 imm = static_cast<u16>(bus_.read8(regs.pc++));
					u16 tmp = static_cast<u16>(regs.a) + imm; 
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = (((regs.a & 0xF) + (imm & 0xF)) > 0xF) ? 1 : 0;
					flags.c = (tmp > 0xFF) ? 1 : 0;
					regs.a = static_cast<u8>(tmp & 0xFF);
					return 8;
				}
			case 0xC8: // RET Z
				{
					if(flags.z) {
						u8 pc_lo = bus_.read8(regs.sp++);
						u8 pc_hi = bus_.read8(regs.sp++);
						regs.pc = (static_cast<u16>(pc_hi) << 8) | static_cast<u16>(pc_lo);
						return 20;
					}
					else return 8;
				}
			case 0xC9: // RET
				{
					u8 pc_lo = bus_.read8(regs.sp++);
					u8 pc_hi = bus_.read8(regs.sp++);
					regs.pc = (static_cast<u16>(pc_hi) << 8) | static_cast<u16>(pc_lo);
					return 16;
				}
			case 0xCD: // CALL a16
				{
					u16 lo = bus_.read8(regs.pc++);
					u16 hi = bus_.read8(regs.pc++) << 8; 
					u16 addr = lo | hi;

					u8 pc_lo = static_cast<u8>(regs.pc & 0xFF);
					u8 pc_hi = static_cast<u8>(regs.pc >> 8);

					bus_.write8(--regs.sp, pc_hi);
					bus_.write8(--regs.sp, pc_lo);
					regs.pc = addr;
					return 24;
				}
			case 0xCE: // ADC A, n8
				{
					u8 carry = static_cast<u8>(flags.c);
					u16 imm = static_cast<u16>(bus_.read8(regs.pc++));
					u16 tmp = static_cast<u16>(regs.a) + imm + carry; 
					flags.z = ((tmp & 0xFF) == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = (((regs.a & 0xF) + (imm & 0xF) + carry) > 0xF) ? 1 : 0;
					flags.c = (tmp > 0xFF) ? 1 : 0;
					regs.a = static_cast<u8>(tmp & 0xFF);
					return 8;
				}
			case 0xD1: // POP DE
				{
					u8 e = bus_.read8(regs.sp++);
					u8 d = bus_.read8(regs.sp++);
					regs.e = e;
					regs.d = d;
					return 12;
				}
			case 0xD5: // PUSH DE
				{
					bus_.write8(--regs.sp, regs.d);
					bus_.write8(--regs.sp, regs.e);
					return 16;
				}
			case 0xD6: // SUB A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					u8 tmp = regs.a - imm;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((regs.a & 0xF) < (imm & 0xF)) ? 1 : 0;
					flags.c = (regs.a < imm) ? 1 : 0;
					regs.a = tmp;
					return 8;
				}
			case 0xD8: // RET C
				{
					if(flags.c) {
						u8 pc_lo = bus_.read8(regs.sp++);
						u8 pc_hi = bus_.read8(regs.sp++);
						regs.pc = (static_cast<u16>(pc_hi) << 8) | static_cast<u16>(pc_lo);
						return 20;
					}
					else return 8;
				}
			case 0xD9: // RETI
				{
					ime_ = true;
					u8 pc_lo = bus_.read8(regs.sp++);
					u8 pc_hi = bus_.read8(regs.sp++);
					regs.pc = (static_cast<u16>(pc_hi) << 8) | static_cast<u16>(pc_lo);
					return 16;
				}
			case 0xDE: // SBC A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					u8 carry = static_cast<u8>(flags.c);
					u8 tmp = regs.a - (imm + carry);
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((regs.a & 0xF) < ((imm & 0xF) + carry)) ? 1 : 0;
					flags.c = (regs.a < (imm + carry)) ? 1 : 0;
					regs.a = tmp;
					return 8;
				}
			case 0xE1: // POP HL
				{
					u8 l = bus_.read8(regs.sp++);
					u8 h = bus_.read8(regs.sp++);
					regs.l = l;
					regs.h = h;
					return 12;
				}
			case 0xE0: // LDH [a8], A
				{
					u16 addr = 0xFF00 + static_cast<u16>(bus_.read8(regs.pc++));
					bus_.write8(addr, regs.a);
					return 12;
				}
			case 0xE2: // LDH [C], A
				{
					u16 addr = 0xFF00 + static_cast<u16>(regs.c);
					bus_.write8(addr, regs.a);
					return 8;
				}
			case 0xE5: // PUSH HL
				{
					bus_.write8(--regs.sp, regs.h);
					bus_.write8(--regs.sp, regs.l);
					return 16;
				}
			case 0xE6: // AND A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.a &= imm;
					flags.z = (regs.a == 0) ? 1 : 0;
					flags.n = 0;
					flags.h = 1;
					flags.c = 0;
					return 8;
				}
			case 0xEE: // XOR A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.a ^= imm;
					flags.z = (regs.a == 0) ? 1 : 0;
					flags.n = flags.h = flags.c = 0;
					return 8;
				}
			case 0xF0: // LDH A, [a8]
				{
					u16 addr = 0xFF00 + static_cast<u16>(bus_.read8(regs.pc++));
					regs.a = bus_.read8(addr);
					return 12;
				}
			case 0xF1: // POP AF
				{
					u8 f = bus_.read8(regs.sp++);
					u8 a = bus_.read8(regs.sp++);
					flags.z = static_cast<bool>((f & 0x80) >> 7);
					flags.n = static_cast<bool>((f & 0x40) >> 6);
					flags.h = static_cast<bool>((f & 0x20) >> 5); 
					flags.c = static_cast<bool>((f & 0x10) >> 4); 
					regs.a = a;
					return 12;
				}
			case 0xF2: // LDH A, [C]
				{
					u16 addr = 0xFF00 + static_cast<u16>(regs.c);
					regs.a = bus_.read8(addr);
					return 8;
				}
			case 0xF5: // PUSH AF
				{
					bus_.write8(--regs.sp, regs.a);
					u8 regf = (static_cast<u8>(flags.z) << 7) |
						(static_cast<u8>(flags.n) << 6) |
						(static_cast<u8>(flags.h) << 5) |
						(static_cast<u8>(flags.c) << 4);
					bus_.write8(--regs.sp, regf);
					return 16;
				}
			case 0xF6: // OR A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					regs.a |= imm;
					flags.z = (regs.a == 0) ? 1 : 0;
					flags.n = flags.h = flags.c = 0;
					return 8;
				}
			case 0xFE: // CP A, n8
				{
					u8 imm = bus_.read8(regs.pc++);
					u8 tmp = regs.a - imm;
					flags.z = (tmp == 0) ? 1 : 0;
					flags.n = 1;
					flags.h = ((regs.a & 0xF) < (imm & 0xF)) ? 1 : 0;
					flags.c = (regs.a < imm) ? 1 : 0;
					return 8;
				}
			default:
				std::cout << "unimplemented opcode detected @pc=" << regs.pc << ", opcode=0x" << std::hex << static_cast<int>(opcode) << std::endl;
		}
		return 0;
	}
}
