#include "prediction.h"
#include <random>

extern Memory apex_mem;

extern bool firing_range;
extern bool show_shield;
extern bool ViewWarn;
extern float item_glow_dist;
extern uint64_t g_Base;
extern char mapname[64];
float smooth = 20.0f;
bool aim_no_recoil = false;
int bone = 3;
float curTime;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> smoothd(-3.0, 3.0);
std::uniform_int_distribution<int> bones(0.0, bone);

bool lowHealth = false;
bool freedm = false;
bool control = false;

float 红色物品col[] = {1.0f, 0.305882f, 0.113725f};
float 紫色物品col[] = {0.490196f, 0, 1.0f};
float 金色物品col[] = {1.0f, 0.803922f, 0.235294f};
float 蓝色物品col[] = {0.117647f, 0.564706f, 1.0f};

float playerglow_vis[3] = {0.0f, 0.837104f, 0.056f};	   // 可见敌人
float playerglow_vis_down[3] = {0.0f, 0.0f, 1.0f};		   // 可见倒地敌人
float playerglow_disvis[3] = {0.914416f, 0.004525f, 1.0f}; // 不可见敌人
float playerglow_disvis_down[3] = {0.0f, 0.0f, 1.0f};	   // 不可见倒地敌人

float GREEN[3] = {0.0f, 1.0f, 0.0f};
float RED[3] = {1.0f, 0.0f, 0.0f};
float BLUE[3] = {0.0f, 0.83529411f, 1.0f};
float PURPLE[3] = {0.70588f, 0.0f, 1.0f};
float ORANGE[3] = {1.0f, 0.6470588f, 0.0f};
float WHITE[3] = {1.0f, 1.0f, 1.0f};
float GOLD[3] = {1.0f, 0.933333f, 0.0f};
int mode1 = 126, mode2 = 9, mode3 = 46, mode4 = 30;

struct GlowMode
{
	int8_t GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
};

bool Entity::Observing(uint64_t entitylist)
{
	/*uint64_t index = *(uint64_t*)(buffer + OFFSET_OBSERVING_TARGET);
	index &= ENT_ENTRY_MASK;
	if (index > 0)
	{
		uint64_t centity2 = apex_mem.Read<uint64_t>(entitylist + ((uint64_t)index << 5));
		return centity2;
	}
	return 0;*/
	return *(bool *)(buffer + OFFSET_OBSERVER_MODE);
}

