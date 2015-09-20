# LogServerSimulator_and_QueryTool
It's a log server simulator and query tool. It can simulate log generating of 1000 servers with a simple record like "time, server ip, cpu id, cpu load".

This file includes:
	- description of each file
	- how to build program
	- how to run program

1. Description of each file:
	./effort.txt			statistics of time spent each day
	./readme.txt			current file you are reading
	./Makefile			makefile
	./comm.cpp			implementation of common functions used by all other source codes
	./comm.h			common definitions used by all other source codes
	./logFile.cpp			implementation of logFile class
	./logFile.h			headfile with definitions of class logFile
	./logGenerator.cpp		implementation of log generator
	./logGenerator.h		headfile with definitions used by log genenrator
	./logQuerier.cpp		implementation of log querier
	./logQuerier.h			headfile with definitions usd by log querier
	./test-records.txt		summary of test results
	./60_times_test.txt		test reuslt of running logQuerier 60 times 
	./generate.sh			script to run logGenerator
	./query.sh			script to run logQuerier

2. How to build:
	- to build: make
	- to clean: make clean

3. how to run
	- to run logGenerator:	./logGenerator <PATH>
	- to run logQuerier:	./logQuerier <PATH>
				  then input "QUERY IP CPUID START-DATE/TIME END-DATE/TIE"
				  or input "EXIT"

	- to generate log files for multiple days: add start date/time end date/time after run command:
		./generate.sh ./ 2014-10-31 00:00 2014-11-01 23:59

	- to query multiple days: just type start date/time end date/time as regular QUERY:
		QUERY 192.168.3.123 1 2014-10-31 00:00 2014-11-01 23:59

