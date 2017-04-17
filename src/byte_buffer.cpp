#include "byte_buffer.h"

/**
* byte_buffer constructor
* Reserves specified size in internal vector
*
* @param size Size (in bytes) of space to preallocate internally. Default is set in DEFAULT_SIZE
*/
byte_buffer::byte_buffer(uint32_t size) {
	buf.reserve(size);
	clear();
}


/**
* byte_buffer constructor
* Consume an entire uint8_t array of length len in the byte_buffer
*
* @param arr uint8_t array of data (should be of length len)
* @param size Size of space to allocate
*/
byte_buffer::byte_buffer(uint8_t* arr, uint32_t size) {
	// If the provided array is NULL, allocate a blank buffer of the provided size
	if(arr == NULL) {
		buf.reserve(size);
		clear();
	} else { // Consume the provided array
		buf.reserve(size);
		clear();
		putBytes(arr, size);
	}
}

/**
* byte_buffer Deconstructor
*
*/
byte_buffer::~byte_buffer() {
}


std::vector<uint8_t>& byte_buffer::getRawBuf()
{
	uint32_t len = (wpos - rpos);
	uint32_t i = 0;

	if (len != 0 && rpos != 0) {
		//move data
		for (; i<len; i++) {
			buf[i] = buf[rpos + i];
		}
		//clear end
		while (buf.size()>len) {
			buf.pop_back();
		}
		//reset R/W position
		rpos = 0;
		wpos = len;
	}
	return buf;
}

uint8_t* byte_buffer::data()
{
	return buf.data();
}

/**
* Bytes Remaining
* Returns the number of bytes from the current read position till the end of the buffer
*
* @return Number of bytes from rpos to the end (size())
*/
uint32_t byte_buffer::bytesRemaining() {
	return size() - rpos;
}

/**
* Clear
* Clears out all data from the internal vector (original preallocated size remains), resets the positions to 0
*/
void byte_buffer::clear() {
	rpos = 0;
	wpos = 0;
	buf.clear();
}


/**
* Copy remaining data to new Buffer
* Allocate an simple copy of the byte_buffer on the heap and return a pointer
*
* @return A pointer to the newly cloned byte_buffer. NULL if no more memory available
*/
byte_buffer* byte_buffer::copy() {
	int remain_size = this->bytesRemaining();
	byte_buffer* ret = new byte_buffer(remain_size);
	
	// Copy data
	for(uint32_t i = 0; i < remain_size; i++) {
		ret->put((uint8_t)get(i+rpos));
	}

	// Reset positions
	ret->setReadPos(0);
	ret->setWritePos(remain_size);

	return ret;
}

/**
* Clone
* Allocate an exact copy of the byte_buffer on the heap and return a pointer
*
* @return A pointer to the newly cloned byte_buffer. NULL if no more memory available
*/
byte_buffer* byte_buffer::clone() {
	byte_buffer* ret = new byte_buffer(buf.size());

	// Copy data
	for(uint32_t i = 0; i < buf.size(); i++) {
		ret->put((uint8_t)get(i));
	}

	// Reset positions
	ret->setReadPos(0);
	ret->setWritePos(0);

	return ret;
}

/**
* Equals, test for data equivilancy
* Compare this byte_buffer to another by looking at each byte in the internal buffers and making sure they are the same
*
* @param other A pointer to a byte_buffer to compare to this one
* @return True if the internal buffers match. False if otherwise
*/
bool byte_buffer::equals(byte_buffer* other) {
	// If sizes aren't equal, they can't be equal
	if (size() != other->size())
		return false;

	// Compare byte by byte
	uint32_t len = size();
	for (uint32_t i = 0; i < len; i++) {
		if ((uint8_t)get(i) != (uint8_t)other->get(i))
			return false;
	}

	return true;
}

/**
* Resize
* Reallocates memory for the internal buffer of size newSize. Read and write positions will also be reset
*
* @param newSize The amount of memory to allocate
*/
void byte_buffer::resize(uint32_t newSize) {
	buf.resize(newSize);
	rpos = 0;
	wpos = 0;
}

/**
* Size
* Returns the size of the internal buffer...not necessarily the length of bytes used as data!
*
* @return size of the internal buffer
*/
uint32_t byte_buffer::size() {
	return buf.size();
}

// Replacement

/**
* Replace
* Replace occurance of a particular uint8_t, key, with the uint8_t rep
*
* @param key uint8_t to find for replacement
* @param rep uint8_t to replace the found key with
* @param start Index to start from. By default, start is 0
* @param firstOccuranceOnly If true, only replace the first occurance of the key. If false, replace all occurances. False by default
*/
void byte_buffer::replace(uint8_t key, uint8_t rep, uint32_t start, bool firstOccuranceOnly) {
	uint32_t len = buf.size();
	for (uint32_t i = start; i < len; i++) {
		uint8_t data = read<uint8_t>(i);
		// Wasn't actually found, bounds of buffer were exceeded
		if ((key != 0) && (data == 0))
			break;

		// Key was found in array, perform replacement
		if (data == key) {
			buf[i] = rep;
			if (firstOccuranceOnly)
				return;
		}
	}
}

// Read Functions

uint8_t byte_buffer::peek() {
	return read<uint8_t>(rpos);
}

uint8_t byte_buffer::get() {
	return read<uint8_t>();
}

