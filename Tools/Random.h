#pragma once

#include <stdlib.h>

class CRandom
{
public:
	static float Get(float fMin, float fMax)
	{
		float f = (float)rand() / (float)RAND_MAX;
		f *= (fMax - fMin);
		f += fMin;
		return f;
	}
};
