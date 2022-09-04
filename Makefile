all: make

make:
	g++ test.cpp -o test `pkg-config gtkmm-4.0 xcb xcb-cursor --cflags --libs` -pthread
