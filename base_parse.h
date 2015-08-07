//FeelUs
#ifndef BASE_PARSE_H
#define BASE_PARSE_H

#include <errno.h>
#include <stdlib.h> //для strtol, strtoll, strtoul, strtoull, strtof, strtod, strtold
#include <wchar.h>  //для wcstol, wcstoll, wcstoul, wcstoull, wcstof, wcstod, wcstold
#include <iterator> //для iterator_traits<it_t>::value_type
namespace str{
using std::iterator_traits;  


/*
 * === стандартные КЛАССЫ СИМВОЛОВ ===
 * его я реализую сам, а не специализирую из <ctype.h> или <wctype.h>, 
 * т.к. функции оттуда зависят от локали
 * см. спраку: http://www.cplusplus.com/reference/cctype/
 * но это не будет работать с ASCII несовместимой (т.е. по первым 128 символам) кодировкой
 */
template<typename ch_t>
struct is{
    static bool cntr(ch_t c)    {   return c>=0 && c<0x20 || c==0x7f;   }
    static bool blank(ch_t c)   {   return c==' ' || c=='\t';   }
    static bool space(ch_t c)   {   return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\f' || c=='\v';   }
    static bool upper(ch_t c)   {   return c>='A' && c<='Z';    }
    static bool lower(ch_t c)   {   return c>='a' && c<='z';    }
    static bool alpha(ch_t c)   {   return c>='a' && c<='z' || c>='A' && c<='Z';    }
    static bool digit(ch_t c)   {   return c>='0' && c<='9';    }
    static bool xdigit(ch_t c)  {   return c>='0' && c<='9' || c>='a' && c<='f' || c>='A' && c<='F';    }
    static bool alnum(ch_t c)   {   return alpha(c) || digit(c);    }
    static bool punct(ch_t c)   {   return c>='!' && c<='/' || c>=':' && c<='@' || c>='[' && c<='`' || c>='{' && c<='~';    }
    static bool graph(ch_t c)   {   return c>='!' && c<='~';    }
    static bool print(ch_t c)   {   return c>=' ' && c<='~';    }
};

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
 * т.е. span - строка набора символов
 * для консистентности read_charclass и read_charclass_c также могут принимать это объект
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
 * для задания отдельного символа - указываются одинаковые символы как нижняя и верхняя граница
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
 * read_charclass(pit,spn_smth<тип>())
 * не забывайте скобочки после spn_smth<тип>
 */
template<> inline bispan<char>  spn_cntr  <char>()   {   return bispan<char>("\1\x20\x7f\x7f\0");   }
template<> inline span  <char>  spn_blank <char>()   {   return span<char>(" \t");   }
template<> inline span  <char>  spn_space <char>()   {   return span<char>(" \t\n\r\f\v");   }
template<> inline bispan<char>  spn_upper <char>()   {   return bispan<char>("AZ\0");    }
template<> inline bispan<char>  spn_lower <char>()   {   return bispan<char>("az\0");    }
template<> inline bispan<char>  spn_alpha <char>()   {   return bispan<char>("azAZ\0");    }
template<> inline bispan<char>  spn_digit <char>()   {   return bispan<char>("09\0");    }
template<> inline bispan<char>  spn_xdigit<char>()   {   return bispan<char>("09afAF\0");    }
template<> inline bispan<char>  spn_alnum <char>()   {   return bispan<char>("azAZ09\0");    }
template<> inline bispan<char>  spn_punct <char>()   {   return bispan<char>("!/:@[`{~\0");    }
template<> inline bispan<char>  spn_graph <char>()   {   return bispan<char>("!~\0"); }
template<> inline bispan<char>  spn_print <char>()   {   return bispan<char>(" ~\0"); }

template<> inline bispan<wchar_t>   spn_cntr  <wchar_t>()   {   return bispan<wchar_t>(L"\1\x20\x7f\x7f\0");   }
template<> inline span  <wchar_t>   spn_blank <wchar_t>()   {   return span  <wchar_t>(L" \t");   }
template<> inline span  <wchar_t>   spn_space <wchar_t>()   {   return span  <wchar_t>(L" \t\n\r\f\v");   }
template<> inline bispan<wchar_t>   spn_upper <wchar_t>()   {   return bispan<wchar_t>(L"AZ\0");    }
template<> inline bispan<wchar_t>   spn_lower <wchar_t>()   {   return bispan<wchar_t>(L"az\0");    }
template<> inline bispan<wchar_t>   spn_alpha <wchar_t>()   {   return bispan<wchar_t>(L"azAZ\0");    }
template<> inline bispan<wchar_t>   spn_digit <wchar_t>()   {   return bispan<wchar_t>(L"09\0");    }
template<> inline bispan<wchar_t>   spn_xdigit<wchar_t>()   {   return bispan<wchar_t>(L"09afAF\0");    }
template<> inline bispan<wchar_t>   spn_alnum <wchar_t>()   {   return bispan<wchar_t>(L"azAZ09\0");    }
template<> inline bispan<wchar_t>   spn_punct <wchar_t>()   {   return bispan<wchar_t>(L"!/:@[`{~\0");    }
template<> inline bispan<wchar_t>   spn_graph <wchar_t>()   {   return bispan<wchar_t>(L"!~\0"); }
template<> inline bispan<wchar_t>   spn_print <wchar_t>()   {   return bispan<wchar_t>(L" ~\0"); }

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
bool atend(ch_t * pc)
{   return !*pc;    }

/*
 * это для того, что бы  не писать типа
 * if(!(errcode=read::smpt(...)))
 * а писать r_if(errcode=read::smth(...))
 * по моему так удобней
 */
#define r_if(expr)        if((expr)==0)
#define r_while(expr)  while((expr)==0)
#define rm_if(expr)       if((expr)>=0)     //типа рег. выр.  '.*' * - multiple -> m
#define rm_while(expr) while((expr)>=0)
#define rp_if(expr)       if((expr)>0)      //типа рег. выр.  '.+' + - plus -> p
#define rp_while(expr) while((expr)>0)
#define r_ifnot(expr)      if(expr)
#define r_whilenot(expr) while(expr)

/*
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
func_obj    err pf(it*) //наподобие int read::spc(it*)
func_obj    err pf(it*, rez*)
                                                                                                    специализация для
len - кол-во символов, добавлненных в *pstr                                                         char    wchar_t char16_t    char_32_t
                                                                возвращаемое значение в случае  реализованность
название                    аргументы           рег.выр.        если EOF    если не EOF     статистика использования
int read::until_eof         (it*)               .*$             0           0               1   OK
int read::until_eof         (it*,    pstr*)     .*$             len         len                 OK

int read::fix_length        (it*, n)            .{n}            -1          0                   OK
int read::fix_length        (it*, n, pstr*)     .{n}            -(1+len)    0                   OK
int read::fix_char          (it*, c)            c               -1          0 или 1         8   OK
int read::fix_str           (it*, s)            str             -1          0 или 1         1   OK
int read::charclass         (it*, is)           [ ]             -1          0 или 1             OK
int read::charclass         (it*, is)           [ ]             -1          0 или 1             OK
int read::charclass         (it*, is)           [ ]             -1          0 или 1             OK
int read::charclass         (it*, is, pstr*)    [ ]             -1          0 или 1             OK
int read::charclass         (it*, spn, pstr*)   [ ]             -1          0 или 1             OK
int read::charclass         (it*, bspn, pstr*)  [ ]             -1          0 или 1         1   OK
int read::charclass_c       (it*, is, ch*)      [ ]             -1          0 или 1             OK
int read::charclass_c       (it*, spn, ch*)     [ ]             -1          0 или 1             OK
int read::charclass_c       (it*, bspn, ch*)    [ ]             -1          0 или 1             OK

int read::until_char        (it*, c)            .*c             -(1+len)    len                 OK
int read::until_charclass   (it*, is)           .*[ ]           -(1+len)    len                 OK
int read::until_charclass   (it*, spn)          .*[ ]           -(1+len)    len                 OK
int read::until_charclass   (it*, bspn)         .*[ ]           -(1+len)    len                 OK
int read::while_charclass   (it*, is)           [ ]*            -(1+len)    len                 OK
int read::while_charclass   (it*, spn)          [ ]*            -(1+len)    len                 OK
int read::while_charclass   (it*, bspn)         [ ]*            -(1+len)    len                 OK
int read::until_char        (it*, c, pstr*)     .*c             -(1+len)    len                 OK
int read::until_charclass   (it*, is, pstr*)    .*[ ]           -(1+len)    len                 OK
int read::until_charclass   (it*, spn, pstr*)   .*[ ]           -(1+len)    len                 OK
int read::until_charclass   (it*, bspn, pstr*)  .*[ ]           -(1+len)    len                 OK
int read::while_charclass   (it*, is, pstr*)    [ ]*            -(1+len)    len                 OK
int read::while_charclass   (it*, spn, pstr*)   [ ]*            -(1+len)    len                 OK
int read::while_charclass   (it*, bspn, pstr*)  [ ]*            -(1+len)    len             1   OK

int read::until_str         (it*, s)            .*str           -(1+len)    len                 OK
int read::until_str         (it*, s, pstr*)     .*str           -(1+len)    len                 OK
int read::until_pattern     (it*, pf)           .*( )           -(1+len)    len                 OK
int read::until_pattern     (it*, pf, pstr*)    .*( )           -(1+len)    len                 OK
int read::until_pattern     (it*, pf, pstr*, rez*)  .*( )       -(1+len)    len                 OK

int read::spc               (it*)               [:space:]       
int read::spcs              (it*)               [:space:]*       							4

int_t может быть : long, long long, unsigned long, unsigned long long
flt_t может быть : float, double, long double
                                                                переполнение неудача
int read::_int          (it*, int ss, int_t*)   [0-"$(($ss-1))"]+   -1      1                       OK      OK
int read::dec               (it*, int_t*)       [0-9]+              -1      1               1       OK      OK
int read::hex               (it*, int_t*)       [:xdigit:]+         -1      1                       OK      OK
int read::oct               (it*, int_t*)       [0-7]+              -1      1                       OK      OK
int read::bin               (it*, int_t*)       [01]+               -1      1                       OK      OK
int read::cfloat            (it*, flt_t*)   
int read::ifloat            (it*, flt_t*)
int read::rus_cfloat        (it*, flt_t*)   
int read::rus_ifloat        (it*, flt_t*)
*/

//======================= until_eof
    /*
     * until_eof(it_t * pit)
     * передвигает указатель в конец файла
     *
     * всегда возвращает 0
     */
    template<typename it_t> inline
    int 
    read_until_eof(it_t * pit){
        while(!atend(*pit))
            (*pit)++;
        return 0;
    }

