/*
 * преобразует файл реестра так, чтобы 
 * на одной строке находился путь и имя значения
 * а на другой строке - данные
 * - для удобного поиска изменений при помощи утилиты diff
 */

#include <iostream>
#include "../forward_stream.h"
#include "../base_parse.h"
#define one_source
#include "../strin.h"

using namespace str;
using std::cerr;//для дебага
using std::cout;
using std::endl;

#define read_fix_str_EXPECTED(mes,it,str) read_fix_str(it,str)>>str_error(mes "Ожидалась строка '" str "'")
#define read_fix_str_s_EXPECTED(mes,it,str,s) read_fix_str(it,str,s)>>str_error(mes "Ожидалась строка '" str "'")
//#define read_fix_char_EXPECTED(it,str) read_fix_char(it,str[0])>>str_error("Ожидался символ '" str "'")
//#define read_fix_char_s_EXPECTED(it,str,s) read_fix_char(it,str[0],s)>>str_error("Ожидался символ '" str "'")
#define ifnot(expr)	if(!(expr))

template <class it_t, class mes_t>
void dump(it_t tmp, mes_t mes){
	string s;
	read_fix_length(tmp,50,&s);
	cout <<mes <<dump(s.c_str()) <<endl;
	//это не рекурсия, это в forward_stream.h определено для дебага, и здесь тоже пригодилось
}

template <class it_t>
	str_error 
read_dir(it_t & it, string * ps){
	ps->clear();
	RETURN_IFNOT(read_fix_char(it,'[')>>str_error("Ожидался символ '['"));
		//возможно экранирование...
	RETURN_IFNOT(read_until_str(it,"]\r\n",ps)>>str_error("неожиданный конец файла"));
	return 0;
}

template <class it_t>
	str_error 
read_file(it_t & it, string * ps){
	ps->clear();
	ifnot(read_fix_char(it,'@',ps)){
		RETURN_IFNOT(read_fix_char(it,'\"',ps)>>str_error("Ожидался символ '\"'"));
		
		while(true){
			char c;
			RETURN_IFNOT(read_until_charclass(it,span("\"\\"),ps)>>str_error("неожиданный конец файла"));
			RETURN_IFNOT(read_ch(it,&c,ps)>>str_error("неожиданный конец файла"));
			if(c=='"')	break;
			else{
				RETURN_IFNOT(read_ch(it,&c,ps)>>str_error("неожиданный конец файла"));
				switch(c){
					case'"': 
					case'\\': 	break;
					default: 	return "неизвестный экранируемый символ";
				}
			}
		}
	}
	RETURN_IFNOT(read_fix_char(it,'=')>>str_error("Ожидался символ '='"));
	return 0;
}

template <class it_t>
	str_error 
read_string(it_t & it, string * ps){
	ps->clear();
	RETURN_IFNOT(read_fix_char(it,'\"',ps)>>str_error("Ожидался символ '\"'"));
	while(true){
		char c;
		RETURN_IFNOT(read_until_charclass(it,span("\"\\"),ps)>>str_error("неожиданный конец файла"));
		RETURN_IFNOT(read_ch(it,&c,ps)>>str_error("неожиданный конец файла"));
		if(c=='"')	break;
		else{
			RETURN_IFNOT(read_ch(it,&c,ps)>>str_error("неожиданный конец файла"));
			switch(c){
				case'"': 
				case'\\': 	break;
				default: 	return "неизвестный экранируемый символ";
			}
		}
	}
	RETURN_IFNOT(read_fix_str_EXPECTED("read_string, after this: ",it,"\r\n"))
	return 0;
}

template <class it_t>
	str_error 
read_hex(it_t & it, string * ps){
	ps->clear();
	RETURN_IFNOT(read_fix_str_s_EXPECTED("read_hex: ",it,"hex",ps));
	if(read_fix_char(it,'(',ps)){
		RETURN_IFNOT(read_charclass_s(it,spn_xdigit,ps)>>str_error("ожидалась шестначцатеричная цифра"));
		//[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\v2.0.50727\NGenService\State]
		//"LastSuccess"=hex(b):6e,fe,f7,58,df,ff,d1,08
		while(read_charclass_s(it,spn_xdigit,ps))
			;
		//[HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Class\{4d36e96e-e325-11ce-bfc1-08002be10318}\Configuration\Driver\MODES]
		//@=hex(200000):
		RETURN_IFNOT(read_fix_char(it,')',ps)>>str_error("Ожидался символ ')'"));
	}
	RETURN_IFNOT(read_fix_char(it,':',ps)>>str_error("Ожидался символ ':'"));
	if(read_fix_str(it,"\r\n"))
		return 0;
	do{
		if(read_fix_char(it,'\\')){
			RETURN_IFNOT(read_fix_str_EXPECTED("read_hex, \\\\r\\n: ",it,"\r\n"));
			read_blanks(it);
		}
		RETURN_IFNOT((read_charclass_s(it,spn_xdigit,ps)&&E2F(read_charclass_s(it,spn_xdigit,ps)))
			>>str_error("ожидалась шестнадцатеричная цифра"));
	}while(read_fix_char(it,',',ps));
	RETURN_IFNOT(read_fix_str_EXPECTED("read_hex, after this: ",it,"\r\n"));
	return 0;
}

template <class it_t>
	str_error 
