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

    camera.setPosition({0, 0, 0.5});

	while (render.isRending())
	{
		render.preRending();
		render.rending(scene, camera);
		
		Sleep(10);
	}

    cout<<"完毕"<<endl;
}

