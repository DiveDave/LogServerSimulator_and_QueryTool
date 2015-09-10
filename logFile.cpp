/*******************************************************************
File name	:	logFile.cpp
Created		:	2015-05-09
Usage		:	Implementation of class logFile
Owner		:	Yuanda Xu
*******************************************************************/

#include "logFile.h"


using namespace std;


/*******************************************************************
Function	:	constructor
Created		:	2015-05-09
Usage		:	construct
input		:	none
return		:	none
*******************************************************************/
LogFile::LogFile()
{
}

/*******************************************************************
Function	:	distructor
Created		:	2015-05-09
Usage		:	to release resource
input		:	none
return		:	none
*******************************************************************/
LogFile::~LogFile()
{
	//to release all resources: inner map layer
	for(map<int, MYRCD>::iterator entry = allRecords.begin(); entry != allRecords.end(); entry++)
	{
		(entry->second).clear();
	}
	//to release all resources: outter map layer
	allRecords.clear();
}

/*******************************************************************
Function	:	loadLog
Created		:	2015-05-09
Usage		:	to load all log content into data structure from a log file
input		:	log filename string
return		:	true or false
*******************************************************************/
bool LogFile::loadLog(STRING logFileName)
{
	//open log file with read only mode
	int fd = open(logFileName.c_str(), O_RDONLY);

	//if success
	if(GEN_ERR != fd)
	{
		//allocate a big buffer, about 110M
		//load whole file into memory at one time, 
		//it's faster than load and check line by line.
		char* buf = new char [115257600+1];
		int len = read(fd, buf, MAX_LOG_SIZE_DAILY);
		close(fd);
		buf[len] = 0;

		//check data in buf and load necessary data into map
		char* pLine = strtok(buf, SEPARATOR_OF_LINES);
		STRING line;

		//keep checking records line by line until finish the whole file.
		while(NULL != pLine)
		{
			line = pLine;

			//first line is just a title, skip it.
			if(string::npos == line.find("timestamp"))
			{
				//temporary variables to hold data
				time_t timeStampInRecord;
				int ipaddr, cpuIdInRecord, cpuLoad;

				//obtain log data
				if( true == getLineData(line, ipaddr, cpuIdInRecord, timeStampInRecord, cpuLoad))
				{
					//current line contains incorrect information
					if(GEN_ERR == ipaddr)
						cout<<"Wrong server IP address "<<ipaddr<<" in log record : ["<<line<<"], skip it."<<endl;
					//timestamp in this log is not correct
					else if(GEN_ERR == timeStampInRecord)
						cout<<"Wrong time stamp "<<timeStampInRecord<<" in log record : ["<<line<<"], skip it."<<endl;
					//cpu ID is not correct
					else if(GEN_ERR == cpuIdInRecord)
						cout<<"Wrong CPU id "<<cpuIdInRecord<<" in log record : ["<<line<<"], skip it."<<endl;
					else
					{
						//use a temporary structure to save cpu load obtained.
						//if current IP and current timestamp does not exist in "allRecords", then put it into new record
						//if the record exists (should be another cpu id comparing to exists one, but in same RCDDT structure)
						RCDDT rcdTmp;
						rcdTmp.cpuLoad_0 = CPULOAD_UNOBTAINED;
						rcdTmp.cpuLoad_1 = CPULOAD_UNOBTAINED;

						//valid cpu load value, save it.
						if((MIN_CPU_LOAD <= cpuLoad) && (MAX_CPU_LOAD >= cpuLoad))
							(0 == cpuIdInRecord)?(rcdTmp.cpuLoad_0 = cpuLoad) : (rcdTmp.cpuLoad_1 = cpuLoad);
						//invalid cpu load value from 0 to 100, marked with (-1)
						else  
						{
							cout<<endl<<"Faile to parse CPU load "<<cpuLoad<<" in log record : ["<<line<<"], mark it as (-1)."<<endl;
							(0 == cpuIdInRecord)?(rcdTmp.cpuLoad_0 = CPULOAD_INVALIDNUM) : (rcdTmp.cpuLoad_1 = CPULOAD_INVALIDNUM);
						}

						//check whether current current IP (as a key) exists or not
						//if not, then put a new map<int, RCDDT> into allRecords.
						if(allRecords.end() == allRecords.find(ipaddr))	
						{
							MYRCD myRcd;
							myRcd[timeStampInRecord] = rcdTmp;
							allRecords[ipaddr] = myRcd;
						}
						//if yes, then go inside map<int, RCDDT> data structure
						else
						{
							//check current timestamp (as a key) exists or not
							//if not, put temporary RCDDT into map
							if((allRecords[ipaddr]).end() == (allRecords[ipaddr]).find(timeStampInRecord))
								(allRecords[ipaddr])[timeStampInRecord] = rcdTmp;
							//if yes, just change corresponding value in data structure. 
							else
							{
								//update record of CPU #0
								if(0 == cpuIdInRecord)
									((allRecords[ipaddr])[timeStampInRecord]).cpuLoad_0 = rcdTmp.cpuLoad_0;
								//update record of CPU #1
								else if(1 == cpuIdInRecord)
									((allRecords[ipaddr])[timeStampInRecord]).cpuLoad_1 = rcdTmp.cpuLoad_1;
								//do nothing
								else
									;
							}
						}

					}
				}

			}
			//keep dividing in line string
			pLine = strtok(NULL, SEPARATOR_OF_LINES);
		}

		//release resource
		delete [] buf;
		
		return true;
	}

	return false;
}