read_dword(it_t & it, string * ps){
	ps->clear();
	RETURN_IFNOT(read_fix_str_s_EXPECTED("read_dword: ",it,"dword:",ps));
	for(int i=0; i<8; i++)
		RETURN_IFNOT(read_charclass_s(it,spn_xdigit,ps) >>str_error("ожидалась шестнадцатеричная цифра"));
	RETURN_IFNOT(read_fix_str_EXPECTED("read_dword, after this: ", it,"\r\n"));
	return 0;
}

template <class it_t>
	strpos_error 
read_content(it_t & it, string * ps){
	ps->clear();
	
	return strpos_error("не удалось прочитать ни string ни hex ни dword",linecol(it)).set_ignored(it)
			//инициализация игнорируемой позиции
		||reifnot_E2F(it,read_string(it,ps) >>&(it))
		||reifnot_E2F(it,::read_hex(it,ps)	>>&(it))
		||reifnot_E2F(it,read_dword(it,ps)	>>&(it))
	;//складываение типов ошибок
	/*
	//	cerr <<"after read_string reifnot "<<(bool)err <<(err?"":err.what()) <<" at position "<<err.where() <<endl;
	auto err= strpos_error("не удалось прочитать ни string ни hex ни dword",linecol(it)).set_ignored(it);
		//инициализация игнорируемой позиции
		cerr <<"after err init "<<err<<endl<<endl;
	//err = err||reifnot_E2F(it,read_string(it,ps) 	>>strpos_error(it));
	auto err2 = read_string(it,ps) 	>>&(it);
		cerr <<"after read_string "<<err2<<endl;
	err = err||reifnot_E2F(it,err2);
		cerr <<"after read_string reifnot "<<err<<endl<<"-------------"<<endl<<endl;
	//err = err||reifnot_E2F(it,((::read_hex(it,ps)	>>strpos_error(it))?strpos_error(true):strpos_error("HEX BUG",it)));
	//auto err3 = ::read_hex(it,ps)	>>&(it);
	//	cerr <<"after read_hex "<<err3<<endl;
	err = err||reifnot_E2F(it,::read_hex(it,ps)	>>&(it));
		cerr <<"after read_hex reifnot "<<err<<endl<<"-------------"<<endl<<endl;
	err = err||reifnot_E2F(it,read_dword(it,ps)	>>&(it));
		cerr <<"after read_dword reifnot "<<err<<endl<<endl;
	return err;
	*/
}

template <class it_t>
	strpos_error 
read_all_reg(it_t & it){
	RETURN_IFNOT(read_fix_str_EXPECTED("start reading: ",it,"Windows Registry Editor Version 5.00\r\n\r\n")>>&(it));
	string dir;
	while(!atend(it)){
		RETURN_IFNOT(read_dir(it,&dir)>>&(it))
		string file,content;
		while(read_file(it,&file)){
			RETURN_IFNOT(read_content(it,&content))
			cout<< dir <<'\\' <<file <<endl 
				<<content <<endl 
				<<endl;
		}
		RETURN_IFNOT(read_fix_str_EXPECTED("after files and values: ",it,"\r\n")>>&(it))
	}
	return 0;
}
//=======================================================================
//=======================================================================
#if 0
int main()
{
	string s;
	RETURN_IFNOT(read_fix_str_EXPECTED(strin,"Windows Registry Editor Version 5.00\r\n\r\n")>>strpos_error(strin))
	read_all_reg_2(strin);
	str_error x;
	x>>strpos_error(strin);
	RETURN_IFNOT(read_dir(strin,&s)>>strpos_error(strin))
}
#endif
#if 1
//делать неиспользуемые копии итераторов не нужно, это препятствует выгрузке отработанных (прочитанных и разоранных) буферов

//можно делать указатели на итераторы
int main2(int argc, const char * argv[]){
	typename forward_adressed_stream::iterator * pit;
	forward_adressed_stream * ps;
	if(argc==2){
		ps = new forward_adressed_stream(true, new file_on_FILE(argv[1],"r"));
			//true - говорит о том, что поток сам удалит этот файл
		if(!*ps)	throw "не смог открыть файл";
		pit = &ps->iter();						
	}
	else{
		pit = &strin;
		start_read_line(*pit);
	}
	strpos_error err;
	//dump(*pit,"dump:\n");
	ifnot(err=read_all_reg(*pit)){
		std::cerr << "на позиции " <<err.where() 
			<<" произошла ошибка: " <<err.what() <<endl;
		dump(*pit,"");
		return 1;
	}
	if(argc==2)
		delete ps;
	return 0;
}

//можно делать ссылки на итераторы, но это удобнее при вызове отдельной функции
int submain(typename forward_adressed_stream::iterator & it){
	strpos_error err;
	//dump(it,"dump:\n");
	ifnot(err=read_all_reg(it)){
		std::cerr << "на позиции " <<err.where()
			<<" произошла ошибка: " <<dump(err.what()) <<endl;
		dump(it,"");
		return 1;
	}
	return 0;
}
int main3(int argc, const char * argv[]){
	if(argc==2){
		forward_adressed_stream stream(true, new file_on_FILE(argv[1],"r"));
										//true - говорит о том, что поток сам удалит этот файл
		if(!stream)	{
			std::cerr<<"не смог открыть файл"<<endl;
			return 2;
		}
		return submain(stream.iter());
	}
	else{
		start_read_line(strin);
		return submain(strin);
	}
	return 0;
}

int main(int argc, const char * argv[]){
	if(argc>2){
		std::cerr << "должно быть 0 или 1 аргументов";
		return 2;
	}
	return main3(argc,argv);
}
#endif
