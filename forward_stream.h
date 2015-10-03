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
файлы на фаловых дескрипторах unix
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

#define DEBUG_fatal(MES)	(std::cerr <<"--------ОШИБКА В ДЕСТРУКТОРЕ: " MES <<std::endl)//никогда не выключать
#define DEBUG_iterator(MES)	//(std::cerr MES <<std::endl)//итераторы: создание, удаление и прыжки между буферами
#define DEBUG_buffer(MES)	//(std::cerr MES <<std::endl)//простой буфер: создание и удаление валидных
#define DEBUG_addrs(MES)	//(std::cerr MES <<std::endl)//адресуемые буферы: создания и изменения адресов
#define DEBUG_stream(MES)	//(std::cerr MES <<std::endl)//поток и его внутренний итератор

//{ debug declarations
struct _hex{
	const void * x;
	_hex(const void * m):x(m){}
};
std::ostream & operator<<(std::ostream & str, _hex h){
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
			if(*d.p<32 && *d.p>=0)
				switch(*d.p){
					case '\n': 	str<<"\\n";	break;
					case '\r': 	str<<"\\r";	break;
					case '\t': 	str<<"\\t";	break;
					case '\v': 	str<<"\\v";	break;
					default:	str<<"\\x"<<std::hex<<(int)*d.p;	break;
				}
			else
				str<<*d.p;
		if(*d.p)
			str<<"...";
	}
	return str;
}

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
	if(!b)	throw stream_exception(x);
}
//}

#define COPYING_DECL(type)\
	my_t & operator=(const	my_t & ) = type;\
	constructor		(const	my_t & ) = type;\
	my_t & operator=(	my_t &&) = type;\
	constructor		(	my_t &&) = type;

// ============ ФАЙЛЫ ============
// ----****----
// ----****---- CLASS basic_file_i ----****----
// ----****----
//basic interface
template <typename ch_t>
class basic_file_i{
#define constructor basic_file_i
typedef basic_file_i<ch_t> my_t;
public:
	virtual ~constructor() = default;
	virtual bool eof()const =0;
	virtual bool good()const=0;
	virtual bool fail()const=0;
	virtual bool bad()const =0;
	virtual size_t read(ch_t * buf, size_t size) =0;
#undef constructor
};

// ----****----
// ----****---- CLASS basic_file_on_cstr_block ----****----
// ----****----
// стрка, выдающая блоки
template <typename ch_t>
class basic_file_on_cstr
	: public basic_file_i<ch_t>
{
#define constructor basic_file_on_cstr
typedef basic_file_on_cstr<ch_t> my_t;
	ch_t * _file;
public:
		//CONSTRUCTION DESTRUCTION COPYING
	constructor(ch_t * str)	: _file(str)	{	}
	constructor() : _file(0){}
	~constructor() = default;// override
	COPYING_DECL(default);
	
		//MEMBERS
	bool eof()const override{	return !*_file;	}
	bool bad()const override{	return _file==0;	}
	bool fail()const override{	return bad();	}
	bool good()const override{	return !fail();	}
	size_t read(ch_t * buf, size_t size)override{
		//todo для специализаций подошли бы и strncpy() и wcsncpy()
		for(size_t i=0; i<size; i++,buf++,_file++)
			if(!(*_file))
				return i;
			else
				*buf = *_file;
		return size;
	}
#undef constructor
};
typedef basic_file_on_cstr<char> file_on_cstr;
typedef basic_file_on_cstr<wchar_t> wfile_on_cstr;
typedef basic_file_on_cstr<char16_t> u16file_on_cstr;
typedef basic_file_on_cstr<char32_t> u32file_on_cstr;

