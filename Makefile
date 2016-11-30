all: catti-plugin.so catti-example

catti-plugin.so: catti-plugin.cpp wg/libwg.a wg/lat-parser/liblat-parser.a
	g++ $^ -shared -o catti-plugin.so -fPIC -I. -g3

catti-example: catti-example.cpp
	g++ catti-example.cpp -I. -ldl -g3 -o catti-example

clean:
	rm catti-plugin.so catti-example
