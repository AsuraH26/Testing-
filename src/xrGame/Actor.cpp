#include "stdafx.h"
#include "pch_script.h"
#include "Actor_Flags.h"
#include "HUDManager.h"
#ifdef DEBUG

#	include "PHDebug.h"
#endif // DEBUG
#include "alife_space.h"
#include "Hit.h"
#include "PHDestroyable.h"
#include "Car.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "CameraLook.h"
#include "CameraFirstEye.h"
#include "effectorfall.h"
#include "EffectorBobbing.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "SleepEffector.h"
#include "character_info.h"
#include "CustomOutfit.h"
#include "actorcondition.h"
#include "UIGameCustom.h"
#include "../xrPhysics/matrix_utils.h"
#include "clsid_game.h"
#include "game_cl_base_weapon_usage_statistic.h"
#include "Grenade.h"
#include "Torch.h"

// breakpoints
#include "../xrEngine/xr_input.h"
//
#include "Actor.h"
#include "ActorAnimation.h"
#include "actor_anim_defs.h"
#include "HudItem.h"
#include "ai_space.h"
#include "trade.h"
#include "Inventory.h"
//#include "Physics.h"
#include "Level.h"
#include "GamePersistent.h"
#include "game_cl_base.h"
#include "game_cl_single.h"
#include "xrMessages.h"
#include "../xrEngine/string_table.h"
#include "UsableScriptObject.h"
#include "../xrEngine/cl_intersect.h"
//#include "ExtendedGeom.h"
#include "alife_registry_wrappers.h"
#include "../Include/xrRender/Kinematics.h"
#include "Artefact.h"
#include "CharacterPhysicsSupport.h"
#include "material_manager.h"
#include "../xrPhysics/IColisiondamageInfo.h"
#include "ui/UIMainIngameWnd.h"
#include "map_manager.h"
#include "GameTaskManager.h"
#include "actor_memory.h"
#include "script_game_object.h"
#include "Game_Object_Space.h"
#include "InventoryBox.h"
#include "location_manager.h"
#include "player_hud.h"
#include "ai/monsters/basemonster/base_monster.h"

#include "../Include/xrRender/UIRender.h"

#include "ai_object_location.h"
#include "ui/UIMotionIcon.h"
#include "ui/UIActorMenu.h"
#include "ActorHelmet.h"
#include "UI/UIDragDropReferenceList.h"
#include "UIFontDefines.h"
#include "PickupManager.h"
#include "HUDAnimItem.h"
#include "ai/monsters/controller/controller.h"
#include "WeaponKnife.h"
#include "WeaponBinoculars.h"

const u32		patch_frames	= 50;
const float		respawn_delay	= 1.f;
const float		respawn_auto	= 7.f;

static float IReceived = 0;
static float ICoincidenced = 0;
extern float cammera_into_collision_shift ;

string32		ACTOR_DEFS::g_quick_use_slots[4]={0, 0, 0, 0};
//skeleton



static Fbox		bbStandBox;
static Fbox		bbCrouchBox;
static Fvector	vFootCenter;
static Fvector	vFootExt;

Flags32			psActorFlags={AF_DISABLE_CONDITION_TEST|AF_AUTOPICKUP|AF_RUN_BACKWARD|AF_IMPORTANT_SAVE|AF_DISPLAY_VOICE_ICON};
int				psActorSleepTime = 1;



CActor::CActor() : CEntityAlive(),current_ik_cam_shift(0)
{
	game_news_registry		= new CGameNewsRegistryWrapper();
	// Cameras
	cameras[eacFirstEye] = new CCameraFirstEye(this, CCameraBase::flKeepPitch);
	cameras[eacFirstEye]->Load("actor_firsteye_cam");

	cameras[eacLookAt] = new CCameraLook2(this);
	cameras[eacLookAt]->Load("actor_look_cam_psp");

	cameras[eacFreeLook]	= new CCameraLook					(this);
	cameras[eacFreeLook]->Load("actor_free_cam");
	cameras[eacFixedLookAt]	= new CCameraFixedLook				(this);
	cameras[eacFixedLookAt]->Load("actor_look_cam");

	cam_active				= eacFirstEye;
	fPrevCamPos				= 0.0f;
	vPrevCamDir.set			(0.f,0.f,1.f);
	fCurAVelocity			= 0.0f;
	fFPCamYawMagnitude = 0.0f; //--#SM+#--
	fFPCamPitchMagnitude = 0.0f; //--#SM+#--
	// эффекторы
	pCamBobbing				= 0;


	r_torso.yaw				= 0;
	r_torso.pitch			= 0;
	r_torso.roll			= 0;
	r_torso_tgt_roll		= 0;
	r_model_yaw				= 0;
	r_model_yaw_delta		= 0;
	r_model_yaw_dest		= 0;

	b_DropActivated			= 0;
	f_DropPower				= 0.f;

	m_fRunFactor			= 2.f;
	m_fCrouchFactor			= 0.2f;
	m_fClimbFactor			= 1.f;
	m_fCamHeightFactor		= 0.87f;

	m_fFallTime				=	s_fFallTime;
	m_bAnimTorsoPlayed		=	false;

	m_pPhysicsShell			=	nullptr;

	m_fFeelGrenadeRadius	=	10.0f;
	m_fFeelGrenadeTime      =	1.0f;
	
	m_holder				=	nullptr;
	m_holderID				=	u16(-1);


#ifdef DEBUG
	Device.seqRender.Add	(this,REG_PRIORITY_LOW);
#endif

	//разрешить использование пояса в inventory
	inventory().SetBeltUseful(true);

	m_pPersonWeLookingAt	= nullptr;
	m_pVehicleWeLookingAt	= nullptr;
	m_pObjectWeLookingAt	= nullptr;
	pPickup = new CPickUpManager(this);

	pStatGraph				= nullptr;

	m_pActorEffector		= nullptr;

	SetZoomAimingMode		(false);

	m_sDefaultObjAction		= nullptr;

	m_fSprintFactor			= 4.f;

	//hFriendlyIndicator.create(FVF::F_LIT,RCache.Vertex.Buffer(),RCache.QuadIB);

	m_pUsableObject			= nullptr;


	m_anims					= new SActorMotions();
	m_vehicle_anims			= new SActorVehicleAnims();
	m_entity_condition		= nullptr;
	m_iLastHitterID			= u16(-1);
	m_iLastHittingWeaponID	= u16(-1);
	m_statistic_manager		= nullptr;
	//-----------------------------------------------------------------------------------
	m_memory				= new CActorMemory(this);
	m_bOutBorder			= false;
	m_hit_probability		= 1.f;
	m_feel_touch_characters = 0;
	//-----------------------------------------------------------------------------------
	m_dwILastUpdateTime		= 0;

	m_location_manager		= new CLocationManager(this);
	m_block_sprint_counter	= 0;

	m_disabled_hitmarks		= false;
	m_inventory_disabled	= false;

	// Alex ADD: for smooth crouch
	CurrentHeight = -1.f;
	bBlockSprint = false;

	_last_camera_height = 0.f;
	_last_cam_update_time = 0;
	_landing_effect_time_remains = 0;
	_landing2_effect_time_remains = 0;
	_landing_effect_finish_time_remains = 0;
	_keyflags = 0;

	ClearActiveControllers();
	_controlled_time_remains = 0;
	_suicide_now = false;
	_planning_suicide = false;
	_lastshot_done_time = 0;
	_death_action_started = false;
	_inventory_disabled_set = false;
	_controller_preparing_starttime = 0;
	_psi_block_failed = true;
	_last_update_time = Device.dwTimeGlobal;
	_jitter_time_remains = 0;
}


CActor::~CActor()
{
	xr_delete				(m_location_manager);
	xr_delete				(m_memory);
	xr_delete				(game_news_registry);
#ifdef DEBUG
	Device.seqRender.Remove(this);
#endif
	//xr_delete(Weapons);
	for (int i=0; i<eacMaxCam; ++i) xr_delete(cameras[i]);

	m_HeavyBreathSnd.destroy();
	m_BloodSnd.destroy		();
	m_DangerSnd.destroy		();

	xr_delete				(m_pActorEffector);

	xr_delete				(m_pPhysics_support);

	xr_delete				(m_anims);
	xr_delete				(pPickup);
	xr_delete				(m_vehicle_anims);
	ClearActiveControllers();
}

void CActor::reinit	()
{
	character_physics_support()->movement()->CreateCharacter		();
	character_physics_support()->movement()->SetPhysicsRefObject	(this);
	CEntityAlive::reinit						();
	CInventoryOwner::reinit						();

	character_physics_support()->in_Init		();
	material().reinit							();

	m_pUsableObject								= nullptr;
	if (!g_dedicated_server)
		memory().reinit							();
	
	set_input_external_handler					(0);
	m_time_lock_accel							= 0;
}

void CActor::reload	(LPCSTR section)
{
	CEntityAlive::reload		(section);
	CInventoryOwner::reload		(section);
	material().reload			(section);
	CStepManager::reload		(section);
	if (!g_dedicated_server)
		memory().reload			(section);
	m_location_manager->reload	(section);
}
void set_box(LPCSTR section, CPHMovementControl &mc, u32 box_num )
{
	Fbox	bb;Fvector	vBOX_center,vBOX_size;
	// m_PhysicMovementControl: BOX
	string64 buff, buff1;
	xr_strconcat(buff, "ph_box",_itoa( box_num, buff1, 10 ),"_center" );
	vBOX_center= pSettings->r_fvector3	(section, buff	);
	xr_strconcat(buff, "ph_box",_itoa( box_num, buff1, 10 ),"_size" );
	vBOX_size	= pSettings->r_fvector3	(section, buff);
	vBOX_size.y += cammera_into_collision_shift/2.f;
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	mc.SetBox		(box_num,bb);
}

xr_vector<xr_string> CActor::GetKnowedPortions() const
{
	static xr_vector<xr_string> SafeVector;
	SafeVector.clear();

	auto KnownInfos = m_known_info_registry->registry().objects_ptr();

	if (KnownInfos == nullptr) {
		return {};
	}

	for (auto Info : *KnownInfos)
	{
		SafeVector.push_back(Info.c_str());
	}

	return SafeVector;
}

void CActor::GiveInfoPortion(const char* infoPortion) {
	this->TransferInfo(infoPortion, true);
}

void CActor::DisableInfoPortion(const char* infoPortion) {
	this->TransferInfo(infoPortion, false);
}

void CActor::SetActorPosition(Fvector pos) {
	CCar* car = smart_cast<CCar*>(this->Holder());
	if (car)
		car->DoExit();

	Fmatrix F = this->XFORM();
	F.c = pos;
	this->ForceTransform(F);
}

void CActor::SetActorDirection(float dir) {
	this->cam_Active()->Set(dir, 0, 0);
}

