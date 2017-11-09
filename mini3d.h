//
// Created by Shunming GUO on 2017/11/3.
// Copyright (c) 2017 ___FULLUSERNAME___. All rights reserved.
//

#ifndef REMOVE_DUPLICATE_LETTERS_MINI3D_H
#define REMOVE_DUPLICATE_LETTERS_MINI3D_H

#include <math.h>
#include <vector>

namespace  mini3d
{
    typedef unsigned int UINT;
    int CMID(int x, int min, int max);

    // 计算插值：t 为 [0, 1] 之间的数值
    template <typename T>
    T interp(T x1, T x2, float t) { return x1 + (x2 - x1) * t; }


    template <typename T>
    class vectorX
    {
    public:
        vectorX(){
            x=y=z= 0;
            w= 1;
        }
        vectorX(T x,T y, T z,T w = 1)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }

        vectorX<T> add(const vectorX<T>& rhs)const
        {
            return vectorX<T>(x+rhs.x,y+rhs.y,z+rhs.z,1);
        }

        vectorX<T> sub(const vectorX<T>& rhs)const
        {
            return vectorX<T>(x-rhs.x,y-rhs.y,z-rhs.z,1);
        }

        vectorX<T> scale(float v)const
        {
            return vectorX<T>(x/=v,y/=v,z/=v,1);
        }

        vectorX<T> normalize()const
        {
            return vectorX<T>(x/w,y/w,z/w,1);
        }

        void normalizeSelf()
        {
            x/=w,y/=w,z/=w,w=1;
        }

        T length()const
        {
            float temp = x*x+y*y+z*z;
            return ::sqrt(temp);
        }

        T dot(const vectorX<T>& rhs)const
        {
            return x*rhs.x + y*rhs.y + z*rhs.z;
        }

        vectorX<T> cross(const vectorX<T>& rhs)const
        {
            float m1, m2, m3;
            m1 = y * rhs.z - z * rhs.y;
            m2 = z * rhs.x - x * rhs.z;
            m3 = x * rhs.y - y * rhs.x;
            return vectorX<T>(m1,m2,m3);
        }

        vectorX<T> interp(const vectorX<T>& rhs,T t)const
        {
            vectorX<T> ret;

            ret.x = mini3d::interp(x,rhs.x,t);
            ret.y = mini3d::interp(y,rhs.y,t);
            ret.z = mini3d::interp(z,rhs.z,t);

            return ret;
        }

        vectorX<T> operator -(const vectorX<T>& rhs)const
        {
            return sub(rhs);
        }

        vectorX<T> operator +(const vectorX<T>& rhs)const
        {
            return add(rhs);
        }

        vectorX<T> operator *(const vectorX<T>& rhs)const
        {
            return dot(rhs);
        }
        vectorX<T> operator ^(const vectorX<T>& rhs)const
        {
            return cross(rhs);
        }

		template<typename TValue>
        vectorX<T> operator /(TValue v)const
        {
            return scale(1/v);
        }		

