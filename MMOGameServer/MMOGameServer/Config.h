#ifndef _GAMESERVER_PARSE_CONFIG_H_
#define _GAMESERVER_PARSE_CONFIG_H_

#include "Parse.h"

class CConfig
{
	enum eNumConfig
	{
		eNUM_BUF = 20,
	};
public:
	CConfig();
	~CConfig();

	bool Set();

public:
	//	NETWORK
	WCHAR BIND_IP[20];
	int BIND_IP_SIZE;
	int BIND_PORT;

	WCHAR LOGIN_BIND_IP[20];
	int LOGIN_BIND_IP_SIZE;
	int LOGIN_BIND_PORT;

	WCHAR MONITOR_BIND_IP[20];
	int MONITOR_BIND_IP_SIZE;
	int MONITOR_BIND_PORT;

	int WORKER_THREAD;

	//	SYSTEM
	int CLIENT_MAX;
	int PACKET_CODE;
	int PACKET_KEY1;
	int PACKET_KEY2;
	int LOG_LEVEL;
	int SEND;
	int AUTH;
	int GAME;

	//	DATABASE
	char ACCOUNT_IP[20];
	int ACCOUNT_IP_SIZE;
	int ACCOUNT_PORT;
	char ACCOUNT_USER[20];
	int ACCOUNT_USER_SIZE;
	char ACCOUNT_PASSWORD[20];
	int ACCOUNT_PASSWORD_SIZE;
	char ACCOUNT_DBNAME[20];
	int ACCOUNT_DBNAME_SIZE;

	CINIParse _Parse;

private:
	char IP[20];
};

#endif