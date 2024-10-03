aux_source_directory(. SRC)
aux_source_directory(./threadPool THREADPOOL)

add_executable(threadPoolServer ${SRC} ${THREADPOOL})