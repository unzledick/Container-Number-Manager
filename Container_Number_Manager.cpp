#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <sstream>

#include "json/json.h"
#include "/home/ricktsai/rick_lib/include/curl/curl.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <cmath>

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

#define T_SLEEP_MS	1000

//Rick part =============================================================

//should be get from json
std::string  HEAPSTER_IP = "172.30.178.127:80";
std::string  NAMESPACE = "container-number-manager2"; 
std::string  RC_NAME = "cnm-app";
int CPU_LIMIT = 400; //milli-core
long long int  MEMORY_LIMIT = 512*1024*1024; //byte 
int POD_NUMBER = 5;
#define SERVER_NUMBER 2
const std::string Server[2] ={"opsm1.pcs.csie.ntu.edu.tw","opsn1.pcs.csie.ntu.edu.tw"};
double Server_capacity_memory[2]={11857728*1024LL,3596144*1024LL};
double Server_capacity_cpu[2]={8000,4000};

//type defineldw
#define TYPE_NAMESPACE 1
#define TYPE_NODE 2
#define RESOURCE_CPU 1
#define RESOURCE_MEMORY 2

//runnung test
#define USE_WHILE 
//#define USE_DAEMON

//Rick part =============================================================

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
	sleep(1);
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
	for (int i = 0; i < servers.size(); i++) {
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

void read_information_from_file(){


	Json::Value root;
	bool read_success = false;

	read_success = read_json_tree_from_file(FILE_RULES, &root);
	if (read_success) {
		construct_rule_sets_from_tree(root);
	}
	else {
		exit(-1);
	}

	read_success = read_json_tree_from_file(FILE_SERVER, &root);
	if (read_success) {
		build_server_type_mapping(root);
	}

#ifdef USE_DAEMON	
	daemon(1,1);
#endif

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
}

size_t WriteToString (void *contents, size_t size, size_t nmemb, void *userp){
	*((std::string*)userp) += (char*)contents;
	return size * nmemb;
}

long long int get_resource_usage(int type, std::string name ,std::string resource){

	CURL *curl;
	curl = curl_easy_init();
	CURLcode res;

	std::string temp;	
	std::string curl_ip;

	//printf("%d\n",type);	
	switch(type){
		case TYPE_NAMESPACE:
			curl_ip = HEAPSTER_IP + "/api/v1/model/namespaces/" + name + "/metrics/" + resource;
			break;
		case TYPE_NODE:
			curl_ip = HEAPSTER_IP + "/api/v1/model/nodes/" + name + "/metrics/" + resource;
			break;
	}

	//std::cout << curl_ip << std::endl;

	curl_easy_setopt(curl, CURLOPT_URL, &curl_ip[0u]);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &temp);	
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 900);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,1000);

	res = curl_easy_perform(curl);
	if(res != 0)
		printf("Curl failed: %d\n", res);

	//std::cout << temp << std::endl;

	Json::Reader reader;
	Json::Value root;

	reader.parse(temp.c_str(),root);	

	//std::cout << root["metrics"][0]["value"].asString() << std::endl;	

	return (long long int)root["metrics"][0]["value"].asLargestUInt();
}

