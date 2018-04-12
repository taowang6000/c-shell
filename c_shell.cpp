#include <sys/stat.h>
#include <sys/wait.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <utility>
#include <string>
#include <cstring>			//strcpy, strlen series functions
#include <unistd.h>			//getcwd
#include <dirent.h>
#include <stdlib.h>			//exit
#include <fcntl.h>			//open/create files

#define MAXLINE 81
#define MAXPATHLENGTH 1024;


/*
 * check the syntax of  the parameter of set command
 */
bool MYPATH_syntax_check(std::string str){
	std::vector<std::string> v;
	std::string token;
	int i = 0;

	for(unsigned int j = 0; j < str.size(); j++){
		if ((str[j] != '=') && (str[j] != ':')){
			token += str[j];
		}
		else{
			if (i == 0 && str[j] == ':')	return false;
			if ( i > 0 && str[j] == '=')	return false;
			v.push_back(token);
			token.clear();
			i++;
		}
	}
	v.push_back(token);		//push in the last token
	token.clear();

	if (v[0] != "MYPATH") return false;
	if (v.size() == 1) return false;

	for (unsigned int i = 1; i < v.size(); i++){
		if (v[i][0] != '/') return false;
	}
	return true;
}

/*
 * process "myls" command
 */
void process_myls(std::vector<std::string> v){

	std::string env;

	if (!getenv("MYPATH")){
		std::cout <<"need to setup MYPATH environment" << std::endl;
		exit(0);
	}
	env = getenv("MYPATH");

	//take out each individual path from the path list, push in a vector
	std::vector<std::string> parsed_path;
	std::string str;

	for (unsigned int i = 0; i < env.size(); i ++){
		if (env[i] != ':'){
			str.push_back(env[i]);
			continue;
		}
		parsed_path.push_back(str);
		str.clear();
	}
	parsed_path.push_back(str);
	str.clear();

	//search each path, looking for the file "myls"
	DIR *dp;
	struct dirent *dirp;
	bool file_hit = false;		//whether "myls" is found or not
	std::string path_found;		//the first path found where "myls" resides

	for (unsigned int i = 0; i < parsed_path.size(); i++){
		if (file_hit == true) break;
		if ((dp = opendir(parsed_path[i].c_str())) == NULL){
			std::cout << "Can't open directory: " << parsed_path[i] << std::endl;
			exit(0);
		}
		while ((dirp = readdir(dp)) != NULL){
			if (strcmp(dirp->d_name, "myls") == 0){
				file_hit = true;
				path_found = parsed_path[i];
				break;
			}
		}
		closedir(dp);
	}
	if (file_hit == true){
		std::string full_name = path_found + "/myls";
		if (v.size() == 1){
			if (execl(full_name.c_str(), "myls", (char*) 0) < 0){
				std::cout << "execl error" << std::endl;
				exit(1);
			}
		}
		else if (v.size() == 2){
			if (execl(full_name.c_str(), "myls", v[1].c_str(), (char*) 0) < 0){
				std::cout << "execl error" << std::endl;
				exit(1);
			}
		}
		else std::cout << "too many parameters for myls" << std::endl;
	}
	else std::cout << "can't locate file: myls" << std::endl;
}


/*
 * * process four built-in commands "exit", "cd", "pwd". and "set"
 */
void process_built_in_commands(std::vector<std::string> v){

	if (v[0] == "exit"){
				exit(0);
	}
	else if (v[0] == "pwd"){
		size_t 	path_size = MAXPATHLENGTH;
		char *path_buffer = new char[path_size];

		if (getcwd(path_buffer, path_size) == NULL){
			delete [] path_buffer;
			std::cout << "getcwd failed" << std::endl;
			exit(1);
		}
		std::cout << path_buffer << std::endl;
		delete [] path_buffer;
	}
	else if (v[0] == "cd"){
		if (v.size() > 1)
			if (chdir(v[1].c_str()) < 0){
				std::cout << "chdir failed" << std::endl;
				exit(1);
			}
	}
	else if (v[0] == "set"){

		if (v.size() >2)
			std::cout <<"set command error;: too many parameters"<< std::endl;
		if (v.size() == 2){
			if (MYPATH_syntax_check(v[1])){

				std::string mypath_value;

				//remove "MYPATH=" 7 characters from v[1]
				for (unsigned int i = 7; i < v[1].size(); i++){
					mypath_value.push_back(v[1][i]);
				}
				size_t env_size = mypath_value.size();
				char *env = new char[env_size + 1];
				std::stringstream ss(mypath_value);
				ss >> env;
				if (setenv("MYPATH", env, 1) < 0){
					delete [] env;
					std::cout << "setenv failed" << std::endl;
					exit(1);
				}
				delete [] env;
			}
			else
				std::cout << "set command syntax error" << std::endl;
		}
	}
}

