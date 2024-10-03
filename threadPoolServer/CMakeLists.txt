aux_source_directory(. SRC)
aux_source_directory(./threadPool THREADPOOL)

include_directories(./threadPool)

add_executable(threadPoolServer ${SRC} ${THREADPOOL})