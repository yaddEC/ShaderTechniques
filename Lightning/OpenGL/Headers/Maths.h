#pragma once
#include <cmath>
#include <vector>
#define M_PI       3.14159265358979323846


namespace Math
{

    //===== Vector2D =====

    class Vector2D
    {
    public:
        union
        {
            struct
            {
                float x;
                float y;
            };
            float xy[2];
        };

    public:
        // Constructors
        Vector2D();
        Vector2D(float a, float b);
        ~Vector2D() {}

        // Member functions
        float GetMagnitude() const;
        float Dot(const Vector2D vec2);

        void Normalize();

        // Operators
        Vector2D operator+(const Vector2D& vec2);
        Vector2D operator-(const Vector2D& vec2);
        Vector2D operator*(const Vector2D& vec2);
        Vector2D operator*(const float& a);
        Vector2D operator/(const float& a);
    };

    //====================


    //===== Vector3D =====

    class Vector3D
    {
    public:
        union
        {
            struct
            {
                float x;
                float y;
                float z;
            };
            float xyz[3];
        };

    public:
        // Constructors
        Vector3D();
        Vector3D(float a, float b, float c);
        ~Vector3D() {}

        // Member functions
        float GetMagnitude() const;
        float Dot(const Vector3D vec3);

        void Normalize();

        Vector3D CrossProduct(const Vector3D vec3);

        // Operators
        Vector3D operator+(const Vector3D& vec3);
        Vector3D operator-(const Vector3D& vec3);
        Vector3D operator*(const Vector3D& vec3);
        Vector3D operator*(const float& a);
        Vector3D operator/(const float& a);

    };

    //====================


    //===== Vector4D =====

    class Vector4D
    {
    public:
        union
        {
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
            float xyz[4];
        };

    public:
        // Constructors
        Vector4D();
        Vector4D(float a, float b, float c, float d);
        Vector4D(const Vector3D& vec3, float _w = 1.0f);
        ~Vector4D() {}

        // Member functions
        float GetMagnitude() const;

        void Homogenize();
        void Normalize();

        Vector4D CrossProduct(const Vector3D vec4);

        // Operators
        Vector4D operator+(const Vector4D& vec4);
        Vector4D operator*(const Vector4D& vec4);
        Vector4D operator*(const float& a);
    };

    //====================


    //===== Matrix3 =====

    class Matrix3
    {
    public:
        union
        {
            struct
            {
                float matrixTab3[3][3];
            };
            Vector3D Matrix3V[3];
            float matrixTab9[9];
        };

    public:
        // Constructors
        Matrix3();
        Matrix3(const float a[3][3]);
        ~Matrix3() {}

        // Member functions
        Matrix3 GetTransposeMat3();
        Matrix3 GetAdjugateMat3();
        Matrix3 GetInvertibleMat3();

        // Operators
        Matrix3 operator*(const Matrix3& mat1);
        Vector3D operator*(const Vector3D& vec1);
    };

    //====================


    //===== Matrix4 =====
    class Quaternion;
    class Matrix4
    {
    public:
        union
        {
            struct
            {
                float matrixTab4[4][4];
            };
            Vector4D Matrix4V[4];
            float matrixTab16[16];
        };

    public:
        // Constructors
        Matrix4();
        Matrix4(const float a[4][4]);
        ~Matrix4() {}

        // Member functions
        Matrix4 GetTransposeMat4();

        static Matrix4 CreateTransformMatrix(const Vector3D& rotation, const Vector3D& position, const Vector3D& scale);
        static Matrix4 CreateTransformMatrixWithQuaternion(const Quaternion& rotation, const Vector3D& position, const Vector3D& scale);
        static Matrix4 CreateTranslationMatrix(const Vector3D& translation);
        static Matrix4 CreateScaleMatrix(const Vector3D& scale);
        static Matrix4 CreateXRotationMatrix(float angle);
        static Matrix4 CreateYRotationMatrix(float angle);
        static Matrix4 CreateZRotationMatrix(float angle);

        Matrix4 GetAdjugateMat4();
        Matrix4 GetInvertibleMat4();
        Matrix4 GetProjection(const float& fov, const float& n, const float& f);

