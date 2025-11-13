FROM gcc:latest
WORKDIR /app
COPY server.c .
RUN gcc -o server server.c
EXPOSE 8080
CMD ["./server"]