uint8_t byte_buffer::get(uint32_t index) {
	return read<uint8_t>(index);
}

void byte_buffer::getBytes(uint8_t* buf, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		buf[i] = read<uint8_t>();
	}
}

char byte_buffer::getChar() {
	return read<char>();
}

char byte_buffer::getChar(uint32_t index) {
	return read<char>(index);
}

double byte_buffer::getDouble() {
	return read<double>();
}

double byte_buffer::getDouble(uint32_t index) {
	return read<double>(index);
}

float byte_buffer::getFloat() {
	return read<float>();
}

float byte_buffer::getFloat(uint32_t index) {
	return read<float>(index);
}

uint32_t byte_buffer::getInt() {
	uint32_t ret = read<uint32_t>();
	if (!am_big_endian()) {
		to_network_order<uint32_t>(&ret);
	}
	return ret;
}

uint32_t byte_buffer::getInt(uint32_t index) {
	uint32_t ret = read<uint32_t>(index);
	if (!am_big_endian()) {
		to_network_order<uint32_t>(&ret);
	}
	return ret;
}

uint64_t byte_buffer::getLong() {
	uint64_t ret = read<uint64_t>();
	//std::cout << ret << std::endl;
	if (!am_big_endian()) {
		to_network_order<uint64_t>(&ret);
	}
	return ret;
}

uint64_t byte_buffer::getLong(uint32_t index) {
	uint64_t ret = read<uint64_t>(index);
	if (!am_big_endian()) {
		to_network_order<uint64_t>(&ret);
	}
	return ret;
}

uint16_t byte_buffer::getShort() {
	uint16_t ret = read<uint16_t>();
	if (!am_big_endian()) {
		to_network_order<uint16_t>(&ret);
	}
	return ret;
}

uint16_t byte_buffer::getShort(uint32_t index) {
	uint16_t ret = read<uint16_t>(index);
	if (!am_big_endian()) {
		to_network_order<uint16_t>(&ret);
	}
	return ret;
}


// Write Functions

void byte_buffer::put(byte_buffer* src) {
	uint32_t len = src->size();
	for (uint32_t i = 0; i < len; i++)
		append<uint8_t>(src->get(i));
}

void byte_buffer::put(uint8_t b) {
	append<uint8_t>(b);
}

void byte_buffer::put(uint8_t b, uint32_t index) {
	insert<uint8_t>(b, index);
}

void byte_buffer::putBytes(const uint8_t* b, uint32_t len) {
	// Insert the data one byte at a time into the internal buffer at position i+starting index
	for (uint32_t i = 0; i < len; i++)
		append<uint8_t>(b[i]);
}

void byte_buffer::putBytes(const uint8_t* b, uint32_t len, uint32_t index) {
	wpos = index;

	// Insert the data one byte at a time into the internal buffer at position i+starting index
	for (uint32_t i = 0; i < len; i++)
		append<uint8_t>(b[i]);
}

void byte_buffer::putChar(char value) {
	append<char>(value);
}

void byte_buffer::putChar(char value, uint32_t index) {
	insert<char>(value, index);
}

void byte_buffer::putDouble(double value) {
	append<double>(value);
}

void byte_buffer::putDouble(double value, uint32_t index) {
	insert<double>(value, index);
}
void byte_buffer::putFloat(float value) {
	append<float>(value);
}

void byte_buffer::putFloat(float value, uint32_t index) {
	insert<float>(value, index);
}

void byte_buffer::putInt(uint32_t value) {
	if (!am_big_endian()) {
		to_network_order<uint32_t>(&value);
	}
	append<uint32_t>(value);
}

void byte_buffer::putInt(uint32_t value, uint32_t index) {
	if (!am_big_endian()) {
		to_network_order<uint32_t>(&value);
	}
	insert<uint32_t>(value, index);
}

void byte_buffer::putLong(uint64_t value) {
	if (!am_big_endian()) {
		to_network_order<uint64_t>(&value);
	}
	append<uint64_t>(value);
}

void byte_buffer::putLong(uint64_t value, uint32_t index) {
	if (!am_big_endian()) {
		to_network_order<uint64_t>(&value);
	}
	insert<uint64_t>(value, index);
}

void byte_buffer::putShort(uint16_t value) {
	if (!am_big_endian()) {
		to_network_order<uint16_t>(&value);
	}
	append<uint16_t>(value);
}

void byte_buffer::putShort(uint16_t value, uint32_t index) {
	if (!am_big_endian()) {
		to_network_order<uint16_t>(&value);
	}
	insert<uint16_t>(value, index);
}

bool byte_buffer::am_big_endian()
{
	union { long l; char c[sizeof(long)]; } u;
	u.l = 1;
	if (u.c[sizeof(long) - 1] == 1)
	{
		// Big Endian
		return 1;
	}
	else
	{
		// Little Endian
		return 0;
	}
}


static const char* hex_flag = "0123456789ABCDEF";

std::string byte_buffer::toHex()
{
	std::string hex;
	uint32_t length = buf.size();

	for (uint32_t i = 0; i < length; i++) {
		uint8_t c = buf[i];
		hex.push_back(hex_flag[c & 0xF0]);
		hex.push_back(hex_flag[c & 0x0F]);
		hex.push_back(' ');
	}
	return hex;
}

std::string byte_buffer::toString()
{
	return std::string(buf.begin(), buf.end());
}