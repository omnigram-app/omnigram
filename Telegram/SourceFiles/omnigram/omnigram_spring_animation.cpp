/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_spring_animation.h"

#include <cmath>
#include <algorithm>

namespace Omnigram {
namespace {

// Evaluates 1D cubic Bezier at parameter u: B(u) = (1-u)^3*p0 + 3*(1-u)^2*u*p1 + 3*(1-u)*u^2*p2 + u^3*p3
inline float64 Bezier1D(float64 p1, float64 p2, float64 u) {
	const auto inv = 1.0 - u;
	return 3.0 * inv * inv * u * p1 + 3.0 * inv * u * u * p2 + u * u * u;
}

// Derivative of 1D cubic Bezier with respect to u
inline float64 Bezier1DDerivative(float64 p1, float64 p2, float64 u) {
	const auto inv = 1.0 - u;
	return 3.0 * inv * inv * p1 + 6.0 * inv * u * (p2 - p1) + 3.0 * u * u * (1.0 - p2);
}

// Solves for parameter u given x using Newton-Raphson
float64 SolveBezierU(float64 p1x, float64 p2x, float64 x) {
	if (x <= 0.0) return 0.0;
	if (x >= 1.0) return 1.0;

	auto u = x;
	for (int i = 0; i < 8; ++i) {
		const auto currentX = Bezier1D(p1x, p2x, u) - x;
		if (std::abs(currentX) < 1e-6) {
			return u;
		}
		const auto dX = Bezier1DDerivative(p1x, p2x, u);
		if (std::abs(dX) < 1e-6) {
			break;
		}
		u -= currentX / dX;
	}

	// Bisection fallback
	auto low = 0.0;
	auto high = 1.0;
	u = x;
	while (low < high) {
		const auto currentX = Bezier1D(p1x, p2x, u);
		if (std::abs(currentX - x) < 1e-5) {
			return u;
		}
		if (x > currentX) {
			low = u;
		} else {
			high = u;
		}
		u = (high + low) * 0.5;
	}
	return u;
}

// Evaluates Y given X for a cubic Bezier curve (0,0) -> (p1x, p1y) -> (p2x, p2y) -> (1,1)
float64 EvaluateCubicBezier(float64 p1x, float64 p1y, float64 p2x, float64 p2y, float64 x) {
	const auto u = SolveBezierU(p1x, p2x, std::clamp(x, 0.0, 1.0));
	return Bezier1D(p1y, p2y, u);
}

} // namespace

SpringSolver::SpringSolver(const SpringParams &params)
: _params(params) {
	const auto m = std::max(params.mass, 0.001);
	const auto k = std::max(params.stiffness, 0.001);
	const auto c = std::max(params.damping, 0.0);

	_gamma = c / (2.0 * m);
	_omega0 = std::sqrt(k / m);

	const auto delta = _gamma * _gamma - _omega0 * _omega0;
	if (std::abs(delta) < 1e-5) {
		_type = DampingType::Critical;
	} else if (delta < 0.0) {
		_type = DampingType::Under;
		_omegaD = std::sqrt(_omega0 * _omega0 - _gamma * _gamma);
	} else {
		_type = DampingType::Over;
		const auto sqrtDelta = std::sqrt(delta);
		_r1 = -_gamma + sqrtDelta;
		_r2 = -_gamma - sqrtDelta;
	}

	// Calculate settling duration (where displacement envelope < 0.002)
	if (_gamma > 0.0) {
		_settlingDuration = std::min(std::log(500.0) / _gamma, 2.0);
	} else {
		_settlingDuration = 0.5;
	}
}

float64 SpringSolver::solve(float64 t) const {
	if (t <= 0.0) {
		return 0.0;
	}
	if (t >= _settlingDuration * 1.5) {
		return 1.0;
	}

	// Initial condition: at t=0, displacement from target = -1.0, velocity = v0
	const auto x0 = -1.0;
	const auto v0 = _params.initialVelocity;
	auto displacement = 0.0;

	switch (_type) {
	case DampingType::Under: {
		const auto envelope = std::exp(-_gamma * t);
		const auto a = x0;
		const auto b = (v0 + _gamma * x0) / _omegaD;
		displacement = envelope * (a * std::cos(_omegaD * t) + b * std::sin(_omegaD * t));
		break;
	}
	case DampingType::Critical: {
		const auto envelope = std::exp(-_gamma * t);
		const auto a = x0;
		const auto b = v0 + _gamma * x0;
		displacement = envelope * (a + b * t);
		break;
	}
	case DampingType::Over: {
		const auto c1 = (v0 - _r2 * x0) / (_r1 - _r2);
		const auto c2 = x0 - c1;
		displacement = c1 * std::exp(_r1 * t) + c2 * std::exp(_r2 * t);
		break;
	}
	}

	// Normalized progress = 1.0 + displacement
	return 1.0 + displacement;
}

float64 SpringSolver::interpolate(float64 from, float64 to, float64 t) const {
	return from + (to - from) * solve(t);
}

float64 SpringSolver::settlingDuration() const {
	return _settlingDuration;
}

float64 EvaluateSystemSpringBezier(float64 progress) {
	// macOS system spring curve: (0.23, 1.0, 0.32, 1.0)
	return EvaluateCubicBezier(0.23, 1.0, 0.32, 1.0, progress);
}

float64 EvaluateEaseInOutBezier(float64 progress) {
	// macOS ease-in-out: (0.42, 0.0, 0.58, 1.0)
	return EvaluateCubicBezier(0.42, 0.0, 0.58, 1.0, progress);
}

QEasingCurve MakeSystemSpringCurve() {
	QEasingCurve curve(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(
		QPointF(0.23, 1.0),
		QPointF(0.32, 1.0),
		QPointF(1.0, 1.0));
	return curve;
}

QEasingCurve MakeEaseInOutCurve() {
	QEasingCurve curve(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(
		QPointF(0.42, 0.0),
		QPointF(0.58, 1.0),
		QPointF(1.0, 1.0));
	return curve;
}

} // namespace Omnigram
