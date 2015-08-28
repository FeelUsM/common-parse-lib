#ifndef STRIN_H
#define STRIN_H

#include <exception>
#include "base_parse.h"
#include "forward_stream.h"

#ifdef one_source
#	define STRIN_EXTERN
#else
#	define STRIN_EXTERN extern
#endif

namespace str{

	STRIN_EXTERN string_file_on_FILE 						FILEin(stdin);
	typedef basic_adressed_buffer<char,string_file_on_FILE> addr_str_FILE_buffer;
	STRIN_EXTERN forward_stream<addr_str_FILE_buffer> 		STREAMin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
	#define	strin STREAMin.internal_iterator()
	//STRIN_EXTERN forward_stream<str_FILE_buffer>::iterator & strin = STREAMin.internal_iterator();
	int __set01(typename forward_stream<addr_str_FILE_buffer>::iterator & it){
		set_linecol(it,linecol(0,1));
		return 0;
	}
	int __unused_int = __set01(strin);

	struct parse_exception : public std::exception
	{
		string _what;
		parse_exception(){}
		parse_exception(string s):_what(s){}
		const char * what()	{	return _what.c_str();	}
	};

	/*
	 * считывает строку до перевода строки
	 * если конец файла встретился сразу - кидает исключение
	 * если конец файла не встретился - добавляет перевод строки в конец считанной строки
	 * в начале адаптируется к strin-у при помощи start_read_line()
	 */
	template<typename it_t>
	it_t & operator>>(it_t & it, string & s){
		start_read_line(it);
		s="";
		int err = read_line(it,&s);
		if(err==-1)
			throw parse_exception("неожиданный конец файла");
		if(err>=0)
			s+='\n';
		return it;
	}

	/*
	 * считывает фиксированную строку
	 * если не получилось - кидает исключение 
	 * в начале адаптируется к strin-у при помощи start_read_line()
	 */
	template<typename it_t>
	it_t & operator>>(it_t & it, const char * s){
		start_read_line(it);
		int err	= read_fix_str(it,s);
		if(err>0)	throw parse_exception(string("заданной строки '")+s+"' не оказалось");
		if(err<0)	throw parse_exception("неожиданный конец файла");
		return it;
	}

}//namespace str
#endif //STRIN_H