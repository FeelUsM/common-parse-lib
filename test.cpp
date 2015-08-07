#include <string>
#include <stdio.h>
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

const char * read_sum(const char **pit, double * prez);

//выр::=spcs('('сумма spcs')'|число)
const char * read_expr(const char **pit, double * prez){
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
const char * read_mul(const char **pit, double * prez){
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
const char * read_sum(const char **pit, double * prez){
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

int main()
{
	using std::string;
	typedef basic_read<char,const char*> read;
	typedef example_read<const char*,string> ex_read;

	string file_string="<sdfghj>";
	const char * p=file_string.c_str();
	string str;
	
	const char * err;
	r_ifnot(err=ex_read::tag(&p,&str)){
		printf("на позиции %d произошла ошибка: %s\n",p-file_string.c_str(),err);
		return -1;
	}
	read::until_eof(&p);//в конце все итераторы должны дойти до канца файла
	printf("результат: %s\n",str.c_str());
	
	
	string str_expression="5+84/(51)";
	printf("вычисляем выражение %s\n",str_expression.c_str());
	p=str_expression.c_str();
	double rez;
	r_ifnot(err=read_sum(&p,&rez)){
		printf("на позиции %d произошла ошибка: %s\n",p-str_expression.c_str(),err);
		return -1;
	}
	read::until_eof(&p);//в конце все итераторы должны дойти до канца файла
	printf("результат: %f\n",rez);
	
	return 0;
}