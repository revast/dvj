/*
 *
 * Particle.cpp
 *
 */

#include "Particle.h"

ParticleObj::
ParticleObj
(
	LGL_Vector inPos,
	LGL_Vector inAxisScale,
	float radiusB, float radiusE,
	float rb, float gb, float bb, float ab,
	float re, float ge, float be, float ae,
	float life, float velocity, float drag,
	LGL_Image* image
)
{
	Pos=inPos;
	Vel.SetXY(0,velocity);
	Vel.SetAngleXY(LGL_RandFloat(0,2*LGL_PI));
	AxisScale=inAxisScale;
	
	RadiusB=radiusB;
	RadiusE=radiusE;
	
	Rb=rb;
	Gb=gb;
	Bb=bb;
	Ab=ab;
	Re=re;
	Ge=ge;
	Be=be;
	Ae=ae;
	
	LifeNow=0;
	LifeMax=life;
	Drag=drag;
	Image=image;
}

ParticleObj::
~ParticleObj()
{
	//
}

bool
ParticleObj::
NextFrame(float secondsElapsed)
{
	LifeNow+=secondsElapsed;
	float LifePercent=LifeNow/LifeMax;

	if(LifePercent>1) return(false);

	Pos=Pos+Vel*secondsElapsed*(1.0-Drag*LifePercent);

	return(true);
}

void
ParticleObj::
Draw(float brightness)
{
	float LifePercent=LifeNow/LifeMax;

	if(LifePercent>1) return;

	float rad=	(
				RadiusB*(1.0-LifePercent) +
				RadiusE*(LifePercent)
			) / 2.0;

	float r=(Rb*(1.0-LifePercent)+Re*(LifePercent))/2.0;
	float g=(Gb*(1.0-LifePercent)+Ge*(LifePercent))/2.0;
	float b=(Bb*(1.0-LifePercent)+Be*(LifePercent))/2.0;
	float a=(Ab*(1.0-LifePercent)+Ae*(LifePercent))/2.0;
	float br=brightness;

	Image->DrawToScreen
	(
		Pos.GetX()-AxisScale.GetX()*rad/2.0,Pos.GetX()-AxisScale.GetX()*rad/2.0+AxisScale.GetX()*rad,
		Pos.GetY()-AxisScale.GetY()*rad/2.0,Pos.GetY()-AxisScale.GetY()*rad/2.0+AxisScale.GetY()*rad,
		0,
		r*br,g*br,b*br,a
	);	
}

//

ParticleSystemObj::
ParticleSystemObj
(
	float x, float y,
	float xAxisScale, float yAxisScale,
	float radiusB, float radiusE,
	float rb, float gb, float bb, float ab,
	float re, float ge, float be, float ae,
	float lifeMin, float lifeMax,
	float velocityMin, float velocityMax,
	float drag,
	LGL_Image* image,
	float particlespersecond
)
{
	Pos.SetXY(x,y);
	Vel.SetXY(0,0);
	AxisScale.SetXY(xAxisScale,yAxisScale);
	RadiusB=radiusB;
	RadiusE=radiusE;
	Rb=rb;
	Gb=gb;
	Bb=bb;
	Ab=ab;
	Re=re;
	Ge=ge;
	Be=be;
	Ae=ae;
	LifeMin=lifeMin;
	LifeMax=lifeMax;
	VelocityMin=velocityMin;
	VelocityMax=velocityMax;
	Drag=drag;
	Image=image;
	ParticlesPerSecond=particlespersecond;
	CarryOver=0;
	ConstantFlow=false;

	KillWhenYLessThanZero=false;
}

ParticleSystemObj::
~ParticleSystemObj()
{
	while(Particles.size()>0)
	{
		ParticleObj* t=Particles[0];
		Particles.erase
		(
			(std::vector<ParticleObj*>::iterator)(&(Particles[0]))
		);
		delete t;
	}
}

void
ParticleSystemObj::
NextFrame(float secondsElapsed)
{
	if(ConstantFlow)
	{
		float ConstantNum=secondsElapsed*ParticlesPerSecond+CarryOver;
		while(ConstantNum>=1)
		{
			//Create a new particle

			float life=LifeMin+LGL_RandFloat()*(LifeMax-LifeMin);
			float velocity=VelocityMin+LGL_RandFloat()*
				(VelocityMax-VelocityMin);
			ParticleObj* neo=new ParticleObj
			(
				Pos,
				AxisScale,
				RadiusB,RadiusE,
				Rb,Gb,Bb,Ab,
				Re,Ge,Be,Ae,
				life, velocity, Drag,
				Image
			);
			if(Vel.GetX()!=0 || Vel.GetY()!=0)
			{
				neo->Vel=Vel;
			}
			Particles.push_back(neo);
			
			ConstantNum--;
		}
		CarryOver=fabs(ConstantNum);
	}
	else
	{
		float LuckyNum=secondsElapsed*ParticlesPerSecond+CarryOver;
		float LuckNumTotal=LuckyNum;
		while(LuckyNum>0)
		{
			if(LuckyNum>LGL_RandFloat())
			{
				//Create a new particle

				float life=LifeMin+LGL_RandFloat()*(LifeMax-LifeMin);
				float velocity=VelocityMin+LGL_RandFloat()*
					(VelocityMax-VelocityMin);
				float percentTowardsNewPosition=LuckyNum/LuckNumTotal;
				LGL_Vector particlePos=
					Pos*(0.0f+percentTowardsNewPosition)+
					PosPrev*(1.0f-percentTowardsNewPosition);
				ParticleObj* neo=new ParticleObj
				(
					particlePos,
					AxisScale,
					RadiusB,RadiusE,
					Rb,Gb,Bb,Ab,
					Re,Ge,Be,Ae,
					life, velocity, Drag,
					Image
				);
				if(Vel.GetX()!=0 || Vel.GetY()!=0)
				{
					neo->Vel=Vel;
				}
				Particles.push_back(neo);
				CarryOver=0;
			}
			else
			{
				CarryOver=0;//fabs(LuckyNum);
			}
			LuckyNum--;
		}
	}

	for(unsigned int a=0;a<Particles.size();a++)
	{
		if
		(
			Particles[a]->NextFrame(secondsElapsed)==false ||
			(
				Particles[a]->Pos.GetY()<0-Particles[a]->RadiusE &&
				KillWhenYLessThanZero
			)
		)
		{
			//The particle wants to die

			ParticleObj* t=Particles[a];
			Particles.erase
			(
				(std::vector<ParticleObj*>::iterator)(&(Particles[a]))
			);
			delete t;
			a--;
		}
			
	}
}

void
ParticleSystemObj::
Draw(float brightness)
{
	for(unsigned int a=0;a<Particles.size();a++)
	{
		Particles[a]->Draw(brightness);
	}
}

