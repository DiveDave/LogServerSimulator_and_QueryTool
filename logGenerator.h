/*******************************************************************
File name		:	logGenerator.h
Created		:	2015-05-08
Usage		:	definitions of log file generator
Owner		:	Yuanda Xu
*******************************************************************/

#ifndef _LOG_GENERATOR_H
#define _LOG_GENERATOR_H

#include <pthread.h>

#include "logFile.h"

//default start/end date/time
#define DEFAULT_START_DATE_TIME_LOG	"2014-10-31 00:00"
#define DEFAULT_END_DATE_TIME_LOG	"2014-10-31 23:59"

//parameters for each thread: start/end timestamp, path to write log file
typedef struct threadPara{time_t startStamp; time_t endStamp; STRING path;} TP;

#endif