// https://github.com/CasualX/apexbot/blob/master/src/state.cpp#L104
void get_class_name(uint64_t entity_ptr, char *out_str)
{
	uint64_t client_networkable_vtable;
	apex_mem.Read<uint64_t>(entity_ptr + 8 * 3, client_networkable_vtable);

	uint64_t get_client_class;
	apex_mem.Read<uint64_t>(client_networkable_vtable + 8 * 3, get_client_class);

	uint32_t disp;
	apex_mem.Read<uint32_t>(get_client_class + 3, disp);
	const uint64_t client_class_ptr = get_client_class + disp + 7;

	ClientClass client_class;
	apex_mem.Read<ClientClass>(client_class_ptr, client_class);

	apex_mem.ReadArray<char>(client_class.pNetworkName, out_str, 32);
}
void get_sign_name(uint64_t entity_ptr, char *out_str)
{
	uint64_t signame_ptr = 0;
	apex_mem.Read<uint64_t>(entity_ptr + OFFSET_SIGN_NAME, signame_ptr);
	apex_mem.ReadArray<char>(signame_ptr, out_str, 15);
}
float GetCurTime()
{
	apex_mem.Read<float>(g_Base + OFFSET_GLOBALVAR + 0x10, curTime);
	return curTime;
}
bool get_button_state(DWORD button)
{
	DWORD a0;
	apex_mem.Read<DWORD>(g_Base + OFFSET_INPUTSYSTEM + ((button >> 5) * 4) + 0xb0, a0);
	return (a0 >> (button & 31)) & 1;
}
void GetGamemode(char *out_str)
{
	uintptr_t gameModePtr;
	apex_mem.Read<uintptr_t>(g_Base + OFFSET_GAMEMODE, gameModePtr);
	apex_mem.ReadArray<char>(gameModePtr, out_str, 15);
}
bool IsLobby()
{
	uintptr_t gameModePtr;
	apex_mem.ReadArray<char>(g_Base + OFFSET_LEVEL_NAME, mapname, 32);
	return (strncmp(mapname, "mp_lobby", 8) == 0);
}
int CheckGameMode()
{
	char gamemode[20] = {};
	GetGamemode(gamemode);
	IsLobby();
	if (strstr(mapname, "arena"))
	{
		freedm = false;
		control = true;
		return CONTROL_MODE;
	}
	else if (!strcmp(mapname, "mp_rr_party_crasher"))
	{
		freedm = false;
		control = true;
		return CONTROL_MODE;
	}
	else if (!strcmp(mapname, "mp_rr_tropic_island_mu1"))
	{
		freedm = false;
		control = true;
		return CONTROL_MODE;
	}
	else if (!strcmp(gamemode, "freedm"))
	{
		freedm = true;
		control = false;
		return FREEDM_MODE;
	}
	else if (!strcmp(gamemode, "control"))
	{
		freedm = false;
		control = true;
		return CONTROL_MODE;
	}
	else
	{
		freedm = false;
		control = false;
		return 0;
	}
}
void charge_rifle_hack(uint64_t entity_ptr)
{
	extern uint64_t g_Base;
	extern bool shooting;
	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(entity_ptr);
	float BulletSpeed = curweap.get_projectile_speed();
	int ammo = curweap.get_ammo();

	if (ammo != 0 && BulletSpeed == 1 && shooting)
	{
		apex_mem.Write<float>(g_Base + OFFSET_TIMESCALE + 0x68, std::numeric_limits<float>::min());
	}
	else
	{
		apex_mem.Write<float>(g_Base + OFFSET_TIMESCALE + 0x68, 1.f);
	}
}

bool ColCheck(float a[], float b[])
{
	if (fabs(a[0] - b[0]) < 1e-6)
		if (fabs(a[1] - b[1]) < 1e-6)
			if (fabs(a[2] - b[2]) < 1e-6)
				return true;
	return false;
}

void GlowHand(uint64_t ptr, float *color)
{
	if (ViewWarn)
	{
		apex_mem.Write<GlowMode>(ptr + GLOW_TYPE, {(int8_t)mode1, (int8_t)mode2, (int8_t)mode3, (int8_t)mode4});
		apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 1);
		apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 2);
		apex_mem.Write<float>(ptr + 0x1d0, color[0]);
		apex_mem.Write<float>(ptr + 0x1d4, color[1]);
		apex_mem.Write<float>(ptr + 0x1d8, color[2]);
	}
	else
	{
		apex_mem.Write<int>(ptr + OFFSET_GLOW_T1, 0);
		apex_mem.Write<int>(ptr + OFFSET_GLOW_T2, 0);
		apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 2);
		apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 5);
	}
}
void GlowHandDisable(uint64_t ptr)
{

	apex_mem.Write<int>(ptr + OFFSET_GLOW_T1, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T2, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 2);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 5);
}

int Entity::getTeamId()
{
	return *(int *)(buffer + OFFSET_TEAM);
}

int Entity::getHealth()
{
	return *(int *)(buffer + OFFSET_HEALTH);
}

int Entity::getShield()
{
	return *(int *)(buffer + OFFSET_SHIELD);
}

int Entity::getShieldType()
{
	int type = 0;
	apex_mem.Read<int>(ptr + OFFSET_SHIELD_TYPE, type);
	return type;
}

Vector Entity::getAbsVelocity()
{
	return *(Vector *)(buffer + OFFSET_ABS_VELOCITY);
}

Vector Entity::getPosition()
{
	return *(Vector *)(buffer + OFFSET_ORIGIN);
}