void CActor::Load	(LPCSTR section )
{
	// Msg						("Loading actor: %s",section);
	inherited::Load				(section);
	material().Load				(section);
	CInventoryOwner::Load		(section);
	m_location_manager->Load	(section);

	if (IsGameTypeSingle())
		OnDifficultyChanged		();
	//////////////////////////////////////////////////////////////////////////
	ISpatial*		self			=	smart_cast<ISpatial*> (this);
	if (self)	{
		self->spatial.type	|=	STYPE_VISIBLEFORAI;
		self->spatial.type	&= ~STYPE_REACTTOSOUND;
	}
	//////////////////////////////////////////////////////////////////////////

	// m_PhysicMovementControl: General
	//m_PhysicMovementControl->SetParent		(this);


	/*
	Fbox	bb;Fvector	vBOX_center,vBOX_size;
	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box2_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box2_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(2,bb);

	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box1_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box1_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(1,bb);

	// m_PhysicMovementControl: BOX
	vBOX_center= pSettings->r_fvector3	(section,"ph_box0_center"	);
	vBOX_size	= pSettings->r_fvector3	(section,"ph_box0_size"		);
	bb.set	(vBOX_center,vBOX_center); bb.grow(vBOX_size);
	character_physics_support()->movement()->SetBox		(0,bb);
	*/
	


	
	
	
	
	//// m_PhysicMovementControl: Foots
	//Fvector	vFOOT_center= pSettings->r_fvector3	(section,"ph_foot_center"	);
	//Fvector	vFOOT_size	= pSettings->r_fvector3	(section,"ph_foot_size"		);
	//bb.set	(vFOOT_center,vFOOT_center); bb.grow(vFOOT_size);
	////m_PhysicMovementControl->SetFoots	(vFOOT_center,vFOOT_size);

	// m_PhysicMovementControl: Crash speed and mass
	float	cs_min		= pSettings->r_float	(section,"ph_crash_speed_min"	);
	float	cs_max		= pSettings->r_float	(section,"ph_crash_speed_max"	);
	float	mass		= pSettings->r_float	(section,"ph_mass"				);
	character_physics_support()->movement()->SetCrashSpeeds	(cs_min,cs_max);
	character_physics_support()->movement()->SetMass		(mass);
	if(pSettings->line_exist(section,"stalker_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtStalker,pSettings->r_float(section,"stalker_restrictor_radius"));
	if(pSettings->line_exist(section,"stalker_small_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtStalkerSmall,pSettings->r_float(section,"stalker_small_restrictor_radius"));
	if(pSettings->line_exist(section,"medium_monster_restrictor_radius"))
		character_physics_support()->movement()->SetActorRestrictorRadius(rtMonsterMedium,pSettings->r_float(section,"medium_monster_restrictor_radius"));
	character_physics_support()->movement()->Load(section);

	set_box( section, *character_physics_support()->movement(), 2 );
	set_box( section, *character_physics_support()->movement(), 1 );
	set_box( section, *character_physics_support()->movement(), 0 );

	m_fWalkAccel				= pSettings->r_float(section,"walk_accel");	
	m_fJumpSpeed				= pSettings->r_float(section,"jump_speed");
	m_fRunFactor				= pSettings->r_float(section,"run_coef");
	m_fRunBackFactor			= pSettings->r_float(section,"run_back_coef");
	m_fWalkBackFactor			= pSettings->r_float(section,"walk_back_coef");
	m_fCrouchFactor				= pSettings->r_float(section,"crouch_coef");
	m_fClimbFactor				= pSettings->r_float(section,"climb_coef");
	m_fSprintFactor				= pSettings->r_float(section,"sprint_koef");

	m_fWalk_StrafeFactor		= READ_IF_EXISTS(pSettings, r_float, section, "walk_strafe_coef", 1.0f);
	m_fRun_StrafeFactor			= READ_IF_EXISTS(pSettings, r_float, section, "run_strafe_coef", 1.0f);


	m_fCamHeightFactor			= pSettings->r_float(section,"camera_height_factor");
	character_physics_support()->movement()->SetJumpUpVelocity(m_fJumpSpeed);
	float AirControlParam		= pSettings->r_float(section,"air_control_param"	);
	character_physics_support()->movement()->SetAirControlParam(AirControlParam);

	pPickup->SetPickupRadius(pSettings->r_float(section,"pickup_info_radius"));

	m_fFeelGrenadeRadius		= pSettings->r_float(section,"feel_grenade_radius");
	m_fFeelGrenadeTime			= pSettings->r_float(section,"feel_grenade_time");
	m_fFeelGrenadeTime			*= 1000.0f;
	
	character_physics_support()->in_Load		(section);
	

if(!g_dedicated_server)
{
	LPCSTR hit_snd_sect = pSettings->r_string(section,"hit_sounds");
	for(int hit_type=0; hit_type<(int)ALife::eHitTypeMax; ++hit_type)
	{
		LPCSTR hit_name = ALife::g_cafHitType2String((ALife::EHitType)hit_type);
		LPCSTR hit_snds = READ_IF_EXISTS(pSettings, r_string, hit_snd_sect, hit_name, "");
		int cnt = _GetItemCount(hit_snds);
		string128		tmp;
		VERIFY			(cnt!=0);
		for(int i=0; i<cnt;++i)
		{
			sndHit[hit_type].push_back		(ref_sound());
			sndHit[hit_type].back().create	(_GetItem(hit_snds,i,tmp),st_Effect,sg_SourceType);
		}
		char buf[256];

		::Sound->create		(sndDie[0],			xr_strconcat(buf,*cName(),"\\die0"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[1],			xr_strconcat(buf,*cName(),"\\die1"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[2],			xr_strconcat(buf,*cName(),"\\die2"), st_Effect,SOUND_TYPE_MONSTER_DYING);
		::Sound->create		(sndDie[3],			xr_strconcat(buf,*cName(),"\\die3"), st_Effect,SOUND_TYPE_MONSTER_DYING);

		m_HeavyBreathSnd.create	(pSettings->r_string(section,"heavy_breath_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
		m_BloodSnd.create		(pSettings->r_string(section,"heavy_blood_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
		m_DangerSnd.create		(pSettings->r_string(section,"heavy_danger_snd"), st_Effect,SOUND_TYPE_MONSTER_INJURING);
	}
}

    cam_Set(eacFirstEye);

	// sheduler
	shedule.t_min				= shedule.t_max = 1;

	// настройки дисперсии стрельбы
	m_fDispBase					= pSettings->r_float		(section,"disp_base"		 );
	m_fDispBase					= deg2rad(m_fDispBase);

	m_fDispAim					= pSettings->r_float		(section,"disp_aim"		 );
	m_fDispAim					= deg2rad(m_fDispAim);

	m_fDispVelFactor			= pSettings->r_float		(section,"disp_vel_factor"	 );
	m_fDispAccelFactor			= pSettings->r_float		(section,"disp_accel_factor" );
	m_fDispCrouchFactor			= pSettings->r_float		(section,"disp_crouch_factor");
	m_fDispCrouchNoAccelFactor	= pSettings->r_float		(section,"disp_crouch_no_acc_factor");

	LPCSTR							default_outfit = READ_IF_EXISTS(pSettings,r_string,section,"default_outfit",0);
	SetDefaultVisualOutfit			(default_outfit);

	invincibility_fire_shield_1st	= READ_IF_EXISTS(pSettings,r_string,section,"Invincibility_Shield_1st",0);
	invincibility_fire_shield_3rd	= READ_IF_EXISTS(pSettings,r_string,section,"Invincibility_Shield_3rd",0);
//-----------------------------------------
	m_AutoPickUp_AABB				= READ_IF_EXISTS(pSettings,r_fvector3,section,"AutoPickUp_AABB",Fvector().set(0.02f, 0.02f, 0.02f));
	m_AutoPickUp_AABB_Offset		= READ_IF_EXISTS(pSettings,r_fvector3,section,"AutoPickUp_AABB_offs",Fvector().set(0, 0, 0));

	m_sCharacterUseAction			= "character_use";
	m_sDeadCharacterUseAction		= "dead_character_use";
	m_sDeadCharacterUseOrDragAction	= "dead_character_use_or_drag";
	m_sDeadCharacterDontUseAction	= "dead_character_dont_use";
	m_sCarTrunk						= "car_trunk_use";
	m_sCarUse						= "car_use";
	m_sCarCharacterUseAction		= "car_character_use";
	m_sInventoryItemUseAction		= "inventory_item_use";
	m_sInventoryBoxUseAction		= "inventory_box_use";
	//---------------------------------------------------------------------
	m_sHeadShotParticle	= READ_IF_EXISTS(pSettings,r_string,section,"HeadShotParticle",0);
	m_fLegs_shift = READ_IF_EXISTS(pSettings, r_float, "actor_hud", "legs_shift_delta", -0.55f);
}

void CActor::legs_shift_callback(CBoneInstance* B) {
	if(cam_active == eacFirstEye) {
		if(g_player_hud && g_player_hud->m_legs_model) {
			B->mTransform.c.mad(B->mTransform.k, m_fLegs_shift);
		}
	}
};

void CActor::PHHit(SHit &H)
{
	m_pPhysics_support->in_Hit( H, false );
}

struct playing_pred
{
	IC	bool	operator()			(ref_sound &s)
	{
		return	(nullptr != s._feedback() );
	}
};

void	CActor::Hit(SHit* pHDS)
{
	bool b_initiated = pHDS->aim_bullet; // physics strike by poltergeist

	pHDS->aim_bullet = false;

	SHit& HDS	= *pHDS;
	if( HDS.hit_type<ALife::eHitTypeBurn || HDS.hit_type >= ALife::eHitTypeMax )
	{
		string256	err;
		xr_sprintf		(err, "Unknown/unregistered hit type [%d]", HDS.hit_type);
		R_ASSERT2	(0, err );
	
	}
#ifdef DEBUG
	if(ph_dbg_draw_mask.test(phDbgCharacterControl)) {
		DBG_OpenCashedDraw();
		Fvector to;to.add(Position(),Fvector().mul(HDS.dir,HDS.phys_impulse()));
		DBG_DrawLine(Position(), to, color_xrgb(124, 124, 0));
		DBG_ClosedCashedDraw(500);
	}
#endif // DEBUG

	bool bPlaySound = true;
	if (!g_Alive()) bPlaySound = false;

	if (!IsGameTypeSingle() && !g_dedicated_server)
	{
		game_PlayerState* ps = Game().GetPlayerByGameID(ID());
		if (ps && ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE))
		{
			bPlaySound = false;
			if (Device.dwFrame != last_hit_frame &&
				HDS.bone() != BI_NONE)
			{		
				// вычислить позицию и направленность партикла
				Fmatrix pos; 

				CParticlesPlayer::MakeXFORM(this,HDS.bone(),HDS.dir,HDS.p_in_bone_space,pos);

				// установить particles
				CParticlesObject* ps_ = nullptr;

				if (eacFirstEye == cam_active && this == Level().CurrentEntity())
					ps_ = CParticlesObject::Create(invincibility_fire_shield_1st,TRUE);
				else
					ps_ = CParticlesObject::Create(invincibility_fire_shield_3rd,TRUE);

				ps_->UpdateParent(pos,Fvector().set(0.f,0.f,0.f));
				GamePersistent().ps_needtoplay.push_back(ps_);
			};
		};
		 

		last_hit_frame = Device.dwFrame;
	};

	if(	!g_dedicated_server				&& 
		!sndHit[HDS.hit_type].empty()	&&
		conditions().PlayHitSound(pHDS)	)
	{
		ref_sound& S			= sndHit[HDS.hit_type][Random.randI((u32)sndHit[HDS.hit_type].size())];
		bool b_snd_hit_playing	= sndHit[HDS.hit_type].end() != std::find_if(sndHit[HDS.hit_type].begin(), sndHit[HDS.hit_type].end(), playing_pred());

		if(ALife::eHitTypeExplosion == HDS.hit_type)
		{
			if (this == Level().CurrentControlEntity())
			{
				S.set_volume(10.0f);
				if(!m_sndShockEffector){
					m_sndShockEffector = new SndShockEffector();
					m_sndShockEffector->Start(this, float(S.get_length_sec()*1000.0f), HDS.damage() );
				}
			}
			else
				bPlaySound = false;
		}
		if (bPlaySound && !b_snd_hit_playing) 
		{
			Fvector point		= Position();
			point.y				+= CameraHeight();
			S.play_at_pos		(this, point);
		}
	}

	if (IsActorControlled() && HDS.whoID == HDS.DestID && HDS.power > 0.3f && (HDS.hit_type == ALife::EHitType::eHitTypeFireWound || HDS.hit_type == ALife::EHitType::eHitTypeExplosion))
	{
		HDS.power *= 100000.f;
		KillEntity(ID());
	}

	//slow actor, only when he gets hit
	m_hit_slowmo = conditions().HitSlowmo(pHDS);

	//---------------------------------------------------------------
	if(		(Level().CurrentViewEntity()==this) && 
			!g_dedicated_server && 
			(HDS.hit_type == ALife::eHitTypeFireWound) )
	{
		CObject* pLastHitter			= Level().Objects.net_Find(m_iLastHitterID);
		CObject* pLastHittingWeapon		= Level().Objects.net_Find(m_iLastHittingWeaponID);
		HitSector						(pLastHitter, pLastHittingWeapon);
	}

	if( (mstate_real&mcSprint) && Level().CurrentControlEntity() == this && conditions().DisableSprint(pHDS) )
	{
		bool const is_special_burn_hit_2_self	=	(pHDS->who == this) && (pHDS->boneID == BI_NONE) && 
													((pHDS->hit_type==ALife::eHitTypeBurn)||(pHDS->hit_type==ALife::eHitTypeLightBurn));
		if ( !is_special_burn_hit_2_self )
		{
			mstate_wishful	&=~mcSprint;
		}
	}
	if(!g_dedicated_server && !m_disabled_hitmarks)
	{
		bool b_fireWound = (pHDS->hit_type==ALife::eHitTypeFireWound || pHDS->hit_type==ALife::eHitTypeWound_2);
		b_initiated		 = b_initiated && (pHDS->hit_type==ALife::eHitTypeStrike);
	
		if(b_fireWound || b_initiated)
			HitMark			(HDS.damage(), HDS.dir, HDS.who, HDS.bone(), HDS.p_in_bone_space, HDS.impulse, HDS.hit_type);
	}

	if(IsGameTypeSingle())	
	{
		float hit_power				= HitArtefactsOnBelt(HDS.damage(), HDS.hit_type);

		if(GodMode())
		{
			HDS.power				= 0.0f;
			inherited::Hit			(&HDS);
			return;
		}else 
		{
			HDS.power				= hit_power;
			HDS.add_wound			= true;
			inherited::Hit			(&HDS);
		}
	}else
	{
		m_bWasBackStabbed			= false;
		if (HDS.hit_type == ALife::eHitTypeWound_2 && Check_for_BackStab_Bone(HDS.bone()))
		{
			// convert impulse into local coordinate system
			Fmatrix					mInvXForm;
			mInvXForm.invert		(XFORM());
			Fvector					vLocalDir;
			mInvXForm.transform_dir	(vLocalDir,HDS.dir);
			vLocalDir.invert		();

			Fvector a				= {0,0,1};
			float res				= a.dotproduct(vLocalDir);
			if (res < -0.707)
			{
				game_PlayerState* ps = Game().GetPlayerByGameID(ID());
				
				if (!ps || !ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE))						
					m_bWasBackStabbed = true;
			}
		};
		
		float hit_power				= 0.0f;

		if (m_bWasBackStabbed) 
			hit_power				= (HDS.damage() == 0) ? 0 : 100000.0f;
		else 
			hit_power				= HitArtefactsOnBelt(HDS.damage(), HDS.hit_type);

		HDS.power					= hit_power;
		HDS.add_wound				= true;
		inherited::Hit				(&HDS);

		if(OnServer() && !g_Alive() && HDS.hit_type==ALife::eHitTypeExplosion)
		{
			game_PlayerState* ps							= Game().GetPlayerByGameID(ID());
			Game().m_WeaponUsageStatistic->OnExplosionKill	(ps, HDS);
		}
	}
}

void CActor::HitMark	(float P, 
						 Fvector dir,			
						 CObject* who_object, 
						 s16 element, 
						 Fvector position_in_bone_space, 
						 float impulse,  
						 ALife::EHitType hit_type_)
{
	// hit marker
	if ( /*(hit_type==ALife::eHitTypeFireWound||hit_type==ALife::eHitTypeWound_2) && */
			g_Alive() && Local() && (Level().CurrentEntity()==this) )	
	{
		HUD().HitMarked				(0, P, dir);

		CEffectorCam* ce			= Cameras().GetCamEffector((ECamEffectorType)effFireHit);
		if( ce )					return;

		int id						= -1;
		Fvector						cam_pos,cam_dir,cam_norm;
		cam_Active()->Get			(cam_pos,cam_dir,cam_norm);
		cam_dir.normalize_safe		();
		dir.normalize_safe			();

		float ang_diff				= angle_difference	(cam_dir.getH(), dir.getH());
		Fvector						cp;
		cp.crossproduct				(cam_dir,dir);
		bool bUp					=(cp.y>0.0f);

		Fvector cross;
		cross.crossproduct			(cam_dir, dir);
		VERIFY						(ang_diff>=0.0f && ang_diff<=PI);

		float _s1 = PI_DIV_8;
		float _s2 = _s1 + PI_DIV_4;
		float _s3 = _s2 + PI_DIV_4;
		float _s4 = _s3 + PI_DIV_4;

		if ( ang_diff <= _s1 )
		{
			id = 2;
		}
		else if ( ang_diff > _s1 && ang_diff <= _s2 )
		{
			id = (bUp)?5:7;
		}
		else if ( ang_diff > _s2 && ang_diff <= _s3 )
		{
			id = (bUp)?3:1;
		}
		else if ( ang_diff > _s3 && ang_diff <= _s4 )
		{
			id = (bUp)?4:6;
		}
		else if( ang_diff > _s4 )
		{
			id = 0;
		}
		else
		{
			VERIFY(0);
		}

		string64 sect_name;
		xr_sprintf( sect_name, "effector_fire_hit_%d", id );
		AddEffector( this, effFireHit, sect_name, P * 0.001f );

	}//if hit_type
}

void CActor::HitSignal(float perc, Fvector& vLocalDir, CObject* who, s16 element)
{
	if (g_Alive()) 
	{

		// check damage bone
		Fvector D;
		XFORM().transform_dir(D,vLocalDir);

		float	yaw, pitch;
		D.getHP(yaw,pitch);
		IRenderVisual *pV = Visual();
		IKinematicsAnimated *tpKinematics = smart_cast<IKinematicsAnimated*>(pV);
		IKinematics *pK = smart_cast<IKinematics*>(pV);
		VERIFY(tpKinematics);
#pragma todo("Dima to Dima : forward-back bone impulse direction has been determined incorrectly!")
		MotionID motion_ID = m_anims->m_normal.m_damage[iFloor(pK->LL_GetBoneInstance(element).get_param(1) + (angle_difference(r_model_yaw + r_model_yaw_delta,yaw) <= PI_DIV_2 ? 0 : 1))];
		float power_factor = perc/100.f; clamp(power_factor,0.f,1.f);
		VERIFY(motion_ID.valid());
		tpKinematics->PlayFX(motion_ID,power_factor);
	}
}
void start_tutorial(LPCSTR name);
void CActor::Die	(CObject* who)
{
#ifdef DEBUG
	Msg("--- Actor [%s] dies !", this->Name());
#endif // #ifdef DEBUG
	inherited::Die		(who);

	if (OnServer())
	{	
		u16 I = inventory().FirstSlot();
		u16 E = inventory().LastSlot();

		for (; I <= E; ++I)
		{
			PIItem item_in_slot = inventory().ItemFromSlot(I);
			if (I == inventory().GetActiveSlot()) 
			{
				if(item_in_slot)
				{
					if (IsGameTypeSingle())
					{
						CGrenade* grenade = smart_cast<CGrenade*>(item_in_slot);
						if (grenade)
							grenade->DropGrenade();
						else
							item_in_slot->SetDropManual(TRUE);
					}else
					{
						//This logic we do on a server site
						/*
						if ((*I).m_pIItem->object().CLS_ID != CLSID_OBJECT_W_KNIFE)
						{
							(*I).m_pIItem->SetDropManual(TRUE);
						}*/							
					}
				};
			continue;
			}
			else
			{
				CCustomOutfit *pOutfit = smart_cast<CCustomOutfit *> (item_in_slot);
				if (pOutfit) continue;
			};
			if(item_in_slot) 
				inventory().Ruck(item_in_slot);
		};


		///!!! чистка пояса
		TIItemContainer &l_blist = inventory().m_belt;
		while (!l_blist.empty())	
			inventory().Ruck(l_blist.front());

		if (!IsGameTypeSingle())
		{
			//if we are on server and actor has PDA - destroy PDA
			TIItemContainer &l_rlist	= inventory().m_ruck;
			for(TIItemContainer::iterator l_it = l_rlist.begin(); l_rlist.end() != l_it; ++l_it)
			{
				if (GameID() == eGameIDArtefactHunt)
				{
					CArtefact* pArtefact = smart_cast<CArtefact*> (*l_it);
					if (pArtefact)
					{
						(*l_it)->SetDropManual(TRUE);
						continue;
					};
				};

				if ((*l_it)->object().CLS_ID == CLSID_OBJECT_PLAYERS_BAG)
				{
					(*l_it)->SetDropManual(TRUE);
					continue;
				};
			};
		};
	};

	if(!g_dedicated_server)
	{
		::Sound->play_at_pos	(sndDie[Random.randI(SND_DIE_COUNT)],this,Position());

		m_HeavyBreathSnd.stop	();
		m_BloodSnd.stop			();		
		m_DangerSnd.stop		();		
	}

	if	(IsGameTypeSingle())
	{
		cam_Set				(eacFreeLook);
		CurrentGameUI()->HideShownDialogs();
		start_tutorial		("game_over");
	} else
	{
		cam_Set				(eacFixedLookAt);
	}
	
	mstate_wishful	&=		~mcAnyMove;
	mstate_real		&=		~mcAnyMove;

	xr_delete				(m_sndShockEffector);
}

void	CActor::SwitchOutBorder(bool new_border_state)
{
	if(new_border_state)
	{
		callback(GameObject::eExitLevelBorder)(lua_game_object());
	}
	else 
	{
//.		Msg("enter level border");
		callback(GameObject::eEnterLevelBorder)(lua_game_object());
	}
	m_bOutBorder=new_border_state;
}

void CActor::g_Physics			(Fvector& _accel, float jump, float dt)
{
	// Correct accel
	Fvector		accel;
	accel.set					(_accel);
	m_hit_slowmo				-=	dt;
	if(m_hit_slowmo<0)			m_hit_slowmo = 0.f;

	accel.mul					(1.f-m_hit_slowmo);

	
	

	if(g_Alive())
	{
		if(mstate_real&mcClimb&&!cameras[eacFirstEye]->bClampYaw)
				accel.set(0.f,0.f,0.f);
		character_physics_support()->movement()->Calculate			(accel,cameras[cam_active]->vDirection,0,jump,dt,false);
		bool new_border_state=character_physics_support()->movement()->isOutBorder();
		if(m_bOutBorder!=new_border_state && Level().CurrentControlEntity() == this)
		{
			SwitchOutBorder(new_border_state);
		}
#ifndef MASTER_GOLD
		if (psActorFlags.test(AF_NO_CLIP))
		{
			character_physics_support()->movement()->SetNonInteractive(true);
			character_physics_support()->movement()->SetVelocity(Fvector().set(0.f,0.f,0.f));
		}
		else if(character_physics_support()->movement()->bNonInteractiveMode)
				character_physics_support()->movement()->SetNonInteractive(false);
		else
			character_physics_support()->movement()->GetPosition		(Position());
#else //DEBUG
		character_physics_support()->movement()->GetPosition		(Position());
#endif //DEBUG
		character_physics_support()->movement()->bSleep				=false;
	}

	if (Local() && g_Alive()) 
	{
		bool isGuns = EngineExternal()[EEngineExternalGunslinger::EnableGunslingerMode];
		if (!isGuns && character_physics_support()->movement()->gcontact_Was)
			Cameras().AddCamEffector(new CEffectorFall (character_physics_support()->movement()->gcontact_Power));

		if (!fis_zero(character_physics_support()->movement()->gcontact_HealthLost))	
		{
			VERIFY( character_physics_support() );
			VERIFY( character_physics_support()->movement() );
			ICollisionDamageInfo* di=character_physics_support()->movement()->CollisionDamageInfo();
			VERIFY( di );
			bool b_hit_initiated =  di->GetAndResetInitiated();
			Fvector hdir;di->HitDir(hdir);
			SetHitInfo(this, nullptr, 0, Fvector().set(0, 0, 0), hdir);
			//				Hit	(m_PhysicMovementControl->gcontact_HealthLost,hdir,di->DamageInitiator(),m_PhysicMovementControl->ContactBone(),di->HitPos(),0.f,ALife::eHitTypeStrike);//s16(6 + 2*::Random.randI(0,2))
			if (Level().CurrentControlEntity() == this)
			{
				
				SHit HDS = SHit(character_physics_support()->movement()->gcontact_HealthLost,
//.								0.0f,
								hdir,
								di->DamageInitiator(),
								character_physics_support()->movement()->ContactBone(),
								di->HitPos(),
								0.f,
								di->HitType(),
								0.0f, 
								b_hit_initiated);
//				Hit(&HDS);

				NET_Packet	l_P;
				HDS.GenHeader(GE_HIT, ID());
				HDS.whoID = di->DamageInitiator()->ID();
				HDS.weaponID = di->DamageInitiator()->ID();
				HDS.Write_Packet(l_P);

				u_EventSend	(l_P);
			}
		}
	}
}
float g_fov = 67.5f;

float CActor::currentFOV()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT|HUD_WEAPON_RT2))
		return g_fov;

	CWeapon* pWeapon = smart_cast<CWeapon*>(inventory().ActiveItem());	

	if (eacFreeLook != cam_active && pWeapon && pWeapon->IsZoomed() && (!pWeapon->ZoomTexture() || (!pWeapon->IsRotatingToZoom() && pWeapon->ZoomTexture())))
	{
		bool isGuns = EngineExternal()[EEngineExternalGunslinger::EnableGunslingerMode];
		if (isGuns)
		{
			float fov = (g_fov / 2.f) * PI / 180.f;
			return (2.f * atan(tan(fov) / pWeapon->GetZoomFactor()) * 180.f / PI);
		}
		else
			return pWeapon->GetZoomFactor() * (0.75f);
	}
	else
		return g_fov;
}
float	NET_Jump = 0;
static bool bLook_cam_fp_zoom = false;
extern ENGINE_API int m_look_cam_fp_zoom;
void CActor::UpdateCL	()
{
	PROF_EVENT("CActor UpdateCL");
	if (EngineExternal()[EEngineExternalGunslinger::EnableGunslingerMode])
	{
		u32 ct = Device.dwTimeGlobal;
		u32 dt = Device.GetTimeDeltaSafe(_last_update_time, ct);
		_last_update_time = ct;

		ProcessKeys();
		UpdateSuicide(dt);
		UpdateFOV();

		if (g_player_hud != nullptr)
			g_player_hud->UpdateWeaponOffset(dt);

		if (_jitter_time_remains > dt)
			_jitter_time_remains = _jitter_time_remains - dt;
		else
			_jitter_time_remains = 0;

		if (GetCurrentControllerInputCorrectionParams().active)
		{
			controller_mouse_control_params contr_k = GetControllerMouseControlParams();

			if (GetMovementState(eReal) & mcLStrafe)
				IR_OnMouseMove(static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.min_offset)), static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.max_offset)));
			else if (GetMovementState(eReal) & mcRStrafe)
				IR_OnMouseMove(static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.max_offset)), static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.max_offset)));
			else if (GetMovementState(eReal) & mcFwd)
				IR_OnMouseMove(static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.min_offset)), static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.max_offset)));
			else if (GetMovementState(eReal) & mcBack)
				IR_OnMouseMove(static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.min_offset)), static_cast<int>(floor(contr_k.keyboard_move_k * contr_k.min_offset)));
		}
	}

	if(g_Alive() && Level().CurrentViewEntity() == this)
	{
		if(CurrentGameUI() && nullptr==CurrentGameUI()->TopInputReceiver())
		{
			int dik = get_action_dik(kUSE, 0);
			if(dik && pInput->iGetAsyncKeyState(dik))
				pPickup->SetPickupMode(true);
			
			dik = get_action_dik(kUSE, 1);
			if(dik && pInput->iGetAsyncKeyState(dik))
				pPickup->SetPickupMode(true);
		}
	}

	UpdateInventoryOwner			(Device.dwTimeDelta);

	if(m_feel_touch_characters>0)
	{
		for(xr_vector<CObject*>::iterator it = feel_touch.begin(); it != feel_touch.end(); it++)
		{
			CPhysicsShellHolder	*sh = smart_cast<CPhysicsShellHolder*>(*it);
			if(sh&&sh->character_physics_support())
			{
				sh->character_physics_support()->movement()->UpdateObjectBox(character_physics_support()->movement()->PHCharacter());
			}
		}
	}
	if (m_holder)
	{
		m_holder->UpdateEx(currentFOV());
	}
	else
	{
		UpdatePlayerView();
	}
	m_snd_noise -= 0.3f*Device.fTimeDelta;

	inherited::UpdateCL				();
	m_pPhysics_support->in_UpdateCL	();


	if (g_Alive()) 
		PickupModeUpdate	();	

	PickupModeUpdate_COD();

	SetZoomAimingMode		(false);
	CWeapon* pWeapon		= smart_cast<CWeapon*>(inventory().ActiveItem());	

	cam_Update(float(Device.dwTimeDelta)/1000.0f, currentFOV());

	if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
	{
		psHUD_Flags.set( HUD_CROSSHAIR_RT2, true );
		psHUD_Flags.set( HUD_DRAW_RT, true );
	}

	CMissile* pMissile = smart_cast<CMissile*>(inventory().ActiveItem());
	CHUDAnimItem* pAnimator = smart_cast<CHUDAnimItem*>(inventory().ActiveItem());

	bBlockSprint = pWeapon != nullptr && pWeapon->NeedBlockSprint() || pMissile != nullptr && pMissile->NeedBlockSprint() || pAnimator != nullptr;

	if (pWeapon)
	{
		if(pWeapon->IsZoomed())
		{
			float full_fire_disp = pWeapon->GetFireDispersion(true);

			CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(Cameras().GetCamEffector(eCEZoom));
			if(S) 
				S->SetParams(full_fire_disp);

			SetZoomAimingMode		(true);
			// Force switch to first-person for zooming
			if (m_look_cam_fp_zoom == 1 && !bLook_cam_fp_zoom && cam_active == eacLookAt) {
				cam_Set(eacFirstEye);
				bLook_cam_fp_zoom = true;
			}
		}
		else {
			// Switch back to third-person if was forced
			if (bLook_cam_fp_zoom && cam_active == eacFirstEye) {
				cam_Set(eacLookAt);
				bLook_cam_fp_zoom = false;
			}
		}

		if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
		{
			float fire_disp_full = pWeapon->GetFireDispersion(true, true);
			m_fdisp_controller.SetDispertion(fire_disp_full);
			
			fire_disp_full = m_fdisp_controller.GetCurrentDispertion();

			HUD().SetCrosshairDisp(fire_disp_full, 0.02f);
			HUD().ShowCrosshair(pWeapon->use_crosshair());
#ifdef DEBUG
			HUD().SetFirstBulletCrosshairDisp(pWeapon->GetFirstBulletDisp());
#endif
			
			BOOL B = ! ((mstate_real & mcLookout) && !IsGameTypeSingleCompatible());

			psHUD_Flags.set( HUD_WEAPON_RT, B );
			B = B && pWeapon->show_crosshair();

			psHUD_Flags.set( HUD_CROSSHAIR_RT2, B );
			psHUD_Flags.set( HUD_DRAW_RT,		pWeapon->show_indicators() );
		}

	}
	else
	{
		if(Level().CurrentEntity() && this->ID()==Level().CurrentEntity()->ID() )
		{
			HUD().SetCrosshairDisp(0.f);
			HUD().ShowCrosshair(false);

			// Switch back to third-person if was forced
			if (bLook_cam_fp_zoom && cam_active == eacFirstEye) {
				cam_Set(eacLookAt);
				bLook_cam_fp_zoom = false;
			}
		}
	}

	UpdateDefferedMessages();

	if (g_Alive()) 
		CStepManager::update(this==Level().CurrentViewEntity());

	spatial.type |=STYPE_REACTTOSOUND;

	if(m_sndShockEffector)
	{
		if (this == Level().CurrentViewEntity())
		{
			m_sndShockEffector->Update();

			if(!m_sndShockEffector->InWork() || !g_Alive())
				xr_delete(m_sndShockEffector);
		}
		else
			xr_delete(m_sndShockEffector);
	}
	Fmatrix							trans;
	Cameras().hud_camera_Matrix(trans);

	if(IsFocused())
		g_player_hud->update			(trans);

	pPickup->SetPickupMode(false);
}

