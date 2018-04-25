#include <conio.h>

#include "Config.h"
#include "GameServer.h"

CConfig Config;

int main()
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	bool bFlag = true;
	int In;
	if (false == Config.Set())
	{
		return false;
	}

	CGameServer Server(Config.CLIENT_MAX, Config.SEND, Config.AUTH, Config.GAME);

//	if (false == Server._pMonitor->Connect(Config.MONITOR_BIND_IP, Config.MONITOR_BIND_PORT, true, LANCLIENT_WORKERTHREAD))
//	{
//		{
//			wprintf(L"[Main :: MonitorClient Connect] Error\n");
//			return 0;
//		}
//	}

	if (false == Server.Start(Config.BIND_IP, Config.BIND_PORT, Config.WORKER_THREAD, true, Config.PACKET_CODE, Config.PACKET_KEY1, Config.PACKET_KEY2))
	{
		{
			wprintf(L"[Main :: Server Start] Error\n");
			return 0;
		}
	}
	Server.MonitorInit();

	while (bFlag)
	{
		In = _getch();
		switch (In)
		{
		case 'q': case 'Q':
		{
			bFlag = false;
			
		}
		break;
		case 'm': case 'M':
		{
			Server.MonitorOnOff();
		}
		break;
		}
	}
	return true;
}