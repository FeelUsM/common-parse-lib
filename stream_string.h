﻿//(c) FeelUs
#ifndef MYSTREAM_H
#define MYSTREAM_H

/*
todo:
перекодировка
вычисление строки-столбца по итератору
файлы на фаловых дескрипторах unix

advance_or_end()
advance()
distance()

internal_file и конструкоры
*/

#include <stdio.h>	//для файлов
#include <wchar.h>	//для файлов
#include <string.h>	//???
#include <memory>	//std::allocator
#include <iterator>	//от std::iterator наследуем свои итераторы
#include <list>		//std::list
#include <iostream>	//для дебага

namespace str {
using std::list;
//using std::pair;

template<typename X> inline
void my_assert(bool b, X x){
	if(!b)	throw x;
}

struct hex{
  void * x;
  template<class T>
  hex(T m):x((void*)m){}
};
std::ostream & operator<<(std::ostream & str, hex h){
  return str << h.x;
}

#define DEBUG_counter(mes)	//(std::cerr mes << std::endl)
#define DEBUG_buffer(mes)	//(std::cerr mes << std::endl)
#define DEBUG_stream(mes)	//(std::cerr mes << std::endl)

// ----****----
// ----****---- CLASS basic_block_file_c_str ----****----
// ----****----
// стрка, выдающая блоки
template <typename ch_t>
class basic_block_file_c_str
{
	ch_t * _file;
public:
		//CONSTRUCTION DESTRUCTION
	basic_block_file_c_str(ch_t * str)	: _file(str)	{	}
	basic_block_file_c_str() = delete;
	~basic_block_file_c_str() = default;
	
		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
#if __cplusplus >= 201103L
	basic_block_file_c_str & operator=	(const	basic_block_file_c_str &	) = default;	
	basic_block_file_c_str				(const	basic_block_file_c_str &	) = default;
	basic_block_file_c_str & operator=	(		basic_block_file_c_str &&) = default;
	basic_block_file_c_str				(		basic_block_file_c_str &&) = default;
#else
	//todo потом допишем
#endif
	
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
class basic_block_file_FILE
{
	FILE * _file;
public:
		//CONSTRUCTION DESTRUCTION
	basic_block_file_FILE(const char * name, const char * mode){
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
		//todo попробовать с и без FILEйных буферов
		//my_assert(!setvbuf(file,NULL,_IONBF,0),"не получилось отключить буферизацию FILE");
	}
	basic_block_file_FILE() = delete;
	~basic_block_file_FILE(){
		fclose(_file);
	}
	
		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
#if __cplusplus >= 201103L
	basic_block_file_FILE & operator=	(const	basic_block_file_FILE &	) = default;	
	basic_block_file_FILE				(const	basic_block_file_FILE &	) = default;
	basic_block_file_FILE & operator=	(		basic_block_file_FILE &&) = default;
	basic_block_file_FILE				(		basic_block_file_FILE &&) = default;
#else
	//todo потом допишем
#endif
	
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
class string_file_FILE
{
	FILE * _file;
	bool external;
public:
		//CONSTRUCTION DESTRUCTION
	string_file_FILE(const char * name, const char * mode)	: external(false)	{
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
	}
	string_file_FILE(FILE * f)	: _file(f)	, external(true) {
	}

	string_file_FILE() = delete;
	~string_file_FILE(){
		if(external)	return;
		fclose(_file);
	}

		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
#if __cplusplus >= 201103L
	string_file_FILE & operator=	(const	string_file_FILE &	) = default;	
	string_file_FILE				(const	string_file_FILE &	) = default;
	string_file_FILE & operator=	(		string_file_FILE &&) = default;
	string_file_FILE				(		string_file_FILE &&) = default;
#else
	//todo потом допишем
#endif
	
