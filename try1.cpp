#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <vector>
#include <fnmatch.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

void deleteSpacesFromStart(string &s) {
    int i = 0;
    while (s[i] == ' ') i++;
    s = s.substr(i, s.size());
}
void deleteSpacesFromEnd(string &s) {
    int i = s.size() - 1;
    while (s[i] == ' ') i--;
    s = s.substr(0, i + 1);
}

void cd(string &input) {
	deleteSpacesFromStart(input);
    deleteSpacesFromEnd(input);
    if (input.empty()) {
        char *home = getenv("HOME");
        struct stat st2;
        if (stat(home, &st2) < 0) {
            fprintf(stderr, "CD error ");
        }
        if (S_ISDIR(st2.st_mode)) {
        	int ch = chdir(home);
        } else {
            fprintf(stderr, "%s:Home didn't set properly\n", home);
        }
    } else {
        struct stat st;
        if (stat(input.c_str(), &st) < 0) {
            fprintf(stderr, "CD error");
        }

        if (S_ISDIR(st.st_mode)) {
            int ch = chdir(input.c_str());
            if (ch == -1) {
            	fprintf(stderr, "Error Code%d\n", errno);
            }
        } else {
            fprintf(stderr, "%s:Not a directory\n", input.c_str());
        }
    }
}

int main(int argc, char **argv, char **envp) {
    uid_t u = getuid();
    char q;
    if (u == 0) {
        q = '!';
    } else {
        q = '>';
    }
    while (1) {
        char add[1000];
        char *s;
        s = getcwd(add, 1000);
        printf("%s%c", s, q);
        fflush(stdout); //Чтобы выводилось сразу же в момент печати
        string input;
        	if (!getline(cin, input).fail()) {
        		deleteSpacesFromStart(input);
        		deleteSpacesFromEnd(input);
                input = input + " ";
                string cmd = input.substr(0, input.find(' '));
                if (cmd == "cd") {
                	string dir = input.substr(input.find(' '), input.size());
                    cd(dir);
                } else {
 //x               	//...
                }
        	} else {
                printf("\n");
                exit(1);
            }
    }
         return 0;
}

