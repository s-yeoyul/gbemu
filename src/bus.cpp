#include "gb/bus.hpp"
#include "gb/timer.hpp"
#include "gb/ppu.hpp"

#include <fstream>
#include <iostream>

namespace gb {
	Bus::Bus(Timer &timer, PPU &ppu) : timer_(timer), ppu_(ppu) {}

	u8 Bus::read8(u16 addr) const {
		// Hooking to Timer class
		if(addr >= 0xFF04 && addr <= 0xFF07) return timer_.read8(addr);

		// Hooking to PPU class
		if(addr >= 0xFF40 && addr <= 0xFF4B) return ppu_.read8(addr);
	
		// Memory access
		if(bootrom_enabled && addr < 0x100) return bootrom_[addr];
		else {
			if(addr < 0x8000) return cartridge_[addr];
			else if(addr >= 0x8000 && addr < 0xA000) return ppu_.read8(addr);
			else if(addr >= 0xC000 && addr < 0xE000) return wram_[addr-0xC000];
			else if(addr >= 0xFE00 && addr < 0xFEA0) return ppu_.read8(addr);
			else if(addr >= 0xFF00 && addr < 0xFF80) return ioregs_[addr-0xFF00];
			else if(addr >= 0xFF80 && addr < 0xFFFF) return hram_[addr-0xFF80];
			else if(addr == 0xFFFF) return intr_reg;
			else {
				std::cout << "invalid addr @0x" << std::hex << addr << "\n";
				return 0xFF; // return -1
			}
		}
	}
	void Bus::write8(u16 addr, u8 value) {
		/* NOTE: It is temporary solution */
		if(addr == 0xFF50) {
			bootrom_enabled = false;
		}

		// Hooking to Timer class
		if(addr >= 0xFF04 && addr <= 0xFF07) {
			timer_.write8(addr, value);
			return;
		}

		// Hooking to PPU class
		if(addr >= 0xFF40 && addr <= 0xFF4B) {
			ppu_.write8(addr, value);
			return;
		}

		if(bootrom_enabled && addr < 0x100) {
			// bootrom_[addr] = value;
		}
		else {
			if(addr < 0x8000) {
				//cartridge_[addr] = value;
			}
			else if(addr >= 0x8000 && addr < 0xA000) ppu_.write8(addr, value);
			else if(addr >= 0xC000 && addr < 0xE000) wram_[addr-0xC000] = value;
			else if(addr >= 0xFE00 && addr < 0xFEA0) ppu_.write8(addr, value);
			else if(addr >= 0xFF00 && addr < 0xFF80) ioregs_[addr-0xFF00] = value;
			else if(addr >= 0xFF80 && addr < 0xFFFF) hram_[addr-0xFF80] = value;
			else if(addr == 0xFFFF) intr_reg = value;
			else {
				std::cout << "invalid addr@=0x" << std::hex << addr << std::endl;
			}
		}

		// NOTE: It is temporal Serial communication impl.
		if(addr == 0xFF02) {
			if(value == 0x81) {
				std::cout << ioregs_[0x01];
				ioregs_[0x02] = 0;
			}
		}
	}

	void Bus::tick(int cycles) {
		// 1. Timer tick
		bool timer_intr = timer_.tick(cycles);
		if(timer_intr) {
			ioregs_[0x0F] |= 0x04;
		}
		
		// 2. PPU tick
		u8 ppu_intr = ppu_.tick(cycles);
		if(timer_intr != 0) {
			ioregs_[0x0F] |= ppu_intr;
		}
	}

	bool Bus::load_bootrom(const std::string &path) {
		std::ifstream ifs(path, std::ios::binary);
		if(!ifs) return false;

		ifs.seekg(0, std::ios::end);
		std::streamsize size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		if(size != 0x100) return false; // Bootrom size should be exactly 256B
		if(!ifs.read(reinterpret_cast<char*>(bootrom_.data()), 0x100)) return false;

		bootrom_enabled = true;
		return true;
	}

	bool Bus::load_cartridge(const std::string &path) {
		std::ifstream ifs(path, std::ios::binary);
		if(!ifs) return false;

		ifs.seekg(0, std::ios::end);
		std::streamsize size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		/* NOTE: It is temporary solution */
		if(size != 0x8000) {
			std::cout << size << std::endl;
			return false;
		}
		if(!ifs.read(reinterpret_cast<char*>(cartridge_.data()), size)) return false;
		return true;
	}
}
