all: make

make:
	g++ main.cpp WindowManager.cpp -o test `pkg-config --cflags --libs gtkmm-4.0 gtk4` -lX11 -pthread
