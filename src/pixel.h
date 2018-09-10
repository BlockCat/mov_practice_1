#pragma once

namespace Tmpl8 {

#define REDMASK	(0xff0000)
#define GREENMASK (0x00ff00)
#define BLUEMASK (0x0000ff)
#define GREY(c) ((c)+((c)<<8)+((c)<<16))

typedef unsigned int Pixel;

inline Pixel Color(unsigned char r, unsigned char g, unsigned char b)
{
	Pixel x = b;
	x |= (g << 8);
	x |= (r << 16);
	return x;
}

// additive blending
inline Pixel AddBlend( Pixel a_Color1, Pixel a_Color2 )
{
	const unsigned int r = (a_Color1 & REDMASK) + (a_Color2 & REDMASK);
	const unsigned int g = (a_Color1 & GREENMASK) + (a_Color2 & GREENMASK);
	const unsigned int b = (a_Color1 & BLUEMASK) + (a_Color2 & BLUEMASK);
	const unsigned r1 = (r & REDMASK) | (REDMASK * (r >> 24));
	const unsigned g1 = (g & GREENMASK) | (GREENMASK * (g >> 16));
	const unsigned b1 = (b & BLUEMASK) | (BLUEMASK * (b >> 8));
	return (r1 + g1 + b1);
}

// subtractive blending
inline Pixel SubBlend( Pixel a_Color1, Pixel a_Color2 )
{
	int red = (a_Color1 & REDMASK) - (a_Color2 & REDMASK);
	int green = (a_Color1 & GREENMASK) - (a_Color2 & GREENMASK);
	int blue = (a_Color1 & BLUEMASK) - (a_Color2 & BLUEMASK);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return (Pixel)(red + green + blue);
}

inline Pixel ScaleColor( Pixel a_Color, unsigned int a_Scale )
{
	unsigned int rb = (((a_Color & (REDMASK|BLUEMASK)) * a_Scale) >> 8) & (REDMASK|BLUEMASK);
	unsigned int g = (((a_Color & GREENMASK) * a_Scale) >> 8) & GREENMASK;
	return rb + g;
}

};