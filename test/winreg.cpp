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
using std::cout;
using std::cerr;
using std::endl;

#define r_if(expr)             if((expr)==0)
#define r_while(expr)       while((expr)==0)
#define r_ifnot(expr)           if(expr)
#define r_whilenot(expr)     while(expr)

template <class it_t>
void dump(it_t tmp, const char * mes){
	string s;
	read_fix_length(tmp,50,&s);
	cerr <<mes <<dump(s.c_str()) <<endl;
	//это не рекурсия, это в forward_stream.h определено для дебага, и здесь тоже пригодилось
}

template <class it_t>
const char * read_dir(it_t & it, string * ps){
	//dump(it,"в начале read_dir:\n");
	ps->clear();
	ifnot(read_fix_char(it,'['))
		return "ожидалась [";
	ifnot(read_until_str(it,"]\r\n",ps))//возможно экранирование...
		return "неожиданный конец файла";
	return 0;
}

template <class it_t>
const char * read_file(it_t & it, string * ps){
	ps->clear();
	if(read_fix_char(it,'@'))
		*ps +='@';
	else{
		ifnot(read_fix_char(it,'"'))
			return "ожидалось \"";
		*ps +='"';
		while(true){
			int err;
			ifnot(err=read_until_charclass(it,span("\"\\"),ps))
				return "неожиданный конец файла";
			char c;
			ifnot(err=read_c(it,&c))
				return "неожиданный конец файла";
			if(c=='"'){
				*ps +='"';
				break;
			}
			else{//c=='\\'
				*ps +='\\';
				ifnot(read_c(it,&c))
					return "неожиданный конец файла";
				switch(c){
					case'"': *ps +='"'; break;
					case'\\': *ps +='\\'; break;
					default: return "неизвестный экранируемый символ";
				}
			}
		}
		*ps +='"';
	}
	ifnot(read_fix_char(it,'='))
		return "ожидался =";
	return 0;
}

template <class it_t>
const char * read_string(it_t & it, string * ps){
	ps->clear();
	ifnot(read_fix_char(it,'"'))
		return "ожидалось \"";
	*ps +='"';
	while(true){
		int err;
		ifnot(err=read_until_charclass(it,span("\"\\"),ps))
			return "неожиданный конец файла";
		//cerr <<"read_until_charclass = " <<err <<endl;
		//cerr << ps->size() <<endl;
		//dump(it,"перед read_c:\n");
		char c;
		ifnot(err=read_c(it,&c)){
			//cerr <<"read_c = " <<err <<endl;
			return "неожиданный конец файла";
		}
		if(c=='"'){
			*ps +='"';
			break;
		}
		else{//c=='\\'
			*ps +='\\';
			ifnot(read_c(it,&c))
				return "неожиданный конец файла";
			switch(c){
				case'"': *ps +='"'; break;
				case'\\': *ps +='\\'; break;
				default: return "неизвестный экранируемый символ";
			}
		}
	}
	*ps +='"';
	//dump(it,"перед ожидался перевод строки:\n");
	ifnot(read_fix_str(it,"\r\n"))
		return "ожидался перевод строки";
	return 0;
}

template <class it_t>
const char * read_hex(it_t & it, string * ps){
	ps->clear();
	ifnot(read_fix_str(it,"hex"))
		return "ожидалась строка 'hex'";
	*ps += "hex";
	if(read_fix_char(it,'(')){
		*ps +='(';
		ifnot(read_charclass_s(it,spn_xdigit,ps))
			return "ожидалась шестначцатеричная цифра";
		//[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\v2.0.50727\NGenService\State]
		//"LastSuccess"=hex(b):6e,fe,f7,58,df,ff,d1,08
		while(read_charclass_s(it,spn_xdigit,ps))
			;
		//[HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Class\{4d36e96e-e325-11ce-bfc1-08002be10318}\Configuration\Driver\MODES]
		//@=hex(200000):
		ifnot(read_fix_char(it,')'))
			return "ожидалась )";
		*ps +=')';
	}
	ifnot(read_fix_char(it,':'))
		return "ожидалось :";
	*ps +=':';
	if(read_fix_str(it,"\r\n"))
		return 0;
	do{
		if(read_fix_char(it,'\\')){
			ifnot(read_fix_str(it,"\r\n"))
				return "ожидался перевод строки";
			read_blanks(it);
		}
		ifnot(read_charclass_s(it,spn_xdigit,ps))
			return "ожидалась шестнадцатеричная цифра";
		ifnot(read_charclass_s(it,spn_xdigit,ps))
			return "ожидалась шестнадцатеричная цифра";
	}while(read_charclass_s(it,span(","),ps));//read_fix_char_s нету, а read_charclass_s есть
	ifnot(read_fix_str(it,"\r\n"))
		return "ожидался перевод строки";
	return 0;
}

