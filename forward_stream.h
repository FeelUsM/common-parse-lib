//(c) FeelUs
#ifndef FORWARD_STREAM_H
#define FORWARD_STREAM_H

/*
 * я тут обратил внимание, что использование этого потока оправдано,
 * только если используются базовые алгоритмы read_fix_str, read_until_str и read_until_pattern
 * или происходит копирование итераторов
 */

/*
todo:
перекодировка
вычисление строки-столбца по итератору
файлы на фаловых дескрипторах unix

internal_file и конструкоры
*/

#include <stdio.h>	//для файлов
#include <wchar.h>	//для файлов
#include <string.h>	//strlen, wcslen - для файлов
#include <memory>	//std::allocator
#include <iterator>	//от std::iterator наследуем свои итераторы
#include <list>		//std::list
#include <iostream>	//для дебага
#include <iomanip>	//для дебага
#include <string>	//для дебага
#include <exception>//для дебага
#include <vector>	//для adressed_buffer
#include <algorithm>//для adressed_buffer
#include <utility>	//для swap-а

namespace str {//нечто среднее между string и stream
using std::list;
using std::string;
using std::pair;
using std::make_pair;
using std::swap;

struct hex{
	const void * x;
	hex(const void * m):x(m){}
};
std::ostream & operator<<(std::ostream & str, hex h){
	if(h.x)
		return str << std::setw(8)<<std::hex<<(unsigned int)h.x<<std::dec;
	else
		return str<<'0';
}

template <typename ch_t>
struct basic_dump{
	const ch_t * p;
	size_t s;
	basic_dump(const ch_t * pp=0, size_t ss=0):p(pp),s(ss){}
};
template <typename ch_t>
basic_dump<ch_t> dump(const ch_t * pp=0, size_t ss=0)	
{	return basic_dump<ch_t>(pp,ss);	}
template <typename ch_t>
std::ostream & operator<<(std::ostream & str, basic_dump<ch_t> d){
	if(d.p){
		size_t i=0;
		for(; *d.p && (d.s ? i<d.s : true); i++, d.p++)
			if(*d.p>=32)
				str<<*d.p;
			else
				switch(*d.p){
					case '\n': 	str<<"\\n";	break;
					case '\r': 	str<<"\\r";	break;
					case '\t': 	str<<"\\t";	break;
					case '\v': 	str<<"\\v";	break;
					default:	str<<"\\x"<<std::hex<<(int)*d.p;	break;
				}
		if(*d.p)
			str<<"...";
	}
	return str;
}

#define DEBUG_fatal(MES)	(std::cerr <<"--------ОШИБКА В ДЕСТРУКТОРЕ: " MES <<std::endl)
#define DEBUG_iterator(MES)	//(std::cerr MES <<std::endl)
#define DEBUG_buffer(MES)	//(std::cerr MES <<std::endl)
#define DEBUG_stream(MES)	//(std::cerr MES <<std::endl)

class stream_exception : public std::exception
{
	string _what;
public:
	stream_exception(){}
	stream_exception(string s):_what(s){}
	const char * what()const noexcept	{	return _what.c_str();	}
};
template<typename X> inline//X - string или const char *
void my_assert(bool b, X x){
	if(!b)	throw stream_exception(string("АВАРИЙНОЕ ИСКЛЮЧЕНИЕ В FORWARD_STREAM_H: ")+x);
}

// ============ ФАЙЛЫ ============
// ----****----
// ----****---- CLASS basic_block_file_c_str ----****----
// ----****----
// стрка, выдающая блоки
template <typename ch_t>
class basic_block_file_on_c_str
{
	ch_t * _file;
public:
		//CONSTRUCTION DESTRUCTION
	basic_block_file_on_c_str(ch_t * str)	: _file(str)	{	}
	basic_block_file_on_c_str() = delete;
	~basic_block_file_on_c_str() = default;
	
		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
	basic_block_file_on_c_str & operator=	(const	basic_block_file_on_c_str &	) = default;	
	basic_block_file_on_c_str				(const	basic_block_file_on_c_str &	) = default;
	basic_block_file_on_c_str & operator=	(		basic_block_file_on_c_str &&) = default;
	basic_block_file_on_c_str				(		basic_block_file_on_c_str &&) = default;
	
		//MEMBERS
	size_t read(ch_t * buf, size_t size){
		//todo для специализаций подошли бы и strncpy() и wcsncpy()
		for(size_t i=0; i<size; i++,buf++,_file++)
			if(!(*_file))
				return i;
			else
				*buf = *_file;
		return size;
	}
	bool eof(){
		return !*_file;
	}
};
	
// ----****----
// ----****---- CLASS basic_block_file_FILE ----****----
// ----****----
// файл, выдающий блоки
template <typename ch_t>
class basic_block_file_on_FILE
{
	FILE * _file;
public:
		//CONSTRUCTION DESTRUCTION
	basic_block_file_on_FILE(const char * name, const char * mode){
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
		//todo попробовать с и без FILEйных буферов
		//my_assert(!setvbuf(file,NULL,_IONBF,0),"не получилось отключить буферизацию FILE");
	}
	basic_block_file_on_FILE() = delete;
	~basic_block_file_on_FILE(){
		fclose(_file);
	}
	
		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
	basic_block_file_on_FILE & operator=	(const	basic_block_file_on_FILE &	) = default;	
	basic_block_file_on_FILE				(const	basic_block_file_on_FILE &	) = default;
	basic_block_file_on_FILE & operator=	(		basic_block_file_on_FILE &&) = default;
	basic_block_file_on_FILE				(		basic_block_file_on_FILE &&) = default;
	