void CActor::UpdatePlayerView()
{
	//если в режиме HUD, то сама модель актера не рисуется
	BOOL has_visible = 1;
	BOOL has_shadow_only = 0;
	if (character_physics_support()->IsRemoved())
		has_visible = 0;
	if (HUDview())
	{
		has_visible = 0;
		has_shadow_only = psGameFlags.test(rsActorShadow) && Render->get_generation() != IRender_interface::GENERATION_R1;
	}
	else if (IsGameTypeSingle())
	{
		Fvector cent;
		Center(cent);
		CWeapon* pWeapon = smart_cast<CWeapon*>(inventory().ActiveItem());
		CCameraLook* pCam = smart_cast<CCameraLook*>(cam_Active());
		has_visible = pCam && pCam->GetDist() >= 0.43f && (!pWeapon || !pWeapon->render_item_ui_query());
		has_shadow_only = psGameFlags.test(rsActorShadow) && Render->get_generation() != IRender_interface::GENERATION_R1;
	}
	setVisible(has_visible, has_shadow_only);

	if (IsFocused())
	{
		BOOL bHudView = HUDview();
		if (bHudView)
		{
			CInventoryItem* pInvItem = inventory().ActiveItem();
			if (pInvItem)
			{
				CHudItem* pHudItem = smart_cast<CHudItem*>(pInvItem);
				if (pHudItem)
				{
					if (pHudItem->IsHidden())
					{
						g_player_hud->detach_item(pHudItem);
					}
					else
					{
						g_player_hud->attach_item(pHudItem);
					}
				}
			}
			else
			{
				g_player_hud->detach_item_idx(0);
			}

			CHudItem* pDetector = smart_cast<CHudItem*>(inventory().ItemFromSlot(DETECTOR_SLOT));
			if (pDetector)
			{
				if (pDetector->IsHidden())
				{
					g_player_hud->detach_item(pDetector->cast_hud_item());
				}
				else if (!g_player_hud->attached_item(1))
				{
					g_player_hud->attach_item(pDetector->cast_hud_item());
				}
			}
		}
		else
		{
			g_player_hud->detach_all_items();
		}
	}

	float dt = Device.fTimeDelta;

	// Check controls, create accel, prelimitary setup "mstate_real"
	//----------- for E3 -----------------------------
	if (Level().CurrentControlEntity() == this && !Level().IsDemoPlay())
		//------------------------------------------------
	{
		g_cl_CheckControls(mstate_wishful, NET_SavedAccel, NET_Jump, dt);
		g_cl_Orientate(mstate_real, dt);
		g_Orientate(mstate_real, dt);

		g_Physics(NET_SavedAccel, NET_Jump, dt);

		g_cl_ValidateMState(dt, mstate_wishful);
		g_SetAnimation(mstate_real);

		// Check for game-contacts
		Fvector C;
		Center(C);
		float R = Radius();

		feel_touch_update(C, R);
		Feel_Grenade_Update(m_fFeelGrenadeRadius);

		// Dropping
		if (b_DropActivated) 
		{
			f_DropPower += dt * 0.1f;
			clamp(f_DropPower, 0.f, 1.f);
		}
		else 
		{
			f_DropPower = 0.f;
		}
		if (!Level().IsDemoPlay())
		{
			mstate_wishful &= ~mcAccel;
			mstate_wishful &= ~mcLStrafe;
			mstate_wishful &= ~mcRStrafe;
			mstate_wishful &= ~mcLLookout;
			mstate_wishful &= ~mcRLookout;
			mstate_wishful &= ~mcFwd;
			mstate_wishful &= ~mcBack;
			if (!psActorFlags.test(AF_CROUCH_TOGGLE))
				mstate_wishful &= ~mcCrouch;
		}
	}
	else
	{
		make_Interpolation();

		if (NET.size())
		{
			g_sv_Orientate(mstate_real, dt);
			g_Orientate(mstate_real, dt);
			g_Physics(NET_SavedAccel, NET_Jump, dt);
			if (!m_bInInterpolation)
				g_cl_ValidateMState(dt, mstate_wishful);
			g_SetAnimation(mstate_real);

			set_state_box(NET_Last.mstate);


		}
		mstate_old = mstate_real;
	}

	NET_Jump = 0;
}

