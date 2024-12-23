
#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
#set(SRC_LIST testserver.cpp)

#设置可执行文件的最终存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#把指定路径下的所有源文件名字放入变量名SRC_LIST里面
aux_source_directory(. SRC_LIST)

add_executable(server ${SRC_LIST})

#表示生成server这个目标程序，需要连接lmymuduo lpthread这三个库文件
target_link_libraries(server mymuduo pthread)
