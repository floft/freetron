find_path(CPPDB_INCLUDE_DIR cppdb/conn_manager.h PATHS /usr/include /usr/local/include)
find_library(CPPDB_LIBRARY NAMES cppdb PATHS /usr/lib64 /usr/lib /usr/local/lib)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPPDB  DEFAULT_MSG  CPPDB_LIBRARY  CPPDB_INCLUDE_DIR)
