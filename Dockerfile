FROM alpine:latest as builder

RUN apk add --no-cache gcc g++ cmake ninja jsoncpp-dev zlib-dev git util-linux-dev brotli-dev
RUN adduser koinonia -H -D && mkdir /compile && chown koinonia:koinonia /compile

USER koinonia
WORKDIR /compile

RUN git clone https://github.com/an-tao/drogon.git --recursive
COPY CMakeLists.txt .
COPY src src

RUN mkdir build
WORKDIR /compile/build

RUN cmake --configure .. -GNinja -DCMAKE_BUILD_TYPE=Release && cmake --build . -j 12

FROM alpine:latest

RUN apk add --no-cache jsoncpp util-linux brotli
RUN adduser koinonia -H -D && mkdir /koinonia && chown koinonia:koinonia /koinonia

USER koinonia
WORKDIR /koinonia

EXPOSE 8080

COPY --chown=koinonia:koinonia --from=builder /compile/build/koinonia /koinonia/koinonia

CMD ["/koinonia/koinonia"]