		//MEMBERS
	size_t read(ch_t * buf, size_t size){
		return fread(buf,size,sizeof(ch_t),_file);
	}
	bool eof(){
		return feof(_file);
	}
	FILE * file(){
		return _file;
	}
};
	
// ----****----
// ----****---- CLASS string_file_FILE ----****----
// ----****----
// файл, выдающий строки
class string_file_on_FILE
{
	FILE * _file;
	bool external;
	bool init = false;
public:
		//CONSTRUCTION DESTRUCTION
	string_file_on_FILE(const char * name, const char * mode)	: external(false)	{
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
	}
	string_file_on_FILE(FILE * f)	: _file(f)	, external(true) {
	}

	string_file_on_FILE() = delete;
	~string_file_on_FILE(){
		if(external)	return;
		fclose(_file);
	}

		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
	string_file_on_FILE & operator=	(const	string_file_on_FILE &	) = default;	
	string_file_on_FILE				(const	string_file_on_FILE &	) = default;
	string_file_on_FILE & operator=	(		string_file_on_FILE &&) = default;
	string_file_on_FILE				(		string_file_on_FILE &&) = default;
	
		//MEMBERS
	size_t read(char * buf, size_t size){
		if(!init){
			my_assert(size>0,"string_file_on_FILE: запрошенный размер буфера =0");
			*buf++ = '\n';
			init = true;
			return 1;
		}
		if(fgets(buf,size,_file))
			return strlen(buf);
		else
			return 0;
	}
	bool eof(){
		return feof(_file);
	}
	
	FILE * file(){
		return _file;
	}
	bool has_internal(){
		return !external;
	}
};
	
// ----****----
// ----****---- CLASS wstring_file_FILE ----****----
// файл, выдающий wстроки
// ----****----
class wstring_file_on_FILE
{
	FILE * _file;
	bool external;
public:
		//CONSTRUCTION DESTRUCTION
	wstring_file_on_FILE(const char * name, const char * mode)	: external(false)	{
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
	}
	wstring_file_on_FILE(FILE * f)	: _file(f), external(true) 	{
	}

	wstring_file_on_FILE() = delete;
	~wstring_file_on_FILE()	{
		if(external)	return;
		fclose(_file);
	}

		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
	wstring_file_on_FILE & operator=	(const	wstring_file_on_FILE &	) = default;	
	wstring_file_on_FILE				(const	wstring_file_on_FILE &	) = default;
	wstring_file_on_FILE & operator=	(		wstring_file_on_FILE &&) = default;
	wstring_file_on_FILE				(		wstring_file_on_FILE &&) = default;

		//MEMBERS
	size_t read(wchar_t * buf, size_t size){
		if(fgetws(buf,size,_file))
			return wcslen(buf);
		else
			return 0;
	}
	bool eof(){
		return feof(_file);
	}
	
