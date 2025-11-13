#include "Webserver.hpp"

int main(int argc, char **argv){
	if (argc != 2)
		return (exitWithError(WRONG_ARGUMENTS));
	/* Maybe a:
		validateConfFile()
		parseFile()
		
	*/
	Webserver miniNginx;
	if (miniNginx.startServers() != 0){
		miniNginx.stopServers();
		return (exitWithError(FAILED_TO_START_SERVER));
	}
	miniNginx.run();
	miniNginx.stopServers();
	return SUCCESS;
}