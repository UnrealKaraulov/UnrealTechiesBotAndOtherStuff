//#define DEBUG
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
//#define BOTDEBUG
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <thread>  
#include <fstream> // debug
#include <cmath>
#include <set>
#include <filesystem>

#include "IniReader.h"
#include "IniWriter.h"

#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0

//#define BOTDEBUG
void PrintDebugInfo(const char* debuginfo);
int IsTypeIdEqual(unsigned char* unit_or_item_addr, int classid);
void GetUnitLocation3D(unsigned char* unitaddr, float& x, float& y, float& z);
unsigned char* GetItemInSlot(unsigned char* unitaddr, unsigned int slotnum);
int GetUnitAbilityLevel(unsigned char* unitaddr, int id, int checkavaiable = 0);
int __stdcall IsNotBadUnit(unsigned char* unitaddr, int onlymem = 0);
float GetProtectForUnit(unsigned char* unitaddr);
int GetUnitHPPercent(unsigned char* unitaddr);
int GetObjectTypeId(unsigned char* unit_or_item_addr);
void TextPrint(const char* szText, float fDuration);
int GetUnitOwnerSlot(unsigned char* unitaddr);
std::vector<unsigned char*> GetUnitsArray();
int GetLocalPlayerNumber();
bool IsHero(unsigned char* unitaddr);
std::vector<unsigned char*> GetOwnerHeroesArray();

std::string techiesBotFileName = "techies";

bool IsInGame = false;
bool IsBotInitialized = false;
bool IsBotStarted = false;
bool IsHotkeyPress = false;

long long InitializeTime = 0;
long long GameStartTime = 0;
long long CurTickCount = 0;

long long LastChatAccess = 0;
long long ForceDetonateTime = 0;
long long LastTechiesWork = 0;


long long LastDmgTime = 0;

// Game.dll address
unsigned char* GameDll = 0;
unsigned char* _W3XGlobalClass;

const char* MapFileName = "dota";

int UseWarnIsBadReadPtr = 1;
int DebugActive = 0;

unsigned char* forceunitaddr = 0;

bool ANY_TECHIES_FOUND = false;


std::vector<unsigned char*> unitstoselect;


struct TechiesActionStruct
{
	int action;
	int data;
};
bool FunModeEnabled = false;

int _Daggercooldown = 0;
int _Forcecooldown = 0;

DWORD latestvalue1 = 0;
DWORD latestvalue2 = 0;

char TechiesCrash[150];

bool ExpertModeEnabled = false;
bool Enable3DPoint = true;
bool StealthMode = false;


struct BombStruct
{
	unsigned char* unitaddr;
	float dmg;
	float dmg2;
	float range1;
	float range2;
	float x;
	float y;
	float z;
	bool remote;
	bool is_stasis;
};
std::vector<BombStruct> BombList;


struct BombConfigStruct
{
	int unit_typeid;
	float dmg;
	float dmg2;
	float range1;
	float range2;
	bool remote;
	float lvl_multiplier; // lvl_skillid multiplier
	int lvl_skillid; // lvl 
	float result_dmg1; // base at dmg1 * lvl_multiplier
	float result_dmg2; // base at dmg2 * lvl_multiplier
};

struct UnitConfigStruct
{
	int unit_typeid;
	int protect_type;
	float protectmult;
};

struct ItemConfigStruct
{
	int item_typeid;
	int protect_type;
	float protectmult;
};

struct AbilConfigStruct
{
	int abil_typeid;
	int type;
	// for type 2 or 3
	int unit_typeid;
	int protect_type;
	std::vector<float> protectmult;
};

bool customGameThread = false;
std::string GameDllName = "Game.dll";

CIniWriter* maincfg_write = NULL;
CIniReader* maincfg_read = NULL;

CIniWriter* mapcfg_write = NULL;
CIniReader* mapcfg_read = NULL;

int EnableAutoExplode = 1;
bool EnableDagger = 0;
bool EnableForceStaff = 1;


int StasisUnitId = 0;
bool StatisForceStaff = false;

bool AutoSuicide = false;
std::vector<int> UseBeforePlaceItems;

int BaseDmgReducing = 10;

float BaseDmgReducingMagic = 0.75;
float BaseDmgReducingPhys = 1.0;

struct TechiesUnitAbilStr
{
	int unitid;
	int abilid;
};

bool RemoteTechiesFound = false;

std::vector<TechiesUnitAbilStr> techies_ids;

std::vector<BombConfigStruct> techies_bombs;

int HuskarPersonalAbilId = 'A0QQ';

std::vector<UnitConfigStruct> protection_list_units;
std::vector<ItemConfigStruct> protection_list_items;
std::vector<AbilConfigStruct> protection_list_abils;

std::string _internal_GetStringFromTypeId(char* type_id)
{
	char type_str[5] = { type_id[3],type_id[2],type_id[1],type_id[0], '\0' };
	return std::string(type_str);
}

std::string GetStringFromTypeId(int type_id)
{
	return _internal_GetStringFromTypeId((char*)&type_id);
}


bool IsUnitProtected(unsigned char* unitaddr, int damagetype, std::string& bufflist)
{
	int input_typeid = GetObjectTypeId(unitaddr);
	for (auto& protect : protection_list_units)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			if (protect.protectmult < 0.001f)
			{
				if (input_typeid == protect.unit_typeid)
				{
					if (protect.protect_type == 0)
						bufflist += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):ALL 100%]";
					else if (protect.protect_type == 1)
						bufflist += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):MAGIC 100%]";
					else
						bufflist += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):PHYS 100%]";
					return true;
				}
			}
		}
	}

	for (auto& protect : protection_list_items)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			if (protect.protectmult < 0.001f)
			{
				for (int i = 0; i < 6; i++)
				{
					unsigned char* nitem = GetItemInSlot(unitaddr, i);
					if (nitem)
					{
						if (IsTypeIdEqual(nitem, protect.item_typeid))
						{
							if (protect.protect_type == 0)
								bufflist += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):ALL 100%]";
							else if (protect.protect_type == 1)
								bufflist += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):MAGIC 100%]";
							else
								bufflist += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):PHYS 100%]";
							return true;
						}
					}
				}
			}
		}
	}


	for (auto& protect : protection_list_abils)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			if (protect.protectmult.size() == 1 && protect.protectmult[0] < 0.001f)
			{
				if (protect.type >= 2)
				{
					if (input_typeid != protect.unit_typeid)
					{
						continue;
					}
				}

				bool fullprotected = false;
				if (protect.type == 0 && GetUnitAbilityLevel(unitaddr, protect.abil_typeid))
				{
					fullprotected = true;
				}
				else if (protect.type == 1 && GetUnitAbilityLevel(unitaddr, protect.abil_typeid, 1))
				{
					fullprotected = true;
				}
				else if (protect.type == 2 && !GetUnitAbilityLevel(unitaddr, protect.abil_typeid, 1))
				{
					fullprotected = true;
				}
				else if (protect.type == 3 && !GetUnitAbilityLevel(unitaddr, protect.abil_typeid))
				{
					fullprotected = true;
				}

				if (fullprotected)
				{
					if (protect.protect_type == 0)
						bufflist += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):ALL 100%]";
					else if (protect.protect_type == 1)
						bufflist += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):MAGIC 100%]";
					else
						bufflist += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):PHYS 100%]";
					return true;
				}

			}
		}
	}

	return false;
}

char tmpProtBuf[32];
std::string GetProtectString(float protect)
{
	sprintf_s(tmpProtBuf, "%f", ((1.0f - protect) * 100.0f));
	return std::string(tmpProtBuf);
}


std::string PrintBuffListStr;

const int DMG_TYPE_MAGIC = 1;
const int DMG_TYPE_PHYS = 2;

float GetUnitDamageWithProtection(unsigned char* unitaddr, int damagetype, float input_dmg)
{
	PrintBuffListStr.clear();

	if (!IsNotBadUnit(unitaddr, 1))
	{
		return 0.0f;
	}

	if (!IsNotBadUnit(unitaddr))
	{
		return 0.0f;
	}

	if (IsUnitProtected(unitaddr, damagetype, PrintBuffListStr))
		return 0.0f;


	float output_dmg = input_dmg;

	if (damagetype == DMG_TYPE_PHYS)
	{
		output_dmg = output_dmg * BaseDmgReducingPhys;
		output_dmg = output_dmg - (output_dmg * GetProtectForUnit(unitaddr));
	}
	else
	{
		output_dmg = output_dmg * BaseDmgReducingMagic;
	}

	int input_typeid = GetObjectTypeId(unitaddr);

	for (auto& protect : protection_list_units)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			if (input_typeid == protect.unit_typeid)
			{
				if (protect.protect_type == 0)
					PrintBuffListStr += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):ALL " + GetProtectString(protect.protectmult) + "%]";
				else if (protect.protect_type == 1)
					PrintBuffListStr += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):MAGIC " + GetProtectString(protect.protectmult) + "%]";
				else
					PrintBuffListStr += "[Unit (" + GetStringFromTypeId(protect.unit_typeid) + "):PHYS " + GetProtectString(protect.protectmult) + "%]";

				output_dmg *= protect.protectmult;
			}
		}
	}

	for (auto& protect : protection_list_items)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			for (int i = 0; i < 6; i++)
			{
				unsigned char* nitem = GetItemInSlot(unitaddr, i);
				if (nitem)
				{
					if (IsTypeIdEqual(nitem, protect.item_typeid))
					{
						if (protect.protect_type == 0)
							PrintBuffListStr += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):ALL " + GetProtectString(protect.protectmult) + "%]";
						else if (protect.protect_type == 1)
							PrintBuffListStr += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):MAGIC " + GetProtectString(protect.protectmult) + "%]";
						else
							PrintBuffListStr += "[Item (" + GetStringFromTypeId(protect.item_typeid) + "):PHYS " + GetProtectString(protect.protectmult) + "%]";

						output_dmg *= protect.protectmult;
					}
				}
			}
		}
	}


	for (auto& protect : protection_list_abils)
	{
		if (protect.protect_type == 0 || protect.protect_type == damagetype)
		{
			if (protect.type >= 2)
			{
				if (input_typeid != protect.unit_typeid)
				{
					continue;
				}
			}

			float target_protect = 0.0f;
			bool protect_found = false;
			if (protect.type == 0)
			{
				int abilLevel = GetUnitAbilityLevel(unitaddr, protect.abil_typeid);
				if (abilLevel)
				{
					if (protect.protectmult.size() == 1)
					{
						target_protect = protect.protectmult[0];
					}
					else if (abilLevel > (int)protect.protectmult.size())
					{
						target_protect = protect.protectmult[protect.protectmult.size() - 1];
					}
					else
					{
						target_protect = protect.protectmult[abilLevel - 1];
					}
					protect_found = true;
				}
			}
			else if (protect.type == 1)
			{
				int abilLevel = GetUnitAbilityLevel(unitaddr, protect.abil_typeid, 1);
				if (abilLevel)
				{
					if (protect.protectmult.size() == 1)
					{
						target_protect = protect.protectmult[0];
					}
					else if (abilLevel > (int)protect.protectmult.size())
					{
						target_protect = protect.protectmult[protect.protectmult.size() - 1];
					}
					else
					{
						target_protect = protect.protectmult[abilLevel - 1];
					}
					protect_found = true;
				}
			}
			else if (protect.type == 2 && !GetUnitAbilityLevel(unitaddr, protect.abil_typeid, 1))
			{
				target_protect = protect.protectmult[0];
				protect_found = true;
			}
			else if (protect.type == 3 && !GetUnitAbilityLevel(unitaddr, protect.abil_typeid))
			{
				target_protect = protect.protectmult[0];
				protect_found = true;
			}

			if (protect_found)
			{
				output_dmg *= target_protect;

				if (protect.protect_type == 0)
					PrintBuffListStr += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):ALL " + GetProtectString(target_protect) + "%]";
				else if (protect.protect_type == 1)
					PrintBuffListStr += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):MAGIC " + GetProtectString(target_protect) + "%]";
				else
					PrintBuffListStr += "[Abil (" + GetStringFromTypeId(protect.abil_typeid) + "):PHYS " + GetProtectString(target_protect) + "%]";
			}
		}
	}

	if (damagetype == 1)
	{
		int huskarlvl = GetUnitAbilityLevel(unitaddr, HuskarPersonalAbilId);
		if (huskarlvl)
		{
			float dmgprotectlvl = (float)huskarlvl + 3.0f;
			int hppercent = 100 - GetUnitHPPercent(unitaddr);
			int parts = hppercent / 7 + 1;
			dmgprotectlvl = dmgprotectlvl * parts;

			PrintBuffListStr += std::string("[HUSKAR ") + std::to_string((int)dmgprotectlvl) + std::string("%]");
			output_dmg = output_dmg * ((100.0f - dmgprotectlvl) / 100.0f);
		}
	}

	return output_dmg;
}