	FILE * file(){
		return _file;
	}
	bool has_internal(){
		return external;
	}
};

/*
 * здесь можно добавить файлы не на FILE а на файловых дескрипторах unix 
 * или на std::istream (последнее - я не знаю зачем)
 */

/* то что выше, наверно, можно перенести в отдельный какой-нибудь file.h*/

// ============ ПРОСТЫЕ БУФЕРА И ИТЕРАТОРЫ ============

// ----****----
// ----****---- forward defs ----****----
// ----****----
template<class buf_t>
class forward_stream;
template<class buf_t>
class _forward_stream_const_iterator;
template<class buf_t>
class _forward_stream_iterator;

//буфер #№[адрес структуры] (кол-во итераторов) [размер буфера] 'первые 10 символов содержимого'
//{DEBUG basic_simple_buffer
template <typename ch_t, class file_t, int buf_size, class alloc_t>
class basic_simple_buffer ;
template <typename ch_t, class file_t, int buf_size, class alloc_t>
std::ostream & operator<<(std::ostream & str, const basic_simple_buffer<ch_t,file_t,buf_size,alloc_t> & b){
	str <<"буфер "
		<<"#" <<b._nomber
		<<"[" <<hex(&b) <<"]";
	if(b._begin){
		str <<"(" <<b._iterator_counter <<")"
			<<"[" <<b._end-b._begin <<"]"
			<<"'" <<dump(b._begin,10) <<"'";
		if(b._atend)
			str <<"end";
	}
	else{
		str <<"инвалидный";
	}
	return str;
}
//}
// ----****----
// ----****---- CLASS basic_simple_buffer ----****----
// ----****----
template <typename ch_t, class file_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_simple_buffer 
{
	//можно сделать buf_size = alloc_t().init_page_size()
	//но его из мануалов почему-то убрали
	typedef basic_simple_buffer<ch_t, file_t, buf_size, alloc_t> my_t;
	template <class T>
	friend class _forward_stream_const_iterator;
	friend std::ostream & operator<< <ch_t,file_t,buf_size,alloc_t>(std::ostream & str, const my_t & b);
public:	//TYPEDEFS AND TYPES
	typedef forward_stream<my_t> 				basic_type;
	typedef file_t								file_type;		//отличие от STL

	typedef ch_t								value_type;
	typedef alloc_t								allocator_type;
	typedef _forward_stream_const_iterator<my_t>const_iterator;
	typedef _forward_stream_iterator<my_t>		iterator;
	typedef typename alloc_t::size_type			size_type;
	typedef typename alloc_t::difference_type	difference_type;
	typedef typename alloc_t::pointer			pointer;
	typedef typename alloc_t::const_pointer		const_pointer;
	typedef typename alloc_t::reference			reference;
	typedef typename alloc_t::const_reference	const_reference;
	
	struct tail_type{};			//это как пример, здесь можно определить любой тип
	struct stream_data_type{};	//это как пример, здесь можно определить любой тип
	
private: //{ //DATA
	basic_type * const _base;//поток
	file_t * 	const _file;	//файл
	const int _nomber;		//что бы можно было быстро определить, какой буфер правее, какой левее
	int _iterator_counter;	//счетчик итераторов, находящихся на этом буфере
//}
protected:
	basic_simple_buffer(basic_type * b, file_t * f, int n)	//доступ к полям выше для наследников
		: _base(b), _file(f), _nomber(n), _iterator_counter(0)	{}

	ch_t * _begin, * _end;	//начало буфера и логический конец буфера (физически может быть больше)
							//буфер обязан завершаться (ch_t)0
	bool _atend; 			//кешируем file->eof()
public:	
		//GETTERS
	ch_t *		begin()const	{	return _begin;	}
	ch_t *		end()const		{	return _end;	}
	size_t		size()const		{	return _end-_begin;	}
	bool 		eof()const		{	return _atend;	}
	bool 		is_free()const	{	return _iterator_counter==0;	}
	int 		nomber()const	{	return _nomber;	}
	basic_type * base()const	{	return _base;	}
	file_t * 	file()const		{	return _file;	}//на всякий случай
	
	//хвост - неперекодированный кусок этого буфера
	tail_type 	tail()		{	return tail_type();	}
		//CONSTRUCTION DESTRUCTION
	/*
	 * поток, файл, хвост, номбер
	 * хвост - неперекодированный кусок предыдущего буфера
	 * может содержать указатели на структуры предыдущего буфера, 
	 * или будет сконструирован по умолчанию
	 * но на всякий случай все равно лучше, что бы было все при себе
	 */
	basic_simple_buffer(basic_type * b, file_t * f, tail_type tail, int n)	
		: basic_simple_buffer{b,f,n}	{
		//т.к. tail() всегда возвращает pair(0,0) в simple_buffer
		_begin = alloc_t().allocate(buf_size);
		_end = _begin+ _file->read(_begin,buf_size-1);
		if(_begin == _end) {
			if((_atend=_file->eof()))
				;//it's OK //внезапный конец файла
			else
				throw "прочитано 0 символов, а конца файла нет";
		}
		*_end = 0;
		_atend = _file->eof();
		//параметры перекодировки можно плоучить при помощи stream_data_type _base->stream_data()
		DEBUG_buffer(<<*this<<" - сконструирован");
	}
	
	basic_simple_buffer()	: basic_simple_buffer{0,0,-1}	{
		_begin = 0;
		//DEBUG_buffer(<<*this<<" - сконструирован по умолчанию");
	}
	~basic_simple_buffer(){
		if(!_begin)	return;
		DEBUG_buffer(<<*this<<" - разрушаем");
		//эта проверка очень желательна
		if(_iterator_counter)
			DEBUG_fatal(<<*this<<" - при разрушении занят итераторами");
		alloc_t().deallocate(_begin,buf_size);	
		_begin = 0;
		_iterator_counter = 0;
	}

		//COPYING
// http://sergeyteplyakov.blogspot.ru/2012/05/c-11-faq.html#default2
	my_t & operator=	(const	my_t &	) = delete;	
	basic_simple_buffer	(const	my_t &	) = delete;
	my_t & operator=	(		my_t &&	) = delete;
	basic_simple_buffer	(		my_t &&	r)	: _base(r._base), _file(r._file), _nomber(r._nomber)	{	
		_iterator_counter=r._iterator_counter;
		_begin			= r._begin;
		_end			= r._end;
		_atend			= r._atend;
		r._begin=0;
		//DEBUG_buffer(<<*this <<" - сконструирован из - " <<r);
	}
}; //CLASS simple_buffer

/* итератор и конст_итератор отличаются тем, что разыменованный итератор можно изменять
 * да да, считанное из файла в буфера можно изменять
 */

template<class buf_t> inline
bool atend(const _forward_stream_const_iterator<buf_t> & it);
template<class buf_t> inline
void swap(_forward_stream_const_iterator<buf_t> & it1, _forward_stream_const_iterator<buf_t> & it2);
template<class buf_t> 
typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type
distance(const _forward_stream_const_iterator<buf_t> & it1, const _forward_stream_const_iterator<buf_t> & it2);
template<class buf_t> 
bool advance_or_end(_forward_stream_const_iterator<buf_t> & it, 
	typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type);
template<class buf_t> 
void advance(_forward_stream_const_iterator<buf_t> & it, 
	typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type);

//
//{DEBUG _forward_stream_const_iterator
template <typename buf_t>
std::ostream & operator<<(std::ostream & str, const _forward_stream_const_iterator<buf_t> & b){
	str <<"итератор "
		<<"[" <<hex(&b) <<"]";
	if(b.point){
		str <<"(" <<b.itbuf->nomber() 			<<"(" <<b.buf_iterator_counter() <<")"
			<<"," <<b.point-b.itbuf->begin() 	<<"('" <<dump(b.point,1) <<"'))";
	}
	else{
		str <<"инвалидный";
	}
	return str;
}
//}
// ----****----		
// ----****---- ITERATOR _forward_stream_const_iterator ----****----
// ----****----		
template<class buf_t>
class _forward_stream_const_iterator
	:public std::iterator<
		std::forward_iterator_tag,
		typename buf_t::value_type,
		typename buf_t::difference_type,
		typename buf_t::const_pointer,
		typename buf_t::const_reference
	>
{
		//private TYPEDEFS
	typedef typename buf_t::value_type				ch_t;
	typedef _forward_stream_const_iterator<buf_t> 	my_t;
	typedef typename list<buf_t>::iterator 			super_iterator;
		//FRIENDS
	friend bool 
	atend<buf_t>(const my_t & );
	friend 
	typename std::iterator_traits<my_t>::difference_type
	distance<buf_t>(const my_t & first, const my_t & last);
	friend bool 
	advance_or_end<buf_t>(my_t & it, typename buf_t::difference_type);
	friend void 
	advance<buf_t>(my_t & it, typename buf_t::difference_type);
	friend void 
	swap<buf_t>(my_t & it1, my_t & it2);
	friend
	std::ostream & operator<< <buf_t>(std::ostream & str, const my_t & b);
	
protected:	//DATA
	ch_t * 			point;//==0 <=> atend
	ch_t * 			endbuf;
	super_iterator 	itbuf;
	
	int buf_iterator_counter()const		{	return itbuf->_iterator_counter;	}
public:	//GETTERS - только ради получения доступа на чтения не надо наследоваться
	ch_t * 			get_point()const	{	return point;	}
	ch_t * 			get_endbuf()const	{	return endbuf;	}
	super_iterator 	get_itbuf()const	{	return itbuf;	}
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_forward_stream_const_iterator(super_iterator sit): point(sit->begin()), endbuf(sit->end()), itbuf(sit) {
		itbuf->_iterator_counter++;
		DEBUG_iterator(<<*this <<" - сконструировали от super_iterator'а");
	}

	_forward_stream_const_iterator()					: point(0), endbuf(0)	{
		DEBUG_iterator(<<*this <<" - сконструировали по умолчанию");
	}
	~_forward_stream_const_iterator(){
		DEBUG_iterator(<<*this <<" - разрушаем");
		if(!point)	return;
		if(--itbuf->_iterator_counter ==0)
			itbuf->base()->del_buf_request(itbuf);
		point=0;
		endbuf=0;
	}
		
		//COPYING
	_forward_stream_const_iterator(const my_t & r)	: point(r.point), endbuf(r.endbuf), itbuf(r.itbuf)	{
		if(point)
			itbuf->_iterator_counter++;
		DEBUG_iterator(<<*this <<" - сконструировали копию от " <<r);
	}
	my_t & operator=(const my_t & r)	{
		//можно оптимизировать
		this->~my_t();
		point=r.point;
		endbuf=r.endbuf;
		itbuf=r.itbuf;
		if(point)
			itbuf->_iterator_counter++;
		DEBUG_iterator(<<*this <<" - присвоили от " <<r);
		return *this;
	}

/*	todo
	_forward_stream_const_iterator( my_t && r)
	my_t & operator=( my_t && r)
*/

		//ACCESS
	const ch_t & operator*()const	{	
		return *point;	
	}
	const ch_t * operator->()const	{	
		return point;	
	}

		//MOVING
	my_t & operator++()	{	// ++myInstance. 
		my_assert(point,"попытка сдвинуть инвалидный указатель");
		if(++point ==endbuf){	//конец буфера
			DEBUG_iterator(<<*this <<"в начале прыжка");
			if(itbuf->eof()){	//конец файла
				this->~_forward_stream_const_iterator();
				return *this;
			}
			typename buf_t::basic_type * mybase = itbuf->base();
			super_iterator nextbuf = itbuf;
			nextbuf++;			//попытка перейти на следующий буфер
			if(nextbuf== mybase->pbufs()->end()){	//но его не оказалось
				nextbuf = mybase->add_buf();
				if(nextbuf== mybase->pbufs()->end()){	//и вообще оказался неожиданный конец файла
					itbuf->_atend = true;;
					this->~_forward_stream_const_iterator();
					return *this;
				}
			}
			if(--itbuf->_iterator_counter)
				mybase->del_buf_request(itbuf);
			itbuf = nextbuf;
			itbuf->_iterator_counter++;
			point = itbuf->begin();
			endbuf = itbuf->end();
		}
		return * this;   
	}
	my_t operator++(int){	// myInstance++.
		my_t orig = *this;
		++(*this);
		return orig;
	}

		//ARITHMETIC
	bool operator==(const my_t & r)const{
		return itbuf==r.itbuf && point==r.point || point==0 && r.point==0;
	}
	bool operator<(const my_t & r)const	{
		if(point==0 || r.point==0)
			return r.point==0 && point!=0;
		my_assert(itbuf->base()==r.itbuf->base(),"сравнение итераторов по разным потокам");
		if(itbuf->nomber()<r.itbuf->nomber())
			return true;
		else if(itbuf->nomber()==r.itbuf->nomber())
			return point<r.point;
		else
			return false;
	}
};

// ----****----		
// ----****---- TEMPLATE CLASS _forward_stream_iterator ----****----
// ----****----		
template<class buf_t>
class _forward_stream_iterator
	:public _forward_stream_const_iterator<buf_t>
	//для него iterator_traits определен отдельно внизу
{
		//private TYPEDEFS
	typedef typename buf_t::value_type		ch_t;
	typedef _forward_stream_iterator<buf_t> 	my_t;
	typedef _forward_stream_const_iterator<buf_t> parent_t;
	typedef typename list<buf_t>::iterator 	super_iterator;
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_forward_stream_iterator(super_iterator sit)	: parent_t(sit)	{	}

	_forward_stream_iterator()					: parent_t()	{	}
	~_forward_stream_iterator()					= default;
		
		//COPYING
	_forward_stream_iterator(const parent_t & r)	: parent_t(r)	{	}
	my_t & operator=(const parent_t & r)	{	parent_t::operator=(r);	return *this;	}
/*	todo
	_forward_stream_const_iterator( my_t && r)
	my_t & operator=( my_t && r)
*/

		//ACCESS
	ch_t & operator*()const		{	return const_cast<ch_t&>  (parent_t::operator*());		}
	ch_t * operator->()const	{	return const_cast<ch_t*> (parent_t::operator->());	}

		//MOVING
	my_t & operator++()			{	parent_t::operator++();	return *this;	}	// ++myInstance. 	
	my_t operator++(int){	// myInstance++.
		my_t orig = *this;
		parent_t::operator++();
		return orig;
	}

		//ARITHMETIC
	bool operator==(const parent_t & r)const	{	return parent_t::operator==(r);	}
	bool operator<(const parent_t & r)const		{	return parent_t::operator<(r);	}
};

//что бы постоянно не подключать std::rel_ops
template <class buf_t> inline bool operator!= 
	(const _forward_stream_const_iterator<buf_t> &  x, const _forward_stream_const_iterator<buf_t> &  y) { return !(x==y); }
template <class buf_t> inline bool operator>  
	(const _forward_stream_const_iterator<buf_t> &  x, const _forward_stream_const_iterator<buf_t> &  y) { return y<x; }
template <class buf_t> inline bool operator<= 
	(const _forward_stream_const_iterator<buf_t> &  x, const _forward_stream_const_iterator<buf_t> &  y) { return !(y<x); }
template <class buf_t> inline bool operator>= 
	(const _forward_stream_const_iterator<buf_t> &  x, const _forward_stream_const_iterator<buf_t> &  y) { return !(x<y); }

template<class buf_t> inline
void swap(_forward_stream_const_iterator<buf_t> & it1, _forward_stream_const_iterator<buf_t> & it2){
	swap(it1.point ,it2.point );
	swap(it1.endbuf,it2.endbuf);
	swap(it1.itbuf ,it2.itbuf );
}

template<class buf_t> inline
void swap(_forward_stream_iterator<buf_t> & it1, _forward_stream_iterator<buf_t> & it2){
	swap(reinterpret_cast<_forward_stream_const_iterator<buf_t> &>(it1),
		reinterpret_cast<_forward_stream_const_iterator<buf_t> &>(it2));
}

template<class buf_t> inline
bool atend(const _forward_stream_const_iterator<buf_t> & it) {	
	return it.point==0;	
}

/*
 * вычисляет расстояние между итераторами
 */
template<class buf_t> 
typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type
distance(const _forward_stream_const_iterator<buf_t> & first1, const _forward_stream_const_iterator<buf_t> & last1){
	typedef typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type diff_t;
	_forward_stream_const_iterator<buf_t> first = first1;//todo: оптимизоровать
	_forward_stream_const_iterator<buf_t> last = last1;
	if(first.point==0 && last.point==0)	
		return 0;
	diff_t factor;
	if(first > last){
		swap(first,last);
		factor = -1;
	}
	else
		factor = 1;
	if(first.point && last.point && first.itbuf->nomber()==last.itbuf->nomber())
		return factor*(last.point-first.point);
	typename list<buf_t>::const_iterator it= first.itbuf;
	typename list<buf_t>::const_iterator lastbuf = 
		last.point ?
		last.itbuf :
		--(first.itbuf->base()->bufs().end());
	diff_t dist= it++ ->end()-first.point;
	for(; it!=lastbuf; it++)
		dist+= it->size();
	dist+= (last.point ? last.point : it->end())-it->begin();
	return dist*factor;
}
template<class buf_t> 
typename std::iterator_traits<_forward_stream_iterator<buf_t>>::difference_type
distance(const _forward_stream_iterator<buf_t> & first1, const _forward_stream_iterator<buf_t> & last1){
	return distance((const _forward_stream_const_iterator<buf_t> &)first1,(const _forward_stream_const_iterator<buf_t> &)last1);
}

/*
 * 
 */
template<class buf_t> 
typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type 
advance_or_end(_forward_stream_const_iterator<buf_t> & it, 
	typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type dist){
	//typedef typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type diff_t;
	if(dist>0){
		while(!it.itbuf->eof() && dist>it.endbuf-it.point){
			dist -= it.endbuf-it.point;
			it.point = it.endbuf-1;
			it++;
		}
		if(it.itbuf->eof() && dist>=it.endbuf-it.point){
			dist -= it.endbuf-it.point;
			it.point = it.endbuf-1;
			it++;
			return dist;
		}
		it.point-=dist;
		return 0;
	}
	else{
		while(it.itbuf!=it.itbuf->base->bufs().begin() && dist>it.point-it.itbuf->begin()){
			dist += it.point-it.itbuf->begin()+1;
			it.point = it.itbuf->bagin();
			it--;
		}
		if(it.itbuf->eof() && dist>=it.endbuf-it.point){
			dist += it.point-it.itbuf->begin();
			it=_forward_stream_const_iterator<buf_t>();
			return dist;
		}
		it.point+=dist;
		return 0;
	}
	return 0;
}
template<class buf_t> 
void advance(_forward_stream_const_iterator<buf_t> & it, 
	typename std::iterator_traits<_forward_stream_const_iterator<buf_t>>::difference_type dist){
	if(advance_or_end(it,dist))
		throw stream_exception("out of range in advance in forward stream iterator");
}

// ============ БУФЕР С ВЫЧИСЛЕНИЕМ СТРОКИ-СТОЛБЦА ============
struct linecol{
	int line,col;
	linecol(int l=1, int c=1):line(l),col(c){}
};

template <typename ch_t, class file_t, int buf_size, class alloc_t>
class basic_adressed_buffer;

template <typename ch_t, class file_t, int buf_size, class alloc_t>
linecol get_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> > & it);
template <typename ch_t, class file_t, int buf_size, class alloc_t>
void set_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> > & it, linecol lc);

