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
#include "SC_Rate.h"
#include "SC_SndBuf.h"
#include "SC_RGen.h"
#include "HashTable.h"
#include "SC_World.h"
#include "SC_Reply.h"
#include "MsgFifo.h"
#include <deque>
#include <map>
#include <optional>
#include <set>

#include "boost/sync/semaphore.hpp"

#include "../../common/server_shm.hpp"

#ifndef NO_LIBSNDFILE
#    include <SC_SndFileHelpers.hpp> // includes sndfile.h with appropriate configuration
#endif

class AllocPool;
class SC_AudioDriver;

extern StringHashTable<struct UnitDef, Malloc> gUnitDefLib;


struct TriggerMsg {
    World* mWorld;
    int32 mNodeID;
    int32 mTriggerID;
    float mValue;

    void Perform();
};


struct NodeReplyMsg {
    World* mWorld;
    int32 mNodeID;
    int32 mID;
    int32 mNumArgs;
    float* mValues;
    int32 mCmdNameSize;
    char* mCmdName;
    void* mRTMemory;
    void Perform();
};


struct NodeEndMsg {
    World* mWorld;
    int32 mNodeID;
    int32 mGroupID;
    int32 mPrevNodeID;
    int32 mNextNodeID;
    int32 mIsGroup;
    int32 mHeadID;
    int32 mTailID;
    int32 mState;

    void Perform();
};

struct DeleteGraphDefMsg {
    struct GraphDef* mDef;

    void Perform();
};


typedef MsgFifoNoFree<TriggerMsg, 1024> TriggersFifo;
typedef MsgFifoNoFree<NodeReplyMsg, 1024> NodeReplyFifo;
typedef MsgFifoNoFree<NodeEndMsg, 1024> NodeEndsFifo;
typedef MsgFifoNoFree<DeleteGraphDefMsg, 512> DeleteGraphDefsFifo;
typedef StringHashTable<struct GraphDef, Malloc> GraphDefTable;
typedef IntHashTable<struct Node, AllocPool> NodeTable;

typedef std::map<struct ReplyAddress, uint32> ClientIDDict;
typedef std::deque<int> ClientIDs;
typedef std::set<ReplyAddress> Clients;

struct HiddenWorld {
    std::unique_ptr<AllocPool> mAllocPool;
    NodeTable mNodeLib;
    GraphDefTable mGraphDefLib;
    uint32 mMaxUsers = 0;
    Clients mUsers;
    ClientIDs mAvailableClientIDs;
    ClientIDDict mClientIDdict;

    std::unique_ptr<SC_AudioDriver> mAudioDriver;
    std::string mPassword;

    uint32 mMaxWireBufs = 0;
    float* mWireBufSpace = nullptr;

    TriggersFifo mTriggers;
    NodeReplyFifo mNodeMsgs;
    NodeEndsFifo mNodeEnds;
    DeleteGraphDefsFifo mDeleteGraphDefs;

    boost::sync::semaphore mQuitProgram { 0 };
    bool mTerminating = false;

#ifndef NO_LIBSNDFILE
    SNDFILE* mNRTInputFile = nullptr;
    SNDFILE* mNRTOutputFile = nullptr;
    FILE* mNRTCmdFile = nullptr;
#endif

    int32 mHiddenID = -8;
    int32 mRecentID = -8;

#ifdef __APPLE__
    std::optional<std::string> mInputStreamsEnabled;
    std::optional<std::string> mOutputStreamsEnabled;
#endif

#ifdef SC_BELA
    uint32 mBelaAnalogInputChannels;
    uint32 mBelaAnalogOutputChannels;
    uint32 mBelaDigitalChannels;
    float mBelaHeadphoneLevel;
    float mBelaPgaGainLeft;
    float mBelaPgaGainRight;
    bool mBelaSpeakerMuted;
    float mBelaDacLevel;
    float mBelaAdcLevel;
    uint32 mBelaNumMuxChannels;
    uint32 mBelaPru;
#endif

    std::string mInDeviceName;
    std::string mOutDeviceName;
    std::unique_ptr<server_shared_memory_creator> mShmem;
};

inline SC_AudioDriver* GetAudioDriver(World* inWorld) { return inWorld->hw->mAudioDriver.get(); }
