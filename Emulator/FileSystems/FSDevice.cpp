// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSDevice *
FSDevice::makeWithFormat(FSDeviceDescriptor &layout)
{
    FSDevice *dev = new FSDevice(layout.numBlocks());
    dev->layout = layout;
        
    /*
    if (FS_DEBUG) {
        printf("cd = %d\n", dev->cd);
        dev->info();
        dev->dump();
    }
    */
    
    return dev;
}

FSDevice *
FSDevice::makeWithFormat(DiskType type)
{
    FSDeviceDescriptor layout = FSDeviceDescriptor(type);
    return makeWithFormat(layout);
}

FSDevice *
FSDevice::makeWithD64(D64File *d64, FSError *error)
{
    assert(d64);

    // Get device descriptor
    FSDeviceDescriptor descriptor = FSDeviceDescriptor(DISK_SS_SD);
        
    // Create the device
    FSDevice *device = makeWithFormat(descriptor);

    // Import file system
    if (!device->importVolume(d64->getData(), d64->getSize(), error)) {
        delete device;
        return nullptr;
    }
    
    return device;
}

FSDevice *
FSDevice::makeWithArchive(AnyArchive *archive, FSError *error)
{
    assert(archive);
    
    // Get device descriptor
    FSDeviceDescriptor descriptor = FSDeviceDescriptor(DISK_SS_SD);
        
    // Create the device
    FSDevice *device = makeWithFormat(descriptor);
        
    // Write BAM
    FSName name = archive->getFSName();
    device->blockPtr(18,0)->writeBAM(name);

    // Create the proper amount of directory blocks
    int numberOfItems = archive->numberOfItems();
    device->setCapacity(numberOfItems);

    // Loop over all entries in archive
    for (int i = 0; i < numberOfItems; i++) {
        
        u8 *buf; size_t cnt;

        archive->selectItem(i);
        archive->getItem(&buf, &cnt);

        device->makeFile(archive->getNameOfItem(), buf, cnt);
        delete[](buf);
    }
    
    device->printDirectory();
        
    return device;
}

FSDevice::FSDevice(u32 capacity)
{
    debug(FS_DEBUG, "Creating device with %d blocks\n", capacity);
    
    // Initialize the block storage
    blocks.reserve(capacity);
    blocks.assign(capacity, 0);
    
    // Create all blocks
    for (u32 i = 0; i < capacity; i++) blocks[i] = new FSBlock(*this, i);
}

FSDevice::~FSDevice()
{
    for (auto &b : blocks) delete b;
}

void
FSDevice::info()
{
}

void
FSDevice::dump()
{
    // Dump all blocks
    for (size_t i = 0; i < blocks.size(); i++)  {
        
        msg("\nBlock %d (%d):", i, blocks[i]->nr);
        msg(" %s\n", sFSBlockType(blocks[i]->type()));
        
        blocks[i]->dump();
    }
}

void
FSDevice::printDirectory()
{
    auto dir = scanDirectory();
    
    for (auto &item : dir) {
        msg("%3d \"%16s\" %s\n",
            HI_LO(item->fileSizeHi, item->fileSizeLo),
            item->getName().c_str(), item->typeString());
    }
}

FSBlockType
FSDevice::blockType(u32 nr)
{
    return blockPtr(nr) ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
}

FSItemType
FSDevice::itemType(u32 nr, u32 pos)
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSDevice::blockPtr(Block b)
{
    return b < blocks.size() ? blocks[b] : nullptr;
}

FSBlock *
FSDevice::blockPtr(BlockRef ref)
{
    Block b;
    layout.translateBlockNr(&b, ref.t, ref.s);
    
    return blockPtr(b);
}

FSBlock *
FSDevice::blockPtr(Track t, Sector s)
{
    Block b;
    layout.translateBlockNr(&b, t, s);
    
    return blockPtr(b);
}

FSBlock *
FSDevice::nextBlockPtr(Block b)
{
    FSBlock *ptr = blockPtr(b);
    
    // Jump to linked block
    if (ptr) { ptr = blockPtr(ptr->data[0], ptr->data[1]); }
    
    return ptr;
}

FSBlock *
FSDevice::nextBlockPtr(Track t, Sector s)
{
    FSBlock *ptr = blockPtr(t, s);
    
    // Jump to linked block
    if (ptr) { ptr = blockPtr(ptr->data[0], ptr->data[1]); }
    
    return ptr;
}

FSBlock *
FSDevice::nextBlockPtr(FSBlock *ptr)
{
    return ptr ? blockPtr(ptr->data[0], ptr->data[1]) : nullptr;
}

