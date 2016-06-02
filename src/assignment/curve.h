#pragma once

#include "AutomaticDifferentiation.h"
#include "base/Math.hpp"

#include <vector>
#include <functional>

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V), the tangent (T), the
// normal (N), and the binormal (B).  It is the responsiblility of
// functions that create these objects to fill in all the data.
struct CurvePoint
{
    FW::Vec3f V; // Vertex
    FW::Vec3f T; // Tangent  (unit)
    FW::Vec3f N; // Normal   (unit)
    FW::Vec3f B; // Binormal (unit)
	FW::Vec3f C; // Curvature (not unit)
	float	  t; // time	 (necessary only for the adaptive tessellation extra)

};

// This is just a handy shortcut.
typedef std::vector<CurvePoint> Curve;

////////////////////////////////////////////////////////////////////////////
// The following two functions take an array of control points (stored
// in P) and generate an STL Vector of CurvePoints.  They should
// return an empty array if the number of control points in C is
// inconsistent with the type of curve.  In both these functions,
// "step" indicates the number of samples PER PIECE.  E.g., a
// 7-control-point Bezier curve will have two pieces (and the 4th
// control point is shared).
////////////////////////////////////////////////////////////////////////////

// Assume number of control points properly specifies a piecewise
// Bezier curve.  I.e., C.size() == 4 + 3*n, n=0,1,...
Curve evalBezier(const std::vector<FW::Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep);

// Bsplines only require that there are at least 4 control points.
Curve evalBspline(const std::vector<FW::Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep);

// Catmull-rom spline requires at least 4 points.
Curve evalCatmullRomspline(const std::vector<FW::Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep);

// Create a circle on the xy-plane of radius and steps
Curve evalCircle(float radius, unsigned steps);

// Creates a trefoil knot curve
Curve evalTrefoilKnot(unsigned steps);

// Creates a trefoil knot curve
Curve evalArchytasKnot(unsigned steps);


// Draw the curve and (optionally) the associated coordinate frames
// If framesize == 0, then no frames are drawn.  Otherwise, drawn.
// Additionally, if framesize < 0, then only frames are drawn, with the
// absolute value used, and the actual curve is not drawn.
void drawCurve(const Curve& curve, float framesize = 0);

float getCurveLength(const Curve & curve);

Curve evalUnitConstantCurve(float scale);

struct DualParamCoord {
	DualNumber x_;
	DualNumber y_;
	DualNumber z_;

	DualParamCoord(const DualNumber & x, const DualNumber & y, const DualNumber & z) :
		x_(x),
		y_(y),
		z_(z) {};

};

Curve evalCustomCurve(const std::function<DualParamCoord(float)> & function, unsigned steps, float from, float to);