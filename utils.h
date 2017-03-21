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


typedef struct {
        string site;
        int num;
        string term;
	string time;
} result;

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

const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
         // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    
    return buf;
}


vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}

void trim(string& str) {
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

class parseConfig {

        public:
                int PF, NF, NP;
                string SF, SITEF;
                parseConfig(int argc, char* argv[]);
};
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
        else {
                fstream file(argv[1]);
                if(file.is_open()) {
                        string line;
                        while(getline(file, line) && !line.empty()) {
                                trim(line);
                                vector<string> parsed = split(line, '=');
                                string param = parsed[0];
                                string val = parsed[1];
                                if(param.compare("PERIOD_FETCH") == 0) PF = stoi(val);
                                else if(param.compare("NUM_FETCH") == 0) NF = stof(val);
                                else if(param.compare("NUM_PARSE") == 0) NP = stof(val);
                                else if(param.compare("SEARCH_FILE") == 0) SF = val;
                                else if(param.compare("SITE_FILE") == 0) SITEF = val;
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
vector<string> get_search_terms(string file){
        fstream SF(file);
        if(SF.is_open()) {
                string line;
                vector<string > searches;
                while(getline(SF, line) && !line.empty()) {
                        trim(line);
                        searches.push_back(line);
                }
                SF.close();
                return searches;
        }
        else {
                cout << "Cannot find SEARCH_FILE " << file << endl;
                exit(0);
        }
}

vector<string> get_fetch_links(string file) {
        fstream SITEF(file);
        if(SITEF.is_open()) {
                string line;
                vector<string > links;
                while(getline(SITEF, line) && !line.empty()) {
                        trim(line);
                        if(line.find("https") == string::npos) {
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