bool Entity::isPlayer()
{
	char signame[30] = {};
	get_sign_name(ptr, signame);
	/*if(strncmp(signame, "player", 6) == 0){
		printf("player\n");
	}*/
	return (strncmp(signame, "player", 6) == 0);
	// return *(uint64_t*)(buffer + OFFSET_NAME) == 125780153691248;
}

bool Entity::isDummy()
{
	char class_name[30] = {};
	get_sign_name(ptr, class_name);

	return strncmp(class_name, "CAI_BaseNPC", 11) == 0;
}

bool Entity::isWorld()
{
	char class_name[30] = {};
	get_sign_name(ptr, class_name);
	// printf("%s\n", class_name);
	return strncmp(class_name, "worldspawn", 10) == 0;
}

bool Entity::isKnocked()
{
	return *(int *)(buffer + OFFSET_BLEED_OUT_STATE) > 0;
}

bool Entity::isAlive()
{
	return *(int *)(buffer + OFFSET_LIFE_STATE) == 0;
}

float Entity::lastVisTime()
{
	return *(float *)(buffer + OFFSET_VISIBLE_TIME);
}
bool Entity::IsVisible()
{
	GetCurTime();
	float visibleTime;
	// float visibleTime = *(float *)(buffer + OFFSET_VISIBLE_TIME);
	apex_mem.Read<float>(ptr + OFFSET_VISIBLE_TIME, visibleTime);
	return (visibleTime > 0.0f && fabsf(visibleTime - curTime) < 0.13f);
}
Vector Entity::getBonePosition(int id)
{
	Vector position = getPosition();
	uintptr_t boneArray = *(uintptr_t *)(buffer + OFFSET_BONES);
	Vector bone = Vector();
	uint32_t boneloc = (id * 0x30);
	Bone bo = {};
	apex_mem.Read<Bone>(boneArray + boneloc, bo);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

// https://www.unknowncheats.me/forum/apex-legends/496984-getting-hitbox-positions-cstudiohdr-externally.html
// https://www.unknowncheats.me/forum/3499185-post1334.html
// https://www.unknowncheats.me/forum/3562047-post11000.html
Vector Entity::getBonePositionByHitbox(int id)
{
	Vector origin = getPosition();

	// BoneByHitBox
	uint64_t Model = *(uint64_t *)(buffer + OFFSET_STUDIOHDR);

	// get studio hdr
	uint64_t StudioHdr;
	apex_mem.Read<uint64_t>(Model + 0x8, StudioHdr);

	// get hitbox array
	uint16_t HitboxCache;
	apex_mem.Read<uint16_t>(StudioHdr + 0x34, HitboxCache);
	uint64_t HitBoxsArray = StudioHdr + ((uint16_t)(HitboxCache & 0xFFFE) << (4 * (HitboxCache & 1)));

	uint16_t IndexCache;
	apex_mem.Read<uint16_t>(HitBoxsArray + 0x4, IndexCache);
	int HitboxIndex = ((uint16_t)(IndexCache & 0xFFFE) << (4 * (IndexCache & 1)));

	uint16_t Bone;
	int boneId = bones(gen);
	apex_mem.Read<uint16_t>(HitBoxsArray + HitboxIndex + (boneId * 0x20), Bone);

	if (Bone < 0 || Bone > 255)
		return Vector();

	// hitpos
	uint64_t BoneArray = *(uint64_t *)(buffer + OFFSET_BONES);

	matrix3x4_t Matrix = {};
	apex_mem.Read<matrix3x4_t>(BoneArray + Bone * sizeof(matrix3x4_t), Matrix);

	return Vector(Matrix.m_flMatVal[0][3] + origin.x, Matrix.m_flMatVal[1][3] + origin.y, Matrix.m_flMatVal[2][3] + origin.z);
}

QAngle Entity::GetSwayAngles()
{
	return *(QAngle *)(buffer + OFFSET_BREATH_ANGLES);
}

QAngle Entity::GetViewAngles()
{
	return *(QAngle *)(buffer + OFFSET_VIEWANGLES);
}

Vector Entity::GetViewAnglesV()
{
	return *(Vector *)(buffer + OFFSET_VIEWANGLES);
}

float Entity::GetYaw()
{
	float yaw = 0;
	apex_mem.Read<float>(ptr + OFFSET_YAW, yaw);

	if (yaw < 0)
		yaw += 360;
	yaw += 90;
	if (yaw > 360)
		yaw -= 360;

	return yaw;
}

bool Entity::isGlowing()
{
	return *(int *)(buffer + OFFSET_GLOW_ENABLE) == 1;
}

bool Entity::isZooming()
{
	return *(int *)(buffer + OFFSET_ZOOMING) == 1;
}
bool is_lowHealth(int shield, int health)
{
	if (50 >= (shield + health))
		lowHealth = true;
	else
		lowHealth = false;
	return lowHealth;
}
float *Set_Color(int shield_type, int shield_health, int health, int teamid, float *&set_color)
{
	float *color = 0;

	if (freedm)
	{
		switch (teamid % 4)
		{
		case 0:
			color = RED;
			break;
		case 1:
			color = PURPLE;
			break;
		case 2:
			color = BLUE;
			break;
		case 3:
			color = GOLD;
			break;
		}
	}
	else if (show_shield)
	{
		if (shield_health == 0)
		{
			color = WHITE;
		}
		else
			switch (shield_type)
			{
			case 0:
				color = WHITE;
				break;
			case 1:
				color = WHITE;
				break;
			case 2:
				color = BLUE;
				break;
			case 3:
				color = PURPLE;
				break;
			case 4:
				color = GOLD;
				break;
			case 5:
				color = RED;
				break;
			default:
				color = WHITE;
				break;
			}
	}
	else
	{
		color = playerglow_disvis;
	}
	set_color = color;
	return color;
}
void Entity::enableGlow(float *color, bool vis, bool down)
{
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T1, 16256);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T2, 1193322764);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 1);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 1);
	if (lowHealth)
		apex_mem.Write<GlowMode>(ptr + GLOW_TYPE, {117, 117, 46, 95});
	else
		apex_mem.Write<GlowMode>(ptr + GLOW_TYPE, {101, 101, 46, 95});
	apex_mem.Write<float>(ptr + GLOW_DISTANCE, 1200.0f * METER_TO_FLOAT);
	if (vis)
	{
		if (down)
		{
			color = playerglow_vis_down;
		}
		else
		{
			color = playerglow_vis;
		}
	}
	else if (down)
		color = playerglow_disvis_down;

	if (color != nullptr)
	{
		apex_mem.Write<float>(ptr + OFFSET_GLOW_COLOR_R, color[0] * 45);
		apex_mem.Write<float>(ptr + OFFSET_GLOW_COLOR_G, color[1] * 45);
		apex_mem.Write<float>(ptr + OFFSET_GLOW_COLOR_B, color[2] * 45);
	}
}

