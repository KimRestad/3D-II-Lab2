#ifndef GLOBALS_H
#define GLOBALS_H

struct MouseInput
{
	bool			leftPressed;
	bool			rightPressed;
	int				x;
	int				y;

	void Update()
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		x = (int)mousePos.x;
		y = (int)mousePos.y;
		
		if(GetAsyncKeyState(VK_LBUTTON))
			leftPressed = true;
		else
			leftPressed = false;

		if(GetAsyncKeyState(VK_RBUTTON))
			rightPressed = true;
		else
			rightPressed = false;

	}
};

#define SafeRelease(x) { if(x) { (x)->Release(); x = NULL; } }

#define SafeDelete(x) { if(x) { delete x; x = NULL; } }

#endif