        void inverse(const Matrix4 in, Matrix4& out);

        // Operators
        Matrix4 operator*(const Matrix4& mat1);
        Vector4D operator*(const Vector4D& vec1);
    };

    //====================


    //==== Quaternion ====

    class Quaternion
    {

    public:
        // Data members
        float s;
        Vector3D v;

    public:
        // Constructors
        Quaternion();
        Quaternion(float uS, Vector3D& uV);
        Quaternion(const Quaternion& value);
        ~Quaternion() {}


        // Member functions
        float Dot(Quaternion& q);
        float Norm();

        void Normalize();
        void ConvertToUnitNormQuaternion();
        void Inverse(Quaternion& q);
        void TransformEulerAnglesToQuaternion(float x, float y, float z);
        void TransformMatrix3nToQuaternion(Matrix3& uMatrix);
        void Negate();

        Quaternion Conjugate();
        Quaternion Inverse();
        Matrix4    TransformQuaternionToMatrix4() const;
        Vector3D   Rotate_vector_by_quaternion(const Vector3D& v);
        Vector3D   TransformQuaternionToEulerAngles();

        // Operators
        inline Quaternion& operator=(const Quaternion& value);
        Quaternion operator+(const Quaternion& q);
        Quaternion operator-(const Quaternion& q);
        Quaternion operator*(const Quaternion& q);
        Quaternion operator*(const Vector3D& uValue);
        Quaternion operator*(const float value);
        void operator+=(const Quaternion& q);
        void operator-=(const Quaternion& q);
        void operator*=(const float value);
        //void operator*=(const Quaternion& q);
        //Quaternion multiply(const Quaternion& q);


    };

    //====================


    //===== inline =====

    inline Matrix3 identity3x3()
    {
        Matrix3 mat;

        float mat4[3][3] =
        {
            {1.f, 0.f, 0.f},
            {0.f, 1.f, 0.f},
            {0.f, 0.f, 1.f},
        };

        mat = mat4;
        return mat;
    }

    inline Matrix4 identity4x4()
    {
        Matrix4 mat;

        float mat4[4][4] =
        {
            {1.f, 0.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            {0.f, 0.f, 0.f, 1.f},
        };

        mat = mat4;
        return mat;
    }

    inline float Radian(const float& degree)
    {
        return degree * (M_PI / 180);
    }

    //====================


    //======= Check ======

    int checkMax2(int a, int b);
    int checkMin2(int a, int b);

    int checkMax3(int a, int b, int c);
    int checkMin3(int a, int b, int c);

    //====================


    //== DeterminantMat ==

    float GetDeterminantMat2(const float a, const float b, const float c, const float d);
    float GetDeterminantMat3(const Vector3D& a, const Vector3D& b, const Vector3D& c);
    float GetDeterminantMat3(const float tab[3][3]);
    float GetDeterminantMat4(const Matrix4& a);

    //==================== 

    //== OtherFunction ===

    float edgeFunction(const Vector3D& a, const Vector3D& b, const Vector3D& c);
    float Lerp(float a, float b, float t);
    Vector3D PosLerp(Vector3D a, Vector3D b, float t);
    Quaternion Slerp(Quaternion q1, Quaternion q2, float t);
    float DotQuat1xQuat2(Quaternion q1, Quaternion q2);

    //==================== 
}

using namespace Math;

Matrix3 operator*(const Matrix3& a, const Matrix3& b);
Matrix3 operator*(const Matrix3& a, const float b);

Matrix4 operator*(const Matrix4& a, const Matrix4& b);
Matrix4 operator*(const Matrix4& a, const float b);

Vector3D operator*(const float b, const Vector3D& a);

Vector4D operator*(const Vector4D& b, const Matrix4& a);
Vector4D operator*(const Matrix4& a, const Vector4D& b);


class OBB
{
private:
    Matrix4 _umv;
    Vector3D* _halfSize;
public:
    Vector3D* rotation;
    OBB();
    ~OBB() {}
    OBB(Vector3D* halfSize, Vector3D* rotation);
    Matrix4 umv();
    Vector3D halfSize();
};

class Sphere
{
public:
    float radius;

    Sphere();
    Sphere(float rad);

};





