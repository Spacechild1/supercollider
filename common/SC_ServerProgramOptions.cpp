//  Server program options, implementation
//
//  Copyright (C) 2009, 2010 Tim Blechmann, 2026 Christof Ressi
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#include "SC_ServerProgramOptions.hpp"
#include "SC_Version.hpp"
#include "clz.h"

#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp> // for physical_concurrency()

#ifdef _WIN32
#    define PATH_SEPARATOR ";"
#else
#    define PATH_SEPARATOR ":"
#endif

bool ServerProgramOptions::parse(int argc, char* argv[]) {
#ifdef NO_LIBSNDFILE
    static constexpr auto serverName = "supernova";
#else
    static constexpr auto serverName = "scsynth";
#endif

    using namespace boost::program_options;

    // prepare options
    options_description allOptions;

    options_description options("general options");
    // clang-format off
    options.add_options()
        ("help,h", "show this help")
        ("udp-port,u", value(&mUdpPort), "udp port")
        ("tcp-port,t", value(&mTcpPort), "tdp port")
        ("control-busses,c", value(&mNumControlBusChannels)->default_value(mNumControlBusChannels),
         "number of control busses")
        ("audio-busses,a", value(&mNumAudioBusChannels)->default_value(mNumAudioBusChannels), "number of audio busses")
        ("block-size,z", value(&mBufLength)->default_value(mBufLength), "audio block size")
        ("buffers,b", value(&mNumBuffers)->default_value(mNumBuffers), "number of sound buffers")
        ("max-nodes,n", value(&mMaxNodes)->default_value(mMaxNodes), "maximum number of server nodes")
        ("max-synthdefs,d", value(&mMaxGraphDefs)->default_value(mMaxGraphDefs), "maximum number of synthdefs")
        ("rt-memory,m", value(&mRealTimeMemorySize)->default_value(mRealTimeMemorySize),
         "size of real-time memory pool in kb")
        ("wires,w", value(&mMaxWireBufs)->default_value(mMaxWireBufs), "number of wire buffers")
        ("randomseeds,r", value(&mNumRGens)->default_value(mNumRGens), "number of random number generators")
        ("load-synthdefs,D", value(&mLoadGraphDefs)->default_value(mLoadGraphDefs), "load synthdefs? (1 or 0)")
        ("rendezvous,R", value(&mRendezvous)->default_value(mRendezvous), "publish to Rendezvous? (1 or 0)")
        ("max-logins,l", value(&mMaxLogins)->default_value(mMaxLogins), "maximum number of named return addresses")
        ("password,p", value(&mPasswordString)->default_value(""),
         "When using TCP, the session password must be the first command sent.\n"
         "The default is no password.\n"
         "UDP ports never require passwords, so for security use TCP.")
        ("nrt,N", value<std::vector<std::string>>()->multitoken(), "nrt synthesis <cmd-filename> <input-filename> "
         "<output-filename> <sample-rate> <header-format> <sample-format>")
        ("memory-locking,L", "enable memory locking")
        ("version,v", "print the supercollider version and exit")
        ("verbose,V", value(&mVerbosity)->default_value(mVerbosity), "verbosity: 0 is normal behaviour\n"
         "-1 suppresses informational messages\n"
         "-2 suppresses informational and many error messages, as well as\n"
         "messages from Poll.")
        ("ugen-search-path,U", value(&mUGenPluginPathString), "A list of paths separated by '" PATH_SEPARATOR "'.\n"
         "If specified, standard paths are NOT searched for plugins.\nMay be specified several times.")
        ("restricted-path,P", value(&mRestrictedPathString), "if specified, prevents file-accessing OSC commands "
         "from accessing files outside <restricted-path>")
        ("socket-address,B", value(&mBindAddressString)->default_value(mBindAddressString),
         "Bind the UDP or TCP socket to this address.\n"
         "Set to 0.0.0.0 to listen on all interfaces.")
#ifdef SUPERNOVA
        ("use-system-clock,C", value(&mUseSystemClock)->default_value(mUseSystemClock),
         "type of clock sampleclock=0 systemclock=1")
        ("threads,T", value(&mNumThreads)->default_value(boost::thread::physical_concurrency()),
         "number of audio threads")
        ("thread-pinning,y", value(&mThreadPinning)->default_value(mThreadPinning), "pin threads to CPU cores")
#endif
        ;
    // clang-format on
    allOptions.add(options);

    options_description audioOptions("audio options");
    // clang-format off
    audioOptions.add_options()
        ("hardware-buffer-size,Z", value(&mPreferredHardwareBufferFrameSize)->default_value(mPreferredHardwareBufferFrameSize),
         "hardware buffer size")
        ("samplerate,S", value(&mPreferredSampleRate)->default_value(mPreferredSampleRate), "hardware sample rate")
        ("inchannels,i", value(&mNumInputBusChannels)->default_value(mNumInputBusChannels), "number of input channels")
        ("outchannels,o", value(&mNumOutputBusChannels)->default_value(mNumOutputBusChannels), "number of output channels")
        ("hardware-device-name,H", value<std::vector<std::string>>()->multitoken(), "hardware device name")
#ifdef __APPLE__
        ("input-streams,I", value(&mInputStreams), "input streams enabled")
        ("output-streams,O", value(&mInputStreams), "output streams enabled")
        ("safety-clip-threshold,s", value(&mSafetyClipThreshold)->default_value(mSafetyClipThreshold),
         "absolute amplitude value outputs will be clipped to.\n"
         "Set to <= 0 or inf to completely disable clipping.")
#endif
        ;
    // clang-format on
    allOptions.add(audioOptions);

#ifdef SC_BELA
    options_description belaOptions("bela options");
    // clang-format off
    audioOptions.add_options()
        ("analog-inchannels,J", value(&mBelaAnalogInputChannels)->default_value(mBelaAnalogInputChannels),
         "analog input channels")
        ("analog-outchannels,K", value(&mBelaAnalogOutputChannels)->default_value(mBelaAnalogOutputChannels),
         "analog output channels")
        ("digital-channels,G", value(&mBelaDigitalChannels)->default_value(mBelaDigitalChannels), "digital channels")
        ("headphone-level,Q", value(&mBelaHeadphoneLevel)->default_value(mBelaHeadphoneLevel),
         "headphone level in dB (max: 0dB, min: -63.5dB)")
        ("pga-gain-left,X", value(&mBelaPgaGainLeft)->default_value(mBelaPgaGainLeft), "left PGA gain in dB")
        ("pga-gain-right,X", value(&mBelaPgaGainRight)->default_value(mBelaPgaGainRight), "right PGA gain in dB")
        ("speaker-mute,A", value(&mBelaSpeakerMuted)->default_value(mBelaSpeakerMuted), "mute speakers")
        ("dac-level,x", value(&mBelaDacLevel)->default_value(mBelaDacLevel), "DAC level in dB")
        ("adc-level,y", value(&mBelaAdcLevel)->default_value(mBelaAdcLevel), "ADC level in dB")
        ("mux-channels,g", value(&mBelaNumMuxChannels)->default_value(mBelaNumMuxChannels), "multiplexer channels")
        ("pru-id,T", value(&mBelaPru)->default_value(mBelaPru), "PRU id")
        ("scope-channels,E", value(&mBelaMaxScopeChannels)->default_value(mBelaMaxScopeChannels), "max. oscilloscope channels")
        ;
    // clang-format on
    allOptions.add(belaOptions);
#endif

    // parse options
    boost::program_options::variables_map vm;

    try {
        store(command_line_parser(argc, argv).options(allOptions).run(), vm);
    } catch (error const& e) { throw std::runtime_error(e.what()); }

    notify(vm);

    // some options need special handling

    // -h/--help
    if (vm.count("help")) {
        std::cout << allOptions << std::endl;
        return false;
    }

    // -v/--version
    if (vm.count("version")) {
        std::cout << serverName << " " << SC_VersionString() << " (" << SC_BuildString() << ")" << std::endl;
        return false;
    }

    // -B/--socket-address
    try {
        mBindAddress = boost::asio::ip::make_address(mBindAddressString);
    } catch (boost::system::system_error const& e) {
        throw std::runtime_error("Cannot parse '" + mBindAddressString + "' as a valid IP address.");
    };

    // -L/--memory-locking
    if (auto memoryLocking = vm.count("memory-locking") > 0) {
#if (_POSIX_MEMLOCK - 0) >= 200112L
        mMemoryLocking = true;
#else
        std::cout << "ignoring -L/--memory-locking option: memory locking not supported on this platform" << std::endl;
#endif
    }

    // -H/--hardware-device-name
    if (vm.count("hardware-device-name") > 0)
        mHardwareDeviceName = vm["hardware-device-name"].as<std::vector<std::string>>();

    // -N/--nrt
    mRealTime = vm.count("nrt") == 0;
    if (!mRealTime) {
        auto const& nrt_options = vm["nrt"].as<std::vector<std::string>>();
        if (nrt_options.size() != 6) {
            throw std::runtime_error("wrong number of arguments  for --nrt/-N option");
        }

#ifdef NO_LIBSNDFILE
        std::stringstream ss;
        ss << "NRT mode not supported: " << serverName << " compiled without libsndfile";
        throw std::runtime_error(ss.str());
#endif

        mCommandFile = nrt_options[0];
        mInputFile = nrt_options[1];
        mOutputFile = nrt_options[2];
        // NOTE: the sample rate provided with -N beats the one provided with -S!
        mPreferredSampleRate = boost::lexical_cast<uint32_t>(nrt_options[3]);
        mHeaderFormat = nrt_options[4];
        mSampleFormat = nrt_options[5];

        // assign the strings to the actual WorldOption arguments
        mNonRealTimeCmdFilename = mCommandFile != "_" ? mCommandFile.c_str() : nullptr;
        mNonRealTimeInputFilename = mInputFile != "_" ? mInputFile.c_str() : nullptr;
        mNonRealTimeOutputFilename = mOutputFile.c_str();
        mNonRealTimeOutputHeaderFormat = mHeaderFormat.c_str();
        mNonRealTimeOutputSampleFormat = mSampleFormat.c_str();
    }

    validateOptions();

    // some options are calculated from others (*after* they have been validated)
    if (mRealTime) {
        assert(mUdpPort > 0 && mTcpPort > 0);
        // use the port number as our shared memory ID
        mSharedMemoryID = mUdpPort > 0 ? mUdpPort : mTcpPort;
    }

    // finally, assign the strings to the actual WorldOption arguments
    if (!mPasswordString.empty())
        mPassword = mPasswordString.c_str();
    if (!mUGenPluginPathString.empty())
        mUGensPluginPath = mUGenPluginPathString.c_str();
    if (!mRestrictedPathString.empty())
        mRestrictedPath = mRestrictedPathString.c_str();
#ifdef __APPLE__
    if (!mInputStreams.empty())
        mInputStreamsEnabled = mInputStreams.c_str();
    if (!mOutputStreams.empty())
        mOutputStreamsEnabled = mOutputStreams.c_str();
#endif
    // The user can specify up to two hardware device names:
    // - two device names: use different input and output devices
    // - one device name: use the same device for input and output
    // - no device name: let the Server pick a default device.
    if (mHardwareDeviceName.size() > 0) {
        mInDeviceName = mHardwareDeviceName[0].c_str();
        if (mHardwareDeviceName.size() > 1) {
            mOutDeviceName = mHardwareDeviceName[1].c_str();
        } else {
            mOutDeviceName = mInDeviceName;
        }
    }

    return true;
}

