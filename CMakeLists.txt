CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(echobot)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -g")

ADD_SUBDIRECTORY(proto)

FIND_PACKAGE(Poco COMPONENTS Foundation Util Net NetSSL JSON REQUIRED)
FIND_PACKAGE(mongocxx REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(echobot src/main.cpp src/context.cpp src/handlers.cpp src/mongo.cpp src/client/telegram.cpp contrib/json2pb.cc)

TARGET_LINK_LIBRARIES(echobot proto ${OPENSSL_LIBRARIES} ${Poco_LIBRARIES} mongo::mongocxx_shared -lprotobuf -ljansson)
