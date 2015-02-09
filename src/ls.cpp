#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>


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

void reg_print_file(string path, string name, const set<char>& flags) {
    struct stat info;
    if(stat(path.c_str(), &info) != 0) {
        perror(path.c_str());
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
    
    if(name[0] == '.' && flags.find('a') == flags.end()) return;
    else if(name[0] == '.') {
        cout << HIDDEN;
    }
    
    cout << name << RESET << endl;
}

void long_print_file(string path, string name, const set<char>& flags) {
    struct stat info;
    if(stat(path.c_str(), &info) != 0) {
        perror(path.c_str());
        exit(1);
    }
    
    if(name[0] == '.' && flags.find('a') == flags.end()) return;
    
    cout << (S_ISDIR(info.st_mode) ? 'd' :
             S_ISLNK(info.st_mode) ? 'l' : '-')
    
        << ((info.st_mode & S_IRUSR) ? 'r' : '-')
        << ((info.st_mode & S_IWUSR) ? 'w' : '-')
        << ((info.st_mode & S_IXUSR) ? 'x' : '-')
        
        << ((info.st_mode & S_IRGRP) ? 'r' : '-')
        << ((info.st_mode & S_IWGRP) ? 'w' : '-')
        << ((info.st_mode & S_IXGRP) ? 'x' : '-')
        
        << ((info.st_mode & S_IROTH) ? 'r' : '-')
        << ((info.st_mode & S_IWOTH) ? 'w' : '-')
        << ((info.st_mode & S_IXOTH) ? 'x' : '-')
    
        << ' ' << info.st_nlink << ' ';
    
    //get owner
    struct passwd *user_info = getpwuid(info.st_uid);
    if(!user_info) {
        perror("Error fetching owner name");
        exit(1);
    }
    cout << user_info->pw_name << ' ';
    
    //get group
    struct group *group_info = getgrgid(info.st_gid);
    if(!group_info) {
        perror("Error fetching group name");
        exit(1);
    }
    cout << group_info->gr_name << ' ';
    
    cout << info.st_size << (info.st_size < 100 ? "\t\t" : "\t");
    struct tm *date = localtime (&info.st_mtime);
    if(!date) {
        perror("Error fetching date");
        exit(1);
    }
    char months[12][4] = {"Jan", "Feb", "Mar", "Apr",
                         "May", "Jun", "Jul", "Aug",
                         "Sep", "Oct", "Nov", "Dec"};
    cout << months[date->tm_mon] << ' ';
    cout << (date->tm_mday < 10 ? "  " : " ") << date->tm_mday << ' '
        << (date->tm_hour < 10 ? "0" : "") << date->tm_hour << ':'
        << (date->tm_min < 10 ? "0" : "") << date->tm_min << ' ';
    
    if(info.st_mode & S_IXUSR) {
        cout << GREEN;
    }
    if(info.st_mode & S_IFDIR) {
        cout << BOLD << BLUE;
    }
    
    if(name[0] == '.') {
        cout << HIDDEN;
    }
    
    cout << name << RESET << endl;

}

void print_dir(string dir, const set<char>& flags) {
    //In the case we have a file
    struct stat info;
    if(stat(dir.c_str(), &info) != 0) {
        perror(dir.c_str());
        exit(1);
    }
    
    if(!(info.st_mode & S_IFDIR)) {
        if(flags.find('l') == flags.end())
            reg_print_file(dir, dir, flags);
        else
            long_print_file(dir, dir, flags);
        return;
    }
    
    //In the case we have a directory
    DIR *dirp = opendir(dir.c_str());
    if(!dirp) {
        perror(dir.c_str());
        exit(1);
    }
    
    vector<string> files;
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        if(!direntp) {
            perror((const char*)dirp);
            exit(1);
        }
        
        files.push_back(direntp->d_name);
    }
    closedir(dirp);
    
    sort(files.begin(), files.end());
    
    for(unsigned x=0; x<files.size(); x++) {
        string current_dir = (dir.at(dir.size()-1) == '/') ?
                            dir + files.at(x) : dir + "/" + files.at(x);
        
        if(flags.find('l') == flags.end())
            reg_print_file(current_dir, files.at(x), flags);
        else
            long_print_file(current_dir, files.at(x), flags);
    }
}

void recursive_print_(string dir, const set<char>& flags) {
    //Make sure we don't have a file
    struct stat info;
    if(stat(dir.c_str(), &info) != 0) {
        perror(dir.c_str());
        exit(1);
    }
    if(!(info.st_mode & S_IFDIR)) return;
    
    //If we have a dir
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
            print_dir(current_dir, flags);
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
    
    for(unsigned x=0; x<locations.size(); x++) {
        if(locations.size() > 1 || flags.find('R') != flags.end()) {
            cout << endl << locations.at(x) << ":" << endl;
        }
        print_dir(locations.at(x), flags);
        if(flags.find('R') != flags.end()) {
            recursive_print_(locations.at(x), flags);
        }
    }
    
    return 0;
}
