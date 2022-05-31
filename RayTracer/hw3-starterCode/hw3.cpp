/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: NiNgHao HE
 * *************************
*/
// REFERENCE TO 
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/// ray-triangle-intersection-geometric-solution
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
// https://raytracing.github.io/books/RayTracingInOneWeekend.html
#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif
#include <cmath>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <cstring>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif
using namespace std;
#include <imageIO.h>

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480
#define ASPECTRATIO double(WIDTH)/HEIGHT
//the field of view of the camera
#define fov 60.0
float i_end = 0.146;
float i_start= 0.016;
unsigned char buffer[HEIGHT][WIDTH][3];
struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};
struct Point 
{
  double x;
  double y;
  double z;
};
struct color 
{
  double r;
  double g;
  double b;
};
struct Ray{
  double origin[3];
  double direction[3];
};
struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};
struct intersection_P
{
  Point intersect;
  Point normal;
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};
int soft_shadow_lights = 0;
intersection_P intersec_Ptri;
intersection_P intersec_Psphere;
color Point_color;
color previous_color;
vector<Ray> ray_vector;
Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];
Point intersection;
int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;
double triangle_t = -1;
double sphere_t;
void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

double dot_product(Point v1, Point v2){
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Point cross_product(Point v1, Point v2){
  Point new_point;
  new_point.x = v1.y*v2.z - v1.z*v2.y;
  new_point.y = v1.z*v2.x - v1.x*v2.z;
  new_point.z = v1.x*v2.y - v1.y*v2.x;
  return new_point;
}

Point subtract_vec(Point v1, Point v2){
  Point new_point;
  new_point.x = v1.x - v2.x;
  new_point.y = v1.y - v2.y;
  new_point.z = v1.z - v2.z;
  return new_point;
}
float adjustable[3] = {0.0005,-0.00005,0.00005};
Point add_vec(Point v1, Point v2){
  Point new_point;
  new_point.x = v1.x + v2.x;
  new_point.y = v1.y + v2.y;
  new_point.z = v1.z + v2.z;
  return new_point;
}
Point normalize(Point v){
  Point new_point;
  double magnitude = std::sqrt (std::pow (v.x, 2) + std::pow (v.y, 2) + std::pow (v.z, 2));
  new_point.x = v.x/magnitude;
  new_point.y = v.y/magnitude;
  new_point.z = v.z/magnitude;
  return new_point;
}

Point multiply(Point v, double u){
  Point new_point;
  new_point.x = v.x * u;
  new_point.y = v.y * u;
  new_point.z = v.z * u;
  return new_point;
}
color getTriCOLOR(Light light, intersection_P tri_intersect){
  Point light_direction;
  light_direction.x = (light.position[0]-tri_intersect.intersect.x);
  light_direction.y = (light.position[1]-tri_intersect.intersect.y);
  light_direction.z = (light.position[2]-tri_intersect.intersect.z);
  light_direction = normalize(light_direction);
  double LdotN = dot_product(light_direction, tri_intersect.normal);
  if(LdotN < 0){
    LdotN = 0.0;
  }else if(LdotN > 1.0f){
    LdotN = 1.0;
  }
  
  Point kd;
  kd.x = tri_intersect.color_diffuse[0];
  kd.y = tri_intersect.color_diffuse[1];
  kd.z = tri_intersect.color_diffuse[2];
  
  Point kd_LdotN = multiply(kd, LdotN);
  Point ks;
  ks.x = tri_intersect.color_specular[0];
  ks.y = tri_intersect.color_specular[1];
  ks.z = tri_intersect.color_specular[2];

  Point R;
  R.x = -tri_intersect.intersect.x;
  R.y = -tri_intersect.intersect.y;
  R.z = -tri_intersect.intersect.z;
  R = normalize(R);
  Point negate_light;
  negate_light.x = -light_direction.x;
  negate_light.y = -light_direction.y;
  negate_light.z = -light_direction.z;
  Point ref;
  // r = 2(l dot n) n - l
  double NdotL = 2 * dot_product(light_direction,tri_intersect.normal);
  Point temp = multiply(tri_intersect.normal, NdotL);
  ref = subtract_vec(temp, light_direction);
  double RdotV = dot_product(R, ref);
  if(RdotV < 0){
    RdotV = 0.0;
  }else if(RdotV > 1.0f){
    RdotV = 1.0;
  }
  RdotV = std::pow(RdotV, tri_intersect.shininess);
  Point ks_RdotV = multiply(ks, RdotV);
  color Overall;
  Overall.r = ks_RdotV.x + kd_LdotN.x;
  Overall.g = ks_RdotV.y + kd_LdotN.y;
  Overall.b = ks_RdotV.z + kd_LdotN.z;
  Overall.r = Overall.r * light.color[0];
  Overall.g = Overall.g * light.color[1];
  Overall.b = Overall.b * light.color[2];
  return Overall;
}
void getTri_color(Triangle tri_P, Point tri_N, Point normal1, Point normal2, Point intersec_Point){
  Point area = cross_product(normal1,normal2);
  intersec_Ptri.intersect = intersec_Point;
    Point c0_coord;
    c0_coord.x = tri_P.v[0].position[0];
    c0_coord.y = tri_P.v[0].position[1];
    c0_coord.z = tri_P.v[0].position[2];

    Point c1_coord;
    c1_coord.x = tri_P.v[1].position[0];
    c1_coord.y = tri_P.v[1].position[1];
    c1_coord.z = tri_P.v[1].position[2];

    Point c2_coord;
    c2_coord.x = tri_P.v[2].position[0];
    c2_coord.y = tri_P.v[2].position[1];
    c2_coord.z = tri_P.v[2].position[2];
  double area_value = (std::sqrt (std::pow (area.x, 2) + std::pow (area.y, 2) + std::pow (area.z, 2)));
  Point temp1 = subtract_vec(c0_coord,intersec_Point);
  Point temp2 = subtract_vec(c1_coord,intersec_Point);
  Point temp3 = subtract_vec(c2_coord,intersec_Point);

  Point CC0C1;
  Point CC0C2;
  Point CC1C2;
  CC0C1 = cross_product(temp1, temp2);
  CC0C2 = cross_product(temp1, temp3);
  CC1C2 = cross_product(temp2, temp3);
  double CC0C1_value = (std::sqrt (std::pow (CC0C1.x, 2) + std::pow (CC0C1.y, 2) + std::pow (CC0C1.z, 2)));
  double CC0C2_value = (std::sqrt (std::pow (CC0C2.x, 2) + std::pow (CC0C2.y, 2) + std::pow (CC0C2.z, 2)));
  double CC1C2_value = (std::sqrt (std::pow (CC1C2.x, 2) + std::pow (CC1C2.y, 2) + std::pow (CC1C2.z, 2)));

  double area_diff_CC0C1 = CC0C1_value / area_value;
  double area_diff_CC0C2 = CC0C2_value / area_value;
  double area_diff_CC1C2 = CC1C2_value / area_value;

  Point normal;
  normal.x = tri_P.v[0].normal[0] * area_diff_CC1C2;
  normal.y = tri_P.v[0].normal[1] * area_diff_CC1C2;
  normal.z = tri_P.v[0].normal[2] * area_diff_CC1C2;
  // cout << area_diff_CC1C2 << " " << area_diff_CC1C2 << " " << area_diff_CC1C2 << endl;
  normal.x = normal.x + tri_P.v[1].normal[0] * area_diff_CC0C2;
  normal.y = normal.y + tri_P.v[1].normal[1] * area_diff_CC0C2;
  normal.z = normal.z + tri_P.v[1].normal[2] * area_diff_CC0C2;

  normal.x = normal.x + tri_P.v[2].normal[0] * area_diff_CC0C1;
  normal.y = normal.y + tri_P.v[2].normal[1] * area_diff_CC0C1;
  normal.z = normal.z + tri_P.v[2].normal[2] * area_diff_CC0C1;

  intersec_Ptri.normal = normal;

  Point diffuse;
  diffuse.x = area_diff_CC1C2 * tri_P.v[0].color_diffuse[0];
  diffuse.y = area_diff_CC1C2 * tri_P.v[0].color_diffuse[1];
  diffuse.z = area_diff_CC1C2 * tri_P.v[0].color_diffuse[2];
  diffuse.x = diffuse.x + area_diff_CC0C2* tri_P.v[1].color_diffuse[0];
  diffuse.y = diffuse.y + area_diff_CC0C2* tri_P.v[1].color_diffuse[1];
  diffuse.z = diffuse.z + area_diff_CC0C2* tri_P.v[1].color_diffuse[2];
  diffuse.x = diffuse.x + area_diff_CC0C1* tri_P.v[2].color_diffuse[0];
  diffuse.y = diffuse.y + area_diff_CC0C1* tri_P.v[2].color_diffuse[1];
  diffuse.z = diffuse.z + area_diff_CC0C1* tri_P.v[2].color_diffuse[2];

  intersec_Ptri.color_diffuse[0] = diffuse.x;
  intersec_Ptri.color_diffuse[1] = diffuse.y;
  intersec_Ptri.color_diffuse[2] = diffuse.z;

  Point specular;
  specular.x = area_diff_CC1C2 * tri_P.v[0].color_specular[0];
  specular.y = area_diff_CC1C2 * tri_P.v[0].color_specular[1];
  specular.z = area_diff_CC1C2 * tri_P.v[0].color_specular[2];
  specular.x = specular.x + area_diff_CC0C2* tri_P.v[1].color_specular[0];
  specular.y = specular.y + area_diff_CC0C2* tri_P.v[1].color_specular[1];
  specular.z = specular.z + area_diff_CC0C2* tri_P.v[1].color_specular[2];
  specular.x = specular.x + area_diff_CC0C1* tri_P.v[2].color_specular[0];
  specular.y = specular.y + area_diff_CC0C1* tri_P.v[2].color_specular[1];
  specular.z = specular.z + area_diff_CC0C1* tri_P.v[2].color_specular[2];

  intersec_Ptri.color_specular[0] = specular.x;
  intersec_Ptri.color_specular[1] = specular.y;
  intersec_Ptri.color_specular[2] = specular.z;

  double shininess;
  shininess = tri_P.v[0].shininess * area_diff_CC1C2;
  shininess = shininess + tri_P.v[1].shininess * area_diff_CC0C2;
  shininess = shininess + tri_P.v[2].shininess * area_diff_CC0C1;

  intersec_Ptri.shininess = shininess;

}

color getSphereCOLOR(Light light, intersection_P sphere_intersect){
  //phong model
  // I = lightColor * (kd * (L dot N) + ks * (R dot V) ^ sh)

  Point light_direction;
  light_direction.x = (light.position[0]-sphere_intersect.intersect.x);
  light_direction.y = (light.position[1]-sphere_intersect.intersect.y);
  light_direction.z = (light.position[2]-sphere_intersect.intersect.z);
  light_direction = normalize(light_direction);
  double LdotN = dot_product(light_direction, sphere_intersect.normal);
  if(LdotN < 0){
    LdotN = 0.0;
  }else if(LdotN > 1.0f){
    LdotN = 1.0;
  }
  
  Point kd;
  kd.x = sphere_intersect.color_diffuse[0];
  kd.y = sphere_intersect.color_diffuse[1];
  kd.z = sphere_intersect.color_diffuse[2];
  
  Point kd_LdotN = multiply(kd, LdotN);
  Point ks;
  ks.x = sphere_intersect.color_specular[0];
  ks.y = sphere_intersect.color_specular[1];
  ks.z = sphere_intersect.color_specular[2];

  Point R;
  R.x = -sphere_intersect.intersect.x;
  R.y = -sphere_intersect.intersect.y;
  R.z = -sphere_intersect.intersect.z;
  R = normalize(R);
  Point negate_light;
  negate_light.x = -light_direction.x;
  negate_light.y = -light_direction.y;
  negate_light.z = -light_direction.z;
  Point ref;
  // r = 2(l dot n) n - l
  double NdotL = 2 * dot_product(light_direction,sphere_intersect.normal);
  Point temp = multiply(sphere_intersect.normal, NdotL);
  ref = subtract_vec(temp, light_direction);
  double RdotV = dot_product(R, ref);
  if(RdotV < 0){
    RdotV = 0.0;
  }else if(RdotV > 1.0f){
    RdotV = 1.0;
  }
  RdotV = std::pow(RdotV, sphere_intersect.shininess);
  Point ks_RdotV = multiply(ks, RdotV);
  color Overall;
  Overall.r = ks_RdotV.x + kd_LdotN.x;
  Overall.g = ks_RdotV.y + kd_LdotN.y;
  Overall.b = ks_RdotV.z + kd_LdotN.z;
  Overall.r = Overall.r * light.color[0];
  Overall.g = Overall.g * light.color[1];
  Overall.b = Overall.b * light.color[2];
  // cout << Overall.r << " " << Overall.g << " " << Overall.b << endl;
  return Overall;
}
void get_Sphere_color(double sphere_P, Sphere sphere, Ray ray){
  Point Intersect_p;
  Point ray_d;
  ray_d.x = ray.direction[0];
  ray_d.y = ray.direction[1];
  ray_d.z = ray.direction[2];
  Intersect_p = multiply(ray_d, sphere_t);
  Intersect_p.x = Intersect_p.x + ray.origin[0];
  Intersect_p.y = Intersect_p.y + ray.origin[1];
  Intersect_p.z = Intersect_p.z + ray.origin[2];
  intersec_Psphere.intersect = Intersect_p;
  intersec_Psphere.color_diffuse[0] = sphere.color_diffuse[0];
  intersec_Psphere.color_diffuse[1] = sphere.color_diffuse[1];
  intersec_Psphere.color_diffuse[2] = sphere.color_diffuse[2];

  intersec_Psphere.color_specular[0] = sphere.color_specular[0];
  intersec_Psphere.color_specular[1] = sphere.color_specular[1];
  intersec_Psphere.color_specular[2] = sphere.color_specular[2];

  intersec_Psphere.shininess = sphere.shininess;
  intersec_Psphere.radius = sphere.radius;

  Point center_sphere;
  center_sphere.x = sphere.position[0];
  center_sphere.y = sphere.position[1];
  center_sphere.z = sphere.position[2];
  intersec_Psphere.normal = normalize(subtract_vec(Intersect_p, center_sphere));
}
bool checkTriangleIntersection(Ray ray, int x, int y, Triangle tris_temp[]){
  bool intersect_or_not = false;
  for(int i = 0; i < num_triangles; i++){
    Point c0_coord;
    c0_coord.x = tris_temp[i].v[0].position[0];
    c0_coord.y = tris_temp[i].v[0].position[1];
    c0_coord.z = tris_temp[i].v[0].position[2];

    Point c1_coord;
    c1_coord.x = tris_temp[i].v[1].position[0];
    c1_coord.y = tris_temp[i].v[1].position[1];
    c1_coord.z = tris_temp[i].v[1].position[2];

    Point c2_coord;
    c2_coord.x = tris_temp[i].v[2].position[0];
    c2_coord.y = tris_temp[i].v[2].position[1];
    c2_coord.z = tris_temp[i].v[2].position[2];


    // Point diffuse = tris_temp[i].v[0].color_diffuse
    Point c_coord;
    c_coord.x = ray.direction[0];
    c_coord.y = ray.direction[1];
    c_coord.z = ray.direction[2];
    // normal for triangle
    Point normal_com1 = subtract_vec(c2_coord, c0_coord);
    Point normal_com2 = subtract_vec(c1_coord, c0_coord);
    Point normal_final = normalize(cross_product(normal_com2, normal_com1));
    double NdotRayDirection = dot_product(normal_final,c_coord); 
    if (fabs(NdotRayDirection) < -1e8){// almost 0 
        continue;
    }
    // 1/2 * || BA x BC ||
    // double lengthOfTwoVectors = 
    // //dot_product(normal_final, cross_two_vectors);
    // (std::sqrt (std::pow (cross_two_vectors.x, 2) + std::pow (cross_two_vectors.y, 2) + std::pow (cross_two_vectors.z, 2)));
    Point origin_temp;
    origin_temp.x = ray.origin[0];
    origin_temp.y = ray.origin[1];
    origin_temp.z = ray.origin[2];
    
    //Point temp = subtract_vec(c0_coord, origin_temp);
    double d = -dot_product(normal_final, c0_coord);
    double t = -(dot_product(normal_final, origin_temp)+d)/NdotRayDirection;

    if (t < 0.000001) {
		  continue;
	  }

    Point intersect;
    intersect = multiply(c_coord, t);
    intersect.x = intersect.x + ray.origin[0];
    intersect.y = intersect.y + ray.origin[1];
    intersect.z = intersect.z + ray.origin[2];
    intersection = intersect;
    if(dot_product(normal_final, cross_product(subtract_vec(c1_coord,c0_coord),subtract_vec(intersect,c0_coord))) < 0.00000001 ||
       dot_product(normal_final, cross_product(subtract_vec(c2_coord,c1_coord),subtract_vec(intersect,c1_coord))) < 0.00000001 ||
       dot_product(normal_final, cross_product(subtract_vec(c0_coord,c2_coord),subtract_vec(intersect,c2_coord))) < 0.00000001
    ){
      // triangle_t = -1;
      continue;
    }
    if(triangle_t == -1){
      triangle_t = t;
      getTri_color(tris_temp[i], normal_final,normal_com1, normal_com2, intersect);
    }else{
      if(triangle_t < t){
        intersect_or_not = true;
        continue;
      }else{
        intersect = multiply(c_coord, t);
        intersect.x = intersect.x + ray.origin[0];
        intersect.y = intersect.y + ray.origin[1];
        intersect.z = intersect.z + ray.origin[2];
        intersection = intersect;
        triangle_t = t;
        getTri_color(tris_temp[i], normal_final,normal_com1, normal_com2, intersect);
      }
    }

    intersect_or_not = true;
    continue;
    // return true;
  }
  return intersect_or_not;
}
bool checkTriangleIntersection2(Ray ray, int x, int y, Triangle tris_temp[],Light lights_temp){
  Point light_source;
  light_source.x = lights_temp.position[0];
  light_source.y = lights_temp.position[1];
  light_source.z = lights_temp.position[2];
  for(int i = 0; i < num_triangles; i++){

    Point c0_coord;
    c0_coord.x = tris_temp[i].v[0].position[0];
    c0_coord.y = tris_temp[i].v[0].position[1];
    c0_coord.z = tris_temp[i].v[0].position[2];

    Point c1_coord;
    c1_coord.x = tris_temp[i].v[1].position[0];
    c1_coord.y = tris_temp[i].v[1].position[1];
    c1_coord.z = tris_temp[i].v[1].position[2];

    Point c2_coord;
    c2_coord.x = tris_temp[i].v[2].position[0];
    c2_coord.y = tris_temp[i].v[2].position[1];
    c2_coord.z = tris_temp[i].v[2].position[2];


    // Point diffuse = tris_temp[i].v[0].color_diffuse
    Point c_coord;
    c_coord.x = ray.direction[0];
    c_coord.y = ray.direction[1];
    c_coord.z = ray.direction[2];
    // normal for triangle
    Point normal_com1 = subtract_vec(c2_coord, c0_coord);
    Point normal_com2 = subtract_vec(c1_coord, c0_coord);
    Point normal_final = normalize(cross_product(normal_com2, normal_com1));
    double NdotRayDirection = dot_product(normal_final,c_coord); 
    if (fabs(NdotRayDirection) < -1e8){// almost 0 
        continue;
    }
    // 1/2 * || BA x BC ||
    // double lengthOfTwoVectors = 
    // //dot_product(normal_final, cross_two_vectors);
    // (std::sqrt (std::pow (cross_two_vectors.x, 2) + std::pow (cross_two_vectors.y, 2) + std::pow (cross_two_vectors.z, 2)));
    Point origin_temp;
    origin_temp.x = ray.origin[0];
    origin_temp.y = ray.origin[1];
    origin_temp.z = ray.origin[2];
    
    //Point temp = subtract_vec(c0_coord, origin_temp);
    double d = -dot_product(normal_final, c0_coord);
    double t = -(dot_product(normal_final, origin_temp)+d)/NdotRayDirection;
    if (t < 0.000001) {
		  continue;
	  }
    Point intersect;
    intersect = multiply(c_coord, t);
    intersect.x = intersect.x + ray.origin[0];
    intersect.y = intersect.y + ray.origin[1];
    intersect.z = intersect.z + ray.origin[2];
    intersection = intersect;
    if(triangle_t != -1){
      triangle_t = min(triangle_t, t);
    }else{
      triangle_t = t;
    }
    if(dot_product(normal_final, cross_product(subtract_vec(c1_coord,c0_coord),subtract_vec(intersect,c0_coord))) < 0.000001 ||
       dot_product(normal_final, cross_product(subtract_vec(c2_coord,c1_coord),subtract_vec(intersect,c1_coord))) < 0.000001 ||
       dot_product(normal_final, cross_product(subtract_vec(c0_coord,c2_coord),subtract_vec(intersect,c2_coord))) < 0.000001
    ){
      triangle_t = -1;
      continue;
    }
    Point hitting = subtract_vec(intersect,origin_temp);
    double magnitude = std::sqrt (std::pow (hitting.x, 2) + std::pow (hitting.y, 2) + std::pow (hitting.z, 2));
    Point lighting = subtract_vec(light_source,origin_temp);
    double magnitude2 = std::sqrt (std::pow (lighting.x, 2) + std::pow (lighting.y, 2) + std::pow (lighting.z, 2));
    if(magnitude > magnitude2){
      continue;
    }
    return true;
  }
  return false;
}
bool checkSphereIntersection(Ray ray, int x, int y, Sphere spheres_temp[]){
  for(int i = 0; i < num_spheres ; i++){
    //doing intersection test for each sphere with the ray we are dealing with'
    double temp_rad = spheres_temp[i].radius;
    double pos_x = spheres_temp[i].position[0];
    double pos_y = spheres_temp[i].position[1];
    double pos_z = spheres_temp[i].position[2];
    //use formula to calculate intersection
    double a = std::pow((ray.direction[0]),2) + 
               std::pow((ray.direction[1]),2) + 
               std::pow((ray.direction[2]),2);

    double b = 2 * (
      ray.direction[0] *(ray.origin[0] - pos_x) + 
      ray.direction[1] *(ray.origin[1] - pos_y) + 
      ray.direction[2] *(ray.origin[2] - pos_z)
      );

    double c = std::pow((ray.origin[0]- pos_x),2) +
               std::pow((ray.origin[1]- pos_y),2) +
               std::pow((ray.origin[2]- pos_z),2) - 
               std::pow(temp_rad,2);
    if((std::pow(b ,2) - 4 * a * c) < 0.00000001){
      //abort if negative
      sphere_t = -1;
      continue;
    }
    double t0 = (-b + std::sqrt( std::pow(b ,2) - 4 * a * c))/2*a;
    //smaller one
    double t1 = (-b - std::sqrt( std::pow(b ,2) - 4 * a * c))/2*a;
    if(t0 > 0.0000000001 && t1 > 0.0000000001){
      sphere_t = min(t0,t1);
      get_Sphere_color(sphere_t,spheres_temp[i], ray);
      return true;
    }else if(t0 > 0.000000000001 && t1 < 0.00000000001){
      sphere_t = t0;
      return true;
    }else if(t0<0.0000000001 && t1>0.0000000001){
      sphere_t = t1;
      return true;
    }else{
      sphere_t = -1;
      continue;
    }
  }
  return false;
}

// do not track color
bool checkSphereIntersection2(Ray ray, int x, int y, Sphere spheres_temp[]){
  for(int i = 0; i < num_spheres ; i++){
    //doing intersection test for each sphere with the ray we are dealing with'
    double temp_rad = spheres_temp[i].radius;
    double pos_x = spheres_temp[i].position[0];
    double pos_y = spheres_temp[i].position[1];
    double pos_z = spheres_temp[i].position[2];
    //use formula to calculate intersection
    double a = std::pow((ray.direction[0]),2) + 
               std::pow((ray.direction[1]),2) + 
               std::pow((ray.direction[2]),2);

    double b = 2 * (
      ray.direction[0] *(ray.origin[0] - pos_x) + 
      ray.direction[1] *(ray.origin[1] - pos_y) + 
      ray.direction[2] *(ray.origin[2] - pos_z)
      );

    double c = std::pow((ray.origin[0]- pos_x),2) +
               std::pow((ray.origin[1]- pos_y),2) +
               std::pow((ray.origin[2]- pos_z),2) - 
               std::pow(temp_rad,2);
    if((std::pow(b ,2) - 4 * a * c) < 0.00000001){
      //abort if negative
      sphere_t = -1;
      continue;
    }
    double t0 = (-b + std::sqrt( std::pow(b ,2) - 4 * a * c))/2*a;
    //smaller one
    double t1 = (-b - std::sqrt( std::pow(b ,2) - 4 * a * c))/2*a;
    if(t0 > 0.0000000001 && t1 > 0.0000000001){
      sphere_t = min(t0,t1);
      // get_Sphere_color(sphere_t,spheres_temp[i], ray);
      return true;
    }else if(t0 > 0.000000000001 && t1 < 0.00000000001){
      sphere_t = t0;
      return true;
    }else if(t0<0.0000000001 && t1>0.0000000001){
      sphere_t = t1;
      return true;
    }else{
      sphere_t = -1;
      continue;
    }
  }
  return false;
}

bool checkShadowIntersection2(Ray ray, int x, int y, Light lights_temp, double intersect_t){
  // 2 version only check if there is an shadow intersection and do not keep record of intersection point
    Point light_source;
    Point light_color;
    light_source.x = lights_temp.position[0];
    light_source.y = lights_temp.position[1];
    light_source.z = lights_temp.position[2];
    light_color.x = lights_temp.color[0];
    light_color.y = lights_temp.color[1];
    light_color.z = lights_temp.color[2];
    Point intersection_point;
    Point c_coord;
    c_coord.x = ray.direction[0];
    c_coord.y = ray.direction[1];
    c_coord.z = ray.direction[2];
    intersection_point = multiply(c_coord, intersect_t);
    intersection_point.x = intersection_point.x + ray.origin[0];
    intersection_point.y = intersection_point.y + ray.origin[1];
    intersection_point.z = intersection_point.z + ray.origin[2];
    Point shadow_ray_dir;
    shadow_ray_dir = normalize(subtract_vec(light_source,intersection_point));
    Ray shadow_ray;
    shadow_ray.direction[0] = shadow_ray_dir.x;
    shadow_ray.direction[1] = shadow_ray_dir.y;
    shadow_ray.direction[2] = shadow_ray_dir.z;
    shadow_ray.origin[0] = intersection_point.x ;
    shadow_ray.origin[1] = intersection_point.y ;
    shadow_ray.origin[2] = intersection_point.z ;
    bool inShadow_spheres;
    bool inShadow_triangles;
    inShadow_spheres = checkSphereIntersection2(shadow_ray,x,y,spheres);
    inShadow_triangles = checkTriangleIntersection2(shadow_ray,x,y,triangles,lights_temp);
    if(inShadow_spheres){
      return true;
    }
    else if(inShadow_triangles){
      return true;
    }
    else{
      return false;
    }
  
  return false;
}
//MODIFY THIS FUNCTION
color doingantialiasing(color temp){
  color new_temp;
  for(int i = 0; i < 4 ; i ++){
    new_temp.r = temp.r + adjustable[i];
    new_temp.g = temp.g + adjustable[i];
    new_temp.b = temp.b + adjustable[i];
  }
  new_temp.r = new_temp.r/4;
  new_temp.g = new_temp.g/4;
  new_temp.b = new_temp.b/4;
  return new_temp;
}
void draw_scene()
{
  //a simple test output
  double x_Min = - ASPECTRATIO * tan((fov*M_PI/180)/2);
  double x_Max = ASPECTRATIO * tan((fov*M_PI/180)/2);
  double y_Min = -tan((fov*M_PI/180)/2);
  double y_Max = tan((fov*M_PI/180)/2);
  int temp = num_lights;
  for(int i = 0; i < temp; i++){
    color light_Color;
    light_Color.r = lights[i].color[0]/(soft_shadow_lights);
    light_Color.g = lights[i].color[1]/(soft_shadow_lights);
    light_Color.b = lights[i].color[2]/(soft_shadow_lights);

    Point light_pos;
    light_pos.x = lights[i].position[0];
    light_pos.y = lights[i].position[1];
    light_pos.z = lights[i].position[2];
    for(int j = 0; j < soft_shadow_lights; j++){
      lights[num_lights].color[0] = light_Color.r;
      lights[num_lights].color[1] = light_Color.g;
      lights[num_lights].color[2] = light_Color.b;

      lights[num_lights].position[0] = light_pos.x+(((double)rand()/RAND_MAX)*(i_end-i_start)+i_start);
      lights[num_lights].position[1] = light_pos.y+(((double)rand()/RAND_MAX)*(i_end-i_start)+i_start);
      lights[num_lights].position[2] = light_pos.z+(((double)rand()/RAND_MAX)*(i_end-i_start)+i_start);
      num_lights = num_lights + 1;
    }
  }
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
      double PixelNDC_x = double(x)/WIDTH;
      double PixelNDC_y = double(y)/HEIGHT;
      double PixelCamera_x = (2 * PixelNDC_x - 1) * ASPECTRATIO * tan((fov*M_PI/180)/2);
      double PixelCamera_y = -(1 - 2 * PixelNDC_y) * tan((fov*M_PI/180)/2);
      double original[3] = {0,0,0};
      double direction[3] = {PixelCamera_x,PixelCamera_y,-1};
      double magnitude = std::sqrt (std::pow (direction[0], 2) + std::pow (direction[1], 2) + std::pow (direction[2], 2));
      direction[0] = direction[0]/magnitude;
      direction[1] = direction[1]/magnitude;
      direction[2] = direction[2]/magnitude;
      Ray new_ray;
      new_ray.origin[0] = 0;
      new_ray.origin[1] = 0;
      new_ray.origin[2] = 0;
      new_ray.direction[0] = direction[0];
      new_ray.direction[1] = direction[1];
      new_ray.direction[2] = direction[2];
      
      ray_vector.push_back(new_ray);
      bool intersect_sphere;
      bool intersect_tri;
      bool intersect_shadow;
      triangle_t = -1;
      intersect_tri = checkTriangleIntersection(new_ray,x,y,triangles);
      intersect_sphere = checkSphereIntersection(new_ray,x,y,spheres);
      if(!intersect_sphere && !intersect_tri){
        // no intersection
        plot_pixel(x, y,  0 + 255,  0 + 255,  0 + 255);
      }else if(intersect_tri && !intersect_sphere){
        // only intersect with triangles
        color temp = {0,0,0};
        double temp_tri_t = triangle_t;
        for(int i = 0 ; i < num_lights; i++){
          intersect_shadow = checkShadowIntersection2(new_ray, x, y, lights[i], temp_tri_t);
          if(intersect_shadow){
            plot_pixel(x, y, temp.r * 255+ambient_light[0], temp.g * 255+ambient_light[1], temp.b * 255+ambient_light[2]);
          }else{
            temp.r = temp.r + getTriCOLOR(lights[i],intersec_Ptri).r;
            temp.g = temp.g + getTriCOLOR(lights[i],intersec_Ptri).g;
            temp.b = temp.b + getTriCOLOR(lights[i],intersec_Ptri).b;
            plot_pixel(x, y, temp.r * 255+ambient_light[0], temp.g * 255+ambient_light[1], temp.b * 255+ambient_light[2]);
          }
        }
      }else if(!intersect_tri && intersect_sphere){
        // only intersect with spheres
        color temp = {0,0,0};
        for(int i = 0 ; i < num_lights; i++){
          bool intersect_shadow = checkShadowIntersection2(new_ray, x, y, lights[i], sphere_t);
          if(intersect_shadow){
            plot_pixel(x, y, temp.r * 255+ambient_light[0], temp.g * 255+ambient_light[1], temp.b * 255+ambient_light[2]);
          }else{
            temp.r =  temp.r + getSphereCOLOR(lights[i],intersec_Psphere).r;
            temp.g =  temp.g + getSphereCOLOR(lights[i],intersec_Psphere).g;
            temp.b =  temp.b + getSphereCOLOR(lights[i],intersec_Psphere).b;
            // temp = doingantialiasing(temp);
            plot_pixel(x, y, temp.r*255+ambient_light[0]   , temp.g *255+ambient_light[1], temp.b *255+ambient_light[2] );
          }

        }
      }else if(intersect_tri && intersect_sphere){
        // intersect with triangles and spheres
        double cloest_t = min(sphere_t, triangle_t);
        color temp = {0,0,0};
        int getWhatColor = -1;
        if(sphere_t < triangle_t){
          // get sphere color
          getWhatColor = 1;
        }else if(sphere_t > triangle_t){
          // get triangle color
          getWhatColor = 0;
        }
        for(int i = 0 ; i < num_lights; i++){
          bool intersect_shadow1 = checkShadowIntersection2(new_ray, x, y, lights[i], cloest_t);
          if(intersect_shadow1){
            plot_pixel(x, y, temp.r * 255+ambient_light[0], temp.g * 255+ambient_light[1], temp.b * 255+ambient_light[2]);
          }else{
            if(getWhatColor == 1){
              temp.r =  temp.r + getSphereCOLOR(lights[i],intersec_Psphere).r;
              temp.g =  temp.g + getSphereCOLOR(lights[i],intersec_Psphere).g;
              temp.b =  temp.b + getSphereCOLOR(lights[i],intersec_Psphere).b;
              plot_pixel(x, y, temp.r*255+ambient_light[0]   , temp.g *255+ambient_light[1], temp.b *255+ambient_light[2] );
            }else if (getWhatColor == 0){
              temp.r =  temp.r + getTriCOLOR(lights[i],intersec_Ptri).r;
              temp.g =  temp.g + getTriCOLOR(lights[i],intersec_Ptri).g;
              temp.b =  temp.b + getTriCOLOR(lights[i],intersec_Ptri).b;
              plot_pixel(x, y, temp.r*255+ambient_light[0]   , temp.g *255+ambient_light[1] , temp.b *255+ambient_light[2]  );
            }
          }
        }
      }

    }
    glEnd();
    glFlush();
  }
  cout << num_triangles;
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = ((float)r) / 255.0f;
  buffer[y][x][1] = ((float)g) / 255.0f;
  buffer[y][x][2] = ((float)b) / 255.0f;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(WIDTH - 1, HEIGHT - 1);
  #endif
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}