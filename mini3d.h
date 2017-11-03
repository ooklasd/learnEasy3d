//
// Created by Shunming GUO on 2017/11/3.
// Copyright (c) 2017 ___FULLUSERNAME___. All rights reserved.
//

#ifndef REMOVE_DUPLICATE_LETTERS_MINI3D_H
#define REMOVE_DUPLICATE_LETTERS_MINI3D_H

#include <math.h>

namespace  mini3d
{
    typedef unsigned int UINT;
    int CMID(int x, int min, int max) { return (x < min)? min : ((x > max)? max : x); }

    // 计算插值：t 为 [0, 1] 之间的数值
    float interp(float x1, float x2, float t) { return x1 + (x2 - x1) * t; }


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

        vectorX<T> add(vectorX<T>& rhs)
        {
            return vectorX<T>(x+rhs.x,y+rhs.y,z+rhs.z,1);
        }

        vectorX<T> sub(vectorX<T>& rhs)
        {
            return vectorX<T>(x-rhs.x,y-rhs.y,z-rhs.z,1);
        }

        vectorX<T> normalize()
        {
            return vectorX<T>(x/w,y/w,z/w,1);
        }

        T length()const
        {
            float temp = x*x+y*y+z*z;
            return ::sqrt(temp);
        }

        T dot(vectorX<T>& rhs)
        {
            return x*rhs.x + y*rhs.y + z*rhs.z;
        }

        vectorX<T> cross(vectorX<T>& rhs)
        {
            float m1, m2, m3;
            m1 = y * rhs.z - z * rhs.y;
            m2 = z * rhs.x - x * rhs.z;
            m3 = x * rhs.y - y * rhs.x;
            return vectorX<T>(m1,m2,m3);
        }

        vectorX<T> interp(vectorX<T>& rhs,T t)
        {
            vectorX<T> ret;

            ret.x = mini3d::interp(x,rhs.x,t);
            ret.y = mini3d::interp(y,rhs.y,t);
            ret.z = mini3d::interp(z,rhs.z,t);

            return ret;
        }

        vectorX<T> operator -(vectorX<T>& rhs)
        {
            return sub(rhs);
        }

        vectorX<T> operator +(vectorX<T>& rhs)
        {
            return add(rhs);
        }

        vectorX<T> operator *(vectorX<T>& rhs)
        {
            return dot(rhs);
        }
        vectorX<T> operator ^(vectorX<T>& rhs)
        {
            return cross(rhs);
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

        MatrixX<T> add(MatrixX<T>& rhs)const
        {
            MatrixX<T> ret;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j <4; ++j) {
                    ret.m[i][j] = m[i][j] + rhs.m[i][j];
                }
            }
            return ret;
        }

        MatrixX<T> sub(MatrixX<T>& rhs)const
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

        MatrixX<T> premul(MatrixX<T>& rhs)const
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
        MatrixX<T> operator+(MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        MatrixX<T> operator-(MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        MatrixX<T> operator*(MatrixX<T>& rhs)
        {
            return add(rhs);
        }

        T m[4][4] = {};
    };

    typedef MatrixX<float> Matrix;

    template <typename  T>
    vectorX<T> apply(vectorX<T> v,MatrixX<T> m)
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
    vectorX<T> operator*(vectorX<T> v,MatrixX<T> m)
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
        union {
            unsigned int color;
            char a,r,g,b;
        };
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

    //场景
    class Scene
    {
    public:

    };

    //渲染器
    class Render
    {
    public:
        Render(float width,float height)
        {
            frameBuffer = new Color[width*height];
            Zbuffer = new float[width*height];
        }

        void rending(Scene& scene,Camera& camera)
        {

        }

        UINT setPixel();


    private:
        Color *frameBuffer;
        float *Zbuffer;     // 1/z坐标深度
    };



}

#endif //REMOVE_DUPLICATE_LETTERS_MINI3D_H
