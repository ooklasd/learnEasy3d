//
// Created by Shunming GUO on 2017/11/3.
// Copyright (c) 2017 ___FULLUSERNAME___. All rights reserved.
//

#include "mini3d.h"
#include <cmath>
#include <cassert>
#include<Windows.h>
#include<windef.h>
#include<memory>


using namespace mini3d;


int Device::screen_exit = 0;
int Device::screen_mx = 0;
int Device::screen_my = 0;
int Device::screen_mb = 0;
int Device::screen_keys[] = {};
HWND Device::screen_handle = 0;
HDC Device::screen_dc = 0;
HBITMAP Device::screen_hb = 0;
HBITMAP Device::screen_ob = 0;


int CMID(int x, int min, int max) {
    return (x < min)? min : ((x > max)? max : x);
}


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

	//UV
	obj.uv.push_back({ 0,0,0 });
	obj.uv.push_back({ 1,0,0 });
	obj.uv.push_back({ 1,1,0 });
	obj.uv.push_back({ 0,1,0 });

	obj.uv.push_back({ 1,1,0 });
	obj.uv.push_back({ 2,1,0 });
	obj.uv.push_back({ 2,2,0 });
	obj.uv.push_back({ 1,2,0 });

    for (int i = 0; i < 8; ++i) {
        obj.colors.push_back(interp(0,0xffffff,i/8.0));
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

	auto& m = worldMatrix.m;

	


	//Z方向移动0.5
	//m[3][2] = 0.5;
}

void mini3d::Render::preRending()
{
	ClearFrame(_bkColor);
	device->dispatch();

	
}

void Render::rending(Scene &scene, Camera &camera) {

	camera.initMatrix();

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

        if(check_cvv(p1) == 0) continue;
        if(check_cvv(p2) == 0) continue;
        if(check_cvv(p3) == 0) continue;

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

            //分解三角形
            auto triangleV = triangle.makeTwo();

			for (auto it = triangleV.begin(); it < triangleV.end(); it++)
			{
				it->sortVectex();
				it->computeTopBottom();
			}

            //渲染三角形
            if(triangleV.size()>=1) drawTriangle(triangleV[0]);
            if(triangleV.size()>=2) drawTriangle(triangleV[1]);
        }
    }

	FrameToWindow();

}

void Render::setPixel(int x, int y, float w, const Color& color) {
	if (x < 0 || x >= width) return;
	if (y < 0 || y >= height) return;

    w = 1/w;
    auto& zbuffer = Zbuffer[x+y*width];
    if(x>=0 && x<width && y>=0 && y<height && zbuffer <= w)
    {
        zbuffer = w;
        frameBuffer[x+y*width] = color;
    }
}

void Render::drawline(vector4 be, vector4 ed) {

	if (be.y > ed.y)
	{
		std::swap(be, ed);
	}
    auto sub = ed - be;
    float dxChange = fabs(sub.y) < 0.0001? sub.x :sub.x/sub.y;
    float dxSum = 0;
    int curX = (int)(be.x+0.5);
	ed.y += 0.5;
    for (int y = (int)(be.y+0.5); y <= ed.y; ++y) {

        dxSum+= dxChange;
        if(-1< dxSum && dxSum < 1)
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

			while (dxSum<=-1)
			{
				setPixel(curX, y, 1, _lineColor);
				++dxSum;
				--curX;
			}
        }
    }
}

void Render::drawTriangle(const Triangle &t) {

    int top = (int)(t.top+0.5);
    int bottom = (int)(t.bottom+0.5);
	if (top == bottom) return;

    //平底三角形转化为平行梯形
    vertex topV[2],bottomV[2];
    if(t.v[0].pos.y == t.v[1].pos.y)
    {
        topV[0] = t.v[0];
        topV[1] = t.v[1];
        bottomV[0] = bottomV[1] = t.v[2];
    }
    else
    {
        topV[0] = topV[1] = t.v[0];
        bottomV[0] = t.v[1];
        bottomV[1] = t.v[2];
    }

    assert(topV[0].pos.y == topV[1].pos.y);
	assert(bottomV[0].pos.y == bottomV[1].pos.y);
	assert(top < bottom);

    //中间向两边
    for (int y = top; y <= bottom; ++y) {
        if(y>=0 && y<height)
        {
            //边缘插值
			float interp = (y - top)*1.0 / (bottom - top);
            auto leftEdge = topV[0].interp(bottomV[0], interp);
            auto rightEdge = topV[1].interp(bottomV[1], interp);

            //直线描写
            drawline(leftEdge,rightEdge);
        }
    }
}