void CActor::set_state_box(u32	mstate)
{
		if ( mstate & mcCrouch)
	{
		if (isActorAccelerated(mstate_real, IsZoomAimingMode()))
			character_physics_support()->movement()->ActivateBox(1, true);
		else
			character_physics_support()->movement()->ActivateBox(2, true);
	}
	else 
		character_physics_support()->movement()->ActivateBox(0, true);
}

void CActor::shedule_Update	(u32 DT)
{
	PROF_EVENT("CActor shedule_Update");
	setSVU							(OnServer());
//.	UpdateInventoryOwner			(DT);

	if(m_holder || !getEnabled() || !Ready())
	{
		m_sDefaultObjAction				= nullptr;
		inherited::shedule_Update		(DT);
		return;
	}

	inherited::shedule_Update	(DT);

	//эффектор включаемый при ходьбе
	if (!pCamBobbing)
	{
		pCamBobbing = new CEffectorBobbing	();
		Cameras().AddCamEffector			(pCamBobbing);
	}
	pCamBobbing->SetState						(mstate_real, conditions().IsLimping(), IsZoomAimingMode());

	//звук тяжелого дыхания при уталости и хромании
	if(this==Level().CurrentControlEntity() && !g_dedicated_server )
	{
		if(conditions().IsLimping() && g_Alive() && !psActorFlags.test(AF_DISABLE_CONDITION_TEST)){
			if(!m_HeavyBreathSnd._feedback()){
				m_HeavyBreathSnd.play_at_pos(this, Fvector().set(0,ACTOR_HEIGHT,0), sm_Looped | sm_2D);
			}else{
				m_HeavyBreathSnd.set_position(Fvector().set(0,ACTOR_HEIGHT,0));
			}
		}else if(m_HeavyBreathSnd._feedback()){
			m_HeavyBreathSnd.stop		();
		}

		// -------------------------------
		float bs = conditions().BleedingSpeed();
		if(bs>0.6f)
		{
			Fvector snd_pos;
			snd_pos.set(0,ACTOR_HEIGHT,0);
			if(!m_BloodSnd._feedback())
				m_BloodSnd.play_at_pos(this, snd_pos, sm_Looped | sm_2D);
			else
				m_BloodSnd.set_position(snd_pos);

			float v = bs+0.25f;

			m_BloodSnd.set_volume	(v);
		}else{
			if(m_BloodSnd._feedback())
				m_BloodSnd.stop();
		}

		if(!g_Alive()&&m_BloodSnd._feedback())
				m_BloodSnd.stop();
		// -------------------------------
		bs = conditions().GetZoneDanger();
		if ( bs > 0.1f )
		{
			Fvector snd_pos;
			snd_pos.set(0,ACTOR_HEIGHT,0);
			if(!m_DangerSnd._feedback())
				m_DangerSnd.play_at_pos(this, snd_pos, sm_Looped | sm_2D);
			else
				m_DangerSnd.set_position(snd_pos);

			float v = bs+0.25f;
//			Msg( "bs            = %.2f", bs );

			m_DangerSnd.set_volume	(v);
		}
		else
		{
			if(m_DangerSnd._feedback())
				m_DangerSnd.stop();
		}

		if(!g_Alive()&&m_DangerSnd._feedback())
			m_DangerSnd.stop();
	}
	
	//что актер видит перед собой
	collide::rq_result& RQ				= HUD().GetCurrentRayQuery();
	
	Fvector ActorPos, PickPos = { 0.0f, 0.0f, 0.0f };
	Center(ActorPos);
	PickPos.mad(Device.vCameraPosition, Device.vCameraDirection, RQ.range);
	const static bool isMonstersInventory = EngineExternal()[EEngineExternalGame::EnableMonstersInventory];

	if (!input_external_handler_installed() && RQ.O && RQ.O->getVisible() && ActorPos.distance_to_sqr(PickPos) < 6.0f)
	{
		m_pObjectWeLookingAt			= smart_cast<CGameObject*>(RQ.O);
		
		CGameObject						*game_object = smart_cast<CGameObject*>(RQ.O);
		m_pUsableObject					= smart_cast<CUsableScriptObject*>(game_object);
		m_pInvBoxWeLookingAt			= smart_cast<CInventoryBox*>(game_object);
		m_pPersonWeLookingAt			= smart_cast<CInventoryOwner*>(game_object);
		m_pVehicleWeLookingAt			= smart_cast<CHolderCustom*>(game_object);
		CEntityAlive* pEntityAlive		= smart_cast<CEntityAlive*>(game_object);
		
		if (m_pVehicleWeLookingAt != nullptr)
		{
			m_pPersonWeLookingAt = nullptr;
		}

		if (IsGameTypeSingle())
		{
			if (m_pUsableObject && m_pUsableObject->tip_text())
			{
				m_sDefaultObjAction = g_pStringTable->translate( m_pUsableObject->tip_text() );
			}
			else
			{
				if (m_pPersonWeLookingAt && pEntityAlive->g_Alive() && m_pPersonWeLookingAt->IsTalkEnabled())
				{
					m_sDefaultObjAction = m_sCharacterUseAction;
				}
				else if ( pEntityAlive && !pEntityAlive->g_Alive() )
				{
					if ( m_pPersonWeLookingAt && m_pPersonWeLookingAt->deadbody_closed_status() )
					{
						m_sDefaultObjAction = m_sDeadCharacterDontUseAction;
					}
					else
					{
						if (CBaseMonster* pMonster = smart_cast<CBaseMonster*>(m_pPersonWeLookingAt))
						{
							if (isMonstersInventory)
							{
								m_sDefaultObjAction = m_sDeadCharacterUseAction;
							}
							else
							{
								m_pPersonWeLookingAt = nullptr;
							}
						}
						else
						{
							bool b_allow_drag = !!pSettings->line_exist("ph_capture_visuals", pEntityAlive->cNameVisual());
							if (b_allow_drag)
							{
								m_sDefaultObjAction = m_sDeadCharacterUseOrDragAction;
							}
							else if (pEntityAlive->cast_inventory_owner())
							{
								m_sDefaultObjAction = m_sDeadCharacterUseAction;
							}
						}
					}
				}
				else if (m_pVehicleWeLookingAt)
				{
					m_sDefaultObjAction = m_sCarCharacterUseAction;

					if (CCar* pCar = smart_cast<CCar*>(m_pVehicleWeLookingAt))
					{
						if (pCar->TryTrunk())
						{
							m_sDefaultObjAction = m_sCarTrunk;
						}
						else if (pCar->TryUsableBones())
						{
							m_sDefaultObjAction = m_sCarUse;
						}
					}
				}
				else if (	m_pObjectWeLookingAt && 
							m_pObjectWeLookingAt->cast_inventory_item() && 
							m_pObjectWeLookingAt->cast_inventory_item()->CanTake() )
				{
					m_sDefaultObjAction = m_sInventoryItemUseAction;
				}
				else 
				{
					m_sDefaultObjAction = nullptr;
				}
			}
		}
	}
	else 
	{
		m_pPersonWeLookingAt	= nullptr;
		m_sDefaultObjAction		= nullptr;
		m_pUsableObject			= nullptr;
		m_pObjectWeLookingAt	= nullptr;
		m_pVehicleWeLookingAt	= nullptr;
		m_pInvBoxWeLookingAt	= nullptr;
	}

//	UpdateSleep									();

	//для свойст артефактов, находящихся на поясе
	UpdateArtefactsOnBeltAndOutfit				();
	m_pPhysics_support->in_shedule_Update		(DT);
	Check_for_AutoPickUp						();
};
#include "debug_renderer.h"
void CActor::renderable_Render	()
{
	VERIFY(_valid(XFORM()));
	inherited::renderable_Render			();
	if(1/*!HUDview()*/)
	{
		CInventoryOwner::renderable_Render	();
	}
	VERIFY(_valid(XFORM()));
}