void Entity::disableGlow()
{
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T1, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_T2, 0);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_ENABLE, 2);
	apex_mem.Write<int>(ptr + OFFSET_GLOW_THROUGH_WALLS, 5);
}

void Entity::SetViewAngles(SVector angles)
{
	apex_mem.Write<SVector>(ptr + OFFSET_VIEWANGLES, angles);
}

void Entity::SetViewAngles(QAngle &angles)
{
	SetViewAngles(SVector(angles));
}

Vector Entity::GetCamPos()
{
	return *(Vector *)(buffer + OFFSET_CAMERAPOS);
}

QAngle Entity::GetRecoil()
{
	return *(QAngle *)(buffer + OFFSET_AIMPUNCH);
}

void Entity::get_name(uint64_t g_Base, uint64_t index, char *name)
{
	index *= 0x10;
	uint64_t name_ptr = 0;
	apex_mem.Read<uint64_t>(g_Base + OFFSET_NAME_LIST + index, name_ptr);
	apex_mem.ReadArray<char>(name_ptr, name, 64);
}

bool Item::isItem()
{
	char class_name[33] = {};
	get_class_name(ptr, class_name);

	return strncmp(class_name, "CPropSurvival", 13) == 0;
}
int Item::getRarityLevel()
{
	float r, g, b;
	// int itemid;apex_mem.Read<int>(ptr + OFFSET_ITEM_ID, itemid);
	r = *(float *)(buffer + OFFSET_ITEM_COLOR_R);
	g = *(float *)(buffer + OFFSET_ITEM_COLOR_G);
	b = *(float *)(buffer + OFFSET_ITEM_COLOR_B);
	/*char modelName[100]={};
	uintptr_t itemModelName_ptr;
	apex_mem.Read<uintptr_t>(ptr + OFFSET_MODEL_NAME, itemModelName_ptr);
	apex_mem.ReadArray<char>(itemModelName_ptr, modelName, 80);*/
	float col[] = {r, g, b};
	if (ColCheck(col, 红色物品col))
		return 红色物品;
	else if (ColCheck(col, 紫色物品col))
		return 紫色物品;
	else if (ColCheck(col, 金色物品col))
		return 金色物品;
	else if (ColCheck(col, 蓝色物品col))
		return 蓝色物品;
	else
		return 白色物品;
}

