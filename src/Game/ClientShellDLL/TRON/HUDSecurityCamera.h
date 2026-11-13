// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSecurityCamera.h
//
// PURPOSE : HUDItem drawn over the view from a security camera
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003a6b0..
// 0x1003acea, vtable 0x101e712c). The class name is retail's (its RTTI);
// the member names are ours. While the interface manager's +0x28b1d flag
// is set it tints the screen, rolls two dark bands down it like a monitor's
// refresh, flickers a textured overlay ("SecurityCam_overlay.dtx") and
// loops a monitor hum.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SECURITYCAMERA_H
#define __HUD_SECURITYCAMERA_H

#include "HUDItem.h"

class CHUDSecurityCamera : public CHUDItem
{
public:
	CHUDSecurityCamera();

    LTBOOL      Init();
	void		Term();

	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games
	// (retail's commonest slot 3, 0x10023440: a switch)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		StartFlicker();			// RETAIL: 0x1003aae0

	HTEXTURE	m_hOverlay;				// +0x10
	LT_POLYFT4	m_Overlay;				// +0x14
	int			m_nTronUnknown68;		// +0x68, cleared by Init
	float		m_fFlickerTime;			// +0x6c: left of this flicker
	float		m_fMinFlickerTime;		// +0x70
	float		m_fMaxFlickerTime;		// +0x74
	float		m_fMinAlpha;			// +0x78
	float		m_fMaxAlpha;			// +0x7c
	LT_POLYF4	m_Tint;					// +0x80
	LT_POLYF4	m_Bands[2];				// +0xb4: one band, split in two where it wraps
	float		m_fBandHeight;			// +0x11c
	float		m_fBandSpeed;			// +0x120
	float		m_fBandStart;			// +0x124: its upper edge
	float		m_fBandEnd;				// +0x128: its lower edge
	bool		m_bShow;				// +0x12c
	float		m_fScale;				// +0x130
	float		m_fTop;					// +0x134
	float		m_fBottom;				// +0x138
	float		m_fHeight;				// +0x13c
	float		m_fLeft;				// +0x140
	float		m_fWidth;				// +0x144
	HLTSOUND	m_hSound;				// +0x148
};

#endif