bool GetBombStructFromCfg(unsigned char* unitaddr, BombStruct& tmpBombStruct)
{
	if (!unitaddr)
		return false;

	int input_typeid = GetObjectTypeId(unitaddr);
	int input_owner = GetUnitOwnerSlot(unitaddr);

	if (input_typeid == StasisUnitId)
	{
		tmpBombStruct = BombStruct();
		tmpBombStruct.unitaddr = unitaddr;
		tmpBombStruct.dmg = 0.0f;
		tmpBombStruct.dmg2 = 0.0f;
		tmpBombStruct.range1 = 300;
		tmpBombStruct.range2 = 300;
		tmpBombStruct.remote = false;
		tmpBombStruct.is_stasis = true;
		GetUnitLocation3D(unitaddr, tmpBombStruct.x, tmpBombStruct.y, tmpBombStruct.z);
		return true;
	}

	tmpBombStruct.is_stasis = false;

	for (auto& bomb : techies_bombs)
	{
		if (input_typeid == bomb.unit_typeid)
		{
			// ignore remote bombs if techies not owned
			if (bomb.remote)
			{
				if (input_owner != GetLocalPlayerNumber())
					continue;
				RemoteTechiesFound = true;
			}

			tmpBombStruct = BombStruct();
			tmpBombStruct.unitaddr = unitaddr;
			tmpBombStruct.dmg = bomb.dmg;
			tmpBombStruct.dmg2 = bomb.dmg2;
			tmpBombStruct.range1 = bomb.range1;
			tmpBombStruct.range2 = bomb.range2;
			tmpBombStruct.remote = bomb.remote;

			GetUnitLocation3D(unitaddr, tmpBombStruct.x, tmpBombStruct.y, tmpBombStruct.z);

			if (bomb.lvl_skillid != 0)
			{
				auto units = GetOwnerHeroesArray();
				for (auto unit : units)
				{
					int abillevel = GetUnitAbilityLevel(unit, bomb.lvl_skillid);
					if (abillevel > 0)
					{
						float outmultiplier = bomb.lvl_multiplier * abillevel;
						tmpBombStruct.dmg = tmpBombStruct.dmg + outmultiplier;
						tmpBombStruct.dmg2 = tmpBombStruct.dmg2 + outmultiplier;
						break;
					}
				}
			}

			return true;
		}
	}
	return false;
}

int DaggerItemId = 0;
int ForceStaffItemId = 0;

int DaggerAbilId = 0;
int ForceStaffAbilId = 0;

float DaggerDistance = 0.0;
float ForceStaffDistance = 0.0;

int DetonateCommand = 0;


int SuicideAbilId = 0;
int SuicideCommand = 0;
float SuicideDmg[4] = { 0.0f, };
float SuicidePartDmg[4] = { 0.0f, };
float SuicideDistanceFull = 0.0f;
float SuicideDistancePart = 0.0f;



void LoadDefaultDotaConfiguration()
{
	techies_ids.push_back({ 'H00K', 0 });
	DaggerItemId = 'I04H';
	DaggerAbilId = 'AIbk';
	ForceStaffItemId = 'I0HI';
	ForceStaffAbilId = 'A19M';
	DaggerDistance = 940.0;
	ForceStaffDistance = 640.0;
	DetonateCommand = 0xd024c;
	StasisUnitId = 'otot';

	UnitConfigStruct tmpUnitStruct = UnitConfigStruct();
	tmpUnitStruct.unit_typeid = 'H00I';
	tmpUnitStruct.protect_type = 1;
	tmpUnitStruct.protectmult = 0.90f;
	protection_list_units.push_back(tmpUnitStruct);
	tmpUnitStruct.unit_typeid = 'H00J';
	protection_list_units.push_back(tmpUnitStruct);


	ItemConfigStruct tmpItemStruct = ItemConfigStruct();
	tmpItemStruct.item_typeid = 'I04P';
	tmpItemStruct.protect_type = 1;
	tmpItemStruct.protectmult = 0.85f;
	protection_list_items.push_back(tmpItemStruct);
	tmpItemStruct.item_typeid = 'I0K6';
	tmpItemStruct.protect_type = 1;
	tmpItemStruct.protectmult = 0.70f;
	protection_list_items.push_back(tmpItemStruct);

	AbilConfigStruct tmpAbilStruct = AbilConfigStruct();
	tmpAbilStruct.abil_typeid = 'B07D';
	tmpAbilStruct.protect_type = 0;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'B08J';
	tmpAbilStruct.protect_type = 0;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'B05S';
	tmpAbilStruct.protect_type = 0;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);

	tmpAbilStruct.abil_typeid = 'B014';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'B041';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'B0FP';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'A2T4';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);
	tmpAbilStruct.abil_typeid = 'B0FQ';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);

	// ethernal form 0% phys dmg
	tmpAbilStruct.abil_typeid = 'Aetl';
	tmpAbilStruct.protect_type = 2;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);

	// ethernal form 150% magic damage
	tmpAbilStruct.abil_typeid = 'Aetl';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(1.5f);
	protection_list_abils.push_back(tmpAbilStruct);

	// Jugger
	tmpAbilStruct.abil_typeid = 'A05G';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 2;
	tmpAbilStruct.unit_typeid = 'Nbbc';
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.0f);
	protection_list_abils.push_back(tmpAbilStruct);

	// medusa
	tmpAbilStruct.abil_typeid = 'BNms';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.5f);
	protection_list_abils.push_back(tmpAbilStruct);

	// admiral captain 
	tmpAbilStruct.abil_typeid = 'B09U';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.5f);
	protection_list_abils.push_back(tmpAbilStruct);

	// sven
	tmpAbilStruct.abil_typeid = 'B0BV';
	tmpAbilStruct.protect_type = 2;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.96f);
	tmpAbilStruct.protectmult.push_back(0.92f);
	tmpAbilStruct.protectmult.push_back(0.88f);
	tmpAbilStruct.protectmult.push_back(0.84f);
	protection_list_abils.push_back(tmpAbilStruct);

	// pudge
	tmpAbilStruct.abil_typeid = 'A06D';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.94f);
	tmpAbilStruct.protectmult.push_back(0.90f);
	tmpAbilStruct.protectmult.push_back(0.86f);
	tmpAbilStruct.protectmult.push_back(0.82f);
	protection_list_abils.push_back(tmpAbilStruct);

	// rubic
	tmpAbilStruct.abil_typeid = 'B0EO';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.95f);
	tmpAbilStruct.protectmult.push_back(0.90f);
	tmpAbilStruct.protectmult.push_back(0.85f);
	tmpAbilStruct.protectmult.push_back(0.80f);
	protection_list_abils.push_back(tmpAbilStruct);

	// spectra
	tmpAbilStruct.abil_typeid = 'A0NA';
	tmpAbilStruct.protect_type = 0;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.90f);
	tmpAbilStruct.protectmult.push_back(0.84f);
	tmpAbilStruct.protectmult.push_back(0.78f);
	tmpAbilStruct.protectmult.push_back(0.72f);
	protection_list_abils.push_back(tmpAbilStruct);

	// magina
	tmpAbilStruct.abil_typeid = 'A0KY';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.74f);
	tmpAbilStruct.protectmult.push_back(0.56f);
	tmpAbilStruct.protectmult.push_back(0.40f);
	tmpAbilStruct.protectmult.push_back(0.24f);
	protection_list_abils.push_back(tmpAbilStruct);

	// viper
	tmpAbilStruct.abil_typeid = 'A0MM';
	tmpAbilStruct.protect_type = 1;
	tmpAbilStruct.type = 0;
	tmpAbilStruct.unit_typeid = 0;
	tmpAbilStruct.protectmult.clear();
	tmpAbilStruct.protectmult.push_back(0.90f);
	tmpAbilStruct.protectmult.push_back(0.85f);
	tmpAbilStruct.protectmult.push_back(0.80f);
	tmpAbilStruct.protectmult.push_back(0.75f);
	protection_list_abils.push_back(tmpAbilStruct);

	BombConfigStruct tmpBomb = BombConfigStruct();

	tmpBomb.result_dmg1 = tmpBomb.result_dmg2 = 0.0f;

	tmpBomb.lvl_multiplier = 0.0f;
	tmpBomb.lvl_skillid = 0;

	tmpBomb.unit_typeid = 'n00O';
	tmpBomb.dmg = 300.0f;
	tmpBomb.dmg2 = 300.0f;
	tmpBomb.range1 = 100.0f;
	tmpBomb.range2 = 100.0f;
	tmpBomb.remote = false;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'n00P';
	tmpBomb.dmg = 375.0f;
	tmpBomb.dmg2 = 375.0f;
	tmpBomb.range1 = 100.0f;
	tmpBomb.range2 = 100.0f;
	tmpBomb.remote = false;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'n00Q';
	tmpBomb.dmg = 450.0f;
	tmpBomb.dmg2 = 450.0f;
	tmpBomb.range1 = 100.0f;
	tmpBomb.range2 = 100.0f;
	tmpBomb.remote = false;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'n00N';
	tmpBomb.dmg = 525.0f;
	tmpBomb.dmg2 = 525.0f;
	tmpBomb.range1 = 100.0f;
	tmpBomb.range2 = 100.0f;
	tmpBomb.remote = false;
	techies_bombs.push_back(tmpBomb);


	tmpBomb.unit_typeid = 'o018';
	tmpBomb.dmg = 300.0f;
	tmpBomb.dmg2 = 300.0f;
	tmpBomb.range1 = 400.0f;
	tmpBomb.range2 = 400.0f;
	tmpBomb.remote = true;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'o002';
	tmpBomb.dmg = 450.0f;
	tmpBomb.dmg2 = 450.0f;
	tmpBomb.range1 = 410.0f;
	tmpBomb.range2 = 410.0f;
	tmpBomb.remote = true;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'o00B';
	tmpBomb.dmg = 600.0f;
	tmpBomb.dmg2 = 600.0f;
	tmpBomb.range1 = 425.0f;
	tmpBomb.range2 = 425.0f;
	tmpBomb.remote = true;
	techies_bombs.push_back(tmpBomb);

	tmpBomb.unit_typeid = 'o01B';
	tmpBomb.dmg = 700.0f;
	tmpBomb.dmg2 = 700.0f;
	tmpBomb.range1 = 425.0f;
	tmpBomb.range2 = 425.0f;
	tmpBomb.remote = true;
	techies_bombs.push_back(tmpBomb);

	// rubik 3 skill
	tmpBomb.unit_typeid = 'h0EG';
	tmpBomb.dmg = 100.0f;
	tmpBomb.dmg2 = 100.0f;
	tmpBomb.range1 = 375.0f;
	tmpBomb.range2 = 375.0f;
	tmpBomb.remote = false;
	tmpBomb.lvl_multiplier = 50.0f;
	tmpBomb.lvl_skillid = 'A2LL';
	techies_bombs.push_back(tmpBomb);

	// Storm 1 skill 
	tmpBomb.unit_typeid = 'h07F';
	tmpBomb.dmg = 100.0f;
	tmpBomb.dmg2 = 100.0f;
	tmpBomb.range1 = 235.0f;
	tmpBomb.range2 = 235.0f;
	tmpBomb.remote = false;
	tmpBomb.lvl_multiplier = 40.0f;
	tmpBomb.lvl_skillid = 'A14P';
	techies_bombs.push_back(tmpBomb);

	//  1 skill 
	/*tmpBomb.unit_typeid = 'h07U';
	tmpBomb.dmg = 30.0f;
	tmpBomb.dmg2 = 30.0f;
	tmpBomb.range1 = 200.0f;
	tmpBomb.range2 = 200.0f;
	tmpBomb.remote = false;
	tmpBomb.lvl_multiplier = 30.0f;
	tmpBomb.lvl_skillid = 'A21J';
	techies_bombs.push_back(tmpBomb);*/
}

int GetTypeIdFromString(std::string type_id)
{
	int outid = 0;
	if (type_id.length() == 4)
	{
		char* outidchar = (char*)&outid;
		outidchar[0] = type_id[3];
		outidchar[1] = type_id[2];
		outidchar[2] = type_id[1];
		outidchar[3] = type_id[0];
	}
	return outid;
}

