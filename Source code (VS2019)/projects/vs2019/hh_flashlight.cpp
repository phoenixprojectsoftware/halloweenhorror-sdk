/***
Code by RafaXIS
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum flashlight_e {
	FLASHLIGHT_IDLE1 = 0,
	FLASHLIGHT_IDLE2,
	FLASHLIGHT_IDLE3,
	FLASHLIGHT_SHOOT,
	FLASHLIGHT_SHOOT_EMPTY,
	FLASHLIGHT_RELOAD,
	FLASHLIGHT_RELOAD_NOT_EMPTY,
	FLASHLIGHT_DRAW,
	FLASHLIGHT_HOLSTER,
	FLASHLIGHT_ADD_SILENCER
};

LINK_ENTITY_TO_CLASS(weapon_flashlight, CFlashlight);



void CFlashlight::Spawn()
{
	pev->classname = MAKE_STRING("weapon_flashlight"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_FLASHLIGHT;
	SET_MODEL(ENT(pev), "models/weapons/flashlight/w_flashlight.mdl");

	m_iDefaultAmmo = FLASHLIGHT_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CFlashlight::Precache(void)
{
	PRECACHE_MODEL("models/weapons/flashlight/v_flashlight.mdl");
	PRECACHE_MODEL("models/weapons/flashlight/w_flashlight.mdl");
	PRECACHE_MODEL("models/weapons/flashlight/p_flashlight.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND("weapons/pl_gun3.wav");//handgun

	m_usFireFLASHLIGHT1 = PRECACHE_EVENT(1, "events/FLASHLIGHT1.sc");
	m_usFireFLASHLIGHT2 = PRECACHE_EVENT(1, "events/FLASHLIGHT2.sc");
}

int CFlashlight::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "battery";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = FLASHLIGHT_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_FLASHLIGHT;
	p->iWeight = FLASHLIGHT_WEIGHT;

	return 1;
}

BOOL CFlashlight::Deploy()
{
	// pev->body = 1;
	return DefaultDeploy("models/weapons/flashlight/v_flashlight.mdl", "models/weapons/flashlight/p_flashlight.mdl", FLASHLIGHT_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0);
}

void CFlashlight::SecondaryAttack(void)
{
}

void CFlashlight::PrimaryAttack(void)
{
	if (m_pPlayer->FlashlightIsOn())
	{
		m_pPlayer->FlashlightTurnOff();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim(FLASHLIGHT_SHOOT);
	}
	else
	{
		m_pPlayer->FlashlightTurnOn();
		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim(FLASHLIGHT_SHOOT);
	}
}

void CFlashlight::FLASHLIGHTFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->FlashlightIsOn())
	{
		m_pPlayer->FlashlightTurnOff();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim(FLASHLIGHT_SHOOT);
	}
	else
	{
		m_pPlayer->FlashlightTurnOn();
		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim(FLASHLIGHT_SHOOT);
	}
}


void CFlashlight::Reload(void)
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload(17, FLASHLIGHT_RELOAD, 1.5);
	else
		iResult = DefaultReload(17, FLASHLIGHT_RELOAD_NOT_EMPTY, 1.5);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
}



void CFlashlight::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = FLASHLIGHT_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = FLASHLIGHT_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = FLASHLIGHT_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim, 1);
	}
};








class CFlashlightAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_FLASHLIGHTclip, CFlashlightAmmo);
LINK_ENTITY_TO_CLASS(ammo_battery, CFlashlightAmmo);















