#pragma once

#include "gb/types.hpp"

namespace gb {
	class Timer {
		public:
			u8 read8(u16 addr) const;
			void write8(u16 addr, u8 value);
			bool tick(int cycles);
		private:
			u64 div_cycles = 0;
			u64 acc_cycles = 0;
			u8 div_ = 0;
			u8 tima_ = 0;
			u8 tma_ = 0;
			u8 tac_ = 0;
	};
} // namespace gb
