#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include <getopt.h>
#include "json/json.h"

#ifdef __unix__
#include <unistd.h>
#elif _WIN32 || _WIN64
#define WINDOWS
#include <windows.h>
#endif // __unix__

#define ForEachElementIn(x) for(Json::Value::iterator element = x.begin(); element != x.end(); element++)

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
std::map<std::string, int> application_pod_number;

bool monitorDataUpdate() {
	// [TODO] check if there are monitor data
	return true;
}
template<class T>
bool map_exist(std::string target, std::map<std::string, T> m) {
	bool result = false;
	if (m.find(target) != m.end()) {
		result = true;
	}
	return result;
}

void go_to_sleep() {
#ifdef WINDOWS
	Sleep(T_SLEEP_MS);
#else
	usleep(T_SLEEP_MS);
#endif
}

bool check_pod_item(Json::Value root, std::string item_name, rules* r) {	
	double usage = root.asDouble();
	bool exceed = false;	

	if (map_exist(item_name, (*r))) {
		double threshold = (*r)[item_name];
		if (usage > threshold) {
			std::cerr << "\t" << item_name << " exceeds threshold" << std::endl;
			exceed = true;
		}
	}
	return exceed;
}
bool check_application_pod(Json::Value pod, rules* r) {
	std::string pod_id = pod["PodID"].asString();
	bool exceed = false;

	std::cout << "\tpod " << pod_id << "..." << std::endl;

	ForEachElementIn(pod["Contents"]){	
		exceed |= check_pod_item(*element, element.name(), r);
	}

	if (!exceed) {
		std::cout << "\tNormal" << std::endl;
	}
	return exceed;
}
void parse_application(Json::Value application) {
	std::string application_id = application["ApplicationID"].asString();
	int num_of_pod = application["NumberOfPod"].asInt();

	std::cout << "Checking application: " << application_id << "..." << std::endl;

	if (!map_exist(application_id, ruleset_application)) {
		std::cout << "No rules for this application, pass." << std::endl;
	}
	else{
		rules* r = ruleset_application[application_id];

		if (!application["pod"].isNull()
			&& !application["pod"]["PodInfo"].isNull()) {
			Json::Value pods = application["pod"]["PodInfo"];

			bool exceed = false;
			ForEachElementIn(pods){			
				exceed |= check_application_pod((*element), r);
			}
			if (exceed) {
				// [TODO] add a new pod
				num_of_pod++;
			}
		}
	}

	application_pod_number.insert(std::pair<std::string, int>(application_id, num_of_pod));
}
void analyze_data_applicaiton(Json::Value applications) {
	application_pod_number.clear();
	ForEachElementIn(applications){	
		parse_application((*element));
	}
}
/*----------------------------------------------------------------------------*/
std::string query_server_type(std::string server_id) {
	std::string server_type = "";
	if (map_exist(server_id, type_server)) {
		server_type = type_server[server_id];
	}
	return server_type;
}
void build_server_type_mapping(Json::Value root) {
	std::string type;
	std::string name;

	type_server.clear();

	ForEachElementIn(root){	
		type = element.key().asString();
		for (Json::Value::iterator it2 = element->begin(); it2 != element->end(); it2++) {
			name = it2->asString();
			type_server.insert(std::pair<std::string, std::string>(name, type));
		}
	}
}
/*----------------------------------------------------------------------------*/
bool check_server_cpu(Json::Value root, rules* r) {
	bool exceed = false;

	if (root["Load"].isNull() || root["NumberOfCore"].isNull()) {
		std::cerr << "\tCPU load information missing" << std::endl;		
	}
	else{		
		double avgLoad = root["Load"].asDouble() / root["NumberOfCore"].asDouble();
		
		if (map_exist("CPU load", (*r))){
			double threshold = (*r)["CPU load"];
			if (avgLoad > threshold) {
				std::cerr << "\tCPU load exceeds threshold" << std::endl;
				exceed = true;
			}
		}
	}
	return exceed;
}
bool check_server_mem(Json::Value root, rules* r) {
	bool exceed = false;

	if (root["SizeOfMem"].isNull() || root["CurrUsage"].isNull()) {
		std::cerr << "\tMemory information missing" << std::endl;		
	}
	else {		
		double percentage = root["CurrUsage"].asDouble() / root["SizeOfMem"].asDouble();

		if (map_exist("Memory", (*r))){
			double threshold = (*r)["Memory"];
			if (percentage > threshold) {
				std::cerr << "\tMemory exceeds threshold" << std::endl;
				exceed = true;
			}
		}
	}
	return exceed;
}
bool check_server_disk(Json::Value root, rules* r) {
	bool exceed = false;
	// [TODO]
	return exceed;
}
bool chech_server_network(Json::Value root, rules* r) {
	bool exceed = false;
	// [TODO]
	return exceed;
}
void parse_server(Json::Value server) {
	std::string server_id = server["ServerID"].asString();
	std::string server_type = query_server_type(server_id);

	std::cout << "Checking server: " << server_id << "..." << std::endl;

	if (!map_exist(server_type, ruleset_server)) {
		std::cout << "No rules for the server, pass." << std::endl;
	}
	else {
		rules* r = ruleset_server[server_type];
		bool exceed = false;

		exceed |= check_server_cpu(server["CoreInfo"], r);
		exceed |= check_server_mem(server["MemInfo"], r);
		exceed |= check_server_disk(server["DiskInfo"], r);
		exceed |= chech_server_network(server["NetworkInfo"], r);

		if (!exceed) {
			std::cout << "Normal" << std::endl;
		}
		else {
			// [TODO] suggest open new server
		}
	}
}
void analyze_data_server(Json::Value root) {
	//int server_amount = root["NumberOfServer"].asInt();
	Json::Value servers = root["ServerInfo"];
	for (unsigned int i = 0; i < servers.size(); i++) {
		parse_server(servers[i]);
	}
}

