add_library(
  stl
  stl/stl_io.cc
  stl/importer/stl_importer_binary.cc
  stl/importer/stl_importer_ascii.cc
  stl/exporter/stl_exporter_binary.cc
  stl/stl_io.hh
  stl/importer/stl_importer_binary.hh
  stl/importer/stl_importer_ascii.hh
  stl/exporter/stl_exporter_binary.hh
  string_buffer.hh
)
target_include_directories(
  stl
  PUBLIC stl
  PRIVATE stl/importer stl/exporter
)
target_compile_features(stl PUBLIC cxx_std_17)