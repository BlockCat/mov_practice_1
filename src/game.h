#pragma once

#define SCRWIDTH	 800
#define SCRHEIGHT	 600

namespace Tmpl8 {

class Surface;

class Game
{
public:
	void SetTarget( Surface* _Surface ) { screen = _Surface; }
	void Init();
	void Shutdown() {} /* implement if you want code to be executed upon app exit */
	void HandleInput(float dt) {}
	void Tick( float dt );
	void MouseUp( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int _X, int _Y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int a_Key ) { /* implement if you want to handle keys */ }
	void KeyDown( int a_Key ) { /* implement if you want to handle keys */ }
private:
	Surface* screen;
	int mx, my;
};

}; // namespace Tmpl8
