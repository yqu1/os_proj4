#include "tsqueue.h"
#include "utils.h"
#include <fstream>
#include <queue>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include "curlUtil.h"
#include <vector>
#include <sstream>

using namespace std;


tsqueue<queueItemSite*> pqueue;
tsqueue<queueItemParse*> cqueue;
queue<result*> resultsqueue;


void* producer(void* a) {
        arg* args = (arg* )a;
        for(int i = 0; i < args->LOOP; i++) {
                pthread_mutex_lock(&pqueue.m_mutex);
                while(pqueue.size() == 0) {
                        pthread_cond_wait(&pqueue.notEmpty, &pqueue.m_mutex);
                }
                queueItemSite* itemSite  = (queueItemSite* ) pqueue.remove();
                curlUtil c(itemSite->site);
		c.get_curl();
		string data = c.get_data();
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



void my_handler(int s) {
	cout << "caught signal " << s << endl;
	exit(1);
}

volatile sig_atomic_t flag = false;

void handle_alarm(int sig) {
	flag = true;
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
