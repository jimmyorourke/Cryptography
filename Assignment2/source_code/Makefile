all:
	rm -f *.o client server time_attack
	g++ -o client client.cpp crypto/crypto.cpp crypto/encodings.cpp -lssl
	g++ -o server server.cpp crypto/crypto.cpp crypto/encodings.cpp -lssl
	g++ -o time_attack time_attack.cpp
clean:
	rm -f *.o client server time_attack
time_attack:
	rm -f *.o time_attack
	g++ -o time_attack time_attack.cpp
server:
	rm -f *.o server
	g++ -o server server.cpp crypto/crypto.cpp crypto/encodings.cpp -lssl
client:
	rm -f *.o client
	g++ -o client client.cpp crypto/crypto.cpp crypto/encodings.cpp -lssl
