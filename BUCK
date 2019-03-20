load('//:buckaroo_macros.bzl', 'buckaroo_deps')

cxx_library(
  name = 'libget',
  header_namespace = 'libget',
  headers = [ './src/constants.h' ],
  exported_headers = [
    './src/Get.hpp',
    './src/Package.hpp',
    './src/Utils.hpp',
    './src/Repo.hpp',
    './src/constants.h',
  ],
  srcs = glob(['./src/*.cpp']),
  licenses = [ 'LICENSE' ],
  deps = buckaroo_deps(),
)

cxx_binary(
  name = 'get',
  srcs = [
    'cli/main.cpp',
  ],
  deps = [
    ':libget',
  ],
)
