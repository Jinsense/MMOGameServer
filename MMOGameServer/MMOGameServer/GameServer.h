#ifndef _GAMESERVER_GAMESERVER_GAMESERVER_H_
#define _GAMESERVER_GAMESERVER_GAMESERVER_H_

#include <Pdh.h>

#include "MonitorProtocol.h"
#include "LanClient.h"
#include "MMOGameServer.h"

#pragma comment(lib, "Pdh.lib")

class CGameServer : public CMMOServer
{
public:
	CGameServer(int iMaxSession);
	~CGameServer();

	void OnConnectionRequest();
	void OnAuth_Update();
	void OnGame_Update();
	void OnError(int iErrorCode, WCHAR *szError);

	bool MonitorInit();
	bool MonitorOnOff();
	static unsigned int __stdcall MonitorThread(void *pParam)
	{
		CGameServer *pMonitorThread = (CGameServer*)pParam;
		if (NULL == pMonitorThread)
		{
			wprintf(L"[GameServer :: MonitorThread] Init Error\n");
			return false;
		}
		pMonitorThread->MonitorThread_update();
		return true;
	}
	bool MonitorThread_update();

	static unsigned int __stdcall LanMonitorThread(void *pParam)
	{
		CGameServer *pLanMonitorThread = (CGameServer*)pParam;
		if (NULL == pLanMonitorThread)
		{
			wprintf(L"[GameServer :: LanMonitorThread] Init Error\n");
			return false;
		}
		pLanMonitorThread->LanMonitorThread_Update();
		return true;
	}
	bool LanMonitorThread_Update();
	bool MakePacket(BYTE DataType);

public:
	CLanClient	*_pMonitor;

	int		_TimeStamp;						//	TimeStamp
	int		_CPU_Total;						//	CPU ��ü �����
	int		_Available_Memory;				//	��밡���� �޸�
	int		_Network_Recv;					//	�ϵ���� �̴��� ����
	int		_Network_Send;					//	�ϵ���� �̴��� �۽�
	int		_Nonpaged_Memory;				//	���������� �޸�

	int		_BattleServer_On;				//	��Ʋ���� ON / OFF
	int		_BattleServer_CPU;				//	��Ʋ���� CPU
	int		_BattleServer_Memory_Commit;	//	��Ʋ���� �޸� Ŀ�� (Private) MByte
	int		_BattleServer_PacketPool;		//	��Ʋ���� ��ŶǮ ��뷮
	int		_BattleServer_Auth_FPS;			//	��Ʋ���� Auth FPS
	int		_BattleServer_Game_FPS;			//	��Ʋ���� Game FPS
	int		_BattleServer_Session_ALL;		//	��Ʋ���� ���� ��ü ������ ��
	int		_BattleServer_Session_Auth;		//	��Ʋ���� Auth ��� ������ ��
	int		_BattleServer_Session_Game;		//	��Ʋ���� Game ��� ������ ��

private:
	bool	_bMonitor;
	CPlayer *_pPlayer;
	HANDLE	_hMonitorThread;

	PDH_HQUERY		_CpuQuery;
	PDH_HCOUNTER	_MemoryAvailableMBytes;
	PDH_HCOUNTER	_MemoryNonpagedBytes;
};

#endif