void SaveMapConfiguration()
{
	mapcfg_write->WriteString("GENERAL", "DAGGER_ITEM_ID", GetStringFromTypeId(DaggerItemId).c_str());
	mapcfg_write->WriteString("GENERAL", "DAGGER_ABILITY_ID", GetStringFromTypeId(DaggerAbilId).c_str());
	mapcfg_write->WriteFloat("GENERAL", "DAGGER_DISTANCE", DaggerDistance);

	mapcfg_write->WriteString("GENERAL", "FORCESTAFF_ITEM_ID", GetStringFromTypeId(ForceStaffItemId).c_str());
	mapcfg_write->WriteString("GENERAL", "FORCESTAFF_ABILITY_ID", GetStringFromTypeId(ForceStaffAbilId).c_str());
	mapcfg_write->WriteFloat("GENERAL", "FORCESTAFF_DISTANCE", ForceStaffDistance);


	mapcfg_write->WriteString("GENERAL", "SUICIDE_ABILITY_ID", GetStringFromTypeId(SuicideAbilId).c_str());

	mapcfg_write->WriteInt("GENERAL", "SUICIDE_COMMAND", SuicideCommand);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_LVL1", SuicideDmg[0]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_LVL2", SuicideDmg[1]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_LVL3", SuicideDmg[2]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_LVL4", SuicideDmg[3]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_PART_LVL1", SuicidePartDmg[0]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_PART_LVL2", SuicidePartDmg[1]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_PART_LVL3", SuicidePartDmg[2]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DMG_PART_LVL4", SuicidePartDmg[3]);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DISTANCE_FULL", SuicideDistanceFull);
	mapcfg_write->WriteFloat("GENERAL", "SUICIDE_DISTANCE_PART", SuicideDistancePart);

	mapcfg_write->WriteInt("GENERAL", "SUPPORTED_HERO_NUM", techies_ids.size());

	for (unsigned int i = 0; i < techies_ids.size(); i++)
	{
		std::string herostr = "HERO";
		herostr += std::to_string(i + 1);

		mapcfg_write->WriteString(herostr.c_str(), "HERO_UNIT_TYPE_ID", GetStringFromTypeId(techies_ids[i].unitid).c_str());
		mapcfg_write->WriteString(herostr.c_str(), "HERO_UNIT_ABIL_ID", GetStringFromTypeId(techies_ids[i].abilid).c_str());
	}

	mapcfg_write->WriteInt("GENERAL", "SUPPORTED_BOMB_COUNT", techies_bombs.size());
	for (unsigned int i = 0; i < techies_bombs.size(); i++)
	{
		std::string bombstr = "BOMB";
		bombstr += std::to_string(i + 1);
		mapcfg_write->WriteString(bombstr.c_str(), "BOMB_TYPE_ID", GetStringFromTypeId(techies_bombs[i].unit_typeid).c_str());
		mapcfg_write->WriteBool(bombstr.c_str(), "BOMB_IS_REMOTE", techies_bombs[i].remote);
		mapcfg_write->WriteFloat(bombstr.c_str(), "BOMB_FULL_DMG", techies_bombs[i].dmg);
		mapcfg_write->WriteFloat(bombstr.c_str(), "BOMB_PART_DMG", techies_bombs[i].dmg2);
		mapcfg_write->WriteFloat(bombstr.c_str(), "BOMB_RANGE_FULL", techies_bombs[i].range1);
		mapcfg_write->WriteFloat(bombstr.c_str(), "BOMB_RANGE_PART", techies_bombs[i].range2);
		mapcfg_write->WriteFloat(bombstr.c_str(), "BOMB_LVL_MULTIPLIER", techies_bombs[i].lvl_multiplier);
		mapcfg_write->WriteString(bombstr.c_str(), "BOMB_LVL_BASESKILLID", GetStringFromTypeId(techies_bombs[i].lvl_skillid).c_str());
	}

	mapcfg_write->WriteInt("GENERAL", "BOMB_DETONATE_COMMAND", DetonateCommand);
	mapcfg_write->WriteString("GENERAL", "STASIS_UNIT_ID", GetStringFromTypeId(StasisUnitId).c_str());

	mapcfg_write->WriteInt("GENERAL", "PROTECT_DMG_ABIL_COUNT", protection_list_abils.size());
	for (unsigned int i = 0; i < protection_list_abils.size(); i++)
	{
		std::string abilstr = "ABIL";
		abilstr += std::to_string(i + 1);

		mapcfg_write->WriteString(abilstr.c_str(), "ABILITY_TYPE_ID", GetStringFromTypeId(protection_list_abils[i].abil_typeid).c_str());
		mapcfg_write->WriteInt(abilstr.c_str(), "ABILITY_TYPE", protection_list_abils[i].type);
		mapcfg_write->WriteString(abilstr.c_str(), "ABILITY_UNIT_CODE", GetStringFromTypeId(protection_list_abils[i].unit_typeid).c_str());
		mapcfg_write->WriteInt(abilstr.c_str(), "DMG_PROTECT_TYPE", protection_list_abils[i].protect_type);
		mapcfg_write->WriteInt(abilstr.c_str(), "DMG_LEVELS", protection_list_abils[i].protectmult.size());
		for (unsigned int n = 0; n < protection_list_abils[i].protectmult.size(); n++)
		{
			std::string dmgstr = "DMG_PROTECT_MULTIPLIER";
			dmgstr += std::to_string(n + 1);
			mapcfg_write->WriteFloat(abilstr.c_str(), dmgstr.c_str(), protection_list_abils[i].protectmult[n]);
		}
	}

	mapcfg_write->WriteInt("GENERAL", "PROTECT_DMG_UNIT_COUNT", protection_list_units.size());
	for (unsigned int i = 0; i < protection_list_units.size(); i++)
	{
		std::string unitstr = "UNIT";
		unitstr += std::to_string(i + 1);

		mapcfg_write->WriteString(unitstr.c_str(), "UNIT_TYPE_ID", GetStringFromTypeId(protection_list_units[i].unit_typeid).c_str());
		mapcfg_write->WriteInt(unitstr.c_str(), "UNIT_PROTECT_TYPE", protection_list_units[i].protect_type);
		mapcfg_write->WriteFloat(unitstr.c_str(), "DMG_PROTECT_MULTIPLIER", protection_list_units[i].protectmult);
	}


	mapcfg_write->WriteInt("GENERAL", "PROTECT_DMG_ITEM_COUNT", protection_list_items.size());
	for (unsigned int i = 0; i < protection_list_items.size(); i++)
	{
		std::string itemstr = "ITEM";
		itemstr += std::to_string(i + 1);

		mapcfg_write->WriteString(itemstr.c_str(), "ITEM_TYPE_ID", GetStringFromTypeId(protection_list_items[i].item_typeid).c_str());
		mapcfg_write->WriteInt(itemstr.c_str(), "ITEM_PROTECT_TYPE", protection_list_items[i].protect_type);
		mapcfg_write->WriteFloat(itemstr.c_str(), "ITEM_PROTECT_MULTIPLIER", protection_list_items[i].protectmult);
	}
}

void ParseMapConfiguration()
{
	std::string path = techiesBotFileName + "\\";
	path = path + MapFileName;
	path.pop_back(); path.pop_back(); path.pop_back(); path.pop_back();
	path += ".ini";

	if (mapcfg_read)
		delete mapcfg_read;

	mapcfg_read = new CIniReader(path.c_str());

	if (mapcfg_write)
		delete mapcfg_write;

	mapcfg_write = new CIniWriter(path.c_str());

	techies_ids.clear();
	techies_bombs.clear();
	protection_list_abils.clear();
	protection_list_items.clear();
	protection_list_units.clear();

	if (!std::filesystem::exists(path))
	{
		LoadDefaultDotaConfiguration();
		SaveMapConfiguration();
		return;
	}


	DaggerItemId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "DAGGER_ITEM_ID", ""));
	DaggerAbilId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "DAGGER_ABILITY_ID", ""));
	DaggerDistance = mapcfg_read->ReadFloat("GENERAL", "DAGGER_DISTANCE", 0.0f);

	ForceStaffItemId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "FORCESTAFF_ITEM_ID", ""));
	ForceStaffAbilId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "FORCESTAFF_ABILITY_ID", ""));
	ForceStaffDistance = mapcfg_read->ReadFloat("GENERAL", "FORCESTAFF_DISTANCE", 0.0f);

	SuicideAbilId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "SUICIDE_ABILITY_ID", "A06B"));
	SuicideCommand = mapcfg_read->ReadInt("GENERAL", "SUICIDE_COMMAND", 852040);
	SuicideDmg[0] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_LVL1", 500.0f);
	SuicideDmg[1] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_LVL2", 650.0f);
	SuicideDmg[2] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_LVL3", 850.0f);
	SuicideDmg[3] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_LVL4", 1150.0f);
	SuicidePartDmg[0] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_PART_LVL1", 260.0f);
	SuicidePartDmg[1] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_PART_LVL2", 300.0f);
	SuicidePartDmg[2] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_PART_LVL3", 340.0f);
	SuicidePartDmg[3] = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DMG_PART_LVL4", 380.0f);
	SuicideDistanceFull = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DISTANCE_FULL", 200.0f);
	SuicideDistancePart = mapcfg_read->ReadFloat("GENERAL", "SUICIDE_DISTANCE_PART", 500.0f);

	int heronum = mapcfg_read->ReadInt("GENERAL", "SUPPORTED_HERO_NUM", 0);
	for (int i = 0; i < heronum; i++)
	{
		std::string herostr = "HERO";
		herostr += std::to_string(i + 1);

		int unitid = GetTypeIdFromString(mapcfg_read->ReadString(herostr.c_str(), "HERO_UNIT_TYPE_ID", ""));
		int abilid = GetTypeIdFromString(mapcfg_read->ReadString(herostr.c_str(), "HERO_UNIT_ABIL_ID", ""));
		if (unitid != 0 || abilid != 0)
		{
			techies_ids.push_back({ unitid,abilid });
		}
	}

	int bombcount = mapcfg_read->ReadInt("GENERAL", "SUPPORTED_BOMB_COUNT", 0);
	for (int i = 0; i < bombcount; i++)
	{
		std::string bombstr = "BOMB";
		bombstr += std::to_string(i + 1);
		int unitid = GetTypeIdFromString(mapcfg_read->ReadString(bombstr.c_str(), "BOMB_TYPE_ID", ""));
		bool isremote = mapcfg_read->ReadBool(bombstr.c_str(), "BOMB_IS_REMOTE", false);
		float dmg1 = mapcfg_read->ReadFloat(bombstr.c_str(), "BOMB_FULL_DMG", 0.0f);
		float dmg2 = mapcfg_read->ReadFloat(bombstr.c_str(), "BOMB_PART_DMG", 0.0f);
		float range1 = mapcfg_read->ReadFloat(bombstr.c_str(), "BOMB_RANGE_FULL", 0.0f);
		float range2 = mapcfg_read->ReadFloat(bombstr.c_str(), "BOMB_RANGE_PART", 0.0f);
		float lvl_multi = mapcfg_read->ReadFloat(bombstr.c_str(), "BOMB_LVL_MULTIPLIER", 0.0f);
		int lvl_abil_id = GetTypeIdFromString(mapcfg_read->ReadString(bombstr.c_str(), "BOMB_LVL_BASESKILLID", ""));

		BombConfigStruct tmpBomb = BombConfigStruct();

		tmpBomb.result_dmg1 = tmpBomb.result_dmg2 = 0.0f;

		tmpBomb.unit_typeid = unitid;
		tmpBomb.dmg = dmg1;
		tmpBomb.dmg2 = dmg2;
		tmpBomb.range1 = range1;
		tmpBomb.range2 = range2;
		tmpBomb.remote = isremote;

		tmpBomb.lvl_skillid = lvl_abil_id;
		tmpBomb.lvl_multiplier = lvl_multi;

		techies_bombs.push_back(tmpBomb);
	}

	DetonateCommand = mapcfg_read->ReadInt("GENERAL", "BOMB_DETONATE_COMMAND", 0);

	StasisUnitId = GetTypeIdFromString(mapcfg_read->ReadString("GENERAL", "STASIS_UNIT_ID", ""));

	int protect_abilcount = mapcfg_read->ReadInt("GENERAL", "PROTECT_DMG_ABIL_COUNT", 0);
	for (int i = 0; i < protect_abilcount; i++)
	{
		std::string abilstr = "ABIL";
		abilstr += std::to_string(i + 1);

		int abilid = GetTypeIdFromString(mapcfg_read->ReadString(abilstr.c_str(), "ABILITY_TYPE_ID", ""));
		int abiltype = mapcfg_read->ReadInt(abilstr.c_str(), "ABILITY_TYPE", 0);
		int unitid = GetTypeIdFromString(mapcfg_read->ReadString(abilstr.c_str(), "ABILITY_UNIT_CODE", ""));
		int dmgtype = mapcfg_read->ReadInt(abilstr.c_str(), "DMG_PROTECT_TYPE", 0);
		int dmglevels = mapcfg_read->ReadInt(abilstr.c_str(), "DMG_LEVELS", 0);

		AbilConfigStruct tmpAbilStruct = AbilConfigStruct();
		tmpAbilStruct.abil_typeid = abilid;
		tmpAbilStruct.type = abiltype;
		tmpAbilStruct.unit_typeid = unitid;
		tmpAbilStruct.protect_type = dmgtype;

		for (int n = 0; n < dmglevels; n++)
		{
			std::string dmgstr = "DMG_PROTECT_MULTIPLIER";
			dmgstr += std::to_string(n + 1);
			tmpAbilStruct.protectmult.push_back(mapcfg_read->ReadFloat(abilstr.c_str(), dmgstr.c_str(), 0.0f));
		}

		protection_list_abils.push_back(tmpAbilStruct);
	}

	int protect_unitcount = mapcfg_read->ReadInt("GENERAL", "PROTECT_DMG_UNIT_COUNT", 0);
	for (int i = 0; i < protect_unitcount; i++)
	{
		std::string unitstr = "UNIT";
		unitstr += std::to_string(i + 1);

		int unitid = GetTypeIdFromString(mapcfg_read->ReadString(unitstr.c_str(), "UNIT_TYPE_ID", ""));
		int dmgtype = mapcfg_read->ReadInt(unitstr.c_str(), "UNIT_PROTECT_TYPE", 0);
		float dmg = mapcfg_read->ReadFloat(unitstr.c_str(), "DMG_PROTECT_MULTIPLIER", 0.0f);

		UnitConfigStruct tmpUnitStruct = UnitConfigStruct();
		tmpUnitStruct.unit_typeid = unitid;
		tmpUnitStruct.protect_type = dmgtype;
		tmpUnitStruct.protectmult = dmg;

		protection_list_units.push_back(tmpUnitStruct);
	}


	int protect_itemcount = mapcfg_read->ReadInt("GENERAL", "PROTECT_DMG_ITEM_COUNT", 0);
	for (int i = 0; i < protect_itemcount; i++)
	{
		std::string itemstr = "ITEM";
		itemstr += std::to_string(i + 1);

		int itemid = GetTypeIdFromString(mapcfg_read->ReadString(itemstr.c_str(), "ITEM_TYPE_ID", ""));
		int dmgtype = mapcfg_read->ReadInt(itemstr.c_str(), "ITEM_PROTECT_TYPE", 0);
		float dmg = mapcfg_read->ReadFloat(itemstr.c_str(), "ITEM_PROTECT_MULTIPLIER", 0.0f);

		ItemConfigStruct tmpItemStruct = ItemConfigStruct();
		tmpItemStruct.item_typeid = itemid;
		tmpItemStruct.protect_type = dmgtype;
		tmpItemStruct.protectmult = dmg;

		protection_list_items.push_back(tmpItemStruct);
	}
}

/*

[GENERAL]
; Disabled = 0
; AutoKill = 1
; AutoExplode = 2
EXPLODE_TYPE = 1
; Use dagger to move techies closest to target
; for using forcestaff
; Disable = 0
; Enabled = 1
DAGGER_ENABLED = 1
; Disable = 0
; Enabled = 1
FORCESTAFF_ENABLED = 1
; NEXT EXPERT SETTINGS
USE_CUSTOM_GAMEDLL = 0
CUSTOM_GAMEDLL = Game.dll
USE_CUSTOM_THREAD_ID = 0
CUSTOM_THREAD_ID = 0
*/

void SaveMainConfiguration()
{
	maincfg_write->WriteInt("GENERAL", "EXPLODE_TYPE", EnableAutoExplode);
	maincfg_write->WriteBool("GENERAL", "DAGGER_ENABLED", EnableDagger);
	maincfg_write->WriteBool("GENERAL", "FORCESTAFF_ENABLED", EnableForceStaff);
	maincfg_write->WriteInt("GENERAL", "BASE_DAMAGE_REDUCE", BaseDmgReducing);
}

