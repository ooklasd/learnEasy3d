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

	amberLight = Color(0.1,0.1,0.1);

	//在面0123前面
	pointLight.push_back({ vector4({ 0.5f,0.5f,-0.1f }) ,1.0f });

	pointLight.push_back({ vector4({ -0.2f,1.1f,1.1f }) ,1.0f });
	pointLight.push_back({ vector4({ 0.5f,1.1f,1.1f}),1.0f });
	pointLight.push_back({ vector4({ 1.2f,1.1f,1.1f }) ,1.0f });

	


    obj.points.push_back({0,0,0});
    obj.points.push_back({1,0,0});
    obj.points.push_back({1,1,0});
    obj.points.push_back({0,1,0});

    obj.points.push_back({0,0,1});
    obj.points.push_back({1,0,1});
    obj.points.push_back({1,1,1});
    obj.points.push_back({0,1,1});

	
	//色彩
	obj.colors.push_back({ 0,0,0 });
	obj.colors.push_back({ 0,0,1 });
	obj.colors.push_back({ 0,1,0 });
	obj.colors.push_back({ 0,1,1 });
	obj.colors.push_back({ 1,0,0 });
	obj.colors.push_back({ 1,0,1 });
	obj.colors.push_back({ 1,1,1 });
	obj.colors.push_back({ 0,0,0 });
    

    //按照定点索引，制造三角形面
    //前面
    obj.faces.push_back({0,1,3});
    obj.faces.push_back({1,2,3});

    //后面
    obj.faces.push_back({4,7,5 });
    obj.faces.push_back({7,6,5 });

    //左面
    obj.faces.push_back({4,0,7});
    obj.faces.push_back({0,3,7});

    //右面
    obj.faces.push_back({1,5,2});
    obj.faces.push_back({5,6,2});

    //下面
    obj.faces.push_back({0,4,1});
    obj.faces.push_back({4,5,1});

    //上面
    obj.faces.push_back({3,2,7});
    obj.faces.push_back({2,6,7});

	for (size_t i = 0; i < 6; i++)
	{
		obj.uv.push_back({ 0,0,0 });
		obj.uv.push_back({ 5,0,0 });
		obj.uv.push_back({ 0,5,0 });
		obj.uv.push_back({ 5,0,0 });
		obj.uv.push_back({ 5,5,0 });
		obj.uv.push_back({ 0,5,0 });
	}

	for (auto it = obj.faces.begin(); it < obj.faces.end(); it++)
	{
		auto& findex = it->index;
		auto& p = obj.points;

		obj.faceNormal.push_back((p[findex[1]]-p[findex[0]])^(p[findex[2]] - p[findex[0]]));
	}





	//生成材质
	obj._textrue = new Color[64 * 64];
	Color c1(0, 162.0 / 255, 232.0 / 255);
	Color c2(206.0 / 255, 240.0 / 255, 255.0 / 255);

	obj._tsize = 64;

	for (size_t y = 0; y < 32; y++)
	{
		for (size_t x = 0; x < 32; x++)
		{
			//蓝色左上
			obj._textrue[y * 64 + x] = c1;

			//蓝色右下		
			obj._textrue[(y+32) * 64 + x+32] = c1;

			//白色右上
			obj._textrue[y * 64 + x + 32] = c2;

			//白色左下
			obj._textrue[(y + 32) * 64 + x] = c2;
		}
	}
}

void mini3d::Render::preRending()
{
	ClearFrame(_bkColor);
	device->dispatch();
	lights.clear();

	switch (_state)
	{
	case mini3d::Render::wireframeRender:
		setPixelFunc = nullptr;
		break;
	case mini3d::Render::colorRender:
		setPixelFunc = &Render::setColorPixel;
		break;
	case mini3d::Render::textureRender:
		setPixelFunc = &Render::setUVPixel;
		break;
	default:
		break;
	}
}