bool
FSDevice::writeByte(u8 byte, Track *pt, Sector *ps, u32 *pOffset)
{
    Track t = *pt;
    Sector s = *ps;
    u32 offset = *pOffset;
    
    assert(layout.isTrackSectorPair(t, s));
    assert(offset >= 2 && offset <= 0x100);
    
    FSBlock *ptr = blockPtr(t, s);
        
    // No free slots in this sector, proceed to next one
    if (offset == 0x100) {
        
        // Only proceed if there is space left on the disk
        if (!layout.nextTrackAndSector(t, s, &t, &s)) return false;
                
        // Mark the new block as allocated
        markAsAllocated(t, s);
        
        // Link previous sector with the new one
        ptr->data[0] = (u8)t;
        ptr->data[1] = (u8)s;
        Track t2; Sector s2; layout.translateBlockNr(ptr->nr, &t2, &s2);

        ptr = blockPtr(t, s);
        offset = 2;
    }
    
    // Write byte
    ptr->data[offset] = byte;
    
    *pt = t;
    *ps = s;
    *pOffset = offset + 1;
    return true;
}

bool
FSDevice::isFree(Block b)
{
    u32 byte, bit;
    FSBlock *bam = locateAllocationBit(b, &byte, &bit);
    
    return GET_BIT(bam->data[byte], bit);
}

bool
FSDevice::isFree(BlockRef ref)
{
    u32 byte, bit;
    FSBlock *bam = locateAllocationBit(ref, &byte, &bit);
    
    return GET_BIT(bam->data[byte], bit);
}

bool
FSDevice::isFree(Track t, Sector s)
{
    u32 byte, bit;
    FSBlock *bam = locateAllocationBit(t, s, &byte, &bit);
    
    return GET_BIT(bam->data[byte], bit);
}

BlockRef
FSDevice::nextFreeBlock(BlockRef ref)
{
    if (!layout.isValidRef(ref)) return {0,0};
    
    while (ref.t && !isFree(ref)) {
        ref = layout.nextBlockRef(ref);
    }
    
    return ref;
}

void
FSDevice::setAllocationBit(Block b, bool value)
{
    Track t; Sector s;
    layout.translateBlockNr(b, &t, &s);

    setAllocationBit(t, s, value);
}

void
FSDevice::setAllocationBit(Track t, Sector s, bool value)
{
    u32 byte, bit;
    FSBlock *bam = locateAllocationBit(t, s, &byte, &bit);

    if (value && !GET_BIT(bam->data[byte], bit)) {

        // Mark sector as free
        SET_BIT(bam->data[byte], bit);

        // Increase the number of free sectors
        bam->data[byte & ~0b11]++;
    }
    
    if (!value && GET_BIT(bam->data[byte], bit)) {
        
        // Mark sector as allocated
        CLR_BIT(bam->data[byte], bit);
        
        // Decrease the number of free sectors
        bam->data[byte & ~0b11]--;
    }
}

std::vector<BlockRef>
FSDevice::allocate(BlockRef ref, u32 n)
{
    assert(n > 0);
    
    std::vector<BlockRef> result;
    FSBlock *block = nullptr;

    // Get to the next free block
    ref = nextFreeBlock(ref);

    if (ref.t) {
        
        for (u32 i = 0; i < n; i++) {
            
            // Collect block reference
            result.push_back(ref);
            markAsAllocated(ref.t, ref.s);
            
            // Link this block
            block = blockPtr(ref.t, ref.s);
            layout.nextTrackAndSector(ref.t, ref.s, &ref.t, &ref.s);
            block->data[0] = ref.t;
            block->data[1] = ref.s;
        }
        
        // Delete the block reference in the last block
        block->data[0] = 0;
        block->data[1] = 0;
    }
    
    return result;
}

FSBlock *
FSDevice::locateAllocationBit(Block b, u32 *byte, u32 *bit)
{
    assert(b < blocks.size());
    
    Track t; Sector s;
    layout.translateBlockNr(b, &t, &s);
    
    return locateAllocationBit(t, s, byte, bit);
}

FSBlock *
FSDevice::locateAllocationBit(BlockRef ref, u32 *byte, u32 *bit)
{
    assert(layout.isValidRef(ref));
        
    /* Bytes $04 - $8F store the BAM entries for each track, in groups of four
     * bytes per track, starting on track 1. [...] The first byte is the number
     * of free sectors on that track. The next three bytes represent the bitmap
     * of which sectors are used/free. Since it is 3 bytes we have 24 bits of
     * storage. Remember that at most, each track only has 21 sectors, so there
     * are a few unused bits.
     */
        
    *byte = (4 * ref.t) + 1 + (ref.s >> 3);
    *bit = ref.s & 0x07;
    
    return bamPtr();
}