void ParseMainConfiguration()
{
	std::string path = techiesBotFileName + "\\main.ini";
	if (!maincfg_read)
		maincfg_read = new CIniReader(path.c_str());
	if (!maincfg_write)
		maincfg_write = new CIniWriter(path.c_str());

	EnableAutoExplode = maincfg_read->ReadInt("GENERAL", "EXPLODE_TYPE", EnableAutoExplode);
	EnableDagger = maincfg_read->ReadBool("GENERAL", "DAGGER_ENABLED", EnableDagger);
	EnableForceStaff = maincfg_read->ReadBool("GENERAL", "FORCESTAFF_ENABLED", EnableForceStaff);

	if (maincfg_read->ReadBool("GENERAL", "USE_CUSTOM_GAMEDLL", false))
	{
		GameDllName = maincfg_read->ReadString("GENERAL", "CUSTOM_GAMEDLL", "Game.dll");
	}

	customGameThread = maincfg_read->ReadBool("GENERAL", "USE_CUSTOM_THREAD_ID", true);

	BaseDmgReducing = maincfg_read->ReadInt("GENERAL", "BASE_DAMAGE_REDUCE", BaseDmgReducing);
	BaseDmgReducingMagic = maincfg_read->ReadFloat("GENERAL", "BASE_REDUCE_MAGIC_DMG", BaseDmgReducingMagic);
	BaseDmgReducingPhys = maincfg_read->ReadFloat("GENERAL", "BASE_REDUCE_PHYS_DMG", BaseDmgReducingPhys);
}


int IsOkayPtr(unsigned char* ptr, unsigned int size = 4)
{
	if (!GameDll)
		return 0;
	/*
	if (UseWarnIsBadReadPtr == 1)
	{
		int returnvalue = IsBadReadPtr(ptr, size) == 0;
		if (!returnvalue)
		{
		}
		return returnvalue;
	}
	else if (UseWarnIsBadReadPtr == 2)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(ptr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
		{
			return 0;
		}

		if ((int)ptr + size > (int)mbi.BaseAddress + mbi.RegionSize)
		{
			return 0;
		}


		if ((int)ptr < (int)mbi.BaseAddress)
		{
			return 0;
		}


		if (mbi.State != MEM_COMMIT)
		{
			return 0;
		}


		if (mbi.Protect != PAGE_READWRITE && mbi.Protect != PAGE_WRITECOPY && mbi.Protect != PAGE_READONLY)
		{
			return 0;
		}

		return 1;
	}
	else*/
	return 1;
}

union DWFP
{
	DWORD dw;
	float fl;
};


struct DebugStruct
{
	int addr;
	int procid;
};


void PrintDebugInfo(const char* debuginfo)
{
	sprintf_s(TechiesCrash, sizeof(TechiesCrash), "%s", debuginfo);
}

unsigned char* _GameUI = 0;
unsigned char* InGame = 0;
int IsGame()
{
	if (!GameDll)
		return 0;

	if (!InGame)
		return 0;

	return *(unsigned char**)InGame && **(unsigned char***)InGame == _GameUI;
}

unsigned char* GAME_PrintToScreen = 0;
void TextPrint(const char* szText, float fDuration)
{
	if (StealthMode)
		return;
	unsigned int dwDuration = *((unsigned int*)&fDuration);
	if (!GameDll || !*(unsigned char**)_W3XGlobalClass)
		return;
	typedef void(__fastcall* GAME_PrintToScreen_t)(int ecx, uint32_t edx, uint32_t arg1, uint32_t arg2, const char* outLinePointer, uint32_t dwDuration, uint32_t arg5);
	int ecx = *(int*)_W3XGlobalClass;
	GAME_PrintToScreen_t _GAME_PrintToScreen = (GAME_PrintToScreen_t)GAME_PrintToScreen;
	_GAME_PrintToScreen(ecx, 0x0, 0x0, 0x0, szText, dwDuration, 0xFFFFFFFF);
}

std::string LastString = "";

void TextPrintUnspammed(const char* szText)
{
	if (!szText || szText[0] == '\0')
		return;
	if (StealthMode)
		return;

	if (llabs(CurTickCount - LastChatAccess) > 1000 || (ExpertModeEnabled && LastString != szText))
	{
		LastChatAccess = CurTickCount;
		LastString = szText;
		float fDuration = 1.3f;
		if (!ExpertModeEnabled)
			fDuration += 0.7f;
		TextPrint(szText, fDuration);
	}
}

int GetUnitCount()
{
	int GlobalClassOffset = *(int*)(_W3XGlobalClass);
	if (GlobalClassOffset)
	{
		int UnitsOffset1 = *(int*)(GlobalClassOffset + 0x3BC);
		if (UnitsOffset1 > 0)
		{
			int* UnitsCount = (int*)(UnitsOffset1 + 0x604);
			if (UnitsCount)
			{
				return *UnitsCount;
			}
		}
	}
	return 0;
}

std::vector<unsigned char*> GetUnitsArray()
{
	std::vector<unsigned char*> return_value = std::vector<unsigned char*>();


	int GlobalClassOffset = *(int*)(_W3XGlobalClass);
	if (GlobalClassOffset)
	{
		int UnitsOffset1 = *(int*)(GlobalClassOffset + 0x3BC);
		if (UnitsOffset1 > 0)
		{
			int* UnitsCountAddr = (int*)(UnitsOffset1 + 0x604);
			if (*(int*)UnitsCountAddr > 0)
			{
				int UnitsCount = *(int*)UnitsCountAddr;
				if (IsOkayPtr((unsigned char*)(UnitsOffset1 + 0x608)) && *(int*)(UnitsOffset1 + 0x608) > 0)
				{
					unsigned char** unitarray = (unsigned char**)*(int*)(UnitsOffset1 + 0x608);

					if (UnitsCount > 0 && unitarray)
					{
						std::set<unsigned char*> addrs;
						for (int i = 0; i < UnitsCount; i++)
						{
							if (addrs.count(unitarray[i]))
								continue;
							addrs.insert(unitarray[i]);
							return_value.push_back(unitarray[i]);
						}
					}

					return return_value;
				}
				else
				{
					return return_value;
				}
			}
		}
	}
	return return_value;
}

std::vector<unsigned char*> GetOwnerHeroesArray()
{
	std::vector<unsigned char*> return_value = std::vector<unsigned char*>();


	int GlobalClassOffset = *(int*)(_W3XGlobalClass);
	if (GlobalClassOffset)
	{
		int UnitsOffset1 = *(int*)(GlobalClassOffset + 0x3BC);
		if (UnitsOffset1 > 0)
		{
			int* UnitsCountAddr = (int*)(UnitsOffset1 + 0x604);
			if (*(int*)UnitsCountAddr > 0)
			{
				int UnitsCount = *(int*)UnitsCountAddr;
				if (IsOkayPtr((unsigned char*)(UnitsOffset1 + 0x608)) && *(int*)(UnitsOffset1 + 0x608) > 0)
				{
					unsigned char** unitarray = (unsigned char**)*(int*)(UnitsOffset1 + 0x608);

					if (UnitsCount > 0 && unitarray)
					{
						std::set<unsigned char*> addrs;
						for (int i = 0; i < UnitsCount; i++)
						{
							if (addrs.count(unitarray[i]))
								continue;

							if (IsNotBadUnit(unitarray[i]) && GetUnitOwnerSlot(unitarray[i]) == GetLocalPlayerNumber() && IsHero(unitarray[i]))
							{
								addrs.insert(unitarray[i]);
								return_value.push_back(unitarray[i]);
							}
						}
					}
					return return_value;
				}
				else
				{
					return return_value;
				}
			}
		}
	}
	return return_value;
}

void(__thiscall* UpdatePlayerSelection)(unsigned char* pPlayerSelectData, int unk) = NULL;
void(__thiscall* SelectUnitReal)(unsigned char* pPlayerSelectData, unsigned char* pUnit, int id, int unk1, int unk2, int unk3) = NULL;

//)(GameDll + 0x424CE0))((int)pSelection, UnitAddr, v4, 1, 1);
//signed int(__thiscall* RemoveUnitFromSelection)(int, int, unsigned int, int, int);
int(__thiscall* UpdateGameSelectionUI)(unsigned char* a1) = NULL;

typedef unsigned char* (__fastcall* sub_6F26EC20)(unsigned char* unitaddr, int unused, unsigned int SLOTID);
sub_6F26EC20 _GetItemInSlot;

unsigned char* GetItemInSlot(unsigned char* unitaddr, unsigned int slotnum)
{
	return _GetItemInSlot(unitaddr, 0, slotnum);
}

//
typedef unsigned char* (__fastcall* sub_6F0787D0)(unsigned char* unitaddr, int unused, int classid, int a3, int a4, int a5, int a6);
sub_6F0787D0 GetAbility;

/*

sub_6F339DD0my: (a1):d0028 (a2):c0114e4 (a3):-411.262 (a4):-347.085 (a5):bfdc104 (a6):4 (a7):4
//ETH - I0LT

sub_6F339DD0my: (a1):d0029 (a2):c010a3c (a3):-443.463 (a4):-383.475 (a5):bfdc104 (a6):100004 (a7):4
//ORCH - I012

sub_6F339CC0my: (a1):d002a (a2):c010f14 (a3):-499.668 (a4):-494.432 (a5):1100002 (a6):4
//HUETA - I0O3
*/


bool IsAbilityCooldown(unsigned char* unitaddr, int id)
{
	unsigned int cooldownflag = 0x200;
	unsigned char* abiladdr = GetAbility(unitaddr, 0, id, 0, 1, 1, 1);
	if (!abiladdr || !IsOkayPtr((unsigned char*)abiladdr))
		return true;

	int avilityflag = (int)abiladdr + 32;
	if (IsOkayPtr((unsigned char*)avilityflag))
	{
		unsigned int state = *(unsigned int*)(avilityflag);
		return (state & cooldownflag) > 0;
	}
	else
		return true;
}

bool IsAbilityHidden(unsigned char* unitaddr, int id)
{
	unsigned char* abiladdr = GetAbility(unitaddr, 0, id, 0, 1, 1, 1);
	if (!abiladdr || !IsOkayPtr((unsigned char*)abiladdr))
		return true;

	unsigned int xavaiableflag = 0x8000;

	int avilityflag = (int)abiladdr + 32;
	if (IsOkayPtr((unsigned char*)avilityflag))
	{
		unsigned int  state = *(unsigned int*)(avilityflag);
		return (state & xavaiableflag) > 0;
	}
	else
		return true;
}

void PrintCooldownFlag(unsigned char* unitaddr, int id)
{
	unsigned char* abiladdr = GetAbility(unitaddr, 0, id, 0, 1, 1, 1);
	if (!abiladdr || !IsOkayPtr((unsigned char*)abiladdr))
		return;

	int abilleveladdr = (int)abiladdr + 4;
	if (IsOkayPtr((unsigned char*)abilleveladdr))
	{
		int xid = id;
		char cc1 = *(char*)((int)(&xid));
		char cc2 = *(char*)((int)(&xid) + 1);
		char cc3 = *(char*)((int)(&xid) + 2);
		char cc4 = *(char*)((int)(&xid) + 3);
		char cc5[] = { cc4, cc3, cc2, cc1, '\0' };
		unsigned int  state = *(unsigned int*)(abilleveladdr);
		char* printdate = new char[100];
		memset(printdate, 0, 100);
		sprintf_s(printdate, 100, "%s->%X->%X", cc5, state, (unsigned int)abiladdr);
		TextPrint(printdate, 2.0f);
		delete[]printdate;
	}
}


unsigned char* GetGlobalPlayerData()
{
	if (GameDll && *(int*)(0xAB65F4 + GameDll) > 0)
	{
		if (IsOkayPtr((unsigned char*)(0xAB65F4 + GameDll)))
			return (unsigned char*)*(int*)(0xAB65F4 + GameDll);
		else
			return 0;
	}
	else
		return 0;
}

unsigned char* GetPlayerByNumber(int number)
{
	unsigned char* arg1 = GetGlobalPlayerData();
	unsigned char* result = 0;
	if (number >= 0 && number <= 12 && arg1)
	{
		result = arg1 + (number * 4) + 0x58;

		if (IsOkayPtr((unsigned char*)result))
		{
			result = *(unsigned char**)result;
		}
	}
	return result;
}

int GetLocalPlayerNumber()
{
	unsigned char* gldata = GetGlobalPlayerData();
	if (gldata)
	{
		int playerslotaddr = (int)gldata + 0x28;
		if (IsOkayPtr((unsigned char*)playerslotaddr))
			return (int)*(short*)(playerslotaddr);
		else
			return 0;
	}
	else
		return 0;
}


unsigned char* GetLocalPlayer()
{
	return GetPlayerByNumber(GetLocalPlayerNumber());
}

int GetUnitOwnerSlot(unsigned char* unitaddr)
{
	if (IsOkayPtr((unsigned char*)(unitaddr + 88)))
		return *(int*)(unitaddr + 88);
	return -1;
}

int GetPlayerTeam(unsigned char* playeraddr)
{
	if (!playeraddr)
		return 0;
	return *(int*)(playeraddr + 0x278);
}

int IsPlayerEnemy(unsigned char* unitaddr)
{
	int teamplayer1 = GetPlayerTeam(GetLocalPlayer());
	int teamplayer2 = GetPlayerTeam(GetPlayerByNumber(GetUnitOwnerSlot(unitaddr)));

	return teamplayer1 != teamplayer2;
}

int IsUnitDead(unsigned char* unitaddr)
{
	unsigned int isdolbany = *(unsigned int*)(unitaddr + 0x5C);
	int UnitNotDead = ((isdolbany & 0x100u) == 0);
	return UnitNotDead == 0;
}

void GetMousePosition(float* x, float* y, float* z)
{
	int globalclass = *(int*)_W3XGlobalClass;

	int offset1 = globalclass + 0x3BC;

	if (globalclass > 0)
	{
		if (IsOkayPtr((unsigned char*)offset1))
		{
			offset1 = *(int*)offset1;
			if (IsOkayPtr((unsigned char*)offset1))
			{
				*x = *(float*)(offset1 + 0x310);
				*y = *(float*)(offset1 + 0x310 + 4);
				*z = *(float*)(offset1 + 0x310 + 4 + 4);
			}
			else
			{
				*x = 0.0f;
				*y = 0.0f;
				*z = 0.0f;
			}
		}
		else
		{
			*x = 0.0f;
			*y = 0.0f;
			*z = 0.0f;
		}

	}
}

