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
bool exit_t = false;
pthread_t* cons;
pthread_t* pros;

pthread_mutex_t write_lock;
pthread_cond_t notWriting;

parseConfig* P;
arg* args;
void* producer(void* a) {
    
        while(keepRunning) {
            pthread_mutex_lock(&fqueue.m_mutex);
            while(fqueue.size() == 0 && !exit_t) {
		//	cout << "fart" << endl;
                pthread_cond_wait(&fqueue.notEmpty, &fqueue.m_mutex);
		//cout << exit_t << endl;
            }
	    if(exit_t) {
		//cout << "h" << endl;
		pthread_exit(0);
		//cout << "h" << endl;
	   }
            queueItemSite* itemSite = fqueue.remove();
            pthread_mutex_unlock(&fqueue.m_mutex);
            
	    string site = itemSite->site;
            curlUtil c(site);
            c.get_curl();
            string data = c.get_data();
	//    cout << "fart" << endl;
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
        while(pqueue.size() == 0 && !exit_t) {
            pthread_cond_wait(&pqueue.notEmpty, &pqueue.m_mutex);
	   // cout << exit_t << endl;
        }
	if(exit_t) pthread_exit(0);
        queueItemParse* itemParse = pqueue.remove();
        pthread_mutex_unlock(&pqueue.m_mutex);

        pthread_mutex_lock(&write_lock);
       	//while(writing) {
        //    pthread_cond_wait(&notWriting, &write_lock);
        //}	//writing = true;
        //	

	//cout << args->searches.size() << endl;
        for(vector<string>::iterator it = args->searches.begin(); it != args->searches.end(); ++it) {
                        
			result* r = new result;
                        r->site = itemParse->site;
			
                        r->num = count(*it, itemParse->data);
                        r->term = *it;
                        resultsqueue.push(r);
        }
        delete itemParse;
        pthread_mutex_unlock(&write_lock);
	//writing = false;
        //pthread_cond_signal(&notWriting);

    }
    pthread_exit(0);
}


void my_handler(int s) {
   cout << "caught signal " << s << endl;
/* while(!resultsqueue.empty()) {
                result* r = resultsqueue.front();
                resultsqueue.pop();
                cout << "site: " << r->site << " term: " << r->term << " num: " << r->num << endl;
                delete r;
   } */
    exit_t = true;
    keepRunning = false;
    for(int i = 0; i < P->NF; i++) {
    	pthread_cond_broadcast(&fqueue.notEmpty);
    }
    for(int i = 0; i < P->NP; i++) {
    	pthread_cond_broadcast(&pqueue.notEmpty);
    }
    //pthread_cond_broadcast(&pqueue.notEmpty);
    //pthread_cond_broadcast(&fqueue.notEmpty);
    
    for(int i = 0; i < P->NF; i++) {
                pthread_join(pros[i], NULL);
    }
    for(int j = 0; j < P->NP; j++) {
                pthread_join(cons[j], NULL);
    }
    delete[] cons;
    delete[] pros;
    delete args;
    pthread_mutex_destroy(&write_lock);
    pthread_cond_destroy(&notWriting);
    while(!resultsqueue.empty()) {
                result* r = resultsqueue.front();
                resultsqueue.pop();
                cout << "site: " << r->site << " term: " << r->term << " num: " << r->num << endl;
                delete r;
    }   
	exit(1);
}

volatile sig_atomic_t flag = false;

void handle_alarm(int sig) {
	flag = true;
}


int main(int argc, char* argv[]) {
	P = new parseConfig(argc, argv);
	
	signal(SIGALRM, handle_alarm);
	alarm(P->PF);
	struct sigaction sigIntHandler;

   	sigIntHandler.sa_handler = my_handler;
   	sigemptyset(&sigIntHandler.sa_mask);
   	sigIntHandler.sa_flags = 0;
	
   	sigaction(SIGINT, &sigIntHandler, NULL);
    cons = new pthread_t[P->NP];
    pros = new pthread_t[P->NF];
                        /*cout << args->LOOP << endl;*/ 
	//cout << P->SF << endl;
    vector<string> searches = get_search_terms(P->SF);
    vector<string> links = get_fetch_links(P->SITEF);
	//cout << searches.size() << endl;
    args = new arg;
    args->searches = searches;
//	cout << "fuck" << endl;
    for(int i = 0; i < P->NF; i++) {
    	pthread_create(&pros[i], NULL, producer, NULL);
	//cout << "lu" << endl;
    }
    for(int j = 0; j < P->NP; j++) {                                                                                               
        pthread_create(&cons[j], NULL, consumer, (void *)args);
    }

	
                                                                 
	while(1) {
		if(flag) {
        		for(vector<string>::iterator it = links.begin(); it != links.end(); ++it) {
        			queueItemSite* item = new queueItemSite;
        			item->site = *it;
        			fqueue.add(item);
        		}
							
				pthread_cond_broadcast(&fqueue.notEmpty);				
			/*pthread_cond_signal(&pqueue.notEmpty);*/
			/*cout << "size: " << resultsqueue.size() << endl;*/
			flag = false;
			alarm(P->PF);
		}
	}
	return 0;
}

