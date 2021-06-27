build:
	g++ server.cpp -o server
	g++ subscriber.cpp -o subscriber

clean:
	rm -rf server subscriber
