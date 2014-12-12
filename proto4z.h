﻿/*
 * proto4z License
 * -----------
 * 
 * proto4z is licensed under the terms of the MIT license reproduced below.
 * This means that proto4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2013-2014 YaweiZhang <yawei_zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */


/*
 * AUTHORS:  YaweiZhang <yawei_zhang@foxmail.com>
 * VERSION:  1.0
 * PURPOSE:  A lightweight library for process protocol .
 * CREATION: 2013.07.04
 * LCHANGE:  2014.08.20
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */

/*
 * Web Site: www.zsummer.net
 * mail: yawei_zhang@foxmail.com
 */

/* 
 * UPDATES LOG
 * 
 * VERSION 0.1.0 <DATE: 2013.07.4>
 * 	create the first project.  
 * 	support big-endian or little-endian
 * VERSION 0.3.0 <DATE: 2014.03.17>
 *  support user-defined header
 *  WriteStream support auto alloc memory or attach exist memory
 *  proto4z support stl container
 * VERSION 0.4.0 <DATE: 2014.05.16>
 *  Add some useful interface method
 * VERSION 0.5.0 <DATE: 2014.08.06>
 *  Add static buff for optimize
 *  Add genProto tools
 * VERSION 1.0.0 <DATE: 2014.08.20>
 *  Add HTTP proto
 * VERSION 1.1.0 <DATE: 2014.11.11>
 *  support http chunked header
 *  support http decode and encode method
 * 
 */
#pragma once
#ifndef _PROTO4Z_H_
#define _PROTO4Z_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <deque>
#include <assert.h>
#ifndef WIN32
#include <stdexcept>
#else
#include <exception>
#endif
#ifndef _ZSUMMER_BEGIN
#define _ZSUMMER_BEGIN namespace zsummer {
#endif  
#ifndef _ZSUMMER_PROTO4Z_BEGIN
#define _ZSUMMER_PROTO4Z_BEGIN namespace proto4z {
#endif
_ZSUMMER_BEGIN
_ZSUMMER_PROTO4Z_BEGIN

#ifdef WIN32
#pragma warning(disable:4996)
#endif

enum ZSummer_EndianType
{
	BigEndian,
	LittleEndian,
};





//////////////////////////////////////////////////////////////////////////
//! protocol traits instruction
//////////////////////////////////////////////////////////////////////////

// Memory layout
//|-----   header  -----|-------  body  --------|
//|---------------pack len----------------------| //PackLenIsContainHead
//|---------------------|-------pack len--------| //not PackLenIsContainHead
//
struct DefaultStreamHeadTraits
{
	typedef unsigned short Integer; //User-Defined Integer Type must in [unsigned char, unsigned short, unsigned int, unsigned long long].
	const static Integer MaxPackLen = (Integer)-1; //User-Defined. example:  Integer = unsigned short(-1) ==>(65535)
	const static bool	 PackLenIsContainHead = true; //User-Defined 
	const static ZSummer_EndianType EndianType = LittleEndian;//User-Defined 
	const static Integer HeadLen = sizeof(Integer); //Don't Touch. Head Length.
};

//protocol Traits example, User-Defined
struct TestBigStreamHeadTraits
{
	typedef unsigned int Integer;
	const static Integer MaxPackLen = -1;
	const static bool PackLenIsContainHead = false;
	const static ZSummer_EndianType EndianType = BigEndian;
	const static Integer HeadLen = sizeof(Integer); //Don't Touch.
};

//stream translate to Integer with endian type.
template<class Integer, class StreamHeadTrait>
Integer streamToInteger(const char stream[sizeof(Integer)]);

//integer translate to stream with endian type.
template<class Integer, class StreamHeadTrait>
void integerToStream(Integer integer, char *stream);

//!get runtime local endian type. 
static const unsigned short __gc_localEndianType = 1;
inline ZSummer_EndianType __localEndianType();


//////////////////////////////////////////////////////////////////////////
//! get the residue length of packet  on the information received.
//////////////////////////////////////////////////////////////////////////


enum INTEGRITY_RET_TYPE
{
	IRT_SUCCESS = 0,
	IRT_SHORTAGE = 1,
	IRT_CORRUPTION = 2,
};
//! return value:
//! first: IRT_SUCCESS data integrity. second: current integrity data lenght.
//! first: IRT_SHORTAGE data not integrity. second: shortage lenght.
//! first: IRT_CORRUPTION data corruption. second: data lenght
template<class StreamHeadTrait>
inline std::pair<INTEGRITY_RET_TYPE, typename StreamHeadTrait::Integer>
CheckBuffIntegrity(const char * buff, typename StreamHeadTrait::Integer curBuffLen, 
typename StreamHeadTrait::Integer maxBuffLen /*= StreamHeadTrait::MaxPackLen*/);



//////////////////////////////////////////////////////////////////////////
//! class WriteStream: serializes the specified data to byte stream.
//////////////////////////////////////////////////////////////////////////
//StreamHeadTrait: User-Defined like DefaultStreamHeadTrait
//AllocType: inner allocate memory used this, default use std::allocator<char>

