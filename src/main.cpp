#include <RayFlow/Engine/engine.h>

#include <iostream>

int main()
{
	rayflow::RayFlowEngine engine;
	engine.Init("C:/FlowSource/code/FlowLab/RayFlow/resources/cornell-box-monster/cornell-box-disneydiffuse.xml");
	engine.Render();
	return 0;
}
