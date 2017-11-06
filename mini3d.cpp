//
// Created by Shunming GUO on 2017/11/3.
// Copyright (c) 2017 ___FULLUSERNAME___. All rights reserved.
//

#include "mini3d.h"

using namespace mini3d;
Object3D::Object3D() {
    _textrue = nullptr;

}

Object3D::~Object3D() {
    delete[] _textrue;
}

void Scene::init() {

    obj.points.push_back({0,0,0});
    obj.points.push_back({1,0,0});
    obj.points.push_back({1,1,0});
    obj.points.push_back({0,1,0});

    obj.points.push_back({0,0,1});
    obj.points.push_back({1,0,1});
    obj.points.push_back({1,1,1});
    obj.points.push_back({0,1,1});

    for (int i = 0; i < 8; ++i) {
        obj.colors.push_back(interp(0,0xffffff,i/8));
    }

    //按照定点索引，制造三角形面
    //上面
    obj.faces.push_back({0,1,3});
    obj.faces.push_back({1,2,3});

    //下面
    obj.faces.push_back({4,5,7});
    obj.faces.push_back({5,6,7});

    //左面
    obj.faces.push_back({0,4,3});
    obj.faces.push_back({4,7,3});

    //右面
    obj.faces.push_back({5,1,6});
    obj.faces.push_back({6,1,2});

    //前面
    obj.faces.push_back({0,1,4});
    obj.faces.push_back({4,1,5});

    //后面
    obj.faces.push_back({2,3,7});
    obj.faces.push_back({2,7,6});


}

void Render::rending(Scene &scene, Camera &camera) {
    auto obj = scene.obj;

    auto transform = scene.worldMatrix*camera.getMatrix();

    //生成各个面
    for (auto face : obj.faces) {
        vector4 p1 = obj.points[face.index[0]]
        ,p2 = obj.points[face.index[1]]
        ,p3 = obj.points[face.index[2]];

        p1 = p1 * transform;
        p2 = p2 * transform;
        p3 = p3 * transform;

        if(check_cvv(p1) == 0) return;
        if(check_cvv(p2) == 0) return;
        if(check_cvv(p3) == 0) return;

        //归一化

        auto p2D_1 = p1.normalize();
        auto p2D_2 = p2.normalize();
        auto p2D_3 = p3.normalize();


        if(_state == wireframeRender) {
            //直接画线
            drawline(p2D_1, p2D_2);
            drawline(p2D_1, p2D_3);
            drawline(p2D_3, p2D_2);
        }
        else
        {
            //生成顶
            Triangle triangle;
            triangle.v[0].set(obj, face.index[0],p2D_1,p1.w);
            triangle.v[1].set(obj, face.index[1],p2D_2,p2.w);
            triangle.v[2].set(obj, face.index[2],p2D_3,p3.w);
            //生成二维（屏幕）三角形
            //分解三角形
            auto triangleV = triangle.makeTwo();

            //渲染三角形
            if(triangleV.size()>=1) drawTriangle(triangleV[0]);
            if(triangleV.size()>=2) drawTriangle(triangleV[1]);
        }
    }

}

void Render::setPixel(int x, int y, float w, Color color) {
    w = 1/w;
    auto& zbuffer = Zbuffer[x+y*width];
    if(x>=0 && x<width && y>=0 && y<height && zbuffer > w)
    {
        zbuffer = w;
        frameBuffer[x+y*width] = color;
    }
}

void Render::drawline(vector4 be, vector4 ed) {
    auto sub = ed - be;
    float dxChange = sub.y == 0?0:sub.x/sub.y;
    float dxSum = 0;
    int curX = (int)(be.x+0.5);

    for (int y = (int)(be.y+0.5); y <= ed.y; ++y) {
        dxSum+= dxChange;
        if(dxSum < 1)
        {
            setPixel(curX, y, 1, _lineColor);
        }
        else
        {
            while(dxSum>=1)
            {
                setPixel(curX,y,1,_lineColor);
                --dxSum;
                ++curX;
            }
        }
    }
}

void Render::drawTriangle(const Triangle &t) {

}

void vertex::normalizeSelf() {
    pos.w = w;
    pos.normalizeSelf();
    UV.w = w;
    UV.normalizeSelf();
    color.color /= w;
}

