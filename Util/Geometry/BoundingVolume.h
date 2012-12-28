#ifndef __BOUNDING_VOLUME_H__
#define __BOUNDING_VOLUME_H__

#include "Box.h"
#include "Sphere.h"

//TODO: this is uncompileable, for now just use box and figure out a good design for Bounding Volumes later

template<typename T = glm::mediump_float>
struct BoundingVolume {
    enum Type {
        BV_AABB,       ///<Axis Aligned Bounding Box
        BV_BOX,        ///<Oriented Box
        BV_SPHERE,     ///<Sphere
        //BV_HULL        ///<Convex Hull, probably won't be supported
    };

    union Shape {
        Box<T> m_aabb;
        Sphere<T> m_sphere;
        //TODO: Oriented Box
        //TODO: Sphere
    };

    //TODO: add a bunch of useful stuff

    Type m_type;
    Shape m_shape;
};

#endif