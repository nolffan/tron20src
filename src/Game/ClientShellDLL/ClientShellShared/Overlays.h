// ----------------------------------------------------------------------- //
//
// MODULE  : Overlays.h
//
// PURPOSE : definitions for overlays
//
// CREATED : 1/28/00
//
// ----------------------------------------------------------------------- //

#ifndef __OVERLAYS_H__
#define __OVERLAYS_H__

// the order of the eOverlayMask enum determines drawing priority, only the first
// exclusive overlay is drawn, while all active non-exclusive overlays are drawn
// RETAIL: Tron has 15 masks (CInterfaceMgr's m_hOverlays, constructor
// 0x100c67b0); BeginScope (0x100cd450), EndScope (0x100cd480) and EndZoom
// (0x100cd4d0) show the first eight; the rest are Tron's (CTronPlayerMgr
// uses them: 8, 11 and 14 one group, 9, 10, 12 and 13 the zoom overlays
// EndZoom removes). The names of OVM_TRON_* are ours.
enum eOverlayMask
{
	OVM_NONE  = -1,

	//exclusive overlays
	OVM_SCOPE = 0,
	OVM_BINOC,
	OVM_SS_MASK,

	//non-exclusive overlays
	OVM_NON_EXCLUSIVE,
	OVM_STATIC = OVM_NON_EXCLUSIVE,
	OVM_CAMERA,
	OVM_CAMERA_TARGET,
	OVM_ZOOM_IN,
	OVM_ZOOM_OUT,

	OVM_TRON_8,
	OVM_TRON_9,
	OVM_TRON_10,
	OVM_TRON_11,
	OVM_TRON_12,
	OVM_TRON_13,
	OVM_TRON_14,

	NUM_OVERLAY_MASKS
};


#endif  // __OVERLAYS_H__