// ----------------------------------------------------------------------- //
//
// MODULE  : LCHUDHelper.h
//
// PURPOSE : Light cycle HUD layout values ("HUDLayout<n>" in layout.txt)
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (RTTI LCHUDHelper,
// vtable 0x101e72b0; code 0x10040600..0x10042070). CTronLayoutMgr owns one
// (retail CTronLayoutMgr+0x116c, created by its constructor and filled by
// its Init). The function names are ours, after the attributes they read.
//
// ----------------------------------------------------------------------- //

#ifndef __LCHUDHELPER_H__
#define __LCHUDHELPER_H__

#include <vector>
#include <string>

class LCHUDHelper
{
public:
	LCHUDHelper();
	virtual ~LCHUDHelper();

	void		Init();

	LTIntPt		GetPowerupIconPos(int nLayout);
	const std::string& GetPowerupIconTexture(int nLayout, int nIndex);

	LTIntPt		GetLivesIconPos(int nLayout);
	int			GetLivesTextOffset(int nLayout);
	int			GetLivesTextHeight(int nLayout);

	LTIntPt		GetRosterPosition(int nLayout);
	int			GetRosterTextHeight(int nLayout);
	int			GetRosterTextOffset(int nLayout);
	int			GetRosterLineOffset(int nLayout);
	int			GetRosterEnableIconScaling(int nLayout);
	LTIntPt		GetRosterIconBaseSizeIfScalingIsEnabled(int nLayout);

	LTIntPt		GetIntroMasterPos(int nLayout);
	int			GetIntroVFrameBorder(int nLayout);
	int			GetIntroHFrameBorder(int nLayout);
	int			GetIntroTitleHeight(int nLayout);
	int			GetIntroEntryHeight(int nLayout);
	int			GetIntroTitleSpace(int nLayout);
	int			GetIntroEntrySpace(int nLayout);
	int			GetIntroHeaderOffset(int nLayout);
	int			GetIntroValueOffset(int nLayout);
	float		GetIntroValueTime(int nLayout);
	std::string	GetIntroTexturedFrameName(int nLayout);

	LTIntPt		GetSpeedometerPos(int nLayout);
	int			GetSpeedometerFontNum(int nLayout);
	int			GetSpeedometerTextHeight(int nLayout);
	int			GetSpeedometerVBorder(int nLayout);
	int			GetSpeedometerHBorder(int nLayout);
	int			GetSpeedometerFrameWidth(int nLayout);
	int			GetSpeedometerFrameHeight(int nLayout);
	std::string	GetSpeedometerTexturedFrameName(int nLayout);

private:
	typedef std::vector<std::string> StringArray;
	typedef std::vector<StringArray> StringArrayArray;

	// RETAIL: +4, one list of "PowerupIconTexture<n>" per HUD layout
	StringArrayArray	m_PowerupIconTextures;
};

#endif // __LCHUDHELPER_H__
