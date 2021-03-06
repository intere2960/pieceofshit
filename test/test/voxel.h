#ifndef VOXEL_H_INCLUDED
#define VOXEL_H_INCLUDED

#include <vector>
#include <string>
#include "algebra3.h"
#include "zomestruc.h"
#include "glm.h"

class voxel
{
public:
    voxel(int t_color, int t_size, float t_scale);
    voxel(int t_color, int t_size, float t_scale, vec3 t_position, vec3 t_rotation);
    voxel(voxel t, vec3 t_position, vec3 t_rotation);

    vec3 position;
    vec3 rotation;
    int color;
    int stick_size;
    float scale;
    int face_toward[6];
    std::vector<vec3> toward_vector;
    float plane_d[6];

    vec3 vertex_p[8];
    std::vector<int> face_p[8];
    vec3 edge_p[12];
    std::vector<int> face_edge[12];
    vec2 edge_point[12];

    bool show = true;
	bool inner_kill = false;
    std::vector<int> coord;
    std::vector<vec3> coord_origin;
};

void assign_coord(voxel &judge, vec3 &origin);
void addexist_toward(std::vector<voxel> &all_voxel, voxel & check);
void oct_tree(std::vector<voxel> &all_voxel, int start, int end, int depth, vec3 origin, int coord_id);
int search_coord(std::vector<voxel> &all_voxel, int start, int end, vec3 &p, int depth, vec3 &ball_error);
void cross_edge(std::vector<voxel> &all_voxel, vec3 &p1, vec3 &p2, int index1, int index2, vec3 &ball_error);
void voxel_zometool(std::vector<voxel> &all_voxel, std::vector<std::vector<zomeconn>> &zome_queue, std::vector<std::vector<int>> &region, vec3 &angle);
void voxelization(GLMmodel *model, std::vector<voxel> &all_voxel, std::vector<std::vector<zomeconn>> &zome_queue, vec3 &bounding_max, vec3 &bounding_min, vec3 &bound_center, vec3 &angle, int v_color, int v_size);
void simple_voxelization(GLMmodel *model, std::vector<voxel> &all_voxel, vec3 &bounding_max, vec3 &bounding_min, vec3 &bound_center, vec3 &angle, float length);
void output_voxel(std::vector<voxel> &all_voxel, int piece_id, std::string model_file);
void voxel_txt(std::vector<voxel> &all_voxel, std::string &filename);
void voxel_parser(std::vector<voxel> &all_voxel, std::string &filename);
void struct_voxelization(GLMmodel *model, std::string &model_file);
void voxel_shell(std::vector<voxel> &all_voxel, std::string model_file);
void voxel_inner_shell(GLMmodel *model, std::string &model_file, float inner_shell_edge);

#endif // VOXEL_H_INCLUDED