unsigned char* GetSelectedOwnedUnit()
{
	unsigned char* plr = GetLocalPlayer();
	if (plr)
	{
		int PlayerData1 = *(int*)(plr + 0x34);
		if (PlayerData1 > 0)
		{
			unsigned char* unitaddr = *(unsigned char**)(PlayerData1 + 0x1E0);
			if (unitaddr)
			{
				if (GetUnitOwnerSlot(unitaddr) == GetLocalPlayerNumber())
				{
					return unitaddr;
				}
			}
		}
	}
	return 0;
}

int GetObjectTypeId(unsigned char* unit_or_item_addr)
{
	return *(int*)(unit_or_item_addr + 0x30);
}

typedef int(__fastcall* sub_6F32C880)(int unit_item_code, int unused);
sub_6F32C880 GetTypeInfo = 0;


const char* GetUnitName(unsigned char* unitaddr)
{
	int unitcode = GetObjectTypeId(unitaddr);
	if (unitcode != 0)
	{
		int v3 = GetTypeInfo(unitcode, 0);
		int v4, v5;
		if (v3 && (v4 = *(int*)(v3 + 40)) != 0)
		{
			v5 = v4 - 1;
			if (v5 >= (unsigned int)0)
				v5 = 0;
			return (const char*)*(int*)(*(int*)(v3 + 44) + 4 * v5);
		}
		else
		{
			return "Default string";
		}
	}
	return "Default String";
}

void PrintClassAddress(unsigned char* unitaddr)
{
	int unitcode = GetObjectTypeId(unitaddr);
	if (unitcode != 0)
	{
		int v3 = GetTypeInfo(unitcode, 0);
		char* printdada = new char[200];
		sprintf_s(printdada, 200, "ClassAddr:%X", v3);
		TextPrint(printdada, 15.0f);
		delete[]printdada;
	}
}/*

void SetUnitColor(unsigned char* unitaddr, int color)
{
	int unitdata = *(int*)(unitaddr + 0x28);
	if (unitdata > 0)
	{
		*(int*)(unitdata + 0x328) = color;
		*(int*)(unitdata + 0x320) = 0;
		*(int*)(unitdata + 0x316) = 0;
		if (!(*(int*)(unitdata + 0x312) & 0x800))
		{
			*(int*)(unitdata + 0x312) = *(int*)(unitdata + 0x316) | 0x800;
		}
	}
}*/

int IsTypeIdEqual(unsigned char* unit_or_item_addr, int classid)
{
	if (unit_or_item_addr && IsOkayPtr((unsigned char*)(unit_or_item_addr), 0x34))
	{
		int unitclassid = *(int*)(unit_or_item_addr + 0x30);
		if (unitclassid == classid)
		{
			return 1;
		}
	}
	return 0;
}

//int IsClassEqual(unsigned char* unit_or_item_addr, const char* classid)
//{
//	if (unit_or_item_addr && IsOkayPtr((unsigned char*)(unit_or_item_addr), 0x34))
//	{
//		int unitclassid = *(int*)(unit_or_item_addr + 0x30);
//		int checkunitclassid = 0;
//		((unsigned char*)&checkunitclassid)[3] = *(unsigned char*)(&classid[0]);
//		((unsigned char*)&checkunitclassid)[2] = *(unsigned char*)(&classid[1]);
//		((unsigned char*)&checkunitclassid)[1] = *(unsigned char*)(&classid[2]);
//		((unsigned char*)&checkunitclassid)[0] = *(unsigned char*)(&classid[3]);
//		if (checkunitclassid == unitclassid)
//		{
//			return 1;
//		}
//	}
//	return 0;
//}

struct UnitLocation
{
	float X;
	float Y;
	float Z;
};

struct Location
{
	float X;
	float Y;
};


int GetCMDbyItemSlot(int slot) // ot 1 do 6
{
	return (0xd0028 + slot);
}

int IsItemCooldown(int itemaddr)
{
	if (itemaddr <= 0)
		return 1;

	return !((*(int*)(itemaddr + 32)) & 0x400u);
}

typedef int(__stdcall* IssueTargetOrPointOrder)(int a1, unsigned char* a2, float a3, float a4, unsigned char* a5, int a6, int a7);
IssueTargetOrPointOrder IssueTargetOrPointOrderorg = NULL;

void  CommandOrItemTarget(int cmd, unsigned char* itemaddr, float targetx, float targety, unsigned char* targetunitaddr, int flags = 4, bool queue = false)
{
	IssueTargetOrPointOrderorg(cmd, itemaddr, targetx, targety, targetunitaddr, flags, queue ? 5 : 4);
}
typedef int(__stdcall* IssueWithoutTargetOrder)(int a1, int a2, unsigned int a3, unsigned int a4);
IssueWithoutTargetOrder IssueWithoutTargetOrderorg = NULL;

typedef int(__stdcall* IssueTargetOrPointOrder2)(int a1, unsigned char* a2, float a3, float a4, int a5, int a6);
IssueTargetOrPointOrder2 IssueTargetOrPointOrder2org = NULL;

void  ItemOrSkillPoint(int cmd, unsigned char* itemorskilladdr, float x, float y, int a5, bool addque = false)
{
	IssueTargetOrPointOrder2org(cmd, itemorskilladdr, x, y, a5, addque ? 5 : 4);
}

void UseDetonator()
{
	IssueWithoutTargetOrderorg(DetonateCommand, 0, 1, 4);
}


Location GetNextPoint(float x, float y, float distance, float angle)
{
	Location returnlocation = Location();
	returnlocation.X = x + distance * cos(angle);
	returnlocation.Y = y + distance * sin(angle);
	return returnlocation;
}

void GetUnitLocation2D(unsigned char* unitaddr, float& x, float& y)
{
	x = *(float*)(unitaddr + 0x284);
	y = *(float*)(unitaddr + 0x288);
}

void GetUnitLocation3D(unsigned char* unitaddr, float& x, float& y, float& z)
{
	x = *(float*)(unitaddr + 0x284);
	y = *(float*)(unitaddr + 0x288);
	z = *(float*)(unitaddr + 0x28C);
}


typedef float* (__thiscall* GetUnitFloatStat)(unsigned char* unitaddr, float* a2, int a3);
GetUnitFloatStat _GetUnitFloatState;

float GetUnitFloatState(unsigned char* unitaddr, DWORD statNum)
{
	float result = 0;
	_GetUnitFloatState(unitaddr, &result, statNum);
	return result;
}


float GetUnitHP(unsigned char* unitaddr)
{
	return GetUnitFloatState(unitaddr, 0);
}


float GetUnitMaxHP(unsigned char* unitaddr)
{
	return GetUnitFloatState(unitaddr, 1);
}

int GetUnitHPPercent(unsigned char* unitaddr)
{
	return (int)((GetUnitHP(unitaddr) / GetUnitMaxHP(unitaddr)) * 100.f);
}


bool IsHero(unsigned char* unitaddr)
{
	if (unitaddr && IsOkayPtr((unsigned char*)(unitaddr + 48)))
	{
		unsigned int ishero = *(unsigned int*)(unitaddr + 48);
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return false;
}

float GetUnitTimer(unsigned char* unitaddr)
{
	int unitdataddr = *(int*)(unitaddr + 0x28);
	if (unitdataddr <= 0)
		return 0.0f;
	if (IsOkayPtr((unsigned char*)(unitdataddr + 0xA0), 4))
		return *(float*)(unitdataddr + 0xA0);
	return 0.0f;
}

unsigned char* UnitVtable = 0;
// Проверяет юнит или не юнит
int __stdcall IsNotBadUnit(unsigned char* unitaddr, int onlymem)
{
	if (unitaddr && IsOkayPtr((unsigned char*)unitaddr, 0x100))
	{
		unsigned char* realvtbladdr = (unsigned char*)&UnitVtable;

		if (realvtbladdr[0] != unitaddr[0])
			return 0;
		else if (realvtbladdr[1] != unitaddr[1])
			return 0;
		else if (realvtbladdr[2] != unitaddr[2])
			return 0;
		else if (realvtbladdr[3] != unitaddr[3])
			return 0;

		unsigned int x1 = *(unsigned int*)(unitaddr + 0xC);
		unsigned int y1 = *(unsigned int*)(unitaddr + 0x10);

		int udata = *(int*)(unitaddr + 0x28);

		if (x1 == 0xFFFFFFFF || y1 == 0xFFFFFFFF || udata == 0)
		{
			return 0;
		}

		if (onlymem)
			return 1;

		unsigned int unitflag = *(unsigned int*)(unitaddr + 0x20);
		unsigned int unitflag2 = *(unsigned int*)(unitaddr + 0x5C);

		if (unitflag & 1u)
		{
			return 0;
		}

		if (!(unitflag & 2u))
		{
			return 0;
		}

		if (unitflag & 8)
		{
			return 0;
		}

		if (unitflag2 & 0x100u)
		{
			return 0;
		}

		if (unitflag2 & 0x40000000u)
		{
			return 0;
		}

		return 1;
	}

	return 0;
}

float Distance2(float dX0, float dY0, float dX1, float dY1)
{
	return sqrt((dX1 - dX0) * (dX1 - dX0) + (dY1 - dY0) * (dY1 - dY0));
}

float Distance(float dX0, float dY0, float dX1, float dY1)
{
	return sqrt(pow(dX0 - dX1, 2.f) + pow(dY0 - dY1, 2.f));
}

float Distance3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	/*if (Enable3DPoint)
	{
		double d[] = { abs((double)x1 - (double)x2), abs((double)y1 - (double)y2), abs((double)z1 - (double)z2) };
		if (d[0] < d[1]) std::swap(d[0], d[1]);
		if (d[0] < d[2]) std::swap(d[0], d[2]);
		return (float)(d[0] * sqrt(1.0 + d[1] / d[0] + d[2] / d[0]));
	}
	else
	{*/
	return Distance(x1, y1, x2, y2);
	//}
}

float DistanceBetweenLocs(Location loc1, Location loc2)
{
	return Distance(loc1.X, loc1.Y, loc2.X, loc2.Y);
}

Location GiveNextLocationFromLocAndAngle(Location startloc, float distance, float angle)
{
	return GetNextPoint(startloc.X, startloc.Y, distance, angle);
}


bool SelectUnit(unsigned char* xunitaddr)
{
	if (!IsNotBadUnit(xunitaddr))
	{
		return false;
	}

	if (*(unsigned char*)(xunitaddr + 32) & 2)
	{
		unsigned char* playerseldata = *(unsigned char**)(GetLocalPlayer() + 0x34);
		int playerslot = GetLocalPlayerNumber();
		SelectUnitReal(playerseldata, xunitaddr, playerslot, 0, 1, 1);
		UpdatePlayerSelection(playerseldata, 0);
		return true;
	}

	return false;
}

int(__cdecl* ClearSelection)(void) = NULL;

int __cdecl SelectAllUnits(int max_per_tick = 12)
{
	int myselectedunits = 0;

	ClearSelection();

	auto unittoselectlocal = unitstoselect;

	for (unsigned int i = 0; i < unittoselectlocal.size(); i++)
	{
		if (myselectedunits < max_per_tick)
		{
			if (SelectUnit(unittoselectlocal[i]))
				myselectedunits++;
		}
	}

	UpdateGameSelectionUI(0);

	return myselectedunits > 0;
}

void SelectAllUnitsAndDetonate()
{
	if (SelectAllUnits()) // fix to while if need
	{
		UseDetonator();
	}
}

float GetProtectForUnit(unsigned char* unitaddr)
{
	float Armor = 0.0f;

	if (IsOkayPtr((unsigned char*)(unitaddr + 0xE0)))
	{
		Armor = *(float*)(unitaddr + 0xE0);
	}

	if (Armor < 0.0f)
		Armor = (float)(1.0 - pow(0.94, -Armor));
	else
		Armor = (float)((0.06 * Armor) / (1 + 0.06 * Armor));
	return Armor;
}

float GetProtectForProtect(float Armor)
{
	if (Armor < 0.0f)
		Armor = (float)(1.0 - pow(0.94, -Armor));
	else
		Armor = (float)((0.06 * Armor) / (1 + 0.06 * Armor));
	return Armor;
}

int GetUnitAbilityLevel(unsigned char* unitaddr, int id, int checkavaiable)
{
	unsigned char* abiladdr = (unsigned char*)GetAbility(unitaddr, 0, id, 0, 1, 1, 1);
	if (!abiladdr || !IsOkayPtr((unsigned char*)abiladdr))
		return 0;

	unsigned int xavaiableflag = 0x8000;

	int abilleveladdr = (int)abiladdr + 80;
	int abilavaiable = (int)abiladdr + 32;
	if (checkavaiable)
	{
		if (IsOkayPtr((unsigned char*)abilavaiable))
		{
			unsigned int avaiableflag = *(unsigned int*)(abilavaiable);
			if (avaiableflag & xavaiableflag)
				return 0;
			if (IsOkayPtr((unsigned char*)abilleveladdr))
				return *(int*)(abilleveladdr)+1;
			else
				return 0;
		}
		else
			return 0;
	}
	else
	{
		if (IsOkayPtr((unsigned char*)abilleveladdr))
			return *(int*)(abilleveladdr)+1;
		else
			return 0;
	}
}

typedef int(__fastcall* pGetSomeAddr)(unsigned int a1, unsigned int a2);
pGetSomeAddr GetSomeAddr;

float GetUnitFacingMega(unsigned char* unitaddr)
{
	if (unitaddr)
	{
		int dataoffset = (int)GetSomeAddr(*(unsigned int*)(unitaddr + 0x16C), *(unsigned int*)(unitaddr + 0x170));
		if (dataoffset)
		{
			float retangle = *(float*)(dataoffset + 0x8C);
			/*	if (retangle < 0)
				{
					return 360.0f + retangle;
				}*/
			return retangle;
		}
	}
	return 0.0f;
}

