#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

using namespace std;

//the following are UBUNTU/LINUX ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

//The following three functions are taken from the first answer on this question
//http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// trim from start
string &ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    return s;
}

// trim from end
string &rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}

// trim from both ends
string &trim(string &s) {
    return ltrim(rtrim(s));
}

//Run a command and do error checking
//Return the pid of the fork running the command
int run_cmd_(char *cmd, char **argv, int in, int out) {
//int run_cmd_(char *cmd, char **argv) {
    if(strcmp(cmd, "exit") == 0) exit(0);
    int pid = fork();
    if(pid == -1) {
        perror("fork fail");
        return -1;
    } else if(pid == 0) {
        if(in != -1) {
            if(close(0) != 0) {
                perror("Error closing stdin");
                exit(-1);
            }
            if(dup(in) != 0) {
                perror("Error redirecting input");
                exit(-1);
            }
        }
        if(out != -1) {
            if(close(1) != 0) {
                perror("Error closing stdout");
                exit(-1);
            }
            if(dup(out) != 1) {
                perror("Error redirecting output");
                exit(-1);
            }
        }
        if(execvp(cmd,argv) != 0) {
            char *err = new char[7 + strlen(cmd) + 1];
            strcpy(err, "-bqsh: ");
            strcat(err, cmd);
            perror(err);
            delete [] err;
        }
        return -1;
    } else {
        if(in != -1 && close(in) == -1) {
            perror("Error closing file");
        }
        if(out != -1 && close(out) == -1) {
            perror("Error closing file");
        }
        return pid;
    }
}

struct Command {
    string cmd;
    int in;
    int out;
    
    Command(): in(-1), out(-1) { }
    Command(string line): cmd(line), in(-1), out(-1) { }
    Command(string line, int in): cmd(line), in(in), out(-1) { }
    Command(string line, int in, int out): cmd(line), in(in), out(out) { }
};

int run_cmds(vector<Command> cmds) {
    vector<int> pids;
    for(unsigned i=0; i<cmds.size(); i++) {
        istringstream stream(cmds.at(i).cmd);
        vector<string> cmds_vec;
        string cmd;
        while(stream >> cmd) {
            cmds_vec.push_back(cmd);
        }
        
        char **argv = new char*[cmds_vec.size() + 1];
        for(unsigned x=0; x<cmds_vec.size(); x++) {
            argv[x] = new char[cmds_vec.at(x).size() + 1];
            strcpy(argv[x], cmds_vec.at(x).c_str());
        }
        argv[cmds_vec.size()] = NULL;
        
        pids.push_back(run_cmd_(argv[0], argv, cmds.at(i).in, cmds.at(i).out));
        
        for(unsigned x=0; x<=cmds_vec.size(); x++) {
            delete []argv[x];
        }
        delete []argv;
    }
    
    for(unsigned x=0; x<pids.size(); x++) {
        if(pids.at(x) == -1) return -1;
        int status;
        if(waitpid(pids.at(x), &status, WUNTRACED) == -1) {
            perror("Error running command");
            return -1;
        }
        if(status != 0) return status;
    }
    return 0;
}