bool this_is_a_background_command(std::vector<std::string> v){
	return (v[v.size()-1] == "&");
}

bool there_are_redirections(std::vector<std::string> v){
	for (unsigned int i = 0; i < v.size(); i++){
		if ((v[i] == "<") || (v[i] == ">"))
			return true;
		}
	return false;
}

/*
 * Check if there are pipe symbols "|" or not
 */
int there_are_pipes(std::vector<std::string> v){
	int pipe_number = 0;
	for (unsigned int i = 0; i < v.size(); i++){
		if (v[i] == "|") pipe_number++;
	}
	return pipe_number;
}

void convert_vector_into_argv(std::vector<std::string> v, char *argv[10]){
	if (v.size() > 9){
		std::cout << "Too many parameters" << std::endl;
	}

	unsigned int i;
	for (i = 0; i < v.size(); i++){
		argv[i] = (char *)v[i].c_str();
	}
	argv[i] = NULL;
}

void process_command_with_one_pipe(std::vector<std::string> v){
	std::vector<std::string> v1, v2;
	int position_of_pipe = 0;

	if ((v[0] == "|") || v[v.size() - 1] == "|" ){
		std::cout << "wrong position of pipe" << std::endl;
		return;
	}

	// split input vector into two vectors: v1 and v2,  separated by "|"
	for (unsigned int i = 0; i < v.size(); i++){
		if (v[i] == "|") break;
		position_of_pipe++;
	}
	for (int i = 0; i < position_of_pipe; i++){
		v1.push_back(v[i]);
	}
	for (unsigned int i = position_of_pipe + 1; i < v.size(); i++){
		v2.push_back(v[i]);
	}

	//execute the commands
	int fds[2];
	pid_t pid1, pid2;
	char *argv[10];

	if (pipe(fds) < 0) {
		std::cout << "pipe error" << std::endl;
		exit(1);
	}

	if ((pid1 = fork()) < 0) {
		std::cout << "fork error" << std::endl;
		exit(1);
	}
	else if (pid1 == 0){
		//child process 1, handles v2(the command after "|")
		close(0);
		if (dup(fds[0]) < 0) {
			std::cout << "dup error" << std::endl;
			exit(1);
		}
		close(fds[0]);
		close(fds[1]);
		convert_vector_into_argv(v2, argv);
		execvp(argv[0], argv);
		std::cout << "could not execute: " << v2[0] << std::endl;
		exit(1);

	}

	if ((pid2 = fork())< 0) {
		std::cout << "fork error" << std::endl;
		exit(1);
	}
	else if (pid2 == 0){
		//child process 2, handles v1(the command before "|")

		close(1);
		if (dup(fds[1]) <0){
			std::cout << " dup error" << std::endl;
			exit(1);
		}
		close(fds[0]);
		close(fds[1]);

		if (v1[0] == "myls"){
				process_myls(v1);
		}
		else {
			convert_vector_into_argv(v1,argv);
			execvp(argv[0], argv);
		}
		std::cout << "could not execute: " << v1[0] << std::endl;
		exit(1);
	}

	close(fds[0]);
	close(fds[1]);

	int status;
	for (int i = 0; i < 2; i++){
		wait(&status);		//wait for three children to finish
	}
}