// ----****----
// ----****---- CLASS basic_file_on_FILE_i ----****----
// ----****----
//interface on_FILE
template <typename ch_t>
class basic_file_on_FILE_i
  : public basic_file_i<ch_t>
{
#define constructor basic_file_on_FILE_i
typedef basic_file_on_FILE_i<ch_t> my_t;
typedef basic_file_i<ch_t> base_t;
protected:
	FILE * _file;
	bool _internal;
	void destroy(){	
		if(_internal)	fclose(_file);	
	}
	my_t & init(const 	my_t & r){	
		_file=r._file; _internal=false;	
		return *this;	
	}
	my_t & init(		my_t &&r){	
		_file=r._file; _internal=r._internal; 
		r._internal=false; return *this;	
	}
public:
	constructor() : _file(0),_internal(false){}
	~constructor() override{	destroy();	}

	constructor(const my_t & r)
		:_file(r._file),_internal(false){}
	my_t & operator=(const my_t & r) {
		destroy();	return init(r);
		//this->~my_t();  new(this)my_t(r); return *this;
		//так делать нельзя потому что надо после деструктора 
		//(вызывающего в конце базовый деструктор, а все они инициализируют 
		//указатель на табличку своего типа (что с указателями на виртуальные функции)) 
		//нужно восстановить указатель именно на ту табличку, которая была при вызове
		//а не на табличку типа my_t
		//хотя можно почитать про type_onfo
		//а лучше его вообще не трогать
	}

	constructor( 	 my_t &&r)
		:_file(r._file),_internal(r._internal){
		r._internal=false;
	}
	my_t & operator=(	 my_t &&r) {
		destroy();	return init(r);
	}
	constructor(FILE * f): _file(f),_internal(false) {
		if(_file)
			my_assert(!setvbuf(_file,NULL,_IONBF,0),
				"не получилось отключить буферизацию FILE");
	}

	bool eof()const override{	return feof(_file);	}
	bool bad()const override{	return _file==0;	}
	bool fail()const override{	return bad()||ferror(_file);	}
	bool good()const override{	return !fail();	}
		//ADDED MEMBER
	FILE * file()const{		return _file;	}
	bool has_internal()const{	return _internal;	}
#undef constructor
};

// ----****----
// ----****---- CLASS basic_file_on_FILE_block ----****----
// ----****----
// файл, выдающий блоки
template <typename ch_t>
class basic_file_on_FILE
	: public basic_file_on_FILE_i<ch_t>
{
#define constructor basic_file_on_FILE
typedef basic_file_on_FILE<ch_t> my_t;
typedef basic_file_on_FILE_i<ch_t> base_t;
public:
	COPYING_DECL(default)
	~constructor() = default;//override
	constructor() = default;
	constructor(FILE * f): base_t(f) {}
	constructor(const char * name, const char * mode): base_t(fopen(name,mode)) {  
		base_t::_internal=true;
	}
	size_t read(ch_t * buf, size_t size) override{
		my_assert(base_t::good(),"не смог открыть файл");
		size_t x = fread(buf,sizeof(ch_t),size,base_t::_file);
		return x;
	};
#undef constructor
};
typedef basic_file_on_FILE<char> file_on_FILE;
typedef basic_file_on_FILE<wchar_t> wfile_on_FILE;
typedef basic_file_on_FILE<char16_t> u16file_on_FILE;
typedef basic_file_on_FILE<char32_t> u32file_on_FILE;
	
// ----****----
// ----****---- CLASS file_on_FILE_string ----****----
// ----****----
// файл, выдающий строки, в начале добавляет \n для start_read_line()
class file_on_FILE_stringbuf
	: public basic_file_on_FILE_i<char>
{
#define constructor file_on_FILE_stringbuf
typedef file_on_FILE_stringbuf my_t;
typedef basic_file_on_FILE_i<char> base_t;
	bool init = false;
public:
	COPYING_DECL(default)
	~constructor() = default;//override
	constructor() = default;
	constructor(FILE * f): base_t(f) {}
	constructor(const char * name, const char * mode): base_t(fopen(name,mode)) {  
		base_t::_internal=true;
	}
	size_t read(char * buf, size_t size)override{
		my_assert(size>0,"string_file_on_FILE: запрошенный размер буфера =0");
		if(!init){
			*buf++ = '\n';
			init = true;
			return 1;
		}
		if(fgets(buf,size,_file))
			return strlen(buf);
		else
			return 0;
	}
#undef constructor
};
	
// ----****----
// ----****---- CLASS wfile_on_FILE_string ----****----
// ----****----
// файл, выдающий wстроки, в начале добавляет \n для start_read_line()
class wfile_on_FILE_stringbuf
	: public basic_file_on_FILE_i<wchar_t>
{
#define constructor wfile_on_FILE_stringbuf
typedef wfile_on_FILE_stringbuf my_t;
typedef basic_file_on_FILE_i<wchar_t> base_t;
	bool init = false;
public:
	COPYING_DECL(default)
	~constructor() = default;//override
	constructor() = default;
	constructor(FILE * f): base_t(f) {}
	constructor(const char * name, const char * mode): base_t(fopen(name,mode)) {  
		base_t::_internal=true;
	}
	size_t read(wchar_t * buf, size_t size)override{
		my_assert(size>0,"string_file_on_FILE: запрошенный размер буфера =0");
		if(!init){
			*buf++ = '\n';
			init = true;
			return 1;
		}
		if(fgetws(buf,size,_file))
			return wcslen(buf);
		else
			return 0;
	}
#undef constructor
};

