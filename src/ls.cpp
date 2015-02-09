#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>
#include <set>

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
#define BOLD    "\033[1m"       /* Bold */
#define BLINK   "\033[5m"       /* Blinks less than 150 per minute */

void parse_params(int argc, char *argv[], set<char>& flags, vector<string>& locs) {
    bool p_flags = true; //Check if we're in flags or files
    for(int x=1; x<argc; x++) {
        if(p_flags) {
            if(argv[x][0] == '-') { //If it starts with '-' it's a flag...
                for(int y=1; argv[x][y]; y++) { //Grab all of them if valid
                    if(argv[x][y] != 'l' && argv[x][y] != 'a' && argv[x][y] != 'R') {
                        cerr << "ls: illegal option -- " << argv[x][y] << endl;
                        cerr << "usage: ls [-Ral] [file ...]" <<endl;
                        exit(1);
                    }
                    flags.insert(argv[x][y]);
                }
            } else { //Toggle to taking paths
                p_flags = false;
            }
        }
        if(!p_flags) { //Need to check both so we don't miss the first
            locs.push_back(argv[x]);
        }
    }
}


int main(int argc, char *argv[]) {
    set<char> flags;
    vector<string> locations;
    parse_params(argc, argv, flags, locations);
    
    //If there's no location, assume current dir
    if(locations.empty()) locations.push_back(".");
    
    
    /*cout << "Flags: ";
    for (std::set<char>::iterator it = flags.begin(); it != flags.end(); ++it)
    {
        cout << *it; // Note the "*" here
    }
    cout << endl << "Params: " << endl;
    
    for(int x=0; x<locations.size(); x++) {
        cout << locations[x] << endl;
    }*/
    
    //char dirName[] = argv[1];
    DIR *dirp = opendir(argv[1]);
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        //struct stat s;
        //stat("test",&s);
        //std::cout << "num bytes: " << s.st_size << std::endl;
        cout << direntp->d_name << endl;  // use stat here to find attributes of file
    }
    closedir(dirp);
}
