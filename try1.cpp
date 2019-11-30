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
            fprintf(stderr, "CD error ");
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
void pwd() {
    char add[1000];
    char *s;
    s = getcwd(add, 1000);
    printf("%s\n", s);
    fflush(stdout);
}

string getcmd(string &input) { //returns first word and truncates it
    unsigned long s = input.find(' ');
    string cmd = input.substr(0, s);
    unsigned long i = s;
    while ((i < input.size()) && input[i] == ' ') i++;

    input = input.substr(i, input.size());


    return cmd;
}

bool empty(string &s) {
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] != ' ')return false;
    }
    return true;
}

int exclVector(string &cmd, string &input, vector<char *> &arg,
               vector<string> &placeholder) {//формирует вектор аргументов(так как они пришли)
    placeholder.push_back(cmd);
    bool search = false;
    int y = 0;
    string copy = input;
    while (!input.empty()) {
        unsigned long l = input.find(' ');
        string s(input.substr(0, l));
        if ((s.find('*') == -1) && (s.find('?') == -1) && (s.find('/') == -1)) {
            placeholder.push_back(s);
        } else {
            copy = s;
            if (s.find('/') != -1) {
                if (s.find('*') == -1 && s.find('?') == -1) {
                    placeholder.push_back(s);
                } else {
                    search = true;
                    deleteSpacesFromStart(s);
                    if (s[0] != '/' && s[0] != '.') {
                        fprintf(stderr, "%s: Нет такого файла или каталога\n", s.c_str());
                    } else {

                        string path = s.substr(0, s.find('/') + 1);
                        s = s.substr(s.find('/') + 1, s.size());


                        vector<string> DirToCheck;
                        vector<string> DirToCheck2;
                        DirToCheck.push_back(path);
                        int k = 0;
                        while (s.find('/') != -1) {

                            string pattern = s.substr(0, s.find('/'));
                            while (!DirToCheck.empty()) {
                                path = DirToCheck.back();
                                DirToCheck.pop_back();
                                struct stat st;
                                if (stat(path.c_str(), &st) < 0) {
                                    fprintf(stderr, "%s: Нет такого файла или каталога\n", path.c_str());
                                    exit(1);
                                }
                                if (S_ISDIR(st.st_mode)) {

                                    DIR *d = opendir(path.c_str());
                                    if (d == NULL) {
                                        if (errno == EACCES) {
                                            //  fprintf(stderr, "%s: В доступе отказано\n", path.c_str());
                                        }
                                        if (errno == ENOTDIR) {
                                            fprintf(stderr, "%s: Не директория\n", path.c_str());
                                        }

                                        continue;
                                    }
                                    for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                                        if (string(de->d_name) == ".") continue;
                                        if (string(de->d_name) == "..") continue;
                                        if (!fnmatch(pattern.c_str(), de->d_name, 0) &&
                                            (de->d_type == DT_DIR)) {
                                            string g;
                                            if (k == 0) {
                                                g = path + de->d_name;
                                            } else { g = path + "/" + de->d_name; }

                                            DirToCheck2.push_back(g);
                                        }
                                    }

                                }

                            }
                            k++;
                            while (!DirToCheck2.empty()) {
                                DirToCheck.push_back(DirToCheck2.back());
                                DirToCheck2.pop_back();
                            }

                            s = s.substr(s.find('/') + 1, s.size());
                        }
                        //Обрабатываю файлы в последних директориях
                        while (!DirToCheck.empty()) {
                            path = DirToCheck.back();
                            DirToCheck.pop_back();

                            string pattern = s;//Возможно есть пробелы в конце

                            struct stat st;
                            if (stat(path.c_str(), &st) < 0) {
                                fprintf(stderr, "%s: Нет такого файла или каталога\n", path.c_str());
                                continue;
                            }
                            if (S_ISDIR(st.st_mode)) {

                                DIR *d = opendir(path.c_str());
                                if (d == NULL) {
                                    if (errno == EACCES) {
                                        //fprintf(stderr, "%s: В доступе отказано\n", path.c_str());
                                    }
                                    continue;
                                }
                                for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                                    if (string(de->d_name) == ".") continue;
                                    if (string(de->d_name) == "..") continue;
                                    if (!fnmatch(pattern.c_str(), de->d_name, 0)) {
                                        string g = path + "/" + de->d_name;
                                        placeholder.push_back(g);
                                        y = true;
                                    }
                                }
                            }
                            //нужно закидывать и файлы и папки


                        }


                    }
                }
            } else {
                DIR *d = opendir(".");
                deleteSpacesFromStart(s);

                for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {

                    if (string(de->d_name) == ".") continue;
                    if (string(de->d_name) == "..") continue;
                    if (!fnmatch(s.c_str(), de->d_name, 0)) {
                        string g = de->d_name;
                        placeholder.push_back(g);
                        y = true;
                    }
                }

            }
        }


        unsigned long i = l;
        while ((i < input.size()) && input[i] == ' ') i++;


        input = input.substr(i, input.size());

    }

    if (empty(copy)) {
        for (int i = 0; i < placeholder.size(); ++i) {
            arg.push_back((char *) placeholder[i].c_str());
        }

    } else {
        if (search && !y) {
            fprintf(stderr, "%s: Нет такого файла или каталога\n", copy.c_str());
        } else {
            for (int i = 0; i < placeholder.size(); ++i) {
                arg.push_back((char *) placeholder[i].c_str());
            }
        }
    }

    arg.push_back(NULL);
    return 1;
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

