.\tools\protobuf\protoc.exe --grpc_out=./generatedCode/ --plugin=protoc-gen-grpc=.\tools\grpc\grpc_cpp_plugin.exe .\Master.proto
.\tools\protobuf\protoc.exe --cpp_out=./generatedCode/ ./Master.proto