//FeelUs
#ifndef BASE_PARSE_H
#define BASE_PARSE_H

/*
 * форматирование в данном файле:
 * после редактирования надо всю табуляцию заменить на пробелы а НАЧАЛЬНЫЕ пробелы заменить на табуляцию
 * это связано в основном с табличками
 * по этому в табличках первый символ должен быть не пробелом и не табуляцией, что бы описанное выше правило не портило табличку
 * т.е. получается что
 * отступы делаются табуляцией
 * а таблички - пробелами
 */

#include <iterator> //для iterator_traits<it_t>::value_type
#include <limits>   //для numeric_limits<int_t>::min() ::max()
#include <errno.h>  //для чтения чисел из си-строк
#include <stdlib.h> //для strtol, strtoll, strtoul, strtoull, strtof, strtod, strtold
#include <wchar.h>  //для wcstol, wcstoll, wcstoul, wcstoull, wcstof, wcstod, wcstold
#include <iostream> //для дебага
namespace str{//нечто среднее между string и stream
using std::numeric_limits;
//using std::cerr;//для дебага
//using std::endl;//для дебага

/* atend()
 * для всех итераторов или указателей по строкам
 * необходим "метод" atend(it)
 * это как тоже самое что и it!=container.end()
 * только здесь для этой операции нам не требуется contaier
 *
 * это сделано именно так потому, что 
 * container.end() вычисляется в начале чтения и предполагается, что положение конца файла заранее известно
 * в то время как с потоками идущими в реальном времени (такими как перенаправление из другой программы, по сети, с клавиатуры...)
 * конец файла может наступить совершенно внезапно
 * да и со строками такая же ситуация: их конец определяется символом а не указателем
 */
/* и перегрузка для указателей (на простые типы)
 * а для "пользовательских" итераторов каждый пользователь будет это перегружать в своем коде (как это сделано в stream_string.h)
 */
template<typename ch_t>
bool atend(ch_t * pc)
{   return !*pc;    }

template<class it_t>
using char_type = typename std::iterator_traits<it_t>::value_type;

//{DEF_STRING
/* использование этой фигни следующее:
 * DEF_STRING(name,"qwerty") - объявление (вне функции)
 * foo(name<wchar_t>())     - использование в случае void foo(const char *)
 * foo(name<wchar_t>().s)   - использование в случае template<typename ch_t> void foo(const ch_t *)
 * работает только для char, wchar_t, char16_t и char32_t
 */
#define DEF_STRING(name,str)\
template <typename ch_t>\
struct name{\
	const ch_t * s;\
	operator const ch_t *(){ return s; }\
};\
template <>\
struct name<char>{\
	const char * s = str;\
	operator const char *(){ return s; }\
};\
template <>\
struct name<wchar_t>{\
	const wchar_t * s = L##str;\
	operator const wchar_t *(){ return s; }\
};\
template <>\
struct name<char16_t>{\
	const char16_t * s = u##str;\
	operator const char16_t *(){ return s; }\
};\
template <>\
struct name<char32_t>{\
	const char32_t * s = U##str;\
	operator const char32_t *(){ return s; }\
};
//}

//{ === стандартные КЛАССЫ СИМВОЛОВ ===
/* см. спраку: http://www.cplusplus.com/reference/cctype/
 * его я реализую сам, а не использую из <ctype.h> или <wctype.h>, 
 * т.к. функции оттуда зависят от локали
 * но это не будет работать с ASCII несовместимой (т.е. по первым 128 символам) кодировкой
 */
template<typename ch_t> inline bool is_cntr(ch_t c)    {   return c>=0 && c<0x20 || c==0x7f;   }
template<typename ch_t> inline bool is_blank(ch_t c)   {   return c==' ' || c=='\t';   }
template<typename ch_t> inline bool is_space(ch_t c)   {   return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\f' || c=='\v';   }
template<typename ch_t> inline bool is_upper(ch_t c)   {   return c>='A' && c<='Z';    }
template<typename ch_t> inline bool is_lower(ch_t c)   {   return c>='a' && c<='z';    }
template<typename ch_t> inline bool is_alpha(ch_t c)   {   return c>='a' && c<='z' || c>='A' && c<='Z';    }
template<typename ch_t> inline bool is_digit(ch_t c)   {   return c>='0' && c<='9';    }
template<typename ch_t> inline bool is_xdigit(ch_t c)  {   return c>='0' && c<='9' || c>='a' && c<='f' || c>='A' && c<='F';    }
template<typename ch_t> inline bool is_alnum(ch_t c)   {   return is_alpha(c) || is_digit(c);    }
template<typename ch_t> inline bool is_punct(ch_t c)   {   return c>='!' && c<='/' || c>=':' && c<='@' || c>='[' && c<='`' || c>='{' && c<='~';    }
template<typename ch_t> inline bool is_graph(ch_t c)   {   return c>='!' && c<='~';    }
template<typename ch_t> inline bool is_print(ch_t c)   {   return c>=' ' && c<='~';    }

/*
 * когда в функции read_while_charclass и until_charclass попадает объект span
 * они вызывают функции стандартной библиотеки strspn, strcspn, wcsspn, wcscspn 
 * (по крайней мере так будет, когда функции будут специализированы)
 * т.е. span - строка набора символов
 * для консистентности read_charclass, read_charclass_s и read_charclass_c также могут принимать это объект
 */
template<typename ch_t>
class basic_span_string;
typedef basic_span_string<char> span_string;
typedef basic_span_string<wchar_t> wspan_string;
typedef basic_span_string<char16_t> u16span_string;
typedef basic_span_string<char32_t> u32span_string;

template<typename ch_t>
struct basic_span{
	const ch_t * s;
	basic_span(const ch_t * ss) :s(ss) {}
	//basic_span_string<ch_t> str(){	return s;	}
	//explicit basic_span(const basic_span_string<ch_t> & str) :s(str.c_str()) {}
};
typedef basic_span<char> 	span;
typedef basic_span<wchar_t> wspan;
typedef basic_span<char16_t> u16span;
typedef basic_span<char32_t> u32span;
template<typename ch_t> inline
basic_span<ch_t> make_span(const ch_t * ss){	return basic_span<ch_t>(ss);	}
//template<typename ch_t> inline
//basic_span<ch_t> make_span(const basic_span_string<ch_t> & str){	return basic_span<ch_t>(str);	}
/*
 * при больших множествах идущих друг за другом символов (например span("abcdefghijklnABCDEFGHIJK")) проверять, 
 * равен ли заданный символ одному из данного множества, неоптимально
 * оптимальнее задавать дипозоны парами символов (bispan("anAK\0"))
 * т.о. происходит проверка, что заданный символ x: bs[0]<=x && x<=bs[1] || bs[2]<=x && x<=bs[3] || bs[4]<=x && x<=bs[5] || ....
 * для задания отдельного символа - указываются одинаковые символы для нижней и верхней границы
 * символов в строке должно быть четное кол-во, если это не так, может произойти что угодно
 * поэтому на всякий случай лучше будет, если строка будет заканчиваться двумя нулевыми символами 
 * "sdfghj\0" - один ваш, один проставляется компилятором
 * также в связи с тем, что ch_t может быть как signed, так и unsigned
 * не желательно, что бы диапозоны пересекали точку 0 и точку изменения знака
 */
template<typename ch_t>
class basic_bispan_string;
typedef basic_bispan_string<char> bispan_string;
typedef basic_bispan_string<wchar_t> wbispan_string;
typedef basic_bispan_string<char16_t> u16bispan_string;
typedef basic_bispan_string<char32_t> u32bispan_string;

template<typename ch_t>
struct basic_bispan{
	const ch_t * s;
	basic_bispan(const ch_t * ss) :s(ss) {}
	//basic_bispan_string<ch_t> str(){	return s;	}
	//explicit basic_bispan(const basic_bispan_string<ch_t> & str) :s(str.c_str()) {}
};
typedef basic_bispan<char> 	bispan;
typedef basic_bispan<wchar_t> wbispan;
typedef basic_bispan<char16_t> u16bispan;
typedef basic_bispan<char32_t> u32bispan;
template<typename ch_t> inline
basic_bispan<ch_t> make_bispan(const ch_t * ss){	return basic_bispan<ch_t>(ss);	}
//template<typename ch_t> inline
//basic_bispan<ch_t> make_bispan(const basic_bispan_string<ch_t> & str){	return basic_bispan<ch_t>(str);	}

/*
 * использовать следующие функции так: 
 * read_charclass(it,make_spn_smth<тип>())
 * или read_charclass(it,spn_smth)
 */
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_cntr() ;
template<typename ch_t> inline basic_span <ch_t>  make_spn_blank();
template<typename ch_t> inline basic_span <ch_t>  make_spn_space();
template<typename ch_t> inline basic_bispan <ch_t>    make_spn_upper();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_lower();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_alpha();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_digit();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_xdigit();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_alnum();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_punct();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_graph();
template<typename ch_t> inline basic_bispan<ch_t>     make_spn_print();

#define DEFSPANS(ch_t,s_prefix,prefix)\
template<> inline basic_bispan<ch_t>  make_spn_cntr  <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"\1\x20\x7f\x7f\0");   }\
template<> inline basic_span  <ch_t>  make_spn_blank <ch_t>()   {   return basic_span  <ch_t>(s_prefix##" \t");   }\
template<> inline basic_span  <ch_t>  make_spn_space <ch_t>()   {   return basic_span  <ch_t>(s_prefix##" \t\n\r\f\v");   }\
template<> inline basic_bispan<ch_t>  make_spn_upper <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"AZ\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_lower <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"az\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_alpha <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"azAZ\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_digit <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"09\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_xdigit<ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"09afAF\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_alnum <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"azAZ09\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_punct <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"!/:@[`{~\0");    }\
template<> inline basic_bispan<ch_t>  make_spn_graph <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##"!~\0"); }\
template<> inline basic_bispan<ch_t>  make_spn_print <ch_t>()   {   return basic_bispan<ch_t>(s_prefix##" ~\0"); }\
prefix##bispan	prefix##spn_cntr 	= make_spn_cntr	  <ch_t>();\
prefix##span	prefix##spn_blabk 	= make_spn_blank  <ch_t>();\
prefix##span	prefix##spn_space 	= make_spn_space  <ch_t>();\
prefix##bispan	prefix##spn_upper 	= make_spn_upper  <ch_t>();\
prefix##bispan	prefix##spn_lower 	= make_spn_lower  <ch_t>();\
prefix##bispan	prefix##spn_alpha 	= make_spn_alpha  <ch_t>();\
prefix##bispan	prefix##spn_digit 	= make_spn_digit  <ch_t>();\
prefix##bispan	prefix##spn_xdigit 	= make_spn_xdigit <ch_t>();\
prefix##bispan	prefix##spn_alnum 	= make_spn_alnum  <ch_t>();\
prefix##bispan	prefix##spn_punct 	= make_spn_punct  <ch_t>();\
prefix##bispan	prefix##spn_graph 	= make_spn_graph  <ch_t>();\
prefix##bispan	prefix##spn_print 	= make_spn_print  <ch_t>();

DEFSPANS(char,,)
DEFSPANS(wchar_t,L,w)
DEFSPANS(char16_t,u,u16)
DEFSPANS(char32_t,U,u32)

#undef DEFSPANS

template<typename ch_t>
class basic_span_string{
	
};

//}

//{ ОШИБКИ
#define ifnot(expr)			if(!(expr))
#define RETURN_IFNOT(expr)	{	auto err=expr;	ifnot(err)return err;	}

//выражение передаем в другую функцию, что бы та его выполнила или не выполнила
#define E2F_cyrq(expr,...)			[__VA_ARGS__]	()					{return expr;}
#define E2F_it_cyrq(it,expr,...)	[__VA_ARGS__]	(decltype(it) & it)	{return expr;}
//наверно после оптимизации взятие всех переменных по ссылке в лямбда-функцию - будет нормально
#define E2F(expr)					[&]				()					{return expr;}
#define E2F_it(it,expr)				[&]				(decltype(it) & it)	{return expr;}

template<class it_t, class fun_t>
auto resin(it_t & it, const fun_t & fun) //restore if not
-> decltype(fun(it)){
	it_t tmp=it;
	auto err = fun(it);
	if(err)	return err;
	it = tmp;
	return err;
}
#define resin_CYRQ(it,expr,...)	resin(it,E2F_it_cyrq	(it,expr,__VA_ARGS__))
#define resin_E(it,expr)			resin(it,E2F_it		(it,expr))
#define resin_E2F(it,expr)			E2F(resin_E(it,expr))

//синтаксический сахар
#define OR(it,expr)	||resin_E2F(it,expr)
#define AND(expr)	&&E2F(expr)

//теже ваши операторы && и ||, только с short circuiting
#define DECLARE_AND_OR_error_operators_for_short_circuiting(type)\
template<class fun_t>\
auto operator&&(type l,const fun_t & fun)\
-> decltype(l&&fun()){\
	if(l)	return l&&fun();/*true && true - в принципе не игрет роли, но там могут "складываться" предупреждения*/\
	else	return l;\
}\
template<class fun_t>\
auto operator||(type l,const fun_t & fun)\
-> decltype(l||fun()){\
	if(l)	return l;\
	else	return l||fun();/*false && false - в принципе не игрет роли, но там могут "складываться" ошибки*/\
}
DECLARE_AND_OR_error_operators_for_short_circuiting(bool)

class base_parse_error{
#define constructor base_parse_error
typedef base_parse_error my_t;
	int _code;
public:
	//COPYING, DESTRUCTION - default
	constructor()=default;
	constructor(bool x):_code(x?0:-1){}
	constructor(int c):_code(c){}
	operator bool(){	return _code>=0;	}
		//сделать в любом случае true
	my_t & reset()	{	_code=0;	return *this;	}
		//+len -> +len-x - если требуется прочитать x или более символов
	my_t & dec(int x)	{	_code-=x;	return *this;	}
		//и len и -(1+len) -> len - если неважна причина завершения чтения
	int len()const	{	return _code>=0 ? _code : -_code-1;	}
		//просто получить код
	int code()const	{	return _code;	}
#undef constructor
};
#define DECLARE_AND_OR_default_error_operators_for(type)\
type operator&&(type l, type r){\
	if(l)	\
		if(r)	\
			return r;	/*TT*/\
		else	\
			return r;	/*TF*/\
	else	\
		if(r)	\
			return l;	/*FT*/\
		else	\
			return r;	/*FF*/\
}\
type operator||(type l, type r){\
	if(l)	\
		if(r)	\
			return r;	/*TT*/\
		else	\
			return l;	/*TF*/\
	else	\
		if(r)	\
			return r;	/*FT*/\
		else	\
			return r;	/*FF*/\
}
DECLARE_AND_OR_default_error_operators_for(base_parse_error)
DECLARE_AND_OR_error_operators_for_short_circuiting(base_parse_error)


//}

/* ТАБЛИЧКА
функции возвращают код ошибки в виде класса base_parse_error (далее bpe)
при успешном прочтении итератор указывает на следующий символ после последнего прочитанного
всё считанное ДОБАВЛЯЕТСЯ в строку, если таковая указана
при неудачном прочтении куда указывает итератор - зависит от функции

size_t      n
ch_t        c
ch_t *      s
func_obj    bool is(c)  //наподобие bool isspace(c)
span        spn  //см выше
bispan      bspn  //см выше
func_obj    err pf(it*) //наподобие int read_spc(it*)
func_obj    err pf(it*, rez*)

len - кол-во символов, добавлненных в *pstr

.                                                                                                   специализация для
.                                                                                                   [w]char char16/32   forward_stream
.                                                               возвращаемое значение в случае  реализованность 
название                    аргументы           рег.выр.        если EOF    если не EOF     статистика использования
'<-' в рег.выр. - говорит о том, что что после прочтения последнего символа итератор стоит не после него а на нем
//в сдучае неудачи итератор НЕ восстанавливают
bpe read_until_eof          (it&)               .*$             0           0               1   OK
bpe read_until_eof          (it&,    pstr*)     .*$             len         len                 OK
bpe read_fix_length         (it&, n)            .{n}            -1          0                   OK
bpe read_fix_length         (it&, n, pstr*)     .{n}            -(1+len)    0               2   OK
bpe read_fix_str_pos        (it&, s)            str             -1          0 или -2            OK
bpe read_fix_str_pos        (it&, s, pstr*)     str             -1          0 или -2            OK
//в сдучае неудачи итератор восстанавливают
bpe read_fix_str            (it&, s)            str             -1          0 или -2        7   OK
bpe read_fix_str            (it&, s, pstr*)     str             -1          0 или -2        2   OK
bpe read_fix_char           (it&, c)            c               -1          0 или -2        3   OK
bpe read_fix_char           (it&, c, pstr*)     c               -1          0 или -2        7   OK
bpe read_charclass          (it&, is)           [ ]             -1          0 или -2            OK
bpe read_charclass          (it&, spn)          [ ]             -1          0 или -2            OK
bpe read_charclass          (it&, bspn)         [ ]             -1          0 или -2            OK
bpe read_charclass_c        (it&, is, ch*)      [ ]             -1          0 или -2            OK
bpe read_charclass_c        (it&, spn, ch*)     [ ]             -1          0 или -2        1   OK
bpe read_charclass_c        (it&, bspn, ch*)    [ ]             -1          0 или -2            OK
bpe read_charclass_s        (it&, is, pstr*)    [ ]             -1          0 или -2            OK
bpe read_charclass_s        (it&, spn, pstr*)   [ ]             -1          0 или -2        1   OK
bpe read_charclass_s        (it&, bspn, pstr*)  [ ]             -1          0 или -2        5   OK
bpe read_c                  (it&, ch*)          .               -1          0               1   OK
bpe read_c                  (it&, ch*, pstr*)   .               -1          0               4   OK
//в сдучае неудачи итератор НЕ восстанавливают
bpe read_while_charclass    (it&, is)           [ ]*            -(1+len)    len                 OK
bpe read_while_charclass    (it&, spn)          [ ]*            -(1+len)    len             2   OK
bpe read_while_charclass    (it&, bspn)         [ ]*            -(1+len)    len                 OK
bpe read_while_charclass    (it&, is, pstr*)    [ ]*            -(1+len)    len                 OK
bpe read_while_charclass    (it&, spn, pstr*)   [ ]*            -(1+len)    len                 OK
bpe read_while_charclass    (it&, bspn, pstr*)  [ ]*            -(1+len)    len             1   OK
bpe read_until_charclass    (it&, is)           .*[ ]<-         -(1+len)    len                 OK
bpe read_until_charclass    (it&, spn)          .*[ ]<-         -(1+len)    len             1   OK
bpe read_until_charclass    (it&, bspn)         .*[ ]<-         -(1+len)    len                 OK
bpe read_until_charclass    (it&, is, pstr*)    .*[ ]<-         -(1+len)    len                 OK
bpe read_until_charclass    (it&, spn, pstr*)   .*[ ]<-         -(1+len)    len             2   OK
bpe read_until_charclass    (it&, bspn, pstr*)  .*[ ]<-         -(1+len)    len                 OK
bpe read_until_char         (it&, c)            .*c             -(1+len)    len                 OK
bpe read_until_char         (it&, c, pstr*)     .*c             -(1+len)    len                 OK

bpe read_until_str          (it&, s)            .*str           -(1+len)    len             2   OK
bpe read_until_str          (it&, s, pstr*)     .*str           -(1+len)    len             1   OK
bpe read_until_pattern      (it&, pf)           .*( )           -(1+len)    len                 OK
bpe read_until_pattern      (it&, pf, rez*)     .*( )           -(1+len)    len                 OK
bpe read_until_pattern_s    (it&, pf, pstr*)    .*( )           -(1+len)    len                 OK
bpe read_until_pattern_s    (it&, pf, pstr*, rez*)  .*( )       -(1+len)    len                 OK
*/
/* ? интересно, а что эффективней:
 * проходиться по потоку за 1 проход, добавляя в строку символы
 * или за 1й проход найти точку, сделать reserve() а за 2й проход добавлять символы в строку (по одному)
 * - это вопрос не для специализации а для алгоритмов для произвольных итераторов
 * понятно, что при специалиации будет так:
 * за 1й проход найти точку, и за 2й несколько раз делаем append(a,b)
 */
/* ? интересно, а можно ли компилятору как-нибудь задать опцию оптимизации,
 * что бы он при вызове x++ и если результат не используется, использовал вызов ++x
 * или это на другом уровне происходит оптимизация
 */
 /* todo:
  * сделать варианты read_while и read_until функций, берущих указатель на итератор и лимит записанных символов
  * и придумать к ним названия (вернее суффикс _n)
  * просто указатель на итератор (или insert_iterator) - не нужно 
  * - для этого сделать string_appender (и его возвращающий appender())
  * это то же что и insert_iterator или back_insert_iterator, (и их возвращающие inserter() и back_inserter())
  * принципиальное отличие в том, что string_appender позволяет добавлять и символы и си-строки
  * и имеет метод append(), позволяющий добавлять буфера
  */
//{======================= until_eof, fix_length, fix_str, fix_str_pos, fix_char, charclass, c
	/*
	 * until_eof(it_t & it)
	 * передвигает указатель в конец файла
	 *
	 * всегда возвращает 0
	 */
	template<typename it_t> inline
	base_parse_error 
	read_until_eof(it_t & it){
		while(!atend(it))
			it++;
		return 0;
	}

	/*
	 * until_eof(it_t & it, str_t * pstr)
	 * считывает все до конца файла
	 * 
	 * возвращает размер считанного
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - у ошибки вызывайте .dec(1)
	 */
	template<typename it_t, typename str_t> inline 
	base_parse_error 
	read_until_eof(it_t & it, str_t * pstr){
		int i=0;
		while(!atend(it)){
			*pstr += *it++;
			i++;
		}
		return i;
	}

	/*
	 * fix_length(it_t & it, size_t n)
	 * передвигает итератор на n позиций
	 *
	 * если встретился конец файла (кроме последней позиции, когда сдвигаться уже не надо)
	 * возвращает -1
	 * и итератор указывает на конец файла
	 */
	template<typename it_t> inline
	base_parse_error 
	read_fix_length(it_t & it, size_t n){
		for(;n>0;n--,it++)
			if(atend(it))
				return -1;
		return 0;
	}
	
	/*
	 * fix_length(it_t & it, size_t n, str_t * pstr)
	 * считывает строку длиной n
	 *
	 * если встретился конец файла (кроме последней позиции, когда сдвигаться уже не надо)
	 * в строку помещается все до конца файла
	 * возвращает -(1 + размер считанного)
	 * и итератор указывает на конец файла
	 *
	 * если не встретился конец файла, возвращает 0
	 */
	template<typename it_t, typename str_t> inline 
	base_parse_error 
	read_fix_length(it_t & it, size_t n, str_t * pstr){
		for(size_t i=0; i<n; i++, it++)
			if(atend(it))
				return (int)-(1+i);
			else
				*pstr += *it;
		return 0;
	}
	
	/*
	 * fix_str_pos(it_t & it, const ch_t * s)
	 * строка, указываемая s, должна являться началом строки, указываемой it,
	 * и если это так, итератор сдвигается на длину этой строки
	 * 
	 * если встречается конец файла - возвращает -1
	 * если с какого-то символа строки начинают различаться - возвращает -2
	 *
	 * и итератор указывает на конец файла или на различающиеся символы
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_fix_str_pos(it_t & it, const ch_t * s){
		int i=0;
		while(!atend(it))
			if(!*s)
				return 0;
			else if(*it!=*s)
				return -2;
			else
				it++, s++, i++;
		if(!*s) return 0;
		return -1;
	}
	
	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_fix_str_pos(it_t & it, const ch_t * s, str_t * pstr)
	{
		RETURN_IFNOT(read_fix_str(it, s))
		*pstr+=s;
		return 0;
	}

	/*
	 * fix_str(it_t & it, const ch_t * s)
	 * то же что и fix_str_pos(), только
	 *
	 * в случае неудачи итератор восстанавливает
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_fix_str(it_t & it, const ch_t * s)
	{
		return resin_E(it,read_fix_str_pos(it, s));
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_fix_str(it_t & it, const ch_t * s, str_t * pstr)
	{
		return resin_E(it,read_fix_str_pos(it, s,pstr));
	}

	/*
	 * fix_char(it_t & it, ch_t c)
	 * если указываемый итератором символ совпадает с c, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если указываемый итератором символ не совпадает с c, то возвращает -2, и итератор не двигает
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_fix_char(it_t & it, ch_t c){
		if(atend(it)) return -1; 
		if(*it!=c)    return -2;
		it++;
		return 0;
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_fix_char(it_t & it, ch_t c, str_t * pstr)
	{
		RETURN_IFNOT(read_fix_char(it, c))
		*pstr+=c;
		return 0;
	}
	
	/*
	 * charclass(it_t & it, const class_t & is)
	 * если is(.) возвращает true от указываемого итератором символа, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает -2, и итератор не двигает
	 */
	template<typename it_t, typename class_t> inline
	base_parse_error 
	read_charclass(it_t & it, const class_t & is){
		if(atend(it)) return -1;
		if(!is(*it))  return -2;
		it++;
		return 0;
	}

	template<typename it_t> inline
	base_parse_error 
	read_charclass(it_t & it, basic_span<char_type<it_t>> s){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it==*s.s++){
				it++;
				return 0;
			}
		return -2;
	}
		
	template<typename it_t> inline
	base_parse_error 
	read_charclass(it_t & it, basic_bispan<char_type<it_t>> s){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				it++;
				return 0;
			}
		return -2;
	}
		
	/*
	 * charclass_s(it_t & it, const class_t & is, pstr_t * pstr)
	 * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает -2, и символ не считывает
	 */
	template<typename it_t, typename class_t, typename str_t> inline
	base_parse_error 
	read_charclass_s(it_t & it, const class_t & is, str_t * pstr){
		if(atend(it)) return -1;
		if(!is(*it))  return -2;
		*pstr += *it++;
		return 0;
	}

	template<typename it_t, typename str_t> inline
	base_parse_error 
	read_charclass_s(it_t & it, basic_span<char_type<it_t>> s, str_t * pstr){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it == *s.s++){
				*pstr += *it++;
				return 0;
			}
		return -2;
	}
		
	template<typename it_t, typename str_t> inline
	base_parse_error 
	read_charclass_s(it_t & it, basic_bispan<char_type<it_t>> s, str_t * pstr){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				*pstr += *it++;
				return 0;
			}
		return -2;
	}
		
	/*
	 * charclass_c(it_t & it, const class_t & is, ch_t * ch)
	 * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
	 */
	template<typename it_t, typename class_t, typename ch_t> inline
	base_parse_error 
	read_charclass_c(it_t & it, const class_t & is, ch_t * pch){
		if(atend(it)) return -1;
		if(!is(*it))  return -2;
		*pch = *it++;
		return 0;
	}
	
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_charclass_c(it_t & it, basic_span<char_type<it_t>> s, ch_t * pch){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it == *s.s++){
				*pch = *it++;
				return 0;
			}
		return -2;
	}
		
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_charclass_c(it_t & it, basic_bispan<char_type<it_t>> s, ch_t * pch){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				*pch = *it++;
				return 0;
			}
		return -2;
	}

	/*
	 * read_c(it_t & it, ch_t * c)
	 * считывает 1 символ
	 * 
	 * если встретился конец файла, возвращает -1
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_c(it_t & it, ch_t * c){
		if(atend(it))   return -1;
		*c = *it++;
		return 0;
	}
	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_c(it_t & it, ch_t * pc, str_t * pstr){
		RETURN_IFNOT(read_c(it,pc));
		*pstr += *pc;
		return 0;
	}
//}
//{======================= until_char, until_charclass, while_charclass
	/*
	 * until_char(it_t & it, ch_t ch)
	 * сдвигает указатель до тех пор, пока (не встретится заданный символ) или конец файла
	 * until_charclass(it_t & it, const class_t & is)
	 * сдвигает указатель до тех пор, пока (is(.) возвращает false) или не встретился конец файла
	 * while_charclass(it_t & it, const class_t & is)
	 * сдвигает указатель до тех пор, пока (is(.) возвращает true) или не встретился конец файла
	 *
	 * если встретился конец файла - возвращает -(1 + размер считанного)
	 * и итератор указывает на конец файла
	 * если встретился заданный символ - возвращает размер считанного
	 * и итератор указывает на заданный символ
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_until_char(it_t & it, ch_t ch){
		int i=0;
		while(!atend(it))
			if(*it==ch){
				it++;
				return i;
			}
			else{
				it++;
				i++;
			}
		return -(1+i);
	}

	template<typename it_t, typename class_t> inline
	base_parse_error 
	read_until_charclass(it_t & it, const class_t & is){
		int i=0;
		while(!atend(it))
			if(is(*it))
				return i;
			else{
				it++;
				i++;
			}
		return -(1+i);
	}
	
			template<typename it_t> inline
			base_parse_error 
			read_until_charclass(it_t & it, basic_span<char_type<it_t>> s){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*it==*ss++)
							return i;
					it++;
					i++;
				}
				return -(1+i);
			}

			template<typename it_t> inline
			base_parse_error 
			read_until_charclass(it_t & it, basic_bispan<char_type<it_t>> s){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++)
							return i;
					it++;
					i++;
				}
				return -(1+i);
			}

	template<typename it_t, typename class_t> inline
	base_parse_error 
	read_while_charclass(it_t & it, const class_t & is){
		int i=0;
		while(!atend(it))
			if(!is(*it))
				return i;
			else{
				it++;
				i++;
			}
		return -(1+i);
	}
	
			template<typename it_t> inline
			base_parse_error 
			read_while_charclass(it_t & it, basic_span<char_type<it_t>> s){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*it==*ss++){
							it++;
							i++;
							goto met;
						}
					return i;
				met:;
				}
				return -(1+i);
			}

			template<typename it_t> inline
			base_parse_error 
			read_while_charclass(it_t & it, basic_bispan<char_type<it_t>> s){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++){
							it++;
							i++;
							goto met;
						}
					return i;
				met:;
				}
				return -(1+i);
			}

	/*
	 * until_char(it_t & it, ch_t ch, str_t * pstr)
	 * считывает строку, до тех пор пока не встретится заданный символ или конец файла
	 * until_charclass(it_t & it, const class_t & is, str_t * pstr)
	 * считывает строку, пока is(.) возвращает false от указываемого итератором символа или не встретился конец файла
	 * while_charclass(it_t & it, const class_t & is, str_t * pstr)
	 * считывает строку, пока is(.) возвращает true от указываемого итератором символа или не встретился конец файла
	 *
	 * если встретился конец файла - возвращает -(1+число помещенных в строку символов)
	 * и итератор указывает на конец файла
	 * если встретился заданный символ - возвращает + число помещенных в строку символов
	 * и итератор указывает на заданный символ
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер считанного
	 */
	template<typename it_t, typename ch_t, typename str_t> inline
	base_parse_error 
	read_until_char(it_t & it, ch_t ch, str_t * pstr){
		int i=0;
		while(!atend(it))
			if(*it==ch){
				it++;
				return i;
			}
			else{
				*pstr += *it++;
				i++;
			}
		return -(1+i);
	}
	
	template<typename it_t, typename class_t, typename str_t> inline
	base_parse_error 
	read_until_charclass(it_t & it, const class_t & is, str_t * pstr){
		int i=0;
		while(!atend(it))
			if(is(*it))
				return i;
			else{
				*pstr += *it++;
				i++;
			}
		return -(1+i);
	}

			template<typename it_t, typename str_t> inline
			base_parse_error 
			read_until_charclass(it_t & it, basic_span<char_type<it_t>> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*it==*ss++)
							return i;
					*pstr += *it++;
					i++;
				}
				return -(1+i);
			}

			template<typename it_t, typename str_t> inline
			base_parse_error 
			read_until_charclass(it_t & it, basic_bispan<char_type<it_t>> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++)
							return i;
					*pstr += *it++;
					i++;
				}
				return -(1+i);
			}

	template<typename it_t, typename class_t, typename str_t> inline
	base_parse_error 
	read_while_charclass(it_t & it, const class_t & is, str_t * pstr){
		int i=0;
		while(!atend(it))
			if(!is(*it))
				return i;
			else{
				*pstr += *it++;
				i++;
			}
		return -(1+i);
	}

			template<typename it_t, typename str_t> inline
			base_parse_error 
			read_while_charclass(it_t & it, basic_span<char_type<it_t>> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*it==*ss++){
							*pstr += *it++;
							i++;
							goto met;
						}
					return i;
				met:;
				}
				return -(1+i);
			}

			template<typename it_t, typename str_t> inline
			base_parse_error 
			read_while_charclass(it_t & it, basic_bispan<char_type<it_t>> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const char_type<it_t> * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++){
							*pstr += *it++;
							i++;
							goto met;
						}
					return i;
				met:;
				}
				return -(1+i);
			}
