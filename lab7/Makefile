app:
	g++ --std=c++17 -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL libraylib.a main.cpp -o build/app

test:
	gcc -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL libraylib.a test.c -o build/app

run:
	./build/app

clear:
	rm ./build/*
