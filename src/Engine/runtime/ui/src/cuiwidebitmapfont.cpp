//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDEBITMAPFONT.CPP
//
//   PURPOSE   : implements the CUIWideBitmapFont font class
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------
//
// TRON: written from retail Lithtech.exe (0x567d70..0x5684f0); see
// cuiwidebitmapfont.h.

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIWIDEBITMAPFONT_H__
#include "cuiwidebitmapfont.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

#ifndef __ILTSTREAM_H__
#include "iltstream.h"
#endif

#include <stdio.h>
#include <string.h>


// interface database
static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);



//	--------------------------------------------------------------------------
// RETAIL: 0x567d70
CUIWideBitmapFont::CUIWideBitmapFont()
{
}


//	--------------------------------------------------------------------------
// RETAIL: 0x567db0
CUIWideBitmapFont::~CUIWideBitmapFont()
{
	Term();
}


//	--------------------------------------------------------------------------
// RETAIL: 0x567e00 (slot 14): the width of the string's last line in
// pixels (a line feed starts a new one; the height it adds is not used)
uint32 CUIWideBitmapFont::GetTextWidth(const std::wstring& sText)
{
	if (sText.begin() == sText.end())
		return 0;

	int32 nLen		= sText.size();
	int32 nWidth	= 0;
	int32 nHeight	= 0;
	int32 nCharHeight = GetDefCharScreenHeight();

	for (int32 i = 0; i < nLen; i++)
	{
		uint16 c = sText[i];
		int32 nCharWidth;

		switch (c)
		{
			case '\n':
				nWidth		= 0;
				nCharWidth	= 0;
				nHeight		+= GetLineSpacing() + nCharHeight;
				break;

			case '\r':
				nCharWidth	= 0;
				break;

			case ' ':
				nCharWidth	= GetSpaceWidth();
				break;

			default:
				if (IsRemapped() && GetRemapTables()[c >> 8])
				{
					c = GetRemapTables()[c >> 8][c & 0xff];
				}
				if (!((CUIWideFontChar***)GetCharTables())[c >> 8] ||
					!((CUIWideFontChar***)GetCharTables())[c >> 8][c & 0xff])
				{
					c = GetDefaultChar();
				}
				nCharWidth = ((CUIWideFontChar***)GetCharTables())[c >> 8][c & 0xff]->m_Width;
				break;
		}

		nWidth += GetCharSpacing() + nCharWidth;
	}

	return nWidth;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x567f80 (slot 15)
bool CUIWideBitmapFont::HasChar(uint16 nChar)
{
	if (((CUIWideFontChar***)GetCharTables())[nChar >> 8] &&
		((CUIWideFontChar***)GetCharTables())[nChar >> 8][nChar & 0xff])
	{
		return true;
	}

	return (nChar == ' ');
}


//	--------------------------------------------------------------------------
// RETAIL: 0x567fe0 (slot 16)
bool CUIWideBitmapFont::Load(ILTStream* pStream)
{
	char	sName[256];
	uint32	nPos;
	uint8	bPresent;
	int		i, j;

	if (!pStream)
		return false;

	// the header, whole
	if (pStream->Read(&m_xTronUnknownC, 0x1118) != LT_OK)
		return false;

	if (!m_xTronUnknownC)
		m_SpaceWidth = m_DefaultSpaceWidth;

	pStream->GetPos(&nPos);

	// the remap tables, one per high byte the file has
	memset(m_pRemap, 0, sizeof(m_pRemap));
	if (m_bRemap)
	{
		for (i = 0; i < 256; i++)
		{
			pStream->Read(&bPresent, 1);
			if (bPresent)
			{
				m_pRemap[i] = (uint16*)new uint8[0x200];
				pStream->Read(m_pRemap[i], 0x200);
			}
			else
			{
				m_pRemap[i] = NULL;
			}
		}
	}
	else
	{
		memset(m_pRemap, 0, sizeof(m_pRemap));
	}

	pStream->GetPos(&nPos);

	// the characters
	memset(m_pChars, 0, sizeof(m_pChars));
	for (i = 0; i < 256; i++)
	{
		pStream->Read(&bPresent, 1);
		if (bPresent)
		{
			m_pChars[i] = (CUIWideFontChar**)new uint8[0x400];
			for (j = 0; j < 256; j++)
			{
				pStream->Read(&bPresent, 1);
				if (bPresent)
				{
					m_pChars[i][j] = (CUIWideFontChar*)new uint8[sizeof(CUIWideFontChar)];
					pStream->Read(m_pChars[i][j], sizeof(CUIWideFontChar));
				}
				else
				{
					m_pChars[i][j] = NULL;
				}
			}
		}
		else
		{
			m_pChars[i] = NULL;
		}
	}

	pStream->Read(&m_Valid, 1);

	// the texture pages
	for (i = 0; i < 256; i++)
	{
		if (i < m_nPages)
		{
			sprintf(sName, "\\interface\\fonts\\%s_%03i.dtx", m_Name, i);
			pTexInterface->CreateTextureFromName(m_hPages[i], sName);
		}
		else
		{
			m_hPages[i] = NULL;
		}
	}

	m_bLoaded = true;
	return true;
}


//	--------------------------------------------------------------------------
// RETAIL: 0x5681d0 (slot 17)
void CUIWideBitmapFont::Term()
{
	int i, j;

	for (i = 0; i < 256; i++)
	{
		if (m_pRemap[i])
		{
			delete [] (uint8*)m_pRemap[i];
			m_pRemap[i] = NULL;
		}
	}

	for (i = 0; i < 256; i++)
	{
		if (m_pChars[i])
		{
			for (j = 0; j < 256; j++)
			{
				if (m_pChars[i][j])
				{
					delete [] (uint8*)m_pChars[i][j];
					m_pChars[i][j] = NULL;
				}
			}
			delete [] (uint8*)m_pChars[i];
			m_pChars[i] = NULL;
		}
	}

	for (i = 0; i < 256; i++)
	{
		if (m_bPage[i])
		{
			pTexInterface->ReleaseTextureHandle(m_hPages[i]);
			m_hPages[i] = NULL;
		}
	}

	m_bLoaded = false;
}
