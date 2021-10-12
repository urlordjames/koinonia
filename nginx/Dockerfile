FROM alpine:latest as static-builder

WORKDIR /static

RUN apk add --no-cache npm && npm install -g webpack-cli

COPY static static
RUN cp static dist -r &&  webpack build ./static/koinonia.js && mv dist/main.js dist/koinonia.js && cp dist/sw.js dist/service-worker.js && gzip dist/* -k

FROM nginx:alpine

COPY nginx.conf /etc/nginx/nginx.conf
COPY --from=static-builder /static/dist /etc/static
