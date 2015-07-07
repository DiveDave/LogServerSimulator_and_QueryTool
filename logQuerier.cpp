/*******************************************************************
File name		:	logQuerier.cpp
Created		:	2015-05-10
Usage		:	Implementation of log file querier
Owner		:	Yuanda Xu
*******************************************************************/

#include "logQuerier.h"



using namespace std;


/*******************************************************************
Function		:	inputChk
Created		:	2015-05-10
Usage		:	check for all input parameters of log querier
input		:	parameters: 	server IP, CPU id, start date/time, end date/time, could cover multiple days
output		:	IP integer
return		:	true or false
*******************************************************************/
bool inputChk(STRING svr_ip, int cpuId, STRING startDate, STRING startMin, STRING endDate, STRING endMin, int& ipaddr)
{
	//check for IP address, if sucess, outpu IP integer, return with indication if failed
	if(false == ipCheck(svr_ip, ipaddr))
	{
		cout<<"Invalid server IP address"<<endl;
		return false;
	}

	//check CPU id, return with indication if failed
	if(0>cpuId || 1<cpuId) 
	{
		cout<<"Invalid server IP address"<<endl;
		return false;
	}

	//parse input start time and end time, and check if numbers are valid
	time_t startingTimeStamp=0;
	time_t endingTimeStamp=0;
	if( (false == str2timeStamp(startDate +" " + startMin, startingTimeStamp))||
		(false == str2timeStamp(endDate +" " + endMin, endingTimeStamp)) )
	{
		cout<<"Invalid time input: check year, month, date, hour and minute ."<<endl;
		return false;
	}

	//make sure end time is later than start time
	if((0 > startingTimeStamp) || (0 > endingTimeStamp) || (endingTimeStamp <= startingTimeStamp))
	{
		cout<<"Invalid time input: end time is earlier than or equal to start time."<<endl;
		return false;
	}

	return true;
}


/*******************************************************************
Function		:	init
Created		:	2015-05-10
Usage		:	mian function of log querier
input		:	parameters: 	path if only for  one day (in default 2014-10-31), logFile object
return		:	true or false
*******************************************************************/
bool init(STRING path, LogFile& lgf)
{
	//prepare for check all content in directory
	DIR *pDir ;  
	struct dirent *pDirent  ;  
	int filesFound=0;
	
	cout<<endl<<"Initialization..."<<endl;

	//access to directory
	pDir=opendir(path.c_str());  

	//keep reading all contents in directory
	while(NULL != (pDirent=readdir(pDir)))
	{  
		//for subdirectorys
		if(pDirent->d_type & DT_DIR)  
		{
			//skip "." and ".."
			STRING sDir=pDirent->d_name;
			if(("." == sDir) || (".." == sDir))
				continue;  

			//if traversal sub directories is neede in future, open these three lines.
			/*ote
			string childpath = path + pDirent->d_name;
			cout<<"Entering child director ["<<childpath<<"]..."<<endl;
			init(childpath);  
			*/
		}  
		//for all files in directory
		else
		{
			//obtian main file name without extension (.xxx)
			STRING fullname = pDirent->d_name;
			STRING filename = fullname.substr(0, fullname.length()-4);
			time_t tmpStamp=0;
			
			//only take filename in format "YYYY-MM-DD.log"
			if( ((fullname.length()-4)== fullname.find(".log")) &&				//extension is ".log"
				(false != str2timeStamp(filename + " 00:00", tmpStamp)) )		//in format "YYYY-MM-DD"			
			{
				cout<<"File "<<(pDirent->d_name)<<" found, parsing..."<<endl;
				filesFound++;	//counting
				//should pass filename with path since user can assign any directory.
				string inputName = path + "/" + (pDirent->d_name);
				//call method provided by logFile class to load records to data structure.
				lgf.loadLog(inputName);
			}
		}
	}  
	closedir(pDir); 	//done

	cout<<"Total "<<filesFound<<" files loaded."<<endl;

	//used for debug.
	//lgf.traversalAllRecords();
}



