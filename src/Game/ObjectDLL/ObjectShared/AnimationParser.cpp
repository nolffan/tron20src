
#include "StdAfx.h"
#include "AnimationParser.h"
#include "AnimationParse.h"
#include "AnimationLex.h"
#include "AnimationMgr.h"

#include "AnimationPropStrings.h"
#include "UberAssert.h"
#include "TRON/CaselessString.h"

using namespace Animation;

const uint32 animodDislocates = 0x01;

#define Output Debug

// RETAIL: Tron's string before the parser (0x10398508; Parse empties it).
// The name is ours. A CaselessString: its assignment checks for NULL
// (0x10232390)
static CaselessString s_sTronUnknown;

CAnimationParser* g_pParser = NULL;
static LTBOOL s_btError = LTFALSE;

void yyanimationerror(const char* szError)
{
//	char szShit[1024];
//	sprintf(szShit, "Parse error at line %d: %s\n", g_Lex.GetLine(), szError);
	s_btError = LTTRUE;
}

inline void ParserError(const char* szFormat, ...)
{
	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);
	// RETAIL: Tron prints the error (0x102324d0)
	g_pLTServer->CPrint(szBuffer);
}

// RETAIL: Tron keeps Build 51's ParserError as well (0x102327b0, its
// own buffer): the property sets report through it.  The name is ours
inline void ParserSetError(const char* szFormat, ...)
{
	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);
	yyanimationerror(szBuffer);
}

extern "C" int yyanimationwrap(void)
{
	return 1;
}

CAnimationParser::CAnimationParser()
{
	m_pAnimationCurrent = LTNULL;
	m_pTransitionCurrent = LTNULL;

	m_cAnimations	= 0;
	m_cTransitions	= 0;

	m_ePropertyGroupCurrent = kAPG_Invalid;

	m_pAnimMgr = LTNULL;
}


CAnimationParser::~CAnimationParser()
{
	if ( m_pAnimationCurrent )
	{
		ANIMATION* pAnimation = m_pAnimationCurrent;
		while ( pAnimation )
		{
			ANIMATION* pAnimationNext = pAnimation->pNext;
			debug_delete(pAnimation);
			pAnimation = pAnimationNext;
		}

		m_pAnimationCurrent = LTNULL;
	}
	if ( m_pTransitionCurrent )
	{
		TRANSITION* pTransition = m_pTransitionCurrent;
		while ( pTransition )
		{
			TRANSITION* pTransitionNext = pTransition->pNext;
			debug_delete(pTransition);
			pTransition = pTransitionNext;
		}

		m_pTransitionCurrent = LTNULL;
	}
}

LTBOOL CAnimationParser::Parse(const std::string& sFile, CAnimationMgr* pAnimMgr)
{
	// RETAIL: Tron's (0x10232370) resets the lexer and the string, and
	// keeps the errors that the parse reports
	g_Lex = CAnimationLex();

	s_btError = LTFALSE;
	s_sTronUnknown = "";

	g_pParser = this;

	m_pAnimMgr = pAnimMgr;
	m_pAnimMgr->GetPropertyMap()->clear();

	g_Lex.Lex(sFile.c_str());
/*
	#ifdef _REZFILE
	{
		BYTE* pbyItem = pItem->Load();
		char* pchBuffer = debug_new_array( char, pItem->GetSize() + 2);
		pchBuffer[pItem->GetSize()] = 0;
		pchBuffer[pItem->GetSize()+1] = 0;
		memcpy(pchBuffer, pbyItem, sizeof(BYTE)*pItem->GetSize());
		pItem->Unload();

		g_pParser = this;

		yyanimation_scan_buffer(pchBuffer);

		delete [] pchBuffer;
	}
	#else // #ifndef _REZFILE
	{
		yyanimationin = fopen(sFile, "rt");
		if ( !yyanimationin )
		{
			ParserError("Couldn't open %s", (const char*)sFile);
			return LTFALSE;
		}

		g_pParser = this;

		yyanimationparse();
	}
	#endif
*/
	return !s_btError;
}

