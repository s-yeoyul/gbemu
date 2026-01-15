#pragma once
#include "gb/types.hpp"

namespace gb {
	struct Button {
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;
		bool a = false;
		bool b = false;
		bool select = false;
		bool start = false;
	};
	
	class Joypad {
		public:
			u8 read8(u16 addr);
			void write8(u16 addr, u8 value);
			bool tick();

			void set_a(bool pressed);
			void set_b(bool pressed);
			void set_select(bool pressed);
			void set_start(bool pressed);
			void set_up(bool pressed);
			void set_down(bool pressed);
			void set_left(bool pressed);
			void set_right(bool pressed);
		private:
			u8 sel_ = 0x30; // 8'b0011_0000
			Button button_{};
			bool pending_intr = false;
	};
} // namespace gb
