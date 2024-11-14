#include "mapData.h"


static char name[200], houseNumber[200], reference[200];


bool readTags(SubFile &subFile, int count, const vector<TagSource> &tags, vector<Tag> &list) {
    vector<uint32_t> ids(count);

    list.resize(count);

    for (int i = 0; i < count; i++) {
        if (!subFile.readVUInt32(ids[i]))
            return false;
        if (ids[i] >= (uint32_t) tags.size())
            return false;
    }
    for (int i = 0; i < count; i++) {
        const TagSource &tag = tags.at(ids.at(i));

        if (tag.value.length() == 2 && tag.value.at(0) == '%') {
//            printf("tagFormat is %s\n", tag.value.c_str());
            string value;

            if (tag.value.at(1) == 'b') {
                uint8_t b;
                if (!subFile.readByte(b))
                    return false;
                value = to_string(b);
            } else if (tag.value.at(1) == 'i') {
                int32_t u;
                if (!subFile.readInt32(u))
                    return false;
                if (tag.key.find(":colour"))
//                    value = QColor((uint32_t)u).name().toLatin1();
                    value = to_string(u);
                else
                    value = to_string(u);
            } else if (tag.value.at(1) == 'f') {
                uint32_t u;
                if (!subFile.readUInt32(u))
                    return false;
                auto *f = (float *) &u;
                value = to_string(*f);
            } else if (tag.value.at(1) == 'h') {
                uint16_t s;
                if (!subFile.readUInt16(s))
                    return false;
                value = to_string(s);
            } else if (tag.value.at(1) == 's') {
                char str[50];
                if (!subFile.readString(str))
                    return false;
                value = string(str);
            } else
                value = tag.value;

            list[i] = Tag(tag.id, value);
        } else
            list[i] = Tag(tag.id, tag.value);
    }

    return true;
}


bool readPaths(SubFileInfo info, SubFile subFile, const VectorTile *tile, int zoom, vector<Path> *list, const vector<TagSource> &pathTags) {
    int rows = info.max - info.min + 1;
    vector<unsigned> paths(rows);
    uint32_t blocks, unused, val, cnt = 0;
    uint16_t bitmap;
    uint8_t sb, flags;

//    printf("readPaths() zoom is %d offset is %zu\n ",zoom,tile->offset);
//    printf("(%f,%f) readPaths is \n",tile->pos.lon(),tile->pos.lat());
    if (!subFile.seek(tile->offset & OFFSET_MASK))
        return false;

    for (int i = 0; i < rows; i++) {
        if (!(subFile.readVUInt32(unused) && subFile.readVUInt32(val)))
            return false;
        cnt += val;
        paths[i] = cnt;
//        printf("level[%d] has %d paths\n", info.min + i, cnt);
    }

    if (!subFile.readVUInt32(val))
        return false;
    if (!subFile.seek(subFile.pos() + val))
        return false;

    paths.reserve(paths[zoom - info.min]);

//    Tag waterTag(_keys["natural"], string("water"));
//    Tag residentialTag(_keys["landuse"], string("residential"));
//    Tag parkTag(_keys["leisure"], string("park"));

    for (unsigned i = 0; i < paths[zoom - info.min]; i++) {
        Path pathItem;
        int32_t lon = 0, lat = 0;

        if (!(subFile.readVUInt32(unused) && subFile.readUInt16(bitmap)
              && subFile.readByte(sb)))
            return false;

        pathItem.layer = sb >> 4;
        int tags = sb & 0x0F;
//        printf("path[%d] info:---------------------- tags is %d\n",i,tags);
        if (!readTags(subFile, tags, pathTags, pathItem.tags))
            return false;

//        if (matchTag(pathItem, residentialTag)) {
//            pathItem.layer = 1;
//        }
//        if (matchTag(pathItem, parkTag)) {
//            pathItem.layer = 2;
//        }
//        if (matchTag(pathItem, waterTag)) {
//            pathItem.layer = 3;
//        }

//        for (const auto &tag: pathItem.tags) {
//            printf_d("(%s,%s) layer:%d", _keysSwap[tag.key].c_str(), tag.value.c_str(), pathItem.layer);
//        }

        if (!subFile.readByte(flags))
            return false;
        if (flags & 0x80) {
            if (!subFile.readString(name))
                return false;
//            printf("path[%d] name is %s tagLen: %d\n", i, name, tags);
//            name = name.split('\r').first();
//            有的字符串含有换行符\r 需要截取掉\r及后面的字符
            char *r_ptr = strchr(name, '\r');
            if (r_ptr != nullptr) {
                *r_ptr = 0;
            }
            pathItem.tags.emplace_back(ID_NAME, name);
        }
        if (flags & 0x40) {
            if (!subFile.readString(houseNumber))
                return false;
            pathItem.tags.emplace_back(ID_HOUSE, houseNumber);
        }
        if (flags & 0x20) {
            if (!subFile.readString(reference))
                return false;
            pathItem.tags.emplace_back(ID_REF, reference);
        }
        if (flags & 0x10) {
            if (!(subFile.readVInt32(lat) && subFile.readVInt32(lon)))
                return false;
        }
        if (flags & 0x08) {
            if (!subFile.readVUInt32(blocks) || !blocks)
                return false;
        } else
            blocks = 1;

        for (unsigned j = 0; j < blocks; j++) {
            if (!readPolygonPath(subFile, tile->pos, flags & 0x04, pathItem.poly))
                return false;
        }
        const vector<Coordinates> &outline = pathItem.poly.first();
        pathItem.closed = isClosed(outline);
        if (flags & 0x10) {

            pathItem.labelPos = Coordinates(float(outline.front().lon() + MD(lon)), float(outline.front().lat() + MD(lat)));

        }
        list->push_back(pathItem);
    }

    return true;
}