void CAnimationParser::BeginProperties()
{
}

void CAnimationParser::AddPropertyGroup(const std::string& sName)
{
	m_ePropertyGroupCurrent = CAnimationMgrList::GetPropGroupFromName(sName.c_str());
	if(m_ePropertyGroupCurrent == kAPG_Invalid)
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::GetPropertyAndGroupEnums: Unrecognized property group: \"%s\"", sName.c_str() );
		ParserError(szBuffer);
		UBER_ASSERT( 0, szBuffer );

		// RETAIL: Tron's (0x10232480) reports it once
		return;
	}
}

void CAnimationParser::AddPropertyToCurrentGroup(const std::string& sName)
{
	EnumAnimProp eProp = CAnimationMgrList::GetPropFromName(sName.c_str());

	if( (eProp == kAP_Any) || (eProp == kAP_None) || ((eProp == kAP_Invalid) && (_strnicmp(s_aszAnimProp[kAP_Any], sName.c_str(), 3) != 0)) )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::GetPropertyAndGroupEnums: Unrecognized property in group \"%s\": \"%s\"", s_aszAnimPropGroup[m_ePropertyGroupCurrent], (const char*)sName.c_str() );
		ParserError(szBuffer);
		return;
	}

	m_pAnimMgr->GetPropertyMap()->insert( PROP_GROUP_MAP::value_type(eProp, m_ePropertyGroupCurrent) );
}

void CAnimationParser::BeginAnimations()
{
	m_pAnimationCurrent = NULL;
	m_pTransitionCurrent = NULL;
	m_cAnimations = 0;
}

void CAnimationParser::AddAnimation(const std::string& sName, const std::string& sMovementType)
{
	ANIMATION* pAnimation = debug_new(ANIMATION);
	pAnimation->pNext = m_pAnimationCurrent;
	pAnimation->sName = sName;
	pAnimation->eAnimMovement = CAnimationMgrList::GetAnimMovementFromName(sMovementType.c_str());

	++m_cAnimations;

	m_pAnimationCurrent = pAnimation;

	for(uint32 iProp=0; iProp < kAPG_Count; ++iProp)
	{
		pAnimation->eAnimProp[iProp] = kAP_None;
	}
}

void CAnimationParser::AddPropertyToCurrentAnimation(const std::string& sProperty)
{
	if ( !m_pAnimationCurrent )
	{
		ParserSetError("No animation specified before property set was declared");
		return;
	}

	// Default the Prop and the Group to Invalid, just in case the they are 
	// not specified in the the GetPropertyAndGroupEnums call.  
	EnumAnimProp eProp = kAP_Invalid;
	EnumAnimPropGroup eGroup = kAPG_Invalid;

	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// Insure good values were retrieved
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	// RETAIL: Tron's (0x10232710) does not test for a prop the group has
	// already
	else {
		m_pAnimationCurrent->eAnimProp[eGroup] = eProp;
	}
}


void CAnimationParser::GetPropertyAndGroupEnums(const std::string& sProperty, 
												EnumAnimPropGroup* peGroup, 
												EnumAnimProp* peProp)
{
	*peProp = CAnimationMgrList::GetPropFromName(sProperty.c_str());

	// If prop name wasn't found, but starts with "any", find group name in hash table.

	if( (*peProp == kAP_Invalid) && ( _strnicmp(s_aszAnimProp[kAP_Any], sProperty.c_str(), 3) == 0) )
	{
		*peGroup = CAnimationMgrList::GetPropGroupFromName(sProperty.c_str() + 3);
		*peProp = kAP_Any;
	}
	else if(*peProp == kAP_Any || *peProp == kAP_None || *peProp == kAP_Invalid)
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::GetPropertyAndGroupEnums: Unrecognized property: \"%s\"", (const char*)sProperty.c_str() );
		ParserError(szBuffer);
		UBER_ASSERT( 0, szBuffer );

		// RETAIL: Tron's (0x102328d8) returns invalid values
		*peGroup = kAPG_Invalid;
		*peProp = kAP_Invalid;
		return;
	}
	else 
	{
		PROP_GROUP_MAP::iterator it = m_pAnimMgr->GetPropertyMap()->find(*peProp);
		
		// Be sure that we really did find the peProp in the map
		if ( it == m_pAnimMgr->GetPropertyMap()->end() )
		{    
			char szBuffer[256];
			sprintf(szBuffer, "CAnimationParser::GetPropertyAndGroupEnums: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
			UBER_ASSERT( 0, szBuffer);
		}
		else
		{
			*peGroup = it->second;
		}
	}
}


