#include <iostream>
#include "assert.h"
#include "RasterSurface.h"

using namespace std;

#define RASTER_WIDTH 500
#define RASTER_HEIGHT 500
#define NUM_PIXELS ((RASTER_WIDTH) * (RASTER_HEIGHT))
#define White 0xFFFFFFFF
#define RED 0x00FF0000
#define BLUE 0x0000FF00
#define RED2 0xFF000000
#define BLUE2 0x00FF0000
#define GREEN 0x0000FF00
#define MAGENTA 0xFFFF00FF
#define CYAN2 0xFFFF00FF
#define CYAN 0xFF00FFFF

#define BUTTERFLYSWAP(x, y) \
	( \
		(x ^= y), \
		(y ^= x), \
		(x ^= y), \
    (void) 0 \
	) \

void PlotPixel(int x, int y, unsigned int color);


struct Point2D
{
	float x, y;
};


struct Vertex_2D
{
	Point2D pos;
	unsigned int clr;
};

//math functions
float implicitLineEquation(Point2D a, Point2D b, Point2D p)
{	//Ax + By + c = 0
	//(y1-y2)x + (x2-x1)y + x1y2 - x2y1
	return (a.y - b.y) * p.x + (b.x - a.x) * p.y + a.x * b.y - b.x * a.y;
}

float Lerpf(int start, int end, float ratio)
{
	return ((static_cast<float>(end) * ratio) + ((1.0f - ratio) * static_cast<float>(start)));
}

float LerpColor(unsigned int startcolor, unsigned int endcolor, float ratio)
{
	unsigned int blend_ratio = static_cast<unsigned int>(ratio * 255.0f);

	unsigned int startalpha = (startcolor & 0xff000000) >> 24;
	unsigned int startred = (startcolor & 0x00ff0000) >> 16;
	unsigned int startgreen = (startcolor & 0x0000ff00) >> 8;
	unsigned int startblue = (startcolor & 0x000000ff);

	unsigned int endalpha = (endcolor & 0xff000000) >> 24;
	unsigned int endred = (endcolor & 0x00ff0000) >> 16;
	unsigned int endgreen = (endcolor & 0x0000ff00) >> 8;
	unsigned int endblue = (endcolor & 0x000000ff);

	unsigned int alpha = ((blend_ratio * startalpha) + (255 - blend_ratio) * endalpha) >> 8;
	unsigned int red = ((blend_ratio * startred) + (255 - blend_ratio) * endred) >> 8;
	unsigned int green = ((blend_ratio * startgreen) + (255 - blend_ratio) * endgreen) >> 8;
	unsigned int blue = ((blend_ratio * startblue) + (255 - blend_ratio) * endblue) >> 8;

	return ((alpha << 24) | (red << 16) | (green << 8) | (blue));

	/*end = 0x00000000;
	
	unsigned int BlueMask = 0xff000000;
	unsigned int GreenMask = 0x00ff0000;
	unsigned int RedMask = 0x0000ff00;
	unsigned int AlphaMask = 0x000000ff;

	unsigned int AlphaMask2 = 0xff000000;
	unsigned int RedMask2 = 0x00ff0000;
	unsigned int GreenMask2 = 0x0000ff00;
	unsigned int BlueMask2 = 0x000000ff;

	
	int SourceBlue = (start & BlueMask) >> 24;
	int SourceGreen = (start & GreenMask) >> 16;
	int SourceRed = (start & RedMask) >> 8;
	int SourceAlpha = (start & AlphaMask);


	int DestAlpha = (end & AlphaMask2) >> 24;
	int DestRed = (end & RedMask2) >> 16;
	int DestGreen = (end & GreenMask2) >> 8;
	int DestBlue = (end & BlueMask2);

	unsigned int FinalRed = (unsigned int)((SourceRed - DestRed) * ratio + DestRed);
	unsigned int FinalGreen = (unsigned int)((SourceGreen - DestGreen) * ratio + DestGreen);
	unsigned int FinalBlue = (unsigned int)((SourceBlue - DestBlue) * ratio + DestBlue);
	unsigned int FinalAlpha = (unsigned int)((SourceAlpha - DestAlpha) * ratio + DestAlpha);

	return end = FinalAlpha << 24 | FinalRed << 16 | FinalGreen << 8 | FinalBlue;*/
}


void DrawMidPointLine(Vertex_2D start, Vertex_2D end)
{
	int currY = static_cast<int>(start.pos.y);

	for (size_t i = static_cast<int>(start.pos.x); i < static_cast<int>(end.pos.x); i++)
	{
		PlotPixel(i, currY, start.clr);
		Point2D mid = { i + 1.0f, currY + 0.5f };

		if (implicitLineEquation(start.pos, end.pos, mid) < 0)
		{
			++currY;
		}
	}
}

void LineMIDPoint(Vertex_2D start, Vertex_2D end)
{
	float deltaX = end.pos.x - start.pos.x;
	float deltaY = end.pos.y - start.pos.y;

	float d = 2 * deltaY - deltaX;
	float increment_Y = 2 * deltaY;
	float increment_XY = 2 * (deltaY - deltaX);

	float x = start.pos.x;
	float y = start.pos.y;

	PlotPixel(x, y, GREEN);

	while (x < end.pos.x)
	{
		if (d <= 0)
		{
			d += increment_Y;
			x++;
		}
		else
		{
			d += increment_XY;
			x++;
			y++;
		}

		PlotPixel(x, y, GREEN);
	}

}

