// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.h
//
// PURPOSE : ModelButeMgr definition
//
// CREATED : 05.22.1999
//
// ----------------------------------------------------------------------- //

#ifndef __MODELBUTE_MGR_H__
#define __MODELBUTE_MGR_H__

// Includes

#include "GameButeMgr.h"
#include "ClientServerShared.h"
#include "SurfaceDefs.h"
#include "..\shared\ButeListReader.h"
#include "DebugNew.h"
#include <string>
#include <vector>

// Defines

#define NODEFLAG_NONE			(0)		// This node has no special flags
#define	NODEFLAG_CRITICALHIT	(1)		// This node is a "critical hit" node
#define	NODEFLAG_TALK			(2)		// This node is a "talk" (mouth) node
#define	NODEFLAG_TIRE			(4)		// This node is a "tire" node
#define NODEFLAG_TURNING_TIRE	(8)		// This node is a "turning tire" node
#define NODEFLAG_ROTOR			(16)	// This node is a "rotor" node (for helicopters)
#define NODEFLAG_EYE			(32)	// This node is a "eye" node (for humans)
#define NODEFLAG_WALLSTICK		(64)	// This node is a "wall stick" node (for humans)

#define MBMGR_DEFAULT_FILE "Attributes\\ModelButes.txt"

// RETAIL: Tron's names are 48 characters (Object.lto 0x100c44f5 reads them
// with a length of 0x30).
#define MAX_MODELBMGR_NAME_LEN	48
#define	MAX_MODELBMGR_MAX_PATH	64

// Enums

enum ModelId
{
	eModelIdInvalid = 0xFF,
};

enum ModelSkeleton
{
	eModelSkeletonInvalid = 0xFF,
};

enum ModelNode
{
	eModelNodeInvalid = 0xFF,
};

enum ModelTrackingNodeGroup
{
	eModelTrackingNodeGroupInvalid = 0xFF,
};

enum ModelTrackingNode
{
	eModelTrackingNodeInvalid = 0xFF,
};

enum ModelType
{
	eModelTypeHuman,
	eModelTypeAnimal,
	eModelTypeVehicle,
	eModelTypeGenericProp,
	eModelTypeInvalid = 0xFF,
};

enum ModelNScript
{
	eModelNScriptInvalid = 0xFF,
};

// Classes

class CModelButeMgr;
extern CModelButeMgr* g_pModelButeMgr;

class CModelButeMgr : public CGameButeMgr
{
	private :

		// Not allowed to create directly.  Use Instance().
		CModelButeMgr();

		// Copy ctor and assignment operator not implemented and should never be used.
		CModelButeMgr( CModelButeMgr const& other );
		CModelButeMgr& operator=( CModelButeMgr const& other );

	public :

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		~CModelButeMgr();

		// Call this to get the singleton instance of the weapon mgr.
		static CModelButeMgr& Instance( );

	public : // Public member variables


        LTBOOL       Init(const char* szAttributeFile=MBMGR_DEFAULT_FILE);
		void		Term();

		CButeMgr*	GetButeMgr() { return &m_buteMgr; }

		// Models

