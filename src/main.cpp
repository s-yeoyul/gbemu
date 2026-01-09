#include <iostream>

#include "gb/bus.hpp"
#include "gb/cpu.hpp"
#include "gb/timer.hpp"

#include <filesystem>

int main() {
	gb::Timer timer;
	gb::Bus bus(timer);
	gb::CPU cpu(bus);
	cpu.reset();

	if(!bus.load_bootrom("roms/bootix_dmg.bin")) {
		std::cout << "load failed\n";
		return 0;
	}

	/* Test Rom 1
	if(!bus.load_cartridge("roms/01-special.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */


	/* Test Rom 2
	if(!bus.load_cartridge("roms/02-interrupts.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 3
	if(!bus.load_cartridge("roms/03-op sp,hl.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 4
	if(!bus.load_cartridge("roms/04-op r,imm.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 5
	if(!bus.load_cartridge("roms/05-op rp.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 6
	if(!bus.load_cartridge("roms/06-ld r,r.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 7
	if(!bus.load_cartridge("roms/07-jr,jp,call,ret,rst.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 8
	if(!bus.load_cartridge("roms/08-misc instrs.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */
	
	/* Test Rom 9
	if(!bus.load_cartridge("roms/09-op r,r.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 10
	if(!bus.load_cartridge("roms/10-bit ops.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom 11
	if(!bus.load_cartridge("roms/11-op a,(hl).gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Test Rom: instr_timing.gb */
	if(!bus.load_cartridge("roms/instr_timing.gb")) {
		std::cout << "load failed\n";
		return 0;
	}

	while(1) {
		int cycles = cpu.step();
		bus.tick(cycles);
		// NOTE: This is temporary solution
		if(cycles == 0) break;
	}
	return 0;
}
