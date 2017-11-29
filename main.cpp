//
//  main.cpp
//  Remove Duplicate Letters
//
//  Created by Shunming GUO on 2017/10/10.
//  Copyright © 2017 Shunming GUO. All rights reserved.
//
#include<Windows.h>
#include <iostream>
#include <string>
#include<corecrt_math_defines.h>
#include<cmath>
#include "mini3d.h"
using namespace std;
using namespace mini3d;


int main(int argc, const char * argv[]) {
    int screenWidth = 800,screenHeight = 500;
    Scene scene;
    scene.init();
    PerspectiveCamera camera(screenWidth,screenHeight,M_PI_2,10,500);
    Render render(camera.width,camera.height);

    camera.setPosition({-0.6f, 0.3f,1.3f});
	render._state = Render::colorRender;
	while (render.isRending())
	{
		//响应按钮
		auto& key = render.device->screen_keys;		
		if (key['w'] || key['W'])
		{
			render._state = (Render::RENDER_STATE)((int)render._state + 1);
			if (render._state > 2) render._state = Render::wireframeRender;
			key['w'] = key['W'] = 0;
		}

		if (key[VK_LEFT])
		{
			camera.position.x -= 0.05;
			key[VK_LEFT] = 0;
		}

		if (key[VK_RIGHT])
		{
			camera.position.x += 0.05;
			key[VK_RIGHT] = 0;
		}

		if (key[VK_DOWN])
		{
			camera.position.y += 0.05;
			key[VK_DOWN] = 0;
		}
		if (key[VK_UP])
		{
			camera.position.y -= 0.05;
			key[VK_UP] = 0;
		}

		if (key['q'] || key['Q'])
		{
			camera.position.z += 0.05;
			key['q'] =  key['Q'] = 0;
		}
		if (key['e'] || key['E'])
		{
			camera.position.z -= 0.05;
			key['e'] = key['E'] = 0;
		}

		if (key[VK_ESCAPE]) break;

		//camera.setLockAt(camera.position, { 2,2,2 }, { 0,1,0 });

		render.preRending();
		render.rending(scene, camera);
		Sleep(10);
	}

    cout<<"完毕"<<endl;
}

