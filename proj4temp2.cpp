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


tsqueue<queueItemSite*> fqueue;
tsqueue<queueItemParse*> pqueue;
queue<result*> resultsqueue;
bool keepRunning = true;

pthread_t* cons;
pthread_t* pros;

pthread_mutex_t write_lock;

parseConfig* P;
arg* args;
void* producer(void* a) {

        while(keepRunning) {
            pthread_mutex_lock(&fqueue.m_mutex);
            while(fqueue.size() == 0) {

                pthread_cond_wait(&fqueue.notEmpty, &fqueue.m_mutex);
                if(!keepRunning){
                        pthread_exit(0);
                }
            }
            queueItemSite* itemSite = fqueue.remove();
            pthread_mutex_unlock(&fqueue.m_mutex);

            string site = itemSite->site;
            curlUtil c(site);
            c.get_curl();
            string data = c.get_data();
            delete itemSite;

            pthread_mutex_lock(&pqueue.m_mutex);

            queueItemParse* itemParse = new queueItemParse;
            itemParse->data = data;
            itemParse->site = site;
            pqueue.add(itemParse);
            pthread_mutex_unlock(&pqueue.m_mutex);
            pthread_cond_signal(&pqueue.notEmpty);

        }
        pthread_exit(0);
}

void* consumer(void* a) {
    arg* args = (arg* ) a;
    while(keepRunning) {
        pthread_mutex_lock(&pqueue.m_mutex);
        while(pqueue.size() == 0) {
            pthread_cond_wait(&pqueue.notEmpty, &pqueue.m_mutex);
            if(!keepRunning){
                pthread_exit(0);
            }

        }
 
        queueItemParse* itemParse = pqueue.remove();
        pthread_mutex_unlock(&pqueue.m_mutex);

        pthread_mutex_lock(&write_lock);


//       cout << args->searches.size() << endl;
        for(vector<string>::iterator it = args->searches.begin(); it != args->searches.end(); ++it) {

                        result* r = new result;
                        r->site = itemParse->site;
			r->time = currentDateTime();
                        r->num = count(*it, itemParse->data);
                        r->term = *it;
                        resultsqueue.push(r);
        }
        delete itemParse;
        pthread_mutex_unlock(&write_lock);


    }
    pthread_exit(0);
}


void my_handler(int s) {
    cout << "caught signal " << s << endl;
    keepRunning = false;
    while(P->NF > 0){
        pthread_cond_broadcast(&fqueue.notEmpty);
        P->NF-=1;
    }
    while(P->NP > 0){
        pthread_cond_broadcast(&pqueue.notEmpty);
        P->NP-=1;
    }

    for(int i = 0; i < P->NF; i++) {
		if(pthread_join(pros[i], NULL)) {
                        printf("Error joining thread\n");
                        exit(1);
                }
    }
    for(int j = 0; j < P->NP; j++) {
		if(pthread_join(cons[j], NULL)) {
                        printf("Error joining thread\n");
                        exit(1);
                }
    }
    delete[] cons;
    delete[] pros;
    delete args;
    pthread_mutex_destroy(&write_lock);
    while(!resultsqueue.empty()) {
                result* r = resultsqueue.front();
                resultsqueue.pop();
                cout << "time: " << r->time << " site: " << r->site << " term: " << r->term << " num: " << r->num << endl;
                delete r;
    }
        exit(1);
}

volatile sig_atomic_t flag = false;

void handle_alarm(int sig) {
        flag = true;
}

static sigset_t signal_mask;

int main(int argc, char* argv[]) {

        P = new parseConfig(argc, argv);

        signal(SIGALRM, handle_alarm);
        alarm(P->PF);
        struct sigaction sigIntHandler;

        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGINT);
        pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
        sigIntHandler.sa_handler = my_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
    	cons = new pthread_t[P->NP];
    	pros = new pthread_t[P->NF];

    	vector<string> searches = get_search_terms(P->SF);
    	vector<string> links = get_fetch_links(P->SITEF);

    	args = new arg;
    	args->searches = searches;

    	for(int i = 0; i < P->NF; i++) {
		if(pthread_create(&pros[i], NULL, producer, NULL) != 0) {
			printf("Error creating thread  \n");
			return -1;			
		}

    	}
    	for(int j = 0; j < P->NP; j++) {                                              
        	if(pthread_create(&cons[j], NULL, consumer, (void *)args) != 0) {
			printf("Error creating thread \n");
			return -1;
		}
    	}

        pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);


        while(1) {
                if(flag) {
                        for(vector<string>::iterator it = links.begin(); it != links.end(); ++it) {
                                queueItemSite* item = new queueItemSite;
                                item->site = *it;
                                fqueue.add(item);
                        }

                                pthread_cond_broadcast(&fqueue.notEmpty);

                        flag = false;
                        alarm(P->PF);
                }
        }
        return 0;
}
