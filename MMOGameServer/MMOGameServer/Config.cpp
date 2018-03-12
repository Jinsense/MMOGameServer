#include <windows.h>

#include "Config.h"

CConfig::CConfig()
{
	ZeroMemory(&BIND_IP, sizeof(BIND_IP));
	BIND_IP_SIZE = eNUM_BUF;
	BIND_PORT = NULL;

	ZeroMemory(&LOGIN_BIND_IP, sizeof(LOGIN_BIND_IP));
	LOGIN_BIND_IP_SIZE = eNUM_BUF;
	LOGIN_BIND_PORT = NULL;

	ZeroMemory(&MONITOR_BIND_IP, sizeof(MONITOR_BIND_IP));
	MONITOR_BIND_IP_SIZE = eNUM_BUF;
	MONITOR_BIND_PORT = NULL;

	WORKER_THREAD = NULL;

	CLIENT_MAX = NULL;
	PACKET_CODE = NULL;
	PACKET_KEY1 = NULL;
	PACKET_KEY2 = NULL;
	LOG_LEVEL = NULL;

	ZeroMemory(&ACCOUNT_IP, sizeof(ACCOUNT_IP));
	ACCOUNT_IP_SIZE = eNUM_BUF;
	ACCOUNT_PORT = NULL;
	ZeroMemory(&ACCOUNT_USER, sizeof(ACCOUNT_USER));
	ACCOUNT_USER_SIZE = eNUM_BUF;
	ZeroMemory(&ACCOUNT_PASSWORD, sizeof(ACCOUNT_PASSWORD));
	ACCOUNT_PASSWORD_SIZE = eNUM_BUF;
	ZeroMemory(&ACCOUNT_DBNAME, sizeof(ACCOUNT_DBNAME));
	ACCOUNT_DBNAME_SIZE = eNUM_BUF;

	ZeroMemory(&IP, sizeof(IP));
}

CConfig::~CConfig()
{

}

bool CConfig::Set()
{
	bool res = true;
	res = _Parse.LoadFile(L"GameServer_Config.ini");
	if (false == res)
		return false;
	res = _Parse.ProvideArea("NETWORK");
	if (false == res)
		return false;
	res = _Parse.GetValue("BIND_IP", &IP[0], &BIND_IP_SIZE);
	if (false == res)
		return false;
	_Parse.UTF8toUTF16(IP, BIND_IP, sizeof(BIND_IP));
	res = _Parse.GetValue("BIND_PORT", &BIND_PORT);
	if (false == res)
		return false;
	_Parse.GetValue("LOGIN_BIND_IP", &IP[0], &LOGIN_BIND_IP_SIZE);
	_Parse.UTF8toUTF16(IP, LOGIN_BIND_IP, sizeof(LOGIN_BIND_IP));
	_Parse.GetValue("LOGIN_BIND_PORT", &LOGIN_BIND_PORT);

	_Parse.GetValue("MONITOR_BIND_IP", &IP[0], &MONITOR_BIND_IP_SIZE);
	_Parse.UTF8toUTF16(IP, MONITOR_BIND_IP, sizeof(MONITOR_BIND_IP));
	_Parse.GetValue("MONITOR_BIND_PORT", &MONITOR_BIND_PORT);

	res = _Parse.GetValue("WORKER_THREAD", &WORKER_THREAD);
	if (false == res)
		return false;

	_Parse.ProvideArea("SYSTEM");
	_Parse.GetValue("CLIENT_MAX", &CLIENT_MAX);
	_Parse.GetValue("PACKET_CODE", &PACKET_CODE);
	_Parse.GetValue("PACKET_KEY1", &PACKET_KEY1);
	_Parse.GetValue("PACKET_KEY2", &PACKET_KEY2);
	_Parse.GetValue("LOG_LEVEL", &LOG_LEVEL);

	_Parse.ProvideArea("DATABASE");
	_Parse.GetValue("ACCOUNT_IP", &ACCOUNT_IP[0], &ACCOUNT_IP_SIZE);
	_Parse.GetValue("ACCOUNT_PORT", &ACCOUNT_PORT);
	_Parse.GetValue("ACCOUNT_USER", &ACCOUNT_USER[0], &ACCOUNT_USER_SIZE);
	_Parse.GetValue("ACCOUNT_PASSWORD", &ACCOUNT_PASSWORD[0], &ACCOUNT_PASSWORD_SIZE);
	_Parse.GetValue("ACCOUNT_DBNAME", &ACCOUNT_DBNAME[0], &ACCOUNT_DBNAME_SIZE);

	return true;
}