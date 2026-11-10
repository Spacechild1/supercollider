/*
 *  Copyright (C) Moss Heim, 2017, Christof Ressi 2026. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA
 */

#pragma once

#include "SC_APIVersion.h"

#include <cassert>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace sc {

/// Contains functions that generating the appropriate error message
/// for a particular error condition.
namespace ErrorMessage {

/// String used to indent all messages.
constexpr const char* indent = "    ";

/// Returns the error message for a major API version mismatch
std::string majorApiVersionMismatch(std::string_view utf8Filename, int version) {
    // both 1 and 2 were introduced in 3.6.
    static const std::unordered_map<int, std::string> scVersionForAPIVersion = {
        { 1, "3.6.0" }, { 2, "3.6.0" }, { 3, "3.9.0" }, { 4, "3.15" }
    };

    int majorVersion = SC_GET_MAJOR_VERSION(version);
    int minorVersion = SC_GET_MINOR_VERSION(version);

    std::stringstream ss;
    ss << "ERROR: API version mismatch: " << utf8Filename << "\n";
    try {
        // majorVersion + 1: the version that broke compatibility
        const auto& scVersion = scVersionForAPIVersion.at(majorVersion + 1);
        ss << indent << "This plugin is not compatible with SuperCollider >= " << scVersion << ".\n";
        ss << indent << "The plugin has not been loaded; please find or compile a newer version.\n";
    } catch (const std::out_of_range& exception) {
        ss << indent << "This plugin has been built against a future API version.\n";
        ss << indent << "You may need to update SuperCollider in order to use it.\n";
    }
    ss << indent << "(Plugin's API version: " << majorVersion << "." << minorVersion
       << ", expected: " << SC_API_VERSION_MAJOR << ".x)\n";

    // if it looks like sc3-plugins, give specific info to help
    if (utf8Filename.find("SC3plugins") != std::string_view::npos) {
        ss << indent << "Releases of sc3-plugins can be downloaded from "
           << "https://github.com/supercollider/sc3-plugins/releases\n";
    }

    return ss.str();
}

/// Returns an error message for the case where the min. required API version is not met.
std::string minApiVersionNotMet(std::string_view utf8Filename, int minVersion) {
    int majorVersion = SC_GET_MAJOR_VERSION(minVersion);
    int minorVersion = SC_GET_MINOR_VERSION(minVersion);

    std::stringstream ss;
    ss << "ERROR: minimum API version requirement not met: " << utf8Filename << "\n";
    ss << indent << "This plugin requires at least API version " << majorVersion << "." << minorVersion << ",\n";
    ss << indent << "but the Server only supports " << SC_API_VERSION_MAJOR << "." << SC_API_VERSION_MINOR
       << " at most.\n";
    ss << indent << "You need to update SuperCollider in order to use this plugin.\n";

    return ss.str();
}

/// Returns an error message for a missing API version.
std::string apiVersionNotFound(std::string_view utf8Filename) {
    return "ERROR: API version not found: " + std::string(utf8Filename) + "\n" + indent
        + "This file may not be a SuperCollider plugin.\n";
}

} // namespace ErrorMessage

/** @brief Function pointer type for reporting the plugin API version or Server type. */
using PluginInfoFunction = int (*)();

/** @brief Checks the plugin API version and returns the appropriate error message.
 * @param utf8Filename The filename of the plugin represented as a UTF-8 string.
 * @param versionFunc The function that returns the plugin's API version. Can be nullptr!
 * @param minVersionFunc The function that returns the min. required API version. Can be nullptr!
 * @returns `true` + and empty string on success, `false` + an error message on failure.
 */
std::pair<bool, std::string> checkAPIVersion(std::string_view utf8Filename, void* versionFunc, void* minVersionFunc) {
    if (!versionFunc) {
        return { false, ErrorMessage::apiVersionNotFound(utf8Filename) };
    }

    if (!minVersionFunc) {
        // before SC 3.15, "api_version" only returned the major version
        assert(versionFunc);
        int majorVersion = reinterpret_cast<PluginInfoFunction>(versionFunc)();
        if (majorVersion > 3) {
            // plugins compiled against API version 4 or higher are supposed to export "api_min_version".
            return { false, "ERROR: \"api_min_version\" not found: " + std::string(utf8Filename) + "\n" };
        } else {
            int version = SC_MAKE_API_VERSION(majorVersion, 0);
            return { false, ErrorMessage::majorApiVersionMismatch(utf8Filename, version) };
        }
    }

    int version = reinterpret_cast<PluginInfoFunction>(versionFunc)();
    if (SC_GET_MAJOR_VERSION(version) != SC_API_VERSION_MAJOR) {
        // API version mismatch
        return { false, ErrorMessage::majorApiVersionMismatch(utf8Filename, version) };
    }

    int minVersion = reinterpret_cast<PluginInfoFunction>(minVersionFunc)();
    if (minVersion > SC_API_VERSION) {
        // min. required version not met
        return { false, ErrorMessage::minApiVersionNotMet(utf8Filename, minVersion) };
    }

    return { true, "" }; // success
}

} // namespace sc