int parse_redirection(string line) {
    vector<string> parsed;
    int last_found = 0;
    
    //Parse parse parse
    for(unsigned i=0; i < line.size(); i++) {
        if(line.at(i) == '<' || line.at(i) == '>' || line.at(i) == '|') {
            if(i-last_found != 0) { //Make sure it's not one after another
                //Push what we found into parsed
                parsed.push_back(line.substr(last_found, i-last_found));
                //Trim the white space and make sure it's not nothing
                trim(parsed.at(parsed.size()-1));
                if(parsed.at(parsed.size()-1) == "") parsed.pop_back();
            }
            //If we have '>>' operator
            if(line.at(i) == '>' && i+1 < line.size() && line.at(i+1) == '>') {
                parsed.push_back(">>");
                i++;
            } else {
                parsed.push_back(string(1, line.at(i)));
            }
            last_found = i+1;
        }
    }
    parsed.push_back(line.substr(last_found, line.size() - last_found));
    trim(parsed.at(parsed.size()-1));
    if(parsed.at(parsed.size()-1) == "") parsed.pop_back();
    
    //Output parsed vector for debugging
    //for(int x=0; x<parsed.size(); x++) {
    //    cout << parsed.at(x) << endl;
    //}
    
    //string cmd;
    //int in;
    //int out;
    
    vector<Command> cmds(parsed.size(), Command());
    //Hookup pipes/outputs
    for(unsigned x=0; x<parsed.size(); x++) {
        if(parsed.at(x) == "<") {
            if(x == 0 || x == parsed.size()-1) {
                cerr << "'<' operator requires two arguments, only 1 given" << endl;
                return -1;
            }
            
            //Create the file handler and the command to add to cmds
            int fd = open(parsed.at(x+1).c_str(), O_RDONLY);
            if(fd == -1) {
                perror(parsed.at(x+1).c_str());
                return -1;
            }
            cmds.at(x-1).cmd = parsed.at(x-1);
            cmds.at(x-1).in = fd;
            
            //Cleanup commands and parsed
            parsed.erase(parsed.begin()+x, parsed.begin()+x+2);
            cmds.erase(cmds.begin()+x, cmds.begin()+x+2);
            x--;
        } else if(parsed.at(x) == ">" || parsed.at(x) == ">>") {
            if(x == 0 || x == parsed.size()-1) {
                cerr << "'" << parsed.at(x) << "' operator requires two arguments, only 1 given" << endl;
                return -1;
            }
            
            //Create the file handler and the command to add to cmds
            int fd;
            if(parsed.at(x) == ">") {
                fd = open(parsed.at(x+1).c_str(), O_WRONLY | O_TRUNC | O_CREAT,
                              S_IRUSR | S_IWUSR);
            } else { //if(parsed.at(x) == ">>") {
                fd = open(parsed.at(x+1).c_str(), O_WRONLY | O_APPEND | O_CREAT,
                         S_IRUSR | S_IWUSR);
            }
            if(fd == -1) {
                perror(parsed.at(x+1).c_str());
                return -1;
            }
            cmds.at(x-1).cmd = parsed.at(x-1);
            cmds.at(x-1).out = fd;
            
            //Cleanup commands and parsed
            parsed.erase(parsed.begin()+x, parsed.begin()+x+2);
            cmds.erase(cmds.begin()+x, cmds.begin()+x+2);
            x--;
        } else if(parsed.at(x) == "|") {
            if(x == 0 || x == parsed.size()-1) {
                cerr << "'|' operator requires two arguments, only 1 given" << endl;
                return -1;
            }
            
            //Create the pipe and update/add commands
            int fd[2];
            if(pipe(fd) == -1) {
                perror("Pipe fail");
                return -1;
            }
            
            cmds.at(x-1).cmd = parsed.at(x-1);
            cmds.at(x-1).out = fd[1];
            
            cmds.at(x+1).cmd = parsed.at(x+1);
            cmds.at(x+1).in = fd[0];
        }
    }
    
    if(cmds.size() == 1) { //NO REDIRECTION
        cmds.at(0).cmd = parsed.at(0);
    }
    
    //Clean up commands
    for(unsigned x=0; x<cmds.size(); x++) {
        if(cmds.at(x).cmd == "") {
            cmds.erase(cmds.begin() + x);
            x--;
        }
    }
    
    //Output commands and they're pipes for debugging
    //for(int x=0; x<cmds.size(); x++) {
    //    Command c = cmds[x];
    //    cout << "cmd: " << c.cmd << "\tin: " << c.in << "\tout: " << c.out << endl;
    //}
    //return -1;
    return run_cmds(cmds);
}

