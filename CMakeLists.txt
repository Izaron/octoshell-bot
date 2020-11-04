CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(octoshellbot)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -g")

ADD_SUBDIRECTORY(proto)

FIND_PACKAGE(Poco COMPONENTS Foundation Util Net NetSSL JSON REQUIRED)
FIND_PACKAGE(mongocxx REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(octoshellbot
    contrib/json2pb.cc
    src/client/telegram.cpp
    src/context.cpp
    src/handlers.cpp
    src/main.cpp
    src/mongo.cpp
    src/states/states_holder.cpp
)

TARGET_LINK_LIBRARIES(octoshellbot proto ${OPENSSL_LIBRARIES} ${Poco_LIBRARIES} mongo::mongocxx_shared -lprotobuf -ljansson)
