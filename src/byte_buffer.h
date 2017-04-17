#ifndef __BYTE_BUFFER_H__
#define __BYTE_BUFFER_H__

// Default number of uint8_ts to allocate in the backing buffer if no size is provided
#define DEFAULT_SIZE 8192
#define DEFAULT_BIGENDIAN TRUE


#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

class byte_buffer
	: public boost::enable_shared_from_this<byte_buffer>
{
private:
	uint32_t rpos, wpos;
	std::vector<uint8_t> buf;

	template <typename T> T read() {
		T data = read<T>(rpos);
		int size = sizeof(T);
		rpos += size;
		return data;
	}

	template <typename T> T read(uint32_t index) const {
		if (index + sizeof(T) <= buf.size())
			return *((T*)&buf[index]);
		return 0;
	}

	void swap(uint8_t *data, int size) {
		uint8_t c;
		int i = 0;

		for (; i<(size / 2); i++) {
			c = data[i];
			data[i] = data[size - 1 - i];
			data[size - 1 - i] = c;
		}
	}

	template <typename T> void append(T data) {
		uint32_t s = sizeof(data);

		if (size() < (wpos + s))
			buf.resize(wpos + s);

		memcpy(&buf[wpos], (uint8_t*)&data, s);
		//printf("writing %c to %i\n", (uint8_t)data, wpos);
		wpos += s;
	}

	template <typename T> void insert(T data, uint32_t index) {
		if ((index + sizeof(data)) > size())
			return;

		memcpy(&buf[index], (uint8_t*)&data, sizeof(data));
		wpos = index + sizeof(data);
	}

public:
	byte_buffer(uint32_t size = DEFAULT_SIZE);
	byte_buffer(uint8_t* arr, uint32_t size);
	~byte_buffer();

	std::vector<uint8_t>& getRawBuf();
	uint8_t* data();

	uint32_t bytesRemaining(); // Number of uint8_ts from the current read position till the end of the buffer
	void clear(); // Clear our the vector and reset read and write positions
	byte_buffer* clone(); // Return a new instance of a byte_buffer with the exact same contents and the same state (rpos, wpos)
						 //byte_buffer compact(); // TODO?
	bool equals(byte_buffer* other); // Compare if the contents are equivalent
	void resize(uint32_t newSize);
	uint32_t size(); // Size of internal vector
	byte_buffer* copy(); //copy the remaining data to new Buffer

						// Basic Searching (Linear)
	template <typename T> int32_t find(T key, uint32_t start = 0) {
		int32_t ret = -1;
		uint32_t len = buf.size();
		for (uint32_t i = start; i < len; i++) {
			T data = read<T>(i);
			// Wasn't actually found, bounds of buffer were exceeded
			if ((key != 0) && (data == 0))
				break;

			// Key was found in array
			if (data == key) {
				ret = (int32_t)i;
				break;
			}
		}
		return ret;
	}

	template <typename T> void to_network_order(T* data) {
		if (!am_big_endian()) {
			swap((uint8_t*)data, sizeof(T));
		}
	}

	// Replacement
	void replace(uint8_t key, uint8_t rep, uint32_t start = 0, bool firstOccuranceOnly = false);


	static bool am_big_endian();

	// Read

	uint8_t peek(); // Relative peek. Reads and returns the next uint8_t in the buffer from the current position but does not increment the read position
	uint8_t get(); // Relative get method. Reads the uint8_t at the buffers current position then increments the position
	uint8_t get(uint32_t index); // Absolute get method. Read uint8_t at index
	void getBytes(uint8_t* buf, uint32_t len); // Absolute read into array buf of length len
	char getChar(); // Relative
	char getChar(uint32_t index); // Absolute
	double getDouble();
	double getDouble(uint32_t index);
	float getFloat();
	float getFloat(uint32_t index);
	uint32_t getInt();
	uint32_t getInt(uint32_t index);
	uint64_t getLong();
	uint64_t getLong(uint32_t index);
	uint16_t getShort();
	uint16_t getShort(uint32_t index);

	// Write

	void put(byte_buffer* src); // Relative write of the entire contents of another byte_buffer (src)
	void put(uint8_t b); // Relative write
	void put(uint8_t b, uint32_t index); // Absolute write at index
	void putBytes(const uint8_t* b, uint32_t len); // Relative write
	void putBytes(const uint8_t* b, uint32_t len, uint32_t index); // Absolute write starting at index
	void putChar(char value); // Relative
	void putChar(char value, uint32_t index); // Absolute
	void putDouble(double value);
	void putDouble(double value, uint32_t index);
	void putFloat(float value);
	void putFloat(float value, uint32_t index);
	void putInt(uint32_t value);
	void putInt(uint32_t value, uint32_t index);
	void putLong(uint64_t value);
	void putLong(uint64_t value, uint32_t index);
	void putShort(uint16_t value);
	void putShort(uint16_t value, uint32_t index);

	// Buffer Position Accessors & Mutators

	void setReadPos(uint32_t r) {
		rpos = r;
	}

	uint32_t getReadPos() {
		return rpos;
	}

	void setWritePos(uint32_t w) {
		wpos = w;
	}

	uint32_t getWritePos() {
		return wpos;
	}

public:
	std::string toHex();
	std::string toString();
};

#endif //__BYTE_BUFFER_H__

