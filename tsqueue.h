#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <pthread.h>
#include <list>
#include <string>
using namespace std;

//a wrapper around std::list to implement a thread safe queue
typedef struct {
	string site;
} queueItemSite;

typedef struct {
	string site;
	string data;
} queueItemParse;


template<typename T> class tsqueue {

	list<T> m_queue;

	public:
		//mutex lock for the queue
		pthread_mutex_t m_mutex;
		//condition variable to signal that the queue is not empty
		pthread_cond_t notEmpty;
		
		//constructor, initialize mutex and condition variables
		tsqueue() {
			pthread_mutex_init(&m_mutex, NULL);
			pthread_cond_init(&notEmpty, NULL);
		}

		//destructor, destroy mutex and condition variables
		~tsqueue() {
			pthread_mutex_destroy(&m_mutex);
			pthread_cond_destroy(&notEmpty);
		}
		
		//add to queue
		void add(T item) {
			m_queue.push_back(item);
		}

		//pop from queue
		T remove() {
			T item = m_queue.front();
			m_queue.pop_front();
			return item;
		}

		//obtain size of queue
		int size() {
			int size = m_queue.size();
			return size;
		}
};

#endif
