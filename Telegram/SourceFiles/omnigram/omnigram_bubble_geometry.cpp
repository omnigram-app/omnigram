/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_bubble_geometry.h"

#include <QtGui/QPainterPath>
#include <algorithm>

namespace Omnigram {

BubbleRadii CalculateBubbleRadii(
		BubbleNeighbors neighbors,
		bool incoming,
		float64 mainRadius,
		float64 auxRadius) {
	auto result = BubbleRadii();

	switch (neighbors) {
	case BubbleNeighbors::None:
		result.topLeft = mainRadius;
		result.topRight = mainRadius;
		result.bottomLeft = mainRadius;
		result.bottomRight = mainRadius;
		result.hasTail = true;
		break;
	case BubbleNeighbors::Both:
		result.topLeft = mainRadius;
		result.topRight = auxRadius;
		result.bottomLeft = mainRadius;
		result.bottomRight = auxRadius;
		result.hasTail = false;
		break;
	case BubbleNeighbors::Bottom:
		result.topLeft = mainRadius;
		result.topRight = auxRadius;
		result.bottomLeft = mainRadius;
		result.bottomRight = mainRadius;
		result.hasTail = true;
		break;
	case BubbleNeighbors::Top:
		result.topLeft = mainRadius;
		result.topRight = mainRadius;
		result.bottomLeft = mainRadius;
		result.bottomRight = auxRadius;
		result.hasTail = false;
		break;
	case BubbleNeighbors::Side:
		result.topLeft = mainRadius;
		result.topRight = mainRadius;
		result.bottomLeft = auxRadius;
		result.bottomRight = auxRadius;
		result.hasTail = false;
		break;
	}

	// For incoming messages, swap left and right corners
	if (incoming) {
		std::swap(result.topLeft, result.topRight);
		std::swap(result.bottomLeft, result.bottomRight);
	}

	return result;
}

QPainterPath GenerateBubblePath(
		const QRectF &rect,
		const BubbleRadii &radii,
		bool incoming,
		float64 tailWidth) {
	const auto x = rect.x();
	const auto y = rect.y();
	const auto w = rect.width();
	const auto h = rect.height();

	const auto tl = std::min(radii.topLeft, std::min(w, h) / 2.0);
	const auto tr = std::min(radii.topRight, std::min(w, h) / 2.0);
	const auto bl = std::min(radii.bottomLeft, std::min(w, h) / 2.0);
	const auto br = std::min(radii.bottomRight, std::min(w, h) / 2.0);

	QPainterPath path;

	// Start at top edge after top-left corner
	path.moveTo(x + tl, y);

	// Top edge to top-right
	path.lineTo(x + w - tr, y);
	if (tr > 0.0) {
		path.arcTo(x + w - 2 * tr, y, 2 * tr, 2 * tr, 90, -90);
	}

	// Right edge
	if (!incoming && radii.hasTail) {
		// Outgoing bubble with bottom-right tail
		path.lineTo(x + w, y + h - br);
		// Tail bottom curve
		const auto tailX = x + w;
		const auto tailY = y + h;
		path.cubicTo(
			tailX, tailY - 6.0,
			tailX + tailWidth, tailY - 2.0,
			tailX + tailWidth, tailY);
		path.cubicTo(
			tailX + tailWidth * 0.5, tailY,
			tailX + 2.0, tailY,
			tailX - br, tailY);
	} else {
		path.lineTo(x + w, y + h - br);
		if (br > 0.0) {
			path.arcTo(x + w - 2 * br, y + h - 2 * br, 2 * br, 2 * br, 0, -90);
		}
	}

	// Bottom edge to bottom-left
	if (incoming && radii.hasTail) {
		// Incoming bubble with bottom-left tail
		path.lineTo(x + bl, y + h);
		const auto tailX = x;
		const auto tailY = y + h;
		path.cubicTo(
			tailX - 2.0, tailY,
			tailX - tailWidth * 0.5, tailY,
			tailX - tailWidth, tailY);
		path.cubicTo(
			tailX - tailWidth, tailY - 2.0,
			tailX, tailY - 6.0,
			tailX, tailY - bl);
	} else {
		path.lineTo(x + bl, y + h);
		if (bl > 0.0) {
			path.arcTo(x, y + h - 2 * bl, 2 * bl, 2 * bl, 270, -90);
		}
	}

	// Left edge to top-left
	path.lineTo(x, y + tl);
	if (tl > 0.0) {
		path.arcTo(x, y, 2 * tl, 2 * tl, 180, -90);
	}

	path.closeSubpath();
	return path;
}

} // namespace Omnigram
