/*******************************************************************
File name		:	comm.h
Created		:	2015-05-08
Usage		:	common definitions
Owner		:	Yuanda Xu
*******************************************************************/

#ifndef _LOG_COMM_H_
#define _LOG_COMM_H_


#include <iostream>
#include <sstream>

#include <string>
#include <vector>
#include <map>

#include <stdio.h>		//printf
#include <stdlib.h>	//for atoi
#include <time.h>
#include <sys/time.h>


#define GEN_ERR (-1)

//cpu load should be >=0 annd <=100
#define MIN_CPU_LOAD 0
#define MAX_CPU_LOAD 100

//for file opening
#define PERMISSION_CHK_READ 04
#define PERMISSION_CHK_RDWT 06

//total 1000 server as required
#define TOTAL_SERVER_AMOUNT 1000

//assuming server IP start from 192.168.1.0 and increases continuously for each one.
#define BASE_SERVER_ADDRESS "192.168.1.0"

//for calculation
#define TOTAL_MINUTES_A_DAY 1440
#define TOTAL_SECONDS_A_DAY 86400

//for convenience
typedef int IPADDR;

//for convenience
typedef std::string STRING;

//transfer integer to string
STRING int2str(int in);

//tranfer integer to string
int str2int(STRING str);

//transfer integer to string with align
STRING int2strAign(int in);

//tanslate IP address string to an unique IP integer
int ipStr2Int(STRING ip_str);

//transfer unique IP integer to IP string
STRING ipaddr2str(IPADDR ipaddr);

//check whether an ip string is a valid ipaddress
bool ipCheck(STRING ip_str, int& ipaddr);

//translate from UNIX timestamp to time string 
STRING timeStamp2str(time_t stamp);

//translate time string to UNIX timestamp
bool str2timeStamp(STRING str, time_t& tmStamp);





#endif