BOOL CActor::renderable_ShadowGenerate	() 
{
	if(m_holder)
		return FALSE;
	
	return inherited::renderable_ShadowGenerate();
}



void CActor::g_PerformDrop	( )
{
	b_DropActivated			= FALSE;

	PIItem pItem			= inventory().ActiveItem();
	if (0==pItem)			return;

	if(pItem->IsQuestItem()) return;

	u16 s					= inventory().GetActiveSlot();
	if(inventory().SlotIsPersistent(s))	return;

	pItem->SetDropManual	(TRUE);
}

bool CActor::use_default_throw_force()
{
	if (!g_Alive())
		return false;
	
	return true;
}

float CActor::missile_throw_force()
{
	return 0.f;
}

#ifdef DEBUG
extern	BOOL	g_ShowAnimationInfo		;
#endif // DEBUG
// HUD

void CActor::OnHUDDraw	(CCustomHUD* Z)
{
	R_ASSERT(IsFocused());

	if (!((mstate_real & mcLookout) && !IsGameTypeSingleCompatible()))
		g_player_hud->render_hud();

	if(auto pGameObject = smart_cast<CGameObject*>(Holder())) {
		pGameObject->OnHUDDraw(Z);
	}
#if 0//ndef NDEBUG
	if (Level().CurrentControlEntity() == this && g_ShowAnimationInfo)
	{
		string128 buf;
		UI().Font().pFontStat->SetColor	(0xffffffff);
		UI().Font().pFontStat->OutSet		(170,530);
		UI().Font().pFontStat->OutNext	("Position:      [%3.2f, %3.2f, %3.2f]",VPUSH(Position()));
		UI().Font().pFontStat->OutNext	("Velocity:      [%3.2f, %3.2f, %3.2f]",VPUSH(m_PhysicMovementControl->GetVelocity()));
		UI().Font().pFontStat->OutNext	("Vel Magnitude: [%3.2f]",m_PhysicMovementControl->GetVelocityMagnitude());
		UI().Font().pFontStat->OutNext	("Vel Actual:    [%3.2f]",m_PhysicMovementControl->GetVelocityActual());
		switch (m_PhysicMovementControl->Environment())
		{
		case CPHMovementControl::peOnGround:	xr_strcpy(buf,"ground");			break;
		case CPHMovementControl::peInAir:		xr_strcpy(buf,"air");				break;
		case CPHMovementControl::peAtWall:		xr_strcpy(buf,"wall");				break;
		}
		UI().Font().pFontStat->OutNext	(buf);

		if (IReceived != 0)
		{
			float Size = 0;
			Size = UI().Font().pFontStat->GetSize();
			UI().Font().pFontStat->SetSize(Size*2);
			UI().Font().pFontStat->SetColor	(0xffff0000);
			UI().Font().pFontStat->OutNext ("Input :		[%3.2f]", ICoincidenced/IReceived * 100.0f);
			UI().Font().pFontStat->SetSize(Size);
		};
	};
#endif
}

void CActor::RenderIndicator			(Fvector dpos, float r1, float r2, const ui_shader &IndShader)
{
	if (!g_Alive()) return;


	UIRender->StartPrimitive(4, IUIRender::ptTriStrip, IUIRender::pttLIT);

	CBoneInstance& BI = smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(u16(m_head));
	Fmatrix M;
	smart_cast<IKinematics*>(Visual())->CalculateBones	();
	M.mul						(XFORM(),BI.mTransform);

	Fvector pos = M.c; pos.add(dpos);
	const Fvector& T        = Device.vCameraTop;
	const Fvector& R        = Device.vCameraRight;
	Fvector Vr, Vt;
	Vr.x            = R.x*r1;
	Vr.y            = R.y*r1;
	Vr.z            = R.z*r1;
	Vt.x            = T.x*r2;
	Vt.y            = T.y*r2;
	Vt.z            = T.z*r2;

	Fvector         a,b,c,d;
	a.sub           (Vt,Vr);
	b.add           (Vt,Vr);
	c.invert        (a);
	d.invert        (b);

	UIRender->PushPoint(d.x+pos.x,d.y+pos.y,d.z+pos.z, 0xffffffff, 0.f,1.f);
	UIRender->PushPoint(a.x+pos.x,a.y+pos.y,a.z+pos.z, 0xffffffff, 0.f,0.f);
	UIRender->PushPoint(c.x+pos.x,c.y+pos.y,c.z+pos.z, 0xffffffff, 1.f,1.f);
	UIRender->PushPoint(b.x+pos.x,b.y+pos.y,b.z+pos.z, 0xffffffff, 1.f,0.f);
	//pv->set         (d.x+pos.x,d.y+pos.y,d.z+pos.z, 0xffffffff, 0.f,1.f);        pv++;
	//pv->set         (a.x+pos.x,a.y+pos.y,a.z+pos.z, 0xffffffff, 0.f,0.f);        pv++;
	//pv->set         (c.x+pos.x,c.y+pos.y,c.z+pos.z, 0xffffffff, 1.f,1.f);        pv++;
	//pv->set         (b.x+pos.x,b.y+pos.y,b.z+pos.z, 0xffffffff, 1.f,0.f);        pv++;
	// render	
	//dwCount 				= u32(pv-pv_start);
	//RCache.Vertex.Unlock	(dwCount,hFriendlyIndicator->vb_stride);

	UIRender->CacheSetXformWorld(Fidentity);
	//RCache.set_xform_world		(Fidentity);
	UIRender->SetShader(*IndShader);
	//RCache.set_Shader			(IndShader);
	//RCache.set_Geometry			(hFriendlyIndicator);
	//RCache.Render	   			(D3DPT_TRIANGLESTRIP,dwOffset,0, dwCount, 0, 2);
	UIRender->FlushPrimitive();
};

