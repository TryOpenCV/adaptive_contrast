default:
	g++ contrast.cpp -o contrast `pkg-config opencv --cflags --libs`
clean:
	rm contrast
