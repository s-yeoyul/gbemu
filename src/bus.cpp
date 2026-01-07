#include "gb/bus.hpp"

#include <fstream>
#include <iostream>

namespace gb {
	u8 Bus::read8(u16 addr) const {
		/* NOTE: It is temporary solution */
		if(addr == 0xFF44) return 0x90;

		if(bootrom_enabled && addr <= 0x100) return bootrom_[addr];
		else {
			if(addr >= 0x8000 && addr < 0xA000) return vram_[addr-0x8000];
			else if(addr >= 0xC000 && addr < 0xE000) return wram_[addr-0xC000];
			else if(addr >= 0xFF00 && addr < 0xFF80) return ioregs_[addr-0xFF00];
			else if(addr >= 0xFF80 && addr < 0xFFFF) return hram_[addr-0xFF80];
			else {
				std::cout << "invalid bootrom addr\n";
				return 0xFF; // return -1
			}
		}
	}
	void Bus::write8(u16 addr, u8 value) {
		/* NOTE: It is temporary solution */
		if(addr == 0xFF50) {
			bootrom_enabled = false;
		}
		if(bootrom_enabled && addr <= 0x100) {
			bootrom_[addr] = value;
		}
		else {
			if(addr >= 0x8000 && addr < 0xA000) vram_[addr-0x8000] = value;
			else if(addr >= 0xC000 && addr < 0xE000) wram_[addr-0xC000] = value;
			else if(addr >= 0xFF00 && addr < 0xFF80) ioregs_[addr-0xFF00] = value;
			else if(addr >= 0xFF80 && addr < 0xFFFF) hram_[addr-0xFF80] = value;
			else {
				std::cout << "invalid addr@=0x" << std::hex << addr << std::endl;
			}
		}
	}

	bool Bus::load_bootrom(const std::string &path) {
		std::ifstream ifs(path, std::ios::binary);
		if(!ifs) return false;

		ifs.seekg(0, std::ios::end);
		std::streamsize size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		if(size != 0x100) return false; // Bootrom size should be exactly 256B
		if(!ifs.read(reinterpret_cast<char*>(bootrom_.data()),  0x100)) return false;

		bootrom_enabled = true;
		return true;
	}
}
