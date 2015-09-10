/*******************************************************************
File name	:	comm.cpp
Created		:	2015-05-08
Usage		:	common process implementation
Owner		:	Yuanda Xu
*******************************************************************/

#include "comm.h"
using namespace std;



/*******************************************************************
Function	:	digitCheck
Created		:	2015-05-08
Usage		:	to check whether all characters in a string are all digits.
input		:	string
return		:	true or false
*******************************************************************/
bool digitCheck(STRING str)
{
	//asisstant string
	STRING dstr = "0123456789";
	int len = str.length();

	for(int ii=0; ii<len; ii++)
	{
		//if a character can not be found in asisstant string, then return false
		if(STRING::npos == dstr.find(str.at(ii)))
			return false;
	}
	return true;
}

/*******************************************************************
Function	:	int2str
Created		:	2015-05-08
Usage		:	to translate integer into string
input		:	integer
return		:	string
*******************************************************************/
STRING int2str(int in)  
{
	STRING str;
	//temporary buffer
	stringstream ss;  
	ss<<in;  
	ss>>str;
	return str;
}  

/*******************************************************************
Function	:	int2strAign
Created		:	2015-05-08
Usage		:	to translate integer into aligned string
input		:	integer
return		:	string with align
*******************************************************************/
STRING int2strAign(int in)  
{
	STRING str="";

	//if there is only 1 digit
	if((0<=in)&&(in<10))
	{
		str += " ";
		str += (in+'0');
	}
	else
	{
		//temporary buffer
		stringstream ss;  
		ss<<in;  
		ss>>str;
	}
	return str;
}  

 
/*******************************************************************
Function	:	str2int
Created		:	2015-05-08
Usage		:	to translate string into integer
input		:	string
retur		:	string with align
*******************************************************************/
int str2int(STRING str)  
{
	if(false == digitCheck(str))
		return GEN_ERR;
	int rst;
	//temporary buffer
	stringstream ss(str);  
	ss>>rst;
	return rst;
}  

/*******************************************************************
Function	:	ipStr2Int
Created		:	2015-05-08
Usage		:	to translate IP string into IP integer
input		:	IP string 
return		:	IP integer 
*******************************************************************/
IPADDR ipStr2Int(STRING ip_str)
{
	int pos = 0;
	int end  = 0;
	IPADDR ipaddr = 0;
	
	do{
		//find next separator to locate a new section in IP address STRING
		end  = ip_str.find(".", pos);

		//translate current section into integer and add it to next byte of result integer.
		ipaddr = (ipaddr<<8) + atoi((ip_str.substr(pos, end)).c_str());

		//move to check next section
		pos = end+1;

	}while(STRING::npos != end);

	return ipaddr;
}

/*******************************************************************
Function	:	ipaddr2str
Created		:	2015-05-08
Usage		:	to translate IP integer to IP string
input		:	IP integer 
return		:	IP string 
*******************************************************************/
STRING ipaddr2str(IPADDR ipaddr)
{
	/*****************************************************************************
	The IP address contains four sections, and each section varies from 0 to 255.
	An integer contains four bytes, each bytes can be a value from 0 to 255.
	Thus an IP address can be transmitted to an integer by put value of each section into each byte of an integer.
	By this way, the integer transmitted from IP address is definitely a unique value, won't be duplicated with other IP address.
	And we can use this transmitted integer as key in the upper layer map to locate corresponding value, which is the map MYRCD defined above.
	******************************************************************************/
	unsigned char c1 = ipaddr&0x000000FF;
	unsigned char c2 = (ipaddr&0x0000FF00)>>8;
	unsigned char c3 = (ipaddr&0x00FF0000)>>16;
	unsigned char c4 = (ipaddr&0xFF000000)>>24;
	STRING ipStr = "";
	ipStr += int2str(c4) + ".";
	ipStr += int2str(c3) + ".";
	ipStr += int2str(c2) + ".";
	ipStr += int2str(c1) ;
	return ipStr;
}

/*******************************************************************
Function	:	ipCheck
Created		:	2015-05-08
Usage		:	to check whether it's a valid IP string or not, and translate to IP integer if valid
input		:	IP string 
output		:	IP integer
return		"	ture or false
*******************************************************************/
bool ipCheck(STRING ip_str, IPADDR& ipaddr)
{

	int pos = 0;
	int end  = 0;

	do{
		//find next separator to locate a new section in IP address STRING
		end  = ip_str.find(".", pos);

		//get one section of IP address, totally there are four sections
		string sec = ip_str.substr(pos, end-pos);
		int len = sec.length();

		//if section length is invalid, for example: 22234, then false
		if((0 >= len)||(3<len))
			return false;

		//check each character, if not a digit return false
		for(int ii=0; ii<sec.length(); ii++)
		{
			if( ('0'> (sec.at(ii))) || ('9'< (sec.at(ii))))
				return false;
		}

		//obtain value of current section
		int cc = atoi((ip_str.substr(pos, end)).c_str());
	
		//check in case an invalid value.
		if((0>cc)||(255<cc))
			return false;

		//translate current section into integer and add it to next byte of result integer.
		ipaddr = (ipaddr<<8) + atoi((ip_str.substr(pos, end)).c_str());

		//move to check next section
		pos = end+1;
	}while(STRING::npos != end);
	
	return true;
}


/*******************************************************************
Function	:	timeStamp2str
Created		:	2015-05-08
Usage		:	to tanslate UNIX timestamp to time string 
			in format "YYYY-MM-DD hh:mm:ss" (ss is not used)
input		:	time_t
return		:	time string
*******************************************************************/
STRING timeStamp2str(time_t stamp)
{
	time_t realstamp = stamp;

	//translate to time structure
	tm* time = localtime(&(stamp));

	//adjust for daylight saving time
	if(time->tm_isdst)
		realstamp = stamp - 3600;	//reduce one hour

	//now we get real time
	time = localtime(&(realstamp));

	char tbuf[100]={0};

	//transmit to time string following given format
	strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M", time);

	return STRING(tbuf);
}

/*******************************************************************
Function	:	str2timeStamp
Created		:	2015-05-08
Usage		:	to tanslate time string to UNIX timestamp
			time string format: "YYYY-MM-DD hh:mm:ss" (ss is not used)
input		:	time string
output		:	time_t
return		:	string
*******************************************************************/
bool str2timeStamp(STRING str, time_t& tmStamp)
{
	tm timeStruct={0};

	//according to requirement, the input string doesn't include seconds, thus add "00" second for each one.
	STRING full_time = str + ":00";

	//tramsmit to time structure
	if( NULL == strptime(full_time.c_str(),"%Y-%m-%d %H:%M:%S",&(timeStruct)))
	{
		//cout<<endl<<"str2timeStamp ######### Faile to parse "<<full_time<<" #######"<<endl;
		return false;
	}

	//calculate UNIX timestamp based on time structure.
	tmStamp = mktime(&timeStruct);
	return true;
}



