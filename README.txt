README Project 4:
Nick Smith (nsmith9)
Yaoxian Qu (yqu1)

Building/Running:
        We have provided a makefile to make this easier for you, all you have to
        do is run make. And to run you just need to run the program as you
        normally would but you have to provide a filename to your config file
        (EX: ./site-tester config.txt) but dont worry if you specify an invalid filename
        our program will tell you! (Please run make clean to clear out any previously 
	generated csv files before running)

Structs:
        Result: used to store info after searching for keyword on site
        arg: used to pass the vector of searches to be performed
        parseConfig: used to parse the config file and store the results
        tsqueue: C++ wrapper for queue, includes mutex, condition variable, and
        underlying data structure is a list
        curlUtil: this is used to perform the curl, it saves the site and data
        that was pulled from that site.

First in main we take in the user input via argv and create a parseConfig object
which holds all of the parameters in the config file. After doing so we start the
alarm and then block the child threads from catching the SIGINT and SIGHUP signals
to guarantee that the main thread catches it. After populating our args data
structure with the searches to be made we create both our producer and consumer
threads after which we unblock the signals specified earlier so our main function
catches them. We then go into a while loop which notifies those wating on
fqueue( fetch queue aka sites queue ) to be populated. It has a flag that is set
by the alarm whenever a fetch needs to be made as well, and adds items into the
fqueue.

In our producer threads, our threads attempts to acquire the mutex and then waits
for there to be sites in the fqueue waiting to be processed. It then pops a new
job from the queue and performs the curl on the proper site and pushes the result
into the pqueue ( which the consumers will process ). It pushes a struct that we
defined which holds both the data from a certain site and the site that it was
curled from. After pushing the job onto the pqueue we signal those waiting for
jobs to appear on the pqueue.

In our consumer threads, they try and acquire the mutex and then if the queue is
empty they wait for work to show up on the queue. Once they have something to
process they pop from the pqueue and then unlock the mutex associated with that
queue, and then attempt to acquire another mutex which is related to writing to
the file (to ensure only one thread is writing to a file at one time). We then
push to a resultsqueue all of the relevant info to be written to file
(timestamp,keyword,site,num_instances on site). We then write that info to a file
and then unlock the mutex.

When the user hits control c or send a SIGHUP to the program, the program terminates
and cleans up the dynamically allocated memory, destroy mutex locks, join the threads
and etc.

Below is a pseudocode description of our logic flow:

tsqueue -- wrapper class for thread safe queue
	member:
		mutex
		cond_t notEmpty
		list

curl -- wrapper class for libcurl utility
	member:
		data 
		getcurl -- write chunk.memory to data


queueItemSite - struct for items in fetch queue
	member:
		site

queueItemParse - struct for items in parse queue
	member:
		data
		site

tsqueue<queueItemSite> fqueue -- fetch queue
tsqueue<queueItemParse> pqueue -- parse queue
write_lock
pthread_t consumer, producer

producer:
	while(keepRunning)
		lock(fqueue.mutex)
		while(fqueue.empty) {
			cond_wait(fqueue.notEmpty)
			if(!keepRunning)
				exit thread
		}
		queueItemSite f = fqueue.pop()
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
	while(keepRunning)
		lock(pqueue.mutex)
		while(pqueue.empty) {
			cond_wait(pqueue.notEmpty)
			if(!keepRunning)
				exit thread
		}
		queueItemParse p = pqueue.pop()
		unlock(pqueue.mutex)
		r = count(arg->searches, p.data) (r contains search phrase and corresponding occurrence)
		lock(write_lock)
		write(r, file)
		unlock(write_lock)
		cond_signal(notWriting)

signal_handler: (for handling control c and SIGHUP)
	keepRunning = false
	wake up all threads
	join threads
	//deal with dynamically allocated memory, destroy locks and such
	exit program

alarm_handler: 
	flag = true

main:
	Install Handlers
	create producer/consumer threads
	parse configuration file
	searches = get search terms from file
	links = get fetch links from file
	start producer and consumer threads
	while(true) {

		if(flag) { //flag set to true on alarm
			populate fqueue with fetch links
			broadcast(fqueue.notEmpty)
			flag = false
			alarm(SomePeriod)
		}
	}
