/*
 * myls.cpp
 *
 *  Created on: Oct 2, 2016
 *      Author: tao
 */
#include <sys/stat.h>
#include <stdlib.h>		//"exit" function defined here
#include <unistd.h>		//"getcwd" function defined here
#include <dirent.h>
#include <time.h>

#include <iostream>
#include <iomanip>		//realize formatted cout
#include <string>
#include <cstring>		//"strcmp" declared here

#define MAXPATHLENGTH 1024;
/*
 * get current work directory
 */
std::string get_current_directory(void){
	size_t 	path_size = MAXPATHLENGTH;
	char *path_buffer = new char[path_size];

	if (getcwd(path_buffer, path_size) == NULL){
		delete [] path_buffer;
		std::cout << "getcwd failed for myls" << std::endl;
			exit(0);
		}
		std::string str = path_buffer;
		delete [] path_buffer;
		return str;
}

/*
 * get the parent directory of input directory
 */
std::string get_parent_directory(std::string str){
	if (str.size() == 1){	//root directory, no father directory
		if (str[0] == '/')
			std::cout << "already root" << std::endl;
		else
			std::cout << "invalid directory name";
			exit(0);
	}
	int j;
	for (j = str.size() -1; j >= 0; j--){
		if (str[j] == '/') break;
	}
	std::string parent;
	for (int i = 0; i < j; i++){
		parent += str[i];
	}
	return parent;
}

/*
 * explain "." or ".." in the second parameter
 */
std::string make_full(std::string str){
	std::string cwd = get_current_directory();
	if ((str[0] == '.') && (str.size() == 1))
		return cwd;
	else if ((str[0] == '.') && (str[1] == '/')){
		for (int i = 1; i < str.size(); i++){
			cwd += str[i];
		}
		return cwd;
	}
	else if ((str[0] == '.') && (str[1] == '.') && (str.size() == 2)){
		std::string p1 = get_parent_directory(cwd);
		return p1;
	}
	else if ((str[0] == '.') && (str[1] == '.') && (str[2] =='/')){
		std::string p2 = get_parent_directory(cwd);
		for (int i = 2; i < str.size(); i++){
			p2 += str[i];
		}
		return p2;
	}
	else {
		std::cout << "invalid parameter for myls" << std::endl;
		exit(0);
	}
}

/*
 * print information of a single file
 */
void list_this_file(std::string str){
	struct stat buf;

	 if (stat(str.c_str(), &buf) <0) {
		 std::cout <<"stat " << str << " failed" << std::endl;
		 exit(0);
	  }
	  if (S_ISREG(buf.st_mode)) std::cout << "-";
	  else if (S_ISDIR(buf.st_mode)) std::cout << "d";
	  else if (S_ISCHR(buf.st_mode)) std::cout << "c";
	  else if (S_ISBLK(buf.st_mode)) std::cout << "b";
	  else if (S_ISFIFO(buf.st_mode)) std::cout << "f";
	  else if (S_ISLNK(buf.st_mode)) std::cout << "l";
	  else if (S_ISSOCK(buf.st_mode)) std::cout << "s";
	  else std::cout << "u";

	  if (S_IRUSR & buf.st_mode) std::cout << "r";
	  else std::cout << "-";
	  if (S_IWUSR & buf.st_mode) std::cout << "w";
	  else std::cout << "-";
	  if (S_ISUID & buf.st_mode){
		  if (S_IXUSR & buf.st_mode) std::cout << "s";
		  else std::cout << "S";
	  }
	  else {
		  if (S_IXUSR & buf.st_mode) std::cout << "x";
		  else std::cout << "-";
	  }

	  if (S_IRGRP & buf.st_mode) std::cout << "r";
	  else std::cout << "-";
	  if (S_IWGRP & buf.st_mode) std::cout << "w";
	  else std::cout << "-";
	  if (S_ISGID & buf.st_mode){
		  if (S_IXGRP & buf.st_mode) std::cout << "s";
		  else std::cout << "S";
	  }
	  else {
		  if (S_IXGRP & buf.st_mode) std::cout << "x";
		  else std::cout << "-";
	  }

	  if (S_IROTH & buf.st_mode) std::cout << "r";
	  else std::cout << "-";
	  if (S_IWOTH & buf.st_mode) std::cout << "w";
	  else std::cout << "-";
	  if (S_IXOTH & buf.st_mode) std::cout << "x";
	  else std::cout << "-";

	  std::cout << " " << std::setw(3) <<  buf.st_nlink << " "
			  << buf.st_uid << " " << buf.st_gid;
	  std::cout << std::setw(10) << buf.st_size;

	  std::string str2 =ctime(&buf.st_mtime),str1;
	  for (int i = 4; i < str2.size()-9; i++){
		  str1 = str1 + str2[i];
	  }
	  std::cout << " " << str1;
	  std::string str3;
	  int j;
	  for (j = str.size()-1; j >= 0; j--){
		  if (str[j] == '/') break;
	  }

	  for (int i = j+1; i < str.size(); i++){
		  str3 = str3 + str[i];
	  }
	  std::cout << " " << str3 << std::endl;
}

int main(int argc, char *argv[]){
	std::string second_para;		//second parameter(if there exists)

	/*
	 * first, explain "." or ".." in the second parameter
	 * if second parameter indicates a file(not a directory)
	 * list the file and return
	 */
	if (argc == 2){
		second_para = argv[1];
		if(second_para[0] != '/')
			second_para = make_full(second_para);
		if (second_para[second_para.size()-1] != '/'){
			struct stat buf;
			if (stat(second_para.c_str(), &buf) < 0){
				std::cout << "state " << second_para <<" failed" <<
						std::endl;
				exit(0);
			}
			if (!S_ISDIR(buf.st_mode)){
				list_this_file(second_para);
				exit(0);
			}
		}
	}

	/*
	 * if there only one parameter, or there two parameters but the
	 * second parameter indicates a directory
	 */
	std::string target_path;

	if (argc == 1){
		target_path = get_current_directory();
		//std::cout << "test: current directory: " <<target_path << std::endl;
	}
	else if(argc == 2){
		if ((second_para[second_para.size()-1] == '/') &&
				(second_para.size() > 1)){
			second_para[second_para.size()-1] = '\0';
		}
		target_path = second_para;
	}
	else{
		std::cout << "too many parameters for myls" << std::endl;
		exit(0);
	}

	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(target_path.c_str())) == NULL){
		std::cout << "can't open: " << target_path << std::endl;
		exit(0);
	}
	while ((dirp = readdir(dp)) != NULL){
		if(strcmp(".",dirp->d_name) == 0 || strcmp("..",dirp->d_name) == 0)
		                continue;
		std::string full_name = target_path + '/' + dirp->d_name;
		list_this_file(full_name);

	}
	closedir(dp);
	exit(0);
}



