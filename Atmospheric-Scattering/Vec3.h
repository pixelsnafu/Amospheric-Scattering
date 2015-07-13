#pragma once

#ifndef _VEC3_H_
#define _VEC3_H_

#include <iostream>
#include <cassert>

#define PI 3.14159

/**
Helper 3D vector template class for math calculations
*/

template<typename T>
class Vec3{

public:
	T x, y, z;

	//constructors
	Vec3():x(T(0)), y(T(0)), z(T(0)){}
	Vec3(T xx):x(xx), y(xx), z(xx){}
	Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

	//function to return the magnitude of the vector
	T magnitude() const { 
		return sqrt(x * x + y * y + z * z); 
	}

	//normalize a vector (perform calculation on the vector itself)
	void normalize() { 
		T mag = magnitude(); 
		if (mag) 
			*this *= 1 / mag; 
	}

	//return a normalized version of a vector (copy)
	Vec3<T> normalized(){
		T mag = magnitude();

		return *this / mag;
	}


	//calculate dot product of this vector with another vector
	T dot(const Vec3<T> &v) const{ 
		return x * v.x + y * v.y + z * v.z; 
	}


	//calculate cross product of this vector with another vector
	Vec3 cross(const Vec3<T> &v) const{
		return Vec3(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x);
	}

	//add two vectors
	Vec3 operator+ (const Vec3<T> &v) const{ 
		return Vec3<T>(x + v.x, y + v.y, z + v.z); 
	}

	//add two vectors (reference)
	Vec3& operator+= (const Vec3<T> &v){
		*this = *this + v;
		return *this;
	}

	//scale a vector
	Vec3 operator* (const T &val) const{ 
		return Vec3<T>(x * val, y * val, z * val); 
	}

	//divide a vector by a constant value
	Vec3 operator/ (const T &val) const{ 
		T invVal = T(1) / val; 
		return Vec3<T>(x * invVal, y * invVal, z * invVal); 
	}

	Vec3& operator/= (const T &val) {
		*this = *this / val;
		return *this;
	}

	//divide a vector by another vector
	Vec3 operator/ (const Vec3<T> &v) const{ 
		return Vec3<T>(x / v.x, y / v.y, z / v.z); 
	}

	//multiple a vector with another vector
	Vec3 operator* (const Vec3<T> &v) const{
		return Vec3<T>(x * v.x, y * v.y, z * v.z); 
	}

	//calculate difference of two vectors
	Vec3 operator- (const Vec3<T> &v) const{
		return Vec3<T>(x - v.x, y - v.y, z - v.z); 
	}

	//negate a vector
	Vec3 operator- () const{
		return Vec3<T>(-x, -y, -z); 
	}

	//scale a vector by reference
	Vec3& operator*=(const T& n){
		*this = *this * n;
		return *this;
	}

	//compare two vectors
	bool operator==(const Vec3<T>& v) const{
		return (x == v.x && y == v.y && z == v.z);
	}

	//compare two vectors for inequality
	bool operator!=(const Vec3<T>& v) const{
		return !(*this == v);
	}

	//divide a vector by a scalar
	friend Vec3<T> operator/ (const T& val, const Vec3<T>& v){
		return Vec3<T>(val/v.x, val/v.y, val/v.z);
	}

	//scale a vector
	friend Vec3<T> operator* (const T& val, const Vec3<T>& v){
		return Vec3<T>(val * v.x, val * v.y, val * v.z);
	}

	//print a vector
	friend std::ostream& operator<<(std::ostream& os, const Vec3<T>& v){
		os << v.x << " " << v.y << " " << v.z << std::endl;
		return os;
	}
};

//helper structure to store faces (triangles)
struct face{
    GLuint a,b,c,n,t;
    face(GLuint aa, GLuint bb, GLuint cc, GLuint nn) :
	a(aa), b(bb), c(cc), n(nn){
		t = -1;
    }

	face(GLuint aa, GLuint bb, GLuint cc, GLuint nn, GLuint tt) :
	a(aa), b(bb), c(cc), n(nn), t(tt){
	}
	
    face(){
        a = 0;
        b = 0;
        c = 0;
        n = 0;
		t = 0;
	}

	const GLuint& operator[] (const GLuint& index){
		assert( index < 5 );
		switch (index){
		case 0:
			return a;
			break;
		case 1:
			return b;
			break;
		case 2:
			return c;
			break;
		case 3:
			return n;
			break;
		case 4:
			return t;
			break;
		default:
			std::cerr << "Face index value can't exceed 5. Aborting..." << std::endl;
			exit(1);
		}
	}
};


//calculate cross product of two vectors
template<typename T>
Vec3<T> cross(const Vec3<T> &va, const Vec3<T> &vb)
{
	return Vec3<T>(
		va.y * vb.z - va.z * vb.y,
		va.z * vb.x - va.x * vb.z,
		va.x * vb.y - va.y * vb.x);
}

//calculate dot product of two vectors
template <typename T>
inline T dot(const Vec3<T>& a, const Vec3<T>& b){
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

//calculate the distance between two vectors
template <typename T>
T vDistance(const Vec3<T>& v1, const Vec3<T>& v2){
	T diffx = v1.x - v2.x;
	T diffy = v1.y - v2.y;
	T diffz = v1.z - v2.z;

	T diffx2 = diffx * diffx;
	T diffy2 = diffy * diffy;
	T diffz2 = diffz * diffz;

	return sqrt(diffx2 + diffy2 + diffz2);
}

typedef Vec3<float> Vec3f;

#pragma warning( disable : 4715)
//comparator for the 3d vector
struct Vec3Comp{
	bool operator()(const Vec3f& v1, const Vec3f& v2){
		if(v1.x < v2.x){
			return true;
		}else if(v1.x > v2.x){
			return false;
		}else if(v1.x == v2.x){
			if(v1.y < v2.y)
				return true;
			else if(v1.y > v2.y)
				return false;
			else if(v1.y == v2.y){
				if(v1.z < v2.z)
					return true;
				else if(v1.z > v2.z)
					return false;
				else{
					return v1.z < v2.z;
				}
			}
		}
	}
};

#endif