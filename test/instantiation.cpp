#include <iostream>
#include <iterator>
#include "../base_parse.h"
#include "../forward_stream.h"
using std::cerr;
using std::cout;
using std::endl;

namespace str{
	using std::string;
	using std::wstring;
	using std::u16string;
	using std::u32string;
	template bool atend<char>(char *);
	template bool atend<const wchar_t>(const wchar_t *);
	template bool atend<char16_t>(char16_t *);
	template bool atend<char32_t>(char32_t *);
	template struct basic_span<const char>;
	template struct basic_span<wchar_t>;
	template struct basic_span<char16_t>;
	template struct basic_span<char32_t>;
	template struct basic_bispan<char>;
	template struct basic_bispan<wchar_t>;
	template struct basic_bispan<const char16_t>;
	template struct basic_bispan<char32_t>;
	
	template base_parse_error read_until_eof<char*>(char *&);
	template base_parse_error read_until_eof<wchar_t*>(wchar_t *&);
	template base_parse_error read_until_eof<char16_t*, u16string>(char16_t *&,u16string*);
	template base_parse_error read_until_eof<const char32_t*>(const char32_t *&);
	template base_parse_error read_fix_length<char*>(char*& it, size_t n);
	template base_parse_error read_fix_length<wchar_t*,wstring>(wchar_t*& it, size_t n, wstring * pstr);
	template base_parse_error read_fix_str<char16_t*,char16_t>(char16_t*& it, const char16_t * s);
	template base_parse_error read_fix_char<const char32_t*,char32_t>(const char32_t*& it, char32_t c);
	template base_parse_error read_charclass<char*,bool (&)(char)>(char*& it, bool (&)(char));
	template base_parse_error read_charclass<wchar_t*>(wchar_t*& it, wspan s);
	template base_parse_error read_charclass<char16_t*>(char16_t*& it, u16bispan s);
	template base_parse_error read_charclass_s<const char32_t*,bool (&)(char),u32string>(const char32_t*& it, bool (&)(char), u32string * pstr);
	template base_parse_error read_charclass_s<char*,string>(char*& it, span s, string * pstr);
	template base_parse_error read_charclass_s<wchar_t*,wstring>(wchar_t*& it, wbispan s, wstring * pstr);
	template base_parse_error read_charclass_c<char16_t*,bool (&)(char16_t),char16_t>(char16_t*& it, bool (&is)(char16_t), char16_t * pch);
	template base_parse_error read_charclass_c<const char32_t*,char32_t>(const char32_t*& it, u32span s, char32_t * pch);
	template base_parse_error read_charclass_c<char*,char>(char*& it, bispan s, char * pch);
	void foo1(){	const char * s="qweuirerio"; read_charclass(s,spn_space);	}
	//void foo2(){	char s[]="qweuirerio"; read_charclass(&s,spn_space<char>());	}
	void foo2(){	char s[]="qweuirerio", * p=s; read_charclass(p,spn_space);	}
	
	template base_parse_error read_until_char     <wchar_t*,wchar_t>(wchar_t*& it, 	wchar_t ch);
	template base_parse_error read_until_charclass<const char16_t*,bool(&)(char16_t)>(const char16_t*& it, 	bool(&)(char16_t));
	template base_parse_error read_until_charclass<char32_t*>(char32_t*& it,u32span s);
	template base_parse_error read_until_charclass<char*>(char*& it,bispan s);
	template base_parse_error read_while_charclass<const wchar_t*,bool(&)(wchar_t)>(const wchar_t*& it,bool(&)(wchar_t));
	template base_parse_error read_while_charclass<char16_t*>(char16_t*& it,u16span s);
	template base_parse_error read_while_charclass<char32_t*>(char32_t*& it,u32bispan s);
	template base_parse_error read_until_char     <const char*,char,string>(const char*& it,char ch, string * pstr);
	template base_parse_error read_until_charclass<wchar_t*,bool(&)(wchar_t),wstring>(wchar_t*& it, bool(&)(wchar_t), wstring * pstr);
	template base_parse_error read_until_charclass<char16_t*,u16string>(char16_t*& it, u16span s, u16string * pstr);
	template base_parse_error read_until_charclass<const char32_t*,u32string>(const char32_t*& it, u32bispan s, u32string * pstr);
	template base_parse_error read_while_charclass<char*,bool(&)(char),string>(char*& it,bool(&)(char), string * pstr);
	template base_parse_error read_while_charclass<wchar_t*,wstring>(wchar_t*& it,wspan s, wstring * pstr);
	template base_parse_error read_while_charclass<const char16_t*,u16string>(const char16_t*& it, 	u16bispan s, u16string * pstr);

	template base_parse_error read_until_str		<char32_t*,char32_t>(char32_t*& it, const char32_t * s);
	template base_parse_error read_until_str		<char*,char,string>(char*& it, const char * s, string * pstr);
	template base_parse_error read_until_pattern	<const wchar_t*,int(&)(const wchar_t *&)>(const wchar_t*& it, int(&)(const wchar_t *&));
	//template int read_until_pattern	<char16_t*,int(&)(const char16_t *&),u16string>(char16_t*& it, int(&)(const char16_t *&), u16string * pstr);
	//пример невозможности преобразования char16_t*& в const char16_t*&
	//а вот если iterator наследуется от const_iterator-а - то все OK
	//кстати переход на ссылки не спасет
	template base_parse_error read_until_pattern_s	<char16_t*,int(&)(char16_t *&),u16string>(char16_t*& it, int(&)(char16_t *&), u16string * pstr);
	template base_parse_error read_until_pattern_s	<char32_t*,int(&)(char32_t *&,u32string*),u32string>(char32_t*& it, int(&)(char32_t *&,u32string*), u32string * pstr, u32string * rez);

