.DEFAULT_GOAL:= Container_Number_Manager

INC = -I /opt/local/include/ -I /home/ricktsai/rick_lib/include

LIB = -L /opt/local/lib -L /home/ricktsai/rick_lib/lib

all: Container_Number_Manager make_SLA_json
	
Container_Number_Manager: Container_Number_Manager.cpp
	g++ -std=c++11 $(INC) $(LIB) Container_Number_Manager.cpp -o Container_Number_Manager.o /home/ricktsai/rick_lib/lib/linux-gcc-4.8.5/libjson_linux-gcc-4.8.5_libmt.a -lcurl

make_SLA_json: make_SLA_json.cpp
	g++ -std=c++11 $(INC) $(LIB) make_SLA_json.cpp -o make_SLA_json.o /home/ricktsai/rick_lib/lib/linux-gcc-4.8.5/libjson_linux-gcc-4.8.5_libmt.a -lcurl

clean:
	rm *.o *.out
