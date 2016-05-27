#include <iostream>
using namespace std;
class Deploy{
public:
	Deploy();
	Deploy(string str , int node);
	
	const string Server[2] = {"opsm1.pcs.csie.ntu.edu.tw" , "opsn1.pcs.csie.ntu.edu.tw"};
	
	int createPod();

};
