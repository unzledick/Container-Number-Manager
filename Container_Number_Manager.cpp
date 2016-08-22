#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <json/json.h>
#include <sys/timeb.h>
#include <sstream>

#ifdef __unix__
#include <unistd.h>
#elif _WIN32 || _WIN64
#define WINDOWS
#include <windows.h>
#endif // __unix__

#define ForEachElementIn(x) for(Json::Value::iterator element = x.begin(); element != x.end(); element++)

// descriptions of files are in README.md
#define	FILE_RULES	"JsonInput/SLA.json"
#define FILE_SERVER	"JsonInput/serverType.json"
//#define	FILE_MDATA	"../../CHT/program/monitoringOutput.json"
#define	FILE_MDATA	"JsonInput/monitoringOutput.json"
#define FILE_APP_INFO	"JsonInput/applicationInfo.json"

//test application name, experiment use
#define TEST_APPLICATION "ticket-test"

//json use
#define STR_SERVER	"Server"
#define STR_APPLICATION	"Application"

//sleep for one second
#define T_SLEEP_MS	1000

//how long in seconds to check monitoringOutput.json
#define MONITER_TIME	15

//after how many consecutive minutes the monitored data does not exceed th SLA, the Container Number Manager scales down the application
#define SCALE_DOWN_COUNT 5

//if the monitored data is below SCALE_DOWN_PERCENTAGE * SLA, SCALE_DOWN_COUNT += 1
#define SCALE_DOWN_PERCENTAGE 1.0

//map define
typedef std::map<std::string, std::map<std::string, double>*> rule_set;
typedef std::map<std::string, double> rules;
typedef std::pair<std::string, rules*> ruleset_pair;
typedef std::pair<std::string, double> theshold_pair;
rule_set ruleset_server;
rule_set ruleset_application;
std::map<std::string, std::string> type_server;
std::map<std::string, int> application_pod_number;
std::map<std::string, std::map<std::string, std::string>> app_info;

//scale down count record of application
std::map<std::string, int> scale_down_count;
std::map<std::string, int> temp_scale_down_count;

//server 
Json::Value Servers;

//experiment testing use
int response_count = 0;
double response_time[100];


//Mode choose======================================================

//use system instrunction if defined
//#define USE_SYSTEM

//run this program forever if defined, run for one time if not defined
#define USE_WHILE 

//run the program as linux daemon if defined
//#define USE_DAEMON

//prinf the system instruction but not use if defined
#define PRINT_DEBUG

//Mode choose======================================================

//not used, check if the monitor output is updated
bool monitorDataUpdate() {
	// [TODO] check if there are monitor data
	return true;
}
template<class T>

//check if the mapping exist
bool map_exist(std::string target, std::map<std::string, T> m) {
	bool result = false;
	if (m.find(target) != m.end()) {
		result = true;
	}
	return result;
}

//sleep for 1s
void go_to_sleep() {
#ifdef WINDOWS
	Sleep(T_SLEEP_MS);
#else
	usleep(T_SLEEP_MS*1000);
#endif
}

