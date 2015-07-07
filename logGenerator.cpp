/*******************************************************************
File name		:	logGenerator.cpp
Created		:	2015-05-08
Usage		:	Implementation of log file generator
Owner		:	Yuanda Xu
*******************************************************************/

#include "logGenerator.h"
#include "comm.h"



using namespace std;


/*******************************************************************
Function		:	genLogThread
Created		:	2015-05-08
Usage		:	thread process of log generator, each thread response for one day's log generating
input		:	thread parameters
return		:	none
*******************************************************************/
void* genLogThread(void* para)
{
	//parse thread parameters
	TP* pPara = (TP*) para;
	STRING filePath = pPara->path;			//should have path to eable writing files to assigned directory
	time_t startStamp = pPara->startStamp;	//get start time stamp of current thread, could be 00:00 or assigned time (first day)
	time_t endStamp = pPara->endStamp;	//get end time stamp of current thread, could be 23:59 or assigned time (last day)

	//generate log from start timestamp
	time_t timestamp=startStamp;

	//cout<<"_mystart time = "<<timeStamp2str(startStamp)<<",,,, my end time = "<<timeStamp2str(endStamp)<<endl;
	
	//construct filename with full path
	STRING start = timeStamp2str(startStamp);
	STRING filename = filePath + "/"+ start.substr(0, 10) + ".log";

	//prepare random data for CPU load
	srand((unsigned)time(NULL));

	//rewrite each time, use O_APPEND if you want to write after the end.
	int fd = open(filename.c_str(), 
					O_WRONLY /*write only*/ | O_CREAT /*create if not exists*/, 
					S_IRWXU /*access control*/);	
	//if file opened successfully
	if(GEN_ERR != fd)
	{
		//log file title
		string title = "timestamp\tIP\tcpu_id\tusage\n";
		write(fd, title.c_str(), title.length());

		//generate log while timestamp increase minute by minute
		while(endStamp >= timestamp )
		{
			//for each minute, will generate log for all 1000 servers.
			//calculate base server IP, start from here
			IPADDR ipaddr = ipStr2Int(BASE_SERVER_ADDRESS);

			//server IP addresses increased one each time
			for(int ss = 0; ss< TOTAL_SERVER_AMOUNT; ss++)
			{
				//can use methods provided by logFile class, 
				//but not necessary since the command tool runs separately from generater.
				//lgf.writeLog(fd, timestamp, ipaddr, 0/*cpu id*/, rand()%101/*cpu load*/);
				//lgf.writeLog(fd, timestamp, ipaddr, 1/*cpu id*/, rand()%101/*cpu load*/);

				//directly write log to log file
				STRING str1 = int2str(timestamp) + '\t' + ipaddr2str(ipaddr) + '\t' + int2str(0) + '\t' + int2str(rand()%101) + '\n';
				write(fd, str1.c_str(), str1.length());
				
				//directly write log to log file
				STRING str2 = int2str(timestamp) + '\t' + ipaddr2str(ipaddr) + '\t' + int2str(1) + '\t' + int2str(rand()%101) + '\n';
				write(fd, str2.c_str(), str2.length());

				//we use an interger to present ipaddress, each char of int save one IP section.
				//thus next ip =  current ip + 1;
				//if current ip is xxx.xxx.z.255, we can automatically get next ip = xxx.xxx.(z+1).0 by add 1
				ipaddr++;
			}

			//increase 60 each time since new log comes out each minute
			timestamp += 60;
		}
		close(fd);		//finish writing.
	}
	else
	{
		cout<<"Create new log file "<<filename<<" failed, log writing stops."<<endl;
		exit(-1);
	}

	return NULL;
}