vertex vertex::sub(const vertex & rhs)const {
    vertex ret;
    ret.pos = pos - rhs.pos;
    ret.UV = UV - rhs.UV;
    ret.w = w - rhs.w;
    ret.color.color = color.color - rhs.color.color;
    return std::move(ret);
}

vertex vertex::interp(const vertex& rhs, float t) {
    vertex ret;
    ret.pos = pos.interp(rhs.pos, t);
    ret.UV = UV.interp(rhs.UV, t);
    ret.w = mini3d::interp(w,rhs.w, t);
    ret.color.color = mini3d::interp(color.color,rhs.color.color,t);
    return std::move(ret);
}

vertex vertex::add(const vertex &rhs) const {
    vertex ret;
    ret.pos = pos + rhs.pos;
    ret.UV = UV + rhs.UV;
    ret.w = w + rhs.w;
    ret.color.color = color.color + rhs.color.color;
    return std::move(ret);
}

LineScaner::LineScaner(const vertex &v1, const vertex &v2, int stepSize) {
    stepValue = v2.sub(v1);
    stepValue.pos = stepValue.pos / stepSize;
    stepValue.UV = stepValue.UV / stepSize;
    stepValue.color /= stepSize;
    stepValue.w  /= stepSize;
}

bool LineScaner::step(vertex &v, int num) const{
    for (int i = 0; i < num; ++i) {
        v = v.add(stepValue);
    }
    return num>0;
}

void Triangle::computeTopBotton() {
    sortVectex();
    top = v[0].pos.y;
    bottom = v[2].pos.y;
}

void Triangle::sortVectex() {
    if(v[0].pos.y <v[1].pos.y) std::swap(v[0], v[1]);
    if(v[0].pos.y <v[2].pos.y) std::swap(v[0], v[2]);
    if(v[1].pos.y <v[2].pos.y) std::swap(v[1], v[2]);

	if (v[0].pos.y == v[1].pos.y && v[0].pos.x > v[1].pos.x) std::swap(v[0], v[1]);
	if (v[1].pos.y == v[2].pos.y && v[1].pos.x > v[2].pos.x) std::swap(v[1], v[2]);
}

std::vector<Triangle> Triangle::makeTwo() {
	std::vector<Triangle> ret;
	ret.reserve(2);
	sortVectex();

	//三个点在同一掉线，则无效
	if (v[0].pos.y == v[1].pos.y && v[0].pos.y == v[2].pos.y) return std::move(ret);
	if (v[0].pos.x == v[1].pos.x && v[0].pos.x == v[2].pos.x) return std::move(ret);

	//平底三角直接返回自己
	if (v[0].pos.y == v[1].pos.y || v[1].pos.y == v[2].pos.y)
	{
		ret.push_back(*this);
		return std::move(ret);
	}

	//拆分三角

	//中点在线段的左边
	auto d1 = v[0].pos - v[2].pos;
	auto d2 = v[1].pos - v[2].pos;

	auto k1 = d1.x / d1.y;
	auto k2 = d2.x / d2.y;

	auto dy01 = (v[0].pos.y - v[1].pos.y);
	auto dy12 = (v[1].pos.y - v[2].pos.y);
	auto dy02 = (v[0].pos.y - v[2].pos.y);

	vertex middle = v[0].interp(v[2], dy01 / dy02);

	if (k1 == k2)
	{
		return std::move(ret);
	}
	if (k1>k2)
	{
		Triangle t1, t2;
		t1.v[0] = v[0];
		t1.v[1] = v[1];
		t1.v[2] = middle;

		t2.v[0] = v[1];
		t2.v[1] = middle;
		t2.v[2] = v[2];

		ret.push_back(t1);
		ret.push_back(t2);
	}
	else
	{
		Triangle t1, t2;
		t1.v[0] = v[0];
		t1.v[1] = middle;
		t1.v[2] = v[1];

		t2.v[0] = middle;
		t2.v[1] = v[1];
		t2.v[2] = v[2];

		ret.push_back(t1);
		ret.push_back(t2);
	}

    return std::move(ret);
}

bool Triangle::isFlat() {
    return false;
}