void ServerProgramOptions::validateOptions() {
    // block size must be a power of two
    if (!ISPOWEROFTWO(mBufLength)) {
        throw std::runtime_error("Block size (" + std::to_string(mBufLength) + ") not a power of two.");
    }

    // hardware buffer size should be a power of two.
    if (auto wantedBufferSize = mPreferredHardwareBufferFrameSize;
        wantedBufferSize > 0 && !ISPOWEROFTWO(wantedBufferSize)) {
        mPreferredHardwareBufferFrameSize = NEXTPOWEROFTWO(wantedBufferSize);
        std::cout << "Hardware buffer size (" << wantedBufferSize << ") not a power of two! "
                  << "Setting to " << mPreferredHardwareBufferFrameSize << "." << std::endl;
    }

    // in non-realtime mode the sample rate cannot be zero
    if (!mRealTime && mPreferredSampleRate == 0) {
        throw std::runtime_error("Bad sample rate (" + std::to_string(mPreferredSampleRate) + ") for NRT synthesis.");
    }

    // realtime mode requires a UDP and/or TCP port
    if (mRealTime && mUdpPort == 0 && mTcpPort == 0) {
        throw std::runtime_error("Realtime mode requires a UDP and/or TCP port. "
                                 "See the -u/--udp-port and -t/--tcp-port options.");
    }

    // check if we have enough audio bus channels
    if (mNumInputBusChannels + mNumOutputBusChannels > mNumAudioBusChannels) {
        std::stringstream ss;
        ss << "Too few audio bus channels (" << mNumAudioBusChannels << ") for the given number of inputs ("
           << mNumInputBusChannels << ") and outputs (" << mNumOutputBusChannels << ")";
        throw std::runtime_error(ss.str());
    }
}
