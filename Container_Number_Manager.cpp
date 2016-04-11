#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include "json/json.h"

//#define CURRENT_KNOWN

typedef std::map<std::string, std::map<std::string, double>*> rule_set;
typedef std::map<std::string, double> rules;
typedef std::pair<std::string, rules*> ruleset_pair;
typedef std::pair<std::string, double> theshold_pair;

rule_set ruleset_server;
rule_set ruleset_application;

bool monitorDataUpdate() {
	// [TODO] check if there are monitor data
	return true;
}

std::string query_server_type(std::string server_id) {
	// [TODO] return the type of a server
	return "type_1";
}
bool check_cpu(Json::Value root, rules* r) {
	bool result = true;
	rules::iterator it;
	if (!root["Load"].isNull() 
		&& ((it = r->find("CPU load")) != r->end())
		&& (root["Load"].asDouble() > it->second)) {
		std::cerr << "\tCPU overload" << std::endl;
		result = false;
	}
	return result;
}
bool check_mem(Json::Value root, rules* r) {
	bool result = true;
	// [TODO]
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
	if (!root["Hardware"].isNull()) {
		analyze_data_server(root["Hardware"]);
	}
	if (!root["Application"].isNull()) {
		// [TODO]
		//analyze_data_applicaiton(root["Application"]);
	}
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

int main(int argc, char *argv[])
{
	Json::Value root;

	read_json_tree_from_file("tmp_SLA.json", &root);
	construct_rule_sets_from_tree(root);

	while (1) {		
		if (!monitorDataUpdate()) {
			continue;
		}

		read_json_tree_from_file("example.json", &root);
		analyze_data(root);
		
	};

	return 0;
}
