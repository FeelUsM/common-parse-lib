#include <iostream>
#include <string>
#include "stream_string.h"
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

namespace calc{
	template<class it_t>
	const char * read_sum(it_t * pit, double * prez);

	//выр::=spcs('('сумма spcs')'|число)
	template<class it_t>
	const char * read_expr(it_t * pit, double * prez){
		const char * err;
		read_spcs(pit);
		r_if(read_fix_char(pit,'(')){
			r_ifnot(err = read_sum(pit,prez))
				return err;
			read_spcs(pit);
			r_ifnot(read_fix_char(pit,')'))
				return "ожидалась закрывающая скобка";
			return 0;
		}
		int x;
		int errc;
		r_ifnot(errc=read_dec(pit,&x)){
			return "ожидалось число";
		}
		*prez = x;
		return 0;
	}

	//множ::=выр spcs(('*'|'/')выр)*
	template<class it_t>
	const char * read_mul(it_t * pit, double * prez){
		const char * err;
		r_ifnot(err = read_expr(pit,prez))
			return err;
		read_spcs(pit);
		while(true){
			int zn;
			r_if(read_fix_char(pit,'*')){
				zn=1;
				goto met;
			}
			else r_if(read_fix_char(pit,'/')){
				zn=0;
				goto met;
			}
			return 0;
		met:
			double x;
			r_ifnot(err=read_expr(pit,&x))
				return err;
			if(zn)	*prez *= x;
			else	*prez /= x;
		}
		return 0;//ни когда не выполниться
	}

	//сумма::=множ spcs(('+'|'-')множ)*
	template<class it_t>
	const char * read_sum(it_t * pit, double * prez){
		const char * err;
		r_ifnot(err = read_mul(pit,prez))
			return err;
		read_spcs(pit);
		while(true){
			int zn;
			r_if(read_fix_char(pit,'+')){
				zn = 1;
				goto met;
			}
			else r_if(read_fix_char(pit,'-')){
				zn = -1;
				goto met;
			}
			return 0;
		met:
			double x;
			r_ifnot(err = read_mul(pit,&x))
				return err;
			*prez += x*zn;
		}
		return 0;//ни когда не выполниться
	}
}//namespace calc
	
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
	
	{
		string str_expression="5+84/(51)";
		cout << "вычисляем выражение " << str_expression << endl;
		const char * p= str_expression.c_str();
		const char * err;
		double rez;
		r_ifnot(err=calc::read_sum(&p,&rez)){
			printf("на позиции %d произошла ошибка: %s\n",p-str_expression.c_str(),err);
			return -1;
		}
		cout << "результат: " << rez << endl;
	}
	
	{
		cout <<"================" << endl;
		try{
			//stream_string<basic_simple_buffer<char,string_file_FILE,200>>::iterator x;
			string_file_on_FILE file(stdin);
			/*
			 * при создании stream_string создается внутренний итератор, а значит создается первый буфер
			 * а значит вы должны ввести хотя бы одну строку
			 */
			stream_string<basic_simple_buffer<char,string_file_on_FILE,200>> str(&file);
			const char * err;
			double rez;
			r_ifnot(err=calc::read_sum(str.pinternal_iterator(),&rez)){
				printf("на позиции --- произошла ошибка: %s\n",err);
				return -1;
			}
			cout << "результат: " << rez << endl;
			
		}
		catch(const char * mes){
			cerr << "ошибка: " << mes << endl;
			return -1;
		}
		catch(string mes){
			cerr << "ошибка: " << mes << endl;
			return -1;
		}
		catch(...){
			cerr << "неизвестная ошибка" << endl;
		}
	}

	return 0;
}
