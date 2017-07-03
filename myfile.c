#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string>
#include <regex>
#include <algorithm>
#include <sstream>

#define BUFFSIZE 1024

char* getinput(char *buffer, size_t buflen) {
	printf("$$ ");
	return fgets(buffer, buflen, stdin);
}
int main(int argc, char **argv) {
	char buf[BUFFSIZE];
	int n;
	pid_t pid;
	int status;
	while (getinput(buf, sizeof(buf))) {
		buf[strlen(buf) - 1] = '\0';
		std::string com = buf;
		com.erase(std::remove_if(com.begin(), com.end(), ::isspace), com.end());
		/* Place your code to check "exit". If so, then exit */
		if(std::regex_match(com, std::regex("exit"))) break;
		int pos = 0;
		while(pos < com.length()){
			int endPos = com.length();
			std::stringstream ss;
			//printf(com.substr(pos, com.length()).c_str());
			//printf("\n");
			if(com.substr(pos, endPos).find("|") < std::string::npos){
				//printf("HAS A PIPE\n");
				endPos = com.substr(pos, endPos).find_first_of("|");
			}
			ss << com.substr(pos, endPos);
			std::string command = ss.str();
			if((pid=fork()) == -1) {
				fprintf(stderr, "shell: can't fork: %s\n", strerror(errno));
				continue;
			} else if (pid == 0) {
				/* child process to do each command 
				 *    – place your code here to handle read, write, append */
				if(std::regex_match(command, std::regex("read<.*"))){
					std::string f_name = command.substr(command.find_first_of("<")+1, command.length());
					//printf(f_name.c_str());
					int readPID = open(f_name.c_str(), O_RDONLY);
					int writePID = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					while ((n = read(readPID, buf, BUFFSIZE)) > 0) {
						if(write(writePID, buf, n) != n) {
							fprintf(stderr, "write error\n");
				        		exit(1);
						}
					}
					if(n < 0) {
						fprintf(stderr, "read error\n");
						exit(1);
					}

				} else if(std::regex_match(command, std::regex("write>.*"))){
					 std::string f_name = command.substr(command.find_first_of(">")+1, command.length());
					//printf(f_name.c_str());
					int writePID = open(f_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					int readPID = open("temp.txt", O_RDONLY);
					while ((n = read(readPID, buf, BUFFSIZE)) > 0) {
						if(write(writePID, buf, n) != n) {
							fprintf(stderr, "write error\n");
							exit(1);
						}
					}
					if(n < 0) {
						fprintf(stderr, "read error\n");
						exit(1);
					}
				
				} else if(std::regex_match(command, std::regex("append>>.*"))){
					std::string f_name = command.substr(command.find_first_of(">>")+2, command.length());
					//printf(f_name.c_str());
					int writePID = open(f_name.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
					int readPID = open("temp.txt", O_RDONLY);
					while ((n = read(readPID, buf, BUFFSIZE)) > 0) {
						if(write(writePID, buf, n) != n) {
							fprintf(stderr, "write error\n");
							exit(1);
						}
					}
					if(n < 0) {
						fprintf(stderr, "read error\n");
						exit(1);
					}
				}else if(std::regex_match(command, std::regex("ls"))){
					DIR* dp = opendir("./");
					struct dirent *dir;
					while((dir = readdir(dp)) != NULL)
						printf("%s\n", dir->d_name);
				} else{
					fprintf(stderr, "Invalid command\n");
				}
				exit(EX_OK);
			}
			pos = endPos+1;
			if ((pid=waitpid(pid, &status, 0)) < 0)
				fprintf(stderr, "shell: waitpid error: %s\n", strerror(errno));
		}
	}
	exit(EX_OK);
																				}

