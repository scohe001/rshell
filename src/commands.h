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
    if(out != -1 && close(out) == -1) perror("Error closing file");
    if(in != -1) {
        std::string path;
        char buf[64];
        int bytes_read;
        while((bytes_read = read(in, buf, 64)) == 64) {
            path += std::string(buf);
        }
        if(bytes_read == -1) perror("Error reading from file");
        if(close(in) == -1) perror("Error closing file");
        
        if(chdir(path.c_str()) == -1) {
            perror("cd");
            return -1;
        }
        
        if(close(in) == -1) perror("Error closing file");
    }
    if(!argv[1]) {
        std::cout << "cd: No directory given" << std::endl;
        return -1;
    }
    //std::cout << "I WORK" << std::endl;
    //int chdir(const char *path);
    if(chdir(argv[1]) == -1) {
        perror("cd");
        return -1;
    }
    return 0;
}

/////////////////////////// SIGNAL CATCHERS ////////////////////////////////////



#endif
