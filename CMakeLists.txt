cmake_minimum_required(VERSION 4.1)
project(swiftcode)

set(CMAKE_CXX_STANDARD 17)

add_executable(swiftcode
        src/main.cpp
        src/sql/sql_pipeline.h
        src/enum/enum_pipline.h
        src/enum/file_resolving.h
        src/enum/files_resolving.cpp
        src/enum/raw_block.h
        src/enum/mid_block.h
        src/enum/output_block.h
        src/sql/raw_block.h
        src/sql/output_block.h
        src/sql/mid_block.h
        src/sql/file_resolving.h
        src/sql/files_resolving.cpp
)
