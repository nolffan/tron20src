// ----------------------------------------------------------------------- //
//
// MODULE  : HUDIOCamera.h
//
// PURPOSE : HUDItem drawn over the view from an I/O node's camera
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002b470..
// 0x1002c180, vtable 0x101e6e64). The class name is retail's (its RTTI);
// the member names are ours. Two rings of four quarter quads around the
// layout's center: the first pulses, the second flickers now and then at
// a random size and alpha. Its attributes are the layout's "IONodeCam".
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_IOCAMERA_H
#define __HUD_IOCAMERA_H

#include "HUDItem.h"

class CHUDIOCamera : public CHUDItem
{
public:
	CHUDIOCamera();

    LTBOOL      Init();

	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		StartFlicker();		// RETAIL: 0x1002bb10

	bool		m_bShow;				// +0x10
	float		m_fScale;				// +0x14
	LTRect		m_rcRect;				// +0x18: the layout's IOCameraRect
	HTEXTURE	m_hPiece1;				// +0x28
	HTEXTURE	m_hPiece2;				// +0x2c
	LT_POLYFT4	m_Piece1[4];			// +0x30
	LT_POLYFT4	m_Piece2[4];			// +0x180

	float		m_fPiece1AlphaRange;	// +0x2d0
	float		m_fPiece1AlphaMid;		// +0x2d4
	float		m_fPiece1Period;		// +0x2d8
	float		m_fPiece1Freq;			// +0x2dc
	float		m_fPiece1Radius;		// +0x2e0
	float		m_fCenterX;				// +0x2e4
	float		m_fCenterY;				// +0x2e8
	float		m_fPiece2MinInterval;	// +0x2ec
	float		m_fPiece2MaxInterval;	// +0x2f0
	float		m_fPiece2MinRadius;		// +0x2f4
	float		m_fPiece2MaxRadius;		// +0x2f8
	float		m_fPiece2MinAlpha;		// +0x2fc
	float		m_fPiece2MaxAlpha;		// +0x300
	int			m_nPiece2MinFlickers;	// +0x304
	int			m_nPiece2MaxFlickers;	// +0x308
	float		m_fPiece2FlickerTime;	// +0x30c

	float		m_fLastTime;			// +0x310
	float		m_fNextFlicker;			// +0x314
	float		m_fFlickerTime;			// +0x318
	int			m_nFlickers;			// +0x31c
};

#endif
