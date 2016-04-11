#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include "json/json.h"

#define CURRENT_KNOWN

typedef std::map<std::string, std::map<std::string, double>*> rule_set;
typedef std::map<std::string, double> rules;
typedef std::pair<std::string, rules*> ruleset_pair;
typedef std::pair<std::string, double> theshold_pair;

rule_set ruleset_server;
rule_set ruleset_application;

rules* parse_rules(Json::Value root) {
	rules* thresholds = new rules();	
	for (Json::Value::iterator it = root.begin(); it != root.end(); it++) {
		std::string threshold_name = it.key().asString();
		double threshold_value = (*it).asDouble();
		thresholds->insert(theshold_pair(threshold_name, threshold_value));
	}
	return thresholds;
}

void construct_rule_set(Json::Value root, rule_set* rs) {
	rs->clear();	
	for (Json::Value::iterator it = root.begin(); it != root.end(); it++) {
		std::string item_name = it.name();
		if ((*it).size() > 0
			&& rs->find(item_name) == rs->end()){
			rules* item_rules = parse_rules((*it));
			rs->insert(ruleset_pair(item_name, item_rules));
		}
	}
}

void construct_rule_sets_from_tree(Json::Value root) {	
	if (!root["server"].isNull()) {		
		construct_rule_set(root["server"], &ruleset_server);
	}	
	if (!root["application"].isNull()) {
		construct_rule_set(root["application"], &ruleset_application);
	}
}

void read_json_tree_from_file(std::string fname, Json::Value* root){
	Json::Reader reader;	
	std::ifstream is;

	is.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		is.open(fname, std::ios::in);
		if (is.is_open()) {
			reader.parse(is, (*root));
		}
		is.close();
	}
	catch(std::ios_base::failure e){
		std::cerr << "Exceptions on opening/reading file:" << fname << std::endl;
	}
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

int main(int argc, char *argv[])
{
	Json::Value root;

	read_json_tree_from_file("tmp_SLA.json", &root);
	
	construct_rule_sets_from_tree(root);

	//while (1) {
		//	parser(argv[1]);
		//	print_parser();
	//};

	return 0;
}