static float mid_size = 0.097f;
static float fontsize = 15.0f;
static float upsize	= 0.33f;
void CActor::RenderText				(LPCSTR Text, Fvector dpos, float* pdup, u32 color)
{
	if (!g_Alive()) return;
	
	CBoneInstance& BI = smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(u16(m_head));
	Fmatrix M;
	smart_cast<IKinematics*>(Visual())->CalculateBones	();
	M.mul						(XFORM(),BI.mTransform);
	//------------------------------------------------
	Fvector v0, v1;
	v0.set(M.c); v1.set(M.c);
	Fvector T        = Device.vCameraTop;
	v1.add(T);

	Fvector v0r, v1r;
	Device.mFullTransform.transform(v0r,v0);
	Device.mFullTransform.transform(v1r,v1);
	float size = v1r.distance_to(v0r);
	CGameFont* pFont = UI().Font().GetFont(ARIAL14_FONT_NAME);
	if (!pFont) return;
//	float OldFontSize = pFont->GetHeight	();	
	float delta_up = 0.0f;
	if (size < mid_size) delta_up = upsize;
	else delta_up = upsize*(mid_size/size);
	dpos.y += delta_up;
	if (size > mid_size) size = mid_size;
//	float NewFontSize = size/mid_size * fontsize;
	//------------------------------------------------
	M.c.y += dpos.y;

	Fvector4 v_res;	
	Device.mFullTransform.transform(v_res,M.c);

	if (v_res.z < 0 || v_res.w < 0)	return;
	if (v_res.x < -1.f || v_res.x > 1.f || v_res.y<-1.f || v_res.y>1.f) return;

	float x = (1.f + v_res.x)/2.f * (Device.TargetWidth);
	float y = (1.f - v_res.y)/2.f * (Device.TargetHeight);

	pFont->SetAligment	(CGameFont::alCenter);
	pFont->SetColor		(color);
//	pFont->SetHeight	(NewFontSize);
	pFont->Out			(x,y,Text);
	//-------------------------------------------------
//	pFont->SetHeight(OldFontSize);
	*pdup = delta_up;
};

void CActor::SetPhPosition(const Fmatrix &transform)
{
	if(!m_pPhysicsShell){ 
		character_physics_support()->movement()->SetPosition(transform.c);
	}
	//else m_phSkeleton->S
}

void CActor::ForceTransform(const Fmatrix& m)
{
	//if( !g_Alive() )
	//			return;
	//VERIFY(_valid(m));
	//XFORM().set( m );
	//if( character_physics_support()->movement()->CharacterExist() )
	//		character_physics_support()->movement()->EnableCharacter();
	//character_physics_support()->set_movement_position( m.c );
	//character_physics_support()->movement()->SetVelocity( 0, 0, 0 );

	character_physics_support()->ForceTransform( m );
	const float block_damage_time_seconds = 2.f;
	if(!IsGameTypeSingle())
		character_physics_support()->movement()->BlockDamageSet( u64( block_damage_time_seconds/fixed_step ) );
}

ENGINE_API extern float		psHUD_FOV;
float CActor::Radius()const
{ 
	float R		= inherited::Radius();
	CWeapon* W	= smart_cast<CWeapon*>(inventory().ActiveItem());
	if (W) R	+= W->Radius();
	//	if (HUDview()) R *= 1.f/psHUD_FOV;
	return R;
}

bool		CActor::use_bolts				() const
{
	if (!IsGameTypeSingle()) return false;
	return CInventoryOwner::use_bolts();
};

int		g_iCorpseRemove = 1;

bool  CActor::NeedToDestroyObject() const
{
	if(IsGameTypeSingle())
	{
		return false;
	}
	else 
	{
		if (g_Alive()) return false;
		if (g_iCorpseRemove == -1) return false;
		if (g_iCorpseRemove == 0 && m_bAllowDeathRemove) return true;
		if(TimePassedAfterDeath()>m_dwBodyRemoveTime && m_bAllowDeathRemove)
			return true;
		else
			return false;
	}
}

ALife::_TIME_ID	 CActor::TimePassedAfterDeath()	const
{
	if(!g_Alive())
		return Level().timeServer() - GetLevelDeathTime();
	else
		return 0;
}


void CActor::OnItemTake(CInventoryItem *inventory_item)
{
	CInventoryOwner::OnItemTake(inventory_item);
	if (OnClient()) return;
}

void CActor::OnItemDrop(CInventoryItem *inventory_item, bool just_before_destroy)
{
	CInventoryOwner::OnItemDrop(inventory_item, just_before_destroy);

	CCustomOutfit* outfit		= smart_cast<CCustomOutfit*>(inventory_item);
	if(outfit && inventory_item->m_ItemCurrPlace.type==eItemPlaceSlot)
	{
		outfit->ApplySkinModel	(this, false, false);

		bool isGuns = EngineExternal()[EEngineExternalGunslinger::EnableGunslingerMode];
		if (isGuns && !outfit->bIsHelmetAvaliable)
		{
			CTorch* torch = smart_cast<CTorch*>(inventory().ItemFromSlot(TORCH_SLOT));
			if (torch && torch->IsSwitched())
				torch->Switch(false);
		}
	}

	CHelmet* helmet = smart_cast<CHelmet*>(inventory_item);
	if (helmet && inventory_item->m_ItemCurrPlace.type == eItemPlaceSlot)
	{
		CTorch* torch = smart_cast<CTorch*>(inventory().ItemFromSlot(TORCH_SLOT));
		if (torch && torch->GetNightVisionStatus())
			torch->SwitchNightVision(false);

		bool isGuns = EngineExternal()[EEngineExternalGunslinger::EnableGunslingerMode];
		if (isGuns)
		{
			if (torch && torch->IsSwitched())
				torch->Switch(false);
		}
	}

	CWeapon* weapon	= smart_cast<CWeapon*>(inventory_item);
	if(weapon && inventory_item->m_ItemCurrPlace.type==eItemPlaceSlot)
	{
		weapon->OnZoomOut();
		if(weapon->GetRememberActorNVisnStatus())
			weapon->EnableActorNVisnAfterZoom();
	}

	if(		!just_before_destroy && 
			inventory_item->BaseSlot()==GRENADE_SLOT && 
			nullptr==inventory().ItemFromSlot(GRENADE_SLOT) )
	{
		PIItem grenade =  inventory().SameSlot(GRENADE_SLOT, inventory_item, true);
		
		if(grenade)
			inventory().Slot(GRENADE_SLOT, grenade, true, true);
	}
}


void CActor::OnItemDropUpdate ()
{
	CInventoryOwner::OnItemDropUpdate		();

	TIItemContainer::iterator				I = inventory().m_all.begin();
	TIItemContainer::iterator				E = inventory().m_all.end();
	
	for ( ; I != E; ++I)
		if( !(*I)->IsInvalid() && !attached(*I))
			attach(*I);
}


void CActor::OnItemRuck		(CInventoryItem *inventory_item, const SInvItemPlace& previous_place)
{
	CInventoryOwner::OnItemRuck(inventory_item, previous_place);
}

void CActor::OnItemBelt		(CInventoryItem *inventory_item, const SInvItemPlace& previous_place)
{
	CInventoryOwner::OnItemBelt(inventory_item, previous_place);
}

#define ARTEFACTS_UPDATE_TIME 0.100f

void CActor::UpdateArtefactsOnBeltAndOutfit()
{
	static float update_time = 0;

	float f_update_time = 0;

	if(update_time<ARTEFACTS_UPDATE_TIME)
	{
		update_time += conditions().fdelta_time();
		return;
	}
	else
	{
		f_update_time	= update_time;
		update_time		= 0.0f;
	}

	for(TIItemContainer::iterator it = inventory().m_belt.begin(); 
		inventory().m_belt.end() != it; ++it) 
	{
		CArtefact*	artefact = smart_cast<CArtefact*>(*it);
		if(artefact)
		{
			conditions().ChangeBleeding		(artefact->m_fBleedingRestoreSpeed  * f_update_time);
			conditions().ChangeHealth		(artefact->m_fHealthRestoreSpeed    * f_update_time);
			conditions().ChangePower		(artefact->m_fPowerRestoreSpeed     * f_update_time);
			conditions().ChangeSatiety		(artefact->m_fSatietyRestoreSpeed   * f_update_time);
			if(artefact->m_fRadiationRestoreSpeed>0.0f) 
			{
				float val = artefact->m_fRadiationRestoreSpeed - conditions().GetBoostRadiationImmunity();
				clamp(val, 0.0f, val);
				conditions().ChangeRadiation(val * f_update_time);
			}
			else
				conditions().ChangeRadiation(artefact->m_fRadiationRestoreSpeed	* f_update_time);
		}
	}
	CCustomOutfit* outfit = GetOutfit();
	if ( outfit )
	{
		conditions().ChangeBleeding		(outfit->m_fBleedingRestoreSpeed  * f_update_time);
		conditions().ChangeHealth		(outfit->m_fHealthRestoreSpeed    * f_update_time);
		conditions().ChangePower		(outfit->m_fPowerRestoreSpeed     * f_update_time);
		conditions().ChangeSatiety		(outfit->m_fSatietyRestoreSpeed   * f_update_time);
		conditions().ChangeRadiation	(outfit->m_fRadiationRestoreSpeed * f_update_time);
	}
	else
	{
		CHelmet* pHelmet				= smart_cast<CHelmet*>(inventory().ItemFromSlot(HELMET_SLOT));
		if(!pHelmet)
		{
			CTorch* pTorch = smart_cast<CTorch*>( inventory().ItemFromSlot(TORCH_SLOT) );
			if ( pTorch && pTorch->GetNightVisionStatus() )
			{
				pTorch->SwitchNightVision(false);
			}
		}
	}
}

float	CActor::HitArtefactsOnBelt(float hit_power, ALife::EHitType hit_type)
{
	TIItemContainer::iterator it  = inventory().m_belt.begin(); 
	TIItemContainer::iterator ite = inventory().m_belt.end() ;
	for( ; it != ite; ++it )
	{
		CArtefact*	artefact = smart_cast<CArtefact*>(*it);
		if ( artefact )
		{
			hit_power -= artefact->m_ArtefactHitImmunities.AffectHit( 1.0f, hit_type );
		}
	}
	clamp(hit_power, 0.0f, flt_max);

	return hit_power;
}

float CActor::GetProtection_ArtefactsOnBelt( ALife::EHitType hit_type )
{
	float sum = 0.0f;
	TIItemContainer::iterator it  = inventory().m_belt.begin(); 
	TIItemContainer::iterator ite = inventory().m_belt.end() ;
	for( ; it != ite; ++it )
	{
		CArtefact*	artefact = smart_cast<CArtefact*>(*it);
		if ( artefact )
		{
			sum += artefact->m_ArtefactHitImmunities.AffectHit( 1.0f, hit_type );
		}
	}
	return sum;
}

void	CActor::SetZoomRndSeed		(s32 Seed)
{
	if (0 != Seed) m_ZoomRndSeed = Seed;
	else m_ZoomRndSeed = s32(Level().timeServer_Async());
};

void	CActor::SetShotRndSeed		(s32 Seed)
{
	if (0 != Seed) m_ShotRndSeed = Seed;
	else m_ShotRndSeed = s32(Level().timeServer_Async());
};

void CActor::spawn_supplies			()
{
	inherited::spawn_supplies		();
	CInventoryOwner::spawn_supplies	();

	if (!pGameGlobals->line_exist("actor_item", "anim_fake_item"))
	{
		Msg("! Animation slot not registered");
		Msg("! [anim_fake] section missing");
		return;
	}

	Level().spawn_item(pGameGlobals->r_string("actor_item", "anim_fake_item"), Position(), ai_location().level_vertex_id(), ID());
}


void CActor::AnimTorsoPlayCallBack(CBlend* B)
{
	CActor* actor		= (CActor*)B->CallbackParam;
	actor->m_bAnimTorsoPlayed = FALSE;
}


/*
void CActor::UpdateMotionIcon(u32 mstate_rl)
{
	CUIMotionIcon*	motion_icon=CurrentGameUI()->UIMainIngameWnd->MotionIcon();
	if(mstate_rl&mcClimb)
	{
		motion_icon->ShowState(CUIMotionIcon::stClimb);
	}
	else
	{
		if(mstate_rl&mcCrouch)
		{
			if (!isActorAccelerated(mstate_rl, IsZoomAimingMode()))
				motion_icon->ShowState(CUIMotionIcon::stCreep);
			else
				motion_icon->ShowState(CUIMotionIcon::stCrouch);
		}
		else
		if(mstate_rl&mcSprint)
				motion_icon->ShowState(CUIMotionIcon::stSprint);
		else
		if(mstate_rl&mcAnyMove && isActorAccelerated(mstate_rl, IsZoomAimingMode()))
			motion_icon->ShowState(CUIMotionIcon::stRun);
		else
			motion_icon->ShowState(CUIMotionIcon::stNormal);
	}
}
*/


CPHDestroyable*	CActor::ph_destroyable	()
{
	return smart_cast<CPHDestroyable*>(character_physics_support());
}

CEntityConditionSimple *CActor::create_entity_condition	(CEntityConditionSimple* ec)
{
	if(!ec)
		m_entity_condition		= new CActorCondition(this);
	else
		m_entity_condition		= smart_cast<CActorCondition*>(ec);
	
	return		(inherited::create_entity_condition(m_entity_condition));
}

DLL_Pure *CActor::_construct			()
{
	m_pPhysics_support				=	new CCharacterPhysicsSupport(CCharacterPhysicsSupport::etActor,this);
	CEntityAlive::_construct		();
	CInventoryOwner::_construct		();
	CStepManager::_construct		();
	
	return							(this);
}

