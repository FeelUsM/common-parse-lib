#include "pin.h"

namespace str{
	string_file_on_FILE 		FILEin(stdin);
	forward_stream<str_FILE_buffer> 			STRin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
#ifndef pin
	forward_stream<str_FILE_buffer>::iterator & pin = STRin.pinternal_iterator();
#endif
}