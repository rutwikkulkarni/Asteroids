/* date = December 27th 2021 2:13 pm */

#ifndef MATHS_H
#define MATHS_H

#include <math.h>

#define PI 3.14159265359
#define Radians(deg) (PI*deg)/180.0

struct Vec2{
    f32 x;
    f32 y;
};

f32 Sign(f32 x){
    if(x >= 0) return 1;
    if(x < 0) return -1;
    else return 0;
}

Vec2 V2(f32 x, f32 y){
    Vec2 v2 = {0};
    v2.x = x;
    v2.y = y;
    return v2;
}

f32 V2Length(Vec2 v2){
    return powf(v2.x, 2)+powf(v2.y, 2);
}

Vec2 operator+(Vec2 a, Vec2 b){
    Vec2 result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

Vec2 operator*(Vec2 a, f32 b){
    Vec2 result = {};
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}

struct Vec3{
    union{
        struct{
            f32 x;
            f32 y;
            f32 z;
        };
        struct{
            u32 r;
            u32 g;
            u32 b;
        };
    };
};

Vec3 V3(f32 x, f32 y, f32 z){
    Vec3 v3 = {0};
    v3.x = x;
    v3.y = y;
    v3.z = z;
    return v3;
}

//colour
Vec3 C3(u32 r, u32 g, u32 b){
    Vec3 c3 = {0};
    c3.r = r;
    c3.g = g;
    c3.b = b;
    return c3;
}

struct Vec4{
    union{
        struct{
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct{
            u32 r;
            u32 g;
            u32 b;
            u32 a;
        };
    };
};

Vec4 V4(f32 x, f32 y, f32 z, f32 w){
    Vec4 v4 = {0};
    v4.x = x;
    v4.y = y;
    v4.z = z;
    v4.w = w;
    return v4;
}

b32 AABB(Vec4 b1, Vec4 b2){
#if 0
    char log[256];
    snprintf(log, 256, "b1: (%f, %f, %f, %f)\nb2: (%f, %f, %f, %f)\n", b1.x, b1.y, b1.z, b1.w, b2.x, b2.y, b2.z, b2.w);
    OutputDebugString(log);
#endif
    if(b1.x + b1.z > b2.x && b1.x < b2.x + b2.z && b1.y + b1.w > b2.y && b1.y < b2.y + b2.w){
        return true;
    }else{
        return false;
    }
}

//colour
Vec4 C4(u32 r, u32 g, u32 b, u32 a){
    Vec4 c4 = {0};
    c4.r = r;
    c4.g = g;
    c4.b = b;
    c4.a = a;
    return c4;
}

struct Mat4{
    f32 elements[4][4];
};

Mat4 M4(){
    Mat4 m4 = {};
    return m4;
}

Mat4 M4I(){
    Mat4 m4 = {};
    for(int i = 0; i < 4; i++){
        m4.elements[i][i] = 1;
    }
    return m4;
}

Mat4 M4Translate(Mat4 m4, Vec3 amount){
    Mat4 final_m4 = m4;
    final_m4.elements[0][3] += amount.x;
    final_m4.elements[1][3] += amount.y;
    final_m4.elements[2][3] += amount.z;
    return final_m4;
}

Mat4 M4RotateZ(Mat4 m4, f32 degrees){
    Mat4 final_m4 = m4;
    f32 radians = Radians(degrees);
    final_m4.elements[0][0] = cos(radians);
    final_m4.elements[0][1] = -sin(radians);
    final_m4.elements[1][0] = sin(radians);
    final_m4.elements[1][1] = cos(radians);
    return final_m4;
}

Mat4 M4Scale(Mat4 m4, Vec3 amount){
    Mat4 final_m4 = m4;
    final_m4.elements[0][0] = amount.x;
    final_m4.elements[1][1] = amount.y;
    final_m4.elements[2][2] = amount.z;
    return final_m4;
}

Mat4 Orthographic(f32 l, f32 r, f32 t, f32 b, f32 n, f32 f){
    Mat4 ortho = M4();
    ortho.elements[0][0] = 2.0f/(r-l);
    ortho.elements[1][1] = 2.0f/(t-b);
    ortho.elements[2][2] = 2.0f/(f-n);
    ortho.elements[3][3] = 1;
    ortho.elements[0][3] = -(r-l)/(r-l);
    ortho.elements[1][3] = -(t+b)/(t-b);
    ortho.elements[2][3] = (f+n)/(f-n);
    ortho.elements[3][3] = 1;
    return ortho;
}

Vec4 M4MultV4(Mat4 m4, Vec4 v4){
    Vec4 result = {};
    result.x = m4.elements[0][0] * v4.x + m4.elements[0][1] * v4.y + m4.elements[0][2] * v4.z + m4.elements[0][3] * v4.w;
    result.y = m4.elements[1][0] * v4.x + m4.elements[1][1] * v4.y + m4.elements[1][2] * v4.z + m4.elements[1][3] * v4.w;
    result.z = m4.elements[2][0] * v4.x + m4.elements[2][1] * v4.y + m4.elements[2][2] * v4.z + m4.elements[2][3] * v4.w;
    result.w = m4.elements[3][0] * v4.x + m4.elements[3][1] * v4.y + m4.elements[3][2] * v4.z + m4.elements[3][3] * v4.w;
    return result;
}

Mat4 operator*(Mat4 a, Mat4 b){
    Mat4 c = {0};
    
    for(int j = 0; j < 4; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            c.elements[j][i] = (a.elements[j][0]*b.elements[0][i] +
                                a.elements[j][1]*b.elements[1][i] +
                                a.elements[j][2]*b.elements[2][i] +
                                a.elements[j][3]*b.elements[3][i]);
        }
    }
    
    return c;
}
#endif //MATHS_H