//Recursively parse and check the conditionals
//This is where the magic happens
int parse_conditional_(string cond, const vector<string>& cmds, int num) {
    int result;
    if(num == 0) {
        return parse_redirection(cmds.at(0));
    } else if(num == 2) {
        result = parse_redirection(cmds.at(num-2));
    } else {
        result = parse_conditional_(cmds.at(num-3), cmds, num-2);
    }
    if(result == 0 && (cond == "||")) return true;
    if(result != 0 && (cond == "&&")) return false;
    return parse_redirection(cmds.at(num));
}

//Put conditionals into array to be parsed recursively
void parse_conditional(char cmds[]) {
    if(cmds == NULL) return;
    
    vector<string> parsed;
    string s_cmds = cmds;
    
    //Check if they lead with a conditional...
    if(s_cmds.substr(0, 2) == "&&" || s_cmds.substr(0, 2) == "||") {
        cerr << "Bad conditionals" << endl;
        return;
    }
    
    //Parse parse parse
    char prev = '\0';
    int last_found = 0;
    for(unsigned i=0; i < s_cmds.size(); i++) {
        if(s_cmds.at(i) == prev && (prev == '&' || prev == '|')) {
            parsed.push_back(s_cmds.substr(last_found, i-last_found-1));
            trim(parsed.at(parsed.size()-1));
            if(parsed.at(parsed.size()-1) == "") parsed.pop_back();
            parsed.push_back(string(2, prev));
            last_found = i + 1;
            prev = '\0';
        } else {
            prev = s_cmds.at(i);
        }
    }
    parsed.push_back(s_cmds.substr(last_found, s_cmds.size() - last_found));
    trim(parsed.at(parsed.size()-1));
    if(parsed.at(parsed.size()-1) == "") parsed.pop_back();
    
    //Output args for debugging
    //for(int x=0; x<parsed.size(); x++) {
    //    cout << parsed[x] << endl;
    //}
    
    if((parsed.size() % 2) == 0) {
        cerr << "Bad conditionals" << endl;
        return;
    }
    
    parse_conditional_((parsed.size() == 1) ? "" : parsed.at(parsed.size()-2),
                       parsed, parsed.size()-1);
}

void parse_semi(char cmds[]) {
    if(cmds == NULL) return;
    char *space_test = new char[strlen(cmds) + 1];
    strcpy(space_test, cmds);
    if(strtok(space_test, " \t\r\n") == NULL) {delete [] space_test; return;}
    delete [] space_test;
    
    char *parsed = strtok(cmds, ";");
    while(parsed != NULL) {
        string space_test = parsed;
        trim(space_test);
        if(space_test != "") parse_conditional(parsed);
        parsed = strtok(NULL, ";");
    }
}

int main(int argc, char *argv[]) {
    //int run_cmd_(char *cmd, char **argv, int in, int out) {
    /*int fd[2];
    if(-1 == pipe(fd)) {
        perror("Pipe fail");
        return 1;
    };
    run_cmd_("ls", argv, -1, fd[0]);
    run_cmd_("tail", argv, fd[1], -1);*/
    while(true) {
        //Grab Username
        char uname[64] = "";
        if(getlogin_r(uname, sizeof(uname)-1) != 0) {
            perror("Error fetching Username");
            strcpy(uname, "???");
        }
        
        //Grab hostname
        char hostname[64] = "";
        if(gethostname(hostname, sizeof(hostname)-1) != 0) {
            perror("Error fetching Hostname");
            strcpy(hostname, "???");
        }
        
        //Grab a line and convert it to a C-String
        string cmds;
        cout << GREEN << uname << "@" << YELLOW << hostname << RESET << "$ ";
        getline(cin, cmds);
        char *cmd = new char[cmds.size() + 1];
        strcpy(cmd, cmds.c_str());
        
        //Cut out a comment if there is one and pass to the parsers
        if(cmds.size() == 0 || cmds.at(0) == '#') {delete [] cmd; continue;}
        parse_semi(strtok(cmd, "#"));
        delete [] cmd;
    }
    
    return 0;
}
