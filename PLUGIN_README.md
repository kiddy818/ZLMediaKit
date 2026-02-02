# ZLMediaKit Plugin System

## 概述 (Overview)

ZLMediaKit现在支持WebApi插件扩展系统。在启动时，程序会自动检索当前目录下的`plugin`子目录，并加载其中的所有动态库文件（`.so`文件在Linux上，`.dll`文件在Windows上）。

ZLMediaKit now supports a WebApi plugin extension system. At startup, the program automatically scans the `plugin` subdirectory in the current directory and loads all dynamic library files (`.so` files on Linux, `.dll` files on Windows).

## 如何工作 (How It Works)

1. **启动时加载**: 当MediaServer启动时，在`installWebApi()`之后，系统会调用`loadPlugins()`
2. **扫描插件目录**: 系统扫描`./plugin`目录下的所有动态库文件
3. **加载并初始化**: 每个插件被加载后，系统会调用其导出的`zlm_plugin_init()`函数
4. **注册API**: 在初始化函数中，插件可以调用`api_regist()`来注册自定义的API路径和处理函数
5. **关闭时卸载**: 当MediaServer关闭时，所有插件会被正确卸载

1. **Load at startup**: When MediaServer starts, after `installWebApi()`, the system calls `loadPlugins()`
2. **Scan plugin directory**: The system scans all dynamic library files in the `./plugin` directory
3. **Load and initialize**: After each plugin is loaded, the system calls its exported `zlm_plugin_init()` function
4. **Register APIs**: In the initialization function, plugins can call `api_regist()` to register custom API paths and handlers
5. **Unload at shutdown**: When MediaServer shuts down, all plugins are properly unloaded

## 创建插件 (Creating a Plugin)

### 最小插件示例 (Minimal Plugin Example)

```cpp
#include "WebApi.h"
#include "Util/logger.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

extern "C" {

// 插件初始化入口点
// Plugin initialization entry point
void zlm_plugin_init() {
    InfoL << "My plugin initializing...";
    
    // 注册一个简单的API
    // Register a simple API
    api_regist("/plugin/myapi/hello", [](API_ARGS_MAP) {
        val["code"] = 0;
        val["msg"] = "Hello from my plugin!";
    });
    
    InfoL << "My plugin initialized";
}

} // extern "C"
```

### 编译插件 (Compiling the Plugin)

#### CMakeLists.txt示例

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_plugin)

set(CMAKE_CXX_STANDARD 11)

# MediaServer源码目录
set(MEDIAKIT_ROOT "/path/to/ZLMediaKit")

# 包含目录
include_directories(
    ${MEDIAKIT_ROOT}/server
    ${MEDIAKIT_ROOT}/src
    ${MEDIAKIT_ROOT}/3rdpart
    ${MEDIAKIT_ROOT}/3rdpart/jsoncpp/include
    ${MEDIAKIT_ROOT}/3rdpart/ZLToolKit/src
)

# 创建共享库
add_library(my_plugin SHARED my_plugin.cpp)

# 去掉lib前缀
set_target_properties(my_plugin PROPERTIES PREFIX "")

# Linux: 允许未定义的符号（运行时从主程序解析）
if(UNIX AND NOT APPLE)
    set_target_properties(my_plugin PROPERTIES 
        LINK_FLAGS "-Wl,--unresolved-symbols=ignore-all")
endif()
```

#### 编译命令

```bash
cd my_plugin
mkdir build && cd build
cmake ..
make
```

### 部署插件 (Deploying the Plugin)

```bash
# 在MediaServer可执行文件目录下创建plugin目录
mkdir -p plugin

# 复制编译好的插件
cp my_plugin.so plugin/

# 启动MediaServer，插件会自动加载
./MediaServer
```

## API注册类型 (API Registration Types)

插件可以注册多种类型的API处理器：

Plugins can register various types of API handlers:

### 1. MAP类型参数 (MAP Type Parameters)

```cpp
// 同步API
api_regist("/plugin/api/sync", [](API_ARGS_MAP) {
    CHECK_ARGS("param1");  // 检查必需参数
    val["result"] = allArgs["param1"];
});