void analyze_data(Json::Value root) {
	if (!root[STR_SERVER].isNull()) {
		analyze_data_server(root[STR_SERVER]);
	}
	if (!root[STR_APPLICATION].isNull()) {
		analyze_data_applicaiton(root[STR_APPLICATION]);
	}
}
/*----------------------------------------------------------------------------*/
rules* parse_rules(Json::Value root) {
	rules* thresholds = new rules();

	ForEachElementIn(root) {	
		std::string threshold_name = element.key().asString();
		double threshold_value = (*element).asDouble();
		thresholds->insert(theshold_pair(threshold_name, threshold_value));
	}
	return thresholds;
}
void construct_rule_set(Json::Value root, rule_set* rs) {
	rules* default_rules = new rules();

	rs->clear();
	rs->insert(ruleset_pair("", default_rules));

	ForEachElementIn(root){	
		std::string item_name = element.name();
		if ((*element).size() > 0
			&& !map_exist(item_name, (*rs))){
			rules* item_rules = parse_rules((*element));
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
	bool result = true;

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
		result = false;
	}
	return result;
}
/*----------------------------------------------------------------------------*/
bool usage(const char *exe) {
	std::cout << std::string(exe) << " [-r rules] [-t server_type] [-m monitor file]" << std::endl;
			  /* [TODO] add descriptions of input files
			  << "\t -r ... " << std::endl
			  << "\t -t ... " << std::endl
			  << "\t -m ... " << std::endl;
			  */

	return false;
}
bool parse_args(int argc, char *argv[], 
	std::string* fn_rules, std::string* fn_stype, std::string* fn_mdata) {
	int c;

	*fn_rules = FILE_RULES;
	*fn_stype = FILE_SERVER;
	*fn_mdata = FILE_MDATA;

	while ((c = getopt(argc, argv, "m:r:t:")) != -1) {
		switch (c)
		{
		case 'm':
			*fn_mdata = std::string(optarg);
			break;
		case 'r':
			*fn_rules = std::string(optarg);
			break;
		case 't':
			*fn_stype = std::string(optarg);
			break;
		default:
			return usage(argv[0]);
		}
	};

	return true;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	Json::Value root;
	bool read_success = false;

	std::string	filename_rules = "";
	std::string	filename_stype = "";
	std::string	filename_mdata = "";

	read_success = parse_args(argc, argv, 
		&filename_rules, &filename_stype, &filename_mdata);
	if (!read_success) {
		return 0;
	}

	read_success = read_json_tree_from_file(filename_rules, &root);
	if (read_success) {
		construct_rule_sets_from_tree(root);
	}
	else {
		exit(-1);
	}

	read_success = read_json_tree_from_file(filename_stype, &root);
	if (read_success) {
		build_server_type_mapping(root);
	}

	// [TODO] fork a child as daemon to do the following

	while (1) {
		if (!monitorDataUpdate()) {
			go_to_sleep();
			continue;
		}

		read_success = read_json_tree_from_file(filename_mdata, &root);
		if (read_success) {
			analyze_data(root);
		}		
	};

	return 0;
}
