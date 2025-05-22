cmake_minimum_required(VERSION 3.16)

project(company C CXX)

# Proto file
get_filename_component(hw_proto "${ALL_PROJECT_GRPC_PROTOS_PATH}/company.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/company.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/company.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/company.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/company.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}"
      COMMAND ${CMAKE_COMMAND} -E copy ${hw_proto_srcs} ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.cc
      COMMAND ${CMAKE_COMMAND} -E copy ${hw_proto_hdrs} ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.pb.h
      COMMAND ${CMAKE_COMMAND} -E copy ${hw_grpc_srcs} ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.cc
      COMMAND ${CMAKE_COMMAND} -E copy ${hw_grpc_hdrs} ${ALL_PROJECT_GRPC_CPP_SOURCE}/company.grpc.pb.h
)

## hw_grpc_proto
add_library(company
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(company
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