enum UserBuffType
{
	UBT_INVALIDE = 0,
	UBT_ATTACH, // attach exist buff.
	UBT_AUTO, //use std::string
	UBT_STATIC_AUTO, //use static std::string
};

template<class StreamHeadTrait/*=DefaultStreamHeadTraits*/, class _Alloc = std::allocator<char> >
class WriteStream
{
public:
	typedef typename StreamHeadTrait::Integer Integer;
	//! maxStreamLen : The maximum length can be written.
	//! bNoWrite : if true then WriteStream will not do any write operation.
	//! attachData : Attach to the existing memory.
	WriteStream(UserBuffType ubt = UBT_AUTO, Integer maxStreamLen = StreamHeadTrait::MaxPackLen);//Automatically allocate memory
	WriteStream(char * attachData, Integer maxStreamLen, bool bNoWrite = false);// attach exist memory
	~WriteStream(){}
public:
	//get total stream buff, the pointer must be used immediately.
	inline char* getStream();
	//get total stream length.
	inline Integer getStreamLen(){return _cursor;}

	//get body stream buff, the pointer used by reflecting immediately.
	inline char* getStreamBody();
	//get body stream length.
	inline Integer getStreamBodyLen(){ return _cursor - StreamHeadTrait::HeadLen; }

	//write original data.
	inline WriteStream<StreamHeadTrait> & appendOriginalData(const void * data, Integer unit);


