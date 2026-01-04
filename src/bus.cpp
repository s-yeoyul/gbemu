#include "gb/bus.hpp"

#include <fstream>

namespace gb {
	u8 Bus::read8(u16 addr) const {
		if(bootrom_enabled) return bootrom_[addr];
		else {
			if(addr >= 0xC000 && addr < 0xE000) return wram_[addr-0xC000];
			else if(addr >= 0xFF80 && addr < 0xFFFF) return hram_[addr-0xFF80];
			else {
				//std::cout << "invalid addr\n";
				return 0xFF; // return -1
			}
		}
	}
	void Bus::write8(u16 addr, u8 value) {
		if(bootrom_enabled) {
			bootrom_[addr] = value;
		}
		else {
			if(addr >= 0xC000 && addr < 0xE000) wram_[addr] = value;
			else if(addr >= 0xFF80 && addr < 0xFFFF) hram_[addr] = value;
			else {
				//std::cout << "invalid addr\n";
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
