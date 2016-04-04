#include "cbase.h"
#include "ai_basenpc.h"
#include "baseanimating.h"
#include "sprite.h"
#include "particle_parse.h"
#include "props.h"
#include "particle_system.h"
#include "player_pickup.h"

#define	CUBE_MODEL			"models/props/metal_box.mdl"
#define	REFLECTION_MODEL	"models/props/reflection_cube.mdl"
#define	SPHERE_MODEL		"models/props/metal_box.mdl"
#define	ANTIQUE_MODEL		"models/props_underground/underground_weighted_cube.mdl"

#define FIZZLE_SOUND		"Prop.Fizzled"

enum CubeType
{
	Standard = 0,
	Companion = 1,
	Reflective,
	Sphere,
	Antique,
};

enum SkinOld
{
	OLDStandard = 0,
	OLDCompanion = 1,
	OLDStandardActivated,
	OLDReflective,
	OLDSphere,
	OLDAntique
};

enum SkinType
{
	Clean = 0,
	Rusted = 1,
};

enum PaintPower
{
	Bounce = 0,
	Stick = 1,
	Speed,
	Portal,
	None,
};

class CPropWeightedCube : public CDynamicProp
{
public:
	DECLARE_CLASS(CPropWeightedCube, CDynamicProp);
	DECLARE_DATADESC();

	CPropWeightedCube()
	{
	}

	bool CreateVPhysics()
	{
		VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
		return true;
	}

	void Spawn(void);
	void Precache(void);

	//Use
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps();

	bool Dissolve(const char* materialName, float flStartTime, bool bNPCOnly, int nDissolveType, Vector vDissolverOrigin, int magnitude);

	void InputDissolve(inputdata_t &data);
	void InputSilentDissolve(inputdata_t &data);
	void InputPreDissolveJoke(inputdata_t &data);
	void InputExitDisabledState(inputdata_t &data);

	//Pickup
	//void Pickup(void);
	void OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);
	void OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t reason);
private:
	int	m_cubeType;
	int m_skinType;
	int m_paintPower;
	bool m_useNewSkins;
	bool m_allowFunnel;
	//int m_oldSkin;

	CHandle<CBasePlayer> m_hPhysicsAttacker;

	COutputEvent m_OnOrangePickup;
	COutputEvent m_OnBluePickup;
	COutputEvent m_OnPlayerPickup;

	COutputEvent m_OnPainted;

	COutputEvent m_OnPhysGunDrop;

	COutputEvent m_OnFizzled;
};

LINK_ENTITY_TO_CLASS(prop_weighted_cube, CPropWeightedCube);

BEGIN_DATADESC(CPropWeightedCube)

// Save/load
DEFINE_USEFUNC(Use),

//DEFINE_KEYFIELD(m_oldSkin, FIELD_INTEGER, "skin"),
DEFINE_KEYFIELD(m_cubeType, FIELD_INTEGER, "CubeType"),
DEFINE_KEYFIELD(m_skinType, FIELD_INTEGER, "SkinType"),
DEFINE_KEYFIELD(m_paintPower, FIELD_INTEGER, "PaintPower"),
DEFINE_KEYFIELD(m_useNewSkins, FIELD_BOOLEAN, "NewSkins"),
DEFINE_KEYFIELD(m_allowFunnel, FIELD_BOOLEAN, "allowfunnel"),

DEFINE_INPUTFUNC(FIELD_VOID, "Dissolve", InputDissolve),
DEFINE_INPUTFUNC(FIELD_VOID, "SilentDissolve", InputSilentDissolve),
DEFINE_INPUTFUNC(FIELD_VOID, "PreDissolveJoke", InputPreDissolveJoke),
DEFINE_INPUTFUNC(FIELD_VOID, "ExitDisabledState", InputExitDisabledState),

// Output
DEFINE_OUTPUT(m_OnPainted, "OnPainted"),
DEFINE_OUTPUT(m_OnFizzled, "OnFizzled"),
DEFINE_OUTPUT(m_OnOrangePickup, "OnOrangePickup"),
DEFINE_OUTPUT(m_OnBluePickup, "OnBluePickup"),
DEFINE_OUTPUT(m_OnPlayerPickup, "OnPlayerPickup"),
DEFINE_OUTPUT(m_OnPhysGunDrop, "OnPhysGunDrop"),

