#include <algorithm>
#include <string>
#include <fstream>
#include <omp.h>
#include "voxel.h"
#include "zomedir.h"
#include "global.h"

voxel::voxel(int t_color, int t_size, float t_scale)
{
	color = t_color;
	stick_size = t_size;
	scale = t_scale;

	position = vec3(0.0, 0.0, 0.0);
	rotation = vec3(0.0, 0.0, 0.0);

	toward_vector.push_back(vec3(1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(-1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(0.0, 1.0, 0.0));
	toward_vector.push_back(vec3(0.0, -1.0, 0.0));
	toward_vector.push_back(vec3(0.0, 0.0, 1.0));
	toward_vector.push_back(vec3(0.0, 0.0, -1.0));

	face_toward[0] = -1;
	face_toward[1] = -1;
	face_toward[2] = -1;
	face_toward[3] = -1;
	face_toward[4] = -1;
	face_toward[5] = -1;

	for (int i = 0; i < 8; i += 1){
		int dx = 1;
		int dy = 1;
		int dz = 1;

		if (i % 2 == 1)
			dx = -1;
		if ((i / 2) % 2 == 1)
			dy = -1;
		if (i / 4 == 1)
			dz = -1;
		vec3 temp = position + scale * vec3((float)dx, (float)dy, (float)dz);
		vertex_p[i] = temp;
	}

	edge_p[0] = position + scale * vec3(1, 1, 0);
	edge_p[1] = position + scale * vec3(0, 1, 1);
	edge_p[2] = position + scale * vec3(-1, 1, 0);
	edge_p[3] = position + scale * vec3(0, 1, -1);
	edge_p[4] = position + scale * vec3(1, 0, 1);
	edge_p[5] = position + scale * vec3(-1, 0, 1);
	edge_p[6] = position + scale * vec3(-1, 0, -1);
	edge_p[7] = position + scale * vec3(1, 0, -1);
	edge_p[8] = position + scale * vec3(1, -1, 0);
	edge_p[9] = position + scale * vec3(0, -1, 1);
	edge_p[10] = position + scale * vec3(-1, -1, 0);
	edge_p[11] = position + scale * vec3(0, -1, -1);

	plane_d[0] = vertex_p[0] * toward_vector.at(0);
	plane_d[1] = vertex_p[1] * toward_vector.at(1);
	plane_d[2] = vertex_p[0] * toward_vector.at(2);
	plane_d[3] = vertex_p[2] * toward_vector.at(3);
	plane_d[4] = vertex_p[0] * toward_vector.at(4);
	plane_d[5] = vertex_p[4] * toward_vector.at(5);

	for (int i = 0; i < 8; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(vertex_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_p[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 12; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(edge_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_edge[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 8; i += 1){
		for (int j = i; j < 8; j += 1){
			vec3 temp = (vertex_p[i] + vertex_p[j]) / 2;
			for (int k = 0; k < 12; k += 1){
				if ((temp - edge_p[k]).length() < 0.001){
					edge_point[k][0] = (float)i;
					edge_point[k][1] = (float)j;
					break;
				}
			}
		}
	}
}

voxel::voxel(int t_color, int t_size, float t_scale, vec3 t_position, vec3 t_rotation)
{
	color = t_color;
	stick_size = t_size;
	scale = t_scale;

	position = t_position;
	rotation = t_rotation;
	
	toward_vector.push_back(vec3(1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(-1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(0.0, 1.0, 0.0));
	toward_vector.push_back(vec3(0.0, -1.0, 0.0));
	toward_vector.push_back(vec3(0.0, 0.0, 1.0));
	toward_vector.push_back(vec3(0.0, 0.0, -1.0));

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(1.0, 0.0, 0.0), rotation[0]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(0.0, 1.0, 0.0), rotation[1]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(0.0, 0.0, 1.0), rotation[2]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	face_toward[0] = -1;
	face_toward[1] = -1;
	face_toward[2] = -1;
	face_toward[3] = -1;
	face_toward[4] = -1;
	face_toward[5] = -1;

	for (int i = 0; i < 8; i += 1){
		int dx = 0;
		int dy = 2;
		int dz = 4;

		if (i % 2 == 1)
			dx = 1;
		if ((i / 2) % 2 == 1)
			dy = 3;
		if (i / 4 == 1)
			dz = 5;
		vec3 temp = position + scale * (toward_vector.at(dx) + toward_vector.at(dy) + toward_vector.at(dz));
		vertex_p[i] = temp;
	}

	edge_p[0] = position + scale * (toward_vector.at(0) + toward_vector.at(2)); //0 2
	edge_p[1] = position + scale * (toward_vector.at(2) + toward_vector.at(4)); //2 4
	edge_p[2] = position + scale * (toward_vector.at(1) + toward_vector.at(2)); //1 2
	edge_p[3] = position + scale * (toward_vector.at(2) + toward_vector.at(5)); //2 5
	edge_p[4] = position + scale * (toward_vector.at(0) + toward_vector.at(4)); //0 4
	edge_p[5] = position + scale * (toward_vector.at(1) + toward_vector.at(4)); //1 4
	edge_p[6] = position + scale * (toward_vector.at(1) + toward_vector.at(5)); //1 5
	edge_p[7] = position + scale * (toward_vector.at(0) + toward_vector.at(5)); //0 5
	edge_p[8] = position + scale * (toward_vector.at(0) + toward_vector.at(3)); //0 3
	edge_p[9] = position + scale * (toward_vector.at(3) + toward_vector.at(4)); //3 4
	edge_p[10] = position + scale * (toward_vector.at(1) + toward_vector.at(3)); //1 3
	edge_p[11] = position + scale * (toward_vector.at(3) + toward_vector.at(5)); //3 5

	plane_d[0] = vertex_p[0] * toward_vector.at(0);
	plane_d[1] = vertex_p[1] * toward_vector.at(1);
	plane_d[2] = vertex_p[0] * toward_vector.at(2);
	plane_d[3] = vertex_p[2] * toward_vector.at(3);
	plane_d[4] = vertex_p[0] * toward_vector.at(4);
	plane_d[5] = vertex_p[4] * toward_vector.at(5);

	for (int i = 0; i < 8; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(vertex_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_p[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 12; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(edge_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_edge[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 8; i += 1){
		for (int j = i; j < 8; j += 1){
			vec3 temp = (vertex_p[i] + vertex_p[j]) / 2;
			for (int k = 0; k < 12; k += 1){
				if ((temp - edge_p[k]).length() < 0.001){
					edge_point[k][0] = (float)i;
					edge_point[k][1] = (float)j;
					break;
				}
			}
		}
	}
}

voxel::voxel(voxel t, vec3 t_position, vec3 t_rotation)
{
	color = t.color;
	stick_size = t.stick_size;
	scale = t.scale;

	position = t_position;
	rotation = t_rotation;

	toward_vector.push_back(vec3(1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(-1.0, 0.0, 0.0));
	toward_vector.push_back(vec3(0.0, 1.0, 0.0));
	toward_vector.push_back(vec3(0.0, -1.0, 0.0));
	toward_vector.push_back(vec3(0.0, 0.0, 1.0));
	toward_vector.push_back(vec3(0.0, 0.0, -1.0));

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(1.0, 0.0, 0.0), rotation[0]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(0.0, 1.0, 0.0), rotation[1]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	for (int i = 0; i < 6; i += 1){
		mat4 R = rotation3D(vec3(0.0, 0.0, 1.0), rotation[2]);
		toward_vector.at(i) = R * toward_vector.at(i);
	}

	face_toward[0] = -1;
	face_toward[1] = -1;
	face_toward[2] = -1;
	face_toward[3] = -1;
	face_toward[4] = -1;
	face_toward[5] = -1;

	for (int i = 0; i < 8; i += 1){
		int dx = 0;
		int dy = 2;
		int dz = 4;

		if (i % 2 == 1)
			dx = 1;
		if ((i / 2) % 2 == 1)
			dy = 3;
		if (i / 4 == 1)
			dz = 5;
		vec3 temp = position + scale * (toward_vector.at(dx) + toward_vector.at(dy) + toward_vector.at(dz));
		vertex_p[i] = temp;
	}

	edge_p[0] = position + scale * (toward_vector.at(0) + toward_vector.at(2)); //0 2
	edge_p[1] = position + scale * (toward_vector.at(2) + toward_vector.at(4)); //2 4
	edge_p[2] = position + scale * (toward_vector.at(1) + toward_vector.at(2)); //1 2
	edge_p[3] = position + scale * (toward_vector.at(2) + toward_vector.at(5)); //2 5
	edge_p[4] = position + scale * (toward_vector.at(0) + toward_vector.at(4)); //0 4
	edge_p[5] = position + scale * (toward_vector.at(1) + toward_vector.at(4)); //1 4
	edge_p[6] = position + scale * (toward_vector.at(1) + toward_vector.at(5)); //1 5
	edge_p[7] = position + scale * (toward_vector.at(0) + toward_vector.at(5)); //0 5
	edge_p[8] = position + scale * (toward_vector.at(0) + toward_vector.at(3)); //0 3
	edge_p[9] = position + scale * (toward_vector.at(3) + toward_vector.at(4)); //3 4
	edge_p[10] = position + scale * (toward_vector.at(1) + toward_vector.at(3)); //1 3
	edge_p[11] = position + scale * (toward_vector.at(3) + toward_vector.at(5)); //3 5

	plane_d[0] = vertex_p[0] * toward_vector.at(0);
	plane_d[1] = vertex_p[1] * toward_vector.at(1);
	plane_d[2] = vertex_p[0] * toward_vector.at(2);
	plane_d[3] = vertex_p[2] * toward_vector.at(3);
	plane_d[4] = vertex_p[0] * toward_vector.at(4);
	plane_d[5] = vertex_p[4] * toward_vector.at(5);

	for (int i = 0; i < 8; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(vertex_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_p[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 12; i += 1){
		for (int j = 0; j < 6; j += 1){
			if (fabs(edge_p[i] * toward_vector.at(j) - plane_d[j]) < 0.001){
				face_edge[i].push_back(j);
			}
		}
	}

	for (int i = 0; i < 8; i += 1){
		for (int j = i; j < 8; j += 1){
			vec3 temp = (vertex_p[i] + vertex_p[j]) / 2;
			for (int k = 0; k < 12; k += 1){
				if ((temp - edge_p[k]).length() < 0.001){
					edge_point[k][0] = (float)i;
					edge_point[k][1] = (float)j;
					break;
				}
			}
		}
	}
}

void assign_coord(voxel &judge, vec3 &origin){
	int id = 0;
	vec3 v = judge.position - origin;
	if (v * judge.toward_vector.at(0) <= 0){
		id += 1;
	}
	if (v * judge.toward_vector.at(2) <= 0){
		id += 2;
	}
	if (v * judge.toward_vector.at(4) <= 0){
		id += 4;
	}
	judge.coord.push_back(id);
	judge.coord_origin.push_back(origin);
}

void addexist_toward(std::vector<voxel> &all_voxel, voxel &check){
	#pragma omp parallel for
	for (int i = 0; i < 6; i += 1){
		if (check.face_toward[i] == -1){
			for (unsigned int j = 0; j < all_voxel.size(); j += 1){
				if (((check.position + check.toward_vector[i] * check.scale * 2) - all_voxel.at(j).position).length() < 0.001){
					check.face_toward[i] = j;

					int opposite_face = 1;
					if (i % 2 == 1)
						opposite_face = -1;
					all_voxel.at(j).face_toward[i + opposite_face] = all_voxel.size();
					break;
				}
			}
		}
	}
}

void oct_tree(std::vector<voxel> &all_voxel, int start, int end, int depth, vec3 origin, int coord_id){
	if (end - start != 1){
		if (depth == 0){
			std::sort(all_voxel.begin(),
				all_voxel.end(),
				[](voxel a, voxel b){
				return b.coord.at(b.coord.size() - 1) > a.coord.at(a.coord.size() - 1);
			});
		}
		else{
			int dx = 0;
			int dy = 2;
			int dz = 4;

			if (coord_id % 2 == 1)
				dx = 1;
			if ((coord_id / 2) % 2 == 1)
				dy = 3;
			if (coord_id / 4 == 1)
				dz = 5;

			origin += all_voxel.at(start).scale * pow((all_voxel.size() / pow(8.0f, depth)), 1.0f / 3.0f) * (all_voxel.at(start).toward_vector.at(dx) + all_voxel.at(start).toward_vector.at(dy) + all_voxel.at(start).toward_vector.at(dz));

			for (int i = start; i < end; i += 1){
				assign_coord(all_voxel.at(i), origin);
			}
			std::sort(all_voxel.begin() + start,
				all_voxel.begin() + end,
				[](voxel a, voxel b){
				return b.coord.at(b.coord.size() - 1) > a.coord.at(a.coord.size() - 1);
			});
		}

		int d = (end - start) / 8;
		oct_tree(all_voxel, start + 0 * d, start + 1 * d, depth + 1, origin, 0);
		oct_tree(all_voxel, start + 1 * d, start + 2 * d, depth + 1, origin, 1);
		oct_tree(all_voxel, start + 2 * d, start + 3 * d, depth + 1, origin, 2);
		oct_tree(all_voxel, start + 3 * d, start + 4 * d, depth + 1, origin, 3);
		oct_tree(all_voxel, start + 4 * d, start + 5 * d, depth + 1, origin, 4);
		oct_tree(all_voxel, start + 5 * d, start + 6 * d, depth + 1, origin, 5);
		oct_tree(all_voxel, start + 6 * d, start + 7 * d, depth + 1, origin, 6);
		oct_tree(all_voxel, start + 7 * d, start + 8 * d, depth + 1, origin, 7);
	}
}

int search_coord(std::vector<voxel> &all_voxel, int start, int end, vec3 &p, int depth, vec3 &ball_error)
{
	if (end - start != 1){
		int id = 0;
		vec3 now_o = all_voxel.at(start).coord_origin.at(depth) + ball_error;
		vec3 v = p - now_o;

		if (v * all_voxel.at(start).toward_vector.at(0) <= 0){
			id += 1;
		}
		if (v * all_voxel.at(start).toward_vector.at(2) <= 0){
			id += 2;
		}
		if (v * all_voxel.at(start).toward_vector.at(4) <= 0){
			id += 4;
		}

		depth += 1;
		int d = (int)(all_voxel.size() / pow(8.0f, depth));

		return search_coord(all_voxel, start + d * id, start + d * (id + 1), p, depth, ball_error);
	}
	return start;
}

void rebuild_facetoward(std::vector<voxel> &all_voxel)
{
	#pragma omp parallel for
	for (int i = 0; i < all_voxel.size(); i += 1){
		for (int j = 0; j < 6; j += 1){
			all_voxel.at(i).face_toward[j] = -1;
		}
	}

	#pragma omp parallel for
	for (int i = 0; i < all_voxel.size(); i += 1){
		for (int j = 0; j < 6; j += 1){
			if (all_voxel.at(i).face_toward[j] == -1){
				for (unsigned int k = 0; k < all_voxel.size(); k += 1){
					if (((all_voxel.at(i).position + all_voxel.at(i).toward_vector[j] * all_voxel.at(i).scale * 2) - all_voxel.at(k).position).length() < 0.001){
						all_voxel.at(i).face_toward[j] = k;

						int opposite_face = 1;
						if (j % 2 == 1)
							opposite_face = -1;
						all_voxel.at(k).face_toward[j + opposite_face] = i;
						break;
					}
				}
			}
		}
	}
}

void cross_edge(std::vector<voxel> &all_voxel, vec3 &p1, vec3 &p2, int index1, int index2, vec3 &ball_error)
{
	if ((index1 != index2) && ((p1 - p2).length() > 0.001)){
		bool judge = false;
		for (int i = 0; i < 6; i += 1){
			if (all_voxel.at(index1).face_toward[i] == index2)
				judge = true;
		}
		if (!judge){
			vec3 new_p = (p1 + p2) / 2;
			int new_index = search_coord(all_voxel, 0, all_voxel.size(), new_p, 0, ball_error);
			cross_edge(all_voxel, p1, new_p, index1, new_index, ball_error);
			cross_edge(all_voxel, p2, new_p, index2, new_index, ball_error);
		}

		if (all_voxel.at(index1).show)
			all_voxel.at(index1).show = false;
		if (all_voxel.at(index2).show)
			all_voxel.at(index2).show = false;
	}
}

void voxel_zometool(std::vector<voxel> &all_voxel, std::vector<std::vector<zomeconn>> &zome_queue, std::vector<std::vector<int>> &region, vec3 &angle){
	zomedir t;
	for (unsigned int i = 0; i < region.size(); i += 1){
		if (region.at(i).size() > 0){
			std::vector<int> done;
			for (unsigned int j = 0; j < region.at(i).size(); j += 1){
				bool p[8] = { false };
				bool e[12] = { false };

				for (int k = 0; k < 6; k += 1){
					if (all_voxel.at(all_voxel.at(region.at(i).at(j)).face_toward[k]).show){
						if ((unsigned)(find(done.begin(), done.end(), all_voxel.at(region.at(i).at(j)).face_toward[k]) - done.begin()) < done.size()){
							for (int a = 0; a < 8; a += 1){
								if (!p[a]){
									if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_p[a].begin(), all_voxel.at(region.at(i).at(j)).face_p[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_p[a].begin()) < all_voxel.at(region.at(i).at(j)).face_p[a].size()){
										p[a] = true;
									}
								}
							}

							for (int a = 0; a < 12; a += 1){
								if (!e[a]){
									if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_edge[a].begin(), all_voxel.at(region.at(i).at(j)).face_edge[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_edge[a].begin()) < all_voxel.at(region.at(i).at(j)).face_edge[a].size()){
										e[a] = true;
									}
								}
							}
						}
						else{
							for (int a = 0; a < 8; a += 1){
								if (!p[a]){
									if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_p[a].begin(), all_voxel.at(region.at(i).at(j)).face_p[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_p[a].begin()) < all_voxel.at(region.at(i).at(j)).face_p[a].size()){
										bool exist = false;
										for (unsigned int b = 0; b < zome_queue.at(COLOR_WHITE).size(); b += 1){
											if ((zome_queue.at(COLOR_WHITE).at(b).position - all_voxel.at(region.at(i).at(j)).vertex_p[a]).length() < 0.001){
												exist = true;
												p[a] = true;
												break;
											}
										}

										if (!exist){
											zomeconn new_point;
											new_point.position = all_voxel.at(region.at(i).at(j)).vertex_p[a];
											new_point.color = COLOR_WHITE;
											zome_queue.at(COLOR_WHITE).push_back(new_point);
											p[a] = true;
										}
									}
								}
							}

							for (int a = 0; a < 12; a += 1){
								if (!e[a]){
									if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_edge[a].begin(), all_voxel.at(region.at(i).at(j)).face_edge[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_edge[a].begin()) < all_voxel.at(region.at(i).at(j)).face_edge[a].size()){
										bool exist = false;
										for (unsigned int b = 0; b < zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size(); b += 1){
											if ((zome_queue.at(all_voxel.at(region.at(i).at(j)).color).at(b).position - all_voxel.at(region.at(i).at(j)).edge_p[a]).length() < 0.001){
												exist = true;
												e[a] = true;
												break;
											}
										}

										if (!exist){
											zomeconn new_edge;
											new_edge.position = all_voxel.at(region.at(i).at(j)).edge_p[a];
											new_edge.color = all_voxel.at(region.at(i).at(j)).color;
											new_edge.size = all_voxel.at(region.at(i).at(j)).stick_size;

											int index[2];
											for (int b = 0; b < 2; b += 1){
												for (unsigned d = 0; d < zome_queue.at(COLOR_WHITE).size(); d += 1){
													if ((all_voxel.at(region.at(i).at(j)).vertex_p[(int)all_voxel.at(region.at(i).at(j)).edge_point[a][b]] - zome_queue.at(COLOR_WHITE).at(d).position).length() < 0.001){
														index[b] = d;
														break;
													}
												}
											}
											new_edge.fromindex = vec2(COLOR_WHITE, (float)index[0]);
											new_edge.towardindex = vec2(COLOR_WHITE, (float)index[1]);

											vec3 v = (new_edge.position - zome_queue.at(COLOR_WHITE).at(index[0]).position).normalize();
											mat4 R = rotation3D(vec3(0.0, 0.0, 1.0), angle[2]).inverse();
											v = R * v;
											R = rotation3D(vec3(0.0, 1.0, 0.0), angle[1]).inverse();
											v = R * v;
											R = rotation3D(vec3(1.0, 0.0, 0.0), angle[0]).inverse();
											v = R * v;

											new_edge.fromface = t.dir_face(v);
											new_edge.towardface = t.opposite_face(new_edge.fromface);

											zome_queue.at(COLOR_WHITE).at(index[0]).connect_stick.push_back(vec2((float)all_voxel.at(region.at(i).at(j)).color, (float)zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size()));
											zome_queue.at(COLOR_WHITE).at(index[1]).connect_stick.push_back(vec2((float)all_voxel.at(region.at(i).at(j)).color, (float)zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size()));
											zome_queue.at(all_voxel.at(region.at(i).at(j)).color).push_back(new_edge);
											e[a] = true;
										}
									}
								}
							}
						}
					}
					else{
						for (int a = 0; a < 8; a += 1){
							if (!p[a]){
								if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_p[a].begin(), all_voxel.at(region.at(i).at(j)).face_p[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_p[a].begin()) < all_voxel.at(region.at(i).at(j)).face_p[a].size()){
									bool exist = false;
									for (unsigned int b = 0; b < zome_queue.at(COLOR_WHITE).size(); b += 1){
										if ((zome_queue.at(COLOR_WHITE).at(b).position - all_voxel.at(region.at(i).at(j)).vertex_p[a]).length() < 0.001){
											exist = true;
											p[a] = true;
											break;
										}
									}

									if (!exist){
										zomeconn new_point;
										new_point.position = all_voxel.at(region.at(i).at(j)).vertex_p[a];
										new_point.color = COLOR_WHITE;
										zome_queue.at(COLOR_WHITE).push_back(new_point);
										p[a] = true;
									}
								}
							}
						}

						for (int a = 0; a < 12; a += 1){
							if (!e[a]){
								if ((unsigned)(find(all_voxel.at(region.at(i).at(j)).face_edge[a].begin(), all_voxel.at(region.at(i).at(j)).face_edge[a].end(), k) - all_voxel.at(region.at(i).at(j)).face_edge[a].begin()) < all_voxel.at(region.at(i).at(j)).face_edge[a].size()){
									bool exist = false;
									for (unsigned int b = 0; b < zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size(); b += 1){
										if ((zome_queue.at(all_voxel.at(region.at(i).at(j)).color).at(b).position - all_voxel.at(region.at(i).at(j)).edge_p[a]).length() < 0.001){
											exist = true;
											e[a] = true;
											break;
										}
									}

									if (!exist){
										zomeconn new_edge;
										new_edge.position = all_voxel.at(region.at(i).at(j)).edge_p[a];
										new_edge.color = all_voxel.at(region.at(i).at(j)).color;
										new_edge.size = all_voxel.at(region.at(i).at(j)).stick_size;

										int index[2];
										for (int b = 0; b < 2; b += 1){
											for (unsigned d = 0; d < zome_queue.at(COLOR_WHITE).size(); d += 1){
												if ((all_voxel.at(region.at(i).at(j)).vertex_p[(int)all_voxel.at(region.at(i).at(j)).edge_point[a][b]] - zome_queue.at(COLOR_WHITE).at(d).position).length() < 0.001){
													index[b] = d;
													break;
												}
											}
										}

										new_edge.fromindex = vec2(COLOR_WHITE, (float)index[0]);
										new_edge.towardindex = vec2(COLOR_WHITE, (float)index[1]);

										vec3 v = (new_edge.position - zome_queue.at(COLOR_WHITE).at(index[0]).position).normalize();										
										mat4 R = rotation3D(vec3(0.0, 0.0, 1.0), angle[2]).inverse();
										v = R * v;
										R = rotation3D(vec3(0.0, 1.0, 0.0), angle[1]).inverse();
										v = R * v;
										R = rotation3D(vec3(1.0, 0.0, 0.0), angle[0]).inverse();
										v = R * v;

										new_edge.fromface = t.dir_face(v);
										new_edge.towardface = t.opposite_face(new_edge.fromface);

										zome_queue.at(COLOR_WHITE).at(index[0]).connect_stick.push_back(vec2((float)all_voxel.at(region.at(i).at(j)).color, (float)zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size()));
										zome_queue.at(COLOR_WHITE).at(index[1]).connect_stick.push_back(vec2((float)all_voxel.at(region.at(i).at(j)).color, (float)zome_queue.at(all_voxel.at(region.at(i).at(j)).color).size()));
										zome_queue.at(all_voxel.at(region.at(i).at(j)).color).push_back(new_edge);
										e[a] = true;
									}
								}
							}
						}
					}
				}
				done.push_back(region.at(i).at(j));
			}
		}
	}
}

void voxelization(GLMmodel *model, std::vector<voxel> &all_voxel, std::vector<std::vector<zomeconn>> &zome_queue, vec3 &bounding_max, vec3 &bounding_min, vec3 &bound_center, vec3 &angle, int v_color, int v_size)
{
	zomedir t;
	voxel start(v_color, v_size, t.color_length(v_color, v_size) / 2.0f, bound_center, angle);
	vec3 origin = vec3(bound_center) + start.scale * (start.toward_vector.at(0) + start.toward_vector.at(2) + start.toward_vector.at(4));

	int max_d = -1;
	for (int i = 0; i < 3; i += 1){
		float pos = ceil((bounding_max[i] - origin[i]) / (start.scale * 2)),
			neg = ceil((origin[i] - bounding_min[i]) / (start.scale * 2));
		if (pos > max_d)
			max_d = (int)pos;
		if (neg > max_d)
			max_d = (int)neg;
	}

	int half_edge = 1;
	while (true){
		if (half_edge < max_d)
			half_edge *= 2;
		else{
			max_d = half_edge;
			break;
		}
	}

	start.position += (float)(max_d - 1) * (start.toward_vector.at(1) + start.toward_vector.at(3) + start.toward_vector.at(5)) * start.scale * 2.0f;

	for (int i = 0; i < max_d * 2; i += 1){
		for (int j = 0; j < max_d * 2; j += 1){
			for (int k = 0; k < max_d * 2; k += 1){
				vec3 new_p = start.position + start.scale * 2.0f * ((float)i * start.toward_vector.at(0) + (float)j * start.toward_vector.at(2) + (float)k * start.toward_vector.at(4));
				voxel temp(start, new_p, start.rotation);
				assign_coord(temp, origin);
				all_voxel.push_back(temp);
			}
		}
	}

	oct_tree(all_voxel, 0, all_voxel.size(), 0, origin, -1);
	rebuild_facetoward(all_voxel);

	#pragma omp parallel for
	for(int i = 0; i < model->numtriangles; i += 1){
		#pragma omp parallel for
		for(int k = 0; k < 8; k += 1){
			int dx = 0;
			int dy = 2;
			int dz = 4;

			if (k % 2 == 1)
				dx = 1;
			if ((k / 2) % 2 == 1)
				dy = 3;
			if (k / 4 == 1)
				dz = 5;

			vec3 ball_error = (all_voxel.at(0).toward_vector.at(dx) + all_voxel.at(0).toward_vector.at(dy) + all_voxel.at(0).toward_vector.at(dz)) * NODE_DIAMETER / 2.0;

			int test_coord[3];
			vec3 test_p[3];
			for(int j = 0; j < 3; j += 1){
				test_p[j] = vec3(model->vertices->at(3 * model->triangles->at(i).vindices[j] + 0), model->vertices->at(3 * model->triangles->at(i).vindices[j] + 1), model->vertices->at(3 * model->triangles->at(i).vindices[j] + 2));
				test_coord[j] = search_coord(all_voxel, 0, all_voxel.size(), test_p[j], 0, ball_error);
				if(all_voxel.at(test_coord[j]).show)
					all_voxel.at(test_coord[j]).show = false;
			}
			cross_edge(all_voxel, test_p[0], test_p[1], test_coord[0], test_coord[1], ball_error);
			cross_edge(all_voxel, test_p[1], test_p[2], test_coord[1], test_coord[2], ball_error);
			cross_edge(all_voxel, test_p[2], test_p[0], test_coord[2], test_coord[0], ball_error);
		}
	}

	bool *use = new bool[all_voxel.size()];
	#pragma omp parallel for
	for(int i = 0; i < all_voxel.size(); i += 1){
		if(!all_voxel.at(i).show)
			use[i] = true;
		else
			use[i] = false;
	}

	std::vector<std::vector<int>> region;
	while(true){
		std::vector<int> t_queue;
		for(unsigned int i = 0; i < all_voxel.size(); i += 1){
			if(!use[i]){
				t_queue.push_back(i);
				use[i] = true;
				break;
			}
		}

		for(unsigned int i = 0; i < t_queue.size(); i += 1){
			for(int j = 0; j < 6; j += 1){
				if(all_voxel.at(t_queue.at(i)).face_toward[j] != -1){
					if(!use[all_voxel.at(t_queue.at(i)).face_toward[j]]){
						t_queue.push_back(all_voxel.at(t_queue.at(i)).face_toward[j]);
						use[all_voxel.at(t_queue.at(i)).face_toward[j]] = true;
					}
				}
			}
		}

		region.push_back(t_queue);

		bool test = true;
		for(unsigned int i = 0; i < all_voxel.size(); i += 1){
			if(!use[i]){
				test = false;
				break;
			}
		}

		if(test)
			break;
	}

	#pragma omp parallel for
	for(int i = 0; i < region.size(); i += 1){
		bool test = false;
		for(unsigned int j = 0; j < region.at(i).size(); j += 1){
			if(test){
				all_voxel.at(region.at(i).at(j)).show = false;
				region.at(i).erase(region.at(i).begin() + j);
				j -= 1;
			}
			else{
				for(int k = 0 ; k < 6; k += 1){
					if(all_voxel.at(region.at(i).at(j)).face_toward[k] == -1){
						j = -1;
						test = true;
						break;
					}
				}
			}
		}
	}

	voxel_zometool(all_voxel, zome_queue, region, angle);
}

void output_voxel(std::vector<voxel> &all_voxel, int piece_id)
{
	GLMmodel *cube = glmReadOBJ("test_model/cube.obj");
	GLMmodel *output = glmCopy(cube);
	int num = 0;
	for (unsigned int i = 0; i < all_voxel.size(); i += 1){
		if (all_voxel.at(i).show){
			if (num == 0){
				glmScale(output, all_voxel.at(i).scale);
				glmR(output, all_voxel.at(i).rotation);
				glmT(output, all_voxel.at(i).position);
			}
			else{
				GLMmodel *temp = glmCopy(cube);
				glmScale(temp, all_voxel.at(i).scale);
				glmR(temp, all_voxel.at(i).rotation);
				glmT(temp, all_voxel.at(i).position);
				glmCombine(output, temp);
			}
			num += 1;
		}
	}	

	if (num != 0){
		std::string s = "voxel_" + std::to_string(piece_id) + ".obj";
		glmWriteOBJ(output, my_strdup(s.c_str()), GLM_NONE);
	}
}

void output_zometool(std::vector<voxel> &all_voxel, std::vector<std::vector<zomeconn>> &zome_queue, int piece_id)
{
	if (zome_queue.at(3).size() != 0){
		std::string s = "voxel_zome_" + std::to_string(piece_id) + ".obj";
		std::ofstream os(s);
		GLMmodel *z_b, *b_s, *b_m, *b_l, *r_s, *r_m, *r_l, *y_s, *y_m, *y_l;
		z_b = glmReadOBJ("test_model/zometool/zomeball.obj");
		b_s = glmReadOBJ("test_model/zometool/BlueS.obj");
		b_m = glmReadOBJ("test_model/zometool/BlueM.obj");
		b_l = glmReadOBJ("test_model/zometool/BlueL.obj");
		r_s = glmReadOBJ("test_model/zometool/RedS.obj");
		r_m = glmReadOBJ("test_model/zometool/RedM.obj");
		r_l = glmReadOBJ("test_model/zometool/RedL.obj");
		y_s = glmReadOBJ("test_model/zometool/YellowS.obj");
		y_m = glmReadOBJ("test_model/zometool/YellowM.obj");
		y_l = glmReadOBJ("test_model/zometool/YellowL.obj");

		os << "mtllib color.mtl" << std::endl;

		for (unsigned int i = 0; i < zome_queue.size(); i += 1){
			for (unsigned int j = 0; j < zome_queue.at(i).size(); j += 1){
				GLMmodel *temp_model = NULL;
				if (i == COLOR_WHITE){
					temp_model = glmCopy(z_b);
					glmR(temp_model, all_voxel.at(0).rotation);
					glmRT(temp_model, vec3(0.0, 0.0, 0.0), zome_queue.at(i).at(j).position);
				}
				else{
					zomedir t;
					if (i == COLOR_BLUE){
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(b_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(b_m);
						}
						else{
							temp_model = glmCopy(b_l);
						}
					}
					else if (i == COLOR_RED){
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(r_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(r_m);
						}
						else{
							temp_model = glmCopy(r_l);
						}
					}
					else {
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(y_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(y_m);
						}
						else{
							temp_model = glmCopy(y_l);
						}
					}

					glmRT(temp_model, vec3(0.0, t.roll(zome_queue.at(i).at(j).fromface), 0.0), vec3(0.0, 0.0, 0.0));
					glmRT(temp_model, vec3(0.0, t.phi(zome_queue.at(i).at(j).fromface), t.theta(zome_queue.at(i).at(j).fromface)), vec3(0.0, 0.0, 0.0));
					glmR(temp_model, all_voxel.at(0).rotation);
					glmRT(temp_model, vec3(0.0, 0.0, 0.0), zome_queue.at(i).at(j).position);
				}

				for (unsigned int k = 1; k <= temp_model->numvertices; k += 1){
					os << "v " << temp_model->vertices->at(3 * k + 0) << " " << temp_model->vertices->at(3 * k + 1) << " " << temp_model->vertices->at(3 * k + 2) << std::endl;
				}
			}
		}

		os << std::endl;

		int num_v = 0;
		for (unsigned int i = 0; i < zome_queue.size(); i += 1){
			for (unsigned int j = 0; j < zome_queue.at(i).size(); j += 1){
				GLMmodel *temp_model = NULL;
				if (i == COLOR_WHITE){
					temp_model = glmCopy(z_b);
					os << "usemtl white" << std::endl;
				}
				else{
					if (i == COLOR_BLUE){
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(b_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(b_m);
						}
						else{
							temp_model = glmCopy(b_l);
						}
						os << "usemtl blue" << std::endl;
					}
					else if (i == COLOR_RED){
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(r_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(r_m);
						}
						else{
							temp_model = glmCopy(r_l);
						}
						os << "usemtl red" << std::endl;
					}
					else {
						if (zome_queue.at(i).at(j).size == SIZE_S){
							temp_model = glmCopy(y_s);
						}
						else if (zome_queue.at(i).at(j).size == SIZE_M){
							temp_model = glmCopy(y_m);
						}
						else{
							temp_model = glmCopy(y_l);
						}
						os << "usemtl yellow" << std::endl;
					}
				}

				for (unsigned int k = 0; k < temp_model->numtriangles; k += 1){
					os << "f " << temp_model->triangles->at(k).vindices[0] + num_v << " " << temp_model->triangles->at(k).vindices[1] + num_v << " " << temp_model->triangles->at(k).vindices[2] + num_v << std::endl;
				}

				num_v += temp_model->numvertices;
			}
		}

		os.close();
	}
}

void output_zometool(std::vector<std::vector<zomeconn>> &output_connect, std::string &filename)
{
	std::ofstream os(filename);
	GLMmodel *z_b, *b_s, *b_m, *b_l, *r_s, *r_m, *r_l, *y_s, *y_m, *y_l;
	z_b = glmReadOBJ("test_model/zometool/zomeball.obj");
	b_s = glmReadOBJ("test_model/zometool/BlueS.obj");
	b_m = glmReadOBJ("test_model/zometool/BlueM.obj");
	b_l = glmReadOBJ("test_model/zometool/BlueL.obj");
	r_s = glmReadOBJ("test_model/zometool/RedS.obj");
	r_m = glmReadOBJ("test_model/zometool/RedM.obj");
	r_l = glmReadOBJ("test_model/zometool/RedL.obj");
	y_s = glmReadOBJ("test_model/zometool/YellowS.obj");
	y_m = glmReadOBJ("test_model/zometool/YellowM.obj");
	y_l = glmReadOBJ("test_model/zometool/YellowL.obj");

	os << "mtllib color.mtl" << std::endl;

	for (unsigned int i = 0; i < output_connect.size(); i += 1){
		for (unsigned int j = 0; j < output_connect.at(i).size(); j += 1){
			GLMmodel *temp_model = NULL;
			if (i == COLOR_WHITE){
				temp_model = glmCopy(z_b);
				glmR(temp_model, vec3(0.0, 0.0, 0.0));
				glmRT(temp_model, vec3(0.0, 0.0, 0.0), output_connect.at(i).at(j).position);
			}
			else{
				zomedir t;
				if (i == COLOR_BLUE){
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(b_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(b_m);
					}
					else{
						temp_model = glmCopy(b_l);
					}
				}
				else if (i == COLOR_RED){
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(r_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(r_m);
					}
					else{
						temp_model = glmCopy(r_l);
					}
				}
				else {
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(y_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(y_m);
					}
					else{
						temp_model = glmCopy(y_l);
					}
				}

				glmRT(temp_model, vec3(0.0, t.roll(output_connect.at(i).at(j).fromface), 0.0), vec3(0.0, 0.0, 0.0));
				glmRT(temp_model, vec3(0.0, t.phi(output_connect.at(i).at(j).fromface), t.theta(output_connect.at(i).at(j).fromface)), vec3(0.0, 0.0, 0.0));
				glmR(temp_model, vec3(0.0, 0.0, 0.0));
				glmRT(temp_model, vec3(0.0, 0.0, 0.0), output_connect.at(i).at(j).position);
			}

			for (unsigned int k = 1; k <= temp_model->numvertices; k += 1){
				os << "v " << temp_model->vertices->at(3 * k + 0) << " " << temp_model->vertices->at(3 * k + 1) << " " << temp_model->vertices->at(3 * k + 2) << std::endl;
			}
		}
	}

	os << std::endl;

	int num_v = 0;
	for (unsigned int i = 0; i < output_connect.size(); i += 1){
		for (unsigned int j = 0; j < output_connect.at(i).size(); j += 1){
			GLMmodel *temp_model = NULL;
			if (i == COLOR_WHITE){
				temp_model = glmCopy(z_b);
				os << "usemtl white" << std::endl;
			}
			else{
				if (i == COLOR_BLUE){
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(b_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(b_m);
					}
					else{
						temp_model = glmCopy(b_l);
					}
					os << "usemtl blue" << std::endl;
				}
				else if (i == COLOR_RED){
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(r_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(r_m);
					}
					else{
						temp_model = glmCopy(r_l);
					}
					os << "usemtl red" << std::endl;
				}
				else {
					if (output_connect.at(i).at(j).size == SIZE_S){
						temp_model = glmCopy(y_s);
					}
					else if (output_connect.at(i).at(j).size == SIZE_M){
						temp_model = glmCopy(y_m);
					}
					else{
						temp_model = glmCopy(y_l);
					}
					os << "usemtl yellow" << std::endl;
				}
			}

			for (unsigned int k = 0; k < temp_model->numtriangles; k += 1){
				os << "f " << temp_model->triangles->at(k).vindices[0] + num_v << " " << temp_model->triangles->at(k).vindices[1] + num_v << " " << temp_model->triangles->at(k).vindices[2] + num_v << std::endl;
			}

			num_v += temp_model->numvertices;
		}
	}

	os.close();
}