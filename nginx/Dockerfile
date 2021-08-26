FROM alpine:latest as static-builder

RUN mkdir /static
WORKDIR /static

RUN apk add --no-cache npm python3 g++ make && npm install -g parcel

COPY static .
RUN parcel build index.html && cp rtc_config.txt dist && gzip dist/* -k

FROM nginx:alpine

COPY nginx.conf /etc/nginx/nginx.conf
COPY --from=static-builder /static/dist /etc/static