template <class it_t>
const char * read_dword(it_t & it, string * ps){
	ps->clear();
	ifnot(read_fix_str(it,"dword:"))
		return "ожидалась строка 'dword:'";
	*ps += "dword:";
	for(int i=0; i<8; i++)
		ifnot(read_charclass_s(it,spn_xdigit,ps))
			return "ожидалась шестнадцатеричная цифра";
	ifnot(read_fix_str(it,"\r\n"))
		return "ожидался перевод строки";
	return 0;
}

template <class it_t>
const char * read_content(it_t & it, string * ps){
	ps->clear();
	it_t prom_it = it;
	const char * err1, * err2, *err3;;
	r_if(err1=read_string(it,ps))
		return 0;
	auto it1=it;
	r_if(err2=::read_hex(it=prom_it,ps))
		return 0;
	auto it2=it;
	r_if(err3=read_dword(it=prom_it,ps))
		return 0;
	auto it3=it;
	it=prom_it;
	
	cerr << "на позиции " <<get_linecol(it1) 
		<<" произошла ошибка: " <<err1 <<endl;
	cerr << "на позиции " <<get_linecol(it2) 
		<<" произошла ошибка: " <<err2 <<endl;
	cerr << "на позиции " <<get_linecol(it3) 
		<<" произошла ошибка: " <<err3 <<endl;
	dump(it,"");
	
	return "не удалось прочитать ни string ни hex ни dword";
}

template <class it_t>
const char * read_all_reg(it_t & it){
	ifnot(read_fix_str(it,"Windows Registry Editor Version 5.00\r\n\r\n"))
		return "ожидалась строка: Windows Registry Editor Version 5.00";
	const char * err;
	string dir;
	while(!atend(it)){
		r_ifnot(err=read_dir(it,&dir))
			return err;
		string file,content;
		//it_t prom_it = it;
		r_while(err=read_file(it,&file)){
			r_ifnot(err=read_content(it,&content))
				return err;
			//prom_it = it;
			cout<< dir <<'\\' <<file <<endl 
				<<content <<endl 
				<<endl;
		}
		ifnot(read_fix_str(it,"\r\n"))
			return err;
	}
	return 0;
}

/*
 * Здесь обнаруживается и решается еще одна проблема:
 * все функции параметризуются итераторами, а итераторы - буферами
 * и для двух разных типов буферов будут генерироваться абсолютно разные наборы всех парсинговых функций
 * в то время как если буферы один наследуется от другого, или различаются типом файла, 
 * то операции с итераторами будут абсолютно одинаковыми за исключением конструирования буфера
 * вывод: для уменьшения кол-ва генерируемого кода надо файлы наследовать от интерфейса
 * т.е. файл больше не будет параметром буфера
 */

//делать неиспользуемые копии итераторов не нужно, это препятствует выгрузке отработанных (прочитанных и разоранных) буферов

//можно делать указатели на итераторы
int main2(int argc, const char * argv[]){
	typename forward_adressed_stream::iterator * pit;
	forward_adressed_stream * ps;
	if(argc==2){
		ps = new forward_adressed_stream(true, new file_on_FILE_block(argv[1],"r"));
		if(!*ps)	throw "не смог открыть файл";
		pit = &ps->iter();						//true - говорит о том, что поток сам удалит этот файл
	}
	else{
		pit = &strin;
		start_read_line(*pit);
	}
	const char * err;
	//dump(*pit,"dump:\n");
	r_ifnot(err=read_all_reg(*pit)){
		cerr << "на позиции " <<get_linecol(*pit) 
			<<" произошла ошибка: " <<err <<endl;
		dump(*pit,"");
		return 1;
	}
	if(argc==2)
		delete ps;
	return 0;
}

//можно делать ссылки на итераторы, но это удобнее при вызове отдельной функции
int submain(typename forward_adressed_stream::iterator & it){
	const char * err;
	//dump(it,"dump:\n");
	r_ifnot(err=read_all_reg(it)){
		cerr << "на позиции " <<get_linecol(it) 
			<<" произошла ошибка: " <<err <<endl;
		dump(it,"");
		return 1;
	}
	return 0;
}
int main3(int argc, const char * argv[]){
	if(argc==2){
		forward_adressed_stream stream(true, new file_on_FILE_block(argv[1],"r"));
		if(!stream)	throw "не смог открыть файл";
												//true - говорит о том, что поток сам удалит этот файл
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
		cerr << "должно быть 0 или 1 аргументов";
		return 2;
	}
	return main3(argc,argv);
}
