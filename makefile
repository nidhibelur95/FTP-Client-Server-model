all: server client
server:
  g++ server.cpp -pthread -o server.o
client:
  g++ client.cpp -o client.o
clean:
  rm server.o client.o
