#include "gb/types.hpp"
#include "gb/joypad.hpp"

namespace gb {
	u8 Joypad::read8(u16 addr) {
		u8 output = 0xF;

		if(sel_ == 0x10 || sel_ == 0x30) {
			if(button_.a) output &= ~0x1;
			if(button_.b) output &= ~0x2;
			if(button_.select) output &= ~0x4;
			if(button_.start) output &= ~0x8;
		} else if (sel_ == 0x20) {
			if(button_.right) output &= ~0x1;
			if(button_.left) output &= ~0x2;
			if(button_.up) output &= ~0x4;
			if(button_.down) output &= ~0x8;
		}
		return (output | 0xC0 | sel_);
	}

	void Joypad::write8(u16 addr, u8 value) {
		sel_ = (value & 0x30);
	}

	bool Joypad::tick() {
		if(pending_intr) {
			pending_intr = false;
			return true;
		}
		return false;
	}

	void Joypad::set_a(bool pressed) {
		if(!button_.a && pressed) {
			pending_intr = true;
			button_.a = true;
		}
		else if(!pressed) button_.a = false;
	}

	void Joypad::set_b(bool pressed) {
		if(!button_.b && pressed) {
			pending_intr = true;
			button_.b = true;
		}
		else if(!pressed) button_.b = false;
	}

	void Joypad::set_select(bool pressed) {
		if(!button_.select && pressed) {
			pending_intr = true;
			button_.select = true;
		}
		else if(!pressed) button_.select = false;
	}

	void Joypad::set_start(bool pressed) {
		if(!button_.start && pressed) {
			pending_intr = true;
			button_.start = true;
		}
		else if(!pressed) button_.start = false;
	}

	void Joypad::set_up(bool pressed) {
		if(!button_.up && pressed) {
			pending_intr = true;
			button_.up = true;
		}
		else if(!pressed) button_.up = false;
	}

	void Joypad::set_down(bool pressed) {
		if(!button_.down && pressed) {
			pending_intr = true;
			button_.down = true;
		}
		else if(!pressed) button_.down = false;
	}

	void Joypad::set_left(bool pressed) {
		if(!button_.left && pressed) {
			pending_intr = true;
			button_.left = true;
		}
		else if(!pressed) button_.left = false;
	}

	void Joypad::set_right(bool pressed) {
		if(!button_.down && pressed) {
			pending_intr = true;
			button_.right = true;
		}
		else if(!pressed) button_.right = false;
	}
} // namespace gb
