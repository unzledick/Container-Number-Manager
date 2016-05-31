/*
 *  * make SLA to json file
 */ 
#include"json/json.h"
#include<iostream>
#include<fstream>

int main(){
    std::ofstream file_id;
    file_id.open("JsonInput/applicationInfo.json");

    Json::Value applicationInfo;
    Json::Value application;
    application["applicationID"] = "ticket-test";
    application["replicationController"] = "ticket-test";
    application["nodeSelector"] = "name=ticket-test";
    applicationInfo.append(application);
    application["applicationID"] = "heapster";
    application["replicationController"] = "heapster";
    application["nodeSelector"] = "name=heapster";
    applicationInfo.append(application);

    Json::StyledWriter styledwriter;
    file_id << styledwriter.write(applicationInfo);

    file_id.close();
}  
