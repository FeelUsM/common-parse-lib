#include "strin.h"

namespace str{
	forward_adressed_stream STREAMin(true, new file_on_FILE_stringbuf(stdin));
	int __unused_int = __set01(strin);
#ifndef strin
	forward_adressed_stream::iterator & strin = STREAMin.iter();
#endif
}