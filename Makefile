all: make

make:
	g++ test.cpp -o test `pkg-config gtkmm-4.0 x11 fontconfig xft --cflags --libs` ; pkill test