    /*
     * until_eof(it_t * pit, str_t * pstr)
     * считывает все до конца файла
     * 
     * возвращает размер считанного
     *
     * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер считанного
     */
    template<typename it_t, typename str_t> inline 
    int 
    read_until_eof(it_t * pit, str_t * pstr){
        int i=0;
        while(!atend(*pit)){
            (*pit)++;
            i++;
        }
        return i;
    }
    
//======================= fix_length, fix_char, fix_str, charclass
    /*
     * fix_length(it_t * pit, size_t n)
     * передвигает итератор на n позиций
     *
     * если встретился конец файла (кроме последней позиции, когда сдвигаться уже не надо)
     * возвращает -1
     * и итератор указывает на конец файла
     */
    template<typename it_t> inline
    int 
    read_fix_length(it_t * pit, size_t n){
        for(;n>0;n--,(*pit)++)
            if(atend(*pit))
                return -1;
        return 0;
    }
    
    /*
     * fix_length(it_t * pit, size_t n, str_t * pstr)
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
    read_fix_length(it_t * pit, size_t n, str_t * pstr){
        for(size_t i=0; i<n; i++, (*pit)++)
            if(atend(*pit))
                return -(1+i);
            else
                (*pstr)+=**pit;
        return 0;
    }
    
    /*
     * fix_char(it_t * pit, ch_t c)
     * если указываемый итератором символ совпадает с c, то сдвигает итератор на 1 позицию
     *
     * если итератор в состоянии atend(), возвращает -1
     * если указываемый итератором символ не совпадает с c, то возвращает 1, и итератор не двигает
     */
    template<typename it_t, typename ch_t> inline
    int 
    read_fix_char(it_t * pit, ch_t c){
        if(atend(*pit)) return -1;
        if(**pit!=c)    return 1;
        (*pit)++;
        return 0;
    }

