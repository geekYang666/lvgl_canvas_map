#include <cstring>
#include "SubFile.h"


#define mod2n(x, m) ((x) & ((m) - 1))

bool SubFile::seek(uint32_t pos)
{

    uint32_t blockNum = pos >> BLOCK_BITS;

	if (_blockNum != blockNum) {
		uint32_t seek = ((uint32_t)blockNum << BLOCK_BITS) + _offset;

		if (seek >= _offset + _size || lv_fs_seek(_file,seek,LV_FS_SEEK_SET)!=LV_FS_RES_OK)
			return false;

        uint32_t readNum;
        lv_fs_res_t readRes = lv_fs_read(_file,_data,sizeof _data,&readNum);
		if (readRes!=LV_FS_RES_OK)
			return false;
		_blockNum = blockNum;
	}

	_blockPos = mod2n(pos, 1U<<BLOCK_BITS);
	_pos = pos;

	return true;
}

bool SubFile::read(char *buff, uint32_t size)
{
	while (size) {
		uint32_t remaining = sizeof(_data) - _blockPos;
		if (size < remaining) {
			memcpy(buff, _data + _blockPos, size);
			_blockPos += size;
			_pos += size;
			return true;
		} else {
			memcpy(buff, _data + _blockPos, remaining);
			buff += remaining;
			size -= remaining;
			_blockPos = 0;
			_pos += remaining;
			if (!seek(_pos))
				return false;
		}
	}

	return true;
}
bool SubFile::readTileOffset(uint64_t &val)
{
    uint8_t buffer[5];
    uint32_t readNum;
    lv_fs_res_t readRes = lv_fs_read(_file,buffer,sizeof buffer,&readNum);
    if (readRes!=LV_FS_RES_OK)
        return false;
    val = buffer[4] | ((uint64_t)buffer[3]) << 8 | ((uint64_t)buffer[2]) << 16
             | ((uint64_t)buffer[1]) << 24 | ((uint64_t)buffer[0]) << 32;

    return true;
}