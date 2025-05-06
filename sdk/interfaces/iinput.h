#pragma once
// used: usercmd
#include "../datatypes/usercmd.h"

#define MULTIPLAYER_BACKUP 150

class IInput
{
public:
	char        pad0[0xC]{ };
	bool                m_bTrackIR{ },
		bMouseInitialized{ },
		bMouseActive{ };
	char			pad1[0x9A];
	bool                bCameraInThirdPerson{ };
	char        pad2[ 0x2 ]{ };
	Vector         vecCameraOffset{ };
	char        pad3[ 0x38 ]{ };
	CUserCmd* pCommands{ };
	CVerifiedUserCmd* pVerifiedCommands{ };

	[[nodiscard]] CUserCmd* GetUserCmd(const int nSequenceNumber) const
	{
		return &pCommands[nSequenceNumber % MULTIPLAYER_BACKUP];
	}

	[[nodiscard]] CVerifiedUserCmd* GetVerifiedCmd(const int nSequenceNumber) const
	{
		return &pVerifiedCommands[nSequenceNumber % MULTIPLAYER_BACKUP];
	}

	__forceinline int CAM_IsThirdPerson( int slot = -1 )
	{
		return MEM::CallVFunc<int>( this, 32, slot );
	}

	__forceinline void CAM_ToThirdPerson( ) {
		return MEM::CallVFunc<void>( this, 35 );
	}

	__forceinline void CAM_ToFirstPerson( ) {
		return MEM::CallVFunc<void>( this, 36 );
	}
};

