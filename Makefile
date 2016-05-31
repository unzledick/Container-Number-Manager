.DEFAULT_GOAL:= Container_Number_Manager

INC =  -I /usr/local/include -I local_lib/include

LIB =  -L /usr/local/lib -L local_lib/lib

all: Container_Number_Manager make_SLA_json
	
Container_Number_Manager: Container_Number_Manager.cpp
	g++ -std=c++11 $(INC) $(LIB) Container_Number_Manager.cpp -o Container_Number_Manager.o local_lib/lib/linux-gcc-4.8.5/libjson_linux-gcc-4.8.5_libmt.a

json: make_application_info.cpp
	g++ -std=c++11 $(INC) $(LIB) make_application_info.cpp -o make_application_info.o local_lib/lib/linux-gcc-4.8.5/libjson_linux-gcc-4.8.5_libmt.a
	./make_application_info.o

clean:
	rm *.o *.out
