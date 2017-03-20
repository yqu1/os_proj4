#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <pthread.h>
#include <list>
#include <string>
using namespace std;

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

		pthread_mutex_t m_mutex;
		pthread_cond_t notEmpty;

		tsqueue() {
			pthread_mutex_init(&m_mutex, NULL);
			pthread_cond_init(&notEmpty, NULL);
		}

		~tsqueue() {
			pthread_mutex_destroy(&m_mutex);
			pthread_cond_destroy(&notEmpty);
		}

		void add(T item) {
			m_queue.push_back(item);
		}

		T remove() {
			T item = m_queue.front();
			m_queue.pop_front();
			return item;
		}

		int size() {
			int size = m_queue.size();
			return size;
		}
};

#endif
