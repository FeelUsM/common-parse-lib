#include <string>
#include <stdio.h>
#include "base_parse.h"

using namespace str;

template<typename it_t, typename str_t>
struct example_read{
	typedef typename str_t::value_type ch_t;
	typedef basic_read<ch_t,it_t> read;

	static
	const char * 
	identify(it_t * pit, str_t * pstr){
		//typedef 
		r_ifnot(read_charclass(pit,spn_alpha<ch_t>(),pstr))
			return "ожидалась буква";
		read_while_charclass(pit,spn_alnum<ch_t>(),pstr);
			//EOF или нет - игнорируем
		return 0;
	}

	static
	const char * 
	tag(it_t * pit, str_t * pstr){
		r_ifnot(read::fix_char(pit,'<'))
			return "ожидался символ '<'";
		const char * error;
		r_ifnot(error=identify(pit,pstr))
			return error;
		r_ifnot(read::fix_char(pit,'>'))
			return "ожидался символ '>'";
		return 0;
	}
};


int main()
{
	using std::string;
	typedef basic_read<char,const char*> read;
	typedef example_read<const char*,string> ex_read;

	char file_string[]="<sdfghj>";
	const char * p=file_string;
	string str;
	
	const char * err;
	r_ifnot(err=ex_read::tag(&p,&str)){
		printf("на позиции %d произошла ошибка: %s\n",p-file_string,err);
		return -1;
	}
	read::until_eof(&p);//в конце все итераторы должны дойти до канца файла
	
	printf("результат: %s\n",str.c_str());
	return 0;
}