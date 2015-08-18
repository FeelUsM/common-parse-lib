#define onesource
#include "pin.h"

using namespace str;

template<class it_t>
const char * read_sum(it_t * pit, double * prez);

//выр::=(spcs'('сумма spcs')'|число)
template<class it_t>
const char * read_expr(it_t * pit, double * prez){
	const char * err;
	r_if(read_s_fix_char(pit,'(')){
		r_ifnot(err = read_sum(pit,prez))
			return err;
		r_ifnot(read_s_fix_char(pit,')'))
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

//множ::=выр (spcs('*'|'/')выр)*
template<class it_t>
const char * read_mul(it_t * pit, double * prez){
	const char * err;
	r_ifnot(err = read_expr(pit,prez))
		return err;
	while(true){
		int zn;
		r_if(read_s_fix_char(pit,'*')){
			zn=1;
			goto met;
		}
		else r_if(read_s_fix_char(pit,'/')){
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

//сумма::=множ (spcs('+'|'-')множ)*
template<class it_t>
const char * read_sum(it_t * pit, double * prez){
	const char * err;
	r_ifnot(err = read_mul(pit,prez))
		return err;
	while(true){
		int zn;
		r_if(read_s_fix_char(pit,'+')){
			zn = 1;
			goto met;
		}
		else r_if(read_s_fix_char(pit,'-')){
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
	using namespace std;
	{
		string str_expression_1 = "5+84/(51)";
		string str_expression_2 = "15 + (16 /6 *4)";
		string str_expression = str_expression_2;
		cout <<"вычисляем выражение:" <<endl <<str_expression <<endl;
		const char * p= str_expression.c_str();
		const char * err;
		double rez;
		r_ifnot(err=read_sum(&p,&rez)){
			int pos=p-str_expression.c_str();
			for(int i=1; i<pos; i++)
				cout <<' ';
			cout <<'^' <<endl;
			printf("произошла ошибка: %s\n",err);
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
			const char * err;
			double rez;
			r_ifnot(err=read_sum(pin,&rez)){
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
