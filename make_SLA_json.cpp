/*
 * make SLA to json file
 */ 
#include<json/json.h>
#include<iostream>
#include<fstream>

int main(){
    std::ofstream file_id;
    file_id.open("SLA.json");

    Json::Value SLA;
	SLA["a"] = 1;
	SLA["b"] = 2;
	SLA["c"] = 3;

    Json::StyledWriter styledwriter;
    file_id << styledwriter.write(SLA);

    file_id.close();
}    