// ----****----
// ----****---- CLASS basic_adressed_buffer ----****----
// ----****----
template <typename ch_t, class file_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_adressed_buffer : public basic_simple_buffer<ch_t,file_t,buf_size,alloc_t>
{
	typedef basic_simple_buffer<ch_t,file_t,buf_size,alloc_t> parent_t;
	typedef basic_adressed_buffer<ch_t, file_t, buf_size, alloc_t> my_t;
	friend linecol 	get_linecol<ch_t,file_t,buf_size,alloc_t>(const _forward_stream_const_iterator<my_t> &);
	friend void 	set_linecol<ch_t,file_t,buf_size,alloc_t>(const _forward_stream_const_iterator<my_t> & , linecol);
	
	/*
	 * самый первый символ и каждый символ ПОСЛЕ '\n' содержится в таблице
	 */
	typedef std::vector<pair<const ch_t *,linecol>> NLs_type;
	NLs_type NLs;
	void NLs_init(linecol lc){
		NLs.clear();
		NLs.push_back(make_pair(parent_t::begin(),lc));
		//todo: использовать строковые функции
		size_t line = lc.line;
		for(const ch_t * pc = parent_t::begin(); pc!=parent_t::end(); pc++)
			if(*pc==(ch_t)'\n')
				NLs.push_back(make_pair(pc+1,linecol(++line,1)));
	}
public:
	typedef _forward_stream_const_iterator<my_t>const_iterator;
	typedef _forward_stream_iterator<my_t>		iterator;
	typedef forward_stream<my_t> 				basic_type;
	typedef linecol tail_type;
	basic_type * base()const	{	return reinterpret_cast<basic_type*>(parent_t::base());	}
	tail_type 	tail()const {
		my_assert(!NLs.empty(),"в буфере с адресацией не задана адресация");
		return tail_type(
			(--NLs.end())->second.line, 
			(--NLs.end())->second.col + parent_t::end()- (--NLs.end())->first);	
	}
	basic_adressed_buffer(basic_type * b, file_t * f, tail_type lc, int n)
	:parent_t(reinterpret_cast<typename parent_t::basic_type*>(b),f,typename parent_t::tail_type(),n)
	{
		NLs_init(lc);
	}
		//COPYING
	my_t & operator=		(const	my_t &	) = delete;	
	basic_adressed_buffer	(const	my_t &	) = delete;
	my_t & operator=		(		my_t &&	) = delete;
	basic_adressed_buffer	(		my_t &&	r)	: parent_t(static_cast<my_t&&>(r)){}	
};