void Render::drawline(const vertex& be, const vertex& ed) {
    auto fb = frameBuffer + (int)(be.pos.y+0.5)*width;
    auto zb = Zbuffer + (int)(be.pos.y+0.5)*width;

    int beginx = (int)(be.pos.x+0.5);
    int endx = (int)(ed.pos.x+0.5);
    LineScaner ls(be,ed,endx - beginx);
    vertex curPoint = be;
    for (int x = beginx; x <= endx; ++x) {
        if(x>=0 && x<width)
        {
            //描写当前点的颜色
            float w = 1/curPoint.w;
            if(zb[x] < w)
            {
                zb[x] = w;
                auto p = curPoint.normalize();
                fb[x] = p.color;
            }
        }
        ls.step(curPoint);
    }
}

void Render::ClearFrame(Color c)
{
	memset(frameBuffer, c, width*height * sizeof(UINT));
	for (size_t y = 0; y < height; y++)
	{
		int offset = y*width;
		for (size_t x = 0; x < width; x++)
		{
			Zbuffer[offset + x] = 0;
		}
	}
}

void mini3d::Render::CreateDevice()
{
	device = std::shared_ptr<Device>(new Device());
	device->create(width, height);
	frameBuffer = (UINT*)device->screen_fb;
}


void Render::FrameToWindow()
{
	if (device) device->frame();
}

UINT Render::check_cvv(const vector4 &v) {
	float w = v.w;
	int check = 0;
	if (v.z < 0.0f) check |= 1;
	if (v.z >  w) check |= 2;
	if (v.x < -w) check |= 4;
	if (v.x >  w) check |= 8;
	if (v.y < -w) check |= 16;
	if (v.y >  w) check |= 32;
	return check;
}

void vertex::normalizeSelf() {
    pos.w = w;
    pos.normalizeSelf();
    UV.w = w;
    UV.normalizeSelf();
    color /= w;
}

vertex vertex::sub(const vertex & rhs)const {
    vertex ret;
    ret.pos = pos - rhs.pos;
    ret.UV = UV - rhs.UV;
    ret.w = w - rhs.w;
	for (size_t i = 0; i < 4; i++)
	{
		ret.color[i] = color[i] - rhs.color[i];

	}
    return std::move(ret);
}

vertex vertex::interp(const vertex& rhs, float t) {
    vertex ret;
    ret.pos = pos.interp(rhs.pos, t);
    ret.UV = UV.interp(rhs.UV, t);
    ret.w = mini3d::interp(w,rhs.w, t);
    ret.color = color.interp(rhs.color,t);
    return std::move(ret);
}

vertex vertex::add(const vertex &rhs) const {
    vertex ret;
    ret.pos = pos + rhs.pos;
    ret.UV = UV + rhs.UV;
    ret.w = w + rhs.w;
    ret.color = color + rhs.color;
    return std::move(ret);
}

vertex vertex::normalize() const {
    auto temp = *this;
    temp.normalizeSelf();
    return temp;
}

LineScaner::LineScaner(const vertex &v1, const vertex &v2, int stepSize) {
	if (stepSize > 0)
	{
		stepValue = v2.sub(v1);
		stepValue.pos = stepValue.pos / stepSize;
		stepValue.UV = stepValue.UV / stepSize;
		stepValue.color /= stepSize;
		stepValue.w /= stepSize;
	}	
}

bool LineScaner::step(vertex &v, int num) const{
    for (int i = 0; i < num; ++i) {
        v = v.add(stepValue);
    }
    return num>0;
}

void Triangle::computeTopBottom() {
    sortVectex();
	bottom  = v[2].pos.y;
	 top = v[0].pos.y;
}

void Triangle::sortVectex() {
    if(v[0].pos.y >v[1].pos.y) std::swap(v[0], v[1]);
    if(v[0].pos.y >v[2].pos.y) std::swap(v[0], v[2]);
    if(v[1].pos.y >v[2].pos.y) std::swap(v[1], v[2]);

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

	//中间点的Y相等
	middle.pos.y = v[1].pos.y;

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

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

UINT mini3d::Device::create(int width, int height, TCHAR title )
{
	auto w = width;
	auto h = height;
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)(&mini3d::Device::events), 0, 0, 0,
		NULL, NULL, NULL, NULL, ("SCREEN") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		(DWORD)(w * h * 4), 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(("SCREEN"), 0,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

UINT mini3d::Device::close()
{
	if (screen_ob)
	{
		::SelectObject(screen_dc, screen_ob);

	}
	return 0;
}

UINT mini3d::Device::frame(Color * frameBuffer)
{
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	return 0;
}

void mini3d::Device::dispatch(void)
{
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

LRESULT mini3d::Device::events(HWND hWnd, UINT msg,WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;;
}


void mini3d::PerspectiveCamera::setLockAt(const vector4 & eye, const vector4 & at, const vector4 & up)
{
	this->eye = eye;
	this->at = at;
	this->up = up;
}
