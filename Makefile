all: make

make:
	g++ test.cpp -o test `pkg-config --cflags --libs gtkmm-4.0 gtk4` -lX11 -pthread