/*******************************************************************
Function		:	main
Created		:	2015-05-10
Usage		:	mian function of log querier
input		:	parameters: 	path of log files
return		:	integer value, 0 in default
*******************************************************************/
int main(int argc, char* argv[])
{
	//for performance statistics.
	struct timeval structBegin, structExit;
	gettimeofday(&structBegin, NULL);
	long beginning = ((long)structBegin.tv_sec)*1000 +  (long)structBegin.tv_usec/1000;
	//cout<<"Initial begin time: "<<beginning<<" ms"<<endl;


	//default path, if the parameter is not mandatory, current directory will be used for queries.
	STRING path = "./";

	//check input parameters and set working directory if all inputs are valid.
	if(argc>1)
	{
		if(GEN_ERR != access(argv[1], PERMISSION_CHK_READ))
			path = argv[1];
	}
	//if the parameter path is a must, exit with help information, if not mask all "else" branch.
	else
	{
		cout<<"Parameter is missing.\n\nUsage : logQuerier <PATH>"<<endl<<endl;
		exit(0);
	}

	//will call method provided by logFile class for pre-process of querier: 
	//load all records in assigned path to data structre.
	LogFile lgf;
	init(path.c_str(), lgf);

	//for performance statistics.
	gettimeofday(&structExit, NULL);
	long exiting = ((long)structExit.tv_sec)*1000 +  (long)structExit.tv_usec/1000;
	//cout<<"Inital exit time: "<<exiting<<" ms"<<endl;
	cout<<">> Total initialization cost is "<<(exiting - beginning)<<" ms."<<endl;


	//print indication and wait for input commands.
	bool leaving= false;
	STRING cmd = "";
	STRING svr_ip="";
	IPADDR ipaddr = 0;
	int cpuId = 0;
	STRING startDate = "", startMin = "";
	STRING endDate = "", endMin = "";

	while(true)
	{
		cout<<endl<<"Please input a new command:"<<endl;
		//read command inputted by user
		cin>>cmd;

		//if it's EXIT
		if("EXIT" == cmd)
			return 0;

		//if its QUERY
		else if("QUERY" == cmd)
		{

			//statistics for query
			struct timeval structBeginQ, structExitQ;
			gettimeofday(&structBeginQ, NULL);
			long beginningQ= ((long)structBeginQ.tv_sec)*1000 +  (long)structBeginQ.tv_usec/1000;
			//cout<<"Query begin time: "<<beginningQ<<" ms"<<endl;

			//read parameters for QUERY request: server IP, CPU id, start data/time, end data/time
			cin>>svr_ip>>cpuId>>startDate>>startMin>>endDate>>endMin;

			//check all inputted parameters, tanslate IP string to IP integer
			if(true == inputChk(svr_ip, cpuId, startDate, startMin, endDate, endMin, ipaddr))
			{
				//translate start/end date/time to start/end timestamp
				time_t start=0;
				time_t end=0;
				if( (true == str2timeStamp(startDate + " " + startMin, start)) &&
						(true == str2timeStamp(endDate + " " + endMin, end)) )
				{
					//call method provided by logFile class to query all records.
					vector<STRING> rst = lgf.queryLog(ipaddr, cpuId, start, end);

					//print all results.
					for(int ii=0; ii<rst.size(); ii++)
						cout<<rst[ii]<<endl;

					//summary
					cout<<endl<<"Total "<<rst.size()<<" records found."<<endl;
				}
			}

			//for performance statistics.
			gettimeofday(&structExitQ, NULL);
			long exitingQ = ((long)structExitQ.tv_sec)*1000 +  (long)structExitQ.tv_usec/1000;
			//cout<<"Query exit time: "<<exitingQ<<" ms"<<endl;
			cout<<">> Query cost is "<<(exitingQ - beginningQ)<<" ms."<<endl;
			
		}
		//error indications for other inputs
		else
			cout<<endl<<"Unknown command."<<endl;
	}
	cout<<endl;
	return 0;
}