float GetUnitFacing(unsigned char* unitaddr)
{
	//return GetUnitFacingMega(unitaddr);
	int unitdataoffset = *(int*)(unitaddr + 0x28);
	if (unitaddr && unitdataoffset > 0)
	{
		double atan2value = atan2((double)*(float*)(unitdataoffset + 0x10c), (double)*(float*)(unitdataoffset + 0x108));
		return (float)atan2value;
	}
	return 1.0f;
}


bool __cdecl IsUnitVisibleToPlayer(unsigned char* unitaddr, unsigned char* player)
{
	int retval = 0;
	if (unitaddr && player && *(int*)unitaddr)
	{
		typedef int(__fastcall* FunctionType)(void*, int, int, int, int);
		FunctionType function = (FunctionType)(*(int*)(*(int*)unitaddr + 0x000000FC));
		retval = function(unitaddr, 0, player[0x30], 0x00, 0x04);
	}
	return retval > 0;
}

bool SelectTechies()
{
	auto units = GetOwnerHeroesArray();

	if (std::find(units.begin(), units.end(), GetSelectedOwnedUnit()) != units.end())
		return true;

	for (auto unit : units)
	{
		if (IsNotBadUnit(unit) && IsHero(unit))
		{
			unitstoselect.clear();
			unitstoselect.push_back(unit);
			int selected = SelectAllUnits();
			unitstoselect.clear();
			if (selected > 0)
			{
				return true;
			}
		}
	}
	return false;
}

void DetonateIfNeed()
{
	if (EnableAutoExplode == 2 || llabs(CurTickCount - ForceDetonateTime) < 500)
	{
		auto unit_list = GetUnitsArray();
		for (auto unit : unit_list)
		{
			if (!unit)
				continue;

			if (IsNotBadUnit(unit) && IsHero(unit))
			{
				if (GetLocalPlayerNumber() != GetUnitOwnerSlot(unit))
				{
					if (IsPlayerEnemy(unit) && IsUnitVisibleToPlayer(unit, GetLocalPlayer()) && GetUnitDamageWithProtection(unit, DMG_TYPE_MAGIC, 600.0f) > 20.0f)
					{
						float unitx = 0.0f, unity = 0.0f, unitz = 0.0f;
						GetUnitLocation3D(unit, unitx, unity, unitz);
						unitstoselect.clear();
						for (unsigned int n = 0; n < BombList.size(); n++)
						{
							if (!BombList[n].remote)
								continue;

							if (Distance3D(unitx, unity, unitz, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range1)
							{
								unitstoselect.push_back(BombList[n].unitaddr);
							}
							else if (Distance3D(unitx, unity, unitz, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range2)
							{
								unitstoselect.push_back(BombList[n].unitaddr);
							}
						}

						if (unitstoselect.size() > 0)
						{
							TextPrintUnspammed("[~~~~AutoExplode~~~~]");
							SelectAllUnitsAndDetonate();
							unitstoselect.clear();
							SelectTechies();
						}
					}
				}
			}
		}
	}

	if (EnableAutoExplode == 1 && CurTickCount - LastDmgTime > 100)
	{
		auto unit_list = GetUnitsArray();
		for (auto unit : unit_list)
		{
			if (!unit)
				continue;

			if (IsNotBadUnit(unit) && IsHero(unit))
			{
				if (GetLocalPlayerNumber() != GetUnitOwnerSlot(unit))
				{
					if (IsPlayerEnemy(unit) && IsUnitVisibleToPlayer(unit, GetLocalPlayer()))
					{
						float okaydmg = 0.0f;
						float unitx = 0.0f, unity = 0.0f, unitz = 0.0f;
						GetUnitLocation3D(unit, unitx, unity, unitz);
						unitstoselect.clear();

						int BombsFound = 0;

						bool xxokaydmg = false;

						for (unsigned int n = 0; n < BombList.size(); n++)
						{
							auto tmpBombStruct = BombList[n];

							/*	if (IsKeyPressed('X'))
								{
									char dbgt[256];
									sprintf_s(dbgt, "%X, %f, %f, %f, %f, %s, %f, %f, %f", (int)unit, tmpBombStruct.dmg, tmpBombStruct.dmg2,
										tmpBombStruct.range1, tmpBombStruct.range2, tmpBombStruct.remote ? "true" : "false", tmpBombStruct.x, tmpBombStruct.y, tmpBombStruct.z);
									TextPrint(dbgt, 2.0f);
									sprintf_s(dbgt, "%f, %f, %f = %f", unitx, unity, unitz, Distance3D(unitx, unity, unitz, BombList[n].x, BombList[n].y, BombList[n].z));
									TextPrint(dbgt, 2.0f);
								}*/

							if (!BombList[n].remote)
								continue;

							if (Distance3D(unitx, unity, unitz, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range1)
							{
								BombsFound++;
								unitstoselect.push_back(BombList[n].unitaddr);



								okaydmg += GetUnitDamageWithProtection(unit, DMG_TYPE_MAGIC, BombList[n].dmg);
								if (okaydmg > GetUnitHP(unit))
								{
									xxokaydmg = true;
									break;
								}
							}
							else if (Distance3D(unitx, unity, unitz, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range2)
							{
								BombsFound++;
								unitstoselect.push_back(BombList[n].unitaddr);
								okaydmg += GetUnitDamageWithProtection(unit, DMG_TYPE_MAGIC, BombList[n].dmg2);
								if (okaydmg > GetUnitHP(unit))
								{
									xxokaydmg = true;
									break;
								}
							}
						}

						if (unitstoselect.size() > 0)
						{
							if (xxokaydmg && (int)okaydmg > (int)GetUnitHP(unit) + BaseDmgReducing)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[AutoKill->OK]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%i|r. Count: %i\n%s", GetUnitName(unit), (int)GetUnitHP(unit), (int)okaydmg, BombsFound, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
								SelectAllUnitsAndDetonate();
								unitstoselect.clear();
								SelectTechies();
								LastDmgTime = CurTickCount;
							}
							else
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~AutoKill~~~]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r. Count: %i\n%s", GetUnitName(unit), (int)GetUnitHP(unit), okaydmg, BombsFound, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
							}
						}
					}
				}
			}
		}
	}
}

float old_unit_angle = 0.0f;

void ProcessForceStaffAndDagger()
{
	if (!EnableForceStaff || EnableAutoExplode == 0 || CurTickCount - LastDmgTime < 100)
		return;

	unsigned char* force_unit = 0;

	auto units = GetOwnerHeroesArray();

	for (auto unit : units)
	{
		for (int i = 0; i < 6; i++)
		{
			unsigned char* itemaddr = GetItemInSlot(unit, i);
			if (itemaddr && IsTypeIdEqual(itemaddr, ForceStaffItemId))
			{
				force_unit = unit;
				break;
			}
		}
	}

	if (!force_unit)
		return;

	int ForceStaffFound = false;
	int forcestaffitemid = 0;
	unsigned char* forceitemaddr = 0;
	for (int i = 0; i < 6; i++)
	{
		forceitemaddr = GetItemInSlot(force_unit, i);
		if (forceitemaddr && IsTypeIdEqual(forceitemaddr, ForceStaffItemId))
		{
			if (IsAbilityCooldown(force_unit, ForceStaffAbilId))
			{
				if (!_Forcecooldown)
				{
					TextPrint("|cFFFF6700F|r|cFFFE6401o|r|cFFFD6102r|r|cFFFC5E03c|r|cFFFB5A04e|r|cFFFA5705s|r|cFFF95406t|r|cFFF95107a|r|cFFF84E08f|r|cFFF74B09f|r|cFFF6470A |r|cFFF5440Bc|r|cFFF4410Co|r|cFFF33E0Co|r|cFFF23B0Dl|r|cFFF1380Ed|r|cFFF0340Fo|r|cFFEF3110w|r|cFFEE2E11n|r|cFFEE2B12 |r|cFFED2813s|r|cFFEC2514t|r|cFFEB2115a|r|cFFEA1E16r|r|cFFE91B17t|r|cFFE81818.|r", 3.0f);
					_Forcecooldown = 1;
				}
			}
			else
			{
				if (_Forcecooldown)
				{
					_Forcecooldown = 0;
					TextPrint("|cFF21FF00F|r|cFF31FF00o|r|cFF42FF00r|r|cFF52FF00c|r|cFF63FF00e|r|cFF73FF00s|r|cFF84FF00t|r|cFF94FF00a|r|cFFA5FF00f|r|cFFB5FF00f|r|cFFC6FF00 |r|cFFD6FF00co|r|cFFC6FF00o|r|cFFB5FF00l|r|cFFA5FF00d|r|cFF94FF00o|r|cFF84FF00w|r|cFF73FF00n|r|cFF63FF00 |r|cFF52FF00e|r|cFF42FF00n|r|cFF31FF00d|r|cFF21FF00.|r", 3.0f);
				}
				ForceStaffFound = true;
				forcestaffitemid = i;
			}

			break;
		}
	}

	bool DaggerFound = false;
	int daggeritemid = 0;
	unsigned char* daggeritemaddr = 0;

	if (!EnableDagger)
	{

	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			daggeritemaddr = GetItemInSlot(force_unit, i);
			if (daggeritemaddr && IsTypeIdEqual(daggeritemaddr, DaggerItemId))
			{
				if (IsAbilityCooldown(force_unit, DaggerAbilId))
				{
					if (!_Daggercooldown)
					{
						TextPrint("|cFFFF6700D|r|cFFFE6301a|r|cFFFD5F02g|r|cFFFC5C03g|r|cFFFB5805e|r|cFFFA5406r|r|cFFF85007 |r|cFFF74D08c|r|cFFF64909o|r|cFFF5450Ao|r|cFFF4410Bl|r|cFFF33E0Dd|r|cFFF23A0Eo|r|cFFF1360Fw|r|cFFF03210n|r|cFFEF2F11 |r|cFFED2B12s|r|cFFEC2713t|r|cFFEB2315a|r|cFFEA2016r|r|cFFE91C17t|r|cFFE81818.|r", 3.0f);
						_Daggercooldown = 1;
					}
				}
				else
				{
					if (_Daggercooldown)
					{
						_Daggercooldown = 0;
						TextPrint("|cFF21FF00D|r|cFF35FF00a|r|cFF49FF00g|r|cFF5DFF00g|r|cFF71FF00e|r|cFF86FF00r|r|cFF9AFF00 |r|cFFAEFF00c|r|cFFC2FF00o|r|cFFD6FF00ol|r|cFFC2FF00d|r|cFFAEFF00o|r|cFF9AFF00w|r|cFF86FF00n|r|cFF71FF00 |r|cFF5DFF00e|r|cFF49FF00n|r|cFF35FF00d|r|cFF21FF00.|r", 3.0f);
					}

					DaggerFound = true;
					daggeritemid = i;
				}
				break;
			}
		}
	}

	if (ForceStaffFound && !_Forcecooldown)
	{

		//float techface = GetUnitFacing(GetSelectedOwnedUnit());
		float techiesunitx = 0.0f, techiesunity = 0.0f, techiesunitz = 0.0f;
		GetUnitLocation3D(force_unit, techiesunitx, techiesunity, techiesunitz);

		auto unit_list = GetUnitsArray();

		for (auto unit : unit_list)
		{
			if (!unit)
				continue;
			if (IsNotBadUnit(unit) && IsHero(unit))
			{
				if (GetLocalPlayerNumber() != GetUnitOwnerSlot(unit))
				{
					if (IsPlayerEnemy(unit) && IsUnitVisibleToPlayer(unit, GetLocalPlayer()))
					{
						float unitface = GetUnitFacing(unit);

						float targetunitx = 0.0f, targetunity = 0.0f, targetunitz = 0.0f;
						GetUnitLocation3D(unit, targetunitx, targetunity, targetunitz);

						if (Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < ForceStaffDistance || (DaggerFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < (ForceStaffDistance + DaggerDistance)))
						{
							float DaggerDist = 0.0f;
							if (DaggerFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < ForceStaffDistance)
							{
								DaggerFound = false;
							}
							else
							{
								DaggerDist = Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) - ForceStaffDistance + 42.0f;
							}

							Location startenemyloc = Location();
							startenemyloc.X = targetunitx;
							startenemyloc.Y = targetunity;

							std::set<int> bombs_in_used;
							float outdmg = 0.0f;
							float enemyhp = GetUnitHP(unit);
							float endenemyloc_dist = 0.0f;
							while (endenemyloc_dist <= ForceStaffDistance)
							{
								Location endenemyloc = GiveNextLocationFromLocAndAngle(startenemyloc, endenemyloc_dist, unitface);
								endenemyloc_dist += 25.0f;

								for (unsigned int n = 0; n < BombList.size(); n++)
								{
									if (bombs_in_used.count(n))
										continue;
									/*if ( IsIgnoreUnit( BombList[ n ].unitaddr ) )
									continue;*/
									if (!BombList[n].remote)
									{
										if (endenemyloc_dist > BombList[n].range1)
										{
											if (BombList[n].is_stasis && StatisForceStaff && Distance3D(endenemyloc.X, endenemyloc.Y, BombList[n].z, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range1)
											{
												outdmg += GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, 99999.f);
												bombs_in_used.insert(n);
											}
											else
											{
												if (Distance3D(endenemyloc.X, endenemyloc.Y, BombList[n].z, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range1)
												{
													outdmg += GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, BombList[n].dmg);
													bombs_in_used.insert(n);
												}
												else if (Distance3D(endenemyloc.X, endenemyloc.Y, BombList[n].z, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range2)
												{
													outdmg += GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, BombList[n].dmg2);
													bombs_in_used.insert(n);
												}
											}
										}
									}
									else if (GetLocalPlayerNumber() == GetUnitOwnerSlot(BombList[n].unitaddr))
									{
										if (Distance3D(endenemyloc.X, endenemyloc.Y, BombList[n].z, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range1)
										{
											outdmg += GetUnitDamageWithProtection(unit, DMG_TYPE_MAGIC, BombList[n].dmg);
											bombs_in_used.insert(n);
										}
										else if (Distance3D(endenemyloc.X, endenemyloc.Y, BombList[n].z, BombList[n].x, BombList[n].y, BombList[n].z) < BombList[n].range2)
										{
											outdmg += GetUnitDamageWithProtection(unit, DMG_TYPE_MAGIC, BombList[n].dmg2);
											bombs_in_used.insert(n);
										}
									}
								}
							}

							if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg || (EnableAutoExplode == 2 && outdmg > 100.0f))
							{
								if (SelectTechies())
								{
									int scmd = GetCMDbyItemSlot(forcestaffitemid);

									if (IsNotBadUnit(unit))
									{
										if (fabs(old_unit_angle - unitface) * (180.0f / 3.141592653f) < 1.0f)
										{
											char* printdata = new char[1024];
											sprintf_s(printdata, 1024, "[~~~FORCESTAFF~~~]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r. Count: %i\n%s", GetUnitName(unit), (int)enemyhp, outdmg, bombs_in_used.size(), PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
											TextPrintUnspammed(printdata);
											delete[]printdata;

											if (DaggerFound)
											{
												int dagcmd = GetCMDbyItemSlot(daggeritemid);

												Location starttechiesloc = Location();
												starttechiesloc.X = techiesunitx;
												starttechiesloc.Y = techiesunity;

												if (DaggerDist < 0.0f)
													DaggerDist = 42.0f;

												float facebetween = atan2((targetunity - techiesunity), (targetunitx - techiesunitx));

												Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, DaggerDist, facebetween);

												ItemOrSkillPoint(dagcmd, daggeritemaddr, endtechiesloc.X, endtechiesloc.Y, 0x100002);
												CommandOrItemTarget(scmd, forceitemaddr, targetunitx, targetunity, unit, 4, true);

												ForceDetonateTime = CurTickCount;
												forceunitaddr = unit;
											}
											else
											{
												ForceDetonateTime = CurTickCount;
												forceunitaddr = unit;

												CommandOrItemTarget(scmd, forceitemaddr, targetunitx, targetunity, unit, 4);
											}
										}
										old_unit_angle = unitface;
										break;
									}
								}
							}
							else if (bombs_in_used.size() > 0)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~FORCESTAFF~~~]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r. Count: %i\n%s", GetUnitName(unit), (int)enemyhp, outdmg, bombs_in_used.size(), PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
							}
						}
					}
				}
			}
		}
	}
}

