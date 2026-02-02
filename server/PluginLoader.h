/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_PLUGINLOADER_H
#define ZLMEDIAKIT_PLUGINLOADER_H

#include <string>
#include <vector>

// Plugin interface for registering custom API handlers
// Plugins should export a function with this signature:
// extern "C" void zlm_plugin_init();
// 
// Inside zlm_plugin_init(), plugins can call api_regist() to register their custom API paths

namespace mediakit {

/**
 * Load all plugins from the plugin directory
 * Scans the "plugin" subdirectory in the current working directory
 * and loads all .so files found
 */
void loadPlugins();

/**
 * Unload all loaded plugins
 * Cleans up all dynamically loaded plugin libraries
 */
void unloadPlugins();

} // namespace mediakit

#endif // ZLMEDIAKIT_PLUGINLOADER_H
