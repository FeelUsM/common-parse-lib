#include <iostream>
#include <iterator>
#include "base_parse.h"
#include "forward_stream.h"
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
	template struct span<const char>;
	template struct span<wchar_t>;
	template struct span<char16_t>;
	template struct span<char32_t>;
	template struct bispan<char>;
	template struct bispan<wchar_t>;
	template struct bispan<const char16_t>;
	template struct bispan<char32_t>;
	
	template int read_until_eof<char*>(char *&);
	template int read_until_eof<wchar_t*>(wchar_t *&);
	template int read_until_eof<char16_t*, u16string>(char16_t *&,u16string*);
	template int read_until_eof<const char32_t*>(const char32_t *&);
	template int read_fix_length<char*>(char*& pit, size_t n);
	template int read_fix_length<wchar_t*,wstring>(wchar_t*& pit, size_t n, wstring * pstr);
	template int read_fix_str<char16_t*,char16_t>(char16_t*& pit, const char16_t * s);
	template int read_fix_char<const char32_t*,char32_t>(const char32_t*& pit, char32_t c);
	template int read_charclass<char*,bool (&)(char)>(char*& pit, bool (&)(char));
	template int read_charclass<wchar_t*>(wchar_t*& pit, span<typename iterator_traits<wchar_t*>::value_type> s);
	template int read_charclass<char16_t*>(char16_t*& pit, bispan<typename iterator_traits<char16_t*>::value_type> s);
	template int read_charclass_s<const char32_t*,bool (&)(char),u32string>(const char32_t*& pit, bool (&)(char), u32string * pstr);
	template int read_charclass_s<char*,string>(char*& pit, span<typename iterator_traits<char*>::value_type> s, string * pstr);
	template int read_charclass_s<wchar_t*,wstring>(wchar_t*& pit, bispan<typename iterator_traits<wchar_t*>::value_type> s, wstring * pstr);
	template int read_charclass_c<char16_t*,bool (&)(char16_t),char16_t>(char16_t*& pit, bool (&is)(char16_t), char16_t * pch);
	template int read_charclass_c<const char32_t*,char32_t>(const char32_t*& pit, span<typename iterator_traits<const char32_t*>::value_type> s, char32_t * pch);
	template int read_charclass_c<char*,char>(char*& pit, bispan<typename iterator_traits<char*>::value_type> s, char * pch);
	void foo1(){	const char * s="qweuirerio"; read_charclass(s,spn_space<char>());	}
	//void foo2(){	char s[]="qweuirerio"; read_charclass(&s,spn_space<char>());	}
	void foo2(){	char s[]="qweuirerio", * p=s; read_charclass(p,spn_space<char>());	}
	
	template int read_until_char     <wchar_t*,wchar_t>(wchar_t*& pit, 	wchar_t ch);
	template int read_until_charclass<const char16_t*,bool(&)(char16_t)>(const char16_t*& pit, 	bool(&)(char16_t));
	template int read_until_charclass<char32_t*>(char32_t*& pit,span<typename iterator_traits<char32_t*>::value_type> s);
	template int read_until_charclass<char*>(char*& pit,bispan<typename iterator_traits<char*>::value_type> s);
	template int read_while_charclass<const wchar_t*,bool(&)(wchar_t)>(const wchar_t*& pit,bool(&)(wchar_t));
	template int read_while_charclass<char16_t*>(char16_t*& pit,span<typename iterator_traits<char16_t*>::value_type> s);
	template int read_while_charclass<char32_t*>(char32_t*& pit,bispan<typename iterator_traits<char32_t*>::value_type> s);
	template int read_until_char     <const char*,char,string>(const char*& pit,char ch, string * pstr);
	template int read_until_charclass<wchar_t*,bool(&)(wchar_t),wstring>(wchar_t*& pit, bool(&)(wchar_t), wstring * pstr);
	template int read_until_charclass<char16_t*,u16string>(char16_t*& pit, span<typename iterator_traits<char16_t*>::value_type> s, u16string * pstr);
	template int read_until_charclass<const char32_t*,u32string>(const char32_t*& pit, bispan<typename iterator_traits<const char32_t*>::value_type> s, u32string * pstr);
	template int read_while_charclass<char*,bool(&)(char),string>(char*& pit,bool(&)(char), string * pstr);
	template int read_while_charclass<wchar_t*,wstring>(wchar_t*& pit,span<typename iterator_traits<wchar_t*>::value_type> s, wstring * pstr);
	template int read_while_charclass<const char16_t*,u16string>(const char16_t*& pit, 	bispan<typename iterator_traits<const char16_t*>::value_type> s, u16string * pstr);

	template int read_until_str		<char32_t*,char32_t>(char32_t*& pit, const char32_t * s);
	template int read_until_str		<char*,char,string>(char*& pit, const char * s, string * pstr);
	template int read_until_pattern	<const wchar_t*,int(&)(const wchar_t *&)>(const wchar_t*& pit, int(&)(const wchar_t *&));
	//template int read_until_pattern	<char16_t*,int(&)(const char16_t *&),u16string>(char16_t*& pit, int(&)(const char16_t *&), u16string * pstr);
	//пример невозможности преобразования char16_t*& в const char16_t*&
	//а вот если iterator наследуется от const_iterator-а - то все OK
	//кстати переход на ссылки не спасет
	template int read_until_pattern_s	<char16_t*,int(&)(char16_t *&),u16string>(char16_t*& pit, int(&)(char16_t *&), u16string * pstr);
	template int read_until_pattern_s	<char32_t*,int(&)(char32_t *&,u32string*),u32string>(char32_t*& pit, int(&)(char32_t *&,u32string*), u32string * pstr, u32string * rez);

	template int read_spc<const char*>(const char*& pit);
	template int read_spcs<wchar_t*>(wchar_t*& pit);
	template int read_s_fix_str<char16_t*,char16_t>(char16_t*& pit, const char16_t * s);
	template int read_s_fix_char<const char32_t*,char32_t>(const char32_t*& pit, const char32_t ch);
	template int read_s_charclass<char*,bool(&)(char)>(char*& pit, bool(&)(char));
	template int read_s_charclass_s<wchar_t*,bool(&)(wchar_t),wstring>(wchar_t*& pit, bool(&)(wchar_t), wstring * ps);
	template int read_s_charclass_c<const char16_t*,bool(&)(char16_t),char16_t>(const char16_t*& pit, bool(&)(char16_t), char16_t * pc);
	template int read_bln<char32_t*>(char32_t*& pit);
	template int read_blns<char*>(char*& pit);
	template int read_b_fix_str<const wchar_t*,wchar_t>(const wchar_t*& pit, const wchar_t * s);
	template int read_b_fix_char<char16_t*,char16_t>(char16_t*& pit, const char16_t ch);
	template int read_b_charclass<char32_t*,bool(&)(char32_t)>(char32_t*& pit, bool(&)(char32_t));
	template int read_b_charclass_s<const char*,bool(&)(char),string>(const char*& pit, bool(&)(char), string * ps);
	template int read_b_charclass_c<wchar_t*,bool(&)(wchar_t),wchar_t>(wchar_t*& pit, bool(&)(wchar_t), wchar_t * pc);
	template int read_line<char16_t*,u16string>(char16_t*& pit, u16string * ps);
	template int start_read_line<const char32_t*>(const char32_t*& pit);
	//todo разобраться с преобразованием символов на этапе компиляции между char, wchar_t, char16_t  и char32_t
	//это в start_read_line и в span-ах
	
	template int read_digit<char*,unsigned long>(char*& pit, int ss, unsigned long * prez);
	template int read_uint<wchar_t*,short>(wchar_t*& pit, int ss, short * prez);
	template int read_sign_uint<const char16_t*,char>(const char16_t*& pit, int ss, char * prez);
	template int read_sign_s_uint<char32_t*,long long >(char32_t*& pit, int ss, long long * prez);
	template int read_int<char*,long>(char*& pit, int ss, long * prez);
	template int read_dec<const wchar_t*,unsigned short>(const wchar_t*& pit, unsigned short * prez);
	template int read_hex<char16_t*,char>(char16_t*& pit, char * prez);
	template int read_oct<char32_t*,unsigned long long>(char32_t*& pit, unsigned long long * prez);
	template int read_bin<const char*,long>(const char*& pit, long * prez);
}