bool Item::isGlowing()
{
	return *(int *)(buffer + OFFSET_ITEM_GLOW) == 1363184265;
}

void Item::enableGlow()
{
	apex_mem.Write<int>(ptr + OFFSET_ITEM_GLOW, 1363184265);
	apex_mem.Write<float>(ptr + GLOW_DISTANCE, item_glow_dist * METER_TO_FLOAT);
}

void Item::disableGlow()
{
	apex_mem.Write<int>(ptr + OFFSET_ITEM_GLOW, 1411417991);
}

Vector Item::getPosition()
{
	return *(Vector *)(buffer + OFFSET_ORIGIN);
}

float CalculateFov(Entity &from, Entity &target)
{
	QAngle ViewAngles = from.GetViewAngles();
	Vector LocalCamera = from.GetCamPos();
	Vector EntityPosition = target.getPosition();
	QAngle Angle = Math::CalcAngle(LocalCamera, EntityPosition);
	return Math::GetFov(ViewAngles, Angle);
}

QAngle CalculateBestBoneAim(Entity &from, uintptr_t t, float max_fov)
{
	Entity target = getEntity(t);
	if (firing_range)
	{
		if (!target.isAlive())
		{
			return QAngle(0, 0, 0);
		}
	}
	else
	{
		if (!target.isAlive() || target.isKnocked())
		{
			return QAngle(0, 0, 0);
		}
	}

	Vector LocalCamera = from.GetCamPos();
	Vector TargetBonePosition = target.getBonePositionByHitbox(bone);
	QAngle CalculatedAngles = QAngle(0, 0, 0);

	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(from.ptr);
	float BulletSpeed = curweap.get_projectile_speed();
	float BulletGrav = curweap.get_projectile_gravity();
	float zoom_fov = curweap.get_zoom_fov();

	if (zoom_fov != 0.0f && zoom_fov != 1.0f)
	{
		max_fov *= zoom_fov / 90.0f;
	}

	/*
	//simple aim prediction
	if (BulletSpeed > 1.f)
	{
		Vector LocalBonePosition = from.getBonePosition(bone);
		float VerticalTime = TargetBonePosition.DistTo(LocalBonePosition) / BulletSpeed;
		TargetBonePosition.z += (BulletGrav * 0.5f) * (VerticalTime * VerticalTime);

		float HorizontalTime = TargetBonePosition.DistTo(LocalBonePosition) / BulletSpeed;
		TargetBonePosition += (target.getAbsVelocity() * HorizontalTime);
	}
	*/

	// more accurate prediction
	if (BulletSpeed > 1.f)
	{
		PredictCtx Ctx;
		Ctx.StartPos = LocalCamera;
		Ctx.TargetPos = TargetBonePosition;
		Ctx.BulletSpeed = BulletSpeed - (BulletSpeed * 0.08);
		Ctx.BulletGravity = BulletGrav + (BulletGrav * 0.05);
		Ctx.TargetVel = target.getAbsVelocity();

		if (BulletPredict(Ctx))
			CalculatedAngles = QAngle{Ctx.AimAngles.x, Ctx.AimAngles.y, 0.f};
	}

	if (CalculatedAngles == QAngle(0, 0, 0))
		CalculatedAngles = Math::CalcAngle(LocalCamera, TargetBonePosition);
	QAngle ViewAngles = from.GetViewAngles();
	QAngle SwayAngles = from.GetSwayAngles();
	// remove sway and recoil
	if (aim_no_recoil)
		CalculatedAngles -= SwayAngles - ViewAngles;
	Math::NormalizeAngles(CalculatedAngles);
	QAngle Delta = CalculatedAngles - ViewAngles;
	double fov = Math::GetFov(SwayAngles, CalculatedAngles);
	if (fov > max_fov)
	{
		return QAngle(0, 0, 0);
	}

	Math::NormalizeAngles(Delta);
	Delta.y /= 80;

	float rdsmooth = smoothd(gen);
	QAngle SmoothedAngles = ViewAngles + Delta / (smooth + rdsmooth);
	return SmoothedAngles;
}

