
tsqueue
	mutex
	cond notEmpty
	queue

curl 
	data
	getcurl

fqueue pqueue

producer:
	lock(fqueue.mutex)
	while(fqueue.empty) {
		cond_wait(fqueue.notEmpty)
	}
	f = fqueue.pop()
	unlock(fqueue.mutex)

	c = new curl
	c.getcurl(f.site)

	lock(pqueue.mutex)
	queueItemParse q;
	q.data = c.data
	q.site = f.site
	pqueue.add(q)
	unlock(pqueue.mutex)
	cond_signal(pqueue.notEmpty)

consumer(arg):
	lock(pqueue.mutex)
	p = pqueue.pop()
	unlock(pqueue.mutex)
	r = count(arg->searches, p.data) (r contains search phrase - occurrence)
	lock(m)
	while(writing) {
		cond_wait(notWriting)
	}
	write(r, file)
	unlock(m)
	cond_signal(notWriting)
	
main:
	create producer/consumer threads
	while(true) {
		if(flag) {
			populate fqueue(fetch queue)
			bcast 
		}
	}
tsqueue
	mutex
	cond notEmpty
	queue

curl 
	data
	getcurl

fqueue pqueue

producer:
	lock(fqueue.mutex)
	while(fqueue.empty) {
		cond_wait(fqueue.notEmpty)
	}
	f = fqueue.pop()
	unlock(fqueue.mutex)

	c = new curl
	c.getcurl(f.site)

	lock(pqueue.mutex)
	queueItemParse q;
	q.data = c.data
	q.site = f.site
	pqueue.add(q)
	unlock(pqueue.mutex)
	cond_signal(pqueue.notEmpty)

consumer(arg):
	lock(pqueue.mutex)
	p = pqueue.pop()
	unlock(pqueue.mutex)
	r = count(arg->searches, p.data) (r contains search phrase - occurrence)
	lock(m)
	while(writing) {
		cond_wait(notWriting)
	}
	write(r, file)
	unlock(m)
	cond_signal(notWriting)
	
main:
	create producer/consumer threads
	while(true) {
		if(flag) {
			populate fqueue(fetch queue)
			bcast 
		}
	}