bool readPoints(SubFileInfo info, SubFile subFile, const VectorTile *tile, int zoom, vector<Point> *list, const vector<TagSource> &pointTags) {
    int rows = info.max - info.min + 1;
    vector<unsigned> points(rows);
    uint32_t val, unused, cnt = 0;
    uint8_t sb, flags;


    if (!subFile.seek(tile->offset & OFFSET_MASK))
        return false;

    for (int i = 0; i < rows; i++) {
        if (!(subFile.readVUInt32(val) && subFile.readVUInt32(unused)))
            return false;
        cnt += val;
        points[i] = cnt;
    }

    if (!subFile.readVUInt32(unused))
        return false;

    list->reserve(points[zoom - info.min]);

    for (unsigned i = 0; i < points[zoom - info.min]; i++) {
        int32_t lat, lon;

        if (!(subFile.readVInt32(lat) && subFile.readVInt32(lon)))
            return false;
        Point point(Coordinates(float(tile->pos.lon() + MD(lon)), float(tile->pos.lat() + MD(lat))));

        if (!subFile.readByte(sb))
            return false;
        point.layer = sb >> 4;
        int tags = sb & 0x0F;
        if (!readTags(subFile, tags, pointTags, point.tags))
            return false;

        if (!subFile.readByte(flags))
            return false;
        if (flags & 0x80) {
            if (!subFile.readString(name))
                return false;
            char *r_ptr = strchr(name, '\r');
            if (r_ptr != nullptr) {
                *r_ptr = 0;
            }
//            printf("tagName is %s nameLen is %zu\n", name, strlen(name));
//            name = name.split('\r').first();
            point.tags.emplace_back(ID_NAME, name);
        }
        if (flags & 0x40) {
            if (!subFile.readString(houseNumber))
                return false;
            point.tags.emplace_back(ID_HOUSE, houseNumber);
        }
        if (flags & 0x20) {
            int32_t elevation;
            if (!subFile.readVInt32(elevation))
                return false;
            point.tags.emplace_back(ID_ELE, to_string(elevation));
        }

        list->push_back(point);
    }

    return true;
}
