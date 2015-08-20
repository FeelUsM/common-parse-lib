#include "strin.h"

namespace str{
	string_file_on_FILE 		FILEin(stdin);
	forward_stream<str_FILE_buffer> 			STREAMin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
#ifndef strin
	forward_stream<str_FILE_buffer>::iterator & strin = STREAMin.internal_iterator();
#endif
}