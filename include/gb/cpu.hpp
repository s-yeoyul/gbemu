#include "gb/types.hpp"

namespace gb {
	class Bus;

	struct Registers {
		u8 a{}, b{}, c{}, d{}, e{}, f{}, h{}, l{};
		u16 pc{}, sp{};
	};

	struct Flags {
		bool z{}, n{}, h{}, c{};
	};

	class CPU {
		public:
			explicit CPU(Bus& bus);
			void reset();
			int step();
      void isr_vec(u8 intr_num, u16 vec);
      int isr_handler();
		private:
			Bus& bus_;
			Registers regs;
			Flags flags;
			bool halted_ = false;
			bool ime_ = false;
	};
}
