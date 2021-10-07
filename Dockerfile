FROM alpine:latest

RUN mkdir -p /compile/build
WORKDIR /compile

# alpine bug: lua versions other than 5.1 cannot be discovered by CMake
RUN apk add --no-cache gcc g++ cmake ninja jsoncpp-dev zlib-dev git util-linux-dev #lua-dev

COPY CMakeLists.txt .
COPY src src

WORKDIR /compile/build

RUN cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release && cmake --build . -j $(nproc)

FROM alpine:latest

RUN apk add --no-cache jsoncpp libuuid #lua
RUN adduser koinonia -H -D && mkdir /koinonia && chown koinonia:koinonia /koinonia

USER koinonia
WORKDIR /koinonia

COPY --chown=koinonia:koinonia --from=0 /compile/build/koinonia /koinonia/koinonia
COPY --chown=koinonia:koinonia nginx/static /koinonia/static

EXPOSE 8080

CMD ["/koinonia/koinonia"]
