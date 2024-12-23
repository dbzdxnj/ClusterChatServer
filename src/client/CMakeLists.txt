# 定义了一个SRC_LIST变量，包含了该目录下所有源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(chatClient ${SRC_LIST})

# 指定可执行文件需要依赖的库文件
target_link_libraries(chatClient pthread)