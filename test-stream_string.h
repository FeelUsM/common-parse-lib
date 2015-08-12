//(c) FeelUs
#ifndef MYSTREAM_H
#define MYSTREAM_H

//#include <deque>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <memory>
#include <iterator>
#include <list>
using std::list;
using std::pair;

#include <iostream>
using std::cerr;
using std::endl;
using std::hex;

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

template<typename X> inline
void my_assert(bool b, X x){
	if(!b)	throw x;
}

#define DEBUG_MES(mes)	(cerr << mes << endl)

namespace str {

// ----****----
// ----****---- CLASS string_file_FILE ----****----
// файл, выдающий строки
// ----****----
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
// ----****---- forward defs ----****----
// ----****----
template<class buf_t>
class stream_string;
class _stream_string_iterator;

/* то что выше, наверно, можно перенести в отдельный какой-нибудь file.h*/

// ----****----
// ----****---- CLASS basic_simple_buffer ----****----
// ----****----
template <typename ch_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_simple_buffer 
{
	//можно сделать buf_size = alloc_t().init_page_size()
	//но его из мануалов почему-то убрали
	typedef basic_simple_buffer<ch_t, buf_size, alloc_t> my_t;
public:
	typedef stream_string<my_t> basic_type;
private:
		//DATA
	basic_type * _base;		//поток
	
	ch_t * _begin, * _end;	//начало буфера и логический конец буфера (физически может быть больше)
	//буфер обязан завершаться (ch_t)0
	bool _atend; 			//кешируем file->eof()

	int _iterator_counter;

public:	//TYPES
	typedef ch_t								value_type;
	typedef typename alloc_t::difference_type	difference_type;
	typedef typename alloc_t::pointer			pointer;
	typedef typename alloc_t::reference			reference;
	

	//typedef pair<int, const char *> 			tail_type;			//это как пример, здесь можно определить любой тип
	//typedef pair<const char *, const char *> 	stream_data_type;	//это как пример, здесь можно определить любой тип
	//это плохие примеры, у них нет конструктора по умолчанию

		//CONSTRUCTION DESTRUCTION
	/*
	 * поток, файл, хвост, номбер
	 * хвост - неперекодированный кусок предыдущего буфера
	 * может содержать указатели на структуры предыдущего буфера, 
	 * или будет сконструирован по умолчанию
	 * но на всякий случай все равно лучше, что бы было все при себе
	 */
	basic_simple_buffer(basic_type * b)
		: _base(b)	, _iterator_counter(0)	{
		//т.к. tail() всегда возвращает pair(0,0) в simple_buffer
		_begin = alloc_t().allocate(buf_size);
		_end = _begin+ 3;
		cerr<<"конструируем буфер"
			<<"["<<hex<< this <<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex<< (void*)_begin <<","<<hex<< (void*)_end <<")"
			<<endl;

		*_end = 0;
		//параметры перекодировки можно плоучить при помощи stream_data_type _base->stream_data()
	}
	
	basic_simple_buffer()	: _begin(0)	, _iterator_counter(0){
		cerr<<"конструируем буфер по умолчанию"
			<<"["<<hex<< this <<"]"
			<<"<"<< _iterator_counter <<">"
			<<"("<<hex<< (void*)_begin <<","<<hex<< (void*)_end <<")"
			<<endl;
	}
	~basic_simple_buffer(){
		cerr<<"разрушаем буфер"
			<<"["<<hex<<this<<"]"
			<<"("<<hex<< (void*)_begin <<","<<hex<< (void*)_end <<")"
			<<endl;
		if(!_begin)	return;
		my_assert(_iterator_counter==0,"simple_buffer: на буфере остались итераторы");
			//эта проверка очень желательна
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
		: _begin(r._begin)
		, _end(r._end)
		, _base(r._base)
		, _iterator_counter(r._iterator_counter)
	{	
		cerr
			<<"конструируем буфер"
			<<"["<<hex<<this<<"]"
			<<"из буфера"
			<<"["<<hex<<&r<<"]"
			<<"("<<hex<< (void*)_begin <<","<<hex<< (void*)_end <<")"
			<<endl;
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
		
#if 1	//PUBLIC MEMBERS

	ch_t *	begin()const	{	return _begin;	}
	ch_t *	end()const		{	return _end;	}
	
	int inc_iterator_counter()	{	return ++_iterator_counter;	}
	int dec_iterator_counter()	{	return --_iterator_counter;	}
	int get_iterator_counter()const{return _iterator_counter;	}

	basic_type * base()const	{	return _base;	}
#endif	
}; //CLASS simple_buffer

const char * test_string="test string";

// ----****----		
//--**-- TEMPLATE CLASS _stream_string_iterator --**--
// ----****----		
class _stream_string_iterator
	:public 
	std::iterator<
		std::forward_iterator_tag,
		char,
		int,
		char*,
		char&
	>
{
		//private TYPEDEFS
	typedef const char		ch_t;
	typedef _stream_string_iterator 	my_t;
	typedef typename list<float>::iterator 	super_iterator;
//далее копия с const_iterator'а, с удаленными const в разделе ACCESS и CONSTRUCTION и во всех вызовах деструкторов 
//а также добавленным конструктором копирования и оператором присваивания от const_iterator'а

		//DATA
	ch_t * pointer;//==0 <=> atend
	ch_t * endbuf;//==0 <=> не связан ни с каким буфером
	super_iterator itbuf;
	
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_stream_string_iterator(super_iterator sit)	: itbuf(sit), pointer(test_string), endbuf(test_string+10){
		cerr<<"конструируем str_iterator"
			<<"[" <<hex<<this <<"]"
			<<"("<<hex<<(int)pointer<<","<<hex<<(int)endbuf<<")"
			<<"от super_iterator'а по буферу[" <<hex<<&*itbuf <<"]" 
			<<endl;
	}

	_stream_string_iterator()				: pointer(0), endbuf(0)	{
		cerr<<"конструируем по умолчанию str_iterator[" <<hex<<this <<"]"
			//<<"("<<hex<<pointer<<","<<hex<<endbuf<<")"
			<<endl;
	}
	~_stream_string_iterator()	{
		cerr << "разрушаем str_iterator [" <<hex<<this <<"](" <<hex<<pointer <<"," <<hex<<endbuf <<")"<< endl;
		if(pointer)	{
			cerr << "и он валидный" << endl;
			pointer=0;
			endbuf=0;
		}
	}
		
		//COPYING
	_stream_string_iterator(const my_t & r)	: pointer(r.pointer), endbuf(r.endbuf), itbuf(r.itbuf)	{
	}
	my_t & operator=(const my_t & r){
		cerr<<"присваиваем в итератор"
			<<"[" <<hex<<this <<"]"
			<<" от итератора"
			<<"[" <<hex<<&r <<"]"
			<<endl;
		//можно оптимизировать
		this->~my_t();
		pointer=r.pointer;
		endbuf=r.endbuf;
		itbuf=r.itbuf;
		return *this;
	}

};

// ----****----		
// ----****---- CONTEINER stream_string ----****----
// ----****----
template<class buf_t>
class stream_string
{
	friend class _stream_string_iterator;
	
		//TYPE DEFINES
public:
	typedef buf_t								buffer_type;	//отличие от STL
	typedef _stream_string_iterator			iterator;
	//не очень нужные типы STL
private:
	//typedef value_type							ch_t; //=> ch_t вообще в буфере не нужно
	typedef stream_string<buf_t>	my_t;
	
		//DATA DEFINES
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
	list<float> _bufs;

	iterator _iterator;//internal iterator

		//PRIVATE MEMBERS
	//если надо - удаляет буфер
	void del_iter_from_buf(typename list<float>::iterator itbuf){
		if( itbuf==_bufs.begin())
			_bufs.pop_front();
	}
	
public:	
		//CONSTRUCTION AND DESTRUCTION
	stream_string()
	{
		cerr << "начали конструировать stream_string" << endl;
		cerr<< "internal _iterator"
			<<"["<<hex<<&_iterator<<"]"
			<<endl;

		_bufs.push_back(17);
		cerr<<"в конструкторе stream_string создали первый буфер" 
			<<"["<<hex<<&*_bufs.begin()<<"]"
			<<endl;
		_iterator = iterator(_bufs.begin());//он сам сконструируется от итераора на буфер
		cerr << "stream_string сконструирован" << endl;
	}
	
	~stream_string()	{
		_iterator.~iterator();
		my_assert(_bufs.empty(),"при деструктировании остались не удаленные буфера");
		//если потом начнут разрушаться итераторы - это пиздец
	}

};//CLASS basic_stream_string

}//namespace str
#endif //MYSTREAM_H
