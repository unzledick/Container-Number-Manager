Container_Number_Manager: Container_Number_Manager.cpp
	g++ -std=c++11 -I/opt/local/include/ -L/opt/local/lib Container_Number_Manager.cpp -o Container_Number_Manager.o -ljsoncpp

make_SLA_json: make_SLA_json.cpp
	g++ -std=c++11 -I/opt/local/include/ -L/opt/local/lib make_SLA_json.cpp -o make_SLA_json.o -ljsoncpp

