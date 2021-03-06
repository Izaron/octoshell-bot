CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(octoshellbot)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -g")

ADD_SUBDIRECTORY(proto)

FIND_PACKAGE(Poco COMPONENTS Foundation Util Net NetSSL JSON REQUIRED)
FIND_PACKAGE(mongocxx REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(octoshellbot
    contrib/json2pb.cc

    src/client/telegram.cpp
    src/client/vkontakte.cpp
    src/client/util.cpp

    src/auth_status.cpp
    src/context.cpp
    src/handlers.cpp
    src/main.cpp
    src/mongo.cpp
    src/octoshell.cpp

    src/states/auth_new_email_state_processor.cpp
    src/states/auth_new_token_state_processor.cpp
    src/states/auth_settings_state_processor.cpp
    src/states/locale_settings_state_processor.cpp
    src/states/main_menu_state_processor.cpp
    src/states/states_holder.cpp
    src/states/ticket_cluster_choose_state_processor.cpp
    src/states/ticket_message_choose_state_processor.cpp
    src/states/ticket_project_choose_state_processor.cpp
    src/states/ticket_subject_choose_state_processor.cpp
    src/states/ticket_topic_choose_state_processor.cpp

    src/translate.cpp
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ru.properties DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/en.properties DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(octoshellbot proto ${OPENSSL_LIBRARIES} ${Poco_LIBRARIES} mongo::mongocxx_shared -lprotobuf -ljansson)
