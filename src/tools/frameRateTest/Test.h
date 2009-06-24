/*
 *
 * Test.h
 *
 */

#ifndef	_TEST_H
#define	_TEST_H

#include <string.h>
#include <assert.h>

class Test
{

public:

				Test()				{ Active=false; strcpy(Name,"Nameless"); };
	virtual			~Test()				{ if(Active) Deactivate(); };

		void		ToggleActive()			{ if(Active) Deactivate(); else Activate(); };
	virtual	void		Activate()			{ assert(Active==false); Active=true; PercentComplete=0.0f; };
		bool		IsActive() const		{ return(Active); };
	virtual void		Update()			{ };
	virtual	void		Deactivate()			{ assert(Active); Active=false; PercentComplete=0.0f; };

	virtual	const char*	GetName() const			{ return(Name); };
	virtual const char*	GetStatus() const		{ return(""); };
		float		GetPercentComplete() const	{ return(PercentComplete); };

protected:

	bool			Active;
	float			PercentComplete;
	char			Name[1024];
};

#endif	//_TEST_H

