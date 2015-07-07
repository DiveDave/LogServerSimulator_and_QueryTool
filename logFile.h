/*******************************************************************
File name		:	logFile.h
Created		:	2015-05-09
Usage		:	definitions of logFile class
Owner		:	Yuanda Xu
*******************************************************************/

#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include "comm.h"

#include <string.h>	//strstr, strtok
#include <unistd.h>	//open, read, close
#include <fcntl.h>		//O_RDONLY
#include <dirent.h>  	//DIR

//for buffer size calculation
#define MAX_LOG_LENGTH_LINE 40
#define MAX_LOG_SIZE_DAILY (MAX_LOG_LENGTH_LINE*TOTAL_MINUTES_A_DAY*(2*TOTAL_SERVER_AMOUNT+1))

//for record line parse
#define SEPARATOR_OF_LINES	("\n")
#define SEPARATOR_IN_LINE	'\t'

//to mark invalid CPU load
#define CPULOAD_UNOBTAINED (-1)
#define CPULOAD_INVALIDNUM (-2)

//CPU load structure, include load of CPU #0 & #1
typedef struct recordData{int cpuLoad_0; int cpuLoad_1;} RCDDT;

//data structure to save all records of a specific server, use UNIX timestamp as key in map
typedef std::map<time_t, RCDDT> MYRCD;

class LogFile{
public:
	LogFile();
	~LogFile();

	//load all log content into data structure
	bool loadLog(STRING logFileName);

	//write one log record to log file
	bool writeLog(int fd, int timestamp, int ipaddr, int cpuId, int cpuLoad);

	//handle query command from user
	std::vector<STRING> queryLog(IPADDR ipaddr, int cpuId, time_t start, time_t end);

	//print all loaded records
	void traversalAllRecords();

private:

	//data structure which stores all records
	std::map<int, MYRCD > allRecords;

	//parse data from one log line
	bool getLineData(STRING line, int& ipaddr, int& cpuIdInRecord, time_t& timeStamp, int& cpuLoad);

};

















#endif
