#include <iostream>
#include <string>

#include "Deploy.h"
int main(int argc , const char* argv[]){
	Deploy newDeploy(argv[1] , 0);
	newDeploy.createPod();

	return 0;
}