/* здесь можно добавить файлы не на FILE а на файловых дескрипторах unix 
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

//{DEBUG basic_simple_buffer
template <typename ch_t, int buf_size, class alloc_t>
class basic_simple_buffer ;
//буфер #№[адрес структуры] (кол-во итераторов) [размер буфера] 'первые 10 символов содержимого' end?
template <typename ch_t, int buf_size, class alloc_t>
std::ostream & operator<<(std::ostream & str, const basic_simple_buffer<ch_t,buf_size,alloc_t> & b){
	str <<"буфер "
		<<"#" <<b._nomber
		<<"[" <<_hex(&b) <<"]";
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
template <typename ch_t, int buf_size=1025, class alloc_t = std::allocator<ch_t>>
class basic_simple_buffer 
{
#define constructor basic_simple_buffer
	//можно сделать buf_size = alloc_t().init_page_size()
	//но его из мануалов почему-то убрали
	typedef basic_simple_buffer<ch_t, buf_size, alloc_t> my_t;
	template <class T>
	friend class _forward_stream_const_iterator;
	friend std::ostream & operator<< <ch_t,buf_size,alloc_t>(std::ostream & str, const my_t & b);
public:	//TYPEDEFS AND TYPES
	typedef forward_stream<my_t> 				basic_type;	//отличие от STL
	typedef basic_file_i<ch_t> 					file_type;

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
	basic_file_i<ch_t> * 	const _file;	//файл
	const int _nomber;		//что бы можно было быстро определить, какой буфер правее, какой левее
	int _iterator_counter;	//счетчик итераторов, находящихся на этом буфере
//}
protected:
	constructor(basic_type * b, file_type * f, int n)	//доступ к полям выше для наследников
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
	file_type * file()const		{	return _file;	}//на всякий случай
	
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
	constructor(basic_type * b, file_type * f, tail_type tail, int n)	
		: constructor{b,f,n}	{
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
	
	constructor()	: constructor{0,0,-1}	{
		_begin = 0;
		//DEBUG_buffer(<<*this<<" - сконструирован по умолчанию");
	}
	~constructor(){
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
	my_t & operator=(const	my_t &	) = delete;	
	constructor		(const	my_t &	) = delete;
	my_t & operator=(		my_t &&	) = delete;
	constructor		(		my_t &&	r)	: _base(r._base), _file(r._file), _nomber(r._nomber)	{	
		_iterator_counter=r._iterator_counter;
		_begin			= r._begin;
		_end			= r._end;
		_atend			= r._atend;
		r._begin=0;
		//DEBUG_buffer(<<*this <<" - сконструирован из - " <<r);
	}
#undef constructor
}; //CLASS simple_buffer
typedef basic_simple_buffer<char> simple_buffer;
typedef basic_simple_buffer<wchar_t> simple_wbuffer;
typedef basic_simple_buffer<char16_t> simple_u16buffer;
typedef basic_simple_buffer<char32_t> simple_u32buffer;

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
#define constructor _forward_stream_const_iterator
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
	ch_t * 			point;//==0 <=> atend <=>(def) инвалидный
	ch_t * 			endbuf;
	super_iterator 	itbuf;
	
	int buf_iterator_counter()const		{	return itbuf->_iterator_counter;	}
public:	//GETTERS - только ради получения доступа на чтения не надо наследоваться
	ch_t * 			get_point()const	{	return point;	}
	ch_t * 			get_endbuf()const	{	return endbuf;	}
	super_iterator 	get_itbuf()const	{	return itbuf;	}
		//CONSTRUCTION, DESTRUCTION
	explicit // создает итератор, указывающий на начало буфера
	constructor(super_iterator sit): point(sit->begin()), endbuf(sit->end()), itbuf(sit) {
		itbuf->_iterator_counter++;
		DEBUG_iterator(<<*this <<" - сконструировали от super_iterator'а");
	}

	constructor()					: point(0)	{
		DEBUG_iterator(<<*this <<" - сконструировали по умолчанию");
	}
	~constructor(){
		DEBUG_iterator(<<*this <<" - разрушаем");
		if(!point)	return;
		if(--itbuf->_iterator_counter ==0)
			itbuf->base()->del_buf_request(itbuf);
		point=0;
	}
		
		//COPYING
	constructor(const my_t & r)	: point(r.point), endbuf(r.endbuf), itbuf(r.itbuf)	{
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
#undef constructor
};

// ----****----		
// ----****---- TEMPLATE CLASS _forward_stream_iterator ----****----
// ----****----		
template<class buf_t>
class _forward_stream_iterator
	:public _forward_stream_const_iterator<buf_t>
	//для него iterator_traits определен отдельно внизу
{
#define constructor _forward_stream_iterator
		//private TYPEDEFS
	typedef typename buf_t::value_type		ch_t;
	typedef _forward_stream_iterator<buf_t> 	my_t;
	typedef _forward_stream_const_iterator<buf_t> parent_t;
	typedef typename list<buf_t>::iterator 	super_iterator;
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	constructor(super_iterator sit)	: parent_t(sit)	{	}

	constructor()					: parent_t()	{	}
	~constructor()					= default;
		
		//COPYING
	constructor		(const parent_t & r)	: parent_t(r)	{	}
	my_t & operator=(const parent_t & r)	{	parent_t::operator=(r);	return *this;	}
/*	todo
	_forward_stream_const_iterator( my_t && r)
	my_t & operator=( my_t && r)
*/

		//ACCESS
	ch_t & operator*()const		{	return const_cast<ch_t&>  (parent_t::operator*());		}
	ch_t * operator->()const	{	return const_cast<ch_t*> (parent_t::operator->());	}

		//MOVING - наследуется
		//ARITHMETIC - наследуется
