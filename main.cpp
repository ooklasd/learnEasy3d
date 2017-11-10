//
//  main.cpp
//  Remove Duplicate Letters
//
//  Created by Shunming GUO on 2017/10/10.
//  Copyright © 2017 Shunming GUO. All rights reserved.
//

#include <iostream>
#include <string>
#include "mini3d.h"
using namespace std;
using namespace mini3d;


int main(int argc, const char * argv[]) {
    int screenWidth = 800,screenHeight = 500;
    Scene scene;
    scene.init();
    PerspectiveCamera camera(screenWidth,screenHeight,M_PI_2,10,500);
    Render render(camera.width,camera.height);

    camera.setPosition({0, 30, 50});

    render.rending(scene, camera);

    cout<<"完毕"<<endl;
}