template <typename ch_t>
bool _NL_pair_lt(const pair<const ch_t *,linecol> & l, const pair<const ch_t *,linecol> & r){
	return l.first < r.first;
}

template <typename ch_t, class file_t, int buf_size, class alloc_t>
linecol get_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> > & it){
	typename basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t>::NLs_type::iterator q= 
		std::lower_bound(it.get_itbuf()->NLs.begin(),it.get_itbuf()->NLs.end(),
			make_pair(it.get_point(),linecol()),_NL_pair_lt<ch_t>);
	return linecol(q->second.line,q->second.col+it.get_point()-q->first);
}
template <typename ch_t, class file_t, int buf_size, class alloc_t>
void set_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> > & it, linecol lc){
	typename basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t>::NLs_type::iterator q= 
		std::lower_bound(it.get_itbuf()->NLs.begin(),it.get_itbuf()->NLs.end(),
			make_pair(it.get_point(),linecol()),_NL_pair_lt<ch_t>);
	if(q->first == it->get_point())//вставляем или переприсваиваем
		q->second = lc;
	else{
		it.get_itbuf()->NLs.insert(q,make_pair(it->get_point(),lc));
		q++;
	}
	q++;
	size_t line=lc.line;
	line++;
	for(; q!=it.get_itbuf()->NLs.end(); q++,line++)//доделываем текущий буфер
		q->second.line = line;
	typedef basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> buf_t;
	//обрабатываем последующие буфера, NLs.init() не пригодился
	for(typename list<buf_t>::iterator bit = ++it.get_itbuf() ; bit!=it.get_itbuf()->base()->bufs().end(); bit++)
		for(q=it.get_itbuf()->NLs.begin() ; q!=it.get_itbuf()->NLs.end(); q++,line++)
			q->second.line = line;
}

