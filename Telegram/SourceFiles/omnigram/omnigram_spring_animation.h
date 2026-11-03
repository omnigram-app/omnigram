/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <QtCore/QEasingCurve>
#include <QtCore/QPointF>

namespace Omnigram {

// Exact Apple Spring Parameters reverse-engineered from Telegram for macOS (TGUIKit)
struct SpringParams {
	float64 mass = 3.0;
	float64 stiffness = 1000.0;
	float64 damping = 500.0;
	float64 initialVelocity = 0.0;
};

// Preset spring configurations
[[nodiscard]] inline SpringParams StandardSpring() {
	// Standard UI transitions: mass 3.0, stiffness 1000.0, damping 500.0
	return SpringParams{
		.mass = 3.0,
		.stiffness = 1000.0,
		.damping = 500.0,
		.initialVelocity = 0.0,
	};
}

[[nodiscard]] inline SpringParams BouncySpring() {
	// Playful Apple bounce: mass 5.0, stiffness 900.0, damping 88.0
	return SpringParams{
		.mass = 5.0,
		.stiffness = 900.0,
		.damping = 88.0,
		.initialVelocity = 0.0,
	};
}

[[nodiscard]] inline SpringParams FlightVerticalSpring() {
	// Message / media send flight: stiffness 272.0, damping 33.0, mass 1.0
	return SpringParams{
		.mass = 1.0,
		.stiffness = 272.0,
		.damping = 33.0,
		.initialVelocity = 0.0,
	};
}

// Analytical Spring Motion Solver for any time t (seconds)
class SpringSolver final {
public:
	explicit SpringSolver(const SpringParams &params);

	// Computes normalized progress in [0..1] (or with overshoot) at time t
	[[nodiscard]] float64 solve(float64 t) const;

	// Computes position from `from` to `to` at time t
	[[nodiscard]] float64 interpolate(float64 from, float64 to, float64 t) const;

	// Settling duration when displacement drops below threshold (e.g. 0.001)
	[[nodiscard]] float64 settlingDuration() const;

private:
	SpringParams _params;
	float64 _gamma = 0.0;      // c / (2m)
	float64 _omega0 = 0.0;     // sqrt(k / m)
	float64 _omegaD = 0.0;     // damped angular frequency for underdamped
	float64 _r1 = 0.0;         // root 1 for overdamped
	float64 _r2 = 0.0;         // root 2 for overdamped
	enum class DampingType { Under, Critical, Over } _type = DampingType::Under;
	float64 _settlingDuration = 0.5;
};

// Custom Bezier Easing Curves matching macOS TGUIKit
// 1. System Spring Curve: cubic-bezier(0.23, 1.0, 0.32, 1.0)
// 2. Ease-in-Out Curve: cubic-bezier(0.42, 0.0, 0.58, 1.0)
[[nodiscard]] float64 EvaluateSystemSpringBezier(float64 progress);
[[nodiscard]] float64 EvaluateEaseInOutBezier(float64 progress);

[[nodiscard]] QEasingCurve MakeSystemSpringCurve();
[[nodiscard]] QEasingCurve MakeEaseInOutCurve();

} // namespace Omnigram