// 异步API
api_regist("/plugin/api/async", [](API_ARGS_MAP_ASYNC) {
    // 异步处理...
    invoker(200, headerOut, val.toStyledString());
});
```

### 2. JSON类型参数 (JSON Type Parameters)

```cpp
api_regist("/plugin/api/json", [](API_ARGS_JSON) {
    auto param = allArgs.args["key"].asString();
    val["received"] = param;
});
```

### 3. 字符串类型参数 (String Type Parameters)

```cpp
api_regist("/plugin/api/string", [](API_ARGS_STRING) {
    // 处理原始HTTP请求内容
    val["content_length"] = allArgs.args.length();
});
```

## 示例插件 (Example Plugin)

本项目包含一个完整的示例插件在 `example_plugin/` 目录：

This project includes a complete example plugin in the `example_plugin/` directory:

- **example_plugin.cpp**: 插件源代码 (Plugin source code)
- **CMakeLists.txt**: 构建配置 (Build configuration)

示例插件注册了两个API：

The example plugin registers two APIs:

1. `/plugin/example/hello` - 返回问候消息 (Returns a greeting message)
2. `/plugin/example/echo` - 回显接收到的参数 (Echoes received parameters)

### 测试示例插件 (Testing the Example Plugin)

```bash
# 启动MediaServer
cd release/linux/Debug
./MediaServer

# 测试hello API
curl "http://localhost:8080/plugin/example/hello"
# 输出: {"code":0,"msg":"Hello from example plugin!","plugin":"example_plugin"}

# 测试echo API
curl "http://localhost:8080/plugin/example/echo?name=test&value=123"
# 输出: {"code":0,"received_params":{"name":"test","value":"123"}}
```

## 技术细节 (Technical Details)

### 符号解析 (Symbol Resolution)

- 插件在运行时从主程序（MediaServer）解析符号
- MediaServer使用`-rdynamic`标志编译，导出其符号
- 插件编译时允许未定义符号

- Plugins resolve symbols from the main program (MediaServer) at runtime
- MediaServer is compiled with the `-rdynamic` flag to export its symbols
- Plugins are compiled allowing undefined symbols

### 日志记录 (Logging)

插件可以使用ZLMediaKit的日志系统：

Plugins can use ZLMediaKit's logging system:

```cpp
InfoL << "Information message";
WarnL << "Warning message";
ErrorL << "Error message";
```

### 线程安全 (Thread Safety)

所有API处理器都在MediaServer的事件循环中执行，确保线程安全。

All API handlers execute in MediaServer's event loop, ensuring thread safety.

## 故障排除 (Troubleshooting)

### 插件未加载 (Plugin Not Loading)

1. 检查插件目录是否存在: `./plugin`
2. 检查插件文件权限: `chmod +x plugin/my_plugin.so`
3. 查看MediaServer日志中的插件加载消息

1. Check if plugin directory exists: `./plugin`
2. Check plugin file permissions: `chmod +x plugin/my_plugin.so`
3. Look for plugin loading messages in MediaServer logs

### 符号未找到错误 (Symbol Not Found Errors)

- 确保包含了正确的头文件
- 确保MediaServer是用`-rdynamic`编译的
- 检查插件是否正确链接

- Ensure correct headers are included
- Ensure MediaServer is compiled with `-rdynamic`
- Check plugin linking

### API未响应 (API Not Responding)

- 检查API路径是否正确注册
- 确保`zlm_plugin_init()`函数被正确导出
- 查看服务器日志了解初始化错误

- Check if API path is correctly registered
- Ensure `zlm_plugin_init()` function is properly exported
- Check server logs for initialization errors

## 安全考虑 (Security Considerations)

- 只加载受信任的插件
- 插件与主程序共享相同的地址空间
- 插件错误可能导致整个服务器崩溃
- 建议在生产环境中充分测试插件

- Only load trusted plugins
- Plugins share the same address space as the main program
- Plugin errors can crash the entire server
- Testing plugins thoroughly in production environments is recommended