    /*
     * fix_str(it_t * pit, ch_t * ch)
     * итератор должен указывать на такую же строку, как и ch, и если это так, сдвигает итератор на длину этой строки
     * 
     * если встречается конец файла
     * возвращает -1
     * и итератор указывает на конец файла
     *
     * если с какого-то символа строки начинают различаться
     * возвращает 1
     * и итератор указывает на этот символ
     */
    template<typename it_t, typename ch_t> inline
    int 
    read_fix_str(it_t * pit, const ch_t * s){
        while(!atend(*pit))
            if(!*s)
                return 0;
            else if(**pit!=*s)
                return 1;
            else
                (*pit)++, s++;
        return -1;
    }
    
    /*
     * charclass(it_t * pit, const class_t & is)
     * если is(.) возвращает true от указываемого итератором символа, то сдвигает итератор на 1 позицию
     *
     * если итератор в состоянии atend(), возвращает -1
     * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и итератор не двигает
     */
    template<typename it_t, typename class_t> inline
    int 
    read_charclass(it_t * pit, const class_t & is){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*pit)++;
        return 0;
    }

    template<typename it_t> inline
    int 
    read_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(**pit==*s.s++){
                (*pit)++;
                return 0;
            }
        return 1;
    }
        
    template<typename it_t> inline
    int 
    read_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(*s.s++<=**pit && **pit<=*s.s++){
                (*pit)++;
                return 0;
            }
        return 1;
    }
        
    /*
     * charclass(it_t * pit, const class_t & is, pstr_t * pstr)
     * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
     *
     * если итератор в состоянии atend(), возвращает -1
     * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
     */
    template<typename it_t, typename class_t, typename str_t> inline
    int 
    read_charclass(it_t * pit, const class_t & is, str_t * pstr){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*pstr)+=**pit;
        (*pit)++;
        return 0;
    }

    template<typename it_t, typename str_t> inline
    int 
    read_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(**pit==*s.s++){
                (*pstr)+=*(*pit)++;
                return 0;
            }
        return 1;
    }
        
    template<typename it_t, typename str_t> inline
    int 
    read_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(*s.s++<=**pit && **pit<=*s.s++){
                (*pstr)+=*(*pit)++;
                return 0;
            }
        return 1;
    }
        
    /*
     * charclass(it_t * pit, const class_t & is, ch_t * ch)
     * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
     *
     * если итератор в состоянии atend(), возвращает -1
     * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
     */
    template<typename it_t, typename class_t, typename ch_t> inline
    int 
    read_charclass_c(it_t * pit, const class_t & is, ch_t * pch){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*pch)=**pit;
        (*pit)++;
        return 0;
    }
    
    template<typename it_t, typename ch_t> inline
    int 
    read_charclass_c(it_t * pit, span<typename iterator_traits<it_t>::value_type> s, ch_t * pch){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(**pit==*s.s++){
                (*pch)=*(*pit)++;
                return 0;
            }
        return 1;
    }
        
    template<typename it_t, typename ch_t> inline
    int 
    read_charclass_c(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s, ch_t * pch){
        if(atend(*pit)) return -1;
        while(*s.s)
            if(*s.s++<=**pit && **pit<=*s.s++){
                (*pch)=*(*pit)++;
                return 0;
            }
        return 1;
    }
        
