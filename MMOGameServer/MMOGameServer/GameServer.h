#ifndef _GAMESERVER_GAMESERVER_GAMESERVER_H_
#define _GAMESERVER_GAMESERVER_GAMESERVER_H_

#include "MMOGameServer.h"

class CGameServer : public CMMOServer
{
	CGameServer(int iMaxSession);
	~CGameServer();

	void OnAuth_Update();
	void OnGame_Update();
	void OnError(int iErrorCode, WCHAR *szError);

private:

};

#endif