//}
//{======================= until_str, until_pattern
	/*
	 * until_str(it_t & it, const ch_t * s[, str_t * pstr])
	 * считывает символы в строку pstr до тех пор, пока с очередного не будет начинаться строка s
	 * итератор указывает на следующий символ после конца найденной строки 
	 * или на конец файла минус размер строки, но не левее начала
	 *
	 * если встретился конец файла - возвращает -(1+число помещенных в строку символов)
	 * если встретился заданная строка - возвращает + число помещенных в строку символов
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_until_str(it_t & it, const ch_t * s){
		int i=0;
		for(;;){
			it_t lit = it;
			base_parse_error err=read_fix_str(lit,s);
			if((err).code()>=-1){//если прочитал фиксированную строку или встретил конец файла
				it = lit;
				return err ? i : -(1+i);
			}
			it++;
			i++;
		}
	}

	template<typename it_t, typename ch_t, typename str_t> inline
	base_parse_error 
	read_until_str(it_t & it, const ch_t * s, str_t * pstr){
		int i=0;
		for(;;){
			it_t lit = it;
			base_parse_error err;
			if((err=read_fix_str(lit,s)).code()>=-1){//если прочитал фиксированную строку или встретил конец файла
				it = lit;
				return err ? i : -(1+i);
			}
			*pstr += *it++;
			i++;
		}
	}

	/*
	 * until_pattern(it_t & it, const pattern_t & pattern[, str_t * pstr[, rez_t * rez]])
	 * считывает символы в строку pstr до тех пор, пока с очередного не будет начинаться последовательноть
	 *     удовлетворяющая (возвращает 0) функциональному объекту pattern(it_t * it[, rez_t * rez])
	 *     или он не окажется концом файла
	 * итератор указывает на следующий символ после конца найденной последовательности или на конец файла
	 *
	 * если встретился конец файла - возвращает -(1+число помещенных в строку символов)
	 * если встретился заданный символ - возвращает + число помещенных в строку символов
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename pattern_t> inline
	base_parse_error 
	read_until_pattern(it_t & it, const pattern_t & read_pattern){
		int i=0;
		for(;!atend(it);it++, i++){
			it_t lit = it;
			if(read_pattern(lit)){
				it = lit;
				return i;
			}
		}
		return -(1+i);
	}
	
	template<typename it_t, typename pattern_t, typename str_t> inline
	base_parse_error 
	read_until_pattern_s(it_t & it, const pattern_t & read_pattern, str_t * pstr){
		int i=0;
		for(;!atend(it);it++, i++){
			it_t lit = it;
			if(read_pattern(lit)){
				it = lit;
				return i;
			}
			*pstr = *it;
		}
		return -(1+i);
	}

	template<typename it_t, typename pattern_t, typename rez_t> inline
	base_parse_error 
	read_until_pattern(it_t & it, const pattern_t & read_pattern, rez_t * rez){
		int i=0;
		for(; !atend(it); it++, i++){
			it_t lit = it;
			if(read_pattern(lit,rez)){
				it = lit;
				return i;
			}
		}
		return -(1+i);
	}

	template<typename it_t, typename pattern_t, typename str_t, typename rez_t> inline
	base_parse_error 
	read_until_pattern_s(it_t & it, const pattern_t & read_pattern, str_t * pstr, rez_t * rez){
		int i=0;
		for(; !atend(it); it++, i++){
			it_t lit = it;
			if(read_pattern(lit,rez)){
				it = lit;
				return i;
			}
			*pstr = *it;
		}
		return -(1+i);
	}

//}
/* ТАБЛИЧКА
ch_t        c
ch_t *      s
func_obj    bool is(c)  //наподобие bool isspace(c)
span        spn  //см выше
bispan      bspn  //см выше
func_obj    err pf(it*) //наподобие int read_spc(it*)
func_obj    err pf(it*, rez*)
.                                                                                                   специализация для
len - кол-во символов, добавлненных в *pstr                                                         [w]char char16/32   stream_string
.                                                               возвращаемое значение в случае  реализованность 
название                    аргументы           рег.выр.        если EOF    если не EOF     статистика использования
bpe read_line               (it&, s)            .*[\r\n]<-      -1 или len  len                 OK
bpe read_line               (it&)               .*[\r\n]<-      -1 или len  len             1   OK
bpe start_read_line         (it&)               .*(\n|\r\n?)    -1          0 или 1         8   OK
<- - говорит о том, что что после прочтения последнего символа итератор стоит не после него а на нем

bpe read_spc                (it&)               [:space:]       -1          0 или -2            OK
bpe read_spcs               (it&)               [:space:]*      -(1+len)    len             5   OK
bpe read_s_fix_str          (it&, s)            [:space:]*str   -1          0 или -2        1   OK
bpe read_s_fix_str          (it&, s, pstr*)     [:space:]*str   -1          0 или -2            OK
bpe read_s_fix_char         (it&, c)            [:space:]*c     -1          0 или -2        8   OK
bpe read_s_fix_char         (it&, c, pstr*)     [:space:]*c     -1          0 или -2            OK
bpe read_s_charclass        (it&, is)           [:space:][ ]    -1          0 или -2            OK
bpe read_s_charclass_s      (it&, is, pstr*)    [:space:][ ]    -1          0 или -2            OK
bpe read_s_charclass_c      (it&, is, pc*)      [:space:][ ]    -1          0 или -2        2   OK
bpe read_blank              (it&)               [:blank:]       -1          0 или -2            OK
bpe read_blanks             (it&)               [:blank:]*      -(1+len)    len             1   OK
bpe read_bl_fix_str         (it&, s)            [:blank:]*str   -1          0 или -2            OK
bpe read_bl_fix_str         (it&, s, pstr*)     [:blank:]*str   -1          0 или -2            OK
bpe read_bl_fix_char        (it&, c)            [:blank:]*c     -1          0 или -2            OK
bpe read_bl_fix_char        (it&, c, pstr*)     [:blank:]*c     -1          0 или -2            OK
bpe read_bl_charclass       (it&, is)           [:blank:][ ]    -1          0 или -2            OK
bpe read_bl_charclass_s     (it&, is, pstr*)    [:blank:][ ]    -1          0 или -2            OK
bpe read_bl_charclass_c     (it&, is, pc*)      [:blank:][ ]    -1          0 или -2            OK
*/
//{======================= line, start_read_line, spc, spcs, s_fix_str, s_fix_char, bln, blns, b_fix_str, b_fix_char
	DEF_STRING(CRLF,"\r\n")
	/*
	 * line(it_t & it, str_t * ps)
	 * line(it_t & it)
	 * считывает все до перевода строки (в каком бы формате он ни был (CR,LF,CRLF))
	 * если не встретился конец файла - в строку добавляет '\n' (не зависимо от формата конца строки)
	 * если встретился конец файла в самом начале - возвращает -1
	 * иначе возвращает размер прочтанного
	 */
	template<typename it_t, typename str_t> inline
	base_parse_error 
	read_line(it_t & it, str_t * ps){
		typedef char_type<it_t> ch_t;
		base_parse_error err;
		if(err=read_until_charclass(it,basic_span<ch_t>(CRLF<ch_t>().s),ps)){
			*ps +=(ch_t)'\n';
			return err.len()+1;
		}
		else
			return err.len()==-1 ? -1 : err.len();
	}

	template<typename it_t> inline
	base_parse_error 
	read_line(it_t & it){
		typedef char_type<it_t> ch_t;
		base_parse_error err;
		if(err=read_until_charclass(it,basic_span<ch_t>(CRLF<ch_t>().s)))
			return err.len()+1;
		else
			return err.len()==-1 ? -1 : err.len();
	}

	/*
	 * start_read_line(it_t & it)
	 * передвигает итератор в начало следующей строки
	 * 
	 */
	template<typename it_t> inline
	base_parse_error 
	start_read_line(it_t & it){
		typedef char_type<it_t> ch_t;
		read_line(it);
		ch_t c;
		ifnot(read_charclass_c(it,basic_span<ch_t>(CRLF<ch_t>().s),&c))
			return -1;
		if(c==(ch_t)'\n')
			return 0;
		read_fix_char(it,(ch_t)'\n');
		return 0;
	}

	template<typename it_t> inline
	base_parse_error 
	read_spc(it_t & it){
		return read_charclass(it,make_spn_space<char_type<it_t>>());
	}
	
	template<typename it_t> inline
	base_parse_error 
	read_spcs(it_t & it){
		return read_while_charclass(it,make_spn_space<char_type<it_t>>());
	}

	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_s_fix_str(it_t & it, const ch_t * s){
		read_spcs(it);
		return read_fix_str(it,s);
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_s_fix_str(it_t & it, const ch_t * s, str_t * pstr){
		read_spcs(it);
		return read_fix_str(it,s,pstr);
	}

	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_s_fix_char(it_t & it, const ch_t ch){
		read_spcs(it);
		return read_fix_char(it,ch);
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_s_fix_char(it_t & it, const ch_t ch, str_t * pstr){
		read_spcs(it);
		return read_fix_char(it,ch,pstr);
	}

	template<typename it_t, typename class_t> inline
	base_parse_error 
	read_s_charclass(it_t & it, const class_t & cl){
		read_spcs(it);
		return read_charclass(it,cl);
	}

	template<typename it_t, typename class_t, typename str_t> inline
	base_parse_error 
	read_s_charclass_s(it_t & it, const class_t & cl, str_t * ps){
		read_spcs(it);
		return read_charclass_s(it,cl,ps);
	}

	template<typename it_t, typename class_t, typename ch_t> inline
	base_parse_error 
	read_s_charclass_c(it_t & it, const class_t & cl, ch_t * pc){
		read_spcs(it);
		return read_charclass_c(it,cl,pc);
	}

	template<typename it_t> inline
	base_parse_error 
	read_blank(it_t & it){
		return read_charclass(it,make_spn_blank<char_type<it_t>>());
	}
	
	template<typename it_t> inline
	base_parse_error 
	read_blanks(it_t & it){
		return read_while_charclass(it,make_spn_blank<char_type<it_t>>());
	}

	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_bl_fix_str(it_t & it, const ch_t * s){
		read_blanks(it);
		return read_fix_str(it,s);
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_bl_fix_str(it_t & it, const ch_t * s, str_t * pstr){
		read_blanks(it);
		return read_fix_str(it,s,pstr);
	}

	template<typename it_t, typename ch_t> inline
	base_parse_error 
	read_bl_fix_char(it_t & it, const ch_t ch){
		read_blanks(it);
		return read_fix_char(it,ch);
	}

	template<typename it_t, typename ch_t, class str_t> inline
	base_parse_error 
	read_bl_fix_char(it_t & it, const ch_t ch, str_t * pstr){
		read_blanks(it);
		return read_fix_char(it,ch,pstr);
	}

	template<typename it_t, typename class_t> inline
	base_parse_error
	read_bl_charclass(it_t & it, const class_t & cl){
		read_blanks(it);
		return read_charclass(it,cl);
	}

	template<typename it_t, typename class_t, typename str_t> inline
	base_parse_error
	read_bl_charclass_s(it_t & it, const class_t & cl, str_t * ps){
		read_blanks(it);
		return read_charclass_s(it,cl,ps);
	}

	template<typename it_t, typename class_t, typename ch_t> inline
	base_parse_error 
	read_bl_charclass_c(it_t & it, const class_t & cl, ch_t * pc){
		read_blanks(it);
		return read_charclass_c(it,cl,pc);
	}

//}
/* ТАБЛИЧКА
int_t может быть : long, long long, unsigned long, unsigned long long - для специализаций
[:digit:]   ::= [0-"$(($ss-1))"]
sign        ::= ('+'|'-')
bpe         ::= spcs[sign]spcs[:digit:]+
.                                                                                                                   специализация для
.                                                                                                                   [w]char char16/32   stream_string     
.                                                                           возвращаемое значение в случае  реализованность     
название                аргументы               рег.выр.                    неудача переполнение    статистика использования
bpe read_digit          (it&, int ss, int_t*)   [:digit:]                   -2       -1(EOF)        1       OK
bpe read_uint           (it&, int ss, int_t*)   [:digit:]+                  -2       -1             1       OK
bpe read_sign_uint      (it&, int ss, int_t*)   [sign][:digit:]+            -2       -1                     OK
bpe read_sign_s_uint    (it&, int ss, int_t*)   [sign]spcs[:digit:]+        -2       -1             1       OK
bpe read_int            (it&, int ss, int_t*)   spcs[sign]spcs[:digit:]+    -2       -1             1       OK      OK
bpe read_dec            (it&, int_t*)           int#[:digit:]=[0-9]         -2       -1             1       OK      OK
bpe read_hex            (it&, int_t*)           int#[:digit:]=[:xdigit:]    -2       -1                     OK      OK
bpe read_oct            (it&, int_t*)           int#[:digit:]=[0-7]         -2       -1                     OK      OK
bpe read_bin            (it&, int_t*)           int#[:digit:]=[01]          -2       -1                     OK      OK
 */
//{======================= digit, uint, sign_uint, sign_s_uint, base_parse_error, dec, hex, oct, bin, 
	/*
	 * считывает 1 цифру в заданной системе счисления (СС) в ASCII-совместимой кодировке
	 * CC - от 2 до 35(буква Z)
	 * если конец файла - возвращает -1
	 * если символ не попадает в диапозон для заданной СС - возвращает -2
	 * если неправильная СС - возвращает -3
	 */
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_digit(it_t & it, int ss, int_t * prez){
		typedef char_type<it_t> ch_t;
		if(atend(it)) 
			return -1;
		if(2<=ss && ss<=10){
			if( (ch_t)'0'<=*it && *it< (ch_t)'0'+ss){
				*prez = *it++ -(ch_t)'0';
				return 0;
			}
			else
				return -2;
		}
		else if(ss<=35){
			if( (ch_t)'0'<=*it && *it<=(ch_t)'9'){
				*prez = *it++ -(ch_t)'0';
				return 0;
			}
			else if((ch_t)'a'<=*it && *it<=(ch_t)'a'+ss-11){
				*prez = *it++ -(ch_t)'a'+10;
				return 0;
			}
			else if((ch_t)'A'<=*it && *it<=(ch_t)'A'+ss-11 ){
				*prez = *it++ -(ch_t)'A'+10;
				return 0;
			}
			else
				return -2;
		}
		else
			//throw "неправильная система счисления";
			return -3;
	}

	/*
	 * считывает число без знака
	 * если переполнение (а это определяется numeric_limits<int_t>::max()) - возвращает -1
	 * если не удалось прочитать ни одной цифры - возвращает -2
	 */
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_uint(it_t & it, int ss, int_t * prez){
		int_t premax = (numeric_limits<int_t>::max()-ss+1)/ss;
		ifnot(read_digit(it,ss,prez))
			return -2;
		while(*prez<=premax){
			int_t tmp;
			ifnot(read_digit(it,ss,&tmp))
				return 0;
			*prez *= ss;
			*prez += tmp;
		}
		return -1;
	}

	//sign - опционально
	//лимиты определяются не numeric_limits<int_t>:: min()..max(), а -max()..max()
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_sign_uint(it_t & it, int ss, int_t * prez){
		if(atend(it))
			return -1;
		if(*it=='-'){
			it++;
			base_parse_error err;
			if(err=read_uint(it,ss,prez)){
				*prez = -*prez;
				return 0;
			}
			else
				return err;
		}
		if(*it=='+')
			it++;
		return read_uint(it,ss,prez);
	}

	//sign - опционально
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_sign_s_uint(it_t & it, int ss, int_t * prez){
		if(atend(it)){
			return -1;
		}
		if(*it=='-'){
			it++;
			read_spcs(it);
			base_parse_error err;
			if(err=read_uint(it,ss,prez)){
				*prez = -*prez;
				return 0;
			}
			else
				return err;
		}
		if(*it=='+'){
			it++;
			read_spcs(it);
		}
		return read_uint(it,ss,prez);
	}

	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_int(it_t & it, int ss, int_t * prez){
		read_spcs(it);
		return read_sign_s_uint(it,ss,prez);
	}

#if 1 //read_int для char и wchar_t
// http://www.cplusplus.com/reference/cstdlib/strtol/
#define def_read_int(ch_t, int_t, func) \
	inline \
	base_parse_error\
	read_int(const ch_t *& ps, int ss, int_t * prez)\
	{\
		errno=0;\
		const ch_t * s = ps;\
		int_t tmp = func(s,const_cast<ch_t**>(&s),ss);\
		if(ps==s)  return -2;\
		ps=s; \
		if(errno)   return -1;\
		*prez = tmp;\
		return 0;\
	}
//todo: добавить условную компиляцию: проверять совпадает ли int и long
	def_read_int(char,      int,               strtol)
	def_read_int(char,      unsigned int,      strtoul)
	def_read_int(wchar_t,   int,               wcstol)
	def_read_int(wchar_t,   unsigned int,      wcstoul)

	def_read_int(char,      long,               strtol)
	def_read_int(char,      unsigned long,      strtoul)
	def_read_int(char,      long long,          strtoll)
	def_read_int(char,      unsigned long long, strtoull)
	def_read_int(wchar_t,   long,               wcstol)
	def_read_int(wchar_t,   unsigned long,      wcstoul)
	def_read_int(wchar_t,   long long,          wcstoll)
	def_read_int(wchar_t,   unsigned long long, wcstoull)
#undef def_read_int
#endif

	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_dec(it_t & it, int_t * prez){
		return read_int(it,10,prez);
	}
	
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_hex(it_t & it, int_t * prez){
		return read_int(it,16,prez);
	}
	
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_oct(it_t & it, int_t * prez){
		return read_int(it,8,prez);
	}
	
	template<typename it_t, typename int_t> inline
	base_parse_error 
	read_bin(it_t & it, int_t * prez){
		return read_int(it,2,prez);
	}
//}
/* todo придумать рег.выр-я
flt_t может быть : float, double, long double
bpe read_cfloat         (it*, flt_t*)
bpe read_ifloat         (it*, flt_t*)
bpe read_com_cfloat     (it*, flt_t*)
bpe read_com_ifloat     (it*, flt_t*)
bpe read_loc_cfloat     (it*, flt_t*)
bpe read_loc_ifloat     (it*, flt_t*)
*/
//{======================= float
	//при чтении точка обязательна, если нет экспоненты
	template<typename it_t, typename flt_t> inline
	base_parse_error 
	read_cfloat(it_t & it, flt_t * prez);
	
	//при чтении обязательна НЕ ТОЧКА А ЗАПЯТАЯ, если нет экспоненты
	template<typename it_t, typename flt_t> inline
	base_parse_error 
	read_com_cfloat(it_t & it, flt_t * prez);
	
	//int сойдет за float
	template<typename it_t, typename flt_t> inline
	base_parse_error 
	read_ifloat(it_t & it, flt_t * prez);
	
	//int сойдет за float
	template<typename it_t, typename flt_t> inline
	base_parse_error 
	read_com_ifloat(it_t & it, flt_t * prez);
//}
/* я тут обратил внимание, что ВСЕ алгоритмы, кроме read_fix_str, read_until_str и read_until_pattern
 * требуют не forward, а input итераторы, которые требуют от потока только ungetc()
 * и следовательно, можно специализровать эти алгоритмы специально для input итерторов
 * что бы они осуществляли собственную буферизацию, 
 * что впрочем может быть полезно только для удачных исходов выполнения этих алгоритмов
 */
//{======================= input_fix_str, input_until_str
struct parse_exception : public std::exception
{
	std::string _what;
	parse_exception(){}
	parse_exception(std::string s):_what(s){}
	virtual const char * what()const noexcept   {   return _what.c_str();   }
};

	template<typename it_t, typename ch_t> inline
	base_parse_error
	input_fix_str(it_t & it, const ch_t * s){
		if(!*s) return 0;
		ifnot(read_fix_char(*s++))
			return -1;
		while(!atend(it))
			if(!*s)
				return 0;
			else if(*it!=*s){
				throw parse_exception("exception in input_fix_str");
			}
			else
				it++, s++;
		if(!*s) return 0;
		throw parse_exception("exception in input_fix_str");
	}

	template<typename it_t, typename ch_t> inline
	base_parse_error
	input_until_str(it_t & it, const ch_t * s);
//}
}//namespace str
#endif //BASE_PARSE_H