		//MEMBERS
	size_t read(char * buf, size_t size){
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
class wstring_file_FILE
{
	FILE * _file;
	bool external;
public:
		//CONSTRUCTION DESTRUCTION
	wstring_file_FILE(const char * name, const char * mode)	: external(false)	{
		_file = fopen(name,mode);
		my_assert(_file,"не смог открыть файл");
	}
	wstring_file_FILE(FILE * f)	: _file(f), external(true) 	{
	}

	wstring_file_FILE() = delete;
	~wstring_file_FILE()	{
		if(external)	return;
		fclose(_file);
	}

		//COPYING
	//файлы можно копировать как хочешь ибо они не содержат буферов
#if __cplusplus >= 201103L
	wstring_file_FILE & operator=	(const	wstring_file_FILE &	) = default;	
	wstring_file_FILE				(const	wstring_file_FILE &	) = default;
	wstring_file_FILE & operator=	(		wstring_file_FILE &&) = default;
	wstring_file_FILE				(		wstring_file_FILE &&) = default;
#else
	//todo потом допишем
#endif

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

// ----****----
// ----****---- forward defs ----****----
// ----****----
template<class buf_t>
class stream_string;
template<class buf_t>
class _stream_string_const_iterator;
template<class buf_t>
class _stream_string_iterator;

// ----****----
// ----****---- CLASS basic_simple_buffer ----****----
// ----****----
template <typename ch_t, class file_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_simple_buffer 
{
	//можно сделать buf_size = alloc_t().init_page_size()
	//но его из мануалов почему-то убрали
	typedef basic_simple_buffer<ch_t, file_t, buf_size, alloc_t> my_t;
public:
	typedef stream_string<my_t> basic_type;
private:
		//DATA
	basic_type * _base;		//поток
	file_t * _file;			//файл
	
	ch_t * _begin, * _end;	//начало буфера и логический конец буфера (физически может быть больше)
	//буфер обязан завершаться (ch_t)0
	bool _atend; 			//кешируем file->eof()

	int _iterator_counter;
	const int _nomber;		//что бы можно было быстро определить, какой буфер правее, какой левее

public:	//TYPES
	typedef ch_t								value_type;
	typedef alloc_t								allocator_type;
	typedef typename alloc_t::size_type			size_type;
	typedef typename alloc_t::difference_type	difference_type;
	typedef typename alloc_t::pointer			pointer;
	typedef typename alloc_t::const_pointer		const_pointer;
	typedef typename alloc_t::reference			reference;
	typedef typename alloc_t::const_reference	const_reference;
	
	typedef file_t								file_type;		//отличие от STL
	
	//typedef pair<int, const char *> 			tail_type;			//это как пример, здесь можно определить любой тип
	//typedef pair<const char *, const char *> 	stream_data_type;	//это как пример, здесь можно определить любой тип
	//это плохие примеры, у них нет конструктора по умолчанию

	struct tail_type{												//это как пример, здесь можно определить любой тип
		unsigned char size;
		char m[7];			//размер 8 байт удобно иметь для выравнивания в памяти
		tail_type()	: size(0)	{}
		tail_type(int sz, const char * str)
			: size(sz)
		{
			my_assert(size>=0 && size<=7,"неправильный размер хвостика");
			for(int i=0; i<size; i++)
				m[i]=str[i];
		}
	};
	struct stream_data_type{										//это как пример, здесь можно определить любой тип
		const char * enc_in;
		const char * enc_out;
		stream_data_type()	: enc_in(0), enc_out(0)	{}
		stream_data_type(const char * in, const char * out)	: enc_in(in), enc_out(out)	{}
	};
	
		//CONSTRUCTION DESTRUCTION
	/*
	 * поток, файл, хвост, номбер
	 * хвост - неперекодированный кусок предыдущего буфера
	 * может содержать указатели на структуры предыдущего буфера, 
	 * или будет сконструирован по умолчанию
	 * но на всякий случай все равно лучше, что бы было все при себе
	 */
	basic_simple_buffer(basic_type * b, file_t * f, tail_type tail, int n)
		: _base(b)	, _file(f)	, _iterator_counter(0) , _nomber(n)	{
		//т.к. tail() всегда возвращает pair(0,0) в simple_buffer
		my_assert(tail.size==0,"simple_buffer: не пустой хвост");
		_begin = alloc_t().allocate(buf_size);
		_end = _begin+ _file->read(_begin,buf_size-1);
		DEBUG_buffer(<<"буфер #"<<_nomber
			<<"["<<hex(this)<<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex(_begin)<<","<<hex(_end)<<")"
			<<" - конструируется");

		if(_begin == _end) {
			if((_atend=_file->eof()))
				;//it's OK //внезапный конец файла
			else
				throw "прочитано 0 символов, а конца файла нет";
		}
		*_end = 0;
		_atend = _file->eof();
		//параметры перекодировки можно плоучить при помощи stream_data_type _base->stream_data()
	}
	
	basic_simple_buffer()	: _begin(0)	, _iterator_counter(0), _nomber(-1){
		DEBUG_buffer(<<"буфер #"<<_nomber
			<<"["<<hex(this)<<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex(_begin)<<","<<hex(_end)<<")"
			<<" - конструируем по умолчанию");
	}
	~basic_simple_buffer(){
		DEBUG_buffer(<<"буфер #"<<_nomber
			<<"["<<hex(this)<<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex(_begin)<<","<<hex(_end)<<")"
			<<" - разрушаем");
		if(!_begin)	return;
		//эта проверка очень желательна
		if(_iterator_counter)
			std::cerr	<<"basic_simple_buffer #"<<_nomber
						<<": на буфере осталось "<<_iterator_counter<<" итераторов"
						<<std::endl;
		alloc_t().deallocate(_begin,buf_size);	
		_begin = 0;
		_iterator_counter = 0;
	}

		//COPYING
// http://sergeyteplyakov.blogspot.ru/2012/05/c-11-faq.html#default2
#if __cplusplus >= 201103L
	my_t & operator=	(const	my_t &	) = delete;	
	basic_simple_buffer	(const	my_t &	) = delete;
	my_t & operator=	(		my_t &&	) = delete;
	basic_simple_buffer	(		my_t &&	r)
		: _base(r._base)
		, _file(r._file)
		, _begin(r._begin)
		, _end(r._end)
		, _atend(r._atend)
		, _iterator_counter(r._iterator_counter)
		, _nomber(r._nomber)
	{	
		DEBUG_buffer(
			<<"буфер"
			<<"["<<hex(this)<<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex(_begin)<<","<<hex(_end)<<")"
			<<" - конструируем из буфера"
			<<"["<<hex(this)<<"]");
		r._begin=0;
	}
#else //будте осторожны при присваиваниях буфера и передачи в функции
private:
	my_t & operator=(const	my_t &	);
public:
	simple_buffer	(		my_t & r)
		: _begin(r._begin)
		, _end(r._end)
		, _base(r.base)
		, iterator_counter(r.iterator_counter)
		, _nomber(r._nomber)
	{	
		r._begin=0;
	}
#endif //CLASS simple_buffer
		
//{	//PUBLIC MEMBERS
	//хвост - неперекодированный кусок этого буфера
	tail_type 	tail()		{	return tail_type();	}
	
	ch_t *	begin()const	{	return _begin;	}
	ch_t *	end()const		{	return _end;	}
	size_t	size()const		{	return _end-_begin;	}
	bool 	eof()const		{	return _atend;	}
	void 	set_eof()		{	_atend=true;	}
	
	int inc_iterator_counter()	{	return ++_iterator_counter;	}
	int dec_iterator_counter()	{	return --_iterator_counter;	}
	int get_iterator_counter()const{return _iterator_counter;	}

	int 		nomber()const	{	return _nomber;	}
	//int		set_nomber(int n)	{	return _nomber=n;	}	//можно _nomber объявить не как const
	basic_type * base()const	{	return _base;	}
	file_t * 	file()const		{	return _file;	}//на всякий случай
//}	
}; //CLASS simple_buffer

/* здесь можно добавить буфер с перекодировкой, 
 * с вычислением строки-столбца по указателю/итератору
 * с блекджеком
 * со шлюхами
 * smth else
 */

/* требование к буферам:
 * механизм перемещения (не копирования)
 * конструирование типа buf_t(stream_string<buf_t>* base, typename buf_t::file_type *, typename buf_t::tail_type, int nomber)
 * определенные типы:
	typedef typename buf_t::value_type			value_type;
	typedef typename buf_t::size_type			size_type;
	typedef typename buf_t::difference_type		difference_type;
	typedef typename buf_t::pointer				pointer;
	typedef typename buf_t::const_pointer		const_pointer;
	typedef typename buf_t::reference			reference;
	typedef typename buf_t::const_reference		const_reference;
	typedef typename buf_t::file_type			file_type;		//отличие от STL
			typename buf_t::tail_type							//отличие от STL
			typename buf_t::stream_data_type					//отличие от STL
 * методы:
	ch_t *	begin()const	{	return _begin;	}
	ch_t *	end()const		{	return _end;	}
	bool 	eof()const		{	return _atend;	}
	int inc_iterator_counter()	{	return ++_iterator_counter;	}
	int dec_iterator_counter()	{	return --_iterator_counter;	}
	int get_iterator_counter()const{return _iterator_counter;	}
	int 		nomber()const	{	return _nomber;	}
	basic_type * base()const	{	return _base;	}
 * необязательные методы:
	size_t	size()const		{	return _end-_begin;	}
 	void 	set_eof()		{	_atend=true;	}
	int		set_nomber(int n)	{	return _nomber=n;	}
	file_t * 	file()const		{	return _file;	}//на всякий случай

 */

/* итератор и конст_итератор отличаются тем, что разыменованный итератор можно изменять
 * да да, считанное из фала в буфера можно изменять
 */

template<class buf_t> inline
bool atend(const _stream_string_const_iterator<buf_t> & it) {	
	return it.pointer==0;	
}

// ----****----		
// --**-- ITERATOR _stream_string_const_iterator --**--
// ----****----		
template<class buf_t>
class _stream_string_const_iterator
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
	typedef _stream_string_const_iterator<buf_t> 	my_t;
	typedef typename list<buf_t>::iterator 			super_iterator;
		//FRIENDS
	friend class _stream_string_iterator<buf_t>;
	friend bool atend<buf_t>(const my_t & );

		//DATA
	ch_t * pointer;//==0 <=> atend
	ch_t * endbuf;//==0 <=> не связан ни с каким буфером
	super_iterator itbuf;
	
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_stream_string_const_iterator(super_iterator sit): pointer(sit->begin()), endbuf(sit->end()) {
		itbuf->inc_iterator_counter();
	}

	_stream_string_const_iterator()					: pointer(0), endbuf(0)	{
	}
	~_stream_string_const_iterator(){
		if(!pointer)	return;
		if(itbuf->dec_iterator_counter() ==0)
			itbuf->base()->del_buf_request(itbuf);
		pointer=0;
		endbuf=0;
	}
		
		//COPYING
	_stream_string_const_iterator(const my_t & r)	: pointer(r.pointer), endbuf(r.endbuf), itbuf(r.itbuf)	{
		itbuf->inc_iterator_counter();
	}
	my_t & operator=(const my_t & r)	{
		//можно оптимизировать
		this->~my_t();
		pointer=r.pointer;
		endbuf=r.endbuf;
		itbuf=r.itbuf;
		itbuf->inc_iterator_counter();
		return *this;
	}

/*	todo
	_stream_string_const_iterator( my_t && r)
	my_t & operator=( my_t && r)
*/

		//ACCESS
	const ch_t & operator*()const	{	
		return *pointer;	
	}
	const ch_t * operator->()const	{	
		return pointer;	
	}

		//MOVING
	my_t & operator++()	{	// ++myInstance. 
		if(++pointer ==endbuf){//конец внутри буфера
			typename buf_t::basic_type * mybase = itbuf->base();
			super_iterator oldbuf = itbuf++;
			if( itbuf == mybase->_bufs.end() )
				if(oldbuf->eof())							//конец буферов
					this->~_stream_string_const_iterator();	//в последнем буфере был конец файла
				else if( (itbuf=mybase->add_buf()) == mybase->_bufs.end() )
					this->~_stream_string_const_iterator();	//роизошел неожиданный конец файла
				else
					goto normal_buffer;						//новый буфер был удачно создан
			else{
		normal_buffer:
				itbuf->inc_iterator_counter();
				pointer=itbuf->begin();
				endbuf=itbuf->end();
			}
			if(oldbuf->dec_iterator_counter() ==0)
				mybase->del_buf_request(oldbuf);
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
		return itbuf==r.itbuf && pointer==r.pointer;
	}
	bool operator<(const my_t & r)const	{
		my_assert(itbuf->base()==r.itbuf->base(),"сравнение итераторов по разным потокам");
		if(itbuf->nomber()<r.itbuf->nomber())
			return true;
		else if(itbuf->nomber()==r.itbuf->nomber())
			return pointer<r.pointer;
		else
			return false;
	}
};

template<class buf_t> inline
bool atend(const _stream_string_iterator<buf_t> & it) {	
	return it.pointer==0;	
}

// ----****----		
//--**-- TEMPLATE CLASS _stream_string_iterator --**--
// ----****----		
template<class buf_t>
class _stream_string_iterator
	:public 
	std::iterator<
		std::forward_iterator_tag,
		typename buf_t::value_type,
		typename buf_t::difference_type,
		typename buf_t::pointer,
		typename buf_t::reference
	>
{
		//private TYPEDEFS
	typedef typename buf_t::value_type		ch_t;
	typedef _stream_string_iterator<buf_t> 	my_t;
	typedef typename list<buf_t>::iterator 	super_iterator;
		//FRIENDS
	friend class _stream_string_const_iterator<buf_t>;
	friend bool atend<buf_t>(const my_t & );
		//DATA
	ch_t * pointer;//==0 <=> atend
	ch_t * endbuf;//==0 <=> не связан ни с каким буфером
	super_iterator itbuf;
	
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_stream_string_iterator(super_iterator sit)	: pointer(sit->begin()), endbuf(sit->end()), itbuf(sit)	{
		itbuf->inc_iterator_counter();
		DEBUG_counter(<<"str_iterator[" <<hex(this)<<"]"
			<<"("<<hex(pointer)<<","<<hex(endbuf)<<")"
			<<" - конструируем от super_iterator'а по буферу #" <<itbuf->nomber() 
		);
	}

	_stream_string_iterator()				: pointer(0), endbuf(0)	{
		DEBUG_counter(
			<<"str_iterator[" <<hex(this)<<"]"
			<<"("<<hex(pointer)<<","<<hex(endbuf)<<")"
			<<" - конструируем по умолчанию"
		);
	}
	~_stream_string_iterator()	{
		DEBUG_counter(
			<<"str_iterator [" <<hex(this)<<"]"
			<<"("<<hex(pointer)<<","<<hex(endbuf)<<")"
			<<" - разрушаем"
		);
		if(pointer)	{
			if(itbuf->dec_iterator_counter() ==0)
				itbuf->base()->del_buf_request(itbuf);
			pointer=0;
			endbuf=0;
		}
	}
		
		//COPYING
	_stream_string_iterator(const my_t & r)	: pointer(r.pointer), endbuf(r.endbuf), itbuf(r.itbuf)	{
		itbuf->inc_iterator_counter();
	}
	my_t & operator=(const my_t & r){
		//можно оптимизировать
		this->~my_t();
		pointer=r.pointer;
		endbuf=r.endbuf;
		itbuf=r.itbuf;
		itbuf->inc_iterator_counter();
		return *this;
	}

	_stream_string_iterator(const _stream_string_const_iterator<buf_t> & r)
											: pointer(r.pointer), endbuf(r.endbuf), itbuf(r.itbuf)	{
		itbuf->inc_iterator_counter();
	}
	my_t & operator=(const _stream_string_const_iterator<buf_t> & r){
		//можно оптимизировать
		this->~my_t();
		pointer=r.pointer;
		endbuf=r.endbuf;
		itbuf=r.itbuf;
		itbuf->inc_iterator_counter();
		return *this;
	}

/*	todo
	_stream_string_const_iterator( my_t && r)
	my_t & operator=( my_t && r)
*/

		//ACCESS
	ch_t & operator*()const	{	
		return *pointer;	
	}
	ch_t * operator->()const{
		return pointer;	
	}

		//MOVING
	my_t & operator++()	{	// ++myInstance. 
		if(++pointer ==endbuf){//конец внутри буфера
			typename buf_t::basic_type * mybase = itbuf->base();
			super_iterator oldbuf = itbuf++;
			if( itbuf == mybase->_bufs.end() )
				if(oldbuf->eof())							//конец буферов
					this->~_stream_string_iterator();	//в последнем буфере был конец файла
				else if( (itbuf=mybase->add_buf()) == mybase->_bufs.end() )
					this->~_stream_string_iterator();	//роизошел неожиданный конец файла
				else
					goto normal_buffer;						//новый буфер был удачно создан
			else{
		normal_buffer:
				itbuf->inc_iterator_counter();
				pointer=itbuf->begin();
				endbuf=itbuf->end();
			}
			if(oldbuf->dec_iterator_counter() ==0)
				mybase->del_buf_request(oldbuf);
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
		return itbuf==r.itbuf && pointer==r.pointer;
	}
	bool operator<(const my_t & r)const	{
		my_assert(itbuf->base()==r.itbuf->base(),"сравнение итераторов по разным потокам");
		if(itbuf->nomber()<r.itbuf->nomber())
			return true;
		else if(itbuf->nomber()==r.itbuf->nomber())
			return pointer<r.pointer;
		else
			return false;
	}
};

// ----****----		
// ----****---- CONTEINER stream_string ----****----
// ----****----
template<class buf_t>
class stream_string
{
	friend class _stream_string_iterator<buf_t>;
	friend class _stream_string_const_iterator<buf_t>;
	
		//TYPE DEFINES
public:
	typedef typename buf_t::value_type			value_type;
	typedef typename buf_t::size_type			size_type;
	typedef typename buf_t::difference_type		difference_type;
	typedef typename buf_t::pointer				pointer;
	typedef typename buf_t::const_pointer		const_pointer;
	typedef typename buf_t::reference			reference;
	typedef typename buf_t::const_reference		const_reference;

	typedef buf_t								buffer_type;	//отличие от STL
	typedef typename buf_t::file_type			file_type;		//отличие от STL
	typedef _stream_string_iterator<buf_t>			iterator;
	typedef _stream_string_const_iterator<buf_t>	const_iterator;
private:
	//typedef value_type							ch_t; //=> ch_t вообще в буфере не нужно
	typedef stream_string<buf_t>	my_t;
	
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
	typename buf_t::stream_data_type _data;

	iterator _iterator;//internal iterator

		//PRIVATE MEMBERS
	/*
	 * читает новый буфер, и настраивает его номер
	 * если прочитано 0 - уничтожет этот буфер, возвращает _bufs.end() //это означает конец файла
	 * иначе возвращает ук-тель(итератор) на этот буфер
	 */
	typename list<buf_t>::iterator 
	add_buf(){
		//хотя бы один буфер уже суествует, иначе этот метод не будет вызываться
		typename list<buf_t>::iterator ppb= --_bufs.end();//поинтер на предыдущий буфер
		_bufs.push_back(buf_t(this,_file,ppb->tail(),ppb->nomber()+1));
		typename list<buf_t>::iterator pb= --_bufs.end();//поинтер на текущий буфер
		if(pb->begin()==pb->end())//внезапный конец файла
		{
			_bufs.pop_front();
			return _bufs.end();
		}
		return pb;
	}

	//если надо - удаляет буфер
	void del_buf_request(typename list<buf_t>::iterator itbuf){
		if(itbuf==_bufs.begin())
			while(_bufs.begin()->get_iterator_counter()==0 && _bufs.begin()!=_bufs.end())
				_bufs.pop_front();
	}
	
public:	
		//CONSTRUCTION AND DESTRUCTION
	stream_string(file_type * f, typename buf_t::stream_data_type dat= typename buf_t::stream_data_type())
		: _file(f)
		, _data(dat)
	{
		DEBUG_stream(
			<< "stream_string - начали конструировать "
			<< "internal _iterator"
			<<"["<<hex(&_iterator)<<"]"
		);

		_bufs.push_back(buf_t(this,_file,typename buf_t::tail_type(),0));
		DEBUG_stream(
			<<"stream_string - в конструкторе создали первый буфер" 
			<<"["<<hex(&*_bufs.begin())<<"]"
			<<"("<<hex(_bufs.begin()->begin())<<","<<hex(_bufs.begin()->end())<<")"
		);
		if(_bufs.begin()->begin()==_bufs.begin()->end())	{//неожиданный конец файла
		DEBUG_stream( << "неожиданный конец файла" );
			_bufs.pop_front();
			_iterator = iterator();
		}
		else
			_iterator = iterator(_bufs.begin());//он сам сконструируется от итераора на буфер
		DEBUG_stream( << "stream_string - сконструирован" );
	}
	
	stream_string() = delete;
	
	/*
	 * нет итераторов - нет буферов
	 * => перед концом области, где объявлены и контейнер и итераторы
	 * (контейнер и итераторы уничтожаются в произвольном порядке)
	 * все итераторы должны дойти до конца или быть удалены, например путем присваивания им stream.end();
	 * итерторы дошедшие до конца с потоком не связаны
	 */
	~stream_string()	{
		_iterator.~iterator();
		DEBUG_stream(
			<< "internal _iterator"
			<<"["<<hex(&_iterator)<<"]"
			<<" - разрушен"
		);
		if(!_bufs.empty())
			std::cerr<<"при деструктировании потока остались не удаленные буфера"<<std::endl;
		else
			DEBUG_stream( <<"деструктирование потока идет упешно" );
		DEBUG_stream( <<"stream_string - закончили разрушаться" );
		//если потом начнут разрушаться итераторы - это пиздец
	}

		//COPYING
//#if __cplusplus >= 201103L
	my_t & operator=(const	my_t &	) = delete;	
	stream_string	(const	my_t &	) = delete;
	my_t & operator=(		my_t &&	) = delete;
	stream_string	(		my_t &&	) = delete;//возможно можно разрешить
//#else
//private:
//	my_t & operator=	(const my_t &);
//	basic_stream_string	(const my_t &);
//public:
//#endif

//{	//PUBLIC MEMBERS
	typename buf_t::stream_data_type
	stream_data()const
	{	return _data;	}
		
	iterator & 
	internal_iterator()
	{	return _iterator;	}

	iterator *
	pinternal_iterator()
	{	return &_iterator;	}

	list<buf_t> &
	bufs()
	{	return _bufs;	}

	list<buf_t> *
	pbufs()
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


/*
		// TEMPLATE CLASS _stream_string_iterator
template<class T, class Alloc> 
class _stream_string_iterator
	:public _stream_string_const_iterator<T,Alloc>
{
public:
	_stream_string_iterator()	{}
	_stream_string_iterator(const my_t & r)
		:_stream_string_const_iterator(r){}
	//my_t & operator=(const my_t & r)
	//const my_t & operator=(const my_t & r)const
	const T & operator*()const
	{	return *pointer;	}
	const T * operator->()const
	{	return pointer;	}
};
*/

}//namespace str
#endif //MYSTREAM_H
