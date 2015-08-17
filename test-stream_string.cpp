#include <iostream>
#include <iterator>
#include "stream_string.h"
using std::cerr;
using std::cout;
using std::endl;

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
	template class _stream_string_const_iterator<buffer_t>;
	template class _stream_string_iterator<buffer_t>;
	template class stream_string<buffer_t>;
	//template bool atend<buffer_t>(const _stream_string_const_iterator<buffer_t> & it);
	//template bool atend<buffer_t>(const _stream_string_iterator<buffer_t> & it);
}
namespace std{
	template struct iterator_traits<str::_stream_string_iterator<str::buffer_t> >;
}	

int main()
{
	using namespace str;
	cout <<"================" << endl;
	try{
		char s[]="qwertyuiop[asdfghjkl;zxcvbnm,.      qwertyuiopasdfghjkl;zxcvbnm,.         qwertyuiop[asdfghjkl;'zxcvbnm,./";
		basic_block_file_on_c_str<char> file(s);
		//buffer_t buffer(0, &file, buffer_t::tail_type(), 0);
		stream_string<buffer_t> str(&file);
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
