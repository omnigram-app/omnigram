/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <QtGui/QPainterPath>
#include <QtCore/QRectF>

namespace Omnigram {

// Matching Swift MessageBubbleImageNeighbors
enum class BubbleNeighbors {
	None,       // Single standalone message
	Both,       // Message in the middle of a consecutive cluster
	Bottom,     // First message in cluster (has neighbor below)
	Top,        // Last message in cluster (has neighbor above)
	Side,       // Album / media cluster side
};

struct BubbleRadii {
	float64 topLeft = 16.0;
	float64 topRight = 16.0;
	float64 bottomLeft = 16.0;
	float64 bottomRight = 16.0;
	bool hasTail = true;
};

// Returns exact 4-corner radii matching macOS ChatMessageBubbleImages.swift
[[nodiscard]] BubbleRadii CalculateBubbleRadii(
	BubbleNeighbors neighbors,
	bool incoming,
	float64 mainRadius = 16.0,
	float64 auxRadius = 8.0);

// Generates an exact QPainterPath for a message bubble matching Telegram for macOS
[[nodiscard]] QPainterPath GenerateBubblePath(
	const QRectF &rect,
	const BubbleRadii &radii,
	bool incoming,
	float64 tailWidth = 6.0);

// Constants
inline constexpr float64 kMacOSBubbleMainRadius = 16.0;
inline constexpr float64 kMacOSBubbleAuxRadius = 8.0;
inline constexpr float64 kMacOSBubbleTailWidth = 6.0;
inline constexpr float64 kMacOSBubbleBaseDiameter = 32.0;

} // namespace Omnigram
