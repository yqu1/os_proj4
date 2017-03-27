#include "tsqueue.h"
#include "utils.h"
#include <fstream>
#include "writeHtml.h"
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

//global variables for various queue, fqueue is the fetch queue, pqueue is the parse queue, results queue is the queue to hold search results
tsqueue<queueItemSite*> fqueue;
tsqueue<queueItemParse*> pqueue;
queue<result*> resultsqueue;
bool keepRunning = true;

//global variable for consumer and producer threads
pthread_t* cons;
pthread_t* pros;
//total number of fetches occurred
int nfetch = 1;

//mutex lock for writing to file
pthread_mutex_t write_lock;

parseConfig* P;
arg* args;
void* producer(void* a) {

        while(keepRunning) {
            //lock to atomically pop from fqueue
            pthread_mutex_lock(&fqueue.m_mutex);

            //while there is no job in fqueue, wait
            while(fqueue.size() == 0) {
                pthread_cond_wait(&fqueue.notEmpty, &fqueue.m_mutex);
                if(!keepRunning){
                        pthread_exit(0);
                }
            }
            //pop a new job to fetch from fqueue
            queueItemSite* itemSite = fqueue.remove();
            pthread_mutex_unlock(&fqueue.m_mutex);

            //perform curl on the site
            string site = itemSite->site;
            curlUtil c(site);
            c.get_curl();
            string data = c.get_data();
            delete itemSite;

            //lock to atomically push to pqueue
            pthread_mutex_lock(&pqueue.m_mutex);
            //push curl results to pqueue
            queueItemParse* itemParse = new queueItemParse;
            itemParse->data = data;
            itemParse->site = site;
            pqueue.add(itemParse);
            pthread_mutex_unlock(&pqueue.m_mutex);
            //wake up threads waiting for pqueue to be not empty
            pthread_cond_signal(&pqueue.notEmpty);

        }
        pthread_exit(0);
}

//helper function to check if file exists
/*bool file_exists( string filename ){
        ifstream f(filename.c_str());
        return f.good();
}
*/
void* consumer(void* a) {
    arg* args = (arg* ) a;
    while(keepRunning) {
        //lock to atomically pop from pqueue
        pthread_mutex_lock(&pqueue.m_mutex);
        //if there is no item in pqueue, wait
        while(pqueue.size() == 0) {
            pthread_cond_wait(&pqueue.notEmpty, &pqueue.m_mutex);
            if(!keepRunning){
                pthread_exit(0);
            }
        }
        //obtain an html to process from pqueue
        queueItemParse* itemParse = pqueue.remove();
        pthread_mutex_unlock(&pqueue.m_mutex);

        //lock to atomically write to file
        pthread_mutex_lock(&write_lock);
        //get search results for each search terms in the search strings and push the results into resultsqueue
        for(vector<string>::iterator it = args->searches.begin(); it != args->searches.end(); ++it) {
                        result* r = new result;
                        r->site = itemParse->site;
                        r->time = currentDateTime();
                        r->num = count(*it, itemParse->data);
                        r->term = *it;
                        resultsqueue.push(r);
        }
        delete itemParse;
        //write result to csv file
        ofstream outputFile;
        ostringstream ss;
        ss << nfetch;
        string filename = ss.str() + ".csv";
        if(!file_exists(filename)){
                outputFile.open(filename);
                outputFile << "Time,Phrase,Site,Count" << endl;
        }
        else outputFile.open(filename,ofstream::app);

        while(!resultsqueue.empty()) {
                result* r = resultsqueue.front();
                resultsqueue.pop();
                outputFile << r->time << "," << r->term << "," <<  r->site << "," << r->num << endl;
                delete r;
        }
        outputFile.close();
        pthread_mutex_unlock(&write_lock);

    }

    pthread_exit(0);
}

//handler for SIGHUP or control c
void my_handler(int s) {
    cout << "caught signal " << s << endl;
    keepRunning = false;
    //wake up all threads to allow them to exit
    while(P->NF > 0){
        pthread_cond_broadcast(&fqueue.notEmpty);
        P->NF-=1;
    }
    while(P->NP > 0){
        pthread_cond_broadcast(&pqueue.notEmpty);
        P->NP-=1;
    }

    //join all threads
    for(int i = 0; i < P->NF; i++) {
                if(pthread_join(pros[i], NULL)) {
			printf("Error joining threads! \n");
			exit(1);
		}	
    }
    for(int j = 0; j < P->NP; j++) {
                if(pthread_join(cons[j], NULL)) {
			printf("Error joining threads! \n");
			exit(1);
		}
    }
    //free pointers and destroy write_lock
    delete[] cons;
    delete[] pros;
    delete args;
    pthread_mutex_destroy(&write_lock);
    //create HTML summary
    writeHtml(nfetch);
    exit(1);
}

volatile sig_atomic_t flag = true;


//alarm handler
void handle_alarm(int sig) {
        flag = true;
        nfetch++;
}

static sigset_t signal_mask;

int main(int argc, char* argv[]) {
        //parse configuration
        P = new parseConfig(argc, argv);

        //install alarm handler
        signal(SIGALRM, handle_alarm);
        alarm(P->PF);
        struct sigaction sigIntHandler;
        struct sigaction sigHupHandler;
        //install handler to handle control c and SIGHUP
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGINT);
        sigaddset(&signal_mask, SIGHUP);
        pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
        sigIntHandler.sa_handler = my_handler;
        sigHupHandler.sa_handler = my_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigemptyset(&sigHupHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigHupHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);
        sigaction(SIGHUP, &sigHupHandler, NULL);
        cons = new pthread_t[P->NP];
        pros = new pthread_t[P->NF];

        //obtain search terms and links to fetch
        vector<string> searches = get_search_terms(P->SF);
        vector<string> links = get_fetch_links(P->SITEF);

        args = new arg;
        args->searches = searches;

        //create threads
        for(int i = 0; i < P->NF; i++) {
                if(pthread_create(&pros[i], NULL, producer, NULL)) {
			printf("Error creating thread!\n");
			exit(1);
		}

        }
        for(int j = 0; j < P->NP; j++) {                                          
                if(pthread_create(&cons[j], NULL, consumer, (void *)args)) {
			printf("Error creating thread!\n");
			exit(1);
		}
        }

        pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);


        while(1) {
                //flag is set to true on every alarm
                if(flag) {
                        //push fetch jobs to the fetch queue
                        for(vector<string>::iterator it = links.begin(); it != links.end(); ++it) {
                                queueItemSite* item = new queueItemSite;
                                item->site = *it;
                                fqueue.add(item);
                        }

                        //wake up all waiting fetch threads
                        pthread_cond_broadcast(&fqueue.notEmpty);

                        flag = false;
                        alarm(P->PF);
                }
        }
        return 0;
}
