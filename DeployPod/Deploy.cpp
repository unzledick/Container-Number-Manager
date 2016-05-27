#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fstream>

#include "Deploy.h"
#include "json/json.h"
using namespace std;

Deploy::Deploy(){
}

Deploy::Deploy(const string filename , int node){
	string label_key = "kubernetes.io/hostname" ;
	string label_value = Server[node];
	Json::Reader reader;
	Json::Value root;
	Json::Value label;
	label[label_key] = label_value;
	ifstream podJson(filename,ifstream::binary);
	//read json file	
	if(reader.parse(podJson , root)){
		//add label to nodeselector
		root["spec"]["nodeSelector"] = label ;
		podJson.close();
	}
	//write to temp.json
	Json::StyledStreamWriter writer ;
	ofstream tempJson("temp.json");
	writer.write(tempJson , root);
	tempJson.close();	
}

int Deploy::createPod(){
	pid_t pid;
	int status;
		
	if((pid = fork() )<0){
		cout << "Fail to fork" << endl;
	}	
	else if(pid ==0){
	//child process
	execlp("oc" , "oc" , "create" , "-f" , "temp.json" , (char* )0);
	
	}else{
	//parent process
	wait(&status);
	execlp("rm" , "rm" , "temp.json" , (char* )0);
	}

return 0;
}

	
