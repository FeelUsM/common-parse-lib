#include <stdio.h>//для EOF
/*
 * это для того, что бы  не писать типа
 * if(!(errcode=read::smpt(...)))
 * а писать rif(errcode=read::smth(...))
 * по моему так удобней
 */
#define rif(expr) if(!(expr))
#define rwhile(expr) while(!(expr))

/*
 * === стандартные КЛАССЫ СИМВОЛОВ ===
 * этот "класс" как char_traits, на самом деле не класс а шаблонный namespace
 * его я реализую сам, а не специализирую из <ctype.h> или <wctype.h>, 
 * т.к. функции оттуда зависят от локали
 * см. спраку: http://www.cplusplus.com/reference/cctype/
 * но это не будет работать с ASCII несовместимой (т.е. по первым 128 символам) кодировкой
 */
template<typename ch_t>
class is{
	static bool cntr(ch_t c)	{	return c>=0 && c<0x20 || c==0x7f;	}
	static bool blank(ch_t c)	{	return c==' ' || c=='\t';	}
	static bool space(ch_t c)	{	return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\f' || c=='\v';	}
	static bool upper(ch_t c)	{	return c>='A' && c<='Z';	}
	static bool lower(ch_t c)	{	return c>='a' && c<='z';	}
	static bool alpha(ch_t c)	{	return c>='a' && c<='z' || c>='A' && c<='Z';	}
	static bool digit(ch_t c)	{	return c>='0' && c<='9';	}
	static bool xdigit(ch_t c)	{	return c>='0' && c<='9' || c>='a' && c<='f' || c>='A' && c<='F';	}
	static bool alnum(ch_t c)	{	return alpha(c) || digit(c);	}
	static bool punct(ch_t c)	{	return c>='!' && c<='/' || c>=':' && c<='@' || c>='[' && c<='`' || c>='{' && c<='~';	}
	static bool graph(ch_t c)	{	return c>='!' && c<='~';	}
	static bool print(ch_t c)	{	return c>=' ' && c<='~';	}
};

namespace str{
	//тут я планирую написать и специализировать строковые функции для разных типов строк
	//они понадобятся при оптимизации функций из read::
}
/*
 * для всех итераторов или указателей по строкам
 * необходим "метод" atend(it)
 * это как тоже самое что и it!=container.end()
 * только здесь для этой операции нам не требуется contaier
 */
template<typename it_t>
bool atend(it_t);
/*
 * и специализация для указателей (на простые типы)
 * а для "пользовательских" итераторов каждый пользователь будет это специализировать в своем коде
 */
template<typename ch_t>
bool atend(const ch_t * pc)
{	return !*pc;	}

namespace read{
/*
функции возвращают код ошибки
при успешном прочтении итератор указывает на следующий символ после последнего прочитанного
при неудачном прочтении итератор указывает на место ошибки

size_t 		n
ch_t 		c
ch_t * 		s
func_obj 	bool cf(c)  //наподобие bool isspace(c)
func_obj	err pf(it*) //наподобие int read::spc(it*)
func_obj	err pf(it*, rez*)

int read::fix_length		(it*, n)			.{n}
int read::fix_length		(it*, n, pstr*)
int read::fix_char			(it*, c)			c
int read::until_char		(it*, c)			.*c
int read::until_char		(it*, c, pstr*)				OK
int read::until_eof			(it*)				.*$
int read::until_eof			(it*,    pstr*)
int read::charclass			(it*, cf)			[ ]
int read::charclass			(it*, cf, ch*)
int read::while_charclass	(it*, cf)			[ ]*
int read::while_charclass	(it*, cf, pstr*)
int read::until_charclass	(it*, cf)			.*[ ]
int read::until_charclass	(it*, cf, pstr*)
int read::fix_str			(it*, s)			str
int read::until_str			(it*, s)			.*str
int read::until_str			(it*, s, pstr*)
int read::until_pattern		(it*, pf)			.*(...)
int read::until_pattern		(it*, pf, pstr*)
int read::until_pattern		(it*, pf, pstr*, rez*)

int read::spc				(it*)				[:space:]
int read::int				(it*, int ss, int_t*) [0-"$(($ss-1))"]+
int read::dec				(it*, int_t*)		[0-9]+
int read::hex				(it*, int_t*)		[:xdigit:]+
int read::oct				(it*, int_t*)		[0-7]+
int read::bin				(it*, int_t*)		[01]+
int read::cfloat				(it*, flt_t*)	
int read::ifloat				(it*, flt_t*)
*/

