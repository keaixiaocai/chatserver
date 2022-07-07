# 配置库文件搜索路径
# link_directories()

# 设置需要编译的源文件列表
set(SRC_LIST muduo_server.cpp)

# 把当前路径下的所有源文件名字放入变量名SRC_LIST里面
# aux_source_directory(. SRC_LIST)

# 表示生成可执行文件server，由SRC_LIST变量所定义的源文件
# add_executable(server ${SRC_LIST})

# 表示server这个目标程序，需要连接muduo_net muduo_base pthread这三个库文件
# target_link_libraries(server muduo_net muduo_base pthread)