namespace str{
	template class basic_block_file_on_c_str<char>;
	template class basic_block_file_on_c_str<wchar_t>;
	template class basic_block_file_on_c_str<char16_t>;
	template class basic_block_file_on_c_str<char32_t>;
	template class basic_block_file_on_FILE<char>;
	template class basic_block_file_on_FILE<wchar_t>;
	template class basic_block_file_on_FILE<char16_t>;
	template class basic_block_file_on_FILE<char32_t>;
	
	template class basic_simple_buffer<char,string_file_on_FILE>;
	template class basic_simple_buffer<wchar_t,wstring_file_on_FILE>;
	template class basic_simple_buffer<char,basic_block_file_on_c_str<char>>;
	typedef basic_simple_buffer<char,basic_block_file_on_c_str<char>> buffer_t;
	template class _forward_stream_const_iterator<buffer_t>;
	template class _forward_stream_iterator<buffer_t>;
	template class forward_stream<buffer_t>;
	template bool atend<buffer_t>(const _forward_stream_const_iterator<buffer_t> & it);

	template class basic_adressed_buffer<char,string_file_on_FILE>;
	template class basic_adressed_buffer<wchar_t,wstring_file_on_FILE>;
	template class basic_adressed_buffer<char,basic_block_file_on_c_str<char>>;
	typedef basic_adressed_buffer<char,basic_block_file_on_c_str<char>> a_buffer_t;
	template class _forward_stream_const_iterator<a_buffer_t>;
	template class _forward_stream_iterator<a_buffer_t>;
	template class forward_stream<a_buffer_t>;
	template bool atend<a_buffer_t>(const _forward_stream_const_iterator<a_buffer_t> & it);
	template linecol get_linecol<char,basic_block_file_on_FILE<char>,512,std::allocator<char>>(
		const _forward_stream_const_iterator<basic_adressed_buffer<char,basic_block_file_on_FILE<char>,512,std::allocator<char>> > & it);

	
	template class basic_example_buffer<char,basic_block_file_on_c_str<char>>;
	typedef basic_example_buffer<char,basic_block_file_on_c_str<char>> e_buffer_t;
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
		char s[]="qwertyuiop[asdfghjkl;zxcvbnm,.      qwertyuiopasdfghjkl;zxcvbnm,.         qwertyuiop[asdfghjkl;'zxcvbnm,./";
		basic_block_file_on_c_str<char> file(s);
		//buffer_t buffer(0, &file, buffer_t::tail_type(), 0);
		forward_stream<buffer_t> str(&file);
		atend(str.internal_iterator());
	}
	catch(char * mes){
		cerr << "ошибка: " << mes << endl;
		return -1;
	}
	catch(...){
		cerr << "неизвестная ошибка" << endl;
	}
}
