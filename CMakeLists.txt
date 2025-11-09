cmake_minimum_required(VERSION 3.16)
project(backend LANGUAGES CXX)

# 指定 Qt 版本
set(QT_MIN_VERSION "6.4.0")

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 组件
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Network
    Sql
    WebSockets
    HttpServer
)

# 查找所有源文件
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.h
)


# 生成可执行文件 backend
add_executable(backend ${SOURCES})

# 链接 Qt 模块
target_link_libraries(backend PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::WebSockets
    Qt6::HttpServer
)

# 启用自动 MOC / UIC / RCC（方便使用 Qt 信号槽）
set_target_properties(backend PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 可选：设置输出路径
set_target_properties(backend PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