bool CActor::use_center_to_aim			() const
{
	return							(!!(mstate_real&mcCrouch));
}

bool CActor::can_attach			(const CInventoryItem *inventory_item) const
{
	const CAttachableItem	*item = smart_cast<const CAttachableItem*>(inventory_item);
	if (!item || /*!item->enabled() ||*/ !item->can_be_attached())
		return			(false);

	//можно ли присоединять объекты такого типа
	if( m_attach_item_sections.end() == std::find(m_attach_item_sections.begin(),m_attach_item_sections.end(),inventory_item->object().cNameSect()) )
		return false;

	//если уже есть присоединненый объет такого типа 
	if(attached(inventory_item->object().cNameSect()))
		return false;

	return true;
}

void CActor::OnDifficultyChanged	()
{
	// immunities
	VERIFY(g_SingleGameDifficulty>=egdNovice && g_SingleGameDifficulty<=egdMaster); 
	LPCSTR diff_name				= get_token_name(difficulty_type_token, g_SingleGameDifficulty);
	string128						tmp;
	xr_strconcat(tmp,"actor_immunities_",diff_name);
	conditions().LoadImmunities		(tmp,pSettings);
	// hit probability
	xr_strconcat(tmp,"hit_probability_",diff_name);
	m_hit_probability				= pSettings->r_float(*cNameSect(),tmp);
	// two hits death parameters
	xr_strconcat(tmp,"actor_thd_",diff_name);
	conditions().LoadTwoHitsDeathParams(tmp);
}

CVisualMemoryManager	*CActor::visual_memory	() const
{
	return							(&memory().visual());
}

float		CActor::GetMass				()
{
	return g_Alive()?character_physics_support()->movement()->GetMass():m_pPhysicsShell?m_pPhysicsShell->getMass():0; 
}

bool CActor::is_on_ground()
{
	return (character_physics_support()->movement()->Environment() != CPHMovementControl::peInAir);
}


bool CActor::is_ai_obstacle				() const
{
	return							(false);//true);
}

float CActor::GetRestoreSpeed( ALife::EConditionRestoreType const& type )
{
	float res = 0.0f;
	switch ( type )
	{
	case ALife::eHealthRestoreSpeed:
	{
		res = conditions().change_v().m_fV_HealthRestore;
		res += conditions().V_SatietyHealth() * ( (conditions().GetSatiety() > 0.0f) ? 1.0f : -1.0f );

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fHealthRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fHealthRestoreSpeed;
		}
		break;
	}
	case ALife::eRadiationRestoreSpeed:
	{	
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fRadiationRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fRadiationRestoreSpeed;
		}
		break;
	}
	case ALife::eSatietyRestoreSpeed:
	{
		res = conditions().V_Satiety();

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fSatietyRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fSatietyRestoreSpeed;
		}
		break;
	}
	case ALife::ePowerRestoreSpeed:
	{
		res = conditions().GetSatietyPower();

		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fPowerRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fPowerRestoreSpeed;
			VERIFY(outfit->m_fPowerLoss!=0.0f);
			res /= outfit->m_fPowerLoss;
		}
		else
			res /= 0.5f;
		break;
	}
	case ALife::eBleedingRestoreSpeed:
	{
		res = conditions().change_v().m_fV_WoundIncarnation;
	
		TIItemContainer::iterator itb = inventory().m_belt.begin();
		TIItemContainer::iterator ite = inventory().m_belt.end();
		for( ; itb != ite; ++itb ) 
		{
			CArtefact*	artefact = smart_cast<CArtefact*>( *itb );
			if ( artefact )
			{
				res += artefact->m_fBleedingRestoreSpeed;
			}
		}
		CCustomOutfit* outfit = GetOutfit();
		if ( outfit )
		{
			res += outfit->m_fBleedingRestoreSpeed;
		}
		break;
	}
	}//switch

	return res;
}


void CActor::On_SetEntity()
{
	CCustomOutfit* pOutfit = GetOutfit();
	if( !pOutfit )
		g_player_hud->load_default();
	else
		pOutfit->ApplySkinModel(this, true, true);
}

bool CActor::unlimited_ammo()
{
	return !!psActorFlags.test(AF_UNLIMITEDAMMO);
}

CCustomDetector* CActor::GetDetector(bool in_slot)
{
	if (in_slot)
		return smart_cast<CCustomDetector*>(inventory().ItemFromSlot(DETECTOR_SLOT));
	else
	{
		if (g_player_hud != nullptr && g_player_hud->attached_item(1) != nullptr)
		{
			attachable_hud_item* i1 = g_player_hud->attached_item(1);
			return smart_cast<CCustomDetector*>(i1->m_parent_hud_item);
		}
	}

	return nullptr;
}

float CActor::DistToSelectedContr(CController* controller)
{
	Fvector3& a_pos = Position();
	Fvector3& c_pos = controller->Position();
	Fvector3 c_pos_cp = c_pos;
	c_pos_cp.sub(a_pos);

	return c_pos_cp.magnitude();
}

float CActor::DistToContr()
{
	float result = 1000.f;
	float dist = 0.0f;

	for (u32 i = 0; i < _active_controllers.size(); ++i)
	{
		dist = DistToSelectedContr(_active_controllers[i]);
		if (dist < result)
			result = dist;
	}

	return result;
}

CActor::controller_psiunblock_params CActor::GetControllerPsiUnblockProb()
{
	_controller_psiunblock_params.min_dist = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_psi_unblock_mindist", 7.0f);
	_controller_psiunblock_params.max_dist = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_psi_unblock_maxdist", 60.0f);
	_controller_psiunblock_params.min_dist_prob = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_psi_unblock_mindist_prob", 0.95f);
	_controller_psiunblock_params.max_dist_prob = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_psi_unblock_maxdist_prob", 0.1f);

	return _controller_psiunblock_params;
}

void CActor::UpdatePsiBlockFailedState(CController* monster_controller)
{
	float dist = DistToSelectedContr(monster_controller);
	controller_psiunblock_params params = GetControllerPsiUnblockProb();
	float prob = 0.f;

	if (dist <= params.min_dist)
		prob = params.min_dist_prob;
	else if (dist >= params.max_dist)
		prob = params.max_dist_prob;
	else
	{
		prob = 1.0f - (dist - params.min_dist) / (params.max_dist - params.min_dist);
		prob = prob * (params.min_dist_prob - params.max_dist_prob) + params.max_dist_prob;
	}

	float random = ::Random.randF(0.f, 1.f);

	_psi_block_failed = random < prob;
}

bool CActor::IsPsiBlocked() const
{
	for (auto& booster : Actor()->conditions().GetCurBoosterInfluences())
	{
		if (booster.second.m_type == eBoostTelepaticProtection)
			return booster.second.fBoostTime > 0.0f;
	}

	return false;
}

CActor::controller_mouse_control_params CActor::GetControllerMouseControlParams()
{
	_controller_mouse_control_params.min_sense_scale = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_mouse_sense_min", 0.1f);
	_controller_mouse_control_params.max_sense_scale = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_mouse_sense_max", 0.5f);
	_controller_mouse_control_params.min_offset = READ_IF_EXISTS(pSettings, r_s32, "gunslinger_base", "controller_mouse_offset_min", -5);
	_controller_mouse_control_params.max_offset = READ_IF_EXISTS(pSettings, r_s32, "gunslinger_base", "controller_mouse_offset_max", 5);
	_controller_mouse_control_params.keyboard_move_k = READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_mouse_keyboard_move_k", 3.0f);

	return _controller_mouse_control_params;
}

void CActor::ChangeInputRotateAngle()
{
	const float MIN_ANGLE = 70.0f / 180.0f * PI;
	const float MAX_ANGLE = 290.0f / 180.0f * PI;

	float min_sense = GetControllerMouseControlParams().min_sense_scale;
	float max_sense = GetControllerMouseControlParams().max_sense_scale;

	_input_correction.rotate_angle = ::Random.randF(0.0f, 1.0f) * (MAX_ANGLE - MIN_ANGLE) + MIN_ANGLE;
	_input_correction.sense_scaler_x = ::Random.randF(0.0f, 1.0f) * (max_sense - min_sense) + min_sense;
	_input_correction.sense_scaler_y = ::Random.randF(0.0f, 1.0f) * (max_sense - min_sense) + min_sense;
	_input_correction.reverse_axis_y = ::Random.randF(0.0f, 1.0f) < 0.5f;
}

CActor::controller_input_correction_params CActor::GetCurrentControllerInputCorrectionParams()
{
	CHudItem* itm = smart_cast<CHudItem*>(inventory().ActiveItem());
	bool suicide_anm = itm != nullptr && itm->IsSuicideAnimPlaying();

	controller_input_correction_params result;

	if ((IsActorPlanningSuicide() && !IsPsiBlocked() && g_SingleGameDifficulty >= egdMaster) || IsActorSuicideNow() || suicide_anm)
	{
		result = _input_correction;
		result.active = true;
	}
	else
	{
		result.rotate_angle = 0.f;
		result.sense_scaler_x = 1.f;
		result.sense_scaler_y = 1.f;
		result.reverse_axis_y = false;
	}

	return result;
}

CActor::controller_input_random_offset CActor::GetControllerInputRandomOffset()
{
	controller_input_random_offset result;

	CHudItem* itm = smart_cast<CHudItem*>(inventory().ActiveItem());
	bool suicide_anm = itm != nullptr && itm->IsSuicideAnimPlaying();

	float min_offset = GetControllerMouseControlParams().min_offset;
	float max_offset = GetControllerMouseControlParams().max_offset;

	if (IsActorSuicideNow() || suicide_anm || (!IsPsiBlocked() && (IsActorControlled() || IsActorPlanningSuicide() || IsControllerPreparing())))
	{
		result.offset_x = floor(::Random.randF(0.0f, 1.0f) * (max_offset - min_offset) + min_offset);
		result.offset_y = floor(::Random.randF(0.0f, 1.0f) * (max_offset - min_offset) + min_offset);
	}
	else
	{
		result.offset_x = 0;
		result.offset_y = 0;
	}

	return result;
}

bool CActor::IsControllerPreparing() const
{
	if (IsPsiBlocked() && !IsPsiBlockFailed())
		return false;
	else
		return Device.GetTimeDeltaSafe(_controller_preparing_starttime) < floor(READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controller_prepare_time", 3.f) * 1000.f) + 1000.f;
}

float CActor::GetCurrentSuicideWalkKoef() const
{
	if (IsActorControlled() || IsActorSuicideNow() || IsActorPlanningSuicide() || IsControllerPreparing())
		return READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controlled_actor_speed_koef", 1.0f);

	CHudItem* itm = smart_cast<CHudItem*>(inventory().ActiveItem());
	if (itm != nullptr && itm->IsSuicideAnimPlaying())
		return READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "controlled_actor_speed_koef", 1.0f);

	return 1.0f;
}

void CActor::AddActiveController(CController* monster_controller)
{
	/*if (_active_controllers.empty())
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("gunsl_controller.on_suicide_scheme_start", funct))
			funct("", monster_controller->ID());
	}*/

	_active_controllers.push_back(monster_controller);
	/*luabind::functor<void> funct;
	if (ai().script_engine().functor("gunsl_controller.on_suicide_selected_by_controller", funct))
		funct("", monster_controller->ID());*/
}

void CActor::ClearActiveControllers()
{
	/*if (!_active_controllers.empty())
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("gunsl_controller.on_suicide_scheme_finish", funct))
			funct("", 0);
	}*/

	_active_controllers.clear();
}

void CActor::ResetActorControl()
{
	ClearActiveControllers();
	_controlled_time_remains = 0;
	_suicide_now = false;
	_lastshot_done_time = 0;
	_planning_suicide = false;
	_death_action_started = false;
}

bool CActor::CanUseItemForSuicide(CHudItemObject* item)
{
	if (item == nullptr)
		return false;

	bool can_shoot = item->WpnCanShoot();
	bool is_knife = !!(smart_cast<CWeaponKnife*>(item) != nullptr);

	if (!is_knife && !can_shoot)
		return false;

	if (READ_IF_EXISTS(pSettings, r_bool, item->HudSection(), "prohibit_suicide", false))
		return false;

	if (can_shoot && static_cast<CWeapon*>(item)->IsGrenadeMode())
	{
		bool can_switch_gl = READ_IF_EXISTS(pSettings, r_bool, item->HudSection(), "controller_can_switch_gl", false); // контроллер выключит подствол
		bool can_shoot_gl = READ_IF_EXISTS(pSettings, r_bool, item->HudSection(), "controller_can_shoot_gl", false);   // контроллер выстрелит из подствола под ноги
		if (static_cast<CWeapon*>(item)->GetAmmoInGLCount() > 0)
			return can_switch_gl || can_shoot_gl;
		else if (static_cast<CWeapon*>(item)->GetAmmoInMagCount() > 0)
			return can_switch_gl;
		else
			return false;
	}
	else
	{
		if (can_shoot)
			return static_cast<CWeapon*>(item)->GetAmmoInMagCount() > 0 && !static_cast<CWeapon*>(item)->IsMisfire() && item->GetState() != CWeapon::eReload && item->GetState() != CWeapon::eUnjam;
		else
			return true;
	}
}

