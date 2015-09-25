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
	 * По этому строковые файлы при заполнении первого буфера выдают фиктивную строку "\n"
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
	STRIN_EXTERN forward_adressed_stream STREAMin
#ifdef one_source
	(true, new file_on_FILE_string(stdin))
#endif
	;
	//интересно, чем следующие конструкции отличаются?
	#define	strin STREAMin.iter()
	//STRIN_EXTERN forward_stream<str_FILE_buffer>::iterator & strin = STREAMin.internal_iterator();
	inline int __set01(typename forward_stream<adressed_buffer>::iterator & it){
		set_linecol(it,linecol(0,1));
		return 0;
	}
	STRIN_EXTERN int __unused_int 
#ifdef one_source
	= __set01(strin)
#endif
	;


}//namespace str
#endif //STRIN_H