	inline WriteStream<StreamHeadTrait> & operator << (bool data) { return writeSimpleData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (char data) { return writeSimpleData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (unsigned char data) { return writeSimpleData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (short data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (unsigned short data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (int data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (unsigned int data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (long data) { return writeIntegerData((long long)data); }
	inline WriteStream<StreamHeadTrait> & operator << (unsigned long data) { return writeIntegerData((unsigned long long)data); }
	inline WriteStream<StreamHeadTrait> & operator << (long long data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (unsigned long long data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (float data) { return writeIntegerData(data); }
	inline WriteStream<StreamHeadTrait> & operator << (double data) { return writeIntegerData(data); }

protected:
	//! check move cursor is valid. if invalid then throw exception.
	inline void checkMoveCursor(Integer unit = (Integer)0);

	//! fix pack len.
	inline void fixPackLen();

	//! write integer data with endian type.
	template <class T>
	inline WriteStream<StreamHeadTrait> & writeIntegerData(T t);

	//! write some types of data with out endian type. It's relative to writeIntegerData method.
	template <class T>
	inline WriteStream<StreamHeadTrait> & writeSimpleData(T t);
private:

	UserBuffType _usedBuffType;
	std::basic_string<char, std::char_traits<char>, _Alloc > _data; //! If not attach any memory, class WriteStream will used this to managing memory.
	static std::basic_string<char, std::char_traits<char>, _Alloc > _staticData; //! warning: single-thread, static buff.
	char * _attachData;//! attach memory pointer
	Integer _maxStreamLen; //! can write max size
	Integer _cursor; //! current move cursor.
	bool  _isNoWrite; //! if true then WriteStream will not do any write operation.
};
template<class StreamHeadTrait/*=DefaultStreamHeadTraits*/, class _Alloc>
std::basic_string<char, std::char_traits<char>, _Alloc > WriteStream<StreamHeadTrait, _Alloc>::_staticData;
//////////////////////////////////////////////////////////////////////////
//class ReadStream: De-serialization the specified data from byte stream.
//////////////////////////////////////////////////////////////////////////

//StreamHeadTrait: User-Defined like DefaultStreamHeadTrait
template<class StreamHeadTrait/*=DefaultStreamHeadTraits*/>
class ReadStream
{
public:
	typedef typename StreamHeadTrait::Integer Integer;
	ReadStream(const char *attachData, Integer attachDataLen);
	~ReadStream(){}
public:

	inline void resetMoveCursor(){ _cursor = StreamHeadTrait::HeadLen;}
	//get attach data buff
	inline const char* getStream();
	//get pack length in stream
	inline Integer getStreamLen();

	//get body stream buff, the pointer used by reflecting immediately.
	inline const char* getStreamBody();
	//get body stream length.
	inline Integer getStreamBodyLen();

	//get current unread stream buff, the pointer used by reflecting immediately.
	inline const char* getStreamUnread();
	//get current unread stream buff length
	inline Integer getStreamUnreadLen();


	inline const char * peekOriginalData(Integer unit);
	inline void skipOriginalData(Integer unit);


	inline ReadStream<StreamHeadTrait> & operator >> (bool & data) { return readSimpleData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (char & data) { return readSimpleData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (unsigned char & data) { return readSimpleData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (short & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (unsigned short & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (int & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (unsigned int & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (long & data){ long long tmp = 0;ReadStream & ret = readIntegerData(tmp);data =(long) tmp;return ret;}
	inline ReadStream<StreamHeadTrait> & operator >> (unsigned long & data){ unsigned long long tmp = 0;ReadStream & ret = readIntegerData(tmp);data = (unsigned long)tmp;return ret;}
	inline ReadStream<StreamHeadTrait> & operator >> (long long & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (unsigned long long & data) { return readIntegerData(data); }
	inline ReadStream<StreamHeadTrait> & operator >> (float & data) { return readIntegerData(data); } 
	inline ReadStream<StreamHeadTrait> & operator >> (double & data) { return readIntegerData(data); }
protected:
	inline void checkMoveCursor(Integer unit);
	template <class T>
	inline ReadStream<StreamHeadTrait> & readIntegerData(T & t);
	template <class T>
	inline ReadStream<StreamHeadTrait> & readSimpleData(T & t);

private:
	const char * _attachData;
	Integer _maxDataLen;
	Integer _cursor;
};


//////////////////////////////////////////////////////////////////////////
//! stl container
//////////////////////////////////////////////////////////////////////////

//write c-style string
template<class StreamHeadTrait>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const char *const data)
{
	typename StreamHeadTrait::Integer len = (typename StreamHeadTrait::Integer)strlen(data);
	ws << len;
	ws.appendOriginalData(data, len);
	return ws;
}

//write std::string
template<class StreamHeadTrait, class _Traits, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::basic_string<char, _Traits, _Alloc> & data)
{
	typename StreamHeadTrait::Integer len = (typename StreamHeadTrait::Integer)data.length();
	ws << len;
	ws.appendOriginalData(data.c_str(), len);
	return ws;
}
//read std::string
template<class StreamHeadTrait, class _Traits, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::basic_string<char, _Traits, _Alloc> & data)
{
	typename StreamHeadTrait::Integer len = 0;
	rs >> len;
	data.assign(rs.peekOriginalData(len), len);
	rs.skipOriginalData(len);
	return rs;
}


//std::vector
template<class StreamHeadTrait, class T, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::vector<T, _Alloc> & vct)
{
	ws << (typename StreamHeadTrait::Integer)vct.size();
	for (typename std::vector<T, _Alloc>::const_iterator iter = vct.begin(); iter != vct.end(); ++iter)
	{
		ws << *iter;
	}
	return ws;
}

template<class StreamHeadTrait, typename T, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::vector<T, _Alloc> & vct)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		T t;
		rs >> t;
		vct.push_back(t);
	}
	return rs;
}

//std::set
template<class StreamHeadTrait, class Key, class _Pr, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::set<Key, _Pr, _Alloc> & k)
{
	ws << (typename StreamHeadTrait::Integer)k.size();
	for (typename std::set<Key, _Pr, _Alloc>::const_iterator iter = k.begin(); iter != k.end(); ++iter)
	{
		ws << *iter;
	}
	return ws;
}

template<class StreamHeadTrait, class Key, class _Pr, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::set<Key, _Pr, _Alloc> & k)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		Key t;
		rs >> t;
		k.insert(t);
	}
	return rs;
}

//std::multiset
template<class StreamHeadTrait, class Key, class _Pr, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::multiset<Key, _Pr, _Alloc> & k)
{
	ws << (typename StreamHeadTrait::Integer)k.size();
	for (typename std::multiset<Key, _Pr, _Alloc>::const_iterator iter = k.begin(); iter != k.end(); ++iter)
	{
		ws << *iter;
	}
	return ws;
}

template<class StreamHeadTrait, class Key, class _Pr, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::multiset<Key, _Pr, _Alloc> & k)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		Key t;
		rs >> t;
		k.insert(t);
	}
	return rs;
}

//std::map
template<class StreamHeadTrait, class Key, class Value, class _Pr, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::map<Key, Value, _Pr, _Alloc> & kv)
{
	ws << (typename StreamHeadTrait::Integer)kv.size();
	for (typename std::map<Key, Value, _Pr, _Alloc>::const_iterator iter = kv.begin(); iter != kv.end(); ++iter)
	{
		ws << iter->first;
		ws << iter->second;
	}
	return ws;
}

template<class StreamHeadTrait, class Key, class Value, class _Pr, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::map<Key, Value, _Pr, _Alloc> & kv)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		std::pair<Key, Value> pr;
		rs >> pr.first;
		rs >> pr.second;
		kv.insert(pr);
	}
	return rs;
}

//std::multimap
template<class StreamHeadTrait, class Key, class Value, class _Pr, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::multimap<Key, Value, _Pr, _Alloc> & kv)
{
	ws << (typename StreamHeadTrait::Integer)kv.size();
	for (typename std::multimap<Key, Value, _Pr, _Alloc>::const_iterator iter = kv.begin(); iter != kv.end(); ++iter)
	{
		ws << iter->first;
		ws << iter->second;
	}
	return ws;
}

template<class StreamHeadTrait, class Key, class Value, class _Pr, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::multimap<Key, Value, _Pr, _Alloc> & kv)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		std::pair<Key, Value> pr;
		rs >> pr.first;
		rs >> pr.second;
		kv.insert(pr);
	}
	return rs;
}


