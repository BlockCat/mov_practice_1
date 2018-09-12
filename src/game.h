#pragma once

// Define windows definitions for linux environment
#ifndef _WIN32
#define GetRValue(rgb)   ((BYTE) (rgb))
#define GetGValue(rgb)   ((BYTE) (((WORD) (rgb)) >> 8))
#define GetBValue(rgb)   ((BYTE) ((rgb) >> 16))
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef DWORD COLORREF;
#define RGB(r,g,b)      ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

namespace Tmpl8 {

class Game
{
private:
	Surface* screen;
public:
	void SetTarget( Surface* _Surface ) { screen = _Surface; }
	void Init();
	int Evaluate();
	void Shutdown();
	void Tick( float _DT );
	void MouseUp( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int _X, int _Y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int _Key ) { /* implement if you want to handle keys */ }
	void KeyDown( int _Key ) { /* implement if you want to handle keys */ }
};

}; // namespace Tmpl8