template <typename ch_t, class file_t, int buf_size, class alloc_t>
void goto_linecol(_forward_stream_const_iterator<basic_adressed_buffer<ch_t,file_t,buf_size,alloc_t> > * it, linecol lc){
	
}

/* здесь можно добавить буфер с перекодировкой, 
 * с вычислением строки-столбца по указателю/итератору
 * с блекджеком
 * со шлюхами
 * smth else
 */

/*минимальный работающий пример наследования: 
 *проверять работоспособность инстанцированием всех классов и функций, параметризованных этим буфером
 */
// ----****----
// ----****---- CLASS basic_example_buffer ----****----
// ----****----
template <typename ch_t, class file_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_example_buffer : public basic_simple_buffer<ch_t,file_t,buf_size,alloc_t>
{
	typedef basic_simple_buffer<ch_t,file_t,buf_size,alloc_t> parent_t;
	typedef basic_example_buffer<ch_t, file_t, buf_size, alloc_t> my_t;
public:
	typedef _forward_stream_const_iterator<my_t>const_iterator;
	typedef _forward_stream_iterator<my_t>		iterator;
	typedef forward_stream<my_t> 				basic_type;
	typedef linecol tail_type;
	basic_type * base()const{	return reinterpret_cast<basic_type*>(parent_t::base());	}
	tail_type 	tail()		{	return tail_type();	}
	basic_example_buffer(basic_type * b, file_t * f, tail_type tail, int n)
	:parent_t(reinterpret_cast<typename parent_t::basic_type*>(b),f,typename parent_t::tail_type(),n)
	{}
		//COPYING
	my_t & operator=	(const	my_t &	) = delete;
	basic_example_buffer(const	my_t &	) = delete;
	my_t & operator=	(		my_t &&	) = delete;
	basic_example_buffer(		my_t &&	r)	: parent_t(static_cast<my_t&&>(r)){}
};

