/*
    SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#pragma once

#include "SC_Types.h"
#include "SC_BoundsMacros.h"
#include "SC_Str4.h"
#include "Hash.h"
#include <cassert>
#include <cstddef>
#include <stdexcept>

template <class Key, class Item, class Allocator> class HashTable {
    Allocator* mPool;
    int32 mNumItems, mMaxItems, mTableSize, mHashMask;
    Item** mItems;
    bool mCanResize;

public:
    HashTable():
        mPool(nullptr),
        mNumItems(0),
        mMaxItems(0),
        mTableSize(0),
        mHashMask(0),
        mItems(nullptr),
        mCanResize(false) {}

    HashTable(Allocator* inPool, int32 inMaxItems, bool inCanResize = true): HashTable() {
        Init(inPool, inMaxItems, inCanResize);
    }

    ~HashTable() {
        if (mPool)
            mPool->Free(mItems);
    }

    HashTable(const HashTable& other) {
        mPool = other.mPool;
        mNumItems = other.mNumItems;
        mMaxItems = other.mMaxItems;
        mTableSize = other.mTableSize;
        mHashMask = other.mHashMask;
        mCanResize = other.mCanResize;

        mItems = AllocTable(mTableSize);
        for (int32 i = 0; i < mTableSize; ++i) {
            mItems[i] = other.mItems[i];
        }
    }

    HashTable(HashTable&& other) noexcept {
        mPool = other.mPool;
        mNumItems = other.mNumItems;
        mMaxItems = other.mMaxItems;
        mTableSize = other.mTableSize;
        mHashMask = other.mHashMask;
        mItems = other.mItems;
        mCanResize = other.mCanResize;
        // invalidate 'other', see ~HashTable()
        other.mPool = nullptr;
    }

    HashTable& operator=(const HashTable& other) {
        if (mPool)
            mPool->Free(mItems);
        mPool = other.mPool;
        mNumItems = other.mNumItems;
        mMaxItems = other.mMaxItems;
        mTableSize = other.mTableSize;
        mHashMask = other.mHashMask;
        mCanResize = other.mCanResize;

        mItems = AllocTable(mTableSize);
        for (int32 i = 0; i < mTableSize; ++i) {
            mItems[i] = other.mItems[i];
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (mPool)
            mPool->Free(mItems);
        mPool = other.mPool;
        mNumItems = other.mNumItems;
        mMaxItems = other.mMaxItems;
        mTableSize = other.mTableSize;
        mHashMask = other.mHashMask;
        mItems = other.mItems;
        mCanResize = other.mCanResize;
        // invalidate 'other', see ~HashTable()
        other.mPool = nullptr;
        return *this;
    }

    void Init(Allocator* inPool, int32 inMaxItems, bool inCanResize = true) {
        assert(inPool != nullptr && inMaxItems > 0);
        if (mPool)
            mPool->Free(mItems);
        mPool = inPool;
        mNumItems = 0;
        mMaxItems = inMaxItems;
        mTableSize = mMaxItems << 1;
        mItems = AllocTable(mTableSize);
        mHashMask = mTableSize - 1;
        mCanResize = inCanResize;
    }

    [[nodiscard]] int32 TableSize() const { return mTableSize; }
    [[nodiscard]] int32 MaxItems() const { return mMaxItems; }
    [[nodiscard]] int32 NumItems() const { return mNumItems; }

    void MakeEmpty() {
        for (int i = 0; i < mTableSize; ++i) {
            mItems[i] = 0;
        }
        mNumItems = 0;
    }

    void Resize() {
        assert(mPool != nullptr);
        int32 newSize = sc_max(mTableSize << 1, 32);
        int32 oldSize = mTableSize;
        Item** oldItems = mItems;
        mItems = AllocTable(newSize);
        mTableSize = newSize;
        mMaxItems = mTableSize >> 1;
        mHashMask = mTableSize - 1;
        mNumItems = 0;
        for (int i = 0; i < oldSize; ++i) {
            Item* item = oldItems[i];
            if (item)
                Add(item);
        }
        mPool->Free(oldItems);
        // printf("mMaxItems %d   mTableSize %d   newSize %d\n", mMaxItems, mTableSize, newSize);
    }

    bool Add(Item* inItem) {
        // printf("mNumItems %d\n", mNumItems);
        // printf("mMaxItems %d\n", mMaxItems);
        // printf("mCanResize %d\n", mCanResize);
        if (mNumItems >= mMaxItems) {
            if (!mCanResize)
                return false;
            Resize();
        }

        // printf("GetHash(inItem) %d\n", GetHash(inItem));
        // printf("GetKey(inItem) %s\n", GetKey(inItem));
        int32 index = IndexFor(GetHash(inItem), GetKey(inItem));
        // printf("index %d\n", index);

        Item* item = mItems[index];
        if (item)
            return item == inItem;

        mItems[index] = inItem;
        mNumItems++;
        return true;
    }

    bool Remove(Item* inItem) {
        int32 index = IndexFor(GetHash(inItem), GetKey(inItem));
        if (mItems[index] != inItem)
            return false;
        mItems[index] = 0;

        FixCollisionsFrom(index);
        mNumItems--;
        return true;
    }

    bool RemoveKey(Key inKey) {
        Item* item = Get(inKey);
        if (!item)
            return false;
        return Remove(item);
    }

    [[nodiscard]] Item* Get(Key inKey) const { return Get(Hash(inKey), inKey); }

    [[nodiscard]] Item* Get(int32 inHashID, Key inKey) const {
        // printf("Get hash %d %s\n", inHashID, inKey);
        int32 index = IndexFor(inHashID, inKey);
        // printf("index %d\n", index);
        return mItems[index];
    }

    [[nodiscard]] bool Includes(Item* inItem) const { return Get(GetHash(inItem), GetKey(inItem)) == inItem; }

    [[nodiscard]] Item* AtIndex(int32 inIndex) const { return mItems[inIndex]; }

private:
    Item** AllocTable(int inTableSize) {
        assert(mPool != nullptr);
        size_t size = inTableSize * sizeof(Item*);
        Item** items = static_cast<Item**>(mPool->Alloc(size));
        if (items == nullptr)
            throw std::runtime_error("FAILURE IN SERVER: HashTable allocation failed: out of memory!\n");
        for (int i = 0; i < inTableSize; ++i) {
            items[i] = 0;
        }
        return items;
    }

    int32 IndexFor(int32 inHashID, Key inKey) const {
        assert(mTableSize > 0);
        int index = inHashID & mHashMask;
        for (;;) {
            Item* item = mItems[index];
            if (!item)
                return index;
            // select the appropriate comparison method at compile time
            if constexpr (std::is_same_v<Key, const int32*>) {
                // special case for OSC strings
                if (GetHash(item) == inHashID && str4eq(inKey, GetKey(item)))
                    return index;
            } else if constexpr (std::is_integral_v<Key>) {
                if (GetHash(item) == inHashID && inKey == GetKey(item))
                    return index;
            } else {
                // Until C++23 the first argument for static_assert() must depend on a template parameter,
                // otherwise the compiler is allowed to evaluate it before instantiation.
                static_assert(!sizeof(Key), "HashTable: Key must be an integral value or an OSC string (const int32*)");
            }
            index = (index + 1) & mHashMask;
        }
    }

    void FixCollisionsFrom(int32 inIndex) {
        int oldIndex = inIndex;
        for (;;) {
            oldIndex = (oldIndex + 1) & mHashMask;
            Item* oldItem = mItems[oldIndex];
            if (!oldItem)
                break;
            int newIndex = IndexFor(GetHash(oldItem), GetKey(oldItem));
            if (oldIndex != newIndex) {
                mItems[oldIndex] = mItems[newIndex];
                mItems[newIndex] = oldItem;
            }
        }
    }
};

template <class Item, class Allocator> using StringHashTable = HashTable<const int32*, Item, Allocator>;

template <class Item, class Allocator> using IntHashTable = HashTable<int32, Item, Allocator>;

struct Malloc {
    void Free(void* ptr) { free(ptr); }
    void* Alloc(size_t size) { return malloc(size); }
};
