#define one_source//что бы не линковать strin.cpp
#include "strin.h"

using namespace str;

template<class it_t>
const char * read_sum(it_t & it, double * prez);

//выр::= spcs'(' сумма spcs')' | число
template<class it_t>
const char * read_expr(it_t & it, double * prez){
	const char * err;
	r_if(read_s_fix_char(it,'(')){
		r_ifnot(err = read_sum(it,prez))
			return err;
		r_ifnot(read_s_fix_char(it,')'))
			return "ожидалась закрывающая скобка";
		return 0;
	}
	int x;
	int errc;
	r_ifnot(errc=read_dec(it,&x)){
		return "ожидалось число";
	}
	*prez = x;
	return 0;
}

//множ::=выр (spcs[*/] выр)*
template<class it_t>
const char * read_mul(it_t & it, double * prez){
	typedef typename std::iterator_traits<it_t>::value_type ch_t;
	const char * err;
	r_ifnot(err = read_expr(it,prez))
		return err;
	while(true){
		ch_t zn;
		r_ifnot(read_s_charclass(it,span<ch_t>("*/"),&zn))//не для char работать не будет
			return 0;
		double x;
		r_ifnot(err=read_expr(it,&x))
			return err;
		if(zn==(ch_t)'*')	*prez *= x;
		else				*prez /= x;
	}
	return 0;//ни когда не выполниться
}

//сумма::=множ (spcs[+-] множ)*
template<class it_t>
const char * read_sum(it_t & it, double * prez){
	typedef typename std::iterator_traits<it_t>::value_type ch_t;
	const char * err;
	r_ifnot(err = read_mul(it,prez))
		return err;
	while(true){
		ch_t zn;
		r_ifnot(read_s_charclass(it,span<ch_t>("+-"),&zn))
			return 0;
		double x;
		r_ifnot(err = read_mul(it,&x))
			return err;
		if(zn==(ch_t)'+')	*prez += x;
		else				*prez -= x;
	}
	return 0;//ни когда не выполниться
}

int main()
{
	using namespace std;
	if(0)
	{
		string str_expression_1 = "5+84/(51)";
		string str_expression_2 = "15 + (16 /6 *4)";
		string str_expression_3 = "7 + ( 4* 4)";
		string str_expression = str_expression_3;
		cout <<"вычисляем выражение:" <<endl <<str_expression <<endl;
		const char * p= str_expression.c_str();
		const char * err;
		double rez;
		r_ifnot(err=read_sum(p,&rez)){
			int pos=p-str_expression.c_str();
			for(int i=1; i<pos; i++)
				cout <<' ';
			cout <<'^' <<endl;
			cout <<"произошла ошибка: " <<err <<endl;
			return -1;
		}
		cout << "результат: " << rez << endl;
		cout <<"================" << endl;
	}
	
	if(1)
	{
		try{
			while(true){
				cout <<"введите арифметическое выражение, заканчивающееся символами 'END'" <<endl;
				start_read_line(strin);
				cout <<"start:";
				const char * err;
				double rez;
				err=read_sum(strin,&rez);
				read_until_str(strin,"END");
				r_ifnot(err){
					cout <<"на позиции --- произошла ошибка: " <<err <<endl;
					return -1;
				}
				cout << "результат: " << rez << endl;
			}
		}
		catch(const char * mes){
			cerr << "ошибка: " << mes << endl;
			return -1;
		}
		catch(string & mes){
			cerr << "ошибка: " << mes << endl;
			return -1;
		}
		catch(stream_exception & mes){
			cerr << "ошибка: " << mes.what() << endl;
			return -1;
		}
		catch(...){
			cerr << "неизвестная ошибка" << endl;
		}
	}

	return 0;
}
