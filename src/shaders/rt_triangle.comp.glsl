#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 4, local_size_y = 4) in;
layout (binding = 0, rgba8) uniform image2D computeImage;

#define PI 3.141592
#define Inf 1000000.0
#define Epsilon 0.0001

#define MaxBounces 4
#define SHADOW 0.35


struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct Material
{
    vec3 color;
    int type;
};


struct Plane
{
    vec3 normal;
    float leng;

    Material mat;
};

struct Sphere
{
    vec3 position;
    float radius;

    Material mat;
};


layout (binding = 1) buffer Spheres
{
    Sphere spheres[ ];
};

layout (binding = 2) buffer Planes
{
    Plane planes[ ];
};

layout (binding = 3) uniform App
{
    float time;
} app;


//////////////////////////////

vec3 Camera (in float x, in float y)
{
    ivec2 dimensions = imageSize(computeImage);
    float w = dimensions.x;
    float h = dimensions.y;

    float fovX = PI / 4;
    float fovY = (h / w) * fovX;

    float _x = ((2 * x - w) / w) * tan(fovX);
    float _y = -((2 * y - h) / h) * tan(fovY);

    return vec3(_x, _y, -1.0);
}


float PlaneIntersection (in Ray ray, in Plane plane)
{
    float d0 = dot(plane.normal, ray.direction);

    if (d0 != 0)
    {
        float t = -1 * (((dot(plane.normal, ray.origin)) + plane.leng) / d0);
        return (t > Epsilon) ? t : 0;
    }

    return 0;
}

float SphereIntersection (in Ray ray, in Sphere sphere)
{
    vec3 delta = ray.origin - sphere.position;
    float b = dot((delta * 2), ray.direction);
    float c = dot(delta, delta) - (sphere.radius * sphere.radius);

    float disc = b * b - 4 * c;
    if (disc < 0)
    return 0;
    else
    disc = sqrt(disc);

    // Always 2 solutions when pulling the square root.
    float result1 = -b + disc;
    float result2 = -b - disc;

    return (result2 > Epsilon) ? result2 / 2 : ((result1 > Epsilon) ? result1 / 2 : 0);
}

vec3 GetSphereNormal (in vec3 hitPos, in Sphere sphere)
{
    return (hitPos - sphere.position) / sphere.radius;
}

bool TryGetIntersection (in Ray ray, out int id, out float distance, out bool sphere)
{
    id = -1;
    distance = Inf;

    for (int i = 0; i < planes.length(); i++)
    {
        Plane p = planes[i];
        float dist = PlaneIntersection (ray, p);
        if (dist > Epsilon && dist < distance)
        {
            distance = dist;
            id = i;
            sphere = false;
        }
    }

    for (int i = 0; i < spheres.length(); i++)
    {
        Sphere s = spheres[i];
        float dist = SphereIntersection(ray, s);
        if (dist > Epsilon && dist < distance)
        {
            distance = dist;
            id = i;
            sphere = true;
        }
    }

    return (id > -1) ? true : false;
}

float GetShadow (in Ray ray, in int id, in bool isSphere, in float maxDist)
{
    float distance = Inf;

    for (int i = 0; i < planes.length(); i++)
    {
        Plane p = planes[i];
        if (!isSphere && i == id)
        continue;

        float dist = PlaneIntersection (ray, p);
        if (dist > Epsilon && dist < distance)
        {
            distance = dist;
        }
    }
    for (int i = 0; i < spheres.length(); i++)
    {
        Sphere s = spheres[i];
        if (isSphere && i == id)
        continue;

        float dist = SphereIntersection(ray, s);
        if (dist > Epsilon && dist < distance)
        {
            distance = dist;
        }
    }

    if (distance < maxDist)
    return SHADOW;

    return 1.0;
}


void ReflectRay(inout Ray ray, in vec3 hitNormal, in Material mat)
{
    // Specular BRDF
    if (mat.type == 2)
    {
        float cost = dot(ray.direction, hitNormal);
        vec3 direction = (ray.direction - hitNormal * (cost * 2));
        ray.direction = normalize(direction);
    }
}


vec3 lightPos ()
{
    return vec3(0, 2.95, -3.2);
}


vec3 Light(in vec3 hitPoint)
{
    if ((abs(hitPoint.y - 2.95) < 0.1) && hitPoint.x >= -0.6 && hitPoint.x <= 0.6
    && hitPoint.z <= -3.05 && hitPoint.z >= -3.45)
    return vec3(50, 50, 50);

    return vec3(0, 0, 0);
}
//////////////////////////////


vec3 Trace (inout Ray ray, out vec3 hitNormal)
{
    vec3 finalColor = vec3(1.0);

    for (int i = 0; i < MaxBounces; i++)
    {
        int id;
        float dist;
        bool isSphere; // Is the hitted object a sphere, or a plane ?
        bool intersection = TryGetIntersection(ray, id, dist, isSphere);
        if (!intersection)
        {
            finalColor *= vec3(1.0);
            break;
        }


        Sphere s = spheres[id];
        Plane p = planes[id];
        vec3 hitPoint = ray.origin + ray.direction * dist;
        ray.origin = hitPoint;
        hitNormal = (isSphere) ? GetSphereNormal(hitPoint, s) : p.normal;

        Material mat = (isSphere) ? s.mat : p.mat;


        vec3 emission = Light(hitPoint);
        if (length(emission) > Epsilon)
        return finalColor * emission;

        ReflectRay(ray, hitNormal, mat);

        if (mat.type == 1)
        {
            vec3 lightDir = normalize(lightPos() - hitPoint);
            float lightAttenuation = clamp(dot(hitNormal, lightDir), 0.1, 1.0);

            finalColor = finalColor * lightAttenuation * mat.color;

            // Shadow Ray
            Ray shadowRay;
            shadowRay.origin = hitPoint;
            shadowRay.direction = lightDir;
            float maxDist = length(lightPos() - hitPoint);

            finalColor *= GetShadow(shadowRay, id, isSphere, maxDist);
            //

            break;
        }
    }

    return finalColor;
}


void main()
{
    uint idx = gl_GlobalInvocationID.x;
    uint idy = gl_GlobalInvocationID.y;


    Ray ray;
    ray.origin = vec3(0, 0, -0.1);
    vec3 cam = Camera(idx, idy);
    ray.direction = normalize( (cam - ray.origin));

    vec3 finalColor = vec3(0.0);
    vec3 hitNormal;
    finalColor = Trace(ray, hitNormal);

    finalColor = vec3(clamp(finalColor.x, 0.0, 1.0), clamp(finalColor.y, 0.0, 1.0), clamp(finalColor.z, 0.0, 1.0));

    imageStore(computeImage, ivec2(gl_GlobalInvocationID.xy), vec4(finalColor, 0.0));
}