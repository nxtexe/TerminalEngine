#include "MathHelper.h"
#include<float.h>
#include<cmath>

using namespace DirectX;

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY(float x, float y) {
	float theta = 0.0f;

	//quadrant I or IV
	if (x >= 0.0f) {
		//if x = 0, then atanf(y/x) = +pi/2 if y > 0
		//               atanf(y/x) = -pi/2 if y < 0

		theta = atanf(y / x); // in range [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += 2.0 * Pi; ///in range [0, 2*pi]
	}

	//quadrant II or III
	else
		theta = atanf(y / x) + Pi; // in range [0, 2*pi]

	return theta;
}

XMVECTOR MathHelper::RandUnitVec3() {
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	//keep trying until we get a point on/in the hemisphere
	while (true) {
		//Generate random point in the cube [-1, 1]^3
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		//ignore points outside the unit sphere in order to get an even distribution
		//over the unit sphere. Otherwise points will clump more on the sphere near
		//the corners of the cube

		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n) {
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	//keep trying until we get a point on/in the hemisphere
	while (true) {
		//generate random point in the cube [-1, 1]^3
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);


		//ignore points outside the unit sphere in order to get an even distribution
		//over the unit sphere. Otherwise points will clump more on the sphere near
		//the corners of the cube

		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		//ignore points in the bottom hemisphere
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
			continue;

		return XMVector3Normalize(v);
	}
}

