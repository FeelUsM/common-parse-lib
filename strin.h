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

	/*
	 * strin - интерактивный, т.е. для работы с пользователем через консоль
	 * это означает, что перед тем, как строка попадет в буфер, пользователь ее должен ввести
	 * а значит программа перейдет в режим ожидания этой строки
	 * 
	 * Для неинтерактивных файлов при создании потока создается итератор, 
	 * указывающий на первый символ, а значит загружается первый буфер
	 * Для strin'а это не допустимо, т.к. программист может захотеть что-то выполнить или вывести на экран до 
	 * того как программма перейдет в режим ожидания ввода строки
	 *
	 * По этому строковые файлы при получении первого буфера выдают фиктивную строку "\n"
	 * для ее прочтения существует функция start_read_line(it), 
	 * после выполнения которой программа переходит в режим ожиданияя ввода строки
	 * после чего можно произвести синт. анализ этой строки,
	 * не выводя при этом итераторы за пределы следующего символа '\n'
	 *
	 * поле анализа прграммист может захотеть опять что-то вывести на экран
	 * и если после этого ему опять понадобятся данные от пользователя
	 * то перед их получением снова следует вызвать start_read_line(strin)
	 * 
	 * т.о. получается цикл:
	 *	while(true){
	 *		cout << "приглашение" <<endl;
	 *		start_read_line(strin);
	 *		analys(strin);
	 *	}
	 */
	STRIN_EXTERN string_file_on_FILE 						FILEin(stdin);
	STRIN_EXTERN forward_stream<basic_adressed_buffer<char>> 		STREAMin(&FILEin);
	//интересно, чем следующие конструкции отличаются?
	#define	strin STREAMin.internal_iterator()
	//STRIN_EXTERN forward_stream<str_FILE_buffer>::iterator & strin = STREAMin.internal_iterator();
	int __set01(typename forward_stream<basic_adressed_buffer<char>>::iterator & it){
		set_linecol(it,linecol(0,1));
		return 0;
	}
	int __unused_int = __set01(strin);

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