//FeelUs
#ifndef BASE_PARSE_H
#define BASE_PARSE_H

/*
 * я тут обратил внимание, что ВСЕ алгоритмы, кроме read_fix_str, read_until_str и read_until_pattern
 * требуют не forward, а input итераторы, которые требуют от потока только ungetc()
 * и следовательно, можно специализровать эти алгоритмы специально для input итерторов
 * что бы они осуществляли собственную буферизацию, 
 * что впрочем может быть полезно только для удачных исходов выполнения этих алгоритмов
 */

/*
 * форматирование в данном файле:
 * после редактирования надо всю табуляцию заменить на пробелы а НАЧАЛЬНЫЕ пробелы заменить на табуляцию
 * это связано в основном с табличкой
 * по этому в табличке первый символ должен быть не пробелом и не табуляцией, что бы описанное выше правило не портило табличку
 * т.е. получается что
 * отступы делаются табуляцией
 * а таблички - пробелами
 */

#include <iterator> //для iterator_traits<it_t>::value_type
#include <limits>   //для numeric_limits<int_t>::min() ::max()
#include <errno.h>  //для чтения чисел из си-строк
#include <stdlib.h> //для strtol, strtoll, strtoul, strtoull, strtof, strtod, strtold
#include <wchar.h>  //для wcstol, wcstoll, wcstoul, wcstoull, wcstof, wcstod, wcstold
//#include <iostream>	//для дебага
namespace str{//нечто среднее между string и stream
using std::iterator_traits;  
using std::numeric_limits;

/*
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
/*
 * и перегрузка для указателей (на простые типы)
 * а для "пользовательских" итераторов каждый пользователь будет это перегружать в своем коде (как это сделано в stream_string.h)
 */
template<typename ch_t>
bool atend(ch_t * pc)
{   return !*pc;    }

/*
 * это для того, что бы  не писать типа
 * if(!(errcode=read_smpt(...)))
 * а писать r_if(errcode=read_smth(...))
 * по моему так удобней
 */
#define r_if(expr)        if((expr)==0)
#define r_while(expr)  while((expr)==0)
#define rm_if(expr)       if((expr)>=0)     //типа рег. выр.  '.*' * - multiple -> m
#define rm_while(expr) while((expr)>=0)
#define rp_if(expr)       if((expr)>0)      //типа рег. выр.  '.+' + - plus -> p
#define rp_while(expr) while((expr)>0)
#define re_if(expr)       if((expr)<=0)     //типа прочитать заданное выражение или обнаружить конец файла (end)
#define re_while(expr) while((expr)<=0)
#define r_ifnot(expr)      if(expr)
#define r_whilenot(expr) while(expr)

/*
 * использование этой фигни следующее:
 * DEF_STRING(name,"qwerty") - объявление (вне функции)
 * foo(name<wchar_t>())		- использование в случае void foo(const char *)
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

/* === стандартные КЛАССЫ СИМВОЛОВ ===
 * его я реализую сам, а не использую из <ctype.h> или <wctype.h>, 
 * т.к. функции оттуда зависят от локали
 * см. спраку: http://www.cplusplus.com/reference/cctype/
 * но это не будет работать с ASCII несовместимой (т.е. по первым 128 символам) кодировкой
 */
//{ is_smth(c)
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
//}

/*
 * когда в функции read_while_charclass и until_charclass попадает объект span
 * они вызывают функции стандартной библиотеки strspn, strcspn, wcsspn, wcscspn
 * т.е. span - строка набора символов
 * для консистентности read_charclass, read_charclass_s и read_charclass_c также могут принимать это объект
 */
template<typename ch_t>
struct span{
	const ch_t * s;
	span(const ch_t * ss) :s(ss) {}
};
/*
 * при больших множествах идущих друг за другом символов (например span("abcdefghijklnABCDEFGHIJK")) проверять, 
 * равен ли заданный символ одному из данного множества, неоптимально
 * оптимальнее задавать дипозоны парами символов (bispan("anAK\0"))
 * т.о. происходит проверка, что заданный символ x: bs[0]<=x && x<=bs[1] || bs[2]<=x && x<=bs[3] || bs[4]<=x && x<=bs[5] || ....
 * для задания отдельного символа - указываются одинаковые символы для нижней и верхней границы
 * символов в строке должно быть четное кол-во, если это не так может произойти что угодно
 * поэтому на всякий случай лучше будет, если строка будет заканчиваться двумя нулевыми символами 
 * "sdfghj\0" - один ваш, один проставляется компилятором
 * также в связи с тем, что ch_t может быть как signed, так и unsigned
 * не желательно, что бы диапозоны пересекали точку 0 и точку изменения знака
 */
template<typename ch_t>
struct bispan{
	const ch_t * s;
	bispan(const ch_t * ss) :s(ss) {}
};

//{ spn_smth
template<typename ch_t> inline bispan<ch_t>     spn_cntr() ;
template<typename ch_t> inline span <ch_t>  spn_blank();
template<typename ch_t> inline span <ch_t>  spn_space();
template<typename ch_t> inline bispan <ch_t>    spn_upper();
template<typename ch_t> inline bispan<ch_t>     spn_lower();
template<typename ch_t> inline bispan<ch_t>     spn_alpha();
template<typename ch_t> inline bispan<ch_t>     spn_digit();
template<typename ch_t> inline bispan<ch_t>     spn_xdigit();
template<typename ch_t> inline bispan<ch_t>     spn_alnum();
template<typename ch_t> inline bispan<ch_t>     spn_punct();
template<typename ch_t> inline bispan<ch_t>     spn_graph();
template<typename ch_t> inline bispan<ch_t>     spn_print();

/*
 * использовать следующие функции так: 
 * read_charclass(it,spn_smth<тип>())
 * не забывайте скобочки после spn_smth<тип>
 */

#define DEFSPANS(ch_t,prefix)\
template<> inline bispan<ch_t>  spn_cntr  <ch_t>()   {   return bispan<ch_t>(prefix##"\1\x20\x7f\x7f\0");   }\
template<> inline span  <ch_t>  spn_blank <ch_t>()   {   return span  <ch_t>(prefix##" \t");   }\
template<> inline span  <ch_t>  spn_space <ch_t>()   {   return span  <ch_t>(prefix##" \t\n\r\f\v");   }\
template<> inline bispan<ch_t>  spn_upper <ch_t>()   {   return bispan<ch_t>(prefix##"AZ\0");    }\
template<> inline bispan<ch_t>  spn_lower <ch_t>()   {   return bispan<ch_t>(prefix##"az\0");    }\
template<> inline bispan<ch_t>  spn_alpha <ch_t>()   {   return bispan<ch_t>(prefix##"azAZ\0");    }\
template<> inline bispan<ch_t>  spn_digit <ch_t>()   {   return bispan<ch_t>(prefix##"09\0");    }\
template<> inline bispan<ch_t>  spn_xdigit<ch_t>()   {   return bispan<ch_t>(prefix##"09afAF\0");    }\
template<> inline bispan<ch_t>  spn_alnum <ch_t>()   {   return bispan<ch_t>(prefix##"azAZ09\0");    }\
template<> inline bispan<ch_t>  spn_punct <ch_t>()   {   return bispan<ch_t>(prefix##"!/:@[`{~\0");    }\
template<> inline bispan<ch_t>  spn_graph <ch_t>()   {   return bispan<ch_t>(prefix##"!~\0"); }\
template<> inline bispan<ch_t>  spn_print <ch_t>()   {   return bispan<ch_t>(prefix##" ~\0"); }

DEFSPANS(char,)
DEFSPANS(wchar_t,L)
DEFSPANS(char16_t,u)
DEFSPANS(char32_t,U)

#undef DEFSPANS
//}

/* ТАБЛИЧКА
функции возвращают код ошибки
при успешном прочтении итератор указывает на следующий символ после последнего прочитанного
всё считанное ДОБАВЛЯЕТСЯ в строку, если таковая указана
при неудачном прочтении итератор указывает на место ошибки

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
.                                                                                                   [w]char char16/32   stream_string
.                                                               возвращаемое значение в случае  реализованность 
название                    аргументы           рег.выр.        если EOF    если не EOF     статистика использования
int read_until_eof          (it&)               .*$             0           0               1   OK
int read_until_eof          (it&,    pstr*)     .*$             len         len                 OK
int read_fix_length         (it&, n)            .{n}            -1          0                   OK
int read_fix_length         (it&, n, pstr*)     .{n}            -(1+len)    0                   OK

int read_fix_str            (it&, s)            str             -(1+len)    0 или len       1   OK
int read_fix_char           (it&, c)            c               -1          0 или 1         8   OK
int read_charclass          (it&, is)           [ ]             -1          0 или 1             OK
int read_charclass          (it&, spn)          [ ]             -1          0 или 1             OK
int read_charclass          (it&, bspn)         [ ]             -1          0 или 1             OK
int read_charclass_s        (it&, is, pstr*)    [ ]             -1          0 или 1             OK
int read_charclass_s        (it&, spn, pstr*)   [ ]             -1          0 или 1             OK
int read_charclass_s        (it&, bspn, pstr*)  [ ]             -1          0 или 1         1   OK
int read_charclass_c        (it&, is, ch*)      [ ]             -1          0 или 1             OK
int read_charclass_c        (it&, spn, ch*)     [ ]             -1          0 или 1             OK
int read_charclass_c        (it&, bspn, ch*)    [ ]             -1          0 или 1             OK
int read_c					(it&, ch*)			.				-1			0					OK

int read_while_charclass    (it&, is)           [ ]*            -(1+len)    len                 OK
int read_while_charclass    (it&, spn)          [ ]*            -(1+len)    len                 OK
int read_while_charclass    (it&, bspn)         [ ]*            -(1+len)    len                 OK
int read_while_charclass    (it&, is, pstr*)    [ ]*            -(1+len)    len                 OK
int read_while_charclass    (it&, spn, pstr*)   [ ]*            -(1+len)    len                 OK
int read_while_charclass    (it&, bspn, pstr*)  [ ]*            -(1+len)    len             1   OK
int read_until_charclass    (it&, is)           .*[ ]<-         -(1+len)    len                 OK
int read_until_charclass    (it&, spn)          .*[ ]<-         -(1+len)    len                 OK
int read_until_charclass    (it&, bspn)         .*[ ]<-         -(1+len)    len                 OK
int read_until_charclass    (it&, is, pstr*)    .*[ ]<-         -(1+len)    len                 OK
int read_until_charclass    (it&, spn, pstr*)   .*[ ]<-         -(1+len)    len                 OK
int read_until_charclass    (it&, bspn, pstr*)  .*[ ]<-         -(1+len)    len                 OK
int read_until_char         (it&, c)            .*c             -(1+len)    len                 OK
int read_until_char         (it&, c, pstr*)     .*c             -(1+len)    len                 OK
<- - говорит о том, что что после прочтения последнего символа итератор стоит не после него а на нем

int read_until_str          (it&, s)            .*str           -(1+len)    len                 OK
int read_until_str          (it&, s, pstr*)     .*str           -(1+len)    len                 OK
int read_until_pattern      (it&, pf)           .*( )           -(1+len)    len                 OK
int read_until_pattern      (it&, pf, rez*)     .*( )           -(1+len)    len                 OK
int read_until_pattern_s    (it&, pf, pstr*)    .*( )           -(1+len)    len                 
int read_until_pattern_s    (it&, pf, pstr*, rez*)  .*( )       -(1+len)    len                 
чтение целых и плавающих чисел - в отдельных табличках
*/
//{======================= until_eof, fix_length, fix_str, fix_char, charclass, c
	/*
	 * until_eof(it_t & it)
	 * передвигает указатель в конец файла
	 *
	 * всегда возвращает 0
	 */
	template<typename it_t> inline
	int 
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
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер считанного при помощи rp_if()
	 */
	template<typename it_t, typename str_t> inline 
	int 
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
	int 
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
	 * и итератор указывает на конец файла
	 * в строку помещается все до конца файла
	 * возвращает -(1 + размер считанного)
	 *
	 * если не встретился конец файла, возвращает 0
	 */
	template<typename it_t, typename str_t> inline 
	int 
	read_fix_length(it_t & it, size_t n, str_t * pstr){
		for(size_t i=0; i<n; i++, it++)
			if(atend(it))
				return -(1+i);
			else
				*pstr += *it;
		return 0;
	}
	
	/*
	 * fix_str(it_t & it, const ch_t * s)
	 * строка, указываемая s, должна являться началом строки, указываемой it,
	 * и если это так, итератор сдвигается на длину этой строки
	 * 
	 * если встречается конец файла
	 * возвращает -(1+количество_совпадающих_символов)
	 * если с какого-то символа строки начинают различаться
	 * возвращает количество_совпадающих_символов
	 *
	 * и итератор не сдвигается
	 */
	template<typename it_t, typename ch_t> inline
	int 
	read_fix_str(it_t & it, const ch_t * s){
		int i=0;
		it_t tmp = it;
		while(!atend(it))
			if(!*s)
				return 0;
			else if(*it!=*s){
				it = tmp;
				return i;
			}
			else
				it++, s++, i++;
		if(!*s) return 0;
		it = tmp;
		return -(1+i);
	}
	
	/*
	 * fix_char(it_t & it, ch_t c)
	 * если указываемый итератором символ совпадает с c, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если указываемый итератором символ не совпадает с c, то возвращает 1, и итератор не двигает
	 */
	template<typename it_t, typename ch_t> inline
	int 
	read_fix_char(it_t & it, ch_t c){
		if(atend(it)) return -1;
		if(*it!=c)    return 1;
		it++;
		return 0;
	}

	/*
	 * charclass(it_t & it, const class_t & is)
	 * если is(.) возвращает true от указываемого итератором символа, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и итератор не двигает
	 */
	template<typename it_t, typename class_t> inline
	int 
	read_charclass(it_t & it, const class_t & is){
		if(atend(it)) return -1;
		if(!is(*it))  return 1;
		it++;
		return 0;
	}

	template<typename it_t> inline
	int 
	read_charclass(it_t & it, span<typename iterator_traits<it_t>::value_type> s){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it==*s.s++){
				it++;
				return 0;
			}
		return 1;
	}
		
	template<typename it_t> inline
	int 
	read_charclass(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				it++;
				return 0;
			}
		return 1;
	}
		
	/*
	 * charclass_s(it_t & it, const class_t & is, pstr_t * pstr)
	 * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
	 */
	template<typename it_t, typename class_t, typename str_t> inline
	int 
	read_charclass_s(it_t & it, const class_t & is, str_t * pstr){
		if(atend(it)) return -1;
		if(!is(*it))  return 1;
		*pstr += *it++;
		return 0;
	}

	template<typename it_t, typename str_t> inline
	int 
	read_charclass_s(it_t & it, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it == *s.s++){
				*pstr += *it++;
				return 0;
			}
		return 1;
	}
		
	template<typename it_t, typename str_t> inline
	int 
	read_charclass_s(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				*pstr += *it++;
				return 0;
			}
		return 1;
	}
		
	/*
	 * charclass_c(it_t & it, const class_t & is, ch_t * ch)
	 * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
	 *
	 * если итератор в состоянии atend(), возвращает -1
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
	 */
	template<typename it_t, typename class_t, typename ch_t> inline
	int 
	read_charclass_c(it_t & it, const class_t & is, ch_t * pch){
		if(atend(it)) return -1;
		if(!is(*it))  return 1;
		*pch = *it++;
		return 0;
	}
	
	template<typename it_t, typename ch_t> inline
	int 
	read_charclass_c(it_t & it, span<typename iterator_traits<it_t>::value_type> s, ch_t * pch){
		if(atend(it)) return -1;
		while(*s.s)
			if(*it == *s.s++){
				*pch = *it++;
				return 0;
			}
		return 1;
	}
		
	template<typename it_t, typename ch_t> inline
	int 
	read_charclass_c(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s, ch_t * pch){
		if(atend(it)) return -1;
		while(*s.s)
			if(*s.s++<=*it && *it<=*s.s++){
				*pch = *it++;
				return 0;
			}
		return 1;
	}

	/*
	 * read_c(it_t & it, ch_t * c)
	 * считывает 1 символ
	 * 
	 * если встретился конец файла, возвращает -1
	 */
	template<typename it_t, typename ch_t> inline
	int 
	read_c(it_t & it, ch_t * c){
		if(atend(c))	return -1;
		it++;
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
	int 
	read_until_char(it_t & it, ch_t ch){
		int i=0;
		while(!atend(it))
			if(*it==ch)
				return i;
			else{
				it++;
				i++;
			}
		return -(1+i);
	}

	template<typename it_t, typename class_t> inline
	int 
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
			int 
			read_until_charclass(it_t & it, span<typename iterator_traits<it_t>::value_type> s){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
					while(*ss)
						if(*it==*ss++)
							return i;
					it++;
					i++;
				}
				return -(1+i);
			}

			template<typename it_t> inline
			int 
			read_until_charclass(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++)
							return i;
					it++;
					i++;
				}
				return -(1+i);
			}

	template<typename it_t, typename class_t> inline
	int 
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
			int 
			read_while_charclass(it_t & it, span<typename iterator_traits<it_t>::value_type> s){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
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
			int 
			read_while_charclass(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
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
	int 
	read_until_char(it_t & it, ch_t ch, str_t * pstr){
		int i=0;
		while(!atend(it))
			if(*it==ch)
				return i;
			else{
				*pstr += *it++;
				i++;
			}
		return -(1+i);
	}
	
	template<typename it_t, typename class_t, typename str_t> inline
	int 
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
			int 
			read_until_charclass(it_t & it, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
					while(*ss)
						if(*it==*ss++)
							return i;
					*pstr += *it++;
					i++;
				}
				return -(1+i);
			}

			template<typename it_t, typename str_t> inline
			int 
			read_until_charclass(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
					while(*ss)
						if(*ss++<=*it && *it<=*ss++)
							return i;
					*pstr += *it++;
					i++;
				}
				return -(1+i);
			}

	template<typename it_t, typename class_t, typename str_t> inline
	int 
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
			int 
			read_while_charclass(it_t & it, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
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
			int 
			read_while_charclass(it_t & it, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
				int i=0;
				while(!atend(it)){
					const typename iterator_traits<it_t>::value_type * ss=s.s;
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
	int 
	read_until_str(it_t & it, const ch_t * s){
		int i=0;
		for(;;){
			it_t lit = it;
			int err;
			re_if(err=read_fix_str(lit,s)){//если прочитал фиксированную строку или встретил конец файла
				it = lit;
				return err<0 ? i : -(1+i);
			}
			it++;
			i++;
		}
	}

	template<typename it_t, typename ch_t, typename str_t> inline
	int 
	read_until_str(it_t & it, const ch_t * s, str_t * pstr){
		int i=0;
		for(;;){
			it_t lit = it;
			int err;
			re_if(err=read_fix_str(lit,s)){//если прочитал фиксированную строку или встретил конец файла
				it = lit;
				return err>0 ? i : -(1+i);
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
	int 
	read_until_pattern(it_t & it, const pattern_t & read_pattern){
		int i=0;
		for(;!atend(it);it++, i++){
			it_t lit = it;
			r_if(read_pattern(lit)){
				it = lit;
				return i;
			}
		}
		return -(1+i);
	}
	
	template<typename it_t, typename pattern_t, typename str_t> inline
	int 
	read_until_pattern_s(it_t & it, const pattern_t & read_pattern, str_t * pstr){
		int i=0;
		for(;!atend(it);it++, i++){
			it_t lit = it;
			r_if(read_pattern(lit)){
				it = lit;
				return i;
			}
			*pstr = *it;
		}
		return -(1+i);
	}

	template<typename it_t, typename pattern_t, typename rez_t> inline
	int 
	read_until_pattern(it_t & it, const pattern_t & read_pattern, rez_t * rez){
		int i=0;
		for(; !atend(it); it++, i++){
			it_t lit = it;
			r_if(read_pattern(lit,rez)){
				it = lit;
				return i;
			}
		}
		return -(1+i);
	}

	template<typename it_t, typename pattern_t, typename str_t, typename rez_t> inline
	int 
	read_until_pattern_s(it_t & it, const pattern_t & read_pattern, str_t * pstr, rez_t * rez){
		int i=0;
		for(; !atend(it); it++, i++){
			it_t lit = it;
			r_if(read_pattern(lit,rez)){
				it = lit;
				return i;
			}
			*pstr = *it;
		}
		return -(1+i);
	}

//}
//{======================= spc, spcs, s_fix_str, s_fix_char, bln, blns, b_fix_str, b_fix_char, line, start_read_line
/*
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
int read_line               (it&, s)            .*\n            -(1+len)    len                 OK
int start_read_line         (it&)               \n              -1          0 или 1         8   OK

int read_spc                (it&)               [:space:]       -(1+len)    len                 OK
int read_spcs               (it&)               [:space:]*      -(1+len)    len             4   OK
int read_s_fix_str          (it&, s)            [:space:]*str   -(1+len)    0 или len       1   OK
int read_s_fix_char         (it&, c)            [:space:]*c     -1          0 или 1         8   OK
int read_s_charclass        (it&, is)           [:space:][ ]    -1          0 или 1             OK
int read_s_charclass_s      (it&, is, pstr*)    [:space:][ ]    -1          0 или 1             OK
int read_s_charclass_c      (it&, is, pc*)      [:space:][ ]    -1          0 или 1             OK
int read_bln                (it&)               [:blank:]       -(1+len)    len                 OK
int read_blns               (it&)               [:blank:]*      -(1+len)    len             4   OK
int read_b_fix_str          (it&, s)            [:blank:]*str   -(1+len)    0 или len       1   OK
int read_b_fix_char         (it&, c)            [:blank:]*c     -1          0 или 1         8   OK
int read_b_charclass        (it&, is)           [:space:][ ]    -1          0 или 1             OK
int read_b_charclass_s      (it&, is, pstr*)    [:space:][ ]    -1          0 или 1             OK
int read_b_charclass_c      (it&, is, pc*)      [:space:][ ]    -1          0 или 1             OK
*/
	template<typename it_t, typename str_t> inline
	int 
	read_line(it_t & it, str_t * ps){
		typedef typename std::iterator_traits<it_t>::value_type ch_t;
		return read_until_char(it,(ch_t)'\n',ps);
	}

	template<typename it_t> inline
	int 
	start_read_line(it_t & it){
		typedef typename std::iterator_traits<it_t>::value_type ch_t;
		return read_fix_char(it,(ch_t)'\n');
	}

	template<typename it_t> inline
	int 
	read_spc(it_t & it){
		return read_charclass(it,spn_space<typename iterator_traits<it_t>::value_type>());
	}
	
	template<typename it_t> inline
	int 
	read_spcs(it_t & it){
		return read_while_charclass(it,spn_space<typename iterator_traits<it_t>::value_type>());
	}

	template<typename it_t, typename ch_t> inline
	int 
	read_s_fix_str(it_t & it, const ch_t * s){
		read_spcs(it);
		return read_fix_str(it,s);
	}

	template<typename it_t, typename ch_t> inline
	int 
	read_s_fix_char(it_t & it, const ch_t ch){
		read_spcs(it);
		return read_fix_char(it,ch);
	}

	template<typename it_t, typename class_t> inline
	int 
	read_s_charclass(it_t & it, const class_t & cl){
		read_spcs(it);
		return read_charclass(it,cl);
	}

	template<typename it_t, typename class_t, typename str_t> inline
	int 
	read_s_charclass_s(it_t & it, const class_t & cl, str_t * ps){
		read_spcs(it);
		return read_charclass_s(it,cl,ps);
	}

	template<typename it_t, typename class_t, typename ch_t> inline
	int 
	read_s_charclass_c(it_t & it, const class_t & cl, ch_t * pc){
		read_spcs(it);
		return read_charclass_c(it,cl,pc);
	}

	template<typename it_t> inline
	int 
	read_bln(it_t & it){
		return read_charclass(it,spn_blank<typename iterator_traits<it_t>::value_type>());
	}
	
	template<typename it_t> inline
	int 
	read_blns(it_t & it){
		return read_while_charclass(it,spn_blank<typename iterator_traits<it_t>::value_type>());
	}

	template<typename it_t, typename ch_t> inline
	int 
	read_b_fix_str(it_t & it, const ch_t * s){
		read_blns(it);
		return read_fix_str(it,s);
	}

	template<typename it_t, typename ch_t> inline
	int 
	read_b_fix_char(it_t & it, const ch_t ch){
		read_blns(it);
		return read_fix_char(it,ch);
	}

	template<typename it_t, typename class_t> inline
	int
	read_b_charclass(it_t & it, const class_t & cl){
		read_blns(it);
		return read_charclass(it,cl);
	}

	template<typename it_t, typename class_t, typename str_t> inline
	int
	read_b_charclass_s(it_t & it, const class_t & cl, str_t * ps){
		read_blns(it);
		return read_charclass_s(it,cl,ps);
	}

	template<typename it_t, typename class_t, typename ch_t> inline
	int 
	read_b_charclass_c(it_t & it, const class_t & cl, ch_t * pc){
		read_blns(it);
		return read_charclass_c(it,cl,pc);
	}

//}
//{======================= digit, uint, sign_uint, sign_s_uint, int, dec, hex, oct, bin, 
/*
int_t может быть : long, long long, unsigned long, unsigned long long - для специализаций
[:digit:]   ::= [0-"$(($ss-1))"]
sign        ::= ('+'|'-')
int         ::= spcs[sign]spcs[:digit:]+
.                                                                                                                   специализация для
.                                                                                                   статистика использования
.                                                                           возвращаемое значение в случае          [w]char char16/32   stream_string     
название                аргументы               рег.выр.                    неудача переполнение            реализованность     
int read_digit          (it*, int ss, int_t*)   [:digit:]                   1       -1(EOF)                 OK
int read_uint           (it*, int ss, int_t*)   [:digit:]+                  1       -1                      OK
int read_sign_uint      (it*, int ss, int_t*)   [sign][:digit:]+            1       -1                      OK
int read_sign_spcs_uint (it*, int ss, int_t*)   [sign]spcs[:digit:]+        1       -1                      OK
int read_int            (it*, int ss, int_t*)   spcs[sign]spcs[:digit:]+    1       -1                      OK      OK
int read_dec            (it*, int_t*)           int#[:digit:]=[0-9]         1       -1              1       OK      OK
int read_hex            (it*, int_t*)           int#[:digit:]=[:xdigit:]    1       -1                      OK      OK
int read_oct            (it*, int_t*)           int#[:digit:]=[0-7]         1       -1                      OK      OK
int read_bin            (it*, int_t*)           int#[:digit:]=[01]          1       -1                      OK      OK
*/

	/*
	 * считывает 1 цифру в заданной системе счисления (СС) в ASCII-совместимой кодировке
	 * CC - от 2 до 35(буква Z)
	 * если конец файла - возвращает -1
	 * если символ не попадает в диапозон для заданной СС - возвращает 1
	 * если неправильная СС - возвращает 2
	 */
	template<typename it_t, typename int_t> inline
	int 
	read_digit(it_t & it, int ss, int_t * prez){
		typedef typename std::iterator_traits<it_t>::value_type ch_t;
		if(atend(it)) 
			return -1;
		if(2<=ss && ss<=10){
			if( (ch_t)'0'<=*it && *it< (ch_t)'0'+ss){
				*prez = *it++ -(ch_t)'0';
				return 0;
			}
			else
				return 1;
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
				return 1;
		}
		else
			//throw "неправильная система счисления";
			return 2;
	}

	/*
	 * считывает число без знака
	 * если переполнение (а это определяется numeric_limits<int_t>::max()) - возвращает -1
	 * если не удалось прочитать ни дной цифры - возвращает +1
	 */
	template<typename it_t, typename int_t> inline
	int 
	read_uint(it_t & it, int ss, int_t * prez){
		int_t premax = (numeric_limits<int_t>::max()-ss+1)/ss;
		r_ifnot(read_digit(it,ss,prez))
			return -1;
		while(*prez<=premax){
			int_t tmp;
			r_ifnot(read_digit(it,ss,&tmp))
				return 0;
			*prez *= ss;
			*prez += tmp;
		}
		return 1;
	}

	//sign - опционально
	//лимиты определяются не numeric_limits<int_t>:: min()..max(), а -max()..max()
	template<typename it_t, typename int_t> inline
	int 
	read_sign_uint(it_t & it, int ss, int_t * prez){
		if(atend(it))
			return -1;
		if(*it=='-'){
			it++;
			int err;
			r_if(err=read_uint(it,ss,prez)){
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
	int 
	read_sign_s_uint(it_t & it, int ss, int_t * prez){
		if(atend(it)){
			return -1;
		}
		if(*it=='-'){
			it++;
			read_spcs(it);
			int err;
			r_if(err=read_uint(it,ss,prez)){
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
	int 
	read_int(it_t & it, int ss, int_t * prez){
		read_spcs(it);
		return read_sign_s_uint(it,ss,prez);
	}

#if 1 //read_int для char и wchar_t
// http://www.cplusplus.com/reference/cstdlib/strtol/
#define def_read_int(ch_t, int_t, func) \
	inline \
	int\
	read_int(const ch_t *& ps, int ss, int_t * prez)\
	{\
		errno=0;\
		const ch_t * s = ps;\
		int_t tmp = func(s,const_cast<ch_t**>(&s),ss);\
		if(ps==s)  return -1;\
		ps=s; \
		if(errno)   return 1;\
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
	int 
	read_dec(it_t & it, int_t * prez){
		return read_int(it,10,prez);
	}
	
	template<typename it_t, typename int_t> inline
	int 
	read_hex(it_t & it, int_t * prez){
		return read_int(it,16,prez);
	}
	
	template<typename it_t, typename int_t> inline
	int 
	read_oct(it_t & it, int_t * prez){
		return read_int(it,8,prez);
	}
	
	template<typename it_t, typename int_t> inline
	int 
	read_bin(it_t & it, int_t * prez){
		return read_int(it,2,prez);
	}
//}
//{======================= float
/* todo придумать рег.выр-я
flt_t может быть : float, double, long double
int read_cfloat         (it*, flt_t*)
int read_ifloat         (it*, flt_t*)
int read_com_cfloat     (it*, flt_t*)
int read_com_ifloat     (it*, flt_t*)
*/
	//при чтении точка обязательна, если нет экспоненты
	template<typename it_t, typename flt_t> inline
	int 
	read_cfloat(it_t & it, flt_t * prez);
	
	//при чтении обязательна НЕ ТОЧКА А ЗАПЯТАЯ, если нет экспоненты
	template<typename it_t, typename flt_t> inline
	int 
	read_com_cfloat(it_t & it, flt_t * prez);
	
	//int сойдет за float
	template<typename it_t, typename flt_t> inline
	int 
	read_ifloat(it_t & it, flt_t * prez);
	
	//int сойдет за float
	template<typename it_t, typename flt_t> inline
	int 
	read_com_ifloat(it_t & it, flt_t * prez);
//}

}//namespace str
#endif //BASE_PARSE_H