/*******************************************************************
Function	:	writeLog
Created		:	2015-05-09
Usage		:	to write one log into file
input		:	file descriptor, UNIX timestamp/IP/CPU id/CPU load of record
return		:	true or false
*******************************************************************/
bool LogFile::writeLog(int fd, int timestamp, int ipaddr, int cpuId, int cpuLoad)
{
	if(GEN_ERR >= fd)
		return false;

	//build new line string
	STRING str = int2str(timestamp) + '\t' + ipaddr2str(ipaddr) + '\t' + int2str(cpuId) + '\t' + int2str(cpuLoad) + '\n';

	//write to log file
	write(fd, str.c_str(), str.length());

	return true;
}

/*******************************************************************
Function	:	queryLog
Created		:	2015-05-09
Usage		:	to query records which meets given conditions
input		:	IP integer, CPU id, query start/end UNIX timestamp
return		:	result string vector
*******************************************************************/
vector<STRING> LogFile::queryLog(IPADDR ipaddr, int cpuId, time_t start, time_t end)
{
	//result vector
	vector<string> rst;
	int len = 0;

	//sicne all data has already been parsed and stored in data structure, here directly go over data structure for queries
	for(map<time_t, RCDDT>::iterator rcd = (allRecords[ipaddr]).begin(); rcd != (allRecords[ipaddr]).end(); rcd++)
	{
		//inner layer map uses timestamp as record
		time_t tmt = rcd->first;

		//filter all records meets time requirement of query.
		if( (start <= tmt) && (end > tmt) )
		{
			//since there are two records in the structure, need pick the required CPU load according to CPU id.
			int cpuLoad = 0;
			if(( 0 == cpuId))
				cpuLoad = (rcd->second).cpuLoad_0;
			else
				cpuLoad = (rcd->second).cpuLoad_1;
			
			//if CPU load haven't been obtained for current IP & timestamp & cpu id, skip it.
			if(CPULOAD_UNOBTAINED == cpuLoad)
				continue;

			//prepare the result string of cpu load
			string cpuLoadstr = "";
			
			//if the CPU load in log file is invalid, for example, AB
			if(CPULOAD_INVALIDNUM == cpuLoad)
				cpuLoadstr = "N/A";
			//valid CPU load data from log
			else
				cpuLoadstr = int2strAign(cpuLoad) + "\%";

			//full result string: (YYYY-MM-DD hh:mm, cpuload),
			string str = "(";
			str += timeStamp2str(tmt);
			str += ", " + cpuLoadstr + "),";
			len = str.length();

			//push this record into result vector
			rst.push_back(str);
		}
	}

	//modify last "," to " "
	if(rst.size())
		(rst[rst.size()-1])[len-1] = ' ';
	
	return rst;
}


