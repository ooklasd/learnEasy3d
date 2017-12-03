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
    PerspectiveCamera camera(screenWidth,screenHeight,M_PI_2,0.2,50);
    Render render(camera.width,camera.height);

	//-1错误表现
    camera.setPosition({0.0f, -0.50f,0});
	render._state = Render::RENDER_STATE::textureRender;
	render._lineColor = 0xff00ff;

	//Y轴旋转
	float angleY = -PI_2;
	float rotateSpeed = 0;
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
		if (key[VK_SPACE])
		{
			rotateSpeed -=1;
			if (rotateSpeed <0) rotateSpeed = 4;
			key[VK_SPACE] = 0;
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

		camera.position.x = cos(angleY) * 1.5 + 0.5;
		camera.position.z = sin(angleY) * 1.5 + 0.5;
		camera.setLockAt(camera.position, { 0.5,0,0.5 }, { 0,1,0 });
		
		angleY += 0.005*rotateSpeed;

		render.preRending();
		render.rending(scene, camera);
		Sleep(16);
	}

    std::cout<<"完毕"<<endl;
}