//======================= until_char, until_charclass, while_charclass
    /*
     * until_char(it_t * pit, ch_t ch)
     * сдвигает указатель до тех пор, пока (не встретится заданный символ) или конец файла
     * until_charclass(it_t * pit, const class_t & is)
     * сдвигает указатель до тех пор, пока (is(.) возвращает false) или не встретился конец файла
     * while_charclass(it_t * pit, const class_t & is)
     * сдвигает указатель до тех пор, пока (is(.) возвращает true) или не встретился конец файла
     *
     * если встретился конец файла - возвращает -(1 + размер считанного)
     * и итератор указывает на конец файла
     * если встретился заданный символ - возвращает размер считанного
     * и итератор указывает на заданный символ
     */
    template<typename it_t, typename ch_t> inline
    int 
    read_until_char(it_t * pit, ch_t ch){
        int i=0;
        while(!atend(*pit))
            if(**pit==ch)
                return i;
            else{
                (*pit)++;
                i++;
            }
        return -(1+i);
    }

    template<typename it_t, typename class_t> inline
    int 
    read_until_charclass(it_t * pit, const class_t & is){
        int i=0;
        while(!atend(*pit))
            if(is(**pit))
                return i;
            else{
                (*pit)++;
                i++;
            }
        return -(1+i);
    }
    
            template<typename it_t> inline
            int 
            read_until_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(**pit==*ss++)
                            return i;
                    (*pit)++;
                    i++;
                }
                return -(1+i);
            }

            template<typename it_t> inline
            int 
            read_until_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(*ss++<=**pit && **pit<=*ss++)
                            return i;
                    (*pit)++;
                    i++;
                }
                return -(1+i);
            }

    template<typename it_t, typename class_t> inline
    int 
    read_while_charclass(it_t * pit, const class_t & is){
        int i=0;
        while(!atend(*pit))
            if(!is(**pit))
                return i;
            else{
                (*pit)++;
                i++;
            }
        return -(1+i);
    }
    
            template<typename it_t> inline
            int 
            read_while_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(**pit==*ss++){
                            (*pit)++;
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
            read_while_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(*ss++<=**pit && **pit<=*ss++){
                            (*pit)++;
                            i++;
                            goto met;
                        }
                    return i;
                met:;
                }
                return -(1+i);
            }

    /*
     * until_char(it_t * pit, ch_t ch, str_t * pstr)
     * считывает строку, до тех пор пока не встретится заданный символ или конец файла
     * until_charclass(it_t * pit, const class_t & is, str_t * pstr)
     * считывает строку, пока is(.) возвращает false от указываемого итератором символа или не встретился конец файла
     * while_charclass(it_t * pit, const class_t & is, str_t * pstr)
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
    read_until_char(it_t * pit, ch_t ch, str_t * pstr){
        int i=0;
        while(!atend(*pit))
            if(**pit==ch)
                return i;
            else{
                (*pstr)+=*(*pit)++;
                i++;
            }
        return -(1+i);
    }
    
    template<typename it_t, typename class_t, typename str_t> inline
    int 
    read_until_charclass(it_t * pit, const class_t & is, str_t * pstr){
        int i=0;
        while(!atend(*pit))
            if(is(**pit))
                return i;
            else{
                (*pstr)+=*(*pit)++;
                i++;
            }
        return -(1+i);
    }

            template<typename it_t, typename str_t> inline
            int 
            read_until_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(**pit==*ss++)
                            return i;
                    (*pstr)+=*(*pit)++;
                    i++;
                }
                return -(1+i);
            }

            template<typename it_t, typename str_t> inline
            int 
            read_until_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(*ss++<=**pit && **pit<=*ss++)
                            return i;
                    (*pstr)+=*(*pit)++;
                    i++;
                }
                return -(1+i);
            }

    template<typename it_t, typename class_t, typename str_t> inline
    int 
    read_while_charclass(it_t * pit, const class_t & is, str_t * pstr){
        int i=0;
        while(!atend(*pit))
            if(!is(**pit))
                return i;
            else{
                (*pstr)+=*(*pit)++;
                i++;
            }
        return -(1+i);
    }

            template<typename it_t, typename str_t> inline
            int 
            read_while_charclass(it_t * pit, span<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(**pit==*ss++){
                            (*pstr)+=*(*pit)++;
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
            read_while_charclass(it_t * pit, bispan<typename iterator_traits<it_t>::value_type> s, str_t * pstr){
                int i=0;
                while(!atend(*pit)){
                    typename iterator_traits<it_t>::pointer ss=s.s;
                    while(*ss)
                        if(*ss++<=**pit && **pit<=*ss++){
                            (*pstr)+=*(*pit)++;
                            i++;
                            goto met;
                        }
                    return i;
                met:;
                }
                return -(1+i);
            }

//======================= until_str, until_pattern
    /*
     * until_str(it_t * pit, const ch_t * s[, str_t * pstr])
     * считывает символы в строку pstr до тех пор, пока с очередного не будет начинаться строка s
     * итератор указывает на следующий символ после конца найденной строки или на конец файла
     *
     * если встретился конец файла - возвращает -(1+число помещенных в строку символов)
     * если встретился заданный символ - возвращает + число помещенных в строку символов
     *
     * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
     */
    template<typename ch_t, typename it_t> inline
    int 
    read_until_str(it_t * pit, const ch_t * s){
        int i=0;
        for(;;){
            it_t lit = *pit;
            int err;
            r_if(err=/*read::*/fix_str(&lit,s)){//если прочитал фиксированную строку
                (*pit)=lit;
                return err>0 ? i : -(1+i);
            }
            (*pit)++;
            i++;
        }
    }

    template<typename it_t, typename ch_t, typename str_t> inline
    int 
    read_until_str(it_t * pit, const ch_t * s, str_t * pstr){
        int i=0;
        for(;;){
            it_t lit = *pit;
            int err;
            r_if(err=/*read::*/fix_str(&lit,s)){//если прочитал фиксированную строку
                (*pit)=lit;
                return err>0 ? i : -(1+i);
            }
            (*pstr)+=*(*pit)++;
            i++;
        }
    }

    /*
     * until_pattern(it_t * pit, const pattern_t & pattern[, str_t * pstr[, rez_t * rez]])
     * считывает символы в строку pstr до тех пор, пока с очередного не будет начинаться последовательноть
     *     удовлетворяющая (возвращает 0) функциональному объекту pattern(it_t * pit[, rez_t * rez])
     *     или он не окажетсяконцом файла
     * итератор указывает на следующий символ после конца найденной последовательности или на конец файла
     *
     * если встретился конец файла - возвращает -(1+число помещенных в строку символов)
     * если встретился заданный символ - возвращает + число помещенных в строку символов
     *
     * если требуется прочитать 1 (а не 0) или более символов - проверяйте размер строки
     */
    template<typename it_t, typename pattern_t> inline
    int 
    read_until_pattern(it_t * pit, const pattern_t & read_pattern){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit))
                return i;
        }
        return -(1+i);
    }
    
    template<typename it_t, typename pattern_t, typename str_t> inline
    int 
    read_until_pattern(it_t * pit, const pattern_t & read_pattern, str_t * pstr){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit))
                return i;
            (*pstr)=**pit;
        }
        return -(1+i);
    }

    template<typename it_t, typename pattern_t, typename str_t, typename rez_t> inline
    int 
    read_until_pattern(it_t * pit, const pattern_t & read_pattern, str_t * pstr, rez_t * rez){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit,rez))
                return i;
            (*pstr)=**pit;
        }
        return -(1+i);
    }
    
