#ifndef __STREAM_PROPERTY_H__
#define __STREAM_PROPERTY_H__

#ifndef STREAM_PROPERTY
#define STREAM_PROPERTY(__TYPE__, __NAME__) \
	private: __TYPE__  m_##__NAME__; \
	public: data_type & __NAME__(__TYPE__ value) { m_##__NAME__ = value; return *this;} \
	public: __TYPE__ & __NAME__() {return  m_##__NAME__;}
#endif


#ifndef STREAM_CONST_PROPERTY
#define STREAM_CONST_PROPERTY(__TYPE__, __NAME__) \
	private: const __TYPE__  m_##__NAME__; \
	public: const __TYPE__ & __NAME__() {return  m_##__NAME__;}
#endif





#endif //__STREAM_PROPERTY_H__