long long getSystemTime() {
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

int choose_server(double pod_cpu, double pod_memory){
	//Server_capacity_memory[]
	//Server_capacity_cpu[]
	double cpu;
	double mem;
	double score_before,score_after,score_diff,minimal_diff;
	int server_chosen= -1;
	for(int i = 0;i<SERVER_NUMBER;i++){
		cpu = get_resource_usage(TYPE_NODE,Server[i],"cpu-usage");
		mem = get_resource_usage(TYPE_NODE,Server[i],"memory-usage");
		if((Server_capacity_cpu[i]-cpu> pod_cpu) && (Server_capacity_memory[i]-mem> pod_memory)){
			server_chosen = i;
		}	
	}

	if(server_chosen == -1){
		printf("Server not enough.\n");
		return -1;
	}
	else if(server_chosen == SERVER_NUMBER-1)
		return server_chosen;

	for(int i = server_chosen+1;i<SERVER_NUMBER;i++){

		cpu = get_resource_usage(TYPE_NODE,Server[i],"cpu-usage");
		mem = get_resource_usage(TYPE_NODE,Server[i],"memory-usage");
		//printf("node %d cpu=%f\n",i,cpu/Server_capacity_cpu[i]);
		//printf("node %d mem=%f\n",i,mem/Server_capacity_memory[i]);
		score_before = std::abs(cpu/Server_capacity_cpu[i]-mem/Server_capacity_memory[i]);
		//printf("score=%f\n",score_before);
		cpu += pod_cpu;
		mem += pod_memory;
		//printf("node %d cpu=%f\n",i,cpu/Server_capacity_cpu[i]);
		//printf("node %d mem=%f\n",i,mem/Server_capacity_memory[i]);
		score_after = std::abs(cpu/Server_capacity_cpu[i]-mem/Server_capacity_memory[i]);
		//printf("score=%f\n",score_after);
		score_diff = score_after - score_before;	
		//printf("score=%f\n",score_diff);
		if((Server_capacity_cpu[i]-cpu> pod_cpu) && (Server_capacity_memory[i]-mem> pod_memory)){	
			if(score_diff<minimal_diff){
				minimal_diff = score_diff;
				server_chosen = i;			
			}
		}		

	}	
	printf("Server chosen = %d\n",server_chosen);
	return server_chosen;
}

void analyze_ap_metrics(){

	FILE *fp;
	char path[1000];
	char pod_name[100][1000];
	int pod_number = 0; 
	char* command = (char*)malloc(1000*sizeof(char));
	printf("\nApplication level metrics:\n");
	printf("==========================\n");


	//get pod
	fp = popen("oc get pod --namespace=container-number-manager2|grep cnm-app|awk '{print$1}'","r");
	if(fp==NULL)
		printf("failed\n");
	while(fgets(path, sizeof(path)-1, fp) != NULL){
		strcpy(pod_name[pod_number],path);
		pod_name[pod_number][strlen(pod_name[pod_number])-1] = 0;
		printf("Pod %d: %s\n",pod_number,pod_name[pod_number]);
		pod_number += 1;
	}

	//get heap memory
	strcpy(command,"kubectl exec ");
	strcat(command,pod_name[0]);
	strcat(command,"  -- ./../../opt/eap/bin/jboss-cli.sh --connect /core-service=platform-mbean/type=memory:read-attribute'('name=heap-memory-usage')' | grep used | awk '{print$3}'");	
	fp = popen(command,"r");
	if(fp==NULL)
                printf("failed\n");
	fgets(path, sizeof(path)-1, fp);
	path[strlen(path)-2]=0;		
        printf("Heap memory used: %lld\n",atoll(path));

	//get thread count
	strcpy(command,"kubectl exec ");
        strcat(command,pod_name[0]);        
	strcat(command," -- ./../../opt/eap/bin/jboss-cli.sh --connect /core-service=platform-mbean/type=threading:read-attribute'('name=thread-count')' | grep result | awk '{print$3}'");	
	fp = popen(command,"r");
	if(fp==NULL)
        	printf("failed\n");
	fgets(path, sizeof(path)-1, fp);
	printf("Thread count: %d\n",atoi(path));

	//get current thread cpu time 
	strcpy(command,"kubectl exec ");
        strcat(command,pod_name[0]);
        strcat(command," -- ./../../opt/eap/bin/jboss-cli.sh --connect /core-service=platform-mbean/type=threading:read-attribute'('name=current-thread-cpu-time')' | grep result | awk '{print$3}'");
	fp = popen(command,"r");
        if(fp==NULL)
                printf("failed\n");
        fgets(path, sizeof(path)-1, fp);
        path[strlen(path)-2]=0;
        printf("Current thread cpu time: %lld\n",atoll(path));

	//get http connector request count
	strcpy(command,"kubectl exec ");
        strcat(command,pod_name[0]);
        strcat(command," -- ./../../opt/eap/bin/jboss-cli.sh --connect /subsystem=web/connector=http:read-attribute'('name=requestCount')' | grep result | awk '{print$3}'");
	fp = popen(command,"r");
        if(fp==NULL)
                printf("failed\n");
        fgets(path, sizeof(path)-1, fp);
        for(int i = 1;i<strlen(path);i++)
		path[i-1] = path[i];
	path[strlen(path)-3]=0;
	printf("Http connector request count: %d\n",atoi(path));	
	
			

}

void analyze_resource_usage(){

	int cpu = get_resource_usage(TYPE_NAMESPACE,NAMESPACE,"cpu-usage");
	long long int mem = get_resource_usage(TYPE_NAMESPACE,NAMESPACE,"memory-usage");
	
	printf("\nResource usage:\n");
	printf("===============\n");
	printf("cpu: %d\n",cpu);
	printf("mem: %lld\n",mem);
	printf("cpu limit: %d\n",CPU_LIMIT);
	printf("mem limit: %lld\n",MEMORY_LIMIT);

	if(cpu > CPU_LIMIT * 0.8 * POD_NUMBER || mem > MEMORY_LIMIT * 0.8 * POD_NUMBER ){

		POD_NUMBER += 1;
		std::stringstream ss;
		std::string POD_NUMBER_STRING;
		ss << POD_NUMBER; 
		ss >> POD_NUMBER_STRING; 

		std::string scale_instruction = "kubectl scale rc " + RC_NAME + " --replicas=" + POD_NUMBER_STRING;
		std::cout << scale_instruction << std::endl;
		int server = choose_server((double)CPU_LIMIT,(double)MEMORY_LIMIT); 
		if(server == -1)
			printf("Scale up failed\n");
		//system(&scale_instruction[0u]);
	
	}
	else{
		printf("resource enough.\n");
	}

}

void read_information_from_API(){

	Json::Value root;
	bool read_success = false;

	read_success = read_json_tree_from_file(FILE_RULES, &root);
	if (read_success) {
		construct_rule_sets_from_tree(root);
	}
	else {
		exit(-1);
	}

	read_success = read_json_tree_from_file(FILE_SERVER, &root);
	if (read_success) {
		build_server_type_mapping(root);
	}
	//for(std::map<std::string, std::string>::iterator iter = type_server.begin(); iter != type_server.end(); iter++)
	//	std::cout<<iter->first<<" "<<iter->second<<std::endl;

#ifdef USE_DAEMON	
	daemon(1,1);
#endif

	//struct timeval start, end;
	long long start=getSystemTime()-15000;
#ifdef USE_WHILE
	while(1){
#endif	
		long long end=getSystemTime();	

		if(end-start>=15*1000){
			start = end;
			analyze_resource_usage();
			analyze_ap_metrics();			
		}
		else{
			go_to_sleep();
			end=getSystemTime();
			//printf("end:%lld\n",end);
			//printf("start:%lld\n",start);
			//printf("time:%d\n",(end-start)/1000);
		}	
#ifdef USE_WHILE
	}
#endif
}


int main(int argc, char *argv[])
{

	//read_information_from_file();
	read_information_from_API();

	exit(0);
}
