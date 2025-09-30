#include "graphics.h"
#include <math.h>

void setColor(SDL_Renderer* renderer, const SDL_Color* color) {
	SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
}

void renderRect(SDL_Renderer* renderer,
			SDL_FRect* rect, int width,
			const SDL_Color* color, const SDL_Color* borderColor)
{
	if (color != NULL) {
		setColor(renderer, color);
		SDL_RenderFillRect(renderer, rect);
	}
	if (borderColor != NULL) {
		setColor(renderer, borderColor);
		for (int i = 0; i < width; i++) {
			SDL_FRect borderRect = {rect->x + i, rect->y + i,
									rect->w - 2*i, rect->h - 2*i};
			SDL_RenderRect(renderer, &borderRect);
		}
	}
}

void renderCircle(SDL_Renderer* renderer,
		float x, float y, float r, float width,
		const SDL_Color* color, const SDL_Color* borderColor) {
	// algorithm calculates an octant of circle and mirrors it using this array
	int mirrors[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

	float max_dx, max_dx2, dx, dy, dy2, dx2, px, py, pxm, pym, py2, pxm2, pym2;
	float r2 = r + width;
	max_dx = ceil(sqrt(0.5)*r);
	max_dx2 = ceil(sqrt(0.5)*r2);

	for (dx = 0; dx <= max_dx2; dx++) {
		if (dx < r) {
			dy = sqrt(r * r - dx * dx);
		} else dy = -1;
		dy2 = sqrt(r2 * r2 - dx * dx);
		if (dy2 < r) {
			dx2 = sqrt(r * r - dy2 * dy2);
		} else dx2 = -1;
		
		for (int i = 0; i < 4; i++) {
			px = x + dx * mirrors[i][0];
			py = y + dy * mirrors[i][1];
			pxm = x + dy * mirrors[i][0];
			pym = y + dx * mirrors[i][1];

			py2 = y + dy2 * mirrors[i][1];
			pxm2 = x + dy2 * mirrors[i][0];
			pym2 = y + dx2 * mirrors[i][1];
			// draw inside
			if (dx <= max_dx && color != NULL) {
				setColor(renderer, color);

				//SDL_RenderPoint(renderer, px, py);
				//SDL_RenderPoint(renderer, pxm, pym);
				SDL_RenderLine(renderer, px, py, px, y);
				SDL_RenderLine(renderer, pxm, pym, pxm, y);

			}
			
			// draw border
			if (width > 0 && borderColor != NULL) {
				setColor(renderer, borderColor);
				SDL_RenderLine(renderer, px, py2, px, py);
				SDL_RenderLine(renderer, pxm2, pym, pxm2, pym2);
			}
		}
	}
}

void renderCircle_int(SDL_Renderer* renderer,
		int centreX, int centreY, int radius, int width,
		const SDL_Color* color, const SDL_Color* borderColor) {
	// algorithm calculates an octant of circle and mirrors it using this array
	if (color != NULL) setColor(renderer, color);
	const int diameter = (radius * 2);

	int x = (radius - 1);
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = (tx - diameter);
	int leftX, rightX, leftXy, rightXy;
	int topY, bottomY, topYx, bottomYx;

	while (x >= y)
	{
		leftX = centreX - x;
		rightX = centreX + x;
		leftXy = centreX - y;
		rightXy = centreX + y;

		topY = centreY - y;
		bottomY = centreY + y;
		topYx = centreY - x;
		bottomYx = centreY + x;

		//  Each of the following renders an octant of the circle
		SDL_RenderLine(renderer, leftX, topY, leftX, bottomY);
		SDL_RenderLine(renderer, rightX, topY, rightX, bottomY);
		SDL_RenderLine(renderer, leftXy, topYx, leftXy, bottomYx);
		SDL_RenderLine(renderer, rightXy, topYx, rightXy, bottomYx);

		//SDL_RenderPoint(renderer, leftX, topY);
		//SDL_RenderPoint(renderer, leftX, bottomY);
		//SDL_RenderPoint(renderer, rightX, topY);
		//SDL_RenderPoint(renderer, rightX, bottomY);
		//SDL_RenderPoint(renderer, leftXy, topYx);
		//SDL_RenderPoint(renderer, leftXy, bottomYx);
		//SDL_RenderPoint(renderer, rightXy, topYx);
		//SDL_RenderPoint(renderer, rightXy, bottomYx);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

