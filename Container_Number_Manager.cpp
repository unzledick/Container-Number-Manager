#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include "json/json.h"

#define CURRENT_KNOWN

typedef std::map<std::string, std::map<std::string, double>*> rules;

rules rules_server;
rules rules_application;

void construct_rules(Json::Value root, rules* r) {
	std::cout << root.size() << std::endl;
	for (Json::Value::iterator it = root.begin(); it != root.end(); it++) {

		std::cout << it.name() << std::endl;
	}
}

void construct_rules_from_file(Json::Value root) {
	// [TODO] change to root["server"]
	if (!root["a"].isNull()) {		
		construct_rules(root["a"], &rules_server);		
	}
	// [TODO] change to root["application"]
	if (!root["b"].isNull()) {
		construct_rules(root["b"], &rules_application);
	}
}

void read_rules_from_file(){
	Json::Reader reader;
	Json::Value root;
	std::ifstream is;

	is.open("SLA.json", std::ios::in);
	
	if(is && reader.parse(is, root)){		
		construct_rules_from_file(root);		
	}

	is.close();
}

/*
 *   parser: parse the imformation from json file
 */
/*
void parser(char* filename){

	Json::Reader reader;
	Json::Value root;

	ifstream is;
	is.open(filename, ios::binary);

	if(reader.parse(is,root)){
		hardware.NumberOfServer = root["Hardware"]["NumberOfServer"].asInt();
		hardware.serverInfo = (ServerInfo*)malloc(hardware.NumberOfServer*sizeof(ServerInfo));

		for(int i = 0; i< hardware.NumberOfServer;i++){
			hardware.serverInfo[i].ServerID = root["Hardware"]["ServerInfo"][i]["ServerID"].asInt();            
			hardware.serverInfo[i].coreInfo.NumberOfCore = 
				root["Hardware"]["ServerInfo"][i]["CoreInfo"]["NumberOfCore"].asInt();
			hardware.serverInfo[i].coreInfo.coreContent = (CoreContent*)
				malloc(hardware.serverInfo[i].coreInfo.NumberOfCore*sizeof(CoreContent)); 

			for(int j = 0; j < hardware.serverInfo[i].coreInfo.NumberOfCore;j++){    
				hardware.serverInfo[i].coreInfo.coreContent[j].CoreID = 
					root["Hardware"]["ServerInfo"][i]["CoreInfo"]["Content"][j]["CoreID"].asInt();
				hardware.serverInfo[i].coreInfo.coreContent[j].load =
					root["Hardware"]["ServerInfo"][i]["CoreInfo"]["Content"][j]["load"].asDouble();
			}

			hardware.serverInfo[i].memInfo.SizeOfMem = 
				root["Hardware"]["ServerInfo"][i]["MemInfo"]["SizeOfMem"].asInt();
			hardware.serverInfo[i].memInfo.CurrUsage =
				root["Hardware"]["ServerInfo"][i]["MemInfo"]["CurrUsage"].asInt();    
		}

		NumberOfApplication = root["Application"].size();
		application = (Application*)malloc(NumberOfApplication*sizeof(Application));

		for(int i = 0; i< NumberOfApplication;i++){
			application[i].ApplicationID = root["Application"][i]["ApplicationID"].asInt();
			application[i].AvgResponseTime = root["Application"][i]["AvgResponseTime"].asDouble();
			application[i].SLA = root["Application"][i]["SLA"].asDouble();
			application[i].container.NumberOfContainer = root["Application"][i]["Container"]["NumberOfContainer"].asInt();
			application[i].container.containerInfo = (ContainerInfo*)malloc(application[i].container.NumberOfContainer*sizeof(ContainerInfo));

			for(int j = 0; j < application[i].container.NumberOfContainer;j++){
				application[i].container.containerInfo[j].ContainerID = root["Application"][i]["Container"]["ContainerInfo"][j]["ContainerID"].asString();
				application[i].container.containerInfo[j].OnServer = root["Application"][i]["Container"]["ContainerInfo"][j]["OnServer"].asString();
				application[i].container.containerInfo[j].CoreUsed = root["Application"][i]["Container"]["ContainerInfo"][j]["CoreUsed"].asInt();
				application[i].container.containerInfo[j].load = root["Application"][i]["Container"]["ContainerInfo"][j]["load"].asDouble();



			}     
		}

	}

}
*/
/*
 *   print imformation stored from parser to check if they are right.
 */
/*
void print_parser(){
	printf("Number of Server: %d\n",hardware.NumberOfServer);

	for(int i = 0; i< hardware.NumberOfServer;i++){
		printf("\n");
		printf("Server ID: %d\n",hardware.serverInfo[i].ServerID);
		printf("Number of core:: %d\n",hardware.serverInfo[i].coreInfo.NumberOfCore);

		for(int j = 0; j < hardware.serverInfo[i].coreInfo.NumberOfCore;j++){
			printf("*CoreID: %d\n",hardware.serverInfo[i].coreInfo.coreContent[j].CoreID);
			printf("load: %f\n",hardware.serverInfo[i].coreInfo.coreContent[j].load);
		}

		printf("Size of Mem: %d\n",hardware.serverInfo[i].memInfo.SizeOfMem);
		printf("Current Usage: %d\n",hardware.serverInfo[i].memInfo.CurrUsage);
	}

	for(int i = 0; i<NumberOfApplication;i++){
		printf("\n");
		printf("Application ID: %d\n", application[i].ApplicationID);
		printf("AvgResponseTime: %f\n", application[i].AvgResponseTime);
		printf("SLA: %f\n", application[i].SLA);
		printf("Number of container: %d\n", application[i].container.NumberOfContainer);

		for(int j = 0; j < application[i].container.NumberOfContainer;j++){
			cout <<"*Container ID: " << application[i].container.containerInfo[j].ContainerID << endl;
			cout << "On server: " <<application[i].container.containerInfo[j].OnServer << endl;
			printf("CoreUsed: %d\n", application[i].container.containerInfo[j].CoreUsed);
			printf("load: %f\n", application[i].container.containerInfo[j].load);
		}
	}    
}
*/
void init_rules() {
	rules_server.clear();
	rules_application.clear();

	read_rules_from_file();
}

int main(int argc, char *argv[])
{
	init_rules();	

	while (1) {
		//	parser(argv[1]);
		//	print_parser();
	};

	return 0;
}
