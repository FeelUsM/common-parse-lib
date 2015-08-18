#include <iostream>
#include <string>
#include "forward_stream.h"
#include "base_parse.h"

using namespace str;
using namespace std;

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

template<typename it_t, typename str_t>
struct xml_parse{
	typedef typename str_t::value_type ch_t;
	typedef basic_read<ch_t,it_t> read;

	static
	const char *
	xml(it_t * pit)
	{
		const char * err;
		r_ifnot(err=DTD(pit))
			return err;
		r_if(err=elem_tag(pit))
			return 0;
		else r_if(err=elem_tag_last(pit))
			return 0;
		return err;
	}

	static
	const char *
	DTD(it_t * pit)
	{
		char str[] = "<!xml>";
		const char* cstr = str;
		r_ifnot(read_fix_str(pit, cstr))
			return "некорректный DTD";
		return 0;
	}

	static
	const char *
	elem_tag(it_t * pit)
	{
		return 0;
	}

	static
	const char *
	elem_tag_last(it_t * pit)
	{
		return 0;
	}

};

int main()
{
	{
		typedef xml_parse<const char*,string> parse;
		string xml_doc = "<!xml>";
		const char* cstr = xml_doc.c_str();
		string str_out;
		
		const char* err_out;
	
		r_ifnot(err_out = parse::xml(&cstr))
			cout << "ошибка: " << err_out << endl;
		else cout << "распарсил xml_doc" << endl;
	}
	
	{
		typedef example_read<const char*,string> ex_read;

		string file_string="<sdfghj>";
		const char * p= file_string.c_str();
		string str;
	
		const char * err;
		r_ifnot(err=ex_read::tag(&p,&str)){
			cout << "на позиции " << p-file_string.c_str() << " произошла ошибка: " << err << endl;
			return -1;
		}
		cout << "результат: " << str << endl;
	}
	
	return 0;
}
