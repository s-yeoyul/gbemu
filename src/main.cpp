#include <iostream>

#include "gb/bus.hpp"
#include "gb/cpu.hpp"

#include <filesystem>

int main() {
	gb::Bus bus;
	if(!bus.load_bootrom("roms/bootix_dmg.bin")) {
		std::cout << "load failed\n";
		return 0;
	}

	gb::CPU cpu(bus);
	cpu.reset();
	while(1) {
		int cycles = cpu.step();
		if(cycles == 0) break;
	}
	return 0;
}
