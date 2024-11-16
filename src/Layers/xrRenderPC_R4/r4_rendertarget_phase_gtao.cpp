#include "stdafx.h"

#include "r4_rendertarget.h"

void CRenderTarget::phase_gtao()
{
	u32 Offset = 0;
    constexpr u32 vertex_color = color_rgba(0, 0, 0, 255);

	//Calculate projection factor, to transform world radius to screen space
	float p_scale = RCache.get_height() / (tan(deg2rad(Device.fFOV) * 0.5f) * 2.0f);
	p_scale *= 0.5;

	//Render the AO and view-z into new rendertarget
    u_setrt(rt_gtao_0, nullptr, nullptr, nullptr);
    RCache.set_CullMode(CULL_NONE);
    RCache.set_Stencil(FALSE);

	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//Go go power rangers
    RCache.set_Element(s_gtao->E[0]);
	RCache.set_c("gtao_parameters", p_scale);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);
	
	//Blur...
    u_setrt(rt_ssao_temp, nullptr, nullptr, nullptr);
    RCache.set_CullMode(CULL_NONE);
    RCache.set_Stencil(FALSE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//Go go power rangers
    RCache.set_Element(s_gtao->E[1]);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);
}

void CRenderTarget::phase_hiz_depth() {
	u32 Offset = 0;
	constexpr u32 vertex_color = color_rgba(0, 0, 0, 255);

	u_setrt(rt_half_depth_temp, nullptr, nullptr, nullptr);

	RImplementation.rmNormal();
	RCache.set_CullMode(CULL_NONE);

	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0); pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0); pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0); pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//Go go power rangers
	RCache.set_Element(s_ssao->E[2]);
	RCache.set_Geometry(g_combine);

	RCache.set_c("sample_mip_level", 0);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	RContext->CopyResource(rt_half_depth->pSurface, rt_half_depth_temp->pSurface);

	for(UINT mip_level = 1, mip_count = rt_half_depth_temp->pMippedRT.size(); mip_level < mip_count; ++mip_level)
	{
		dwWidth /= 2;
		dwHeight /= 2;

		u_setrt(dwWidth, dwHeight, 
			rt_half_depth_temp->pMippedRT[mip_level], nullptr, nullptr, nullptr);

		RImplementation.rmNormal();

		RCache.set_CullMode(CULL_NONE);

		pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
		pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0); pv++;
		pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0); pv++;
		pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0); pv++;
		RCache.Vertex.Unlock(3, g_combine->vb_stride);

		//Go go power rangers
		RCache.set_Element(s_ssao->E[3]);
		RCache.set_Geometry(g_combine);

		RCache.set_c("sample_mip_level", int(mip_level) - 1);
		RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

		RContext->CopyResource(rt_half_depth->pSurface, rt_half_depth_temp->pSurface);
	}
}

void CRenderTarget::phase_sslr() {
	u32 Offset = 0;
	constexpr u32 vertex_color = color_rgba(0, 0, 0, 255);

	//Render the AO and view-z into new rendertarget
	u_setrt(rt_sslr, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);

	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//Go go power rangers
	RCache.set_Element(s_gtao->E[2]);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	u_setrt(rt_sslr_temp1, nullptr, nullptr, nullptr);
	RCache.set_CullMode(CULL_NONE);

	pv = (FVF::TL*)RCache.Vertex.Lock(3, g_combine->vb_stride, Offset);
	pv->set(-1.0, 1.0, 1.0, 1.0, vertex_color, 0.0, 0.0);
	pv++;
	pv->set(3.0, 1.0, 1.0, 1.0, vertex_color, 2.0, 0.0);
	pv++;
	pv->set(-1.0, -3.0, 1.0, 1.0, vertex_color, 0.0, 2.0);
	pv++;
	RCache.Vertex.Unlock(3, g_combine->vb_stride);

	//Go go power rangers
	RCache.set_Element(s_gtao->E[3]);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 3, 0, 1);

	RContext->CopyResource(rt_sslr_temp->pSurface, rt_sslr_temp1->pSurface);
}