void Render::rending(Scene &scene, Camera &camera) {
	this->scene = &scene;
	this->camera = &camera;

	camera.initMatrix();

    auto& obj = scene.obj;

	//所有的变换矩阵
	auto masterTransform = scene.worldMatrix*camera.getMatrix();

	//平移和旋转
	auto transform = scene.worldMatrix*camera.getTransfrom();
	auto rotateM = scene.worldMatrix*camera.getRotate();

	//灯光变换
	lights = scene.pointLight;	

    //生成各个面
	for (size_t iface = 0; iface < obj.faces.size(); iface++)
	{
		auto& face = obj.faces[iface];
		auto p1 = obj.points[face.index[0]];
		auto p2 = obj.points[face.index[1]];
		auto p3 = obj.points[face.index[2]];
		
		//场景归一化到 xyz[-1,1]
        vector4 p3D_1 = p1 * masterTransform;
        vector4 p3D_2 = p2 * masterTransform;
        vector4 p3D_3 = p3 * masterTransform;
		vector4 faceNormal = (obj.faceNormal[iface] * rotateM);
		faceNormal.normalizeSelfW();
		if (faceNormal.dot({ 0,0,1 }) < 0) continue;

        if(check_cvv(p3D_1) != 0) continue;
        if(check_cvv(p3D_2) != 0) continue;
        if(check_cvv(p3D_3) != 0) continue;

		//视口变换
		auto p2D_1 = p3D_1 * camera.getMatrixViewProt();
		auto p2D_2 = p3D_2 * camera.getMatrixViewProt();
		auto p2D_3 = p3D_3 * camera.getMatrixViewProt();

        //w归一化

        p2D_1 = p2D_1.normalizeW();
		p2D_2 = p2D_2.normalizeW();
		p2D_3 = p2D_3.normalizeW();


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
            triangle.v[0].set(obj, face.index[0],p2D_1, p1,p3D_1.w,obj.uv[iface*3+0], obj.faceNormal[iface]);
            triangle.v[1].set(obj, face.index[1],p2D_2, p2,p3D_2.w,obj.uv[iface*3+1], obj.faceNormal[iface]);
            triangle.v[2].set(obj, face.index[2],p2D_3, p3,p3D_3.w,obj.uv[iface*3+2], obj.faceNormal[iface]);

            //分解三角形
            auto triangleV = triangle.makeTwo();

			for (auto it = triangleV.begin(); it < triangleV.end(); it++)
			{
				it->sortVectex();
				it->computeTopBottom();

				////三角形分解测试
				//drawline(it->v[0].pos, it->v[1].pos);
				//drawline(it->v[0].pos, it->v[2].pos);
				//drawline(it->v[1].pos, it->v[2].pos);
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

void mini3d::Render::setUVPixel(Color & pixel, const vertex & v)
{
	auto& obj = *v.obj;
	auto& UV = v.UV;
	float U = fmod(UV.x, 1.0f);
	float V = fmod(UV.y, 1.0f);
	UINT offset = interp(0, obj._tsize, U)*obj._tsize;
	offset += interp(0, obj._tsize, V);
	pixel.set(obj._textrue[offset]);
}

void mini3d::Render::setLightPixel(Color & pixel, const vertex & v)
{
	//添加光源
	float PhongLight = 0;
	float AmbientLight = 0;
	float LambertLight = 0;
	for each (auto& light in lights)
	{
		//Lambert材质属性
		auto L = ( v.pos3D- light.position).normalize();
		auto N = v.normal;
		float lightV = N.dot(L);
		LambertLight += (lightV >= 0 ? 0.1 : 0);

		//Phong材质属性
		auto reflection = L - N * 2 * (L.dot(N));
		auto eye = camera->getPosition() - v.pos3D;		
		reflection.normalizeSelf();
		eye.normalizeSelf();

		auto reflectionValue = reflection.dot(eye);
		reflectionValue = max(reflectionValue, 0);
		reflectionValue = pow(reflectionValue, 6);
		PhongLight += reflectionValue*light.power;
	}
	LambertLight = max(LambertLight, 0);
	PhongLight = max(PhongLight, 0);

	//环境光+散射光+Phong高光
	pixel = pixel*scene->amberLight + pixel*LambertLight + Color(1,1,1)* PhongLight;

	for each (auto& v in pixel.value.color)
	{
		v = min(v, 1);
	}
}

void Render::drawline(vector4 be, vector4 ed) {

	if (be.y > ed.y)
	{
		std::swap(be, ed);
	}

	be.x = (int)(be.x + 0.5);
	be.y = (int)(be.y + 0.5);
	ed.x = (int)(ed.x + 0.5);
	ed.y = (int)(ed.y + 0.5);

    auto sub = ed - be;
    float dxChange = fabs(sub.y) < 1? sub.x :sub.x/(sub.y+1);
    float dxSum = 0;
    int curX = be.x;
    for (int y = be.y; y <= ed.y; ++y) {

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

    //平底三角形转化为梯形
    vertex topV[2],bottomV[2];
    if(t.v[0].pos2D.y == t.v[1].pos2D.y)
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

    assert(topV[0].pos2D.y == topV[1].pos2D.y);
	assert(bottomV[0].pos2D.y == bottomV[1].pos2D.y);
	assert(top < bottom);

    //中间向两边
    for (int y = top; y < bottom; ++y) {
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
    auto fb = frameBuffer + (UINT)(be.pos2D.y+0.5)*width;
    auto zb = Zbuffer + (UINT)(be.pos2D.y+0.5)*width;

    int beginx = (int)(be.pos2D.x+0.5);
    int endx = (int)(ed.pos2D.x+0.5);

    LineScaner ls(be,ed,endx - beginx);
    vertex curPoint = be;
    for (int x = beginx; x <= endx; ++x) {
        if(x>=0 && x<width)
        {
            //描写当前点的颜色
            float w = curPoint.rw;
            if(zb[x] < w)
            {
                zb[x] = w;
                auto p = curPoint.normalize();
				if (setPixelFunc != nullptr)
				{
					Color color;
					(this->*setPixelFunc)(color, p);
					setLightPixel(color, p);
					fb[x] = color;
				}
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
	/*pos = pos.scale(rw);
    pos.w = 1;*/
	auto w = 1.0/rw;
	UV.x *= w;
	UV.y *= w;

	color *= w;
}

vertex vertex::sub(const vertex & rhs)const {
	vertex ret = *this;
	ret.pos2D = pos2D - rhs.pos2D;
	ret.pos3D = pos3D - rhs.pos3D;
	ret.normal = normal - rhs.normal;
    ret.UV = UV - rhs.UV;
    ret.rw = rw - rhs.rw;
	ret.obj = rhs.obj;
	for (size_t i = 0; i < 4; i++)
	{
		ret.color[i] = color[i] - rhs.color[i];

	}
    return std::move(ret);
}

vertex vertex::interp(const vertex& rhs, float t) {
	vertex ret = *this;
	ret.pos2D = pos2D.interp(rhs.pos2D, t);
	ret.pos3D = pos3D.interp(rhs.pos3D, t);
	ret.rw = mini3d::interp(rw, rhs.rw, t);

    ret.UV = UV.interp(rhs.UV, t);
    ret.color = color.interp(rhs.color,t);
	ret.obj = obj;
	ret.normal = normal.interp(rhs.normal, t);

    return std::move(ret);
}

vertex vertex::add(const vertex &rhs) const {
    vertex ret = *this;
	ret.pos2D = pos2D + rhs.pos2D;
	ret.pos3D = pos3D + rhs.pos3D;
	ret.normal = normal + rhs.normal;
	ret.UV = UV + rhs.UV;
    ret.rw = rw + rhs.rw;
    ret.color = color + rhs.color;
	ret.obj = rhs.obj;
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
		stepValue.pos2D = stepValue.pos2D / stepSize;
		stepValue.pos3D = stepValue.pos3D / stepSize;
		stepValue.UV = stepValue.UV / stepSize;
		stepValue.color /= stepSize;
		stepValue.rw /= stepSize;
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
	bottom  = v[2].pos2D.y;
	 top = v[0].pos2D.y;
}

void Triangle::sortVectex() {
    if(v[0].pos2D.y >v[1].pos2D.y) std::swap(v[0], v[1]);
    if(v[0].pos2D.y >v[2].pos2D.y) std::swap(v[0], v[2]);
    if(v[1].pos2D.y >v[2].pos2D.y) std::swap(v[1], v[2]);

	if (v[0].pos2D.y == v[1].pos2D.y && v[0].pos2D.x > v[1].pos2D.x) std::swap(v[0], v[1]);
	if (v[1].pos2D.y == v[2].pos2D.y && v[1].pos2D.x > v[2].pos2D.x) std::swap(v[1], v[2]);
}

std::vector<Triangle> Triangle::makeTwo() {
	std::vector<Triangle> ret;
	ret.reserve(2);
	sortVectex();

	//三个点在同一掉线，则无效
	if (v[0].pos2D.y == v[1].pos2D.y && v[0].pos2D.y == v[2].pos2D.y) return std::move(ret);
	if (v[0].pos2D.x == v[1].pos2D.x && v[0].pos2D.x == v[2].pos2D.x) return std::move(ret);

	//平底三角直接返回自己
	if (v[0].pos2D.y == v[1].pos2D.y || v[1].pos2D.y == v[2].pos2D.y)
	{
		ret.push_back(*this);
		return std::move(ret);
	}

	//拆分三角

	//中点在线段的左边
	auto d1 = v[0].pos2D - v[2].pos2D;
	auto d2 = v[1].pos2D - v[2].pos2D;

	auto k1 = d1.x / d1.y;
	auto k2 = d2.x / d2.y;

	auto dy01 = (v[0].pos2D.y - v[1].pos2D.y);
	auto dy12 = (v[1].pos2D.y - v[2].pos2D.y);
	auto dy02 = (v[0].pos2D.y - v[2].pos2D.y);

	vertex middle = v[0].interp(v[2], dy01 / dy02);

	//中间点的Y相等
	assert(fabs(middle.pos2D.y - v[1].pos2D.y)<0.001);
	middle.pos2D.y  = v[1].pos2D.y;

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


void mini3d::PerspectiveCamera::setLockAt( vector4  eye,  vector4 lookat,  vector4  up)
{
	auto n = lookat - eye;
	n.normalizeSelf();
	up.normalizeSelf();
	auto u = up.cross(n);
	u.normalizeSelf();

	auto v = n.cross(u);
	v.normalizeSelf();

	rotateM.identity();
	auto& m = rotateM.m;
	for (size_t j = 0; j < 3; j++)
	{
		m[j][0] = u[j];
		m[j][1] = v[j];
		m[j][2] = n[j];
	}

	/*m[3][0] = -u.dot(eye);
	m[3][1] = -v.dot(eye);
	m[3][2] = -n.dot(eye);*/

	position = eye;
	initMatrix();
}




const Matrix & mini3d::PerspectiveCamera::getMatrixViewProt() const
{
	return viewProtM;
}