#undef constructor
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
struct linecol{ //eof <=> 0:0
	int line,col;
	linecol(int l=1, int c=1):line(l),col(c){}
	template<class it_t>
	explicit linecol(const it_t & it){	*this = get_linecol(it);	}
};
inline
bool operator==(linecol l, linecol r){
	return l.line==r.line && l.col==r.col;
}
inline
std::ostream & operator<<(std::ostream & str, linecol lc){
	return str <<lc.line <<":" <<lc.col;
}


template <typename ch_t, int buf_size, class alloc_t>
class basic_adressed_buffer;

template <typename ch_t, int buf_size, class alloc_t>
linecol get_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,buf_size,alloc_t> > & it);
template <typename ch_t, int buf_size, class alloc_t>
void set_linecol(_forward_stream_const_iterator<basic_adressed_buffer<ch_t,buf_size,alloc_t> > & it, linecol lc);

//{ DEBUG basic_adressed_buffer
template <typename ch_t, int buf_size, class alloc_t>
std::ostream & operator<<(std::ostream & str, const basic_adressed_buffer<ch_t,buf_size,alloc_t> & b){
	typedef basic_simple_buffer<ch_t,buf_size,alloc_t> parent_t;
	str <<(const parent_t &)b;
	b.printNLs(str);	
	return str;
}
//}
// ----****----
// ----****---- CLASS basic_adressed_buffer ----****----
// ----****----
template <typename ch_t, int buf_size=1025, class alloc_t = std::allocator<ch_t>>
class basic_adressed_buffer : public basic_simple_buffer<ch_t,buf_size,alloc_t>
{
#define constructor basic_adressed_buffer
	typedef basic_simple_buffer<ch_t,buf_size,alloc_t> parent_t;
	typedef basic_adressed_buffer<ch_t, buf_size, alloc_t> my_t;
	friend linecol 	get_linecol<ch_t,buf_size,alloc_t>(const _forward_stream_const_iterator<my_t> &);
	friend void 	set_linecol<ch_t,buf_size,alloc_t>(_forward_stream_const_iterator<my_t> & , linecol);
	friend std::ostream & operator<< <ch_t,buf_size,alloc_t>(std::ostream & str, const my_t & b);
	
