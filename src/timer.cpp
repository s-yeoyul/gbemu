#include "gb/timer.hpp"

namespace gb {
	u8 Timer::read8(u16 addr) const {
		switch(addr) {
			case 0xFF04: return div_;
			case 0xFF05: return tima_;
			case 0xFF06: return tma_;
			case 0xFF07: return tac_;
			default: return 0;
		}
	}

	void Timer::write8(u16 addr, u8 value) {
		switch(addr) {
			case 0xFF04: div_ = 0;
									 break;
			case 0xFF05: tima_ = value;
									 break;
			case 0xFF06: tma_ = value;
									 break;
			case 0xFF07: tac_ = value;
									 break;
		}
	}

	bool Timer::tick(int cycles) {
		bool is_intr = false;

		// 1. DIV increment
		div_cycles += cycles;
		if(div_cycles >= 256) {
			div_++;
			div_cycles -= 256;
		}

		// 2. Check if timer is enabled
		if((tac_ & 0x04) == 0x04) {
			acc_cycles += cycles;

			// 3. If enabled, increment TIMA according to TAC
			u64 clock_cycle = 0;
			switch((tac_ & 0x03)) {
				case 0x00: clock_cycle = 1024;
									 break;
				case 0x01: clock_cycle = 16;
									 break;
				case 0x02: clock_cycle = 64;
									 break;
				case 0x03: clock_cycle = 256;
									 break;
			}
			while(acc_cycles >= clock_cycle) {
				tima_++;
				acc_cycles -= clock_cycle;

				if(tima_ == 0x00) {
					tima_ = tma_;
					is_intr = true;
				}
			}
		}

		return is_intr;

	}
} // namespace gb