		int				GetNumModels() { return m_cModels; }
		ModelId			GetModelId(const char *szName);
		const char*		GetModelName(ModelId eModelId);
		const char*		GetModelSoundTemplate(ModelId eModelId);
		const char*		GetModelAnimationMgr(ModelId eModelId);
		const char*		GetModelAIName(ModelId eModelId);
		// RETAIL: Tron's (the name is ours; Object.lto 0x100c5b80)
		const char*		GetModelHiddenPieces(ModelId eModelId);
		ModelType		GetModelType(ModelId eModelId);
		ModelSkeleton	GetModelSkeleton(ModelId eModelId);
		// RETAIL: Tron's (the name is ours; Object.lto 0x100c5c40)
		bool			GetModelSupportsDamageFX(ModelId eModelId);
        LTFLOAT         GetModelMass(ModelId eModelId);
        LTFLOAT         GetModelHitPoints(ModelId eModelId);
        LTFLOAT         GetModelMaxHitPoints(ModelId eModelId);
		LTFLOAT         GetModelEnergy(ModelId eModelId);
        LTFLOAT         GetModelMaxEnergy(ModelId eModelId);
        LTFLOAT         GetModelArmor(ModelId eModelId);
        LTFLOAT         GetModelMaxArmor(ModelId eModelId);
		// RETAIL: Tron's (the names are ours; Object.lto 0x100c5e40, 0x100c5e80)
		LTFLOAT			GetModelMinSearchTime(ModelId eModelId);
		LTFLOAT			GetModelMaxSearchTime(ModelId eModelId);
		const char*		GetModelLoudMovementSnd(ModelId eModelId);
		const char*		GetModelQuietMovementSnd(ModelId eModelId);
		// RETAIL: Tron's models have a range of name ids; each call gives
		// the next id of the range (Object.lto 0x100c5ec0).
		uint16			GetModelNameId(ModelId eModelId);
		// RETAIL: Tron's (the name is ours; Object.lto 0x100c60c0)
		void			ResetModelNameIds(uint32 nSeed);
		const char*		GetModelPlayerPainSndDir(ModelId eModelId);
		// RETAIL: Tron's (the names are ours; Object.lto 0x100c6160..0x100c61e0)
		const char*		GetModelRezInFX(ModelId eModelId);
		const char*		GetModelGibFX(ModelId eModelId);
		uint16			GetModelHitpointsDropped(ModelId eModelId);

		// Styles

		int				GetNumStyles() { return m_cStyles; }

		// Skeleton

