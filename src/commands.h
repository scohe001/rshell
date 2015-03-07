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

typedef int (*command)(char**);

int quit(char **argv);
int cd(char **argv);

std::map<std::string, command> COMMANDS;

int quit(char **argv) {
    exit(0);
    return -1;
}

int cd(char **argv) {
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
