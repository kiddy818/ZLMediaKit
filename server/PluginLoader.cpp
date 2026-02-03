/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include "PluginLoader.h"
#include "Util/File.h"
#include "Util/logger.h"

#include <vector>
#include <string>

#if !defined(_WIN32)
#include <dlfcn.h>
#else
#include <windows.h>
#endif

using namespace std;
using namespace toolkit;

namespace mediakit {

// Store loaded plugin handles for cleanup
static vector<void*> s_plugin_handles;

void loadPlugins() {
    // Get plugin directory path (./plugin relative to current directory)
    string plugin_dir = "./plugin";
    
    if (!File::is_dir(plugin_dir.c_str())) {
        InfoL << "Plugin directory not found: " << plugin_dir << ", skipping plugin loading";
        return;
    }
    
    InfoL << "Scanning plugin directory: " << plugin_dir;
    
    // Scan the plugin directory for .so files
    File::scanDir(plugin_dir, [](const string &path, bool isDir) {
        if (isDir) {
            return true; // Continue scanning
        }
        
#if !defined(_WIN32)
        // Unix/Linux: Load .so files
        if (!end_with(path, ".so")) {
            return true; // Skip non-.so files
        }
        
        InfoL << "Loading plugin: " << path;
        
        // Load the shared library
        // Use RTLD_NOW to catch symbol resolution errors early
        void* handle = dlopen(path.data(), RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            WarnL << "Failed to load plugin " << path << ": " << dlerror();
            return true;
        }
        
        // Look for the plugin initialization function
        typedef void (*plugin_init_func)();
        plugin_init_func init_func = (plugin_init_func)dlsym(handle, "zlm_plugin_init");
        
        if (!init_func) {
            WarnL << "Plugin " << path << " does not export zlm_plugin_init function: " << dlerror();
            dlclose(handle);
            return true;
        }
        
        // Call the plugin initialization function
        try {
            init_func();
            InfoL << "Successfully loaded and initialized plugin: " << path;
            s_plugin_handles.push_back(handle);
        } catch (const std::exception &e) {
            ErrorL << "Plugin " << path << " initialization failed: " << e.what();
            dlclose(handle);
        }
#else
        // Windows: Load .dll files
        if (!end_with(path, ".dll")) {
            return true; // Skip non-.dll files
        }
        
        InfoL << "Loading plugin: " << path;
        
        // Load the dynamic library
        HMODULE handle = LoadLibraryA(path.data());
        if (!handle) {
            WarnL << "Failed to load plugin " << path << ": Error code " << GetLastError();
            return true;
        }
        
        // Look for the plugin initialization function
        typedef void (*plugin_init_func)();
        plugin_init_func init_func = (plugin_init_func)GetProcAddress(handle, "zlm_plugin_init");
        
        if (!init_func) {
            WarnL << "Plugin " << path << " does not export zlm_plugin_init function: Error code " << GetLastError();
            FreeLibrary(handle);
            return true;
        }
        
        // Call the plugin initialization function
        try {
            init_func();
            InfoL << "Successfully loaded and initialized plugin: " << path;
            s_plugin_handles.push_back(handle);
        } catch (const std::exception &e) {
            ErrorL << "Plugin " << path << " initialization failed: " << e.what();
            FreeLibrary(handle);
        }
#endif
        
        return true; // Continue scanning
    }, false); // Don't scan subdirectories
    
    InfoL << "Plugin loading complete. Loaded " << s_plugin_handles.size() << " plugin(s)";
}

void unloadPlugins() {
    InfoL << "Unloading " << s_plugin_handles.size() << " plugin(s)";
    
    for (auto handle : s_plugin_handles) {
#if !defined(_WIN32)
        dlclose(handle);
#else
        FreeLibrary((HMODULE)handle);
#endif
    }
    
    s_plugin_handles.clear();
}

} // namespace mediakit
