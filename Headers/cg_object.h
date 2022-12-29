#ifndef CG_OBJECT_H
#define CG_OBJECT_H

#include <bits/stdc++.h>
using namespace std;

template<class T> class Packing;

//Object: base class
class Object {
public:
	Object() {}
	~Object() {}
	template <typename T> T *to_son_class() {
		if(typeid(Packing<T>).hash_code() != hash_code) {
			return nullptr;
		}
		return &(((Packing<T>*)this)->data);
	}
protected:
	size_t hash_code;
};

// package
template<class T>
class Packing : public Object {
public:
	Packing(T _data) {
		data = _data;
		hash_code = typeid(*this).hash_code();
	}
	~Packing() {}
public:
	T data;
};

#endif