// ============ ПОТОК ============

template <typename buf_t>
std::ostream & operator<<(std::ostream & str, const forward_stream<buf_t> & s){
	for(auto i = s.cbufs().begin(); i!=s.cbufs().end(); i++)
		str <<" |" << *i;
	return str;
}
// ----****----		
// ----****---- CONTEINER forward_stream ----****----
// ----****----
template<class buf_t>
class forward_stream
{
		//TYPE DEFINES
public:
	typedef typename buf_t::value_type			value_type;
	typedef typename buf_t::size_type			size_type;
	typedef typename buf_t::difference_type		difference_type;
	typedef typename buf_t::pointer				pointer;
	typedef typename buf_t::const_pointer		const_pointer;
	typedef typename buf_t::reference			reference;
	typedef typename buf_t::const_reference		const_reference;
	typedef typename buf_t::iterator			iterator;
	typedef typename buf_t::const_iterator		const_iterator;
	//отличие от STL:
	typedef buf_t								buffer_type;	
	typedef typename buf_t::file_type			file_type;	
	typedef typename buf_t::stream_data_type 	stream_data_type;
	typedef typename buf_t::tail_type 			tail_type;
	
private: //{
	//typedef value_type							ch_t; //=> ch_t вообще в буфере не нужно
	typedef forward_stream<buf_t>	my_t;
	
		//DATA DEFINES
	file_type * _file;
	//bool _has_internal_file; //todo сделать internal file