	// самый первый символ и каждый символ ПОСЛЕ '\n' содержится в таблице
	typedef std::vector<pair<const ch_t *,linecol>> NLs_type;//а можно было и map сделать
	std::ostream & printNLs(std::ostream & str)const{
		if(NLs.empty())	
			str<<"void";
		else
			for(typename NLs_type::const_iterator it=NLs.begin(); it!=NLs.end(); it++)
				str <<"," <<it->first-parent_t::begin() <<"("<<it->second <<")";
		return str;
	}
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
	typedef typename parent_t::file_type		file_type;
	typedef linecol tail_type;
	basic_type * base()const	{	return reinterpret_cast<basic_type*>(parent_t::base());	}
	tail_type 	tail()const {
		my_assert(!NLs.empty(),"в буфере с адресацией не задана адресация");
		return tail_type(
			(--NLs.end())->second.line, 
			(--NLs.end())->second.col + parent_t::end()- (--NLs.end())->first);	
	}
	constructor(basic_type * b, file_type * f, tail_type lc, int n)
	:parent_t(reinterpret_cast<typename parent_t::basic_type*>(b),f,typename parent_t::tail_type(),n)
	{
		NLs_init(lc);
		DEBUG_addrs(<<*this << " - сконструирован");
	}
		//COPYING
	my_t & operator=(const	my_t &	) = delete;	
	constructor		(const	my_t &	) = delete;
	my_t & operator=(		my_t &&	) = delete;
	constructor		(		my_t &&	r)	: parent_t(static_cast<my_t&&>(r)),NLs(r.NLs){}	
#undef constructor
};
typedef basic_adressed_buffer<char> adressed_buffer;
typedef basic_adressed_buffer<wchar_t> adressed_wbuffer;
typedef basic_adressed_buffer<char16_t> adressed_u16buffer;
typedef basic_adressed_buffer<char32_t> adressed_u32buffer;

template <typename ch_t> inline
bool _NL_pair_lt(const pair<const ch_t *,linecol> & l, const pair<const ch_t *,linecol> & r){
	return l.first < r.first;
}

template <typename ch_t, int buf_size, class alloc_t>
linecol get_linecol(const _forward_stream_const_iterator<basic_adressed_buffer<ch_t,buf_size,alloc_t> > & it){
	if(atend(it))	return linecol(0,0);
	typename basic_adressed_buffer<ch_t,buf_size,alloc_t>::NLs_type::iterator q= 
		std::upper_bound(it.get_itbuf()->NLs.begin(),it.get_itbuf()->NLs.end(),
			make_pair(it.get_point(),linecol()),_NL_pair_lt<ch_t>);
	q--;
	DEBUG_addrs(<<*it.get_itbuf() <<" - для позиции " <<(void*)it.get_point() <<" найдено: " <<(void*)q->first <<"("<<q->second<<")");
	return linecol(q->second.line,q->second.col+it.get_point()-q->first);
}

template <typename ch_t, int buf_size, class alloc_t>
void set_linecol(_forward_stream_const_iterator<basic_adressed_buffer<ch_t,buf_size,alloc_t> > & it, linecol lc){
	//DEBUG_addrs(<<*it.get_itbuf() <<" - ищу "<<(it.get_point()-it.get_itbuf()->begin()) );
	typedef typename basic_adressed_buffer<ch_t,buf_size,alloc_t>::NLs_type NLs_t;
 	NLs_t & _NLs = it.get_itbuf()->NLs;
	typename NLs_t::iterator q= 
		std::lower_bound(_NLs.begin(),_NLs.end(),
			make_pair(it.get_point(),linecol()),_NL_pair_lt<ch_t>);
	/*
	std::cerr <<"нашел ";
	if(q==it.get_itbuf()->NLs.end())
		std::cerr <<"end" <<std::endl;
	else
		std::cerr <<q->first-it.get_itbuf()->begin() <<"("<<q->second.line<<","<<q->second.col<<")"<<std::endl;
	*/
	if(q!=it.get_itbuf()->NLs.end() && q->first == it.get_point())//вставляем или переприсваиваем
		q->second = lc;
	else{
		it.get_itbuf()->NLs.insert(q,make_pair(it.get_point(),lc));
		q++;
	}
	
	q++;
	size_t line=lc.line;
	line++;
	for(; q!=it.get_itbuf()->NLs.end(); q++,line++)//доделываем текущий буфер
		q->second.line = line;
	DEBUG_addrs(<<"в результате установки " <<lc <<" на " <<it <<" получаем " <<*it.get_itbuf() );
	typedef basic_adressed_buffer<ch_t,buf_size,alloc_t> buf_t;
	//обрабатываем последующие буфера, NLs.init() не пригодился
	for(typename list<buf_t>::iterator bit = ++it.get_itbuf() ; bit!=it.get_itbuf()->base()->bufs().end(); bit++){
		for(q=it.get_itbuf()->NLs.begin() ; q!=it.get_itbuf()->NLs.end(); q++,line++)
			q->second.line = line;
		DEBUG_addrs(<<"наведенное изменение lc " <<*it.get_itbuf() );
	}
}

