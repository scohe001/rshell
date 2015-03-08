//
//  commands.h
//  
//  Create all of our custom commands here
//  to be put into the COMMANDS map in main
//
//  Also create signal interrupt functions
//  to be applied in main

#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <map>
#include <string>
#include <iostream>
#include <unistd.h>
#include <errno.h>

/////////////////////////// MISC ////////////////////////////////////

//The following three functions are taken from the first answer on this question
//http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// trim from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

//The following function is take from the second answer on this question:
//http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

/////////////////////////// CUSTOM COMMANDS ////////////////////////////////////

typedef int (*command)(char**, int, int);

int quit(char **argv);
int cd(char **argv);

std::map<std::string, command> COMMANDS;

int quit(char **argv, int in, int out) {
    exit(0);
    return -1;
}

int cd(char **argv, int in, int out) {
    //If we have someting piped to us
    if(out != -1 && close(out) == -1) perror("Error closing file");
    if(in != -1) {
        std::string path;
        char buf[64];
        int bytes_read;
        while((bytes_read = read(in, buf, 64)) == 64) {
            //buf[bytes_read] = '\0';
            path += std::string(buf);
        }
        buf[bytes_read] = 0;
        path += std::string(buf);
        trim(path);
        if(bytes_read == -1) perror("Error reading from file");
        if(close(in) == -1) perror("Error closing file");
        
        if(chdir(path.c_str()) == -1) {
            perror(path.c_str());
            return -1;
        }
        return 0;
    }
    
    //Otherwise look at argv
    if(!argv[1]) {
        std::cout << "cd: No directory given" << std::endl;
        return -1;
    }
    
    if(chdir(argv[1]) == -1) {
        perror(argv[1]);
        return -1;
    }
    return 0;
}

#endif
