#include "gb/types.hpp"

struct Registers {
	unsigned char A;
	unsigned char B;
	unsigned char C;
	unsigned char D;
	unsigned char E;
	unsigned char F;
	unsigned char G;
	unsigned char H;
};

struct Flags {
	unsigned char Z;
	unsigned char N;
	unsigned char H;
	unsigned char C;
	bool HALT;
};

namespace gb {
	class CPU {
		public:
	};
}
