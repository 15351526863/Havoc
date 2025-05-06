#pragma once
// used: angle
#include "../datatypes/qangle.h"

#pragma region clientmode_definitions
#define SIGNONSTATE_NONE		0	// no state yet, about to connect
#define SIGNONSTATE_CHALLENGE	1	// client challenging server, all OOB packets
#define SIGNONSTATE_CONNECTED	2	// client is connected to server, netchans ready
#define SIGNONSTATE_NEW			3	// just got serverinfo and string tables
#define SIGNONSTATE_PRESPAWN	4	// received signon buffers
#define SIGNONSTATE_SPAWN		5	// ready to receive entity packets
#define SIGNONSTATE_FULL		6	// we are fully connected, first non-delta packet received (in-game check)
#define SIGNONSTATE_CHANGELEVEL	7	// server is changing level, please wait
#pragma endregion

class CViewSetup
{
public:
	int			iX;
	int			iUnscaledX;
	int			iY;
	int			iUnscaledY;
	int			iWidth;
	int			iUnscaledWidth;
	int			iHeight;
	int			iUnscaledHeight;
	bool		bOrtho;
	float		flOrthoLeft;
	float		flOrthoTop;
	float		flOrthoRight;
	float		flOrthoBottom;
	std::byte	pad0[0x7C];
	float		flFOV;
	float		flViewModelFOV;
	Vector		vecOrigin;
	QAngle		angView;
	float		flNearZ;
	float		flFarZ;
	float		flNearViewmodelZ;
	float		flFarViewmodelZ;
	float		flAspectRatio;
	float		flNearBlurDepth;
	float		flNearFocusDepth;
	float		flFarFocusDepth;
	float		flFarBlurDepth;
	float		flNearBlurRadius;
	float		flFarBlurRadius;
	float		flDoFQuality;
	int			nMotionBlurMode;
	float		flShutterTime;
	Vector		vecShutterOpenPosition;
	QAngle		vecShutterOpenAngles;
	Vector		vecShutterClosePosition;
	QAngle		vecShutterCloseAngles;
	float		flOffCenterTop;
	float		flOffCenterBottom;
	float		flOffCenterLeft;
	float		flOffCenterRight;
	bool		bOffCenter : 1;
	bool		bRenderToSubrectOfLargerScreen : 1;
	bool		bDoBloomAndToneMapping : 1;
	bool		bDoDepthOfField : 1;
	bool		bHDRTarget : 1;
	bool		bDrawWorldNormal : 1;
	bool		bCullFontFaces : 1;
	bool		bCacheFullSceneState : 1;
	bool		bCSMView : 1;
};

class IHudChat;
class IClientModeShared
{
public:
	std::byte	pad0[0x1B];
	void*		pViewport;
	IHudChat*	pChatElement;
	HCursor		hCursorNone;
	void*		pWeaponSelection;
	int			nRootSize[2];
};

class IAppSystem
{
private:
	virtual void function0() = 0;
	virtual void function1() = 0;
	virtual void function2() = 0;
	virtual void function3() = 0;
	virtual void function4() = 0;
	virtual void function5() = 0;
	virtual void function6() = 0;
	virtual void function7() = 0;
	virtual void function8() = 0;
};

class CEventInfo
{
public:
	short					iClassID;
	short pad;
	float					flFireDelay;
	const void* vSendTable;
	const CBaseClient* pClientClass;
	int m_Packed;
	int flags;
	int filter[8];
	CEventInfo* next;
};

class INetChannel;
class CClockDriftMgr
{
public:
	float m_ClockOffsets[16];
	uint32_t m_iCurClockOffset;
	uint32_t m_nServerTick;
	uint32_t m_nClientTick;
};

class IClientState
{
public:
	char		pad0[156];				// 0x0000
	INetChannel*	pNetChannel;			// 0x009C
	int				iChallengeNr;			// 0x00A0
	char		pad1[100];				// 0x00A4
	int				iSignonState;			// 0x0108
	int		pad2[2];				// 0x010C
	float			flNextCmdTime;			// 0x0114
	int				nServerCount;			// 0x0118
	int				iCurrentSequence;		// 0x011C
	int		musor_pads[ 2 ];			// 0x0120
	CClockDriftMgr m_ClockDriftMgr;
	int				iDeltaTick;				// 0x0174
	bool m_bPaused;
	char paused_align[3];
	int m_nViewEntity;
	int m_nPlayerSlot;
	int bruh;
	char m_szLevelName[260];
	char m_szLevelNameShort[80];
	char m_szGroupName[80];
	char pad_032[92];
	int m_nMaxClients;
	char pad_0314[18828];
	float m_nLastServerTickTime; // m_nLastServerTickTime
	bool m_bInSimulation;
	char pad_4C9D[3];
	int m_nOldTickCount;
	float m_flTickReminder;
	float m_flFrametime;
	int				iLastOutgoingCommand;	// 0x4CAC
	int				nChokedCommands;		// 0x4CB0
	int				iLastCommandAck;		// 0x4CB4
	int m_nPacketEndTickUpdate;//int				iLastServerTick;		// 0x4CB8
	int				iCommandAck;			// 0x4CBC
	int				iSoundSequence;			// 0x4CC0
	char pad_4CCD[76];
	Vector viewangles;
	int pads[54];
	CEventInfo* m_pEvents;

	bool& m_bIsHLTV()
	{
		return *(bool*)((uintptr_t)this + 0x4D48);
	}
	//bool			bIsHLTV;				// 0x4CC8
	//std::byte		pad8[ 0x123 ];			// 0x4CC4
	//CEventInfo* pEvents;				// 0x4DEC
}; // Size: 0x4E70
