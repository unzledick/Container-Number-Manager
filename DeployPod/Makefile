INC = -I /opt/local/include/ -I /home/b01501085/write-nodeselector-pod/jsoncpp/include
LIB = -L /opt/local/lib  -L /home/b01501085/write-nodeselector-pod/jsoncpp/libs

all: main

main: Deploy.cpp main.cpp
	g++ -std=c++11 $(INC) $(LIB) Deploy.cpp main.cpp -o main /home/b01501085/write-nodeselector-pod/jsoncpp/libs/linux-gcc-4.8.5/libjson_linux-gcc-4.8.5_libmt.a

clean:
	rm -rf *.o *.out 
