#include "Config.h"
#include "GameServer.h"

//# MMOServer 더미 사용패킷
//
//== Auth 스레드
//
//en_PACKET_CS_GAME_REQ_LOGIN
//en_PACKET_CS_GAME_RES_LOGIN
//
//처리 후 게임모드로 전환
//
//== Game 스레드
//
//en_PACKET_CS_GAME_REQ_ECHO
//en_PACKET_CS_GAME_RES_ECHO
//
//
//이외의 패킷은 사용하지 않습니다.

int main()
{
	CConfig Config;
	if (false == Config.Set())
	{
		return false;
	}

	CGameServer Server(Config.CLIENT_MAX);
	
	

	return true;
}