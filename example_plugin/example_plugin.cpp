/*
 * Example plugin for ZLMediaKit WebApi extension
 * 
 * This plugin demonstrates how to register custom API handlers
 * using the plugin system.
 */

#include "WebApi.h"
#include "Util/logger.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

extern "C" {

// This is the entry point that will be called when the plugin is loaded
void zlm_plugin_init() {
    InfoL << "Example plugin initializing...";
    
    // Register a simple test API
    api_regist("/plugin/example/hello", [](API_ARGS_MAP) {
        val["code"] = 0;
        val["msg"] = "Hello from example plugin!";
        val["plugin"] = "example_plugin";
    });
    
    // Register an API that echoes back parameters
    api_regist("/plugin/example/echo", [](API_ARGS_MAP) {
        val["code"] = 0;
        val["received_params"] = Json::Value(Json::objectValue);
        for (auto &pr : allArgs.args) {
            val["received_params"][pr.first] = pr.second;
        }
    });
    
    InfoL << "Example plugin initialized successfully";
    InfoL << "  - Registered API: /plugin/example/hello";
    InfoL << "  - Registered API: /plugin/example/echo";
}

} // extern "C"
