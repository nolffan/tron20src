/****************************************************************************
;
;	MODULE:			GeneralInventory.h
;
;	PURPOSE:		General inventory system for characers
;
;	HISTORY:		3/20/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _GENERAL_INVENTORY_H_
#define _GENERAL_INVENTORY_H_

// Includes
// stl vector
#include <vector>

#define CHARACTER_MAX_INVENTORY	5
#define MAX_INVENTORY_ITEM_NAME_LEN	64
#define MAX_COMMAND_LEN 256

// Defines
#define ADD_GENERAL_INVENTORY_PROP(group) \
	PROP_DEFINEGROUP(GeneralInventory, group) \
	ADD_STRINGPROP_FLAG(Item1ID,0,group | PF_STATICLIST) \
	ADD_LONGINTPROP_FLAG(Item1Count,0,group)\
	ADD_STRINGPROP_FLAG(Item2ID,0,group | PF_STATICLIST) \
	ADD_LONGINTPROP_FLAG(Item2Count,0,group)\
	ADD_STRINGPROP_FLAG(Item3ID,0,group | PF_STATICLIST) \
	ADD_LONGINTPROP_FLAG(Item3Count,0,group)\
	ADD_STRINGPROP_FLAG(Item4ID,0,group | PF_STATICLIST) \
	ADD_LONGINTPROP_FLAG(Item4Count,0,group)\
	ADD_STRINGPROP_FLAG(Item5ID,0,group | PF_STATICLIST) \
	ADD_LONGINTPROP_FLAG(Item5Count,0,group)\

// Typedefs
typedef struct
{
	int		nItemID;
	uint8	nCount;
} GEN_INVENTORY_ITEM;

// RETAIL: Tron's (its CInventoryButeMgr::ReadItem, retail 0x100c1d70: Name,
// Image, Type, TextID, Points; 0xcc bytes); NOLF2's had a name and a command
#define MAX_INVENTORY_ITEM_IMAGE_LEN	128

typedef struct
{
	char	szName[MAX_INVENTORY_ITEM_NAME_LEN];
	char	szImage[MAX_INVENTORY_ITEM_IMAGE_LEN];
	int		nType;
	int		nTextID;
	int		nPoints;
} GEN_INVENTORY_ITEM_DEF;

typedef std::vector<GEN_INVENTORY_ITEM> GEN_INVENTORY_LIST;
typedef std::vector<GEN_INVENTORY_ITEM_DEF> GEN_INVENTORY_ITEM_DEF_LIST;

#endif // _GENERAL_INVENTORY_H_