#pragma once

#include "base/Math.hpp"

namespace FW {

	class BezierCurve {

	public:
		static Vec3f evalBezier(
			const Vec3f & p0,
			const Vec3f & p1,
			const Vec3f & p2,
			const Vec3f & p3,
			const float t) {

			float a = 1.0f - t;

			return
				a*a*a*p0 +
				3.0f*a*a*t*p1 +
				3.0f*a*t*t*p2 +
				t*t*t*p3;

		}
	private:

	};

};