void Bresenham(Vertex_2D start, Vertex_2D end)
{
	float rise = (float)start.pos.y - (float)end.pos.y;
	float run = (float)start.pos.x - (float)end.pos.x;

	//straight line 
	if (start.pos.x == end.pos.x)
	{
		if (end.pos.y < start.pos.y)
		{
			int endY = end.pos.y;
			int startY = start.pos.y;

			BUTTERFLYSWAP(endY, startY);
		}

		for (unsigned int y = start.pos.y; y < end.pos.y; y++)
		{
			PlotPixel(start.pos.x, y, RED);
		}
	}

	else
	{
		float slope = rise / run;
		float B_intercept = start.pos.y - (slope * start.pos.x);

		if (-1 <= slope && slope <= 1)
		{
			if (end.pos.x < start.pos.x)
			{
				int endX = end.pos.x;
				int startX = start.pos.x;

				BUTTERFLYSWAP(endX, startX);
			}

			//looping over the x
			for (unsigned int x = start.pos.x; x <= end.pos.x; x++)
			{
				//used slope intercept formula
				float y = (float)((slope * (float)x) + B_intercept);

				//called setpixel to make the line
				PlotPixel(x, y, RED);
			}
		}


		else if (-1 > slope || slope > 1)
		{

			if (end.pos.y < start.pos.y)
			{
				int endY = end.pos.y;
				int startY = start.pos.y;
				BUTTERFLYSWAP(endY, startY);
			}

			//looping over the y
			for (unsigned int y = start.pos.y; y < end.pos.y; y++)
			{
				//used slope intercept formula
				float x = (float)((y - B_intercept) / slope);

				//called setpixel to make the line
				PlotPixel(x, y, RED);
			}

		}
	}


}

void ParametricLineDraw2D(Vertex_2D start, Vertex_2D end)
{
	float ratio = 0;
	float deltaX = end.pos.x - start.pos.x;
	float deltaY = end.pos.y - start.pos.y;

	int StartX = start.pos.x, EndX = end.pos.x, StartY = start.pos.y, EndY = end.pos.y;

	int xDir = deltaX > 0 ? 1 : -1;
	xDir = deltaX == 0 ? 0 : xDir;
	int yDir = deltaY > 0 ? 1 : -1;
	yDir = deltaY == 0 ? 0 : yDir;

	while (StartX != EndX || StartY != EndY)
	{
		if (deltaX == 0)
		{
			ratio = float(StartY - start.pos.y) / deltaY;
			PlotPixel(StartX, StartY, LerpColor(start.clr, end.clr, ratio)); //lerp color
			StartY += yDir;
		}

		else
		{
			if (abs(deltaY) > abs(deltaX))
			{
				ratio = float(StartY - start.pos.y) / deltaY;
				StartX = (Lerpf(start.pos.x, end.pos.x, ratio));
				if (StartX == EndX && StartY == EndY)
				{
					break;
				}
				PlotPixel(StartX, StartY, LerpColor(start.clr, end.clr, ratio)); // lerp color
				StartY += yDir;
			}

			else
			{
				ratio = float(StartX - start.pos.x) / deltaX;
				StartY = Lerpf(start.pos.y, end.pos.y, ratio);
				if (StartX == EndX && StartY == EndY)
				{
					break;
				}

				PlotPixel(StartX, StartY, LerpColor(start.clr, end.clr, ratio)); //lerp color
				StartX += xDir;
			}
		}

	}

	//p1.color = COLORS::YELLOW;
	//p2.color = COLORS::YELLOW;
	//PlotPoint(p1);
	//PlotPoint(p2);
}


unsigned int Raster[NUM_PIXELS];

void PlotPixel(int x, int y, unsigned int color)
{
	assert(x >= 0 && x < RASTER_WIDTH);
	assert(x >= 0 && x < RASTER_HEIGHT);

	Raster[y * RASTER_WIDTH + x] = color;
}

int main()
{
	RS_Initialize(RASTER_WIDTH, RASTER_HEIGHT);

	Vertex_2D line[6] =
	{
		{{0, 100}, RED},
		{{499, 400}, RED},
		{{0, 90}, White},
		{{499, 390}, White},
		{{0, 110}, MAGENTA},
		{{499, 410}, CYAN}
	};


	for (size_t i = 0; i < 2000; i++)
	{
		PlotPixel(rand() % 500, rand() % 500, White);
	}

	while (RS_Update(Raster, NUM_PIXELS))
	{
		//draw stuff
		//PlotPixe(250, 250, 0x3D69B8);
		/*DrawMidPointLine(line[1], line[0]);
		PlotPixe(254, 151, 0xFFFFFFFF);
		PlotPixe(52, 51, 0xFFFFFFFF);*/	
		Bresenham(line[0], line[1]);
		LineMIDPoint(line[2], line[3]);
		ParametricLineDraw2D(line[4], line[5]);

		/*if ()
		{

		}*/

		//P1(358, 267), P2(430, 126)
		//lineMidPoint(0, 90, 499, 390);
		//DrawMidPointLine(line[3], line[2]);
	}

	RS_Shutdown();

	return 0;
}