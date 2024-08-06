#ifndef	dx103DFluidData_included
#define	dx103DFluidData_included
#pragma once

#include "dx103DFluidEmitters.h"

class dx103DFluidData
{
public:
	enum eVolumePrivateRT
	{
		VP_VELOCITY0 = 0,
		VP_PRESSURE,
		VP_COLOR,	//	Swap with global after update
		VP_NUM_TARGETS
	};

	enum SimulationType
	{
		ST_FOG = 0,
		ST_FIRE,
	};

	struct Settings
	{
		float			m_fHemi;
		float			m_fConfinementScale;
		float			m_fDecay;
		float			m_fGravityBuoyancy;
		SimulationType	m_SimulationType;
	};
public:
	dx103DFluidData();
	~dx103DFluidData();

	void	Load(IReader *data);

	void	SetTexture(eVolumePrivateRT id, ITexture3D *pT) { pT->AddRef(); m_pRTTextures[id]->Release(); m_pRTTextures[id] = pT;}
	void	SetView(eVolumePrivateRT id, IRenderTargetView *pV) { pV->AddRef(); m_pRenderTargetViews[id]->Release(); m_pRenderTargetViews[id] = pV;}

	ITexture3D*					GetTexture(eVolumePrivateRT id) const { m_pRTTextures[id]->AddRef(); return m_pRTTextures[id];}
	IRenderTargetView*			GetView(eVolumePrivateRT id) const { m_pRenderTargetViews[id]->AddRef(); return m_pRenderTargetViews[id];}
	const Fmatrix&				GetTransform() const { return m_Transform;}
	const xr_vector<Fmatrix>&	GetObstaclesList() const { return m_Obstacles;}
	const xr_vector<dx103DFluidEmitters::CEmitter>& GetEmittersList() const { return m_Emitters;}
	const Settings&				GetSettings() const { return m_Settings;}

	//	Allow real-time config reload
#ifdef	DEBUG_DRAW
	void	ReparseProfile(const xr_string &Profile);
#endif	//	DEBUG

private:
	typedef	dx103DFluidEmitters::CEmitter	CEmitter;

private:

	void	CreateRTTextureAndViews(int rtIndex, const STexture3DDesc& TexDesc);
	void	DestroyRTTextureAndViews(int rtIndex);

	void	ParseProfile(const xr_string &Profile);

private:
	Fmatrix					m_Transform;

	xr_vector<Fmatrix>		m_Obstacles;
	xr_vector<CEmitter>		m_Emitters;

	Settings				m_Settings;

	IRenderTargetView		*m_pRenderTargetViews[ VP_NUM_TARGETS ];
	ITexture3D				*m_pRTTextures[ VP_NUM_TARGETS ];

	static	ERHITextureFormat	m_VPRenderTargetFormats[VP_NUM_TARGETS];
};

#endif	//	dx103DFluidData_included