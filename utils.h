#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <queue>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <time.h>
using namespace std;

//struct for an item in the resultsqueue
typedef struct {
        string site;
        int num;
        string term;
        string time;
} result;

//struct for arguments passed into the parse thread
typedef struct {
        vector<string> searches;
        int LOOP;
} arg;


vector<string> split(string str, char delimiter);
void trim(string& str);
vector<string> get_search_terms(string file);
vector<string> get_fetch_links(string file);
int count(string sub, string str);
const string currentDateTime();

//function to get current date and time
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}


//function to split string
vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}

//function to trim string
void trim(string& str) {
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

//a class for parsing configuration
class parseConfig {

        public:
                int PF, NF, NP;
                string SF, SITEF;
                parseConfig(int argc, char* argv[]);
};

//function to check if file read in from config exists

bool file_exists( string filename ){
        ifstream f(filename.c_str());
        return f.good();
}

parseConfig::parseConfig(int argc, char* argv[]) {
        PF = 180;
        NF = 1;
        NP = 1;
        SF = "Search.txt";
        SITEF = "Sites.txt";
        if(argc != 2) {
                cout << "Please enter name of configuration file" << endl;
                exit(0);
        }
        else if(!file_exists(argv[1])){
                cout << "Error No such config file exists!" << endl;
                exit(0);
        }
        else {
                fstream file(argv[1]);
                if(file.is_open()) {
                        string line;
                        while(getline(file, line) && !line.empty()) {
                                trim(line);
                                vector<string> parsed = split(line, '=');
                                string param = parsed[0];
                                string val = parsed[1];
                                if(param.compare("PERIOD_FETCH") == 0){
                                        PF = stoi(val);
                                        if(PF <= 0){
                                                printf("Error invalid period! Must be above 0\n");
                                                exit(1);
                                        }
                                }
                                else if(param.compare("NUM_FETCH") == 0){
                                        NF = stof(val);
                                        if(NF <= 0 || NF > 8){
                                                printf("Error invalid # of fetch threads\n");
                                                exit(1);
                                        }
                                }
                                else if(param.compare("NUM_PARSE") == 0){
                                        NP = stof(val);
                                        if(NP <= 0 || NF > 8){
                                                printf("Error invalid # of parse threads\n");
                                                exit(1);
                                        }
                                }
                                else if(param.compare("SEARCH_FILE") == 0){
                                        SF = val;
                                        if(!file_exists(SF)){
                                                printf("No such SEARCH_FILE exists\n");
                                                exit(1);
                                        }
                                }
                                else if(param.compare("SITE_FILE") == 0){
                                        SITEF = val;
                                        if(!file_exists(SITEF)){
                                                printf("No such SITE_FILE exists\n");
                                                exit(1);
                                        }
                                }
                                else {
                                        cout << "Unknown Parameter " << param << endl;
                                }
                        }
                        file.close();
                }
                else {
                        cout << "Cannot open " << argv[1] << endl;
                        exit(1);
                }
        }
}

//function for getting search terms
vector<string> get_search_terms(string file){
        fstream SF(file);
        if(SF.is_open()) {
                string line;
                vector<string > searches;
                while(getline(SF, line) && !line.empty()) {
                        trim(line);
			if(line.find(',') == string::npos && line.find('\r') == string::npos) { 
                        	searches.push_back(line);
			}
                }
                SF.close();
                return searches;
        }
        else {
                cout << "Cannot find SEARCH_FILE " << file << endl;
                exit(0);
        }
}

//function to get fetch links
vector<string> get_fetch_links(string file) {
        fstream SITEF(file);
        if(SITEF.is_open()) {
                string line;
                vector<string > links;
                while(getline(SITEF, line) && !line.empty()) {
                        trim(line);
                        if(line.find("https") == string::npos && line.find("http") != string::npos) {
                                links.push_back(line);
                        }
                }
                SITEF.close();
                return links;
        }
        else {
                cout << "Cannot find SITE_FILE " << file << endl;
                exit(0);
        }
}

//function to count occurrence of one string in another
int count(string sub, string str) {
        int count = 0;
        size_t nPos = str.find(sub, 0);
        while(nPos != string::npos) {
                count++;
                nPos = str.find(sub, nPos + 1);
        }
        return count;
}


#endif