void SuicideIfNeed()
{
	if (!AutoSuicide || CurTickCount - LastDmgTime < 100)
		return;

	unsigned char* suicide_unit = 0;

	auto units = GetOwnerHeroesArray();

	int suicide_lvl = 0;

	for (auto unit : units)
	{
		for (int i = 0; i < 6; i++)
		{
			suicide_lvl = GetUnitAbilityLevel(unit, SuicideAbilId);
			if (suicide_lvl > 0 && !IsAbilityCooldown(unit, SuicideAbilId))
			{
				suicide_unit = unit;
				break;
			}
		}
	}

	if (!suicide_unit)
		return;

	bool DaggerFound = false;
	int daggeritemid = 0;
	unsigned char* daggeritemaddr = 0;

	for (int i = 0; i < 6; i++)
	{
		daggeritemaddr = GetItemInSlot(suicide_unit, i);
		if (daggeritemaddr && IsTypeIdEqual(daggeritemaddr, DaggerItemId))
		{
			if (IsAbilityCooldown(suicide_unit, DaggerAbilId))
			{
				if (!_Daggercooldown)
				{
					TextPrint("|cFFFF6700D|r|cFFFE6301a|r|cFFFD5F02g|r|cFFFC5C03g|r|cFFFB5805e|r|cFFFA5406r|r|cFFF85007 |r|cFFF74D08c|r|cFFF64909o|r|cFFF5450Ao|r|cFFF4410Bl|r|cFFF33E0Dd|r|cFFF23A0Eo|r|cFFF1360Fw|r|cFFF03210n|r|cFFEF2F11 |r|cFFED2B12s|r|cFFEC2713t|r|cFFEB2315a|r|cFFEA2016r|r|cFFE91C17t|r|cFFE81818.|r", 3.0f);
					_Daggercooldown = 1;
				}
			}
			else
			{
				if (_Daggercooldown)
				{
					TextPrint("|cFF21FF00D|r|cFF35FF00a|r|cFF49FF00g|r|cFF5DFF00g|r|cFF71FF00e|r|cFF86FF00r|r|cFF9AFF00 |r|cFFAEFF00c|r|cFFC2FF00o|r|cFFD6FF00ol|r|cFFC2FF00d|r|cFFAEFF00o|r|cFF9AFF00w|r|cFF86FF00n|r|cFF71FF00 |r|cFF5DFF00e|r|cFF49FF00n|r|cFF35FF00d|r|cFF21FF00.|r", 3.0f);
					_Daggercooldown = 0;
				}

				DaggerFound = true;
				daggeritemid = i;
			}
			break;
		}
	}

	bool ForceStaffFound = false;
	int forcestaffitemid = 0;
	unsigned char* forceitemaddr = 0;
	for (int i = 0; i < 6; i++)
	{
		forceitemaddr = GetItemInSlot(suicide_unit, i);
		if (forceitemaddr && IsTypeIdEqual(forceitemaddr, ForceStaffItemId))
		{
			if (IsAbilityCooldown(suicide_unit, ForceStaffAbilId))
			{
				if (!_Forcecooldown)
				{
					TextPrint("|cFFFF6700F|r|cFFFE6401o|r|cFFFD6102r|r|cFFFC5E03c|r|cFFFB5A04e|r|cFFFA5705s|r|cFFF95406t|r|cFFF95107a|r|cFFF84E08f|r|cFFF74B09f|r|cFFF6470A |r|cFFF5440Bc|r|cFFF4410Co|r|cFFF33E0Co|r|cFFF23B0Dl|r|cFFF1380Ed|r|cFFF0340Fo|r|cFFEF3110w|r|cFFEE2E11n|r|cFFEE2B12 |r|cFFED2813s|r|cFFEC2514t|r|cFFEB2115a|r|cFFEA1E16r|r|cFFE91B17t|r|cFFE81818.|r", 3.0f);
					_Forcecooldown = 1;
				}
			}
			else
			{
				if (_Forcecooldown)
				{
					_Forcecooldown = 0;
					TextPrint("|cFF21FF00F|r|cFF31FF00o|r|cFF42FF00r|r|cFF52FF00c|r|cFF63FF00e|r|cFF73FF00s|r|cFF84FF00t|r|cFF94FF00a|r|cFFA5FF00f|r|cFFB5FF00f|r|cFFC6FF00 |r|cFFD6FF00co|r|cFFC6FF00o|r|cFFB5FF00l|r|cFFA5FF00d|r|cFF94FF00o|r|cFF84FF00w|r|cFF73FF00n|r|cFF63FF00 |r|cFF52FF00e|r|cFF42FF00n|r|cFF31FF00d|r|cFF21FF00.|r", 3.0f);
				}
				ForceStaffFound = true;
				forcestaffitemid = i;
			}

			break;
		}
	}

	// Если есть даггер поиск юнитов в радиусе 1200 ...
	// Если есть forcestaff и на пути есть юнит ...
	// Если в радиусе FULL/PART есть юнит...

	float techiesunitx = 0.0f, techiesunity = 0.0f, techiesunitz = 0.0f;
	GetUnitLocation3D(suicide_unit, techiesunitx, techiesunity, techiesunitz);

	auto unit_list = GetUnitsArray();

	suicide_lvl -= 1;
	if (suicide_lvl < 0)
		suicide_lvl = 0;
	if (suicide_lvl > 3)
		suicide_lvl = 3;

	float DMG_FULL = SuicideDmg[suicide_lvl];
	float DMG_PART = SuicidePartDmg[suicide_lvl];

	for (auto unit : unit_list)
	{
		if (!unit)
			continue;
		if (IsNotBadUnit(unit) && IsHero(unit))
		{
			if (GetLocalPlayerNumber() != GetUnitOwnerSlot(unit))
			{
				if (IsPlayerEnemy(unit) && IsUnitVisibleToPlayer(unit, GetLocalPlayer()))
				{
					float techface = GetUnitFacing(GetSelectedOwnedUnit());
					float enemyhp = GetUnitHP(unit);

					float targetunitx = 0.0f, targetunity = 0.0f, targetunitz = 0.0f;
					GetUnitLocation3D(unit, targetunitx, targetunity, targetunitz);

					if (Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_FULL);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							char* printdata = new char[1024];
							sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
							TextPrintUnspammed(printdata);
							delete[]printdata;
							CommandOrItemTarget(SuicideCommand, 0, techiesunitx, techiesunity, 0, 4);

							LastDmgTime = CurTickCount;
						}
					}
					else if (Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_PART);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							char* printdata = new char[1024];
							sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][2]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
							TextPrintUnspammed(printdata);
							delete[]printdata;
							CommandOrItemTarget(SuicideCommand, 0, techiesunitx, techiesunity, 0, 4);

							LastDmgTime = CurTickCount;
						}
					}
					else if (DaggerFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull + DaggerDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_FULL);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							float DaggerDist = Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz);

							if (DaggerDist < 0.0f)
								DaggerDist = 42.0f;

							float facebetween = atan2((targetunity - techiesunity), (targetunitx - techiesunitx));

							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, DaggerDist, facebetween);
							char* printdata = new char[1024];
							sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][DAGGER]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
							TextPrintUnspammed(printdata);
							delete[]printdata;
							int dagcmd = GetCMDbyItemSlot(daggeritemid);
							ItemOrSkillPoint(dagcmd, daggeritemaddr, endtechiesloc.X, endtechiesloc.Y, 0x100002);
						}
					}
					else if (DaggerFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart + DaggerDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_PART);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							float DaggerDist = Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz);

							if (DaggerDist < 0.0f)
								DaggerDist = 42.0f;

							float facebetween = atan2((targetunity - techiesunity), (targetunitx - techiesunitx));

							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, DaggerDist, facebetween);
							char* printdata = new char[1024];
							sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][DAGGER2]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
							TextPrintUnspammed(printdata);
							delete[]printdata;
							int dagcmd = GetCMDbyItemSlot(daggeritemid);
							ItemOrSkillPoint(dagcmd, daggeritemaddr, endtechiesloc.X, endtechiesloc.Y, 0x100002);
						}
					}
					else if (ForceStaffFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull + ForceStaffDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_FULL);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, ForceStaffDistance, techface);
							int scmd = GetCMDbyItemSlot(forcestaffitemid);

							if (Distance3D(endtechiesloc.X, endtechiesloc.Y, targetunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][FORCE]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
								CommandOrItemTarget(scmd, forceitemaddr, techiesunitx, techiesunity, suicide_unit, 4);
							}
						}
					}
					else if (ForceStaffFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart + ForceStaffDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_PART);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, ForceStaffDistance, techface);
							int scmd = GetCMDbyItemSlot(forcestaffitemid);

							if (Distance3D(endtechiesloc.X, endtechiesloc.Y, targetunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][FORCE2]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
								CommandOrItemTarget(scmd, forceitemaddr, techiesunitx, techiesunity, suicide_unit, 4);
							}
						}
					}
					else if (DaggerFound && ForceStaffFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull + DaggerDistance + ForceStaffDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_FULL);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, ForceStaffDistance, techface);
							int scmd = GetCMDbyItemSlot(forcestaffitemid);

							if (Distance3D(endtechiesloc.X, endtechiesloc.Y, targetunitz, targetunitx, targetunity, targetunitz) < SuicideDistanceFull + DaggerDistance)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][DAG+FORCE]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
								CommandOrItemTarget(scmd, forceitemaddr, techiesunitx, techiesunity, suicide_unit, 4);
							}
						}
					}
					else if (DaggerFound && ForceStaffFound && Distance3D(techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart + DaggerDistance + ForceStaffDistance)
					{
						float outdmg = GetUnitDamageWithProtection(unit, DMG_TYPE_PHYS, DMG_PART);
						if ((int)(enemyhp + BaseDmgReducing) < (int)outdmg)
						{
							Location starttechiesloc = Location();
							starttechiesloc.X = techiesunitx;
							starttechiesloc.Y = techiesunity;

							Location endtechiesloc = GiveNextLocationFromLocAndAngle(starttechiesloc, ForceStaffDistance, techface);
							int scmd = GetCMDbyItemSlot(forcestaffitemid);

							if (Distance3D(endtechiesloc.X, endtechiesloc.Y, targetunitz, targetunitx, targetunity, targetunitz) < SuicideDistancePart + DaggerDistance)
							{
								char* printdata = new char[1024];
								sprintf_s(printdata, 1024, "[~~~SUICIDE ATTACK~~~][DAG+FORCE2]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%.3f|r.\n%s", GetUnitName(unit), (int)enemyhp, outdmg, PrintBuffListStr.size() > 0 ? PrintBuffListStr.c_str() : "No buff found");
								TextPrintUnspammed(printdata);
								delete[]printdata;
								CommandOrItemTarget(scmd, forceitemaddr, techiesunitx, techiesunity, suicide_unit, 4);
							}
						}
					}
				}
			}
		}
	}
}

