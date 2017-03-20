#include "tsqueue.h"
#include <fstream>
#include <queue>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <curl/curl.h>
using namespace std;

typedef struct {
        string site;
        int num;
        string term;
} result;

typedef struct {
        vector<string> searches;
        int LOOP;
} arg;

string data;
tsqueue<queueItemSite*> pqueue;
tsqueue<queueItemParse*> cqueue;
queue<result*> resultsqueue;



void* producer(void* a);
void* consumer(void* a);
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
void get_curl(string site);
vector<string> split(string str, char delimiter);
void trim(string& str);
vector<string> get_search_terms(string file);
vector<string> get_fetch_links(string file);
int count(string sub, string str);

void* producer(void* a) {
	arg* args = (arg* )a;
	for(int i = 0; i < args->LOOP; i++) {
		pthread_mutex_lock(&pqueue.m_mutex);
		while(pqueue.size() == 0) {
			pthread_cond_wait(&pqueue.notEmpty, &pqueue.m_mutex);
		}
		queueItemSite* itemSite  = (queueItemSite* ) pqueue.remove();
		get_curl(itemSite->site);
		queueItemParse* itemParse = new queueItemParse;
		itemParse->site = itemSite->site;
		itemParse->data = data;
		delete itemSite;
		cqueue.add(itemParse);
		pthread_mutex_unlock(&pqueue.m_mutex);
		pthread_cond_signal(&cqueue.notEmpty);
	}
	pthread_exit(0);
}

void* consumer(void* a) {
	arg* args = (arg* )a;
	for(int i = 0; i < args->LOOP; i++) {
		pthread_mutex_lock(&cqueue.m_mutex);
		while(cqueue.size() == 0) {
			pthread_cond_wait(&cqueue.notEmpty, &cqueue.m_mutex);
		}

		queueItemParse* item = cqueue.remove();
		for(vector<string>::iterator it = args->searches.begin(); it != args->searches.end(); ++it) {
			result* r = new result;
			r->site = item->site;
			r->num = count(*it, item->data);
			r->term = *it;
			resultsqueue.push(r);			
		}
		delete item;
		pthread_mutex_unlock(&cqueue.m_mutex);
		pthread_cond_signal(&pqueue.notEmpty);
	}
	pthread_exit(0);
}

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
    data.clear();
    for (size_t c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb;
}

void get_curl(string site) {
	CURL* curl;
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, site.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
       	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
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

void my_handler(int s) {
	cout << "caught signal " << s << endl;
	exit(1);
}

volatile sig_atomic_t flag = false;

void handle_alarm(int sig) {
	flag = true;
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

int main(int argc, char* argv[]) {
	parseConfig P(argc, argv);
	signal(SIGALRM, handle_alarm);
	alarm(P.PF);
	struct sigaction sigIntHandler;

   	sigIntHandler.sa_handler = my_handler;
   	sigemptyset(&sigIntHandler.sa_mask);
   	sigIntHandler.sa_flags = 0;
	
   	sigaction(SIGINT, &sigIntHandler, NULL);
	
	while(1) {
		if(flag) {
			vector<string> searches = get_search_terms(P.SF);
        		vector<string> links = get_fetch_links(P.SITEF);
        		for(vector<string>::iterator it = links.begin(); it != links.end(); ++it) {
        			queueItemSite* item = new queueItemSite;
        			item->site = *it;
        			pqueue.add(item);
        		}
			arg* args = new arg;
			args->searches = searches;
			args->LOOP = pqueue.size();					 									    pthread_t pros[P.NF];
			pthread_t cons[P.NP];
			//cout << args->LOOP << endl;
			for(int i = 0; i < P.NF; i++) {
				pthread_create(&pros[i], NULL, producer, (void *)args);
			}
			for(int j = 0; j < P.NP; j++) {
				pthread_create(&cons[j], NULL, consumer, (void *)args);	
			}
			//pthread_cond_signal(&pqueue.notEmpty);
			for(int i = 0; i < P.NF; i++) {
				pthread_join(pros[i], NULL);
			}
			for(int j = 0; j < P.NP; j++) {
				pthread_join(cons[j], NULL);
			}
			delete args;
			//cout << "size: " << resultsqueue.size() << endl;
			while(!resultsqueue.empty()) {
				result* r = resultsqueue.front();
				resultsqueue.pop();
				cout << "site: " << r->site << "term: " << r->term << "num: " << r->num << endl;
				delete r;
			}	
			flag = false;
			alarm(P.PF);
		}
	}
	return 0;
}
