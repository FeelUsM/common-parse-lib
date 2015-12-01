#ifndef STRIN_H
#define STRIN_H

#include <exception>
#include "base_parse.h"
#include "forward_stream.h"

#ifdef ONE_SOURCE
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
#ifdef ONE_SOURCE
	(true, new file_on_FILE_stringbuf(stdin))
#endif
	;
	//интересно, чем следующие конструкции отличаются? - namespace-ом
	//#define	strin STREAMin.iter()
	STRIN_EXTERN forward_adressed_stream::iterator & strin = STREAMin.iter();
	inline int __set01(typename forward_adressed_stream::iterator & it){
		set_linecol(it,linecol(0,1));
		return 0;
	}
	STRIN_EXTERN int __unused_int 
#ifdef ONE_SOURCE
	= __set01(strin)
#endif
	;


class str_error{
#define constructor str_error
typedef str_error my_t;
protected:
	const char * message;
public:
	constructor():message(0){}
	constructor(const char * mes):message(mes){}
	constructor(int x):message(0){	if(x)throw "инициализация от ненулевого int";	}
	constructor(bool x)	:message(x?0:""){}
	operator bool()const	{	return message==0;	}
	const char * what()const {	return message;	}
#undef constructor
};
DECLARE_AND_OR_default_error_operators(str_error)
DECLARE_AND_OR_error_operators_for_short_circuiting(str_error)
str_error operator>>(base_parse_error from, str_error to){
	if(from) return str_error(true);
	else	return to;
}

class strpos_error : public str_error{
#define constructor strpos_error
typedef strpos_error my_t;
typedef str_error base_t;
	linecol lc;
	friend strpos_error operator&&(strpos_error l, strpos_error r);
	friend strpos_error operator||(strpos_error l, strpos_error r);
public:
	linecol ignored_lc=linecol(0,0);
	bool ignored()const{	return lc==ignored_lc;	}
	constructor(){};
	constructor(const char * mes):base_t(mes){}
	constructor(int x)	:base_t(x){}
	constructor(bool x)	:base_t(x){}
	
	constructor(const linecol & l)
		:lc(l){};
	constructor(const char * mes, const linecol & l)
		:base_t(mes),lc(l){};
	constructor(bool mes, const linecol & l)
		:base_t(mes),lc(l){};

	linecol where()const	
	{	return lc;	}
	my_t & set_what(const char * mes)
	{	base_t::message=mes;	return *this;	}
	template<class it_t>
	my_t & set_ignored(const it_t & it)
	{	ignored_lc =linecol(it);	return *this;	}
#undef constructor
};

template <class it_t>
strpos_error operator>>(str_error from, const it_t * to){
	return strpos_error(from.what(),linecol(*to));
}
std::ostream & operator<<(std::ostream & str, const strpos_error & e){
	return str<<"("<<"ignored:"<<e.ignored()<<" at "<<e.ignored_lc<<")"<<(bool)e<<";"<<(e?"OK":e.what())<<" at "<<e.where();
}
strpos_error operator&&(strpos_error l, strpos_error r){
	r.set_ignored(l.ignored_lc);
	if(l)	
		if(r)	
			if(r.ignored())
				return l;
			else
				return r;	/*TT*/
		else	
			return r;	/*TF*/
	else	
		if(r)	
			return l;	/*FT*/
		else	
			if(r.ignored())
				return l;
			else
				return r;	/*FF*/
}
strpos_error operator||(strpos_error l, strpos_error r){
	r.set_ignored(l.ignored_lc);
	//cerr <<"operator||( "<<l<<" , "<<r<<")"<<endl;
	if(l)	
		if(r)	
			if(r.ignored())
				return l;
			else
				return r;	/*TT*/
		else	
			return l;	/*TF*/
	else	
		if(r)	
			return r;	/*FT*/
		else	
			if(r.ignored())
				return l;
			else
				return r;	/*FF*/
}
DECLARE_AND_OR_error_operators_for_short_circuiting(strpos_error)

}//namespace str
#endif //STRIN_H