template <typename ch_t, int buf_size, class alloc_t>
void goto_linecol(_forward_stream_const_iterator<basic_adressed_buffer<ch_t,buf_size,alloc_t> > * it, linecol lc){
	
}

/* здесь можно добавить буфер с перекодировкой, 
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
template <typename ch_t, int buf_size=512, class alloc_t = std::allocator<ch_t>>
class basic_example_buffer : public basic_simple_buffer<ch_t,buf_size,alloc_t>
{
#define constructor basic_example_buffer
	typedef basic_simple_buffer<ch_t,buf_size,alloc_t> parent_t;
	typedef basic_example_buffer<ch_t, buf_size, alloc_t> my_t;
public:
	typedef _forward_stream_const_iterator<my_t>const_iterator;
	typedef _forward_stream_iterator<my_t>		iterator;
	typedef forward_stream<my_t> 				basic_type;
	typedef typename parent_t::file_type		file_type;
	typedef linecol tail_type;
	basic_type * base()const{	return reinterpret_cast<basic_type*>(parent_t::base());	}
	tail_type 	tail()		{	return tail_type();	}
	constructor(basic_type * b, file_type * f, tail_type tail, int n)
	:parent_t(reinterpret_cast<typename parent_t::basic_type*>(b),f,typename parent_t::tail_type(),n)
	{}
		//COPYING
	my_t & operator=(const	my_t &	) = delete;
	constructor		(const	my_t &	) = delete;
	my_t & operator=(		my_t &&	) = delete;
	constructor		(		my_t &&	r)	: parent_t(static_cast<my_t&&>(r)){}
#undef constructor
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
#define constructor forward_stream
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
	typedef typename buf_t::file_type 			file_type;
	typedef typename buf_t::tail_type 			tail_type;
	typedef typename buf_t::stream_data_type 	stream_data_type;
	
private: //{
	//typedef value_type							ch_t; //=> ch_t вообще в буфере не нужно
	typedef forward_stream<buf_t>	my_t;
	
		//DATA DEFINES
	file_type * _file;
	bool need_del_file;
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
	constructor(bool ndf, file_type * f, typename buf_t::stream_data_type dat= typename buf_t::stream_data_type())
		: _file(f)
		, need_del_file(ndf)
		, _data(dat)
	{
		if(_file->good()){
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
	}
	
	explicit
	constructor(file_type * f, typename buf_t::stream_data_type dat= typename buf_t::stream_data_type())
		: constructor(false,f,dat){}

	/*
	 * нет итераторов - нет буферов
	 * => перед концом области, где объявлены и контейнер и итераторы
	 * (контейнер и итераторы уничтожаются в произвольном порядке)
	 * все итераторы должны дойти до конца или быть удалены, например путем присваивания им stream.end();
	 * итерторы дошедшие до конца с потоком не связаны
	 */
	~constructor()	{
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
		if(need_del_file)
			delete _file;
		//если потом начнут разрушаться итераторы - это пиздец
	}
	constructor() = delete;
	COPYING_DECL(delete);

//{	//PUBLIC MEMBERS
	bool eof()const {	return _file->eof();	}
	bool good()const{	return _file->good();	}
	bool fail()const{	return _file->fail();	}
	bool bad()const {	return _file->bad();	}
	operator bool()const {	return good();	}

	typename buf_t::stream_data_type
	stream_data()const
	{	return _data;	}
		
	iterator & 
	iter()
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
#undef constructor
};//CLASS basic_stream_string
typedef forward_stream<adressed_buffer> 	forward_adressed_stream;
typedef forward_stream<adressed_wbuffer> 	forward_adressed_wstream;
typedef forward_stream<adressed_u16buffer> 	forward_adressed_u16stream;
typedef forward_stream<adressed_u32buffer> 	forward_adressed_u32stream;
typedef forward_stream<simple_buffer> 	forward_simple_stream;
typedef forward_stream<simple_wbuffer> 	forward_simple_wstream;
typedef forward_stream<simple_u16buffer> 	forward_simple_u16stream;
typedef forward_stream<simple_u32buffer> 	forward_simple_u32stream;

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