FSBlock *
FSDevice::locateAllocationBit(Track t, Sector s, u32 *byte, u32 *bit)
{
    assert(layout.isTrackSectorPair(t, s));
        
    /* Bytes $04 - $8F store the BAM entries for each track, in groups of four
     * bytes per track, starting on track 1. [...] The first byte is the number
     * of free sectors on that track. The next three bytes represent the bitmap
     * of which sectors are used/free. Since it is 3 bytes we have 24 bits of
     * storage. Remember that at most, each track only has 21 sectors, so there
     * are a few unused bits.
     */
        
    *byte = (4 * t) + 1 + (s >> 3);
    *bit = s & 0x07;
    
    return bamPtr();
}

/*
FSDirEntry *
FSDevice::seek(u32 nr)
{
    return nullptr;
}
*/

FSDirEntry *
FSDevice::nextFreeDirEntry()
{
    // The directory starts on track 18, sector 1
    FSBlock *ptr = blockPtr(18, 1);
    
    // The number of files is limited by 144
    for (int i = 0; ptr && i < 144; i++) {
    
        FSDirEntry *entry = (FSDirEntry *)ptr->data + (i % 8);
        
        // Return if this entry is not used
        if (entry->isEmpty()) return entry;
     
        // Jump to the next sector if this was the last directory item
        if (i % 8 == 7) ptr = nextBlockPtr(ptr);
    }
    
    return nullptr;
}

std::vector<FSDirEntry *>
FSDevice::scanDirectory(bool skipInvisible)
{
    std::vector<FSDirEntry *> result;
    
    // The directory starts on track 18, sector 1
    FSBlock *ptr = blockPtr(18, 1);
    
    // The number of files is limited by 144
    for (int i = 0; ptr && i < 144; i++) {
    
        FSDirEntry *entry = (FSDirEntry *)ptr->data + (i % 8);
        
        // Terminate if there are no more entries
        if (entry->isEmpty()) break;

        // Add file to the result list
        if (!(skipInvisible && entry->isHidden())) result.push_back(entry);
     
        // Jump to the next sector if this was the last directory item
        if (i % 8 == 7) ptr = nextBlockPtr(ptr);
    }
    
    return result;
}

bool
FSDevice::setCapacity(u32 n)
{
    // A disk can hold up to 144 files
    assert(n <= 144);
    
    // Determine how many directory blocks are needed
    u32 numBlocks = (n + 7) / 8;
    debug(FS_DEBUG, "Allocation %d directory blocks for %d files\n", numBlocks, n);

    // The firsr directory block is located at (18,1)
    Track t = 18;
    Sector s = 1;
    FSBlock *ptr = blockPtr(t, s);
    
    for (u32 i = 1; i < numBlocks; i++, ptr = blockPtr(t, s)) {

        // Get location of the next block
        if (!layout.nextTrackAndSector(t, s, &t, &s)) return false;
        
        // Link blocks
        ptr->data[0] = t;
        ptr->data[1] = s;
    }
    
    return true;
}

bool
FSDevice::makeFile(const char *name, const u8 *buf, size_t cnt)
{
    // The directory starts on track 18, sector 1
    FSBlock *ptr = blockPtr(18, 1);
    
    // Search for the next free slot
    for (int i = 0; ptr && i < 144; i++) {
    
        FSDirEntry *dir = (FSDirEntry *)ptr->data + (i % 8);
        
        // Create the file once we've found a free slot
        if (dir->isEmpty()) return makeFile(name, dir, buf, cnt);
     
        // Jump to the next sector if this was the last directory item
        if (i % 8 == 7) ptr = nextBlockPtr(ptr);
    }
    
    return true;
}

bool
FSDevice::makeFile(const char *name, FSDirEntry *dir, const u8 *buf, size_t cnt)
{
    // Determine the number of blocks needed for this file
    u32 numBlocks = (u32)((cnt + 253) / 254);
    
    // Allocate data blocks
    auto blockList = allocate(numBlocks);
    if (blockList.empty()) return false;
        
    auto it = blockList.begin();
    FSBlock *ptr = blockPtr(*it);
    
    // Write data
    size_t i, j;
    for (i = 0, j = 2; i < cnt; i++, j++) {

        if (j == 0x100) {
            ++it;
            ptr = blockPtr(*it);
            j = 2;
            Block b;
            layout.translateBlockNr(&b, it->t, it->s);
        }
        ptr->data[j] = buf[i];
    }
 
    // Write directory entry
    dir->init(name, blockList[0], numBlocks);
    
    return true;
}

