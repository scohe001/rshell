#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

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
#define HIDDEN  "\033[47m"      /* White background */

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

void reg_print(string dir, const set<char>& flags) {
    DIR *dirp = opendir(dir.c_str());
    if(!dirp) {
        perror("Error opening directory");
        exit(1);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        if(!direntp) {
            perror((const char*)dirp);
            exit(1);
        }
        
        string s = direntp->d_name;
        string current_dir = (dir.at(dir.size()-1) == '/') ? dir+ s : dir + "/" + s;
        
        struct stat info;
        if(stat(current_dir.c_str(), &info) != 0) {
            perror((const char*)direntp->d_name);
            exit(1);
        }
        
        if(info.st_mode & S_IXUSR) {
            //cout << "found exexutable: ";
            cout << GREEN;
        }
        if(info.st_mode & S_IFDIR) {
            //cout << "found dir: ";
            cout << BOLD << BLUE;
        }
        
        if(direntp->d_name[0] == '.' && flags.find('a') == flags.end()) continue;
        else if(direntp->d_name[0] == '.') {
            cout << HIDDEN;
        }
        
        cout << direntp->d_name << RESET << endl;
    }
    closedir(dirp);
}

void long_print(string dir, const set<char>& flags) {
    
}

void recursive_print_(string dir, const set<char>& flags) {
    DIR *dirp = opendir(dir.c_str());
    if(!dirp) {
        perror("Error opening directory");
        exit(1);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        if(!direntp) {
            perror("Error reading directory");
            exit(1);
        }
        
        string s = direntp->d_name;
        string current_dir = (dir.at(dir.size()-1) == '/') ? dir+ s : dir + "/" + s;
        
        struct stat info;
        if(stat(current_dir.c_str(), &info) != 0) {
            perror("Error fetching file info");
            exit(1);
        }
        
        if((info.st_mode & S_IFDIR) && s != "." && s != "..") {
            if(direntp->d_name[0] == '.' && flags.find('a') == flags.end()) continue;
            cout << endl << current_dir << ":" << endl;
            if(flags.find('l') != flags.end()) {
                long_print(current_dir, flags);
            } else {
                reg_print(current_dir, flags);
            }
            recursive_print_(current_dir, flags);
        }
        
    }
    closedir(dirp);

}

int main(int argc, char *argv[]) {
    set<char> flags;
    vector<string> locations;
    parse_params(argc, argv, flags, locations);
    
    //If there's no location, assume current dir
    if(locations.empty()) locations.push_back(".");
    
    for(int x=0; x<locations.size(); x++) {
        if(flags.find('R') != flags.end()) {
            recursive_print_(locations.at(x), flags);
        } else if(flags.find('l') != flags.end()) {
            long_print(locations.at(x), flags);
        } else {
            reg_print(locations.at(x), flags);
        }
    }
    //char dirName[] = argv[1];
    
    return 0;
}
