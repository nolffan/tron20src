// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIFrame.h
//
// PURPOSE : Simple resizeable frame control
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUIFRAME_H_)
#define _LTGUIFRAME_H_


class CLTGUIFrame : public CLTGUICtrl
{
public:
	CLTGUIFrame();
	virtual ~CLTGUIFrame();

	// Create the control
    LTBOOL	Create (HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch = LTFALSE);
    LTBOOL	Create (uint32 argbColor, uint16 nWidth, uint16 nHeight);



    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);
	virtual void	SetTextureScale(float fScale);
	
	void SetFrame(HTEXTURE hFrame);
	void SetColor(uint32 argbColor);
	void SetSize(uint16 nWidth, uint16 nHeight);

	// Sets the width of the frames's border, set to 0 to not show the frame
	void SetBorder(uint8 nBorderWidth, uint32 nBorderColor);

	// TRON: a frame cut from source rectangles of its texture (in texels,
	// one per cell of the 3x3 grid) instead of NOLF2's fixed quarters, and
	// a hollow frame that does not draw its centre cell. The setters are
	// ours (no caller in retail sets these members out of line).
	void SetSourceRects(const LTRect *pRects)		{ for (int i = 0; i < 9; i++) m_rcSource[i] = pRects[i]; m_bUseSourceRects = true; }
	void SetHollow(bool bHollow)					{ m_bHollow = bHollow; }


	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }


protected:
	void			SetRenderState();
	void			InitPolies();
	void			ScalePolies();

	// TRON: the source rectangle versions of InitPolies/ScalePolies
	// (RETAIL: 0x100beb70, 0x100be760; names ours)
	void			InitSourceRectPolies();
	void			ScaleSourceRectPolies();

protected:

	LT_POLYGT4	m_Poly[9];
	HTEXTURE	m_hFrame;			// normal texture
	uint32		m_argbColor;
	LTFLOAT		m_fTextureScale;

	LTBOOL		m_bSimpleStretch;

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control
    uint16		m_nBaseWidth;          // The unscaled width of the control
    uint16		m_nBaseHeight;         // The unscaled height of the control

	uint8		m_nBorderWidth;
	uint32		m_BorderColor;
	LT_POLYF4	m_Border[4];

	// TRON: retail's 0x494..0x527 (RETAIL: the constructor 0x100bdc80;
	// names ours)
	bool		m_bHollow;					// Render skips the centre cell
	bool		m_bScaleSourceRects;		// set in the constructor; ScaleSourceRectPolies does nothing without it
	bool		m_bUseSourceRects;			// cells from m_rcSource
	LTRect		m_rcSource[9];				// texels of each cell, inclusive

};

#endif // _LTGUIFRAME_H_