//std::list
template<class StreamHeadTrait, class Value, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::list<Value, _Alloc> & l)
{
	ws << (typename StreamHeadTrait::Integer)l.size();
	for (typename std::list<Value,_Alloc>::const_iterator iter = l.begin(); iter != l.end(); ++iter)
	{
		ws << *iter;
	}
	return ws;
}

template<class StreamHeadTrait, class Value, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::list<Value, _Alloc> & l)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		Value t;
		rs >> t;
		l.push_back(t);
	}
	return rs;
}
//std::deque
template<class StreamHeadTrait, class Value, class _Alloc>
inline WriteStream<StreamHeadTrait> & operator << (WriteStream<StreamHeadTrait> & ws, const std::deque<Value, _Alloc> & l)
{
	ws << (typename StreamHeadTrait::Integer)l.size();
	for (typename std::deque<Value,_Alloc>::const_iterator iter = l.begin(); iter != l.end(); ++iter)
	{
		ws << *iter;
	}
	return ws;
}

template<class StreamHeadTrait, class Value, class _Alloc>
inline ReadStream<StreamHeadTrait> & operator >> (ReadStream<StreamHeadTrait> & rs, std::deque<Value, _Alloc> & l)
{
	typename StreamHeadTrait::Integer totalCount = 0;
	rs >> totalCount;
	for (typename StreamHeadTrait::Integer i = 0; i < totalCount; ++i)
	{
		Value t;
		rs >> t;
		l.push_back(t);
	}
	return rs;
}



//////////////////////////////////////////////////////////////////////////
//! implement 
//////////////////////////////////////////////////////////////////////////

inline ZSummer_EndianType __localEndianType()
{
	if (*(const unsigned char *)&__gc_localEndianType == 1)
	{
		return LittleEndian;
	}
	return BigEndian;
}

template<class Integer, class StreamHeadTrait>
Integer streamToInteger(const char stream[sizeof(Integer)])
{
	unsigned short integerLen = sizeof(Integer);
	Integer integer = 0 ;
	if (integerLen == 1)
	{
		integer = (Integer)stream[0];
	}
	else
	{
		if (StreamHeadTrait::EndianType != __localEndianType())
		{
			unsigned char *dst = (unsigned char*)&integer;
			unsigned char *src = (unsigned char*)stream + integerLen;
			while (integerLen > 0)
			{
				*dst++ = *--src;
				integerLen --;
			}
		}
		else
		{
			memcpy(&integer, stream, integerLen);
		}
	}
	return integer;
}