FSErrorReport
FSDevice::check(bool strict)
{
    FSErrorReport result;

    long total = 0, min = LONG_MAX, max = 0;
    size_t numBlocks = blocks.size();
    
    // Analyze all blocks
    for (u32 i = 0; i < numBlocks; i++) {

        if (blocks[i]->check(strict) > 0) {
            min = MIN(min, i);
            max = MAX(max, i);
            blocks[i]->corrupted = (u32)++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    if (total) {
        result.corruptedBlocks = total;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    } else {
        result.corruptedBlocks = 0;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    }
    
    return result;
}

FSError
FSDevice::check(u32 blockNr, u32 pos, u8 *expected, bool strict)
{
    return blocks[blockNr]->check(pos, expected, strict);
}

u32
FSDevice::getCorrupted(u32 blockNr)
{
    return blockPtr(blockNr) ? blocks[blockNr]->corrupted : 0;
}

bool
FSDevice::isCorrupted(u32 blockNr, u32 n)
{
    size_t numBlocks = blocks.size();
    
    for (u32 i = 0, cnt = 0; i < numBlocks; i++) {
        
        if (isCorrupted(i)) {
            cnt++;
            if (blockNr == i) return cnt == n;
        }
    }
    return false;
}

u32
FSDevice::nextCorrupted(u32 blockNr)
{
    size_t numBlocks = blocks.size();
    
    for (u32 i = blockNr + 1; i < numBlocks; i++) {
        if (isCorrupted(i)) return i;
    }
    return blockNr;
}

u32
FSDevice::prevCorrupted(u32 blockNr)
{
    size_t numBlocks = blocks.size();
    
    for (u32 i = blockNr - 1; i < numBlocks; i--) {
        if (isCorrupted(i)) return i;
    }
    return blockNr;
}

u8
FSDevice::readByte(u32 block, u32 offset)
{
    assert(offset < 256);
    assert(block < blocks.size());
    
    return blocks[block]->data[offset];
}

bool
FSDevice::importVolume(const u8 *src, size_t size, FSError *error)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system (%zu bytes)...\n", size);

    // Only proceed if the buffer size matches
    if (blocks.size() * 256 != size) {
        warn("BUFFER SIZE MISMATCH (%d %d)\n", blocks.size(), blocks.size() * 256);
        if (error) *error = FS_WRONG_CAPACITY;
        return false;
    }
        
    // Import all blocks
    for (u32 i = 0; i < blocks.size(); i++) {
        
        const u8 *data = src + i * 256;
        blocks[i]->importBlock(data);
    }
    
    if (error) *error = FS_OK;

    if (FS_DEBUG) {
        
        // info();
        // dump();
        printDirectory();
    }
    
    return true;
}

bool
FSDevice::exportVolume(u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(0, layout.numBlocks() - 1, dst, size, error);
}

bool
FSDevice::exportBlock(u32 nr, u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSDevice::exportBlocks(u32 first, u32 last, u8 *dst, size_t size, FSError *error)
{
    assert(last < layout.numBlocks());
    assert(first <= last);
    assert(dst);
    
    u32 count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %d blocks (%d - %d)\n", count, first, last);

    // Only proceed if the source buffer contains the right amount of data
    if (count * 256 != size) {
        if (error) *error = FS_WRONG_CAPACITY;
        return false;
    }
        
    // Wipe out the target buffer
    memset(dst, 0, size);
    
    // Export all blocks
    for (u32 i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * 256);
    }

    debug(FS_DEBUG, "Success\n");
    
    if (error) *error = FS_OK;
    return true;
}

bool
FSDevice::exportFile(FSDirEntry *item, const char *path, FSError *error)
{
    debug(FS_DEBUG, "Exporting file %s to %s\n", item->getName().c_str(), path);

    assert(false);
}

bool
FSDevice::exportDirectory(const char *path, FSError *err)
{
    assert(path);
        
    // Only proceed if path points to an empty directory
    long numItems = numDirectoryItems(path);
    if (numItems != 0) return FS_DIRECTORY_NOT_EMPTY;
    
    // Collect all directory entries
    auto items = scanDirectory();
    
    // Export all items
    for (auto const& item : items) {

        if (!exportFile(item, path, err)) {
            msg("Export error: %d\n", err);
            return false;
        }
    }
    
    msg("Exported %d items", items.size());
    *err = FS_OK;
    return true;
}