		int				GetNumSkeletons() { return m_cSkeletons; }
		int				GetSkeletonNumNodes(ModelSkeleton eModelSkeleton);
		ModelNode		GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName);
		const char*		GetSkeletonDefaultFrontDeathAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultBackDeathAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultFrontShortRecoilAni(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonDefaultBackShortRecoilAni(ModelSkeleton eModelSkeleton);
		ModelNode		GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton);
		const char*		GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        uint32          GetSkeletonNodeFlags(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeFrontDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeBackDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeFrontShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		const char*		GetSkeletonNodeBackShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT         GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		ModelNode		GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
		HitLocation		GetSkeletonNodeLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT         GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        LTFLOAT         GetSkeletonNodeHitPriority(ModelSkeleton eModelSkeleton, ModelNode eModelNode);
        ModelTrackingNodeGroup GetSkeletonTrackingNodesLookAt(ModelSkeleton eModelSkeleton);
        ModelTrackingNodeGroup GetSkeletonTrackingNodesAimAt(ModelSkeleton eModelSkeleton);

		// Tracking Nodes

		LTBOOL			IsTrackingNodeGroupValid(ModelTrackingNodeGroup eModelTrackingNodeGroup);
		LTBOOL			IsTrackingNodeValid(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		int				GetNumTrackingNodes(ModelTrackingNodeGroup eModelTrackingNodeGroup);
		int				GetNumClonedTrackingNodes(ModelTrackingNodeGroup eModelTrackingNodeGroup);
		const char*		GetTrackingNodeName(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const char*		GetTrackingNodeClonedName(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		void			GetTrackingNodeForward(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode, LTVector* pvForward);
		void			GetTrackingNodeUp(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode, LTVector* pvUp);
		const LTBOOL	GetTrackingNodeAxesSpecified(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const LTFLOAT	GetTrackingNodeDiscomfortAngleX(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const LTFLOAT	GetTrackingNodeDiscomfortAngleY(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const LTFLOAT	GetTrackingNodeMaxAngleX(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const LTFLOAT	GetTrackingNodeMaxAngleY(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		const LTFLOAT	GetTrackingNodeMaxVelocity(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);
		// RETAIL: Tron's (the name is ours)
		const LTBOOL	GetTrackingNodeIgnoreParent(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode);

		// Node Scripts

		int				GetNumNodeScripts() { return m_cNScripts; }
		int				GetNScriptNumPts(ModelNScript eModelNScript);
		const char*		GetNScriptNodeName(ModelNScript eModelNScript);
        uint8           GetNScriptFlags(ModelNScript eModelNScript);
        LTFLOAT         GetNScriptPtTime(ModelNScript eModelNScript, int nPt);
        const LTVector& GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt);
        const LTVector& GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt);

		// Files

		const char*		GetMultiModelFilename(ModelId eModelId);
		const char*		GetModelFilename(ModelId eModelId);
		uint8			GetNumSkins(ModelId eModelId);
		const char*		GetSkinFilename(ModelId eModelId, uint8 iSkin);
		void			CopySkinFilenames(ModelId eModelId, uint8 iStart, char* paszDest, int strLen);
		const char*		GetHandsSkinFilename(ModelId eModelId);
		void			CopyRenderStyleFilenames(ModelId eModelId, ObjectCreateStruct* pCreateStruct);
		CButeListReader* GetSkinReader(ModelId eModelId);
		CButeListReader* GetRenderStyleReader(ModelId eModelId);
		uint8			GetNumClientFX(ModelId eModelId);
		const char*		GetClientFX(ModelId eModelId, uint8 iClientFX);
		void			CopyClientFX(ModelId eModelId, uint8 iStart, char* paszDest, int strLen);
		CButeListReader* GetClientFXReader(ModelId eModelId);
		uint8			GetNumAltHeadSkins( ModelId eModelId );
		const char*		GetAltHeadSkin( ModelId eModelId, uint8 iSkin );
		uint8			GetNumAltBodySkins( ModelId eModelId );
		const char*		GetAltBodySkin( ModelId eModelId, uint8 iSkin );

		// AI

		LTBOOL			IsModelAIOnly(ModelId eModelId);

		LTBOOL			CanModelBeCarried(ModelId eModelId);


		LTBOOL			IsModelTranslucent(ModelId eModelId);

		//access the different surface types for the object
		SurfaceType		GetArmorSurfaceType(ModelId eModelId);
		SurfaceType		GetFleshSurfaceType(ModelId eModelId);

		// Default Attachments

		uint8			GetNumDefaultAttachments(ModelId eModelId);
		void			GetDefaultAttachment(ModelId eModelId, uint8 iAttachment, const char*& pszAttachmentPosition, const char*& pszAttachment);

		// RETAIL: Tron's attachment substitutions (the name is ours;
		// Object.lto 0x100c7020): the substitute of an attachment, or the
		// attachment itself.
		const char*		GetAttachmentSubstitution(ModelId eModelId, const char* const& pszAttachment);

		// PlayerView Attachment
		
		uint8			GetNumPlayerViewAttachments( ModelId eModelId );
		void			GetPlayerViewAttachment( ModelId eModelId, uint8 iPVAttachment, const char*& pszPVAttachmentPosition, const char*& pszPVAttachment );


		LTFLOAT			GetUnalertDamageFactor( ModelId eModelId );


		// Multiplayer models

		// RETAIL: Tron's multiplayer models (the names are ours): a model,
		// its name id, and per color its skins, rez-in FX and disc FX.
		struct MPModel
		{
			ModelId		m_eModelId;
			int			m_nNameId;
			std::vector< std::vector<std::string> >	m_vecColorSkins;
			std::vector<std::string>	m_vecColorRezInFX;
			std::vector<std::string>	m_vecColorHHDiscFX;
		};

		int				GetNumMPModels() { return m_cMPModels; }
		// RETAIL: Object.lto 0x100c70f0
		const MPModel*	GetMPModel(int num);

		int				GetNumColorNameIds() { return m_vecColorNameIds.size(); }
		// RETAIL: CShell.dll 0x1014b5a0 (out of line; the name is ours)
		int				GetColorNameId(uint8 nColor);

		// PORT: NOLF2's callers of its cooperative, deathmatch and team
		// models; Tron has one list of multiplayer models.
		int				GetNumCPModels() { return m_cMPModels; }
		ModelId			GetCPModel(int num);
		int				GetNumDMModels() { return m_cMPModels; }
		ModelId			GetDMModel(int num);
		int				GetNumTeamModels() { return m_cMPModels; }
		ModelId			GetTeamModel(int num);
		int				GetTeamDefaultModel(int nTeam);


	protected : // Protected inner classes

		class CStyle;
		class CNode;
		class CSkeleton;
		class CModel;
		class CNScriptPt;
		class CNScript;
		class CTrackingNode;
		class CTrackingNodeGroup;

	protected : // Protected member variables

		int				m_cModels;
		CModel*			m_aModels;

		int				m_cStyles;
		CStyle*			m_aStyles;

		int				m_cSkeletons;
		CSkeleton*		m_aSkeletons;

		int					m_cTrackingNodeGroups;
		CTrackingNodeGroup*	m_aTrackingNodeGroups;

		int				m_cNScripts;
		CNScript*		m_aNScripts;

		// RETAIL: Tron's (CModelButeMgr is 0x1158 bytes, Object.lto 0x100c27e0)
		int				m_cMPModels;
		MPModel*		m_aMPModels;

		std::vector<int>	m_vecColorNameIds;

	protected :

		// RETAIL: Object.lto 0x100c5350 (the name is ours)
		bool			ReadMPModelColor(int iModel, int iColor);

};

// ----------------------------------------------------------------------- //
//
// !!!!!!!!!!!!!!!!  CModelButeMgr inner class definitions  !!!!!!!!!!!!!!
//
// ----------------------------------------------------------------------- //

class CModelButeMgr::CStyle
{
	public :

		CStyle()
		{
			m_szName[0] = 0;
		}

	public :

		char		m_szName[MAX_MODELBMGR_NAME_LEN];
};

class CModelButeMgr::CNode
{
	public :

		CNode()
		{
			m_szName[0] = 0;
			m_dwFlags = 0x0;
			m_fDamageFactor = 0.0f;
			m_eModelNodeParent = eModelNodeInvalid;
			m_eHitLocation = HL_UNKNOWN;
			m_fHitRadius = 10.0f;
			m_fHitPriority = 0.0f;
		}

	public :

		// RETAIL: Tron's nodes are 0x78 bytes (Object.lto 0x100c2d64); their
		// animations are lists, one of which is chosen at random.
		char			m_szName[MAX_MODELBMGR_NAME_LEN];
		std::vector<std::string>	m_lstFrontDeathAni;
		std::vector<std::string>	m_lstBackDeathAni;
		std::vector<std::string>	m_lstFrontShortRecoilAni;
		std::vector<std::string>	m_lstBackShortRecoilAni;
        uint32          m_dwFlags;
        LTFLOAT         m_fDamageFactor;
		ModelNode		m_eModelNodeParent;
		HitLocation		m_eHitLocation;
        LTFLOAT         m_fHitRadius;
        LTFLOAT         m_fHitPriority;
};

class CModelButeMgr::CSkeleton
{
	public :

		CSkeleton()
		{
			m_cNodes = 0;
            m_aNodes = LTNULL;
			m_eModelNodeDefaultHit = eModelNodeInvalid;
			m_eModelTrackingNodesLookAt = eModelTrackingNodeGroupInvalid;
			m_eModelTrackingNodesAimAt = eModelTrackingNodeGroupInvalid;
		}

	public :

		// RETAIL: Tron's skeletons are 0x44 bytes (Object.lto 0x100c5962)
		CNode*		m_aNodes;
		int			m_cNodes;
		std::vector<std::string>	m_lstDefaultFrontDeathAni;
		std::vector<std::string>	m_lstDefaultBackDeathAni;
		ModelNode	m_eModelNodeDefaultHit;
		std::vector<std::string>	m_lstDefaultFrontShortRecoilAni;
		std::vector<std::string>	m_lstDefaultBackShortRecoilAni;

		ModelTrackingNodeGroup	m_eModelTrackingNodesLookAt;
		ModelTrackingNodeGroup	m_eModelTrackingNodesAimAt;
};


class CModelButeMgr::CTrackingNode
{
	public :

		CTrackingNode()
		{
			m_szName[0] = 0;
			m_szClonedName[0] = 0;
			m_vForward = LTVector(0.f, 0.f, 0.f);
			m_vUp = LTVector(0.f, 0.f, 0.f);
			m_bAxesSpecified = LTFALSE;
			m_fDiscomfortAngleX = 0.f;
			m_fDiscomfortAngleY = 0.f;
			m_fMaxAngleX = 0.f;
			m_fMaxAngleY = 0.f;
			m_fMaxVelocity = 0.f;
			m_bIgnoreParent = LTFALSE;
		}

public:

	char			m_szName[MAX_MODELBMGR_NAME_LEN];
	char			m_szClonedName[MAX_MODELBMGR_NAME_LEN];
	LTVector		m_vForward;
	LTVector		m_vUp;
	LTBOOL			m_bAxesSpecified;
	LTFLOAT			m_fDiscomfortAngleX;
	LTFLOAT			m_fDiscomfortAngleY;
	LTFLOAT			m_fMaxAngleX;
	LTFLOAT			m_fMaxAngleY;
	LTFLOAT			m_fMaxVelocity;
	// RETAIL: Tron's (0x90; Object.lto 0x100c3ca8)
	LTBOOL			m_bIgnoreParent;
};

class CModelButeMgr::CTrackingNodeGroup
{
	public :

		CTrackingNodeGroup()
		{
			m_aTrackingNodes = LTNULL;
			m_cTrackingNodes = 0;
			m_cClonedTrackingNodes = 0;
		}

	public :

		CTrackingNode*	m_aTrackingNodes;
		int				m_cTrackingNodes;
		int				m_cClonedTrackingNodes;
};


class CModelButeMgr::CNScriptPt
{
	public :

		CNScriptPt()
		{
			m_fTime = 0.0f;
			m_vPosOffset.Init();
			m_vRotOffset.Init();
		}

	public :

        LTFLOAT      m_fTime;
        LTVector     m_vPosOffset;
        LTVector     m_vRotOffset;
};

class CModelButeMgr::CNScript
{
	public :

		CNScript()
		{
            m_aNScriptPts = LTNULL;
			m_cNScriptPts = 0;

			m_szName[0] = 0;
			m_bFlags = 0;
		}

	public:

		CNScriptPt*		m_aNScriptPts;
		int				m_cNScriptPts;

		char			m_szName[MAX_MODELBMGR_NAME_LEN];
        uint8           m_bFlags;
};

struct DefaultAttachmentStruct
{
	DefaultAttachmentStruct()
	:	szAttachmentPosition	( LTNULL ),
		szAttachment			( LTNULL )	
	{
	}

	char* szAttachmentPosition;
	char* szAttachment;
};

struct PlayerViewAttachmentStruct
{
	PlayerViewAttachmentStruct()
	:	szPVAttachmentPosition	( LTNULL ),
		szPVAttachment			( LTNULL )	
	{
	}

	char *szPVAttachmentPosition;
	char *szPVAttachment;
};

// RETAIL: Tron's attachment substitutions (the name is ours)
struct AttachmentSubstitutionStruct
{
	AttachmentSubstitutionStruct()
	:	szAttachment	( LTNULL ),
		szSubstitute	( LTNULL )
	{
	}

	char* szAttachment;
	char* szSubstitute;
};

typedef std::vector<DefaultAttachmentStruct> DEFAULT_ATTACHMENT_LIST;
typedef std::vector<AttachmentSubstitutionStruct> ATTACHMENT_SUBSTITUTION_LIST;
typedef std::vector<PlayerViewAttachmentStruct> PLAYERVIEW_ATTACHMENT_LIST;

class CModelButeMgr::CModel
{
	public :

		CModel()
		{
			m_szName[0] = 0;
			m_szSoundTemplate[0] = 0;
			m_eModelSkeleton = eModelSkeletonInvalid;
			m_eModelType = eModelTypeInvalid;
			m_fModelMass = 0.0f;
			m_fModelHitPoints = 0.0f;
			m_fModelMaxHitPoints = 0.0f;
			m_fModelEnergy = 0.0f;
			m_fModelMaxEnergy = 0.0f;
			m_fModelArmor = 0.0f;
			m_fModelMaxArmor = 0.0f;
			m_fMinSearchTime = 0.0f;
			m_fMaxSearchTime = 0.0f;
			m_szModelFile[0] = 0;
			m_szHandsSkin[0] = 0;
			m_szAIName[0] = 0;
			m_bAIOnly = LTTRUE;
			m_bSupportsDamageFX = LTFALSE;
			m_bCanBeCarried = LTTRUE;
			m_bTranslucent = LTFALSE;
			m_szAnimationMgr = LTNULL;
			m_fUnalertDamageFactor;
			m_szLoudMovementSnd[0] = 0;
			m_szQuietMovementSnd[0] = 0;
			m_szHiddenPieces[0] = 0;
			m_nNameIdMin = 0;
			m_nNameIdMax = 0;
			m_szPlayerPainSoundDir[0] = 0;
			m_szRezInFX[0] = 0;
			m_szGibFX[0] = 0;
			m_nHitpointsDropped = 0;
			m_eFleshSurfaceType = ST_FLESH;
			m_eArmorSurfaceType = ST_ARMOR;
		}

		~CModel()
		{
			debug_deletea(m_szAnimationMgr);

			DEFAULT_ATTACHMENT_LIST::iterator it;
			for( it = m_lstDefaultAttachments.begin(); it != m_lstDefaultAttachments.end(); ++it )
			{
				debug_deletea( it->szAttachmentPosition );
				debug_deletea( it->szAttachment );
			}

			ATTACHMENT_SUBSTITUTION_LIST::iterator iSub;
			for( iSub = m_lstAttachmentSubstitutions.begin(); iSub != m_lstAttachmentSubstitutions.end(); ++iSub )
			{
				debug_deletea( iSub->szAttachment );
				debug_deletea( iSub->szSubstitute );
			}

			PLAYERVIEW_ATTACHMENT_LIST::iterator iter;
			for( iter = m_lstPVAttachments.begin(); iter != m_lstPVAttachments.end(); ++iter )
			{
				debug_deletea( iter->szPVAttachmentPosition );
				debug_deletea( iter->szPVAttachment );
			}
		}

	public :

		// RETAIL: Tron's models are 0x3a0 bytes (Object.lto 0x100c50bd);
		// the offsets are from its Init.
		char			m_szName[MAX_MODELBMGR_NAME_LEN];
		char			m_szSoundTemplate[MAX_MODELBMGR_NAME_LEN];
		ModelSkeleton	m_eModelSkeleton;
		ModelType		m_eModelType;
        LTFLOAT         m_fModelMass;
        LTFLOAT         m_fModelHitPoints;
        LTFLOAT         m_fModelMaxHitPoints;
		LTFLOAT         m_fModelEnergy;
        LTFLOAT         m_fModelMaxEnergy;
        LTFLOAT         m_fModelArmor;
        LTFLOAT         m_fModelMaxArmor;
		LTFLOAT			m_fMinSearchTime;
		LTFLOAT			m_fMaxSearchTime;
		char			m_szModelFile[MAX_MODELBMGR_MAX_PATH];
		char			m_szHandsSkin[MAX_MODELBMGR_MAX_PATH];
		char			m_szAIName[MAX_MODELBMGR_NAME_LEN];
		LTBOOL			m_bAIOnly;
		LTBOOL			m_bSupportsDamageFX;
		LTBOOL			m_bCanBeCarried;
		LTBOOL			m_bTranslucent;
		CButeListReader m_blrSkinReader;
		CButeListReader m_blrRenderStyleReader;
		CButeListReader m_blrClientFXReader;
		char*			m_szAnimationMgr;
		LTFLOAT			m_fUnalertDamageFactor;
		char			m_szLoudMovementSnd[MAX_MODELBMGR_NAME_LEN];
		char			m_szQuietMovementSnd[MAX_MODELBMGR_NAME_LEN];
		CButeListReader	m_blrAltHeadSkin;
		CButeListReader	m_blrAltBodySkin;
		char			m_szHiddenPieces[256];
		uint16			m_nNameIdMin;
		uint16			m_nNameIdMax;
		char			m_szPlayerPainSoundDir[MAX_MODELBMGR_NAME_LEN];
		char			m_szRezInFX[MAX_MODELBMGR_NAME_LEN];
		char			m_szGibFX[MAX_MODELBMGR_NAME_LEN];
		uint16			m_nHitpointsDropped;
		SurfaceType		m_eFleshSurfaceType;
		SurfaceType		m_eArmorSurfaceType;
		
		DEFAULT_ATTACHMENT_LIST	m_lstDefaultAttachments;
		ATTACHMENT_SUBSTITUTION_LIST m_lstAttachmentSubstitutions;
		PLAYERVIEW_ATTACHMENT_LIST m_lstPVAttachments;
};


////////////////////////////////////////////////////////////////////////////
//
// CModelButeMgrPlugin is used to help facilitate populating the DEdit
// object properties that use CModelButeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#ifndef __PSX2
#include "iobjectplugin.h"

class CModelButeMgrPlugin : public IObjectPlugin
{
	public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

    void PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

        static LTBOOL            sm_bInitted;
};

#endif // !__PSX2
#endif // _CLIENTBUILD


#endif // __MODELBUTE_MGR_H__