	/*
	 * итераторы потока имеют итераторы этого контейнера
	 * и если это будет deque, то там возможно перераспределение памяти
	 * и в этом случае все итераторы контейнера станут недействительными
	 * 
	 * мало того, что у нас запрещен оператор присваивания в буферах
	 * - это можно изменить
	 * самое главное, что итераторы потока не смогут найти структуры буферов, 
	 * что бы получить указатели на объект потока, в котором находится этот контейнер
	 * что бы в него добавить или удалить другой буфер
	 */
	list<buf_t> _bufs;
	stream_data_type _data;

	mutable iterator _iterator;//internal iterator
//}
public:	
		//old PRIVATE MEMBERS
	/*
	 * читает новый буфер, и настраивает его номер
	 * если прочитано 0 символов - 
	 *	уничтожет этот буфер, 
	 *	возвращает _bufs.end() //это означает конец файла
	 * иначе возвращает ук-тель(итератор) на этот буфер
	 */
	typename list<buf_t>::iterator 
	add_buf(){
		//хотя бы один буфер уже суествует, иначе этот метод не будет вызываться
		typename list<buf_t>::iterator ppb= --_bufs.end();//поинтер на предыдущий буфер
		DEBUG_stream(<<"stream: сейчас будет добавлен буфер:");
		_bufs.push_back(buf_t(this,_file,ppb->tail(),ppb->nomber()+1));
		typename list<buf_t>::iterator pb= --_bufs.end();//поинтер на текущий буфер
		if(pb->size()==0)//внезапный конец файла
		{
			_bufs.pop_back();
			return _bufs.end();
		}
		return pb;
	}

	//если надо - удаляет буфер
	void del_buf_request(typename list<buf_t>::iterator itbuf){
		if(itbuf==_bufs.begin())
			while(_bufs.begin()!=_bufs.end() && _bufs.begin()->is_free())
				_bufs.pop_front();
	}
	
		//CONSTRUCTION AND DESTRUCTION
	/*
	 * читает первый буфер
	 * и создает первый (internal) iterator
	 */
	explicit
	forward_stream(file_type * f, typename buf_t::stream_data_type dat= typename buf_t::stream_data_type())
		: _file(f)
		, _data(dat)
	{
		_bufs.push_back(buf_t(this,_file,typename buf_t::tail_type(),0));
		if(_bufs.begin()->size()==0)	{//неожиданный конец файла
			DEBUG_stream( << "неожиданный конец файла" );
			_bufs.pop_front();
			_iterator = iterator();
		}
		else
			_iterator = iterator(_bufs.begin());//он сам сконструируется от итераора на буфер
		DEBUG_stream( << "forward_stream ("<<*this<<")- сконструирован" );
	}
	
	forward_stream() = delete;
	
	/*
	 * нет итераторов - нет буферов
	 * => перед концом области, где объявлены и контейнер и итераторы
	 * (контейнер и итераторы уничтожаются в произвольном порядке)
	 * все итераторы должны дойти до конца или быть удалены, например путем присваивания им stream.end();
	 * итерторы дошедшие до конца с потоком не связаны
	 */
	~forward_stream()	{
		_iterator.~iterator();
		DEBUG_stream(
			<< "internal _iterator"
			<<"["<<hex(&_iterator)<<"]"
			<<" - разрушен"
		);
		del_buf_request(_bufs.begin());
		if(!_bufs.empty())
			DEBUG_fatal(<<"forward_stream: в потоке остались не удаленные буфера");
		else
			DEBUG_stream( <<"деструктирование потока идет упешно" );
		DEBUG_stream( <<"forward_stream - закончили разрушаться" );
		//если потом начнут разрушаться итераторы - это пиздец
	}

		//COPYING
	my_t & operator=(const	my_t &	) = delete;	
	forward_stream	(const	my_t &	) = delete;
	my_t & operator=(		my_t &&	) = delete;
	forward_stream	(		my_t &&	) = delete;//возможно можно разрешить

//{	//PUBLIC MEMBERS
	typename buf_t::stream_data_type
	stream_data()const
	{	return _data;	}
		
	iterator & 
	internal_iterator()
	{	return _iterator;	}
	const_iterator & 
	internal_citerator()const
	{	return _iterator;	}

	iterator *
	pinternal_iterator()
	{	return &_iterator;	}
	const_iterator *
	pinternal_citerator()const
	{	return &_iterator;	}

	list<buf_t> &
	bufs()
	{	return _bufs;	}
	const list<buf_t> &
	cbufs()const
	{	return _bufs;	}

	list<buf_t> *
	pbufs()
	{	return &_bufs;	}
	const list<buf_t> *
	pcbufs()const
	{	return &_bufs;	}

	bool 
	empty()const
	{	return _bufs.empty();	}

	bool 
	atend()const
	{	return empty() || (--_bufs.end())->eof();	}

	const_iterator 
	end()const
	{	return const_iterator();	}
//}
};//CLASS basic_stream_string

}//namespace str
namespace std{
template<class buf_t>
	struct iterator_traits<str::_forward_stream_iterator<buf_t> >{
			typedef std::forward_iterator_tag		iterator_category;
			typedef typename buf_t::value_type		value_type;
			typedef typename buf_t::difference_type	difference_type;
			typedef typename buf_t::pointer			pointer; //исправляется здесь
			typedef typename buf_t::reference		reference; //и здесь (убирается const)
	};
}
#endif //FORWARD_STREAM_H