END_DATADESC()

void CPropWeightedCube::Precache(void)
{
	PrecacheModel(CUBE_MODEL);
	PrecacheModel(REFLECTION_MODEL);
	PrecacheModel(SPHERE_MODEL);
	PrecacheModel(ANTIQUE_MODEL);
	PrecacheScriptSound(FIZZLE_SOUND);
	BaseClass::Precache();
}

void CPropWeightedCube::InputPreDissolveJoke(inputdata_t &data)
{
	// Sets some VO to do before fizzling.
}

void CPropWeightedCube::InputExitDisabledState(inputdata_t &data)
{
	// Exits the disabled state of a reflector cube.
}


void CPropWeightedCube::InputDissolve(inputdata_t &data)
{
	Dissolve(NULL, gpGlobals->curtime, false, 0, GetAbsOrigin(), 1);
	EmitSound(FIZZLE_SOUND);
}

void CPropWeightedCube::InputSilentDissolve(inputdata_t &data)
{
	Dissolve(NULL, gpGlobals->curtime, false, 0, GetAbsOrigin(), 1);
}


void CPropWeightedCube::Spawn(void)
{
	Precache();
	BaseClass::Spawn();
	if (m_useNewSkins)
	{
		m_nSkin = 0;
		switch (m_cubeType) 
		{
		case Standard:
			SetModel(CUBE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		case Companion:
			SetModel(CUBE_MODEL);
			m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 10;
			if (m_paintPower == Speed)
				m_nSkin = 11;
			break;
		case Reflective:
			SetModel(REFLECTION_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 2;
			if (m_paintPower == Speed)
				m_nSkin = 3;
			break;
		case Sphere:
			SetModel(SPHERE_MODEL);
			break;
		case Antique:
			SetModel(ANTIQUE_MODEL);
			if (m_paintPower == Stick)
				m_nSkin = 1;
			if (m_paintPower == Speed)
				m_nSkin = 2;
			break;
		}
	}
	else
	{
		switch (m_nSkin)
		{
		case OLDStandard:
			SetModel(CUBE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		case OLDCompanion:
			SetModel(CUBE_MODEL);
			m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 10;
			if (m_paintPower == Speed)
				m_nSkin = 11;
			break;
		case OLDReflective:
			SetModel(REFLECTION_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 2;
			if (m_paintPower == Speed)
				m_nSkin = 3;
			break;
		case OLDSphere:
			SetModel(SPHERE_MODEL);
			break;
		case OLDAntique:
			SetModel(ANTIQUE_MODEL);
			if (m_paintPower == Stick)
				m_nSkin = 1;
			if (m_paintPower == Speed)
				m_nSkin = 2;
			break;
		}
	}
	SetSolid(SOLID_VPHYSICS);

	// In order to pick it up, needs to be physics.
	CreateVPhysics();

	SetUse(&CPropWeightedCube::Use);
}

int CPropWeightedCube::ObjectCaps()
{
	int caps = BaseClass::ObjectCaps();

	caps |= FCAP_IMPULSE_USE;

	return caps;
}

bool CPropWeightedCube::Dissolve(const char* materialName, float flStartTime, bool bNPCOnly, int nDissolveType, Vector vDissolverOrigin, int magnitude)
{
	m_OnFizzled.FireOutput(this, this);
	return BaseClass::Dissolve(materialName, flStartTime, bNPCOnly, nDissolveType, vDissolverOrigin, magnitude);
}

void CPropWeightedCube::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBasePlayer *pPlayer = ToBasePlayer(pActivator);
	if (pPlayer)
	{
		pPlayer->PickupObject(this);
	}
}

void CPropWeightedCube::OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason)
{
	m_hPhysicsAttacker = pPhysGunUser;

	if (reason == PICKED_UP_BY_CANNON || reason == PICKED_UP_BY_PLAYER)
	{
		m_OnPlayerPickup.FireOutput(pPhysGunUser, this);
	}
}

void CPropWeightedCube::OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t reason)
{
	m_OnPhysGunDrop.FireOutput(pPhysGunUser, this);
}