	/*
	 * fix_length(it_t * pit, size_t n)
	 * передвигает итератор на n позиций
	 *
	 * если встретился конец файла (кроме последней позиции, когда сдвигаться уже не надо)
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 */
	template<typename it_t>
	int 
	fix_length(it_t * pit, size_t n);
	
	/*
	 * fix_length(it_t * pit, size_t n, str_t * pstr)
	 * считывает строку длиной n
	 *
	 * если встретился конец файла (кроме последней позиции, когда сдвигаться уже не надо)
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 * в строку помещается все до конца файла
	 */
	template<typename it_t, typename str_t>
	int 
	fix_length(it_t * pit, size_t n, str_t * pstr);
	
	/*
	 * fix_char(it_t * pit, ch_t c)
	 * если указываемый итератором символ совпадает с c, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает EOF
	 * если указываемый итератором символ не совпадает с c, то возвращает 1, и итератор не двигает
	 */
	template<typename it_t, typename ch_t>
	int 
	fix_char(it_t * pit, ch_t c);
	
	/*
	 * until_char(it_t * pit, ch_t ch)
	 * сдвигает указатель до тех пор, пока не встретится заданный символ
	 *
	 * если встретился конец файла (до того, как встретился заданный символ), 
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 */
	template<typename it_t, typename ch_t>
	int 
	until_char(it_t * pit, ch_t ch);

	/*
	 * until_char(it_t * pit, ch_t ch, str_t * pstr)
	 * считывает строку, до тех пор пока не встретится заданный символ
	 *
	 * если встретился конец файла (до того, как встретился заданный символ), 
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 * в строку помещается все до конца файла
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename ch_t, typename str_t>
	int 
	until_char(it_t * pit, ch_t ch, str_t * pstr){
		while(!atend(*pit) && **pit!=ch)
			(*pstr)+=*(*pit)++;
		return atend(*pit) ? EOF : 0;
	}
	
	/*
	 * until_eof(it_t * pit)
	 * передвигает указатель в конец файла
	 *
	 * всегда возвращает 0
	 */
	template<typename it_t>
	int 
	until_eof(it_t * pit);

	/*
	 * until_eof(it_t * pit, str_t * pstr)
	 * считывает все до конца файла
	 * 
	 * всегда возвращает 0
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename str_t>
	int 
	until_eof(it_t * pit, str_t * pstr);
	
	/*
	 * charclass(it_t * pit, const class_t & is)
	 * если is(.) возвращает true от указываемого итератором символа, то сдвигает итератор на 1 позицию
	 *
	 * если итератор в состоянии atend(), возвращает EOF
	 * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и итератор не двигает
	 */
	template<typename it_t, typename class_t>
	int 
	charclass(it_t * pit, const class_t & is);
	
	/*
	 * while_charclass(it_t * pit, const class_t & is)
	 * сдвигает итератор, пока is(.) возвращает true от указываемого итератором символа
	 * 
	 * если встретился конец файла до того как is(.) вернул false
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 */
	template<typename it_t, typename class_t>
	int 
	while_charclass(it_t * pit, const class_t & is);
	