	template base_parse_error read_spc<const char*>(const char*& it);
	template base_parse_error read_spcs<wchar_t*>(wchar_t*& it);
	template base_parse_error read_s_fix_str<char16_t*,char16_t>(char16_t*& it, const char16_t * s);
	template base_parse_error read_s_fix_char<const char32_t*,char32_t>(const char32_t*& it, const char32_t ch);
	template base_parse_error read_s_charclass<char*,bool(&)(char)>(char*& it, bool(&)(char));
	template base_parse_error read_s_charclass_s<wchar_t*,bool(&)(wchar_t),wstring>(wchar_t*& it, bool(&)(wchar_t), wstring * ps);
	template base_parse_error read_s_charclass_c<const char16_t*,bool(&)(char16_t),char16_t>(const char16_t*& it, bool(&)(char16_t), char16_t * pc);
	template base_parse_error read_blank<char32_t*>(char32_t*& it);
	template base_parse_error read_blanks<char*>(char*& it);
	template base_parse_error read_bl_fix_str<const wchar_t*,wchar_t>(const wchar_t*& it, const wchar_t * s);
	template base_parse_error read_bl_fix_char<char16_t*,char16_t>(char16_t*& it, const char16_t ch);
	template base_parse_error read_bl_charclass<char32_t*,bool(&)(char32_t)>(char32_t*& it, bool(&)(char32_t));
	template base_parse_error read_bl_charclass_s<const char*,bool(&)(char),string>(const char*& it, bool(&)(char), string * ps);
	template base_parse_error read_bl_charclass_c<wchar_t*,bool(&)(wchar_t),wchar_t>(wchar_t*& it, bool(&)(wchar_t), wchar_t * pc);
	template base_parse_error read_line<char16_t*,u16string>(char16_t*& it, u16string * ps);
	template base_parse_error start_read_line<const char32_t*>(const char32_t*& it);
	//todo разобраться с преобразованием символов на этапе компиляции между char, wchar_t, char16_t  и char32_t
	//это в start_read_line и в span-ах
	
	template base_parse_error read_digit<char*,unsigned long>(char*& it, int ss, unsigned long * prez);
	template base_parse_error read_uint<wchar_t*,short>(wchar_t*& it, int ss, short * prez);
	template base_parse_error read_sign_uint<const char16_t*,char>(const char16_t*& it, int ss, char * prez);
	template base_parse_error read_sign_s_uint<char32_t*,long long >(char32_t*& it, int ss, long long * prez);
	template base_parse_error read_int<char*,long>(char*& it, int ss, long * prez);
	template base_parse_error read_dec<const wchar_t*,unsigned short>(const wchar_t*& it, unsigned short * prez);
	template base_parse_error read_hex<char16_t*,char>(char16_t*& it, char * prez);
	template base_parse_error read_oct<char32_t*,unsigned long long>(char32_t*& it, unsigned long long * prez);
	template base_parse_error read_bin<const char*,long>(const char*& it, long * prez);
}


namespace str{
	template class basic_file_on_cstr<char>;
	template class basic_file_on_cstr<wchar_t>;
	template class basic_file_on_cstr<char16_t>;
	template class basic_file_on_cstr<char32_t>;
	template class basic_file_on_FILE<char>;
	template class basic_file_on_FILE<wchar_t>;
	template class basic_file_on_FILE<char16_t>;
	template class basic_file_on_FILE<char32_t>;
	
	template class basic_simple_buffer<char>;
	template class basic_simple_buffer<wchar_t>;
	typedef basic_simple_buffer<char> buffer_t;
	template class _forward_stream_const_iterator<buffer_t>;
	template class _forward_stream_iterator<buffer_t>;
	template class forward_stream<buffer_t>;
	template bool atend<buffer_t>(const _forward_stream_const_iterator<buffer_t> & it);

	template class basic_adressed_buffer<char>;
	template class basic_adressed_buffer<wchar_t>;
	typedef basic_adressed_buffer<char> a_buffer_t;
	template class _forward_stream_const_iterator<a_buffer_t>;
	template class _forward_stream_iterator<a_buffer_t>;
	template class forward_stream<a_buffer_t>;
	template bool atend<a_buffer_t>(const _forward_stream_const_iterator<a_buffer_t> & it);
	template linecol get_linecol<char,512,std::allocator<char>>(
		const _forward_stream_const_iterator<basic_adressed_buffer<
			char,512,std::allocator<char>> > & it);
	
	template class basic_example_buffer<char>;
	typedef basic_example_buffer<char> e_buffer_t;
	template class _forward_stream_const_iterator<e_buffer_t>;
	template class _forward_stream_iterator<e_buffer_t>;
	template class forward_stream<e_buffer_t>;
	template bool atend<e_buffer_t>(const _forward_stream_const_iterator<e_buffer_t> & it);
}
namespace std{
	template struct iterator_traits<str::_forward_stream_iterator<str::buffer_t> >;
}	

int main()
{
	using namespace str;
	cout <<"================" << endl;
	try{
		char s[]="qwert\nyuiop[asdfghjkl;\nzxcvbnm,.  \n    qwerty\nuiopasdfghjkl;z\nxcvbnm,.         qwertyuiop[asdfghjkl;'zxcvbnm,./";
		basic_file_on_cstr<char> file(s);
		buffer_t buffer(0, &file, buffer_t::tail_type(), 0);
		forward_stream<buffer_t> str(&file);
		atend(str.iter());
	}
	catch(char * mes){
		cerr << "ошибка: " << mes << endl;
		return -1;
	}
	catch(...){
		cerr << "неизвестная ошибка" << endl;
	}
}