void CAnimationParser::BeginTransitions()
{
	m_pTransitionCurrent = NULL;
	m_cTransitions = 0;
}

void CAnimationParser::AddTransition(const std::string& sName, const std::string& sMovementType)
{
	TRANSITION* pTransition = debug_new(TRANSITION);
	pTransition->pNext = m_pTransitionCurrent;
	pTransition->sName = sName;
	pTransition->eAnimMovement = CAnimationMgrList::GetAnimMovementFromName(sMovementType.c_str());

	++m_cTransitions;

	m_pTransitionCurrent = pTransition;

	for(uint32 iProp=0; iProp < kAPG_Count; ++iProp)
	{
		pTransition->eAnimPropInitialSet[iProp] = kAP_None;
		pTransition->eAnimPropAddSet[iProp]		= kAP_None;
		pTransition->eAnimPropRemoveSet[iProp]	= kAP_None;
		pTransition->eAnimPropConstantSet[iProp]= kAP_None;
		pTransition->eAnimPropNotSet[iProp]		= kAP_None;
	}
}

void CAnimationParser::AddPropertyToCurrentTransitionInitialSet(const std::string& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserSetError("No transition specified before property set was declared");
		return;
	}

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// RETAIL: Tron's tests the values (with the animation's message:
	// 0x102329b0 and the next ones)
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	else {
		m_pTransitionCurrent->eAnimPropInitialSet[eGroup] = eProp;
	}
}

void CAnimationParser::AddPropertyToCurrentTransitionAddSet(const std::string& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserSetError("No transition specified before property set was declared");
		return;
	}

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// RETAIL: Tron's tests the values (with the animation's message:
	// 0x102329b0 and the next ones)
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	else {
		m_pTransitionCurrent->eAnimPropAddSet[eGroup] = eProp;
	}
}

void CAnimationParser::AddPropertyToCurrentTransitionRemoveSet(const std::string& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserSetError("No transition specified before property set was declared");
		return;
	}

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// RETAIL: Tron's tests the values (with the animation's message:
	// 0x102329b0 and the next ones)
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	else {
		m_pTransitionCurrent->eAnimPropRemoveSet[eGroup] = eProp;
	}
}

void CAnimationParser::AddPropertyToCurrentTransitionConstantSet(const std::string& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserSetError("No transition specified before property set was declared");
		return;
	}

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// RETAIL: Tron's tests the values (with the animation's message:
	// 0x102329b0 and the next ones)
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	else {
		m_pTransitionCurrent->eAnimPropConstantSet[eGroup] = eProp;
	}
}

void CAnimationParser::AddPropertyToCurrentTransitionNotSet(const std::string& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserSetError("No transition specified before property set was declared");
		return;
	}

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	GetPropertyAndGroupEnums(sProperty, &eGroup, &eProp);

	// RETAIL: Tron's tests the values (with the animation's message:
	// 0x102329b0 and the next ones)
	if ( eProp == kAP_Invalid || eGroup == kAPG_Invalid )
	{
		char szBuffer[256];
		sprintf(szBuffer, "CAnimationParser::AddPropertyToCurrentAnimation: Could not find animation in map: %s\n", (const char*)sProperty.c_str() );
		UBER_ASSERT( 0, szBuffer);
	}
	else {
		m_pTransitionCurrent->eAnimPropNotSet[eGroup] = eProp;
	}
}