bool CActor::IsControllerSeeActor(CController* monster_controller)
{
	return monster_controller->EnemyMan.see_enemy_now(this);
}

void CActor::UpdateSuicide(u32 dt)
{
	if (GetfHealth() <= 0.f)
	{
		ResetActorControl();
		return;
	}

	CHudItemObject* wpn = smart_cast<CHudItemObject*>(inventory().ActiveItem());

	u32 last_contr_time = _controlled_time_remains;
	_controlled_time_remains = (_controlled_time_remains > dt) ? (_controlled_time_remains - dt) : 0;

	if (_controlled_time_remains == 0 && _lastshot_done_time == 0 && !_death_action_started)
	{
		if (_inventory_disabled_set)
		{
			set_inventory_disabled(false);
			_inventory_disabled_set = false;
		}

		if (last_contr_time > 0)
			SetHandsJitterTime(floor(READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "actor_shock_time", 10.0f) * 1000.0f));

		ResetActorControl();

	}
	else if (wpn != nullptr && smart_cast<CWeaponKnife*>(wpn) != nullptr)
	{
		if (_planning_suicide && (wpn->GetActualCurrentAnim() == "anm_prepare_suicide"))
			_suicide_now = true;
		else if (_suicide_now && (wpn->GetActualCurrentAnim() == "anm_selfkill"))
			_death_action_started = true;
	}
	else if (_lastshot_done_time > 0)
	{
		SetMovementState(eWishful, mcFwd, false);
		SetMovementState(eWishful, mcBack, false);
		SetMovementState(eWishful, mcLStrafe, false);
		SetMovementState(eWishful, mcRStrafe, false);

		if (wpn == nullptr || Device.GetTimeDeltaSafe(_lastshot_done_time, Device.dwTimeGlobal) > floor(1000.f * READ_IF_EXISTS(pSettings, r_float, wpn->HudSection(), "suicide_delay", 0.1f)))
		{
			KillEntity(ID());
			_lastshot_done_time = 0;
		}
	}

	if (_controlled_time_remains > 0)
	{
		if (GetDetector())
		{
			if (GetDetector()->GetState() != CCustomDetector::eHiding && GetDetector()->GetState() != CCustomDetector::eHidden)
				GetDetector()->SwitchState(CCustomDetector::eHiding);
		}

		if (wpn != nullptr && (wpn->WpnCanShoot() || smart_cast<CWeaponBinoculars*>(wpn) != nullptr) && static_cast<CWeapon*>(wpn)->IsZoomed())
			SetActorKeyRepeatFlag(kfUNZOOM, true, true);

		if (wpn != nullptr && wpn->WpnCanShoot())
		{
			// Если стрельба "залипла", не допускаем исчерпания боезапаса (не ножом же резаться :) )
			if (wpn->GetState() == CWeapon::eFire && _lastshot_done_time == 0 && static_cast<CWeapon*>(wpn)->GetAmmoElapsed() <= 3)
				static_cast<CWeapon*>(wpn)->SetWorkingState(false);
		}

		/*if (wpn != nullptr && GetSection(wpn) == GetPDAShowAnimator())
		{
			if (IsPDAWindowVisible())
				HidePDAMenu();
		}
		else */if (wpn != nullptr && smart_cast<CMissile*>(wpn) != nullptr)
		{
			_planning_suicide = true;
			_suicide_now = false;
			if (wpn->GetState() == CMissile::eReady)
			{
				// Делаем бросок под ноги
				wpn->SwitchState(CMissile::eThrow);
				static_cast<CMissile*>(wpn)->PrepareGrenadeForSuicideThrow(READ_IF_EXISTS(pSettings, r_float, wpn->m_section_id, "suicide_ready_force", 8.f));
				wpn->Action(kWPN_ZOOM, CMD_STOP);
			}
			else if (wpn->GetState() == CMissile::eThrowStart)
			{
				if (static_cast<CMissile*>(wpn)->IsMissileInSuicideState())
				{
					static_cast<CMissile*>(wpn)->SetConstPowerStatus(true);
					static_cast<CMissile*>(wpn)->SetImmediateThrowStatus(true);
				}
				else
				{
					static_cast<CMissile*>(wpn)->PrepareGrenadeForSuicideThrow(READ_IF_EXISTS(pSettings, r_float, wpn->m_section_id, "suicide_ready_force", 8.f));
					wpn->Action(kWPN_ZOOM, CMD_STOP);
				}
			}
			else if (smart_cast<CGrenade*>(wpn) != nullptr && DistToContr() > READ_IF_EXISTS(pSettings, r_float, wpn->HudSection(), "controller_g_attack_min_dist", 10.f) && wpn->GetState() == CHUDState::eIdle && !READ_IF_EXISTS(pSettings, r_bool, wpn->HudSection(), "prohibit_suicide", false))
				wpn->SwitchState(CMissile::eThrowStart);
			else if (CanUseItemForSuicide(smart_cast<CHudItemObject*>(inventory().ItemFromSlot(1))))
				inventory().Activate(1, false);
			else
				inventory().Activate(0, false);
		}
		else
		{
			_planning_suicide = CanUseItemForSuicide(wpn);
			if (!_planning_suicide)
			{
				if (wpn != nullptr)
					g_PerformDrop();

				if (CanUseItemForSuicide(smart_cast<CHudItemObject*>(inventory().ItemFromSlot(1))))
				{
					inventory().Activate(1, false);
					_planning_suicide = true;
					_suicide_now = false;
				}
				else
				{
					inventory().Activate(0, false);
					_suicide_now = false;
				}
			}
		}
	}
}

void CActor::DoSuicideShot()
{
	CWeapon* wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
	if (wpn == nullptr)
		return;

	if (_lastshot_done_time > 0)
		return;

	//SetDisableInputStatus(true);
	_lastshot_done_time = Device.dwTimeGlobal;
	_death_action_started = true;

	wpn->SwitchState(CWeapon::eFire);
	wpn->SetWorkingState(true);

	NotifySuicideShotCallbackIfNeeded();
}

bool CActor::CheckActorVisibilityForController()
{
	if (g_SingleGameDifficulty >= egdVeteran)
		return true;

	for (int i = 0; i < _active_controllers.size(); ++i)
	{
		if (IsControllerSeeActor(_active_controllers[i]))
			return true;
	}

	return false;
}

void CActor::OnSuicideAnimEnd()
{
	CWeapon* wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
	if (wpn == nullptr)
		return;

	if ((!IsPsiBlocked() || IsPsiBlockFailed()) && (_suicide_now || _planning_suicide) && CheckActorVisibilityForController())
		DoSuicideShot();
	else
	{
		_suicide_now = false;
		_planning_suicide = false;
		NotifySuicideStopCallbackIfNeeded();
		ClearActiveControllers();
		wpn->SwitchState(CWeapon::eSuicideStop);
		SetHandsJitterTime(floor(READ_IF_EXISTS(pSettings, r_float, "gunslinger_base", "actor_shock_time", 10.0f) * 1000.0f));
	}
}

void CActor::NotifySuicideStopCallbackIfNeeded() const
{
	/*if (_active_controllers.size() > 0)
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("gunsl_controller.on_stop_suicide", funct))
			funct("", 0);
	}*/
}

void CActor::NotifySuicideShotCallbackIfNeeded() const
{
	/*if (_active_controllers.size() > 0)
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("gunsl_controller.on_suicide_shot", funct))
			funct("", 0);
	}*/
}

bool CActor::IsHandJitter(CHudItemObject* itm) const
{
	return ((IsActorControlled() || itm->IsSuicideAnimPlaying() || IsControllerPreparing()) && !IsSuicideInreversible()) || (_jitter_time_remains > 0);
}

float CActor::GetHandJitterScale(CHudItemObject* itm) const
{
	u32 restore_time = floor(READ_IF_EXISTS(pSettings, r_float, itm->HudSection(), "jitter_stop_time", 3.0f) * 1000.f);

	if (IsActorControlled() || itm->IsSuicideAnimPlaying() || IsControllerPreparing() || (_jitter_time_remains > restore_time))
		return 1.0f;
	else if (_jitter_time_remains == 0)
		return 0.0f;
	else
		return static_cast<float>(_jitter_time_remains) / restore_time;
}

ENGINE_API extern float psHUD_FOV_def;

void CActor::UpdateFOV()
{
	float hud_fov = 0.f;

	CHudItemObject* wpn = smart_cast<CHudItemObject*>(inventory().ActiveItem());
	CCustomDetector* det = GetDetector();

    float fov = g_fov;

    if (wpn != nullptr)
        fov *= player_hud::GetCachedCfgParamFloatDef(cached_fov_factor, wpn->m_section_id, "fov_factor", 1.0f);
	else if (det != nullptr)
        fov *= player_hud::GetCachedCfgParamFloatDef(cached_fov_factor, det->m_section_id, "fov_factor", 1.0f);

	g_fov = fov;

    fov = psHUD_FOV_def;
    if (wpn != nullptr)
	{
		fov = wpn->GetHudFov();
        hud_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_factor_wpn, wpn->HudSection(), "hud_fov_factor", 1.0f);
        if (wpn->WpnCanShoot() && static_cast<CWeapon*>(wpn)->get_ScopeStatus() == 2 && static_cast<CWeapon*>(wpn)->IsScopeAttached())
            hud_fov *= player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_factor_scope, static_cast<CWeapon*>(wpn)->GetCurrentScopeSection(), "hud_fov_factor", 1.0f);

        if (wpn->WpnCanShoot() && (static_cast<CWeapon*>(wpn)->IsZoomed() || wpn->GetActualCurrentAnim().find("anm_idle_aim") == 0))
		{
			/*alter_zoom_factor = GetAlterZoomDirectSwitchMixupFactor();
			if (CanUseAlterScope(wpn) && (buf->IsAlterZoomMode() || (GetAimFactor(wpn) > 0.f && buf->IsLastZoomAlter())))
			    alter_zoom_factor = 1.f - alter_zoom_factor;*/

			float zoom_fov = 0.f;

			shared_str zoom_fov_section = nullptr;

            if (static_cast<CWeapon*>(wpn)->IsGrenadeMode())
                zoom_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_gl_zoom_factor, wpn->HudSection(), "hud_fov_gl_zoom_factor", hud_fov);
			else if (static_cast<CWeapon*>(wpn)->get_ScopeStatus() == 2 && static_cast<CWeapon*>(wpn)->IsScopeAttached())
                zoom_fov_section = static_cast<CWeapon*>(wpn)->GetCurrentScopeSection();
			else
                zoom_fov_section = wpn->HudSection();

			//float alter_zoom_fov = alter_zoom_factor = 0.f;

            if (zoom_fov_section != nullptr)
			{
                //if (alter_zoom_factor < EPS)
                    zoom_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_zoom_factor, zoom_fov_section, "hud_fov_zoom_factor", hud_fov);
				/*else if (alter_zoom_factor > 1.f - EPS)
                    zoom_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_alter_zoom_factor, zoom_fov_section, "hud_fov_alter_zoom_factor", zoom_fov);
				else
				{
                    zoom_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_zoom_factor, zoom_fov_section, "hud_fov_zoom_factor", hud_fov);
                    alter_zoom_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_alter_zoom_factor, zoom_fov_section, "hud_fov_alter_zoom_factor", zoom_fov);
                    zoom_fov = zoom_fov + (alter_zoom_fov - zoom_fov) * alter_zoom_factor;
                }*/
            }

            float af = static_cast<CWeapon*>(wpn)->GetAimFactor();
            hud_fov = hud_fov - (hud_fov - zoom_fov) * af;
        }

        fov *= hud_fov;
    }
	else if (det != nullptr)
	{
		fov = det->GetHudFov();
        hud_fov = player_hud::GetCachedCfgParamFloatDef(cached_hud_fov_factor_wpn, det->HudSection(), "hud_fov_factor", 1.0f);
        fov *= hud_fov;
    }

	psHUD_FOV = fov;
}