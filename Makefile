.DEFAULT_GOAL:= Container_Number_Manager

INC =  -I /usr/local/include -I local_lib/include

LIB =  -L /usr/local/lib -L local_lib/lib

all: Container_Number_Manager make_SLA_json
	
Container_Number_Manager: Container_Number_Manager.cpp
	g++ -std=c++11 $(INC) $(LIB) Container_Number_Manager.cpp -o Container_Number_Manager.o local_lib/lib/libjson_linux-gcc-4.2.1_libmt.a -lcurl

make_json: make_json.cpp
	g++ -std=c++11 $(INC) $(LIB) make_json.cpp -o make_json.o local_lib/lib/libjson_linux-gcc-4.2.1_libmt.a -lcurl

clean:
	rm *.o *.out