/*******************************************************************
Function	:	getLineData
Created		:	2015-05-09
Usage		:	to parse timestamp, IP, CPU id and CPU load from one record line in log file
intput 		:	line string
output		:	IP integer, CPU id, timestamp, CPU load
return		:	true of false
*******************************************************************/
bool LogFile::getLineData(STRING line, int& ipaddr, int& cpuIdInRecord, time_t& timeStamp, int& cpuLoad)
{
	//keep pointing to previous line separator
	int pre = 0;
	//keep pointing to current line separator
	int pos = line.find(SEPARATOR_IN_LINE);

	//no separater found, return false
	if(string::npos == pos)
		return false;

	//extract timestamp from line string
	timeStamp = str2int(line.substr(pre, pos));

	//in case there are multiple seperator
	while(SEPARATOR_IN_LINE == line.at(pos))
		pos++;

	//update "pre" to current separator then search next one
	pre = pos;
	pos = line.find(SEPARATOR_IN_LINE, pre);

	//if cannot find more separator, return false
	if(string::npos == pos)
		return false;

	//now get ip address string
	STRING ipStrInRecord = line.substr(pre, pos-pre);
	//translate ip string to IP integer
	ipaddr = ipStr2Int(ipStrInRecord);

	//skip multiple separator if there are
	while(SEPARATOR_IN_LINE == line.at(pos))
		pos++;

	//update "pre" to current separator then search next one
	pre = pos;
	pos = line.find(SEPARATOR_IN_LINE, pre);

	//if cannot find more separator, return false
	if(string::npos == pos)
		return false;

	//parse CPU id
	cpuIdInRecord = str2int(line.substr(pre, pos-pre));

	//skip multiple separator if there are
	while(SEPARATOR_IN_LINE == line.at(pos))
		pos++;

	//update "pre" to current separator then search next one
	pre = pos;
	pos = line.find(SEPARATOR_IN_LINE, pre);

	//parse CPU id
	cpuLoad = str2int(line.substr(pre, pos-pre));

	return true;
}


/*******************************************************************
Function	:	traversalAllRecords
Created		:	2015-05-09
Usage		:	to print all loaded log records, for debug.
intput 		:	none
output		:	none
return		:	none
*******************************************************************/
void LogFile::traversalAllRecords()
{
	cout<<"============================ All recordes ==========================="<<endl;

	int total = 0;

	//use IP integer as key in outter layer map to obtain all records for a specific servre.
	for(map<int, MYRCD>::iterator entry = allRecords.begin(); entry != allRecords.end(); entry++)
	{
		//get IP integer of current server
		IPADDR ipaddr = entry->first;
		cout<<"---------------- server "<<ipaddr2str(ipaddr)<<" --------------------"<<endl;

		//use timestamp as key in inner map to extract each record of this server.
		for(map<time_t, RCDDT>::iterator rcd = (entry->second).begin(); rcd != (entry->second).end(); rcd++)
		{
			//get timestamp of one record
			time_t tmt = rcd->first;

			//check records of both CPU #0 & #1
	 		int cpuLoad_0 = (rcd->second).cpuLoad_0;
			int cpuLoad_1 = (rcd->second).cpuLoad_1;

			total++;	//count total records amount

			//skip unobtained records 
			if(CPULOAD_UNOBTAINED ==cpuLoad_0)
				total--;	//reduce skipped record
			//invalid number in record
			else if(CPULOAD_INVALIDNUM== cpuLoad_0)
				cout<<"	["<<ipaddr2str(ipaddr)<<"] "<<timeStamp2str(tmt)<<", CPU[0] load is [N/A]."<<endl;
			//correct record of CPU #0
			else
				cout<<"	["<<ipaddr2str(ipaddr)<<"]  "<<timeStamp2str(tmt)<<", CPU[0] load is ["<<(int2str(cpuLoad_0))<<"]."<<endl;;

			total++;
				
			//skip unobtained records 
			if(CPULOAD_UNOBTAINED ==cpuLoad_1)
				total--;	//reduce skipped record
			//invalid number in record
			else if(CPULOAD_INVALIDNUM== cpuLoad_1)
				cout<<"	["<<ipaddr2str(ipaddr)<<"]  "<<timeStamp2str(tmt)<<", CPU[1] load is [N/A]."<<endl;
			//correct record of CPU #0
			else
				cout<<"	["<<ipaddr2str(ipaddr)<<"]  "<<timeStamp2str(tmt)<<", CPU[1] load is ["<<(int2str(cpuLoad_1))<<"]."<<endl;;

		}
		cout<<endl;
	}
	
	cout<<"Totally find "<<total<<" records for "<<allRecords.size()<<" servers."<<endl;

	return;
}
