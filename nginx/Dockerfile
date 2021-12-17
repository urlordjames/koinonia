FROM alpine:latest as static-builder

WORKDIR /static

RUN apk add --no-cache npm && npm install -g parcel

COPY static .
RUN parcel build index.html && gzip dist/* -k

FROM nginx:alpine

COPY nginx.conf /etc/nginx/nginx.conf
COPY --from=static-builder /static/dist /etc/static
