// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFconMonster.cpp
//
// PURPOSE : AIGoalFconMonster implementation
//
// RETAIL: Tron 2.0 only, written from retail Object.lto (TU 0x101f9530..
//         0x101f9fb0). Names of members, methods and tables are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalFconMonster.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AI.h"
#include "AISounds.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFconMonster, kGoal_FconMonster);

// RETAIL: the TU's tables (retail .data 0x1031263c...): the pieces hidden
// when an arm is destroyed, and the tentacles shown in its place.
static const char* s_aszArmPieces[2][4] =
{
	{ "L_Arm_Shell", "Popoff", "Prankster", "" },
	{ "R_Arm_Shell", "Baza", "Mesh_Ring", "MeshGlow" },
};
static const char* s_aszArmTentacles[2] = { "Tentacles_Left", "Tentacles_Right" };
static uint32 s_acArmPieces[2] = { 3, 4 };
static EnumAISoundType s_aeArmPainSounds[3] = { kAIS_FconPopoffPain, kAIS_FconBazaPain, kAIS_FconCrownPain };
static EnumAISoundType s_aeArmDeathSounds[2] = { kAIS_FconPopoffDeath, kAIS_FconBazaDeath };


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFconMonster::CAIGoalFconMonster
//
//	PURPOSE:	Constructor (RETAIL: 0x101f9910)
//
// ----------------------------------------------------------------------- //

CAIGoalFconMonster::CAIGoalFconMonster()
{
	m_afHitPoints[0] = m_afHitPoints[1] = 30.f;
	m_astrCommandObject[0] = "fconleft";
	m_astrCommandObject[1] = "fconright";
	m_abDestroyed[1] = false;
	m_abDestroyed[0] = false;
	m_abInvulnerable[1] = false;
	m_abInvulnerable[0] = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFconMonster::DestroyArm
//
//	PURPOSE:	Hide an arm's pieces, show its tentacles (RETAIL: 0x101f97b0)
//
// ----------------------------------------------------------------------- //

void CAIGoalFconMonster::DestroyArm(uint32 iArm)
{
	m_abDestroyed[iArm] = true;

	HMODELPIECE hPiece;
	for( uint32 iPiece = 0; iPiece < s_acArmPieces[iArm]; ++iPiece )
	{
		g_pModelLT->GetPiece( m_pAI->m_hObject, (char*)s_aszArmPieces[iArm][iPiece], hPiece );
		g_pModelLT->SetPieceHideStatus( m_pAI->m_hObject, hPiece, LTTRUE );
	}

	g_pModelLT->GetPiece( m_pAI->m_hObject, (char*)s_aszArmTentacles[iArm], hPiece );
	if( hPiece != INVALID_MODEL_PIECE )
	{
		g_pModelLT->SetPieceHideStatus( m_pAI->m_hObject, hPiece, LTFALSE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFconMonster::HandleDamage
//
//	PURPOSE:	Cry out when an arm (or the crown) is hit (RETAIL: 0x101f9860)
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFconMonster::HandleDamage(const DamageStruct& damage)
{
	ModelNode eModelNode = m_pAI->GetModelNodeLastHit();

	// RETAIL: also calls the AI's GetUserObject() and drops the result.
	HOBJECT hAI = m_pAI->GetUserObject();

	// RETAIL: 0x100c6660, the node's field 0x6c in Tron's skeleton nodes:
	// 0 left arm, 1 right arm, anything else the crown. Taken here to be the
	// node's hit location; not confirmed.
	uint32 iArm = g_pModelButeMgr->GetSkeletonNodeLocation( m_pAI->GetModelSkeleton(), eModelNode );
	if( iArm > 1 )
	{
		m_pAI->PlaySound( kAIS_FconCrownPain, LTTRUE );
		return LTFALSE;
	}

	if( m_afHitPoints[iArm] <= 0.f )
	{
		if( !m_abDestroyed[iArm] )
		{
			m_pAI->PlaySound( s_aeArmDeathSounds[iArm], LTTRUE );
		}
	}
	else if( !m_abDestroyed[iArm] )
	{
		m_pAI->PlaySound( s_aeArmPainSounds[iArm], LTTRUE );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFconMonster::Save / Load
//
//	PURPOSE:	Save / Load (RETAIL: 0x101f9b20 / 0x101f9bc0)
//
// ----------------------------------------------------------------------- //

void CAIGoalFconMonster::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_afHitPoints[0]);
	SAVE_FLOAT(m_afHitPoints[1]);
	SAVE_CHARSTRING(m_astrCommandObject[0].c_str());
	SAVE_CHARSTRING(m_astrCommandObject[1].c_str());
	SAVE_BOOL(m_abDestroyed[0]);
	SAVE_BOOL(m_abDestroyed[1]);
	SAVE_BOOL(m_abInvulnerable[0]);
	SAVE_BOOL(m_abInvulnerable[1]);
}

void CAIGoalFconMonster::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_afHitPoints[0]);
	LOAD_FLOAT(m_afHitPoints[1]);

	char szBuffer[256];
	LOAD_CHARSTRING(szBuffer, sizeof(szBuffer));
	m_astrCommandObject[0] = szBuffer;
	LOAD_CHARSTRING(szBuffer, sizeof(szBuffer));
	m_astrCommandObject[1] = szBuffer;

	LOAD_BOOL(m_abDestroyed[0]);
	LOAD_BOOL(m_abDestroyed[1]);
	LOAD_BOOL(m_abInvulnerable[0]);
	LOAD_BOOL(m_abInvulnerable[1]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFconMonster::HandleNameValuePair
//
//	PURPOSE:	Handles updating the goal's parameters (RETAIL: 0x101f9d10).
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFconMonster::HandleNameValuePair(const char *szName, const char *szValue)
{
	if ( !_stricmp(szName, "leftcommandobject") )
	{
		m_astrCommandObject[0] = szValue;
		return LTTRUE;
	}

	if ( !_stricmp(szName, "rightcommandobject") )
	{
		m_astrCommandObject[1] = szValue;
		return LTTRUE;
	}

	if ( !_stricmp(szName, "lefthitpoints") )
	{
		m_afHitPoints[0] = (LTFLOAT)atof(szValue);
		return LTTRUE;
	}

	if ( !_stricmp(szName, "righthitpoints") )
	{
		m_afHitPoints[1] = (LTFLOAT)atof(szValue);
		return LTTRUE;
	}

	if ( !_stricmp(szName, "LeftCanHurt") )
	{
		m_abInvulnerable[0] = ( atoi(szValue) != 1 );
	}

	if ( !_stricmp(szName, "RightCanHurt") )
	{
		m_abInvulnerable[1] = ( atoi(szValue) != 1 );
	}

	if ( !_stricmp(szName, "LeftArmDestroy") && ( atoi(szValue) == 1 ) )
	{
		DestroyArm(0);
		m_abDestroyed[0] = true;
		m_pAI->PlaySound( s_aeArmDeathSounds[0], LTTRUE );

		// RETAIL: then calls the AI's 0x101311f0 (an LTVector out, 0): a
		// Tron CAI method, not yet written.
	}

	if ( !_stricmp(szName, "RightArmDestroy") && ( atoi(szValue) == 1 ) )
	{
		DestroyArm(1);
		m_abDestroyed[1] = true;
		m_pAI->PlaySound( s_aeArmDeathSounds[1], LTTRUE );

		// RETAIL: then calls the AI's 0x101311f0, as above.
	}

	return LTTRUE;
}
