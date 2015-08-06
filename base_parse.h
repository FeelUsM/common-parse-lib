//FeelUs
#ifndef BASE_PARSE_H
#define BASE_PARSE_H

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
 * === стандартные КЛАССЫ СИМВОЛОВ ===
 * этот "класс" как char_traits, на самом деле не класс а шаблонный namespace
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
bool atend(ch_t * pc)
{   return !*pc;    }

/*
функции возвращают код ошибки
при успешном прочтении итератор указывает на следующий символ после последнего прочитанного
всё считанное ДОБАВЛЯЕТСЯ в строку, если таковая указана
при неудачном прочтении итератор указывает на место ошибки

size_t      n
ch_t        c
ch_t *      s
func_obj    bool cf(c)  //наподобие bool isspace(c)
func_obj    err pf(it*) //наподобие int read::spc(it*)
func_obj    err pf(it*, rez*)

len - кол-во символов, добавлненных в *pstr
                                                                возвращаемое значение в случае  реализованность
название                    аргументы           рег.выр.        если EOF    если не EOF     статистика использования
int read::until_eof         (it*)               .*$             0           0               1   OK
int read::until_eof         (it*,    pstr*)     .*$             len         len                 OK

int read::fix_length        (it*, n)            .{n}            -1          0                   OK
int read::fix_length        (it*, n, pstr*)     .{n}            -(1+len)    0                   OK
int read::fix_char          (it*, c)            c               -1          0 или 1         2   OK
int read::fix_str           (it*, s)            str             -1          0 или 1         1   OK
int read::charclass         (it*, cf)           [ ]             -1          0 или 1             OK
int read::charclass         (it*, cf, ch*)      [ ]             -1          0 или 1             OK
int read::charclass         (it*, cf, pstr*)    [ ]             -1          0 или 1         1   OK

int read::until_char        (it*, c)            .*c             -(1+len)    len                 OK
int read::until_charclass   (it*, cf)           .*[ ]           -(1+len)    len                 OK
int read::while_charclass   (it*, cf)           [ ]*            -(1+len)    len                 OK
int read::until_char        (it*, c, pstr*)     .*c             -(1+len)    len                 OK
int read::until_charclass   (it*, cf, pstr*)    .*[ ]           -(1+len)    len                 OK
int read::while_charclass   (it*, cf, pstr*)    [ ]*            -(1+len)    len             1   OK

int read::until_str         (it*, s)            .*str           -(1+len)    len                 OK
int read::until_str         (it*, s, pstr*)     .*str           -(1+len)    len                 OK
int read::until_pattern     (it*, pf)           .*( )           -(1+len)    len                 OK
int read::until_pattern     (it*, pf, pstr*)    .*( )           -(1+len)    len                 OK
int read::until_pattern     (it*, pf, pstr*, rez*)  .*( )       -(1+len)    len                 OK

int read::spc               (it*)               [:space:]       
int read::_int              (it*, int ss, int_t*)   [0-"$(($ss-1))"]+
int read::dec               (it*, int_t*)       [0-9]+
int read::hex               (it*, int_t*)       [:xdigit:]+
int read::oct               (it*, int_t*)       [0-7]+
int read::bin               (it*, int_t*)       [01]+
int read::cfloat            (it*, flt_t*)   
int read::ifloat            (it*, flt_t*)
*/
template<typename ch_t, typename it_t>
struct basic_read{
//======================= until_eof
    /*
     * until_eof(it_t * pit)
     * передвигает указатель в конец файла
     *
     * всегда возвращает 0
     */
    static
    int 
    until_eof(it_t * pit){
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
    template<typename str_t> static 
    int 
    until_eof(it_t * pit, str_t * pstr){
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
    static
    int 
    fix_length(it_t * pit, size_t n){
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
    template<typename str_t> static 
    int 
    fix_length(it_t * pit, size_t n, str_t * pstr){
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
    static
    int 
    fix_char(it_t * pit, ch_t c){
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
    static
    int 
    fix_str(it_t * pit, const ch_t * s){
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
    template<typename class_t> static
    int 
    charclass(it_t * pit, const class_t & is){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*pit)++;
        return 0;
    }
    
    /*
     * charclass(it_t * pit, const class_t & is, ch_t * ch)
     * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
     *
     * если итератор в состоянии atend(), возвращает -1
     * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
     */
    template<typename class_t> static
    int 
    charclass(it_t * pit, const class_t & is, ch_t * ch){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*ch)=**pit;
        (*pit)++;
        return 0;
    }
    
    /*
     * charclass(it_t * pit, const class_t & is, pstr_t * pstr)
     * если is(.) возвращает true от указываемого итератором символа, то считывает 1 символ
     *
     * если итератор в состоянии atend(), возвращает -1
     * если is(.) возвращает false от указываемого итератором символа, то возвращает 1, и символ не считывает
     */
    template<typename class_t, typename str_t> static
    int 
    charclass(it_t * pit, const class_t & is, str_t * pstr){
        if(atend(*pit)) return -1;
        if(!is(**pit))  return 1;
        (*pstr)+=**pit;
        (*pit)++;
        return 0;
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
    static
    int 
    until_char(it_t * pit, ch_t ch){
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

    template<typename class_t> static
    int 
    until_charclass(it_t * pit, const class_t & is){
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
    
    template<typename class_t> static
    int 
    while_charclass(it_t * pit, const class_t & is){
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
    template<typename str_t> static
    int 
    until_char(it_t * pit, ch_t ch, str_t * pstr){
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
    
    template<typename class_t, typename str_t> static
    int 
    until_charclass(it_t * pit, const class_t & is, str_t * pstr){
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

    template<typename class_t, typename str_t> static
    int 
    while_charclass(it_t * pit, const class_t & is, str_t * pstr){
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
    static
    int 
    until_str(it_t * pit, const ch_t * s){
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

    template<typename str_t> static
    int 
    until_str(it_t * pit, const ch_t * s, str_t * pstr){
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
    template<typename pattern_t> static
    int 
    until_pattern(it_t * pit, const pattern_t & read_pattern){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit))
                return i;
        }
        return -(1+i);
    }
    
    template<typename pattern_t, typename str_t> static
    int 
    until_pattern(it_t * pit, const pattern_t & pattern, str_t * pstr){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit))
                return i;
            (*pstr)=**pit;
        }
        return -(1+i);
    }

    template<typename pattern_t, typename str_t, typename rez_t> static
    int 
    until_pattern(it_t * pit, const pattern_t & pattern, str_t * pstr, rez_t * rez){
        int i=0;
        for(;!atend(*pit);(*pit)++, i++){
            it_t lit = *pit;
            r_if(read_pattern(&lit,rez))
                return i;
            (*pstr)=**pit;
        }
        return -(1+i);
    }
    
//======================= spc, _int, dec, hex, oct, bin, cfloat, ifloat, rus_cfloat, rus_ifloat

    static
    int 
    spc(it_t * it);
    
    template<typename int_t> static
    int 
    _int(it_t * pit, int ss, int_t * prez);
    
    template<typename int_t> static
    int 
    dec(it_t * pit, int_t * prez);
    
    template<typename int_t> static
    int 
    hex(it_t * pit, int_t * prez);
    
    template<typename int_t> static
    int 
    oct(it_t * pit, int_t * prez);
    
    template<typename int_t> static
    int 
    bin(it_t * pit, int_t * prez);
    
    //при чтении точка обязательна, если нет экспоненты
    template<typename flt_t> static
    int 
    cfloat(it_t * pit, flt_t * prez);
    
    //при чтении обязательна НЕ ТОЧКА А ЗАПЯТАЯ, если нет экспоненты
    template<typename flt_t> static
    int 
    rus_cfloat(it_t * pit, flt_t * prez);
    
    //int сойдет за float
    template<typename flt_t> static
    int 
    ifloat(it_t * pit, flt_t * prez);
    
    //int сойдет за float
    template<typename flt_t> static
    int 
    rus_ifloat(it_t * pit, flt_t * prez);
    
};//class basic_read<sh_t,it_t>
#endif //BASE_PARSE_H
