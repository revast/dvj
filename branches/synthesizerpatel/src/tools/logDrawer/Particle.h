/*
 *
 * Particle.h
 *
 */

#ifndef DEFINE_PARTICLE
#define DEFINE_PARTICLE

#include <vector>

#include "LGL.module/LGL.h"

class ParticleObj
{

public:

			ParticleObj
			(
				LGL_Vector inPos,
				LGL_Vector inAxisScale,
				float radiusB, float radiusE,
				float rb, float gb, float bb, float ab,
				float re, float ge, float be, float ae,
				float life, float velocity, float drag,
				LGL_Image* image
			);
			~ParticleObj();

	bool		NextFrame(float secondsElapsed);
	void		Draw(float brightness);

	LGL_Vector	Pos;
	LGL_Vector	Vel;
	LGL_Vector	AxisScale;
	float		RadiusB,RadiusE;
	float		Rb,Gb,Bb,Ab;
	float		Re,Ge,Be,Ae;
	float		LifeNow,LifeMax;
	float		Drag;

	LGL_Image*	Image;
};

class ParticleSystemObj
{

public:
	
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
			);
			~ParticleSystemObj();

	void		NextFrame(float secondsElapsed);
	void		Draw(float brightness=1);

	std::vector
	<ParticleObj*>	Particles;

	LGL_Vector	PosPrev;
	LGL_Vector	Pos;
	LGL_Vector	Vel;
	LGL_Vector	AxisScale;
	float		RadiusB,RadiusE;
	float		Rb,Gb,Bb,Ab;
	float		Re,Ge,Be,Ae;
	float		LifeMin,LifeMax;
	float		VelocityMin,VelocityMax;
	float		Drag;
	LGL_Image*	Image;
	float		ParticlesPerSecond;
	float		CarryOver;
	bool		ConstantFlow;

	bool		KillWhenYLessThanZero;
};

#endif