template<class Integer, class StreamHeadTrait>
void integerToStream(Integer integer, char *stream)
{
	unsigned short integerLen = sizeof(Integer);
	if (integerLen == 1)
	{
		stream[0] = (char)integer;
	}
	else
	{
		if (StreamHeadTrait::EndianType != __localEndianType())
		{
			unsigned char *src = (unsigned char*)&integer + integerLen;
			unsigned char *dst = (unsigned char*)stream;
			while (integerLen > 0)
			{
				*dst++ = *--src;
				integerLen --;
			}
		}
		else
		{
			memcpy(stream, &integer, integerLen);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//! implement 
//////////////////////////////////////////////////////////////////////////


template<class StreamHeadTrait>
inline std::pair<INTEGRITY_RET_TYPE, typename StreamHeadTrait::Integer> CheckBuffIntegrity(const char * buff, typename StreamHeadTrait::Integer curBuffLen, typename StreamHeadTrait::Integer maxBuffLen)
{
	//! 检查包头是否完整
	if (curBuffLen < StreamHeadTrait::HeadLen)
	{
		return std::make_pair(IRT_SHORTAGE, StreamHeadTrait::HeadLen - curBuffLen);
	}

	//! 获取包长度
	typename StreamHeadTrait::Integer packLen = streamToInteger<typename StreamHeadTrait::Integer, StreamHeadTrait>(buff);
	if (!StreamHeadTrait::PackLenIsContainHead)
	{
		typename StreamHeadTrait::Integer oldInteger = packLen;
		packLen += StreamHeadTrait::HeadLen;
		if (packLen < oldInteger) //over range
		{
			return std::make_pair(IRT_CORRUPTION, curBuffLen);
		}
	}

	//! check
	if (packLen > maxBuffLen)
	{
		return std::make_pair(IRT_CORRUPTION, curBuffLen);
	}
	if (packLen == curBuffLen)
	{
		return std::make_pair(IRT_SUCCESS, packLen);
	}
	if (packLen < curBuffLen)
	{
		return std::make_pair(IRT_SUCCESS, packLen);
	}
	return std::make_pair(IRT_SHORTAGE, packLen - curBuffLen);
}



//////////////////////////////////////////////////////////////////////////
//! implement 
//////////////////////////////////////////////////////////////////////////

template<class StreamHeadTrait, class AllocType>
WriteStream<StreamHeadTrait, AllocType>::WriteStream(UserBuffType ubt, Integer maxStreamLen)
{
	_usedBuffType = ubt;
	_attachData = NULL;
	_isNoWrite = false;
	_maxStreamLen = maxStreamLen;
	_cursor = StreamHeadTrait::HeadLen;
	Integer reserveSize = sizeof(Integer) == 1 ? 255 : 1200;
	if (reserveSize < StreamHeadTrait::HeadLen)
	{
		reserveSize = StreamHeadTrait::HeadLen;
	}
	if (_usedBuffType == UBT_AUTO)
	{
		_data.reserve(reserveSize);
		_data.resize((size_t)StreamHeadTrait::HeadLen, '\0');
	}
	else if (_usedBuffType == UBT_STATIC_AUTO)
	{
		if (_staticData.capacity() < (size_t)StreamHeadTrait::HeadLen)
		{
			_staticData.reserve((size_t)StreamHeadTrait::HeadLen);
		}
		_staticData.resize((size_t)StreamHeadTrait::HeadLen, '\0');
	}
}


template<class StreamHeadTrait, class AllocType>
WriteStream<StreamHeadTrait, AllocType>::WriteStream(char * attachData, Integer maxStreamLen, bool bNoWrite)
{
	_usedBuffType = UBT_ATTACH;
	_attachData = attachData;
	_maxStreamLen = maxStreamLen;
	_cursor = StreamHeadTrait::HeadLen;
	_isNoWrite = bNoWrite;
}

template<class StreamHeadTrait, class AllocType>
inline void WriteStream<StreamHeadTrait, AllocType>::checkMoveCursor(Integer unit)
{
	if (_maxStreamLen < StreamHeadTrait::HeadLen)
	{
		throw std::runtime_error("construction param error. attach memory size less than StreamHeadTrait::HeadLen.");
	}
	if (_cursor > _maxStreamLen)
	{
		throw std::runtime_error("bound over. cursor in end-of-data.");
	}
	if (unit > _maxStreamLen)
	{
		throw std::runtime_error("bound over. new unit be discarded.");
	}
	if (_maxStreamLen - _cursor < unit)
	{
		throw std::runtime_error("bound over. new unit be discarded.");
	}
}

template<class StreamHeadTrait, class AllocType>
inline void WriteStream<StreamHeadTrait, AllocType>::fixPackLen()
{
	if (_isNoWrite)
	{
		return;
	}
	Integer packLen = _cursor;
	if (!StreamHeadTrait::PackLenIsContainHead)
	{
		packLen -= StreamHeadTrait::HeadLen;
	}
	if (_usedBuffType == UBT_ATTACH)
	{
		integerToStream<Integer, StreamHeadTrait>(packLen, &_attachData[0]);
	}
	else if (_usedBuffType == UBT_AUTO)
	{
		integerToStream<Integer, StreamHeadTrait>(packLen, &_data[0]);
	}
	else if (_usedBuffType == UBT_STATIC_AUTO)
	{
		integerToStream<Integer, StreamHeadTrait>(packLen, &_staticData[0]);
	}
	
}



template<class StreamHeadTrait, class AllocType>
inline char* WriteStream<StreamHeadTrait, AllocType>::getStream()
{
	if (_isNoWrite)
	{
		return NULL;
	}
	if (_usedBuffType == UBT_ATTACH)
	{
		return _attachData;
	}
	else if (_usedBuffType == UBT_AUTO)
	{
		return &_data[0];
	}
	else if (_usedBuffType == UBT_STATIC_AUTO)
	{
		return &_staticData[0];
	}
	return NULL;
}

template<class StreamHeadTrait, class AllocType>
inline char* WriteStream<StreamHeadTrait, AllocType>::getStreamBody()
{
	if (_isNoWrite)
	{
		return NULL;
	}
	if (_usedBuffType == UBT_ATTACH)
	{
		return _attachData + StreamHeadTrait::HeadLen;
	}
	else if (_usedBuffType == UBT_AUTO)
	{
		return &_data[0] + StreamHeadTrait::HeadLen;
	}
	else if (_usedBuffType == UBT_STATIC_AUTO)
	{
		return &_staticData[0] + StreamHeadTrait::HeadLen;
	}
	return NULL;
}

template<class StreamHeadTrait, class AllocType>
inline WriteStream<StreamHeadTrait> & WriteStream<StreamHeadTrait, AllocType>::appendOriginalData(const void * data, Integer unit)
{
	checkMoveCursor(unit);
	if (!_isNoWrite)
	{
		if (_usedBuffType == UBT_ATTACH)
		{
			memcpy(&_attachData[_cursor], data, unit);
		}
		else if (_usedBuffType == UBT_AUTO)
		{
			_data.append((const char*)data, unit);
		}
		else if (_usedBuffType == UBT_STATIC_AUTO)
		{
			_staticData.append((const char*)data, unit);
		}
	}
	_cursor += unit;
	fixPackLen();
	return *this;
}

template<class StreamHeadTrait, class AllocType> template <class T> 
inline WriteStream<StreamHeadTrait> & WriteStream<StreamHeadTrait, AllocType>::writeIntegerData(T t)
{
	Integer unit = sizeof(T);
	checkMoveCursor(unit);
	if (!_isNoWrite)
	{
		if (_usedBuffType == UBT_ATTACH)
		{
			integerToStream<T, StreamHeadTrait>(t, &_attachData[_cursor]);
		}
		else if (_usedBuffType == UBT_AUTO)
		{
			_data.append((const char*)&t, unit);
			if (StreamHeadTrait::EndianType != __localEndianType())
			{
				integerToStream<T, StreamHeadTrait>(t, &_data[_cursor]);
			}
		}
		else if (_usedBuffType == UBT_STATIC_AUTO)
		{
			_staticData.append((const char*)&t, unit);
			if (StreamHeadTrait::EndianType != __localEndianType())
			{
				integerToStream<T, StreamHeadTrait>(t, &_staticData[_cursor]);
			}
		}
	}
	_cursor += unit;
	fixPackLen();
	return * this;
}


template<class StreamHeadTrait, class AllocType> template <class T>
inline WriteStream<StreamHeadTrait> & WriteStream<StreamHeadTrait, AllocType>::writeSimpleData(T t)
{
	Integer unit = sizeof(T);
	checkMoveCursor(unit);
	if (!_isNoWrite)
	{
		if (_usedBuffType == UBT_ATTACH)
		{
			memcpy(&_attachData[_cursor], &t, unit);
		}
		else if (_usedBuffType == UBT_AUTO)
		{
			_data.append((const char*)&t, unit);
		}
		else if (_usedBuffType == UBT_STATIC_AUTO)
		{
			_staticData.append((const char*)&t, unit);
		}
	}

	_cursor += unit;
	fixPackLen();
	return * this;
}






//////////////////////////////////////////////////////////////////////////
//! implement 
//////////////////////////////////////////////////////////////////////////
template<class StreamHeadTrait>
ReadStream<StreamHeadTrait>::ReadStream(const char *attachData, Integer attachDataLen)
{
	_attachData = attachData;
	_maxDataLen = attachDataLen;
	_cursor = StreamHeadTrait::HeadLen;
	if (_maxDataLen > StreamHeadTrait::MaxPackLen)
	{
		_maxDataLen = StreamHeadTrait::MaxPackLen;
	}
	if (_maxDataLen < StreamHeadTrait::HeadLen)
	{
		_attachData = NULL;
	}
}


template<class StreamHeadTrait>
inline void ReadStream<StreamHeadTrait>::checkMoveCursor(Integer unit)
{
	if (_cursor > _maxDataLen)
	{
		throw std::runtime_error("bound over. cursor in end-of-data.");
	}
	if (unit > _maxDataLen)
	{
		throw std::runtime_error("bound over. new unit be discarded.");
	}
	if (_maxDataLen - _cursor < unit)
	{
		throw std::runtime_error("bound over. new unit be discarded.");
	}
}


template<class StreamHeadTrait>
inline const char* ReadStream<StreamHeadTrait>::getStream()
{
	return _attachData;
}

template<class StreamHeadTrait>
inline typename ReadStream<StreamHeadTrait>::Integer ReadStream<StreamHeadTrait>::getStreamLen()
{
	Integer packLen = streamToInteger<Integer, StreamHeadTrait>(&_attachData[0]);
	if (!StreamHeadTrait::PackLenIsContainHead)
	{
		return packLen + StreamHeadTrait::HeadLen;
	}
	if (packLen > _maxDataLen)
	{
		return 0;
	}
	return packLen;
}

template<class StreamHeadTrait>
inline const char* ReadStream<StreamHeadTrait>::getStreamBody()
{
	return &_attachData[StreamHeadTrait::HeadLen];
}

template<class StreamHeadTrait>
inline typename ReadStream<StreamHeadTrait>::Integer ReadStream<StreamHeadTrait>::getStreamBodyLen()
{
	return getStreamLen() - StreamHeadTrait::HeadLen;
}

template<class StreamHeadTrait>
inline const char* ReadStream<StreamHeadTrait>::getStreamUnread()
{
	return &_attachData[_cursor];
}

template<class StreamHeadTrait>
typename ReadStream<StreamHeadTrait>::Integer ReadStream<StreamHeadTrait>::getStreamUnreadLen()
{
	return getStreamBodyLen() + StreamHeadTrait::HeadLen - _cursor;
}

template<class StreamHeadTrait> template <class T>
inline ReadStream<StreamHeadTrait> & ReadStream<StreamHeadTrait>::readIntegerData(T & t)
{
	Integer unit = sizeof(T);
	checkMoveCursor(unit);
	t = streamToInteger<T, StreamHeadTrait>(&_attachData[_cursor]);
	_cursor += unit;
	return * this;
}
template<class StreamHeadTrait> template <class T>
inline ReadStream<StreamHeadTrait> & ReadStream<StreamHeadTrait>::readSimpleData(T & t)
{
	Integer unit = sizeof(T);
	checkMoveCursor(unit);
	memcpy(&t, &_attachData[_cursor], unit);
	_cursor += unit;
	return * this;
}
template<class StreamHeadTrait>
inline const char * ReadStream<StreamHeadTrait>::peekOriginalData(Integer unit)
{
	checkMoveCursor(unit);
	return &_attachData[_cursor];
}
template<class StreamHeadTrait>
inline void ReadStream<StreamHeadTrait>::skipOriginalData(Integer unit)
{
	checkMoveCursor(unit);
	_cursor += unit;
}








/////////////////////////
//http proto 
/////////////////////////
const char *const CRLF = "\r\n";
const char CR = '\r'; //CRLF
const char LF = '\n';
const char SEGM = ':';
const char BLANK = ' ';
typedef std::pair<std::string, std::string> PairString;
typedef std::map<std::string, std::string> HTTPHeadMap;

inline INTEGRITY_RET_TYPE checkHTTPBuffIntegrity(const char * buff, unsigned int curBuffLen, unsigned int maxBuffLen, 
							bool hadHeader, bool & isChunked, PairString& commonLine, HTTPHeadMap & head, std::string & body, unsigned int &usedCount);

std::string urlEncode(const std::string& orgString);
std::string urlDecode(const std::string& orgString);
class WriteHTTP
{
public:
	const char * getStream(){ return m_buff.c_str();}
	unsigned int getStreamLen() { return (unsigned int)m_buff.length();}
	void addHead(std::string key, std::string val)
	{
		m_head.insert(std::make_pair(key, val));
	}
	void post(std::string uri, std::string content)
	{
		char buf[100];
		sprintf(buf, "%u", (unsigned int)content.length());
		m_head.insert(std::make_pair("Content-Length", buf));
		m_buff.append("POST " + uri + " HTTP/1.1" + CRLF);
		writeGeneralHead();
		m_buff.append(CRLF);
		m_buff.append(content);
	}
	void get(std::string uri)
	{
		m_head.insert(std::make_pair("Content-Length", "0"));
		m_buff.append("GET " + uri + " HTTP/1.1" + CRLF);
		writeGeneralHead();
		m_buff.append(CRLF);
	}
	void response(std::string statusCode, std::string content)
	{
		char buf[100];
		sprintf(buf, "%u", (unsigned int)content.length());
		m_head.insert(std::make_pair("Content-Length", buf));
		m_buff.append("HTTP/1.1 " + statusCode + " ^o^" + CRLF);
		writeGeneralHead();
		m_buff.append(CRLF);
		m_buff.append(content);
	}
protected:
	void writeGeneralHead()
	{
		for (HTTPHeadMap::iterator iter = m_head.begin(); iter != m_head.end(); ++iter)
		{
			m_buff.append(iter->first + ":" + iter->second + CRLF);
		}
	}
private:
	HTTPHeadMap m_head;
	std::string m_buff;
};


inline std::string urlEncode(const std::string & orgString)
{
	std::string ret;
	for (int i = 0; i < (int)orgString.length(); ++i)
	{
		char ch = orgString[i];
		if (ch == '\0')
		{
			break;
		}
		if ((ch >= 'A' && ch <= 'Z')
			|| (ch >= 'a' && ch <= 'z')
			|| (ch >= '0' && ch <= '9')
			|| ch == '-' || ch == '_' || ch == '.' || ch == '~')
		{
			ret += ch;
		}
		else if (ch == ' ')
		{
			ret += '+';
		}
		else
		{
			ret += '%';
			unsigned char tmp = ch / 16;
			ret += tmp > 9 ? tmp + 55 : tmp + 48;
			tmp = ch % 16;
			ret += tmp > 9 ? tmp + 55 : tmp + 48;
		}
	}
	return ret;
}
inline std::string urlDecode(const std::string & orgString)
{
	std::string ret;
	unsigned int count = (unsigned int)orgString.length();
	unsigned int cursor = 0;
	while (cursor < count)
	{
		char ch = orgString[cursor];
		if (ch == '\0')
		{
			break;
		}
		if (ch == '+')
		{
			ret += ' ';
			cursor++;
			continue;
		}
		if (ch == '%')
		{
			if (count - cursor < 2)
			{
				break; //error
			}
			unsigned char och;
			unsigned char x = orgString[cursor + 1];
			if (x >= 'A' && x <= 'Z') x = x - 'A' + 10;
			else if (x >= 'a' && x <= 'z') x = x - 'a' + 10;
			else if (x >= '0' && x <= '9') x = x - '0';
			och = x * 16;
			x = orgString[cursor + 2];
			if (x >= 'A' && x <= 'Z') x = x - 'A' + 10;
			else if (x >= 'a' && x <= 'z') x = x - 'a' + 10;
			else if (x >= '0' && x <= '9') x = x - '0';
			och += x;
			ret += och;
			cursor += 3;
			continue;
		}
		ret += ch;
		cursor++;
	}
	return ret;
}


inline unsigned int InnerReadLine(const char * buff, unsigned int curBuffLen, unsigned int maxBuffLen,  bool isKV, bool isCommondLine,
	std::string & outStringKey, std::string &outStringValue)
{
	if (curBuffLen == 0)
	{
		return IRT_CORRUPTION;
	}
	
	unsigned int cursor = 0;
	short readStatus = 0;
	INTEGRITY_RET_TYPE isIntegrityData = IRT_SHORTAGE;

	outStringKey.clear();
	outStringValue.clear();

	while (cursor < curBuffLen)
	{
		if (cursor >= maxBuffLen)
		{
			isIntegrityData = IRT_CORRUPTION;
			break;
		}
		if (buff[cursor] == CR)
		{
			cursor++;
			continue;
		}
		if (buff[cursor] == LF)
		{
			cursor++;
			isIntegrityData = IRT_SUCCESS;
			break;
		}
		if (!isKV)
		{
			outStringKey.push_back(buff[cursor]);
			cursor++;
			continue;
		}
		else
		{
			if (isCommondLine)
			{
				if (buff[cursor] == BLANK)
				{
					readStatus++;
					cursor++;
					continue;
				}
			}
			else // ! isCommondLine
			{
				if (buff[cursor] == SEGM)
				{
					readStatus++;
					cursor++;
					continue;
				}
				if (buff[cursor] == BLANK && readStatus == 0)
				{
					cursor++;
					continue;
				}
				if (buff[cursor] == BLANK && readStatus == 1 && outStringValue.empty())
				{
					cursor++;
					continue;
				}
			}//end. !isCommondLine
			if (readStatus == 0)
			{
				outStringKey.push_back(buff[cursor]);
			}
			else if (readStatus == 1)
			{
				outStringValue.push_back(buff[cursor]);
			}
			cursor++;
			continue;
		}//end. isKV
	}//extract character loop

	if (isIntegrityData != IRT_SUCCESS)
	{
		throw isIntegrityData;
	}
	return cursor;
}

inline INTEGRITY_RET_TYPE checkHTTPBuffIntegrity(const char * buff, unsigned int curBuffLen, unsigned int maxBuffLen,
	bool hadHeader, bool &isChunked, PairString& commonLine, HTTPHeadMap & head, std::string & body, unsigned int &usedCount)
{
	if (!hadHeader)
	{
		isChunked = false;
	}
	int bodyLenght = -1;
	usedCount = 0;
	PairString keyValue;

	//extract head
	if (!hadHeader)
	{

		//extract common line
		try
		{
			usedCount += InnerReadLine(buff + usedCount, curBuffLen - usedCount, maxBuffLen - usedCount, true, true, commonLine.first, commonLine.second);
		}
		catch (INTEGRITY_RET_TYPE t)
		{
			return t;
		}

		//extract head line
		head.clear();
		try
		{
			do
			{
				keyValue.first.clear();
				keyValue.second.clear();
				usedCount += InnerReadLine(buff + usedCount, curBuffLen - usedCount, maxBuffLen - usedCount, true, false, keyValue.first, keyValue.second);
				if (keyValue.first.empty() && keyValue.second.empty())
				{
					break;
				}
				if (keyValue.first == "Content-Length")
				{
					bodyLenght = atoi(keyValue.second.c_str());
				}
				else if (keyValue.first == "Transfer-Encoding")
				{
					isChunked = true;
				}
				head.insert(keyValue);
			} while (true);
		}
		catch (INTEGRITY_RET_TYPE t)
		{
			return t;
		}
	}
	


	//read chunked header
	if (isChunked)
	{
		try
		{
			keyValue.first.clear();
			keyValue.second.clear();
			usedCount += InnerReadLine(buff + usedCount, curBuffLen - usedCount, maxBuffLen - usedCount, false, false, keyValue.first, keyValue.second);
			//chunked end. need closed.
			if (keyValue.first.empty())
			{
				return IRT_CORRUPTION;
			}
			sscanf(keyValue.first.c_str(), "%x", &bodyLenght);
			if (bodyLenght == 0)
			{
				//http socket end.
				return IRT_CORRUPTION;
			}
			
		}
		catch (INTEGRITY_RET_TYPE t)
		{
			return t;
		}
	}
	else if (hadHeader)
	{
		return IRT_SHORTAGE;
	}
	else if (commonLine.first == "GET")
	{
		return IRT_SUCCESS;
	}
	
	if (bodyLenght == -1 || usedCount + bodyLenght > maxBuffLen)
	{
		return IRT_CORRUPTION;
	}
	if (bodyLenght + usedCount > curBuffLen)
	{
		return IRT_SHORTAGE;
	}
	body.assign(buff + usedCount, bodyLenght);
	usedCount += bodyLenght;

	if (isChunked)//clean chunked end CRLF
	{
		try
		{
			keyValue.first.clear();
			keyValue.second.clear();
			usedCount += InnerReadLine(buff + usedCount, curBuffLen - usedCount, maxBuffLen - usedCount, false, false, keyValue.first, keyValue.second);
			//chunked end. need closed.
			if (!keyValue.first.empty())
			{
				return IRT_CORRUPTION;
			}
		}
		catch (INTEGRITY_RET_TYPE t)
		{
			return t;
		}
	}
	

	return IRT_SUCCESS;
}









#ifndef _ZSUMMER_END
#define _ZSUMMER_END }
#endif  
#ifndef _ZSUMMER_PROTO4Z_END
#define _ZSUMMER_PROTO4Z_END }
#endif

_ZSUMMER_PROTO4Z_END
_ZSUMMER_END

#endif