# базовый образ
FROM ubuntu:18.04 as build

# установка необходимых пакетов для компиляции приложения
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    libjansson-dev \
    protobuf-compiler \
    libprotobuf-dev \
    libpoco-dev \
    wget \
    curl \
    git

# устанавливаем библиотеки для работы с MongoDB
# сначала "libmongoc"
RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.2/mongo-c-driver-1.17.2.tar.gz -O /mongoc.tar.gz
RUN tar xzf /mongoc.tar.gz
WORKDIR /mongo-c-driver-1.17.2
RUN mkdir cmake-build && \
    cd cmake-build && \
    cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. && \
    cmake --build . && \
    cmake --build . --target install

# потом "mongocxx"
WORKDIR /
RUN curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.1/mongo-cxx-driver-r3.6.1.tar.gz
RUN tar -xzf mongo-cxx-driver-r3.6.1.tar.gz
WORKDIR /mongo-cxx-driver-r3.6.1/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
RUN cmake --build . && cmake --build . --target install

# построение исходников приложения
COPY ./src/resources/app.properties /app.properties
COPY ./src /app/src
WORKDIR /app/bin
RUN cmake ../src && cmake --build .

# запуск бота
ENTRYPOINT ["./octoshellbot", "/app.properties"]
