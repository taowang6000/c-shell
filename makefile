c_shell.x: c_shell.o 
	g++ -std=c++11 -o c_shell.x c_shell.o
c_shell.o: c_shell.cpp
	g++ -std=c++11 -c c_shell.cpp
myls:   myls.o
	g++ -std=c++11 -o myls myls.o
myls.o: myls.cpp
	g++ -std=c++11 -c myls.cpp

ttt
