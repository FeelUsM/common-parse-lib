#include <string>
#include <stdio.h>
#include "base_parse.h"

#define r_ifnot(expr) if(expr)
#define r_whilenot(expr) while(expr)
//int r_ifnot;
#define a b;
int a;


template<typename it_t, typename str_t>
const char * read_identify(it_t * pit, str_t * pstr){
	typedef typename str_t::value_type ch_t;
	ch_t ch;
						printf("read_identify: before read::charclass(alpha)\n");
	//r_ifnot
	if(read::charclass(pit,is<ch_t>::alpha,&ch))
		return "ожидался символ";
	(*pstr)+=ch;
						//printf("read_identify: before read::while_charclass(alnum)\n");
	str_t s1;
	read::while_charclass(pit,is<ch_t>::alnum,&s1);
	(*pstr)+=s1;
	return 0;
}

template<typename it_t, typename str_t>
const char * read_tag(it_t * pit, str_t * pstr){
						//printf("read_tag: before read::fix_char('<')\n");
	//r_ifnot
	if(read::fix_char(pit,'<'))
		return "ожидался символ '<'";
						//printf("read_tag: before read_identify()\n");
	const char * error;
	//r_ifnot
	if(error=read_identify(pit,pstr))
		return error;
						//printf("read_tag: before read::fix_char('>')\n");
	//r_ifnot
	if(read::fix_char(pit,'>'))
		return "ожидался символ '>'";
}

int main()
{
	char s[]="<sdfghj>";
	const char * p=s;
	std::string str;
	
	read_tag(&p,&str);
	
	printf("%s\n",str.c_str());
}