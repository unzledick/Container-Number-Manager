#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include "json/json.h"

#ifdef __unix__
#include <unistd.h>
#elif _WIN32 || _WIN64
#define WINDOWS
#include <windows.h>
#endif // __unix__

#define	FILE_RULES	"SLA.json"
#define FILE_SERVER	"serverType.json"
#define	FILE_MDATA	"example.json"

#define STR_SERVER	"Server"
#define STR_APPLICATION	"Application"

#define T_SLEEP_MS	500

typedef std::map<std::string, std::map<std::string, double>*> rule_set;
typedef std::map<std::string, double> rules;
typedef std::pair<std::string, rules*> ruleset_pair;
typedef std::pair<std::string, double> theshold_pair;

rule_set ruleset_server;
rule_set ruleset_application;

std::map<std::string, std::string> type_server;

bool monitorDataUpdate() {
	// [TODO] check if there are monitor data
	return true;
}

std::string query_server_type(std::string server_id) {
	std::string server_type = "";
	if (type_server.find(server_id) != type_server.end()){
		server_type = type_server[server_id];
	}
	return server_type;
}

void build_server_type_mapping(Json::Value root) {
	std::string type;
	std::string name;

	type_server.clear();

	for (Json::Value::iterator it = root.begin(); it != root.end(); it++) {
		type = it.key().asString();
		for (Json::Value::iterator it2 = it->begin(); it2 != it->end(); it2++) {
			name = it2->asString();
			type_server.insert(std::pair<std::string, std::string>(name, type));
		}
	}
}

bool check_cpu(Json::Value root, rules* r) {
	bool result = true;

	if (root["Load"].isNull() || root["NumberOfCore"].isNull()) {
		std::cerr << "\tCPU load information missing" << std::endl;
		result = false;
	}
	else{
		rules::iterator it;
		double avgLoad = root["Load"].asDouble() / root["NumberOfCore"].asDouble();
		
		if ((it = r->find("CPU load")) != r->end()
			&& (avgLoad > it->second)) {
			std::cerr << "\tCPU load exceeds threshold" << std::endl;
			result = false;
		}
	}
	return result;
}
bool check_mem(Json::Value root, rules* r) {
	bool result = true;

	if (root["SizeOfMem"].isNull() || root["CurrUsage"].isNull()) {
		std::cerr << "\tMemory information missing" << std::endl;
		result = false;
	}
	else {
		rules::iterator it;
		double percentage = root["CurrUsage"].asDouble() / root["SizeOfMem"].asDouble();

		if ((it = r->find("Memory")) != r->end()
			&& (percentage > it->second)) {
			std::cerr << "\tMemory exceeds threshold" << std::endl;
			result = false;
		}
	}
	return result;
}
bool check_disk(Json::Value root, rules* r) {
	bool result = true;
	// [TODO]
	return result;
}
bool chech_network(Json::Value root, rules* r) {
	bool result = true;
	// [TODO]
	return result;
}

void parse_server(Json::Value root) {
	std::string server_id = root["ServerID"].asString();
	std::string server_type = query_server_type(server_id);
	rules* r = ruleset_server[server_type];
	bool normal = true;

	std::cout << "Checking server " << server_id << "..." << std::endl;

	normal &= check_cpu(root["CoreInfo"], r);
	normal &= check_mem(root["MemInfo"], r);
	normal &= check_disk(root["DiskInfo"], r);
	normal &= chech_network(root["NetworkInfo"], r);

	if (normal) {
		std::cout << "Normal" << std::endl;
	}
}

void analyze_data_server(Json::Value root) {
	//int server_amount = root["NumberOfServer"].asInt();
	Json::Value servers = root["ServerInfo"];
	for (int i = 0; i < servers.size(); i++) {
		parse_server(servers[i]);
	}
}

void analyze_data(Json::Value root) {
	if (!root[STR_SERVER].isNull()) {
		analyze_data_server(root[STR_SERVER]);
	}
	if (!root[STR_APPLICATION].isNull()) {
		// [TODO]
		//analyze_data_applicaiton(root["Application"]);
	}
}

void go_to_sleep() {
#ifdef WINDOWS
	Sleep(T_SLEEP_MS);
#else
	usleep(T_SLEEP_MS);
#endif
}

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
	rules* default_rules = new rules();

	rs->clear();
	rs->insert(ruleset_pair("", default_rules));

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
	if (!root[STR_SERVER].isNull()) {
		construct_rule_set(root[STR_SERVER], &ruleset_server);
	}	
	if (!root[STR_APPLICATION].isNull()) {
		construct_rule_set(root[STR_APPLICATION], &ruleset_application);
	}
}

bool read_json_tree_from_file(std::string fname, Json::Value* root){
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
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	Json::Value root;
	bool read_success = false;

	read_success = read_json_tree_from_file(FILE_RULES, &root);
	if (read_success) {
		construct_rule_sets_from_tree(root);
	}

	read_success = read_json_tree_from_file(FILE_SERVER, &root);
	build_server_type_mapping(root);


	// [TODO] fork a child as daemon to do the following

	while (1) {
		if (!monitorDataUpdate()) {
			go_to_sleep();
			continue;
		}

		read_success = read_json_tree_from_file(FILE_MDATA, &root);
		if (read_success) {
			analyze_data(root);
		}		
	};

	return 0;
}
