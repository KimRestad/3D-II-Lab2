#include "D3DApplication.h"
#include "Game.h"

// ----------------------------------------------------- METHODS ------------------------------------------------------ //
// The main function, where it all starts. Initializes the window and runs the function
int WINAPI WinMain(HINSTANCE applicationInstance, HINSTANCE prevInstance, PSTR cmdLineArgs, int showSetting)
{
	Game game(applicationInstance);
	return game.Run();
}