//return system time
long long getSystemTime() {
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

//check if the monitored pod data and the response time of application exceeds threshold 
int check_item(Json::Value root, std::string item_name, rules* r) {	
	double usage = root.asDouble();
	int exceed = 0;	

	if (map_exist(item_name, (*r))) {
		double threshold = (*r)[item_name];
		if (usage > threshold) {
			std::cerr << "\t" << item_name << " exceeds threshold" << std::endl;
			exceed = 1;
		}
		else if( usage < threshold * SCALE_DOWN_PERCENTAGE){
			exceed = -1;
		}
	}
	else
		exceed = -1;

	return exceed;
}

//check pod if its monitored data exceeds threshold
int check_application_pod(Json::Value pod, rules* r) {
	std::string pod_id = pod["PodID"].asString();
	bool exceed = false;
	bool scale_down = true;

	std::cout << "\tpod " << pod_id << "..." << std::endl;

	ForEachElementIn(pod["Contents"]){	
		int check = check_item(*element, element.name(), r) ;
		exceed |= (check == 1);
		scale_down &= (check == -1);
	}

	if(scale_down)
		return -1;
	if (!exceed) {
		std::cout << "\tNormal" << std::endl;
		return 0;
	}
	return 1;
}

//return the cpu of a pod
double get_pod_cpu(Json::Value pod){
	if(pod["Contents"]["CpuLimit"].asDouble() == -1)
		return pod["Contents"]["CpuUsage"].asDouble();
	else
		return pod["Contents"]["CpuLimit"].asDouble();
}

//return the memory of a pod
double get_pod_memory(Json::Value pod){
	if(pod["Contents"]["MemoryLimit"].asDouble() == -1)
                return pod["Contents"]["MemoryUsage"].asDouble();
        else
                return pod["Contents"]["MemoryLimit"].asDouble();
}

//return the balance score
double get_score(double server_cpu_usage, double server_cpu_limit,
	double server_memory_usage, double server_memory_limit){
	return std::fabs(server_cpu_usage/server_cpu_limit -
		server_memory_usage/server_memory_limit);
}

//return the difference of the balance score before and after deploying the pod
double get_score_diff(Json::Value server, Json::Value pod){
	
	double pod_cpu = get_pod_cpu(pod);
	double pod_memory = get_pod_memory(pod);
	double server_cpu_usage = server["CoreInfo"]["Load"].asDouble();
	double server_cpu_limit = server["CoreInfo"]["NumberOfCore"].asDouble();
	double server_memory_usage = server["MemInfo"]["CurrUsage"].asDouble();
	double server_memory_limit = server["MemInfo"]["SizeOfMem"].asDouble();
	
	double score_before = get_score(server_cpu_usage, server_cpu_limit,
		server_memory_usage, server_memory_limit);
	double score_after = get_score(server_cpu_usage + pod_cpu/1000,
		server_cpu_limit, server_memory_usage + pod_memory,
		server_memory_limit);

#ifdef PRINT_DEBUG
	//printf("score before = %f\n",score_before);
	//printf("score after = %f\n",score_after);
#endif

	return score_after-score_before;
}

// add new pod of an application
void deploy(Json::Value application, std::string server_name){
	
	std::string application_id = application["ApplicationID"].asString();
	std::string nodeSelector = app_info[application_id]["nodeSelector"];	
	std::string rc = app_info[application_id]["replicationController"];
	std::stringstream number_of_pod ;
	number_of_pod <<  (application["pod"]["NumberOfPod"].asInt()+1); 
        
	//label node
	std::string instruction = "kubectl label node " + server_name + " " + nodeSelector;
#ifdef PRINT_DEBUG
	std::cout << instruction << std::endl;
#endif
#ifdef USE_SYSTEM
	system(&instruction[0u]);
#endif

	//scale up rc
	instruction = "kubectl scale rc/" + rc + " --replicas=" + number_of_pod.str() + " -n " + application_id;
#ifdef PRINT_DEBUG
	std::cout << instruction << std::endl;
#endif
#ifdef USE_SYSTEM
	system(&instruction[0u]);
#endif
	

	//cancel label
	instruction = "kubectl label node " + server_name + " name-";
#ifdef PRINT_DEBUG
    	std::cout << instruction << std::endl;
#endif
#ifdef USE_SYSTEM
	system(&instruction[0u]);
#endif

}

//close one pod of application
void close(Json::Value application){

	std::string application_id = application["ApplicationID"].asString();
	std::string rc = app_info[application_id]["replicationController"];
	std::stringstream number_of_pod ;
	number_of_pod <<  (application["pod"]["NumberOfPod"].asInt()-1); 
	
	//scale down rc
	std::string instruction = "kubectl scale rc/" + rc + " --replicas=" + number_of_pod.str() + " -n " + application_id;

#ifdef PRINT_DEBUG
	std::cout << instruction << std::endl;
#endif

#ifdef USE_SYSTEM
	system(&instruction[0u]);
#endif

}

//calculate the balance score and choose the machine to deploy new pod
void schedule_new_pod(Json::Value application) {
	
	Json::Value pod = application["pod"]["PodInfo"][0]; 
	int min_score_diff = get_score_diff(Servers[0],pod);
	int min_score_diff_server = 0;

	if(Servers.size() > 1){ 
		double score_diff;
		for(int i = 1; i < Servers.size(); i++){
			score_diff = get_score_diff(Servers[i],pod);
#ifdef PRINT_DEBUG
			//printf("score diff = %f\n.",score_diff);		
#endif
			if (score_diff < min_score_diff){
				min_score_diff = score_diff;
				min_score_diff_server = i;
			}
		}
	}	

	//deploy new pod
	deploy(application,Servers[min_score_diff_server]["ServerID"].asString());
}

//check if we need to scale down the number of pod
bool check_scale_down(Json::Value application){
	
	int count = scale_down_count.count(application["ApplicationID"].asString());
	
	//update scale_down_count
	if(count == 0)
		temp_scale_down_count[application["ApplicationID"].asString()] = 1;
	else
		temp_scale_down_count[application["ApplicationID"].asString()] = 
			scale_down_count[application["ApplicationID"].asString()]+1;

	//check if exceed SCALE_DOWN_COUNT
	if(temp_scale_down_count[application["ApplicationID"].asString()] == SCALE_DOWN_COUNT){
		close(application);
		temp_scale_down_count.erase(application["ApplicationID"].asString());
		return true;
	}
	return false;
}

// check if application's monitored data exceeds threshold
void parse_application(Json::Value application) {
	std::string application_id = application["ApplicationID"].asString();
	int num_of_pod = application["pod"]["NumberOfPod"].asInt();
	std::cout << "Checking application: " << application_id << "..." << std::endl;

	// check if there are rules for the application
	if (!map_exist(application_id, ruleset_application)) {
		std::cout << "No rules for this application, pass." << std::endl;
	}
	else{


		rules* r = ruleset_application[application_id];
		Json::Value pods = application["pod"]["PodInfo"];

		//*experiment testing use
		if(application_id=="ticket-monster"){
			response_time[response_count] = application["AvgResponseTime"].asDouble();
			response_count += 1;
			printf("response time\n");
			for(int i = 0; i< response_count;i++){
				printf("%d %f\n",i,response_time[i]);
			}

		}

		bool scale_down = true;;		
	
		//check application's response time
		int check = check_item(application["AvgResponseTime"],"AvgResponseTime",r);
		scale_down &= (check == -1);
		if (check == 1) {
			schedule_new_pod(application);
			num_of_pod++;			
		}

		//check pod's monitored data 
		else if (!application["pod"].isNull()
				&& !application["pod"]["PodInfo"].isNull()) {

			bool exceed = false;
			ForEachElementIn(pods){			
				check = check_application_pod((*element), r);
				exceed |= (check == 1);
				scale_down &= (check == -1);
			}

			//exceed threshold
			if (exceed) {
				schedule_new_pod(application);
				num_of_pod++;
			}
			//scale down
			else if(num_of_pod>1 && scale_down){
				if(check_scale_down(application))
					num_of_pod--;
			}

		}


	}

	application_pod_number.insert(std::pair<std::string, int>(application_id, num_of_pod));
}

// check if applications' monitored data exceeds threshold
void analyze_data_applicaiton(Json::Value applications) {

 	application_pod_number.clear();
	ForEachElementIn(applications){	
		parse_application((*element));
	}

	//clear temporary scale down count
	scale_down_count = temp_scale_down_count;
	temp_scale_down_count.clear();

#ifdef PRINT_DEBUG
	printf("All monitored data of %s is below %f*SLA, count: %d\n",TEST_APPLICATION,SCALE_DOWN_PERCENTAGE,scale_down_count[TEST_APPLICATION]);
#endif

}


// return the type of server
std::string query_server_type(std::string server_id) {
	std::string server_type = "";
	if (map_exist(server_id, type_server)) {
		server_type = type_server[server_id];
	}
	return server_type;
}

// store the type of the server in type_server
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

//check server if its monitored cpu usage exceeds threshold
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

//check server if its monitored memory usage exceeds threshold
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

//check server if its monitored disk usage exceeds threshold
bool check_server_disk(Json::Value root, rules* r) {

	bool exceed = false;

	if (root["FilesystemLimit"].isNull() || root["FilesystemUsage"].isNull()) {
		std::cerr << "\tDisk information missing" << std::endl;
	}
	else {
		double percentage = root["FilesystemUsage"].asDouble() / root["FilesystemLimit"].asDouble();
		if (map_exist("Disk", (*r))){
			double threshold = (*r)["Disk"];
			if (percentage > threshold) {
				std::cerr << "\tDisk exceeds threshold" << std::endl;
				exceed = true;
			}
		}
	}

	return exceed;
}

//check server if its monitored network usage exceeds threshold
bool chech_server_network(Json::Value root, rules* r) {

	bool exceed = false;

	if (root["ReceivedCumulativeBytes"].isNull() || root["SentCumulativeBytes"].isNull()) {
		std::cerr << "\tNetwork information missing" << std::endl;
	}
	else {

		double network_receive  = root["ReceivedCumulativeBytes"].asDouble();
		double network_sent  = root["SentCumulativeBytes"].asDouble();

		if (map_exist("Network received", (*r))){
			double threshold = (*r)["Network received"];
			if (network_receive > threshold) {
				std::cerr << "\tNetwork received exceeds threshold" << std::endl;
				exceed = true;
			}
		}

		if (map_exist("Network sent", (*r))){
			double threshold  = (*r)["Network sent"];
			if (network_sent > threshold) {
				std::cerr << "\tNetwork sent exceeds threshold" << std::endl;
				exceed = true;
			}
		}

	}

	return exceed;
}

//check one server if its monitored data exceeds threshold
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
			printf("Server not enough.");
		}
	}
}