        union
        {
            T x,y,z,w;
            T v[4];
        };
    };
    typedef vectorX<float> vector4;




    template <typename T>
    class MatrixX
    {
    public:
        MatrixX(){
            m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
        }
        MatrixX(T m[]){
            for (int i = 0; i < 16; ++i) {
                this->m[i] = m[i];
            }
        }

        MatrixX<T> add(const MatrixX<T>& rhs)const
        {
            MatrixX<T> ret;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j <4; ++j) {
                    ret.m[i][j] = m[i][j] + rhs.m[i][j];
                }
            }
            return ret;
        }

        MatrixX<T> sub(const MatrixX<T>& rhs)const
        {
            MatrixX<T> ret;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j <4; ++j) {
                    ret.m[i][j] = m[i][j] - rhs.m[i][j];
                }
            }
            return ret;
        }

        MatrixX<T> mul(const MatrixX<T>& rhs)const
        {
            MatrixX<T> ret;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j <4; ++j) {
                    ret.m[i][j] = m[j][0] * rhs.m[0][j]
                            +m[j][1] * rhs.m[1][j]
                            +m[j][2] * rhs.m[2][j]
                            +m[j][3] * rhs.m[3][j];
                }
            }
            return ret;
        }

        MatrixX<T> premul(const MatrixX<T>& rhs)const
        {
            return rhs.mul(*this);
        }

        MatrixX<T> scale(T v)const
        {
            MatrixX<T> ret;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j <4; ++j) {
                    ret.m[i][j] *= v;
                }
            }
            return ret;
        }

        MatrixX<T>& identity(){
            *this = MatrixX<T>();
            return *this;
        }
        MatrixX<T> operator+(const MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        MatrixX<T> operator-(const MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        MatrixX<T> operator*(const MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        T m[4][4] = {};
    };

    typedef MatrixX<float> Matrix;

    template <typename  T>
    vectorX<T> apply(const vectorX<T> v,const MatrixX<T> m)
    {
        vectorX<T> ret;
        auto& mt = m.m;
        ret.x = v.x * mt[0][0]+v.y * mt[0][1]+v.z * mt[0][2]+v.w * mt[0][3];
        ret.y = v.x * mt[1][0]+v.y * mt[1][1]+v.z * mt[1][2]+v.w * mt[1][3];
        ret.z = v.x * mt[2][0]+v.y * mt[2][1]+v.z * mt[2][2]+v.w * mt[2][3];
        ret.w = v.x * mt[3][0]+v.y * mt[3][1]+v.z * mt[3][2]+v.w * mt[3][3];
        return ret;
    }

    template <typename  T>
    vectorX<T> operator*(const vectorX<T> v,const MatrixX<T> m)
    {
        return apply(v, m);
    }




    class Camera {
    public:
        virtual const Matrix& getMatrix()const = 0;
        float znear,  zfar;

    };

    struct Color
    {
        Color(UINT c = 0xffffff){
            value.color = c;
        }
        union {
            unsigned int color;
            struct aRGB{
                char a,r,g,b;
            };
        }value;

        operator UINT ()const{return value.color;}
        operator float ()const{return (float)value.color;}
    };

    //透视相机
    class PerspectiveCamera:public Camera
    {
    public:
        PerspectiveCamera(float width, float height, float angle, float znear, float zfar) {
            this->width = width;
            this->height = height;
            this->angle = angle;
            this->znear = znear;
            this->zfar = zfar;
        }

        const Matrix &getMatrix() const override {
            return perspectiveMatrix;
        }

        float width, height,  angle;
        vector4 look,up,right;
        Matrix perspectiveMatrix;
    };


    //3d对象，为一个box
    class Object3D
    {
    public:
        struct face{
            int index[3];
        };
        Object3D();

        virtual ~Object3D();

        std::vector<Color> colors;
        std::vector<vector4> points;
        std::vector<vector4> uv;
        std::vector<face> faces;

        UINT* _textrue;
    };


    struct vertex
    {
        vertex(){}
        vector4 pos;
        vector4 UV;
        float w;
        float color;
        void set(const Object3D &obj, int index,const vector4& pos2D,float w)
        {
            color = (float)obj.colors[index];
            UV = obj.uv[index];
        }
        void normalizeSelf();
        vertex normalize()const;

        vertex sub(const vertex& rhs)const;
        vertex add(const vertex& rhs)const;
        vertex interp(const vertex& rhs,float t);
    };

    //平底三角形
    struct Triangle
    {
        Triangle():top(0),bottom(0){}
        vertex v[3];
        float top,bottom;
        void computeTopBotton();
        void sortVectex();
        std::vector<Triangle> makeTwo();
        bool isFlat();
        //转化成梯形
        //std::vector<vertex> toTrapezoid();
    };

    //线段扫描器，用于插值线段
    class LineScaner
    {
    public:
        LineScaner(const vertex& v1,const vertex& v2,int stepSize);

        bool step(vertex& v,int num = 1)const ;
    private:
        vertex stepValue;
    };


    //场景
    class Scene
    {
    public:
        Scene(){}

        //初始化obj
        void init();
        Matrix worldMatrix;
        Object3D obj;
    };

	//设备接口
	class Device {
	public:
		Device():lpData(nullptr){}
		virtual UINT Create(int width, int height) = 0;
		virtual UINT Frame(Color *frameBuffer) = 0;
	protected:
		void * lpData;
	};

    //渲染器
    class Render
    {
    public:
        enum RENDER_STATE{wireframeRender,colorRender,textureRender};

        Render(int width,int height)
        {
            frameBuffer = new Color[width*height* sizeof(Color)];
            Zbuffer = new float[width*height* sizeof(float)];
            this->width = width;
            this->height = height;
        }

        void rending(Scene& scene,Camera& camera);

        Color _bkColor;
        Color _lineColor;
        RENDER_STATE _state;
    private:
        static UINT check_cvv(const vector4& p);

        //设置像素
        void setPixel(int x,int y,float w,Color color);

        //画线
        void drawline(vector4 be,vector4 ed);
        void drawline(const vertex& be,const vertex& ed);

        //画三角形
        void drawTriangle(const Triangle& t);

		//用背景颜色清空缓存
		void ClearFrame(Color c);

        //创建窗口设备
		void CreateWindow();        

        //渲染到设备
		void FrameToWindow();

    private:
        int width,height;
        Color *frameBuffer;
        float *Zbuffer;     // 1/z坐标深度
		void * device;
    };
}

#endif //REMOVE_DUPLICATE_LETTERS_MINI3D_H