void process_command_with_multiple_pipes(std::vector<std::string> v,
		int num_pipes){
	// vector of each command
	std::vector<std::vector<std::string>> commands;
	std::vector<int> positions;		//position of "|"
	std::vector<std::string>	v_temp;

	//split input vector into multiple vectors(commands),  separated by "|"
	for (unsigned int i = 0; i < v.size(); i++){
		if (v[i] == "|") positions.push_back(i);
	}

	for (unsigned int i = 0; i < positions[0]; i++){ 	//first command
		v_temp.push_back(v[i]);
	}
	commands.push_back(v_temp);
	v_temp.clear();

	for (unsigned int i = 0; i < num_pipes - 1; i++){	//middle command(s)
		for (unsigned int j = positions[i]+1; j < positions[i+1]; j++){
			v_temp.push_back(v[j]);
		}
		commands.push_back(v_temp);
		v_temp.clear();
	}

	for (unsigned int i = positions[num_pipes-1]+1; i < v.size(); i++){
		v_temp.push_back(v[i]);							//last command
	}
	commands.push_back(v_temp);
	v_temp.clear();

	//parent create all needed pipes
	int fds[num_pipes * 2];
	for (unsigned int i = 0; i < num_pipes; i++ ){
	    if( pipe(fds + i*2) < 0 ){
	        std::cout << "pipe error" << std::endl;
	        exit(1);
	    }
	}

	//main body of commands execution
	pid_t	pid;
	char * argv[10];
	int status;

	for (unsigned int i = 0; i < num_pipes + 1; i++){
		pid = fork();
		if (pid < 0){
			std::cout << "fork error" << std::endl;
			exit(1);
		}else if (pid == 0){
			// child gets the input from the previous command
			if (i != 0){
				if (dup2(fds[(i-1)*2],0) < 0){
					std::cout << "first dup2 error " << i <<  std::endl;
					exit(1);
				}
			}
			//child outputs to the next command
			if (i != num_pipes){
				if (dup2(fds[i*2 + 1],1) < 0){
					std::cout << "second dup2 error " << i <<std::endl;
					exit(1);
				}
			}
			for (unsigned int i = 0; i < num_pipes * 2; i++){
				close(fds[i]);
			}
			if (commands[i][0] == "myls") {
				process_myls(commands[i]);
			}
			else {
				convert_vector_into_argv(commands[i],argv);
				if (execvp(argv[0], argv) < 0){
					std::cout << "execvp error" << std::endl;
					exit(1);
				}
			}
		}
	}

	//parent closes all its copies
	for (unsigned int i = 0; i < num_pipes * 2; i++){
		close(fds[i]);
	}

	for (unsigned int i = 0; i < num_pipes + 1; i++){
		wait(&status);
	}
}

