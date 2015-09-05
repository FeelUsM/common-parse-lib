#include "strin.h"

namespace str{
	forward_adressed_stream STREAMin(true, new string_file_on_FILE(stdin));
	int __unused_int = __set01(strin);
#ifndef strin
	forward_stream<adressed_buffer>::iterator & strin = STREAMin.iter();
#endif
}