/*******************************************************************
Function		:	createThreads
Created		:	2015-05-08
Usage		:	create threads for log generator, each thread response for one day's log generating
input		:	start date, start UNIX timestamp, end UNIX timestamp, path assigned by user
return		:	0 on success, GEN_ERR on failure
*******************************************************************/
int createThreads(STRING start, time_t startStamp, time_t endStamp, STRING path)
{
	//calculate the border of current day and next day to decide how many days to be generated.
	time_t nextDayStamp;
	str2timeStamp(start, nextDayStamp);
	int num=0;	//how many days to generate

	//put parameters of all threads into a vector
	//each thread will have its own parameter since the start/end timestamps of each thread are different 
	vector<TP> threadParasl;
	TP tp;
	//path should be same for all threads
	tp.path = path;

	//calculate start/end timestamp for each day (thread).
	while(endStamp > nextDayStamp)
	{
		//if it's not the first day, start from border timestamp (00:00)
		if(num)
			tp.startStamp = nextDayStamp;
		//if it's first day, start from assigned start timestamp
		else
			tp.startStamp = startStamp;

		//calculate next border timestamp
		nextDayStamp += TOTAL_SECONDS_A_DAY;

		//if it's not the last day, end timestamp should be one minute ahead of border time stamp (23:59)
		if(endStamp > nextDayStamp)
			tp.endStamp = nextDayStamp-60;
		//if it's the last day, end timestamp is the end timestamp assigned by user.
		else
			tp.endStamp = endStamp;

		//push it to vector
		threadParasl.push_back(tp);

		//counting for total threas to be created.
		num++;
	}

	//thread pool
	pthread_t* tids  = new pthread_t[num];

	//create threads one by one, each one take its own parameter from vector
	for(int ii=0; ii<num; ii++)
	{
		if(GEN_ERR == pthread_create(&(tids[ii]), NULL, genLogThread, (void*)(&(threadParasl[ii]))) )
		{
			cout<<"######### Thread creation failed ###########"<<endl;
			return GEN_ERR;
		}
	}

	//wait untill all threads finish their jobs.
	for(int pp=0; pp<num; pp++)
		pthread_join(tids[pp], NULL);

	//release allocated memeory
	delete [] tids;

	return 0;
}


/*******************************************************************
Function		:	main
Created		:	2015-05-08
Usage		:	mian function of log generator
input		:	parameters: 	path if only for  one day (in default 2014-10-31)
							or start date/time, end date/time if user want to assign it, could cover multiple days
return		:	integer value, 0 in default
*******************************************************************/
int main(int argc, char* argv[])
{

	//for performance statistics.
	struct timeval structBegin, structExit;
	gettimeofday(&structBegin, NULL);
	long beginning = ((long)structBegin.tv_sec)*1000 +  (long)structBegin.tv_usec/1000;
	//cout<<"Begin time: "<<beginning<<" ms"<<endl;


	//default path, if the parameter is not mandatory, current directory will be used for queries.
	STRING path = "./";

	//default start/end date/time
	STRING start = DEFAULT_START_DATE_TIME_LOG;
	STRING end = DEFAULT_END_DATE_TIME_LOG;

	//get timestamp of start/end data/time
	time_t startStamp=0;
	time_t endStamp=0;
	str2timeStamp(start, startStamp);
	str2timeStamp(end, endStamp);

	//check input parameters and set working directory if all inputs are valid.
	//enable user type in start time and end time, could cover multiple days from start to end.
	if(argc>5)
	{
		//combine start/end date/time assigned by user
		STRING ss = " ";
		STRING cmdStart = argv[2] + ss + argv[3];
		STRING cmdEnd = argv[4] + ss + argv[5];

		//check whether start/end date/time assigned by user is valid or not
		if( (true != str2timeStamp(cmdStart, startStamp)) || (true != str2timeStamp(cmdEnd, endStamp)) )
		{
			cout<<"Invalid time.\n\nTime format : YYYY-MM-DD hh:mm"<<endl<<endl;
			exit(-1);
		}

		//start date/time should be earlier than end date/time
		if( startStamp >= endStamp)
		{
			cout<<"Start time is later than or same as end time."<<endl<<endl;
			exit(-1);
		}
	}
	else if(argc>1)
	{
		//if path is defined by user, try to access it to make sure it's accessible
		if(GEN_ERR != access(argv[1], PERMISSION_CHK_RDWT))
			path = argv[1];
		//if failed, quit with indication
		else	
		{
			cout<<"Invalid path."<<endl<<endl;
			exit(-1);
		}
	}
	//if the parameter path is a must, exit with help information, if not, can mask all "else" branch below.
	else
	{
		cout<<"Parameter is missing.\n\nUsage : logQuerier <PATH> [optional: start date] [optional: start time] [optional: end date] [optional: end time]";
		cout<<"\n		start/end date format YYYY-MM-DD\n		start/end time format hh:00"<<endl<<endl;
		exit(-1);
	}

	//create threads to handle generating jobs.
	createThreads(start, startStamp, endStamp, path);
	cout<<endl<<"Log generation accomplished."<<endl;

	//for performance statistics.
	gettimeofday(&structExit, NULL);
	long exiting = ((long)structExit.tv_sec)*1000 +  (long)structExit.tv_usec/1000;
	//cout<<"Exit time: "<<exiting<<" ms"<<endl;
	cout<<"Log generation cost is "<<(exiting - beginning) <<"ms."<<endl;
	
	return 0;
}