Entity getEntity(uintptr_t ptr)
{
	Entity entity = Entity();
	entity.ptr = ptr;
	apex_mem.ReadArray<uint8_t>(ptr, entity.buffer, sizeof(entity.buffer));
	return entity;
}

Item getItem(uintptr_t ptr)
{
	Item entity = Item();
	entity.ptr = ptr;
	apex_mem.ReadArray<uint8_t>(ptr, entity.buffer, sizeof(entity.buffer));
	return entity;
}

bool WorldToScreen(Vector from, float *m_vMatrix, int targetWidth, int targetHeight, Vector &to)
{
	float w = m_vMatrix[12] * from.x + m_vMatrix[13] * from.y + m_vMatrix[14] * from.z + m_vMatrix[15];

	if (w < 0.01f)
		return false;

	to.x = m_vMatrix[0] * from.x + m_vMatrix[1] * from.y + m_vMatrix[2] * from.z + m_vMatrix[3];
	to.y = m_vMatrix[4] * from.x + m_vMatrix[5] * from.y + m_vMatrix[6] * from.z + m_vMatrix[7];

	float invw = 1.0f / w;
	to.x *= invw;
	to.y *= invw;

	float x = targetWidth / 2;
	float y = targetHeight / 2;

	x += 0.5 * to.x * targetWidth + 0.5;
	y -= 0.5 * to.y * targetHeight + 0.5;

	to.x = x;
	to.y = y;
	to.z = 0;
	return true;
}

void WeaponXEntity::update(uint64_t LocalPlayer)
{
	extern uint64_t g_Base;
	uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
	uint64_t wephandle = 0;
	apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wephandle);

	wephandle &= 0xffff;

	wep_entity = 0;
	apex_mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);

	projectile_speed = 0;
	apex_mem.Read<float>(wep_entity + OFFSET_BULLET_SPEED, projectile_speed);
	projectile_scale = 0;
	apex_mem.Read<float>(wep_entity + OFFSET_BULLET_SCALE, projectile_scale);
	zoom_fov = 0;
	apex_mem.Read<float>(wep_entity + OFFSET_ZOOM_FOV, zoom_fov);
	ammo = 0;
	apex_mem.Read<int>(wep_entity + OFFSET_AMMO, ammo);
}

float WeaponXEntity::get_projectile_speed()
{
	return projectile_speed;
}

float WeaponXEntity::get_projectile_gravity()
{
	return 750.0f * projectile_scale;
}

float WeaponXEntity::get_zoom_fov()
{
	return zoom_fov;
}

int WeaponXEntity::get_ammo()
{
	return ammo;
}