void process_redirections_command(std::vector<std::string> v, bool bg_bit){
	if (bg_bit) v.pop_back();	//delete last element "&"

	std::vector<std::string>	command;	//command to be executed
	std::vector<int>			positions;	//positions of redirection symbols
	std::vector<std::string>	symbols;	//redirection symbols
	std::vector<std::string>	file_names;	//input source and output target

	for (unsigned int i = 0; i < v.size(); i++){
		if ((v[i] == "<") || (v[i] == ">")){
			positions.push_back(i);
			symbols.push_back(v[i]);
		}
		else{
			if (positions.size() == 0)	command.push_back(v[i]);
			else file_names.push_back(v[i]);
		}
	}
	if (positions.size() > 2){
		std::cout << "too many redirection sysbols" << std::endl;
		return;
	}
	if (positions.size() != file_names.size()){
		std::cout << "wrong format" << std::endl;
		return;
	}

	pid_t 	pid;
	int 	status;
	char *argv[10];

	if ((pid = fork()) < 0){
		std::cout << "fork error" << std::endl;
		exit(1);
	}
	else if (pid == 0){		//child
		if (bg_bit){		//change the process to a background group
			if ((setpgid(0,0))< 0){
				std::cout << "setpgid error" << std::endl;
				exit(1);
			}
		}
		for (unsigned int i = 0; i < positions.size(); i++){
			if (symbols[i] == "<") {
				close(0);
				open(file_names[i].c_str(),O_RDWR);
			}
			else if(symbols[i] == ">"){
				close(1);
				open(file_names[i].c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
			}
		}

		if (v[0] == "myls") process_myls(command);
		else{
			convert_vector_into_argv(command,argv);
			execvp(argv[0], argv);
		}
			std::cout << "execvp error" << std::endl;
			exit(1);
		}

	//parent
	if (bg_bit){
		if ((setpgid(pid,pid))< 0){
			std::cout << "setpgid error" << std::endl;
			exit(1);
		}
		waitpid(-1, &status, WNOHANG);
	}
	else if ((pid = waitpid(pid, &status, 0)) < 0){
		std::cout << "waitpid error" << std::endl;
		exit(1);
	}
}

/*
 * process simple command, who does contain "|", "<", ">", "&"
 */
void process_simple_command(std::vector<std::string> v, bool bg_bit){
	pid_t 	pid;
	int 	status;
	char *argv[10];

	if (bg_bit) v.pop_back();	//delete last element "&"

	if ((pid = fork()) < 0){
		std::cout << "fork error" << std::endl;
		exit(1);
	}
	else if (pid == 0){
		if (bg_bit){		//change the process to a background group
			if ((setpgid(0,0))< 0){
				std::cout << "setpgid error" << std::endl;
				exit(1);
			}
		}
		if (v[0] == "myls") process_myls(v);
		else{
			convert_vector_into_argv(v,argv);
			execvp(argv[0], argv);
		}
		std::cout << "could execute command: " << v[0] << std::endl;
		exit(1);
	}

	//parent
	if (bg_bit){
		if ((setpgid(pid,pid))< 0){
			std::cout << "setpgid error" << std::endl;
			exit(1);
		}
		waitpid(-1, &status, WNOHANG);
	}
	else if ((pid = waitpid(pid, &status, 0)) < 0){
		std::cout << "waitpid error" << std::endl;
		exit(1);
	}
}

/*
 * delete the quote mark in the command, such as use
 * "last | grep root" instead of "last | grep 'root'"
 */
void process_quote_mark(std::vector<std::string> &v){
	std::vector<std::string> v1 = v;
	std::string str;
	v.clear();

	for (unsigned int i = 0; i < v1.size(); i++){
		for (unsigned int j = 0; j < v1[i].size(); j++){
			if ((v1[i][j] != '\'') && (v1[i][j] != '\"')){
				str.push_back(v1[i][j]);
			}
		}
		v.push_back(str);
		str.clear();
	}
}

int main(void){
	std::string	oneline, token;
	std::vector<std::string> parsed_token;

	std::cout << "$ ";			//print prompt

	/*
	 * Parse the input command into a vector variable parsed_token
	 */

	while (getline(std::cin,oneline)) {
		if (oneline.empty()){
			std::cout << "$ ";
			continue;
		}
		std::stringstream ss(oneline);
		while (ss >> token){
			parsed_token.push_back(token);
		}

		process_quote_mark(parsed_token);//delete the quote mark in the command

		/*
		 * reap possibly existing background zombie
		 */
		waitpid(-1, NULL, WNOHANG);


		/*
		 * process four built-in commands "exit", "cd", "pwd". and "set"
		 */
		if ((parsed_token[0] == "exit") || (parsed_token[0] == "pwd")
				|| (parsed_token[0] == "cd") || (parsed_token[0] == "set")){
			process_built_in_commands(parsed_token);
		}

		else{

			int pipe_count = there_are_pipes(parsed_token);
			if (pipe_count > 0) {	//pipes will not coexist with "&" or "<,>"
				if (pipe_count == 1) process_command_with_one_pipe(parsed_token);
				else
					process_command_with_multiple_pipes(parsed_token,pipe_count);
			}
			else {
				bool bg_set = this_is_a_background_command(parsed_token);
				if (there_are_redirections(parsed_token)){
					process_redirections_command(parsed_token, bg_set);
				}
				else
					process_simple_command(parsed_token, bg_set);
			}
		}
		parsed_token.clear();
		oneline.clear();

		//waitpid(-1, NULL, WNOHANG);
		std::cout << "$ ";
	}
	exit(0);
}

