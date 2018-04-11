#include "Player.h"

CPlayer::CPlayer()
{
	_AccountNo = NULL;
	ZeroMemory(&_SessionKey, sizeof(_SessionKey));
	_Version = NULL;
}

CPlayer::~CPlayer()
{

}

void CPlayer::OnAuth_ClientJoin()
{
	//	�ش� �÷��̾� �������� ���� ���ӻ��� ������ �Ҵ� �� �غ�

	return;
}

void CPlayer::OnAuth_ClientLeave()
{
	//	�������������� Ư���ϰ� ����ߴ� �����Ͱ� �ִٸ� ���� (�ƹ��͵� ���� ���� ����)

	return;
}

void CPlayer::OnAuth_Packet(CPacket *pPacket)
{
	//	�α��� ��Ŷ Ȯ�� �� DB ������ �ε�
	//== Auth ������
	//
	//en_PACKET_CS_GAME_REQ_LOGIN
	//en_PACKET_CS_GAME_RES_LOGIN
	if (0 == pPacket->GetDataSize())
	{
		_pLog->Log(L"Error", LOG_SYSTEM, L"OnAuthPacket - Packet DataSize is 0  Index : %d", this->_iArrayIndex);
		Disconnect();
		return;
	}
	WORD Type;
	pPacket->PopData((char*)&Type, sizeof(WORD));

	switch (Type)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:
	{
		*pPacket >> _AccountNo;
		pPacket->PopData((char*)_SessionKey, sizeof(_SessionKey));
		*pPacket >> _Version;

		_AuthToGameFlag = true;

		CPacket *pNewPacket = CPacket::Alloc();
		Type = en_PACKET_CS_GAME_RES_LOGIN;
		BYTE Status = 1;

		*pNewPacket << Type << Status << _AccountNo;
		SendPacket(pNewPacket);
		pNewPacket->Free();
	}
	break;
	default:
		wprintf(L"Wrong Packet Type !!\n");
		g_CrashDump->Crash();
		break;
	}
	return;
}

void CPlayer::OnGame_ClientJoin()
{
	//	���������� ������ ���� ������ �غ� �� ���� ( ���� �ʿ� ĳ���� ����, ����Ʈ �غ� ��� )

	return;
}

void CPlayer::OnGame_ClientLeave()
{
	//	���� ���������� �÷��̾� ���� ( ����ʿ��� ����, ��Ƽ���� ��� )

	return;
}

void CPlayer::OnGame_Packet(CPacket *pPacket)
{
	// ���� ���� ��Ŷ ó��
	//== Game ������
	//
	//en_PACKET_CS_GAME_REQ_ECHO
	//en_PACKET_CS_GAME_RES_ECHO
	if (0 == pPacket->GetDataSize())
	{
		_pLog->Log(L"Error", LOG_SYSTEM, L"OnGamePacket - Packet DataSize is 0  Index : %d", this->_iArrayIndex);
		Disconnect();
		return;
	}
	WORD Type;
	pPacket->PopData((char*)&Type, sizeof(WORD));

	switch (Type)
	{
	case en_PACKET_CS_GAME_REQ_ECHO:
	{
		LONGLONG SendTick;
		*pPacket >> _AccountNo >> SendTick;

		CPacket *pNewPacket = CPacket::Alloc();
		Type = en_PACKET_CS_GAME_RES_ECHO;

		*pNewPacket << Type << _AccountNo << SendTick;
		SendPacket(pNewPacket);
		pNewPacket->Free();
	}
	break;
	default:
		wprintf(L"Wrong Packet Type !!\n");
		g_CrashDump->Crash();
		break;
	}
	return;
}

void CPlayer::OnGame_ClientRelease()
{
	//	�ش� Ŭ���̾�Ʈ�� ������ ���� ����, ����, ���� ��� 



	return;
}