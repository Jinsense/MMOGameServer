#include "Config.h
#include "GameServer.h"

//# MMOServer ���� �����Ŷ
//
//== Auth ������
//
//en_PACKET_CS_GAME_REQ_LOGIN
//en_PACKET_CS_GAME_RES_LOGIN
//
//ó�� �� ���Ӹ��� ��ȯ
//
//== Game ������
//
//en_PACKET_CS_GAME_REQ_ECHO
//en_PACKET_CS_GAME_RES_ECHO
//
//
//�̿��� ��Ŷ�� ������� �ʽ��ϴ�.

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