#include "strin.h"

namespace str{
	string_file_on_FILE 						FILEin(stdin);
	forward_stream<basic_adressed_buffer<char>> STREAMin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
#ifndef strin
	forward_stream<basic_adressed_buffer<char>>::iterator & strin = STREAMin.internal_iterator();
#endif
}