	/*
	 * while_charclass(it_t * pit, const class_t & is, str_t * pstr)
	 * считывает строку, пока is(.) возвращает true от указываемого итератором символа
	 * 
	 * если встретился конец файла до того как is(.) вернул false
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 * в строку помещается все до конца файла
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename class_t, typename str_t>
	int 
	while_charclass(it_t * pit, const class_t & is, str_t * pstr);

	/*
	 * until_charclass(it_t * pit, const class_t & is)
	 * сдвигает итератор, пока is(.) возвращает false от указываемого итератором символа
	 * 
	 * если встретился конец файла до того как is(.) вернул true
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 */
	template<typename it_t, typename class_t>
	int 
	until_charclass(it_t * pit, const class_t & is);
	
	/*
	 * until_charclass(it_t * pit, const class_t & is, str_t * pstr)
	 * считывает строку, пока is(.) возвращает false от указываемого итератором символа
	 * 
	 * если встретился конец файла до того как is(.) вернул true
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 * в строку помещается все до конца файла
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename class_t, typename str_t>
	int 
	until_charclass(it_t * pit, const class_t & is, str_t * pstr);

	/*
	 * fix_str(it_t * pit, ch_t * ch)
	 * итератор должен указывать на такую же строку, как и ch, и если это так, сдвигает итератор на длину этой строки
	 * 
	 * если встречается конец файла
	 * возвращает EOF
	 * и итератор указывает на конец файла
	 *
	 * если с какого-то символа строки начинают различаться
	 * возвращает 1
	 * и итератор указывает на этот символ
	 */
	template<typename it_t, typename ch_t>
	int 
	fix_str(it_t * pit, const ch_t * s);
	
	/*
	 * until_str(it_t * pit, const ch_t * s)
	 * сдвигает указатель до тех пор, пока с него не будет нчинаться строка s
	 *
	 * если встречается конец файла при попытке прочитать строку
	 * возвращает EOF
	 * итератор указывает на начало неудачно прочитанной строки, т.е. на конец файла минус длина строки плюс 1
	 */
	template<typename it_t, typename ch_t>
	int 
	until_str(it_t * pit, const ch_t * s);

	/*
	 * until_str(it_t * pit, const ch_t * s, str_t * pstr)
	 * считывает символы в строку pstr до тех пор, пока с очередного не будет нчинаться строка s
	 *
	 * если встречается конец файла при попытке прочитать строку s из файла
	 * возвращает EOF
	 * итератор указывает на начало неудачно прочитанной строки, т.е. на конец файла минус длина строки плюс 1
	 * в строку pstr помещается все до этого итератора
	 *
	 * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
	 */
	template<typename it_t, typename ch_t, typename str_t>
	int 
	until_str(it_t * pit, const ch_t * s, str_t * pstr);

	template<typename it_t, typename pattern_t>
	int 
	until_pattern(it_t * pit, const pattern_t & pattern);
	
	template<typename it_t, typename pattern_t, typename str_t>
	int 
	until_pattern(it_t * pit, const pattern_t & pattern, str_t * pstr);

	template<typename it_t, typename pattern_t, typename str_t, typename rez_t>
	int 
	until_pattern(it_t * pit, const pattern_t & pattern, str_t * pstr, rez_t * rez);
	
//=======================================

	template<typename it_t>
	int 
	spc(it_t * it);
	
	template<typename it_t, typename int_t>
	int 
	_int(it_t * pit, int ss, int_t * prez);
	
	template<typename it_t, typename int_t>
	int 
	dec(it_t * pit, int_t * prez);
	
	template<typename it_t, typename int_t>
	int 
	hex(it_t * pit, int_t * prez);
	
	template<typename it_t, typename int_t>
	int 
	oct(it_t * pit, int_t * prez);
	
	template<typename it_t, typename int_t>
	int 
	bin(it_t * pit, int_t * prez);
	
	//при чтении точка обязательна, если нет экспоненты
	template<typename it_t, typename flt_t>
	int 
	cfloat(it_t * pit, flt_t * prez);
	
	//int сойдет за float
	template<typename it_t, typename flt_t>
	int 
	ifloat(it_t * pit, flt_t * prez);
	
}//namespace read