#include <iostream>
#include <chrono>
#include <thread>

#include "gb/bus.hpp"
#include "gb/cpu.hpp"
#include "gb/timer.hpp"
#include "gb/ppu.hpp"
#include "gb/joypad.hpp"
#include "gb/cartridge.hpp"

const int CYCLES_PER_FRAME = 70224;
const double FPS = 59.7275;
using my_clock = std::chrono::steady_clock;
const auto frame_dt =
		std::chrono::duration_cast<my_clock::duration>(
				std::chrono::duration<double>(1.0 / FPS));


int main() {
	gb::Timer timer;
	gb::PPU ppu;
	gb::Joypad joypad;
	gb::Cartridge cartridge;

	gb::Bus bus(timer, ppu, joypad, cartridge);
	gb::CPU cpu(bus);
	cpu.reset();
	ppu.initPPU();

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

	/* Test Rom: instr_timing.gb
	if(!bus.load_cartridge("roms/instr_timing.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Game 1: Dr. Mario (MBC0)
	if(!bus.load_cartridge("roms/Dr. mario.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Game 2: Tetris (MBC0)
	if(!cartridge.load_cartridge("roms/Tetris.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Game 3: Kirby's Dream Land (MBC1)
	if(!cartridge.load_cartridge("roms/Kirby's Dream Land.gb")) {
		std::cout << "load failed\n";
		return 0;
	} */

	/* Game 4: Pokemon Red (MBC3) */
	if(!cartridge.load_cartridge("roms/Pokemon Red.gb")) {
		std::cout << "load failed\n";
		return 0;
	}

	// Load Save Data
	if(cartridge.is_save()) {
		cartridge.load_savefile();
	}

	auto next_frame = my_clock::now();

	while(ppu.pump_events(joypad)) {
		int budget = CYCLES_PER_FRAME;
		while(budget > 0) {
			int cycles = cpu.step();
			bus.tick(cycles);
			budget -= cycles;
			if(cycles == 0) return 0;
		}
    next_frame += frame_dt;
    std::this_thread::sleep_until(next_frame);

    auto now = my_clock::now();
    if (now > next_frame + frame_dt) next_frame = now;
	}

	// Save data
	if(cartridge.is_save()) {
		cartridge.store_savefile();
	}

	return 0;
}
