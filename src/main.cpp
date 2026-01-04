#include <iostream>

#include "gb/bus.hpp"
#include "gb/cpu.hpp"

int main() {
	std::cout << "hello world\n";
	gb::Bus bus;
	if(!bus.load_bootrom("../roms/gbboot.rom")) return 0;

//	gb::CPU cpu(bus);
//	cpu.reset();

	return 0;
}