void ProcessHotkeys()
{
	//if (RemoteTechiesFound)
	{
		// X + 1
		if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x31))
		{
			IsHotkeyPress = true;
			EnableAutoExplode++;

			if (EnableAutoExplode > 2)
				EnableAutoExplode = 0;

			if (EnableAutoExplode == 2)
			{
				TextPrint("AutoExplode: |cFFEF2020AutoExplode|r", 3.0f);
			}
			else if (EnableAutoExplode == 1)
			{
				TextPrint("AutoExplode: |cFF00FF00AutoKill|r ", 3.0f);
			}
			else if (EnableAutoExplode == 0)
			{
				TextPrint("AutoExplode: |cFFFF0000Disabled|r ", 3.0f);
			}
		}
	}
	/*else
	{
		if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x31))
		{
			IsHotkeyPress = true;
			TextPrint("AutoExplode: |cFFEF2020No hero with access|r", 3.0f);
		}*
	}*/
	// X + 2
	if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x32))
	{
		IsHotkeyPress = true;
		EnableForceStaff = !EnableForceStaff;

		if (EnableForceStaff)
		{
			TextPrint("ForceStaff: |cFF00FF00ENABLED|r", 3.0f);
		}
		else if (!EnableForceStaff)
		{
			TextPrint("ForceStaff: |cFFFF0000DISABLED|r", 3.0f);
		}
	}
	// X + 3
	if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x33))
	{
		IsHotkeyPress = true;
		EnableDagger = !EnableDagger;

		if (EnableDagger)
		{
			TextPrint("Dagger: |cFF00FF00ENABLED|r", 3.0f);
		}
		else if (!EnableDagger)
		{
			TextPrint("Dagger: |cFFFF0000DISABLED|r", 3.0f);
		}
	}

	// X + 4
	if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x34))
	{
		IsHotkeyPress = true;
		StatisForceStaff = !StatisForceStaff;

		if (StatisForceStaff)
		{
			TextPrint("Force to Stasis Trap: |cFF00FF00ENABLED|r", 3.0f);
		}
		else if (!StatisForceStaff)
		{
			TextPrint("Force to Stasis Trap: |cFFFF0000DISABLED|r", 3.0f);
		}
	}

	// X + 5
	if (!IsHotkeyPress && IsKeyPressed('X') && IsKeyPressed(0x35))
	{
		IsHotkeyPress = true;
		AutoSuicide = !AutoSuicide;

		if (AutoSuicide)
		{
			TextPrint("Auto Suicide: |cFF00FF00ENABLED|r", 3.0f);
		}
		else if (!AutoSuicide)
		{
			TextPrint("Auto Suicide: |cFFFF0000DISABLED|r", 3.0f);
		}
	}


	// X + 6
	if (IsKeyPressed('X') && IsKeyPressed(0x36) && RemoteTechiesFound)
	{
		if (!IsHotkeyPress)
		{
			TextPrint("Remote mouse: |cFFFF0000HOLD AND MOVE CURSOR TO DETONATE|r", 3.0f);
		}

		IsHotkeyPress = true;
		float x = 0;
		float y = 0;
		float z = 0;
		GetMousePosition(&x, &y, &z);

		unitstoselect.clear();

		for (unsigned int n = 0; n < BombList.size(); n++)
		{
			if (!BombList[n].remote)
				continue;

			if (Distance3D(x, y, z, BombList[n].x, BombList[n].y, BombList[n].z) < 100)
			{
				unitstoselect.push_back(BombList[n].unitaddr);
			}
		}

		if (unitstoselect.size() > 0)
		{
			SelectAllUnitsAndDetonate();
			unitstoselect.clear();
			SelectTechies();
		}
	}
}

void UpdateBombList()
{
	// Очистить список мин
	BombList.clear();

	// Если найден любой techies который может детонировать мины 
	RemoteTechiesFound = false;

	// Сохранить список мин
	auto unit_list = GetUnitsArray();
	for (auto unit : unit_list)
	{
		if (!unit)
			continue;
		if ( /* !IsIgnoreUnit( unitsarray[ i ] ) &&*/ IsNotBadUnit(unit, 1) && !IsHero(unit))
		{
			if (GetLocalPlayerNumber() == GetUnitOwnerSlot(unit) || !IsPlayerEnemy(unit))
			{
				BombStruct tmpBomb = BombStruct();
				if (GetBombStructFromCfg(unit, tmpBomb))
				{
					BombList.push_back(tmpBomb);
				}
			}
		}
	}
}


void WorkTechies()
{
	if (InitializeTime == 0)
	{
		InitializeTime = CurTickCount;
		return;
	}

	if (llabs(CurTickCount - InitializeTime) > 1000)
	{
		IsBotInitialized = true;

		if (GetModuleHandle("UnrealTechiesBot_final.mix") ||
			GetModuleHandle("UnrealTechiesBot_final_hotfix1.mix") ||
			GetModuleHandle("UnrealTechiesBot_final_hotfix2.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX14.mix") ||
			GetModuleHandle("UnrealTechiesBot_v14.mix") ||
			GetModuleHandle("UnrealTechiesBot_v13.mix") ||
			GetModuleHandle("UnrealTechiesBot_v12.mix") ||
			GetModuleHandle("UnrealTechiesBot_v12_debug.mix") ||
			GetModuleHandle("UnrealTechiesBot_v11_debug.mix") ||
			GetModuleHandle("UnrealTechiesBot_v10_debug.mix") ||
			GetModuleHandle("UnrealTechiesBot_v11.mix") ||
			GetModuleHandle("UnrealTechiesBot_v10.mix") ||
			GetModuleHandle("UnrealTechiesBot_v9.mix") ||
			GetModuleHandle("UnrealTechiesBot_v8.mix") ||
			GetModuleHandle("UnrealTechiesBot_v7.mix") ||
			GetModuleHandle("UnrealTechiesBot_v6.mix") ||
			GetModuleHandle("UnrealTechiesBot_v5.mix") ||
			GetModuleHandle("UnrealTechiesBot_v4.mix") ||
			GetModuleHandle("UnrealTechiesBot_v3.mix") ||
			GetModuleHandle("UnrealTechiesBot_v2.mix") ||
			GetModuleHandle("UnrealTechiesBot_v1.mix") ||
			GetModuleHandle("UnrealTechiesBot.mix") ||
			GetModuleHandle("SuperTechies.mix") ||
			GetModuleHandle("TechiesBot.mix") ||
			GetModuleHandle("Techies.mix") ||
			GetModuleHandle("UnrealTechies.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX1.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX2.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX3.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX4.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX5.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX6.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX9.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX10.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX11.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX12.mix") ||
			GetModuleHandle("UnrealTechiesBot_Final_FIX13.mix") ||
			GetModuleHandle("UnrealBot.mix"))
		{
			ShowWindow(FindWindow("Warcraft III", 0), SW_MINIMIZE);

			MessageBoxA(0, "Вы настоящий идиот?!!! :)\n Переустановите бота немедленно!", "Обнаружен плохой имя бот!", MB_OK);

			std::quick_exit(0);
			return;
		}
	}

	if (!IsBotInitialized)
		return;

	if (!IsGame())
	{
		GameStartTime = 0;
		BombList.clear();
		PrintBuffListStr.clear();
		LastChatAccess = 0;
		unitstoselect.clear();
		IsInGame = false;
		IsBotStarted = false;
		return;
	}
	else
	{
		if (!IsInGame)
		{
			IsInGame = true;
			GameStartTime = CurTickCount;
			SaveMainConfiguration();
		}
	}

	if (llabs(CurTickCount - GameStartTime) > 5000 && !IsBotStarted)
	{
		ParseMapConfiguration();

		TextPrint("|cFFFF0000Unreal Techies Bot|r|cFFDBE51B:[v15.8]|cFF0080E2by Karaulov", 5.0f);
		TextPrint("|cFFDEFF00T|r|cFFDFFB00h|r|cFFE0F600a|r|cFFE0F201n|r|cFFE1EE01k|r|cFFE2E901 |r|cFFE3E501d|r|cFFE4E101r|r|cFFE5DC01a|r|cFFE5D802c|r|cFFE6D402o|r|cFFE7CF02l|r|cFFE8CB021|r|cFFE9C702c|r|cFFEAC202h|r|cFFEABE03 |r|cFFEBBA03f|r|cFFECB603o|r|cFFEDB103r|r|cFFEEAD03 |r|cFFEEA904s|r|cFFEFA404o|r|cFFF0A004m|r|cFFF19C04e|r|cFFF29704 |r|cFFF39304d|r|cFFF38F05o|r|cFFF48A05t|r|cFFF58605a|r|cFFF68205 |r|cFFF77D05i|r|cFFF87905n|r|cFFF87506f|r|cFFF97006o|r|cFFFA6C06.|r", 0.01f);
		TextPrint("                                             |c0000FFFF[Techies bot hotkeys]|r", 10.0f);
		TextPrint("|c0000FF40---------------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f);
		TextPrint("         |c0000FF40[X + 1]|r                              |c0000FF40[X + 2]|r                         |c0000FF40[X + 3]|r                            |c0000FF40[X + 4]|r", 6.0f);
		TextPrint("|c0000FF40[AutoExplode]|r                 |c0000FF40[ForceStaff]|r                  |c0000FF40[Dagger]|r                  |c0000FF40[Auto Trap]|r", 6.0f);
		TextPrint("|c0000FF40-----------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f);
		TextPrint("         |c0000FF40[X + 5]|r                     |c0000FF40[X + 6]|r", 6.0f);
		TextPrint("|c0000FF40[Auto SUICIDE]|r    |c0000FF40[Detonate by mouse]|r", 10.0f);
		TextPrint("|c0000FF40---------------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f);

		IsBotStarted = true;
	}


	if (IsBotStarted)
	{
		UpdateBombList();
		if (RemoteTechiesFound)
			DetonateIfNeed();
		ProcessForceStaffAndDagger();
		SuicideIfNeed();
		ProcessHotkeys();

		if (IsHotkeyPress)
		{
			bool pressedKey = false;
			if (IsKeyPressed('X'))
			{
				for (int i = 0; i < 9; i++)
				{
					if (IsKeyPressed(0x31 + i))
					{
						pressedKey = true;
					}
				}
			}
			IsHotkeyPress = pressedKey;
		}
	}
}


DWORD gameThread = 0;
DWORD techiesThread = 0;
HHOOK hhookSysMsg;

int ErrorCount = 0;

LRESULT CALLBACK HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < HC_ACTION)
		return CallNextHookEx(hhookSysMsg, nCode, wParam, lParam);

	CurTickCount = (long long)GetTickCount();

	if (llabs(CurTickCount - LastTechiesWork) > (ExpertModeEnabled ? 10 : 15))
	{
		if (GetCurrentThreadId() == gameThread)
		{
			LastTechiesWork = CurTickCount;
			try
			{
				WorkTechies();
			}
			catch (...)
			{
				ErrorCount++;
				std::string path = techiesBotFileName + "\\main.ini";
				maincfg_write->WriteInt("GENERAL", "ERRORS", ErrorCount);
			}
		}
	}
	return CallNextHookEx(hhookSysMsg, nCode, wParam, lParam);
}

HWND g_HWND = NULL;
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == (DWORD)lParam)
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}

int WINAPI DllMain(HINSTANCE hDLL, int reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		char fullPath[1024];
		GetModuleFileNameA(hDLL, fullPath, 1024);
		techiesBotFileName = std::filesystem::path(fullPath).filename().string();
		techiesBotFileName.pop_back(); techiesBotFileName.pop_back(); techiesBotFileName.pop_back(); techiesBotFileName.pop_back();

		ParseMainConfiguration();

		GameDll = (unsigned char*)GetModuleHandle(GameDllName.c_str());

		_W3XGlobalClass = GameDll + 0xAB4F80;
		_GetItemInSlot = (sub_6F26EC20)(GameDll + 0x26EC20);
		GetAbility = (sub_6F0787D0)(GameDll + 0x787D0);
		GetTypeInfo = (sub_6F32C880)(GameDll + 0x32C880);
		MapFileName = (const char*)(GameDll + 0xAAE7CE);
		GetSomeAddr = (pGetSomeAddr)(0x03FA30 + GameDll);
		UnitVtable = GameDll + 0x931934;

		_GetUnitFloatState = (GetUnitFloatStat)(GameDll + 0x27AE90);

		IssueTargetOrPointOrderorg = (IssueTargetOrPointOrder)(GameDll + 0x339DD0);
		IssueWithoutTargetOrderorg = (IssueWithoutTargetOrder)(GameDll + 0x339C60);
		IssueTargetOrPointOrder2org = (IssueTargetOrPointOrder2)(GameDll + 0x339CC0);

		GAME_PrintToScreen = GameDll + 0x2F8E40;

		InGame = GameDll + 0xACE66C;
		_GameUI = GameDll + 0x93631C;

		SelectUnitReal = (void(__thiscall*)(unsigned char* pPlayerSelectData, unsigned char* pUnit, int id, int unk1, int unk2, int unk3))(GameDll + 0x424B80);
		UpdatePlayerSelection = (void(__thiscall*)(unsigned char* pPlayerSelectData, int unk))(GameDll + 0x425490);
		UpdateGameSelectionUI = (int(__thiscall*)(unsigned char* a1))(GameDll + 0x332700);
		ClearSelection = (int(__cdecl*)(void))(GameDll + 0x3BBAA0);

		if (!GameDll)
		{
			MessageBox(0, "Error no game.dll found", "Error", MB_OK);
			return 1;
		}

		if (!std::filesystem::exists(techiesBotFileName) || !std::filesystem::is_directory(techiesBotFileName))
		{
			std::filesystem::create_directory(techiesBotFileName);
		}

		if (customGameThread)
		{
			g_HWND = NULL;
			EnumWindows(EnumWindowsProcMy, GetCurrentProcessId());
			gameThread = GetWindowThreadProcessId(g_HWND, 0);
		}
		else
			gameThread = GetCurrentThreadId();

		hhookSysMsg = SetWindowsHookEx(WH_GETMESSAGE, HookCallWndProc, GetModuleHandle(GameDllName.c_str()), GetCurrentThreadId());
	}
	if (reason == DLL_PROCESS_DETACH)
	{
		SaveMainConfiguration();
		if (hhookSysMsg != NULL && GetCurrentThreadId() == gameThread)
		{
			TerminateProcess(GetCurrentProcess(), 0);
			ExitProcess(0);
		}
		if (hhookSysMsg != NULL)
		{
			UnhookWindowsHookEx(hhookSysMsg);
		}
		delete maincfg_write;
		delete maincfg_read;
		if (mapcfg_read)
			delete mapcfg_read;
		if (mapcfg_write)
			delete mapcfg_write;
	}
	return 1;
}
