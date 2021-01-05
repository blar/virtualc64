// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AnyFile.h"
#include "Snapshot.h"
#include "RomFile.h"
#include "TAPFile.h"
#include "CRTFile.h"
#include "T64File.h"
#include "PRGFile.h"
#include "P00File.h"
#include "D64File.h"
#include "G64File.h"

AnyFile::AnyFile(usize capacity)
{
    data = new u8[capacity]();
    size = capacity;
}

AnyFile::~AnyFile()
{
    if (data) delete[] data;
}

bool
AnyFile::alloc(usize capacity)
{
    assert(data == nullptr);
    
    // dealloc();
    if ((data = new u8[capacity]()) == nullptr) return false;
    size = capacity;
    
    return true;
}

void
AnyFile::setPath(string path)
{
    this->path = path;
}

PETName<16>
AnyFile::getName()
{
    auto idx = path.rfind('/');
    usize start = idx != std::string::npos ? idx + 1 : 0;
    
    idx = path.rfind('.');
    usize len = idx != std::string::npos ? idx - start : std::string::npos;
    
    return PETName<16>(path.substr(start, len));
}

u64
AnyFile::fnv()
{
    return data ? fnv_1a_64(data, size) : 0;    
}

void
AnyFile::flash(u8 *buffer, size_t offset)
{
    assert(buffer);
    memcpy(buffer + offset, data, size);
}

usize
AnyFile::readFromFile(const char *path)
{
    assert(path);
        
    std::ifstream stream(path);

    if (!stream.is_open()) {
        throw Error(ERROR_CANT_READ);
    }
    
    usize result = readFromStream(stream);
    assert(result == size);
    
    setPath(path);
    return size;
}

usize
AnyFile::readFromBuffer(const u8 *buf, size_t len)
{
    assert(buf);

    std::istringstream stream(std::string((const char *)buf, len));
    
    usize result = readFromStream(stream);
    assert(result == size);
    return size;
}

usize
AnyFile::readFromStream(std::istream &stream)
{
    // Get stream size
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::end);
    fsize = stream.tellg() - fsize;
    stream.seekg(0, std::ios::beg);
                    
    // Read from stream
    if (!alloc((usize)fsize)) {
        throw Error(ERROR_OUT_OF_MEMORY);
    }
    assert((usize)fsize == size);
    stream.read((char *)data, size);
    
    // Repair the file (if applicable)
    repair();
    
    return size;
}

usize
AnyFile::writeToFile(const char *path)
{
    assert(path);
        
    std::ofstream stream(path);

    if (!stream.is_open()) {
        throw Error(ERROR_CANT_WRITE);
    }
    
    usize result = writeToStream(stream);
    assert(result == size);
    
    return size;
}

usize
AnyFile::writeToBuffer(u8 *buf)
{
    assert(buf);

    std::ostringstream stream;
    usize len = writeToStream(stream);
    stream.write((char *)buf, len);
    
    return len;
}

usize
AnyFile::writeToStream(std::ostream &stream)
{
    stream.write((char *)data, size);
    return size;
}