//check all servers if their monitored data exceeds threshold
void analyze_data_server(Json::Value root) {

	//int server_amount = root["NumberOfServer"].asInt();
	Servers = root["ServerInfo"];

	for (int i = 0; i < Servers.size(); i++) {
		parse_server(Servers[i]);
	}
}

//check if application/server monitored data exceeds threshold
void analyze_data(Json::Value root) {
	if (!root[STR_SERVER].isNull()) {
		analyze_data_server(root[STR_SERVER]);
	}
	if (!root[STR_APPLICATION].isNull()) {
		analyze_data_applicaiton(root[STR_APPLICATION]);
	}
}

//return rules from input jason value
rules* parse_rules(Json::Value root) {
	rules* thresholds = new rules();

	ForEachElementIn(root) {	
		std::string threshold_name = element.key().asString();
		double threshold_value = (*element).asDouble();
		thresholds->insert(theshold_pair(threshold_name, threshold_value));
	}
	return thresholds;
}

//store rules in rule_set 
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

//construct rules from json tree
void construct_rule_sets_from_tree(Json::Value root) {	
	if (!root["Application"].isNull()) {
		construct_rule_set(root[STR_APPLICATION], &ruleset_application);
	}
	if (!root[STR_SERVER].isNull()) {
		construct_rule_set(root[STR_SERVER], &ruleset_server);
	}	
}

