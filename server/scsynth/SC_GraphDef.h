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

#include "SC_SynthDef.h"
#include "HashTable.h"
#include <filesystem>

struct UnitSpec;
struct World;

struct ParamSpec {
    int32 mName[kSCNameLen];
    int32 mIndex;
    int32 mHash;
    int32 mNumChannels;
};

inline int32 GetHash(const ParamSpec* inParamSpec) { return inParamSpec->mHash; }
inline const int32* GetKey(const ParamSpec* inParamSpec) { return inParamSpec->mName; }

typedef StringHashTable<ParamSpec, Malloc> ParamSpecTable;

/** \note Relevant scsynth code: `GraphDef_Read(World *, char*&, GraphDef*, int32)`
 *  \note Relevant supernova code: `sc_synthdef::prepare(void)`
 */
struct GraphDef {
    NodeDef mNodeDef;

    int32 mRefCount;

    uint32 mNumParamSpecs;
    ParamSpec* mParamSpecs;
    ParamSpecTable mParamSpecTable;

    uint32 mNumControls;
    uint32 mNumAudioControls;

    uint32 mNumWires;
    uint32 mNumConstants;
    uint32 mNumUnitSpecs;
    uint32 mNumWireBufs;
    uint32 mNumCalcUnits;

    float32* mInitialControlValues;
    float32* mConstants;

    UnitSpec* mUnitSpecs;

    size_t mWiresAllocSize;
    size_t mUnitsAllocSize;
    size_t mCalcUnitsAllocSize;
    size_t mControlAllocSize;
    size_t mMapControlsAllocSize;
    size_t mMapControlRatesAllocSize;
    size_t mAudioMapBusOffsetSize;

    int32 mBlockSize;
    uint32 mBlockSizeIndex;

    float32 mResampleFactor;
    uint32 mResampleIndex;

    GraphDef* mNext;
    GraphDef* mOriginal;

    uint32 mNumVariants;
    GraphDef* mVariants;
};

inline const int32* GetKey(GraphDef* inGraphDef) { return inGraphDef->mNodeDef.mName; }
inline int32 GetHash(GraphDef* inGraphDef) { return inGraphDef->mNodeDef.mHash; }

GraphDef* GraphDef_Recv(World* inWorld, const char* buffer, size_t size, GraphDef* inList);
GraphDef* GraphDef_Load(World* inWorld, const std::filesystem::path& path, GraphDef* inList);
GraphDef* GraphDef_LoadDir(World* inWorld, const std::filesystem::path& path, GraphDef* inList);
GraphDef* GraphDef_LoadGlob(World* inWorld, const char* pattern, GraphDef* inList);
SCErr GraphDef_Remove(World* inWorld, const int32* inName);
SCErr GraphDef_DeleteMsg(World* inWorld, GraphDef* inDef);
void GraphDef_Dump(GraphDef* inGraphDef);
