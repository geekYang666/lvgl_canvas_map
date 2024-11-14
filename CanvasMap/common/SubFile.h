#ifndef MAPSFORGE_SUBFILE_H
#define MAPSFORGE_SUBFILE_H
#include "lvgl.h"

#define BLOCK_BITS 12 /* 4096 bytes */


class SubFile
{
public:

    SubFile() = default;

	SubFile(lv_fs_file_t *file, uint64_t offset, uint64_t size)
	  : _file(file), _offset(offset), _size(size), _pos(-1),
	  _blockNum(-1), _blockPos(-1) {}

    uint32_t pos() const {return _pos;}
	bool seek(uint32_t pos);

	bool read(char *buff, uint32_t size);
    bool readTileOffset(uint64_t &val);

	bool readByte(uint8_t &val)
	{
		val = _data[_blockPos++];
		_pos++;
		return (_blockPos >= (int)sizeof(_data)) ? seek(_pos) : true;
	}

	template<typename T>
	bool readUInt16(T &val)
	{
		uint8_t b0, b1;
		if (!(readByte(b0) && readByte(b1)))
			return false;
		val = b1 | ((uint16_t)b0) << 8;
		return true;
	}

	bool readUInt32(uint32_t &val)
	{
		uint8_t b0, b1, b2, b3;
		if (!(readByte(b0) && readByte(b1) && readByte(b2) && readByte(b3)))
			return false;
		val = b3 | ((uint32_t)b2) << 8 | ((uint32_t)b1) << 16 | ((uint32_t)b0) << 24;
		return true;
	}

	bool readUInt64(uint64_t &val)
	{
		uint8_t b0, b1, b2, b3, b4, b5, b6, b7;
		if (!(readByte(b0) && readByte(b1) && readByte(b2) && readByte(b3)
		 && readByte(b4) && readByte(b5) && readByte(b6) && readByte(b7)))
			return false;
		val = b7 | ((uint64_t)b6) << 8 | ((uint64_t)b5) << 16
		  | ((uint64_t)b4) << 24 | ((uint64_t)b3) << 32 | ((uint64_t)b2) << 40
		  | ((uint64_t)b1) << 48 | ((uint64_t)b0) << 56;
		return true;
	}


	bool readInt32(int32_t &val)
	{
		return readUInt32(reinterpret_cast<uint32_t&>(val));
	}

	bool readVUInt32(uint32_t &val)
	{
		int shift = 0;
		uint8_t b;

		val = 0;
		do {
			if (!readByte(b))
				return false;
			val |= (uint32_t)(b & 0x7F) << shift;
			shift += 7;
		} while (b & 0x80);

		return true;
	}

	bool readVInt32(int32_t &val)
	{
		int shift = 0;
		uint8_t b;

		val = 0;
		while (true) {
			if (!readByte(b))
				return false;
			if (b & 0x80) {
				val |= (int32_t)(b & 0x7F) << shift;
				shift += 7;
			} else {
				val |= (int32_t)(b & 0x3F) << shift;
				if (b & 0x40)
					val = -val;
				break;
			}
		}

		return true;
	}

	bool readString(char * str)
	{
        uint32_t len;

		if (!readVUInt32(len))
			return false;
        
        if(len>200){
            return false;
        }
        
		if (!read(str, len))
			return false;
        *(str+len) = 0;
   
		return true;
	}

//private:
	lv_fs_file_t * _file;
	uint8_t _data[1U<<BLOCK_BITS];
	uint32_t _offset;
	uint32_t _size;
    uint32_t _pos;
    uint32_t _blockNum;
    uint32_t _blockPos;

};


#endif // MAPSFORGE_SUBFILE_H