//read from json file and check if fail
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

// store application information in app_info
void construct_application_information(Json::Value info){
	ForEachElementIn(info){	
		app_info[(*element)["applicationID"].asString()]["nodeSelector"] = (*element)["nodeSelector"].asString();
		app_info[(*element)["applicationID"].asString()]["replicationController"] = (*element)["replicationController"].asString();
	}
}

int main(int argc, char *argv[])
{
	Json::Value root;
	bool read_success = false;


	//read SLA from json 
	read_success = read_json_tree_from_file(FILE_RULES, &root);
	if (read_success) {
		construct_rule_sets_from_tree(root);
	}
	else {
		exit(-1);
	}

	//read type of server from json 
	read_success = read_json_tree_from_file(FILE_SERVER, &root);
	if (read_success) {
		build_server_type_mapping(root);
	}


	// fork a child as daemon to do the following
#ifdef USE_DAEMON
	daemon(1,1);
#endif



#ifdef USE_WHILE
	long long start=getSystemTime() - MONITER_TIME*1000;	
	long long end;	

	while (1) {
		end=getSystemTime();
		if(end-start < MONITER_TIME*1000){
			go_to_sleep();
			end=getSystemTime();
			continue;
		}
		else{		
			start = end;

#endif
			//read application information form json
			read_success = read_json_tree_from_file(FILE_APP_INFO, &root);
			if (read_success) {
				construct_application_information(root);
			}

			//read monitor output form json
			read_success = read_json_tree_from_file(FILE_MDATA, &root);
			if (read_success) {
				analyze_data(root);
			}		

#ifdef USE_WHILE
		}
	};
#endif

	return 0;
}
