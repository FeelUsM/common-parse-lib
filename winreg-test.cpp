#include <iostream>
#include "forward_stream.h"
#include "base_parse.h"

using namespace str;
using std::cout;
using std::endl;

template <class it_t>
void dump(it_t tmp, const char * mes){
	string s;
	read_fix_length(tmp,50,&s);
	cout <<mes <<dump(s.c_str(),50) <<endl;
}

template <class it_t>
const char * read_dir(it_t & it, string * ps){
	//dump(it,"в начале read_dir:\n");
	ps->clear();
	r_ifnot(read_fix_char(it,'['))
		return "ожидалась [";
	rm_ifnot(read_until_str(it,"]\r\n",ps)<0)//возможно экранирование...
		return "неожиданный конец файла";
	return 0;
}

template <class it_t>
const char * read_file(it_t & it, string * ps){
	ps->clear();
	r_if(read_fix_char(it,'@'))
		*ps +='@';
	else{
		r_ifnot(read_fix_char(it,'"'))
			return "ожидалось \"";
		*ps +='"';
		while(true){
			int err;
			rm_ifnot(err=read_until_charclass(it,span<char>("\"\\"),ps)<0)
				return "неожиданный конец файла";
			char c;
			r_ifnot(err=read_c(it,&c))
				return "неожиданный конец файла";
			if(c=='"'){
				*ps +='"';
				break;
			}
			else{//c=='\\'
				*ps +='\\';
				r_ifnot(read_c(it,&c))
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
	r_ifnot(read_fix_char(it,'='))
		return "ожидался =";
	return 0;
}

template <class it_t>
const char * read_string(it_t & it, string * ps){
	ps->clear();
	r_ifnot(read_fix_char(it,'"'))
		return "ожидалось \"";
	*ps +='"';
	while(true){
		int err;
		rm_ifnot(err=read_until_charclass(it,span<char>("\"\\"),ps)<0)
			return "неожиданный конец файла";
		//cout <<"read_until_charclass = " <<err <<endl;
		//cout << ps->size() <<endl;
		//dump(it,"перед read_c:\n");
		char c;
		r_ifnot(err=read_c(it,&c)){
			//cout <<"read_c = " <<err <<endl;
			return "неожиданный конец файла";
		}
		if(c=='"'){
			*ps +='"';
			break;
		}
		else{//c=='\\'
			*ps +='\\';
			r_ifnot(read_c(it,&c))
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
	r_ifnot(read_fix_str(it,"\r\n"))
		return "ожидался перевод строки";
	return 0;
}

template <class it_t>
const char * read_hex(it_t & it, string * ps){
	ps->clear();
	r_ifnot(read_fix_str(it,"hex"))
		return "ожидалась строка 'hex'";
	*ps += "hex";
	r_if(read_fix_char(it,'(')){
		*ps +='(';
		r_ifnot(read_charclass_s(it,spn_xdigit<char>(),ps))
			return "ожидалась шестначцатеричная цифра";
		//[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework\v2.0.50727\NGenService\State]
		//"LastSuccess"=hex(b):6e,fe,f7,58,df,ff,d1,08
		r_while(read_charclass_s(it,spn_xdigit<char>(),ps))
			;
		//[HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Class\{4d36e96e-e325-11ce-bfc1-08002be10318}\Configuration\Driver\MODES]
		//@=hex(200000):
		r_ifnot(read_fix_char(it,')'))
			return "ожидалась )";
		*ps +=')';
	}
	r_ifnot(read_fix_char(it,':'))
		return "ожидалось :";
	*ps +=':';
	r_if(read_fix_str(it,"\r\n"))
		return 0;
	do{
		r_if(read_fix_char(it,'\\')){
			r_ifnot(read_fix_str(it,"\r\n"))
				return "ожидался перевод строки";
			read_blns(it);
		}
		r_ifnot(read_charclass_s(it,spn_xdigit<char>(),ps))
			return "ожидалась шестнадцатеричная цифра";
		r_ifnot(read_charclass_s(it,spn_xdigit<char>(),ps))
			return "ожидалась шестнадцатеричная цифра";
	}r_while(read_charclass_s(it,span<char>(","),ps));//read_fix_char_s нету, а read_charclass_s есть
	r_ifnot(read_fix_str(it,"\r\n"))
		return "ожидался перевод строки";
	return 0;
}

template <class it_t>
const char * read_dword(it_t & it, string * ps){
	ps->clear();
	r_ifnot(read_fix_str(it,"dword:"))
		return "ожидалась строка 'dword:'";
	*ps += "dword:";
	for(int i=0; i<8; i++)
		r_ifnot(read_charclass_s(it,spn_xdigit<char>(),ps))
			return "ожидалась шестнадцатеричная цифра";
	r_ifnot(read_fix_str(it,"\r\n"))
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
	
	cout << "на позиции " <<get_linecol(it1) 
		<<" произошла ошибка: " <<err1 <<endl;
	cout << "на позиции " <<get_linecol(it2) 
		<<" произошла ошибка: " <<err2 <<endl;
	cout << "на позиции " <<get_linecol(it3) 
		<<" произошла ошибка: " <<err3 <<endl;
	dump(it,"");
	
	return "не удалось прочитать ни string ни hex ни dword";
}

template <class it_t>
const char * read_all_reg(it_t & it){
	r_ifnot(read_fix_str(it,"Windows Registry Editor Version 5.00\r\n\r\n"))
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
			cout<<">>>" << dir <<'\\' <<file <<endl 
				<<">>>" //<<content 
					<<endl 
				<<endl;
		}
		r_ifnot(read_fix_str(it,"\r\n"))
			return err;
	}
	return 0;
}

int main(){
	basic_block_file_on_FILE<char> reg_file("../../regstore/hklm.reg-utf8","r");
	forward_stream<basic_adressed_buffer<char,basic_block_file_on_FILE<char>>> reg_stream(&reg_file);
	const char * err;
	//dump(reg_stream.internal_iterator(),"dump:\n");
	r_ifnot(err=read_all_reg(reg_stream.internal_iterator())){
		cout << "на позиции " <<get_linecol(reg_stream.internal_iterator()) 
			<<" произошла ошибка: " <<err <<endl;
		dump(reg_stream.internal_iterator(),"");
	}
}