//======================= spc, spcs, _int, dec, hex, oct, bin, cfloat, ifloat, rus_cfloat, rus_ifloat

    template<typename it_t> inline
    int 
    read_spc(it_t * pit){
        return read_charclass(pit,spn_space<typename iterator_traits<it_t>::value_type>());
    }
    
    template<typename it_t> inline
    int 
    read_spcs(it_t * pit){
        return read_while_charclass(pit,spn_space<typename iterator_traits<it_t>::value_type>());
    }

    /*
     * On success, the function returns (as rez) the converted integral number as a int_t value.
     * If no valid conversion could be performed, a zero value is returned (as rez) and return 1.
     * If the value read is out of the range of representable values by a int_t, 
     * the function returns (as rez) LONG_MAX or LONG_MIN (зависит от int_t) (defined in <climits>), 
     *     and return -1
     * перед вызовом errno должно быть установлено в 0 (оно для каждого потока свое)
     */
    //template<typename it_t, typename int_t> inline
    //int 
    //read_int(it_t * pit, int ss, int_t * prez);
//todo оптимизировать, чтобы не заморачивался с пробелами
#if 1 //реализации read_int (внутри #if чтобы был один уровень вложенности для свертывания (в notepad++) )
#define def_read_int(ch_t, int_t, func) \
    inline \
    int\
    read_int(const ch_t ** ps, int ss, int_t * prez)\
    {\
        const ch_t * s = *ps;\
        r_if(read_spc(ps)){\
            *ps=s;\
            *prez=0;\
            return 1;\
        }\
		errno=0;\
        *prez=func(*ps,const_cast<ch_t**>(ps),ss);\
        if(*ps==s)  return 1;\
        if(errno)   return -1;\
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
    read_dec(it_t * pit, int_t * prez){
        return read_int(pit,10,prez);
    }
    
    template<typename it_t, typename int_t> inline
    int 
    read_hex(it_t * pit, int_t * prez){
        return read_int(pit,16,prez);
    }
    
    template<typename it_t, typename int_t> inline
    int 
    read_oct(it_t * pit, int_t * prez){
        return read_int(pit,8,prez);
    }
    
    template<typename it_t, typename int_t> inline
    int 
    read_bin(it_t * pit, int_t * prez){
        return read_int(pit,2,prez);
    }
    
    //при чтении точка обязательна, если нет экспоненты
    template<typename it_t, typename flt_t> inline
    int 
    read_cfloat(it_t * pit, flt_t * prez);
    
    //при чтении обязательна НЕ ТОЧКА А ЗАПЯТАЯ, если нет экспоненты
    template<typename it_t, typename flt_t> inline
    int 
    read_rus_cfloat(it_t * pit, flt_t * prez);
    
    //int сойдет за float
    template<typename it_t, typename flt_t> inline
    int 
    read_ifloat(it_t * pit, flt_t * prez);
    
    //int сойдет за float
    template<typename it_t, typename flt_t> inline
    int 
    read_rus_ifloat(it_t * pit, flt_t * prez);
    

template<typename ch_t, typename it_t>
struct basic_read{
    static int until_eof(it_t * pit)
    {   return read_until_eof(pit); }
    
                    template<typename str_t>    
    static int until_eof(it_t * pit, str_t * pstr)
    {   return read_until_eof(pit,pstr);    }
    
    static int fix_length(it_t * pit, size_t n)
    {   return read_fix_length(pit,n);  }
    
                    template<typename str_t>    
    static int fix_length(it_t * pit, size_t n, str_t * pstr)
    {   return read_fix_length(pit,n,pstr); }
    
    static int fix_char(it_t * pit, ch_t c)
    {   return read_fix_char(pit,c);    }
    
    static int fix_str(it_t * pit, const ch_t * s)
    {   return read_fix_str(pit,s); }
    
                    template<typename class_t>  
    static int charclass(it_t * pit, const class_t & is)
    {   return read_charclass(pit,is);  }
    
                    template<typename class_t, typename str_t> 
    static int charclass(it_t * pit, const class_t & is, str_t * pstr)
    {   return read_charclass(pit,is,pstr); }
    
                    template<typename class_t>  
    static int charclass_c(it_t * pit, const class_t & is, ch_t * pch)
    {   return read_charclass_c(pit,is,pch);    }
    
    static int until_char(it_t * pit, ch_t ch)
    {   return read_until_char(pit,ch); }
    
                    template<typename class_t>  
    static int until_charclass(it_t * pit, const class_t & is)
    {   return read_until_charclass(pit,is);    }
    
                    template<typename class_t>  
    static int while_charclass(it_t * pit, const class_t & is)
    {   return read_while_charclass(pit,is);    }
    
                    template<typename str_t>    
    static int until_char(it_t * pit, ch_t ch, str_t * pstr)
    {   return read_until_char(pit,ch,pstr);    }
    
                    template<typename class_t, typename str_t> 
    static int until_charclass(it_t * pit, const class_t & is, str_t * pstr)
    {   return read_until_charclass(pit,is,pstr);   }
    
                    template<typename class_t, typename str_t> 
    static int while_charclass(it_t * pit, const class_t & is, str_t * pstr)
    {   return read_while_charclass(pit,is,pstr);   }
    
    static int until_str(it_t * pit, const ch_t * s)
    {   return read_until_str(pit,s);   }
    
                    template<typename str_t>    
    static int until_str(it_t * pit, const ch_t * s, str_t * pstr)
    {   return read_until_str(pit,s,pstr);  }
    
                    template<typename pattern_t>
    static int until_pattern(it_t * pit, const pattern_t & read_pattern)
    {   return read_until_pattern(pit,read_pattern);    }
    
                    template<typename pattern_t, typename str_t> 
    static int until_pattern(it_t * pit, const pattern_t & read_pattern, str_t * pstr)
    {   return read_until_pattern(pit,read_pattern,pstr);   }
    
                    template<typename pattern_t, typename str_t, typename rez_t> 
    static int until_pattern(it_t * pit, const pattern_t & read_pattern, str_t * pstr, rez_t * rez)
    {   return read_until_pattern(pit,read_pattern,pstr,rez);   }
    
    static int spc(it_t * it)
    {   return read_spc(it);    }
    
    static int spcs(it_t * it)
    {   return read_spcs(it);   }
    
                    template<typename int_t>    
    static int _int(it_t * pit, int ss, int_t * prez)
    {   read_int(pit,ss,prez);  }
    
                    template<typename int_t>    
    static int dec(it_t * pit, int_t * prez)
    {   read_dec(pit,prez);     }
    
                    template<typename int_t>    
    static int hex(it_t * pit, int_t * prez)
    {   read_hex(pit,prez);     }
    
                    template<typename int_t>    
    static int oct(it_t * pit, int_t * prez)
    {   read_oct(pit,prez);     }
    
                    template<typename int_t>    
    static int bin(it_t * pit, int_t * prez)
    {   read_bin(pit,prez);     }
    
                    template<typename flt_t>    
    static int cfloat(it_t * pit, flt_t * prez)
    ;
    
                    template<typename flt_t>    
    static int rus_cfloat(it_t * pit, flt_t * prez)
    ;
    
                    template<typename flt_t>    
    static int ifloat(it_t * pit, flt_t * prez)
    ;
    
                    template<typename flt_t>    
    static int rus_ifloat(it_t * pit, flt_t * prez)
    ;
};//class basic_read<sh_t,it_t>
    //тут я планирую написать и специализировать строковые функции для разных типов строк
    //они понадобятся при оптимизации функций из read::
}//namespace str
#endif //BASE_PARSE_H
