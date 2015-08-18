#ifndef PIN_H
#define PIN_H

#include <exception>
#include "base_parse.h"
#include "forward_stream.h"

namespace str{
	class parse_exception : public std::exception
	{
		string _what;
	public:
		parse_exception(){}
		parse_exception(string s):_what(s){}
		const char * what()	{	return _what.c_str();	}
	};

	/*
	 * считывает строку до перевода строки
	 * если конец файла встретился сразу - кидает исключение
	 * если конец файла не встретился - добавляет перевод строки в конец считанной строки
	 * в начале адаптируется к pin-у при помощи start_read_line()
	 *
	template<typename it_t>
	it_t * operator>>(it_t * pit, string & s){
		start_read_line(pit);
		s="";
		int err = read_line(pit,&s);
		if(err==-1)
			throw parse_exception("неожиданный конец файла");
		if(err>=0)
			s+='\n';
		return pit;
	}

	/*
	 * считывает фиксированную строку
	 * если не получилось - кидает исключение 
	 * в начале адаптируется к pin-у при помощи start_read_line()
	 *
	template<typename it_t>
	it_t * operator>>(it_t * pit, const char * s){
		start_read_line(pit);
		int err	= read_fix_str(pit,s);
		if(err>0)	throw parse_exception("заданной строки '"+s+"' не оказалось");
		if(err<0)	throw parse_exception("неожиданный конец файла");
		return pit;
	}
*/
#ifdef onesource
#	define PIN_EXTERN
#else
#	define PIN_EXTERN extern
#endif
	PIN_EXTERN string_file_on_FILE 		FILEin(stdin);
	typedef basic_simple_buffer<char,string_file_on_FILE> 	str_FILE_buffer;
	PIN_EXTERN forward_stream<str_FILE_buffer> 			STRin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
	#define	pin STRin.pinternal_iterator()
	//PIN_EXTERN forward_stream<str_FILE_buffer>::iterator & pin = STRin.pinternal_iterator();
}//namespace str
#endif //PIN_H