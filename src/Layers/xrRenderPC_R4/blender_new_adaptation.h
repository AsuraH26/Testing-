#pragma once

class CBlender_new_adaptation : public IBlender
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: ground-truth based ambient occlusion";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_new_adaptation();
	virtual ~CBlender_new_adaptation();
};
