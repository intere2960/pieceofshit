#include "main.h"
#include <omp.h>

void drawObj(GLMmodel *myObj)
{
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i<myObj->numtriangles; i += 1) {
		for (int v = 0; v<3; v += 1) {
			//		    glColor3fv( & myObj->vertices[ myObj->triangles[i].vindices[v]*3 ] );
			//		    glNormal3fv( & myObj->normals[ myObj->triangles[i].nindices[v]*3 ] );
			//		    glColor3f(& myObj->vertices[ myObj->triangles[i].vindices[v]*3 ]);
			//		    glNormal3fv( & myObj->facetnorms.at(3 * (i + 1)));
			glVertex3fv(&myObj->vertices->at(myObj->triangles->at(i).vindices[v] * 3));
		}
	}
	glEnd();
}

void display(void)
{
	/* display callback, clear frame buffer and z buffer,
	rotate cube and draw, swap buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_pos[0], eye_pos[1], eye_pos[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	//lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	light0_pos[0] = bound_size[0] * cos(lightTheta*M_PI / 180.0f);
	light0_pos[1] = bound_size[1] * 1.0f / sin(M_PI / 4.0f) * sin(lightTheta*M_PI / 180.0f);
	light0_pos[2] = bound_size[2] * cos(lightTheta*M_PI / 180.0f);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

	//    setShaders();

	glPushMatrix();

	glTranslatef(-bound_center[0], -bound_center[1], -bound_center[2]);

	if (show){
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		//        drawObj(&temp_piece);
		drawObj(myObj);
		//        drawObj(myObj_inner);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}

	//    drawObj(myObj_inner);
	if (show_piece)
		drawObj(&temp_piece);

	glPopMatrix();

	//draw_bounding_box();
	//draw_best_bounding_box();

	glutSwapBuffers();
	glutPostRedisplay();
}

void myReshape(int w, int h)
{
	width = w;
	height = h;
	float ratio = w * 1.0f / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(50.0, ratio, 0.1, 100000.0);
	glMatrixMode(GL_MODELVIEW);
}

void init()
{
	//    glmRT(myObj, vec3(0.0, 90.0, 0.0), vec3(0.0, 0.0, 0.0));

	computeSimpleBB(myObj->numvertices, myObj->vertices, bound_size, bounding_max, bounding_min, bound_center);

	eye_pos[2] = eye_pos[2] + 2.0f * bound_size[2];

	//    recount_normal(myObj);
	//    process_inner(myObj, myObj_inner);
	//
	////    glmRT(myObj_inner, vec3(0.0, 90.0, 0.0), vec3(0.0, 0.0, 500.0));
	//
	//    combine_inner_outfit(myObj, myObj_inner);
	//
	//    collect_edge(myObj, all_edge);
	//
	//    planes.push_back(test_plane1);
	//    planes.push_back(test_plane2);
	//    planes.push_back(test_plane3);
	//    planes.push_back(test_plane4);
	//    planes.push_back(test_plane5); //dir_plane
	////    planes.push_back(test_plane6); //dir_plane
	////    planes.push_back(test_plane7); //dir_plane
	////    planes.push_back(test_plane8); //dir_plane
	//
	//    cut_intersection(myObj, planes, face_split_by_plane, false);
	//
	//    split_face(myObj, all_edge, face_split_by_plane, planes);
	//
	//    find_loop(myObj, all_edge, planes);
	//
	//    process_piece(temp_piece, myObj, face_split_by_plane);
	//
	//    fill_hole(temp_piece, true);
}

void findzoom()
{
	zometable splite_table(SPLITE);
	zometable merge_table(MERGE);
}

void test()
{
	cout << "sdf segment" << endl;
	sdf_segment(seg, myObj, model_source);

	all_voxel.resize(seg.size());
	zome_queue.resize(seg.size());

	obb_center.resize(seg.size());
	obb_max.resize(seg.size());
	obb_min.resize(seg.size());
	obb_size.resize(seg.size());
	obb_angle.resize(seg.size());

	cout << "generate piece" << endl;
	#pragma omp parallel for
	for (int i = 0; i < seg.size(); i += 1){
		if (seg.size() > 1){
			vector<edge> fill_edge;
			collect_edge(&seg.at(i), fill_edge);
			inform_vertex(&seg.at(i), fill_edge);

			std::vector<int> single_use;
			for (unsigned int j = 0; j < fill_edge.size(); j += 1){
				if (fill_edge.at(j).face_id[1] == -1){
					if ((unsigned)(find(single_use.begin(), single_use.end(), fill_edge.at(j).index[0]) - single_use.begin()) >= single_use.size()){
						single_use.push_back(fill_edge.at(j).index[0]);
					}
					if ((unsigned)(find(single_use.begin(), single_use.end(), fill_edge.at(j).index[1]) - single_use.begin()) >= single_use.size()){
						single_use.push_back(fill_edge.at(j).index[1]);
					}
				}
			}

			find_easy_loop(&seg.at(i), fill_edge, single_use);
			fill_hole(seg.at(i), false);
		}

		std::string s = "test";
		std::string piece = s + std::to_string(i) + ".obj";
		glmWriteOBJ(&seg.at(i), my_strdup(piece.c_str()), GLM_NONE);


		cout << "piece " << i + 1 << " :" << endl;
		zome_queue.at(i).resize(4);
		//computeBestFitOBB(seg.at(i).numvertices, seg.at(i).vertices, obb_size.at(i), obb_max.at(i), obb_min.at(i), obb_center.at(i), obb_angle.at(i), start_m);
		computeSimpleBB(seg.at(i).numvertices, seg.at(i).vertices, obb_size.at(i), obb_max.at(i), obb_min.at(i), obb_center.at(i));
		//voxelization(&seg.at(i), all_voxel.at(i), zome_queue.at(i), obb_max.at(i), obb_min.at(i), obb_center.at(i), obb_angle.at(i), COLOR_BLUE, SIZE_S);
		voxelization(&seg.at(i), all_voxel.at(i), zome_queue.at(i), obb_max.at(i), obb_min.at(i), obb_center.at(i), vec3(0.0, 0.0, 0.0), COLOR_BLUE, SIZE_S);
		//voxelization(&seg.at(i), all_voxel.at(i), zome_queue.at(i), obb_max.at(i), obb_min.at(i), bound_center, vec3(0.0, 0.0, 0.0), COLOR_BLUE, SIZE_S);
		cout << "output piece " << i + 1 << endl;
		output_voxel(all_voxel.at(i), i);
		output_zometool(all_voxel.at(i).at(0).rotation, zome_queue.at(i), i);

		//for (int j = 0; j < 4; j += 1){
		//	cout << j << " : " << endl;
		//	if (j != 3){
		//		/*for (int k = 0; k < zome_queue.at(i).at(j).size(); k += 1){
		//			cout << "\t " << k << " : (" << zome_queue.at(i).at(j).at(k).fromindex[0] << " , " << zome_queue.at(i).at(j).at(k).fromindex[1]
		//			<< ") (" << zome_queue.at(i).at(j).at(k).towardindex[0] << " , " << zome_queue.at(i).at(j).at(k).towardindex[1] << ")" << endl;
		//			}*/
		//	}
		//	else{
		//		for (int k = 0; k < zome_queue.at(i).at(3).size(); k += 1){
		//			cout << "\t " << k << " : ";
		//			for (int a = 0; a < 62; a += 1){
		//				if (zome_queue.at(i).at(3).at(k).connect_stick[a] != vec2(-1.0f, -1.0f))
		//					cout << " (" << zome_queue.at(i).at(3).at(k).connect_stick[a][0] << " , " << zome_queue.at(i).at(3).at(k).connect_stick[a][1] << ")";
		//			}
		//			cout << endl;
		//			cout << "\t " << zome_queue.at(i).at(3).at(k).position[0] << " " << zome_queue.at(i).at(3).at(k).position[1] << " " << zome_queue.at(i).at(3).at(k).position[2] << endl;
		//		}
		//	}
		//}
	}
}

vector<vector<zomeconn>> test_connect(4);
vec3 delta;

void fake()
{
	zomedir t;

	vec3 p1(26.576f, 120.994f, 75.2333f);
	//vec3 p2 = p1 + t.dir->at(2) * t.color_length(COLOR_BLUE, SIZE_M) + t.dir->at(30) * t.color_length(COLOR_RED, SIZE_S) + t.dir->at(34) * t.color_length(COLOR_RED, SIZE_S) + t.dir->at(54) * t.color_length(COLOR_YELLOW, SIZE_S) + t.dir->at(35) * t.color_length(COLOR_RED, SIZE_M);
	//vec3 p2 = p1 + t.dir->at(2) * t.color_length(COLOR_BLUE, SIZE_S); 
	//cout << p2[0] << " " << p2[1] << " " << p2[2] << endl;
	vec3 temp1(61.6096f, 285.876f, 69.4521f);
	vec3 temp2(64.8425f, 288.677f, 69.6503f);
	delta = temp2 - temp1;
	vec3 p2(61.6096f, 285.876f, -25.1479f);
	//vec3 p2(61.6096f, 238.576f, -25.1479f);
	cout << delta[0] << " " << delta[1] << " " << delta[2] << endl;
	p2 += delta;

	vec3 asd = p2 - p1;
	int near_d = t.find_near_dir(asd);
	int best_s, best_i;
	cout << asd.length() << endl;
	cout << near_d << endl;
	t.find_best_zome(p1, p2, near_d, best_s, best_i);
	cout << best_s << " " << best_i << endl;

	vector<vec3> record;
	record.push_back(p1);
	connect_points_optimize(p1, p2, record);

	t.find_best_zome(p1, record.at(0), near_d, best_s, best_i);
	
	cout << record.at(0)[0] << " " << record.at(0)[1] << " " << record.at(0)[2] << endl;
	cout << best_i << " " << best_s << endl;

	zomeconn start_ball;
	start_ball.color = COLOR_WHITE;
	start_ball.position = record.at(0);
	test_connect.at(COLOR_WHITE).push_back(start_ball);

	zomeconn start_stick;
	start_stick.color = t.face_color(best_i);
	start_stick.position = (record.at(0) + p1) / 2;
	start_stick.fromface = t.opposite_face(best_i);
	start_stick.towardface = best_i;
	start_stick.size = best_s;
	test_connect.at(t.face_color(best_i)).push_back(start_stick);

	cout << "best_i : " << best_i << endl;
	test_connect.at(COLOR_WHITE).at(0).connect_stick[t.opposite_face(best_i)] = vec2(t.face_color(best_i), test_connect.at(t.face_color(best_i)).size() - 1);

	for (int i = 0; i < 62; i += 1){
		if (test_connect.at(COLOR_WHITE).at(0).connect_stick[i] != vec2(-1.0f, -1.0f)){
			cout << test_connect.at(COLOR_WHITE).at(0).connect_stick[i][0] << " " << test_connect.at(COLOR_WHITE).at(0).connect_stick[i][1] << endl;
		}
	}

	test_connect.at(t.face_color(best_i)).at(0).fromindex = vec2(COLOR_WHITE, 0);


	for (unsigned int i = 1; i < record.size(); i += 1){
		if (i < record.size() - 1){
			zomeconn temp_ball;
			temp_ball.color = COLOR_WHITE;
			temp_ball.position = record.at(i);
			test_connect.at(COLOR_WHITE).push_back(temp_ball);
		}

		cout << record.at(i)[0] << " " << record.at(i)[1] << " " << record.at(i)[2] << endl;
		t.find_best_zome(record.at(i - 1), record.at(i), near_d, best_s, best_i);
		cout << best_i << " " << best_s << endl;

		zomeconn temp_stick;
		temp_stick.color = t.face_color(best_i);
		temp_stick.position = (record.at(i) + record.at(i - 1)) / 2;
		temp_stick.fromface = t.opposite_face(best_i);
		temp_stick.towardface = best_i;
		temp_stick.size = best_s;
		if (i < record.size() - 1)
			temp_stick.fromindex = vec2(COLOR_WHITE, i);
		temp_stick.towardindex = vec2(COLOR_WHITE, i - 1);
		test_connect.at(t.face_color(best_i)).push_back(temp_stick);

		test_connect.at(COLOR_WHITE).at(i - 1).connect_stick[best_i] = vec2(t.face_color(best_i), test_connect.at(t.face_color(best_i)).size() - 1);
		if (i < record.size() - 1)
			test_connect.at(COLOR_WHITE).at(i).connect_stick[t.opposite_face(best_i)] = vec2(t.face_color(best_i), test_connect.at(t.face_color(best_i)).size() - 1);
	}

	//for (int j = 0; j < 4; j += 1){
	//	cout << j << " : " << endl;
	//	if (j != 3){
	//		for (int k = 0; k < test_connect.at(j).size(); k += 1){
	//			cout << "\t " << k << " : (" << test_connect.at(j).at(k).fromindex[0] << " , " << test_connect.at(j).at(k).fromindex[1]
	//				<< ") (" << test_connect.at(j).at(k).towardindex[0] << " , " << test_connect.at(j).at(k).towardindex[1] << ")" << endl;
	//		}
	//	}
	//	else{
	//		for (int k = 0; k < test_connect.at(3).size(); k += 1){
	//			if (test_connect.at(3).at(k).connect_stick.size() != 6){
	//				cout << "\t " << k << " : ";
	//				for (int a = 0; a < test_connect.at(3).at(k).connect_stick.size(); a += 1){
	//					cout << " (" << test_connect.at(3).at(k).connect_stick.at(a)[0] << " , " << test_connect.at(3).at(k).connect_stick.at(a)[1] << ")";
	//				}
	//				cout << endl;
	//				//cout << "\t " << test_connect.at(3).at(k).position[0] << " " << test_connect.at(3).at(k).position[1] << " " << test_connect.at(3).at(k).position[2] << endl;
	//			}
	//		}
	//	}
	//}

	output_zometool(test_connect, string("test123.obj"));
}

float forbidden_energy(float dist)
{
	zomedir t;
	vec2 p1(t.color_length(COLOR_BLUE, SIZE_S) / 3.0f, 0.0f);
	vec2 p2(t.color_length(COLOR_BLUE, SIZE_S) * 1.0f, 70.0f);
	float a = p2[0] / pow((p2[0] - p1[0]), 2);

	if (dist > p2[0]){
		return p2[1];
	}
	if (dist < p1[0]){
		return 0.0f;
	}

	return a * pow((dist - p1[0]), 2);
}

float compute_energy(vector<vector<zomeconn>> &test_connect, GLMmodel *model)
{
	float energy = 0.0f;
	zomedir t;

	for (unsigned int a = 0; a < test_connect.size(); a += 1){
		for (unsigned int i = 0; i < test_connect.at(a).size(); i += 1){
			//cout << a << " " << i << endl;
			//cout << test_connect.at(a).at(i).position[0] << " " << test_connect.at(a).at(i).position[1] << " " << test_connect.at(a).at(i).position[2] << endl;
			test_connect.at(a).at(i).surface_d = point_surface_dist(model, test_connect.at(a).at(i).position);
		}
	}

	float energy_dist = 0.0f;
	for (unsigned int a = 0; a < test_connect.size(); a += 1){
		for (unsigned int i = 0; i < test_connect.at(a).size(); i += 1){
			energy_dist += pow(test_connect.at(a).at(i).surface_d, 2) * (1.0 + forbidden_energy(test_connect.at(a).at(i).surface_d));
		}
	}
	energy_dist /= (pow(t.color_length(COLOR_BLUE, SIZE_S), 2) * (test_connect.at(COLOR_BLUE).size() + test_connect.at(COLOR_RED).size() + test_connect.at(COLOR_YELLOW).size() + test_connect.at(COLOR_WHITE).size()));

	//float energy_fair = 0.0f;
	//for (unsigned int i = 0; i < test_connect.at(COLOR_WHITE).size(); i += 1){
	//	vec3 temp_ring_p;
	//	int use_stick = 0;
	//	for (unsigned int j = 0; j < 62; j += 1){
	//		if (test_connect.at(COLOR_WHITE).at(i).connect_stick[j] != vec2(-1.0f, -1.0f)){
	//			//cout << "abcd : " << test_connect.at(COLOR_WHITE).at(i).connect_stick[j][0] << " " << test_connect.at(COLOR_WHITE).at(i).connect_stick[j][1] << endl;
	//			temp_ring_p += test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[j][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[j][1]).position;
	//			use_stick += 1;
	//		}
	//	}
	//	temp_ring_p /= use_stick;
	//	energy_fair += (test_connect.at(COLOR_WHITE).at(i).position - temp_ring_p).length2();
	//}
	//energy_fair /= (pow(t.color_length(COLOR_BLUE, SIZE_S), 2) * test_connect.at(COLOR_WHITE).size());

	//energy = energy_dist + energy_fair;
	energy = energy_dist;
	cout << "energy : " << energy << endl;

	return energy;
}

void split(vector<vector<zomeconn>> &test_connect, int s_index, GLMmodel *model)
{
	zomedir t;

	int c_index;
	float dist = 100000000000000000.0f;
	
	for (unsigned int i = 0; i < 62; i += 1){
		if (test_connect.at(COLOR_WHITE).at(s_index).connect_stick[i] != vec2(-1.0f, -1.0f)){
			vec2 test = test_connect.at(COLOR_WHITE).at(s_index).connect_stick[i];
			//cout << i << " : " << test[0] << " " << test[1] << endl;
			if (test_connect.at(test[0]).at(test[1]).surface_d < dist){
				dist = test_connect.at(test[0]).at(test[1]).surface_d;
				c_index = i;
			}
		}
	}

	vec2 test = test_connect.at(COLOR_WHITE).at(s_index).connect_stick[c_index];
	//cout << "test : " << test[0] << " " << test[1] << endl;
	int from_face = test_connect.at(test[0]).at(test[1]).fromface;
	int from_size = test_connect.at(test[0]).at(test[1]).size;

	//cout << "from_face : " << from_face << endl;
	//cout << "from_size : " << from_size << endl;

	vec3 use_ball_p = test_connect.at(COLOR_WHITE).at(s_index).position;

	//cout << "use_ball_p : " << use_ball_p[0] << " " << use_ball_p[1] << " " << use_ball_p[2] << endl;

	vec3 use_stick_p = test_connect.at(test[0]).at(test[1]).position;
	vec3 judge = use_stick_p + 0.5f * t.dir->at(from_face) * t.color_length(test[0], from_size) - use_ball_p;
	
	int toward_ball_index = test_connect.at(test[0]).at(test[1]).towardindex[1];

	//cout << "judge.length() : " << judge.length() << endl;

	if (from_face != c_index)
		from_face = c_index;

	//if (judge.length() < 0.001){
	if (toward_ball_index == s_index){
		//cout << "in" << endl;
		toward_ball_index = test_connect.at(test[0]).at(test[1]).fromindex[1];
	}

	vec3 toward_p = test_connect.at(COLOR_WHITE).at(test_connect.at(test[0]).at(test[1]).towardindex[1]).position;

	//cout << "fuck you : " << s_index << " " << toward_ball_index << endl;

	//cout << "from_face : " << from_face << endl;

	vector<zomerecord> temp;
	for (unsigned int i = 0; i < splite_table.table.at(from_face).size(); i += 1){
		for (unsigned int j = 0; j < splite_table.table.at(from_face).at(i).size(); j += 1){
			if (splite_table.table.at(from_face).at(i).at(j).origin[1] == from_size){
				temp.push_back(splite_table.table.at(from_face).at(i).at(j));
			}
		}
	}

	//cout << temp.size() << endl;

	for (unsigned int i = 0; i < temp.size(); i += 1){
		if (test_connect.at(COLOR_WHITE).at(s_index).connect_stick[(int)temp.at(i).travel_1[0]] != vec2(-1.0f, -1.0f)){
			temp.erase(temp.begin() + i);
			i -= 1;
			continue;
		}

		if (test_connect.at(COLOR_WHITE).at(toward_ball_index).connect_stick[(int)temp.at(i).travel_2[0]] != vec2(-1.0f, -1.0f)){
			temp.erase(temp.begin() + i);
			i -= 1;
			continue;
		}
	}

	//cout << temp.size() << endl;

	float choose_split = 0;
	float split_dist = 10000000000000000.0f;
	for (unsigned int i = 0; i < temp.size(); i += 1){
		vec3 test_d = use_ball_p + t.dir->at(temp.at(i).travel_1[0]) * t.face_length(temp.at(i).travel_1[0], temp.at(i).travel_1[1]);
		bool dist = (ball_surface_dist(model, test_d) < 100000000000000.0f);
		bool insect = !check_stick_intersect(model, test_d, use_ball_p) && !check_stick_intersect(model, (test_d + use_ball_p) / 2.0f, use_ball_p) && !check_stick_intersect(model, test_d, (test_d + use_ball_p) / 2.0f) && !check_stick_intersect(model, test_d, toward_p) && !check_stick_intersect(model, (test_d + use_ball_p) / 2.0f, toward_p) && !check_stick_intersect(model, test_d, (test_d + use_ball_p) / 2.0f);
		bool not_near = (ball_surface_dist(model, (test_d + use_ball_p) / 2.0f) < 100000000000000.0f) && (ball_surface_dist(model, (test_d + toward_p) / 2.0f) < 100000000000000.0f);
		if (!(dist && insect && not_near)){
			temp.erase(temp.begin() + i);
			i -= 1;
		}
	}

	if (temp.size() > 0){
		choose_split = rand() % temp.size();
		cout << "choose_split : " << choose_split << endl;

		//cout << "o : " << temp.at(choose_split).origin[0] << " " << temp.at(choose_split).origin[1] << " " << temp.at(choose_split).origin[2] << endl;
		//cout << "t1 : " << temp.at(choose_split).travel_1[0] << " " << temp.at(choose_split).travel_1[1] << " " << temp.at(choose_split).travel_1[2] << endl;
		//cout << "t2 : " << temp.at(choose_split).travel_2[0] << " " << temp.at(choose_split).travel_2[1] << " " << temp.at(choose_split).travel_2[2] << endl;

		zomeconn new_ball;
		zomeconn new_stick_f, new_stick_t;

		new_ball.position = test_connect.at(COLOR_WHITE).at(s_index).position + t.dir->at(temp.at(choose_split).travel_1[0]) * t.face_length(temp.at(choose_split).travel_1[0], temp.at(choose_split).travel_1[1]);
		new_ball.color = COLOR_WHITE;

		new_stick_f.position = test_connect.at(COLOR_WHITE).at(s_index).position + t.dir->at(temp.at(choose_split).travel_1[0]) * t.face_length(temp.at(choose_split).travel_1[0], temp.at(choose_split).travel_1[1]) / 2;
		new_stick_f.color = t.face_color(temp.at(choose_split).travel_1[0]);
		new_stick_f.size = temp.at(choose_split).travel_1[1];
		new_stick_f.fromface = temp.at(choose_split).travel_1[0];
		new_stick_f.towardface = temp.at(choose_split).travel_1[2];

		//cout << "f:" << endl;
		//cout << new_stick_f.color << endl;
		//cout << new_stick_f.size << endl;
		//cout << new_stick_f.fromface << endl;
		//cout << new_stick_f.towardface << endl;

		new_stick_t.position = test_connect.at(COLOR_WHITE).at(toward_ball_index).position + t.dir->at(temp.at(choose_split).travel_2[0]) * t.face_length(temp.at(choose_split).travel_2[0], temp.at(choose_split).travel_2[1]) / 2;
		new_stick_t.color = t.face_color(temp.at(choose_split).travel_2[0]);
		new_stick_t.size = temp.at(choose_split).travel_2[1];
		new_stick_t.fromface = temp.at(choose_split).travel_2[0];
		new_stick_t.towardface = temp.at(choose_split).travel_2[2];

		//cout << "t:" << endl;
		//cout << new_stick_t.color << endl;
		//cout << new_stick_t.size << endl;
		//cout << new_stick_t.fromface << endl;
		//cout << new_stick_t.towardface << endl;

		//cout << toward_ball_index << endl;
		//cout << new_stick_t.fromface << endl;
		//cout << new_stick_t.towardface << endl;

		test_connect.at(COLOR_WHITE).at(s_index).connect_stick[new_stick_f.fromface] = vec2(new_stick_f.color, test_connect.at(new_stick_f.color).size());
		new_ball.connect_stick[new_stick_f.towardface] = vec2(new_stick_f.color, test_connect.at(new_stick_f.color).size());

		new_stick_f.fromindex = vec2(COLOR_WHITE, s_index);
		new_stick_f.towardindex = vec2(COLOR_WHITE, test_connect.at(COLOR_WHITE).size());
		test_connect.at(new_stick_f.color).push_back(new_stick_f);

		test_connect.at(COLOR_WHITE).at(toward_ball_index).connect_stick[new_stick_t.fromface] = vec2(new_stick_t.color, test_connect.at(new_stick_t.color).size());
		new_ball.connect_stick[new_stick_t.towardface] = vec2(new_stick_t.color, test_connect.at(new_stick_t.color).size());

		new_stick_t.fromindex = vec2(COLOR_WHITE, toward_ball_index);
		new_stick_t.towardindex = vec2(COLOR_WHITE, test_connect.at(COLOR_WHITE).size());
		test_connect.at(new_stick_t.color).push_back(new_stick_t);

		test_connect.at(COLOR_WHITE).push_back(new_ball);
	}
}

void fake_case(int index)
{
	zomedir t;

	vec2 test = test_connect.at(COLOR_WHITE).at(index).connect_stick[5];
		
	int from_face = test_connect.at(test[0]).at(test[1]).fromface;
	int from_size = test_connect.at(test[0]).at(test[1]).size;
	
	zomeconn new_ball;
	zomeconn new_stick_f, new_stick_t;
	
	vec3 use_ball_p = test_connect.at(COLOR_WHITE).at(index).position;

	vec3 use_stick_p = test_connect.at(test[0]).at(test[1]).position;
	vec3 judge = use_stick_p + 0.5f * t.dir->at(from_face) * t.color_length(test[0], from_size) - use_ball_p;

	int toward_ball_index = test_connect.at(test[0]).at(test[1]).towardindex[1];

	if (from_face != 5)
		from_face = 5;
	
	if (toward_ball_index == 0){
		toward_ball_index = test_connect.at(test[0]).at(test[1]).fromindex[1];
	}

	vector<zomerecord> temp;
	for (unsigned int i = 0; i < merge_table.table.at(from_face).size(); i += 1){
		for (unsigned int j = 0; j < merge_table.table.at(from_face).at(i).size(); j += 1){
			if (merge_table.table.at(from_face).at(i).at(j).origin[1] == from_size){
				temp.push_back(merge_table.table.at(from_face).at(i).at(j));
			}
		}
	}

	new_ball.position = test_connect.at(COLOR_WHITE).at(toward_ball_index).position + t.dir->at(temp.at(2).travel_1[0]) * t.face_length(temp.at(2).travel_1[0], temp.at(2).travel_1[1]);
	new_ball.color = COLOR_WHITE;

	new_stick_f.position = test_connect.at(COLOR_WHITE).at(toward_ball_index).position + t.dir->at(temp.at(2).travel_1[0]) * t.face_length(temp.at(2).travel_1[0], temp.at(2).travel_1[1]) / 2;
	new_stick_f.color = t.face_color(temp.at(2).travel_1[0]);
	new_stick_f.size = temp.at(2).travel_1[1];
	new_stick_f.fromface = temp.at(2).travel_1[0];
	new_stick_f.towardface = temp.at(2).travel_1[2];

	new_stick_t.position = test_connect.at(COLOR_WHITE).at(index).position + t.dir->at(temp.at(2).travel_2[0]) * t.face_length(temp.at(2).travel_2[0], temp.at(2).travel_2[1]) / 2;
	new_stick_t.color = t.face_color(temp.at(2).travel_2[0]);
	new_stick_t.size = temp.at(2).travel_2[1];
	new_stick_t.fromface = temp.at(2).travel_2[0];
	new_stick_t.towardface = temp.at(2).travel_2[2];

	test_connect.at(COLOR_WHITE).at(toward_ball_index).connect_stick[new_stick_f.fromface] = vec2(new_stick_f.color, test_connect.at(new_stick_f.color).size());
	new_ball.connect_stick[new_stick_f.towardface] = vec2(new_stick_f.color, test_connect.at(new_stick_f.color).size());

	new_stick_f.fromindex = vec2(COLOR_WHITE, toward_ball_index);
	new_stick_f.towardindex = vec2(COLOR_WHITE, test_connect.at(COLOR_WHITE).size());
	test_connect.at(new_stick_f.color).push_back(new_stick_f);

	test_connect.at(COLOR_WHITE).at(index).connect_stick[new_stick_t.fromface] = vec2(new_stick_t.color, test_connect.at(new_stick_t.color).size());
	new_ball.connect_stick[new_stick_t.towardface] = vec2(new_stick_t.color, test_connect.at(new_stick_t.color).size());

	new_stick_t.fromindex = vec2(COLOR_WHITE, index);
	new_stick_t.towardindex = vec2(COLOR_WHITE, test_connect.at(COLOR_WHITE).size());
	test_connect.at(new_stick_t.color).push_back(new_stick_t);

	test_connect.at(COLOR_WHITE).push_back(new_ball);
}

void check_merge(vector<vec4> &can_merge, GLMmodel *model)
{
	zomedir t;

	for (unsigned int i = 0; i < test_connect.at(COLOR_WHITE).size(); i += 1){
		vector<int> use_slot;
		for (int j = 0; j < 62; j += 1){
			if (test_connect.at(COLOR_WHITE).at(i).connect_stick[j] != vec2(-1.0f, -1.0f)){
				use_slot.push_back(j);
			}
		}

		for (unsigned int j = 0; j < use_slot.size() - 1; j += 1){
			
			int use_index = test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][1]).towardindex[1];
			if (use_index == i)
				use_index = test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][1]).fromindex[1];
			vec3 use_p = test_connect.at(COLOR_WHITE).at(use_index).position;
			
			for (unsigned int k = j + 1; k < use_slot.size(); k += 1){
				int opp_face = t.opposite_face(use_slot.at(j));
		
				for (unsigned int a = 0; a < merge_table.table.at(opp_face).at(use_slot.at(k)).size(); a += 1){
					bool judge1 = (merge_table.table.at(opp_face).at(use_slot.at(k)).at(a).origin[1] == test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(j)][1]).size);
					bool judge2 = (merge_table.table.at(opp_face).at(use_slot.at(k)).at(a).travel_1[1] == test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][1]).size);
					if (judge1 && judge2){
						
						int toward_index = test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][1]).towardindex[1];
						if (toward_index == i)
							toward_index = test_connect.at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][0]).at(test_connect.at(COLOR_WHITE).at(i).connect_stick[use_slot.at(k)][1]).fromindex[1];
						vec3 toward_p = test_connect.at(COLOR_WHITE).at(toward_index).position;
												
						vec3 test_p = (test_connect.at(COLOR_WHITE).at(use_index).position + test_connect.at(COLOR_WHITE).at(toward_index).position) / 2.0f;
						
						bool judge = !check_stick_intersect(model, use_p, toward_p) && !check_stick_intersect(model, test_p, use_p) && !check_stick_intersect(model, test_p, toward_p);
						if (judge){
							vec4 connect(use_index, toward_index, t.face_color(merge_table.table.at(opp_face).at(use_slot.at(k)).at(a).travel_2[0]), merge_table.table.at(opp_face).at(use_slot.at(k)).at(a).travel_2[1]);
							can_merge.push_back(connect);
						}
					}
				}
			}
		}
	}
}

void merge(vector<vector<zomeconn>> &test_connect, vec4 &merge_vector)
{
	zomedir t;

	vec3 p1 = test_connect.at(COLOR_WHITE).at(merge_vector[0]).position;
	vec3 p2 = test_connect.at(COLOR_WHITE).at(merge_vector[1]).position;
	vec3 v = (p2 - p1).normalize();

	vec3 stick_p = (p1 + p2) / 2.0f;
	int from_face = t.dir_face(v);
	int toward_face = t.opposite_face(from_face);

	zomeconn new_stick;
	new_stick.position = stick_p;
	new_stick.color = merge_vector[2];
	new_stick.size = merge_vector[3];
	new_stick.fromface = from_face;
	new_stick.fromindex = vec2(COLOR_WHITE, merge_vector[0]);
	new_stick.towardface = toward_face;
	new_stick.towardindex = vec2(COLOR_WHITE, merge_vector[1]);

	test_connect.at(COLOR_WHITE).at(merge_vector[0]).connect_stick[from_face] = vec2(new_stick.color, test_connect.at(new_stick.color).size());
	test_connect.at(COLOR_WHITE).at(merge_vector[1]).connect_stick[toward_face] = vec2(new_stick.color, test_connect.at(new_stick.color).size());

	test_connect.at(new_stick.color).push_back(new_stick);
}

void check_bridge(vector<vec4> &can_bridge, GLMmodel *model)
{
	zomedir t;

	for (unsigned int i = 0; i < test_connect.at(COLOR_WHITE).size(); i += 1){
		for (unsigned int j = 0; j < test_connect.at(COLOR_WHITE).size(); j += 1){
			if (i != j){
				vec3 test_l = test_connect.at(COLOR_WHITE).at(j).position - test_connect.at(COLOR_WHITE).at(i).position;
				vec3 test_n = (test_connect.at(COLOR_WHITE).at(j).position - test_connect.at(COLOR_WHITE).at(i).position).normalize();
				int test_index = t.dir_face(test_n);
				if (test_index != -1){
					if (test_connect.at(COLOR_WHITE).at(i).connect_stick[test_index] == vec2(-1.0f, -1.0f)){
						float l = test_l.length();
						for (int size = 0; size < 3; size += 1){
							if (fabs(t.face_length(test_index, size) - l) < 0.001f){
								bool add = true;
								for (unsigned int k = 0; k < can_bridge.size(); k += 1){
									if ((can_bridge.at(k)[0] == j) && (can_bridge.at(k)[1] == i)){
										add = false;
									}
								}

								if (add){
									vec3 test_p = (test_connect.at(COLOR_WHITE).at(i).position + test_connect.at(COLOR_WHITE).at(j).position) / 2.0f;
									vec3 from_p = test_connect.at(COLOR_WHITE).at(i).position;
									vec3 toward_p = test_connect.at(COLOR_WHITE).at(j).position;
									bool judge = !check_stick_intersect(model, from_p, toward_p) && !check_stick_intersect(model, test_p, from_p) && !check_stick_intersect(model, test_p, toward_p);
									if (judge){
										vec4 connect(i, j, t.face_color(test_index), size);
										can_bridge.push_back(connect);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void bridge(vector<vector<zomeconn>> &test_connect, vec4 &bridge_vector)
{
	zomedir t;

	vec3 p1 = test_connect.at(COLOR_WHITE).at(bridge_vector[0]).position;
	vec3 p2 = test_connect.at(COLOR_WHITE).at(bridge_vector[1]).position;
	vec3 v = (p2 - p1).normalize();

	vec3 stick_p = (p1 + p2) / 2.0f;
	int from_face = t.dir_face(v);
	int toward_face = t.opposite_face(from_face);

	zomeconn new_stick;
	new_stick.position = stick_p;
	new_stick.color = bridge_vector[2];
	new_stick.size = bridge_vector[3];
	new_stick.fromface = from_face;
	new_stick.fromindex = vec2(COLOR_WHITE, bridge_vector[0]);
	new_stick.towardface = toward_face;
	new_stick.towardindex = vec2(COLOR_WHITE, bridge_vector[1]);

	test_connect.at(COLOR_WHITE).at(bridge_vector[0]).connect_stick[from_face] = vec2(new_stick.color, test_connect.at(new_stick.color).size());
	test_connect.at(COLOR_WHITE).at(bridge_vector[1]).connect_stick[toward_face] = vec2(new_stick.color, test_connect.at(new_stick.color).size());

	test_connect.at(new_stick.color).push_back(new_stick);
}

bool collision_test(vector<vector<zomeconn>> &test_connect, vec3 & give_up)
{
	zomedir t;
	int ball_ball_count = 0;
	for (unsigned int i = 0; i < test_connect.at(COLOR_WHITE).size(); i += 1){
		for (unsigned int j = 0; j < test_connect.at(COLOR_WHITE).size(); j += 1){
			if (i != j){
				float judge = (test_connect.at(COLOR_WHITE).at(i).position - test_connect.at(COLOR_WHITE).at(j).position).length();
				if (fabs(judge - NODE_DIAMETER) < 0.5f){
					//cout << fabs(judge - NODE_DIAMETER) << endl;
					ball_ball_count += 1;
				}
			}
		}
	}

	ball_ball_count /= 2.0f;
	
	int stick_ball_count = 0;
	for (int i = 0; i < 3; i += 1){
		for (unsigned int j = 0; j < test_connect.at(i).size(); j += 1){
			vec3 p1 = test_connect.at(COLOR_WHITE).at(test_connect.at(i).at(j).fromindex[1]).position;
			vec3 p2 = test_connect.at(COLOR_WHITE).at(test_connect.at(i).at(j).towardindex[1]).position;
			float times = t.color_length(test_connect.at(i).at(j).color, test_connect.at(i).at(j).size);
			for (unsigned int k = 0; k < test_connect.at(COLOR_WHITE).size(); k += 1){
				if (k != test_connect.at(i).at(j).fromindex[1] && k != test_connect.at(i).at(j).towardindex[1]){
					vec3 o = test_connect.at(COLOR_WHITE).at(k).position;
					vec3 v1 = p2 - p1;
					vec3 v2 = o - p1;
					float o_times = (v2 * v1) / v1.length();
					if ((o_times < times) && (o_times > 0.0f)){
						vec3 check_p = p1 + t.dir->at(test_connect.at(i).at(j).fromface) * o_times;
						float judge = (o - check_p).length() - NODE_DIAMETER / 2.0f;
						if (judge < ERROR_THICKNESS){
							stick_ball_count += 1;
						}
					}
				}
			}
		}
	}

	int stick_stick_count = 0;
	for (int i = 0; i < 3; i += 1){
		for (unsigned int j = 0; j < test_connect.at(i).size(); j += 1){
			vec3 p1 = test_connect.at(COLOR_WHITE).at(test_connect.at(i).at(j).fromindex[1]).position;
			vec3 p2 = test_connect.at(COLOR_WHITE).at(test_connect.at(i).at(j).towardindex[1]).position;
			int fromindex1 = test_connect.at(i).at(j).fromindex[1];
			int towardindex1 = test_connect.at(i).at(j).towardindex[1];
			
			
			for (int a = 0; a < 3; a += 1){
				for (unsigned int b = 0; b < test_connect.at(a).size(); b += 1){
					if (!((a == i) && (b == j))){
						int fromindex2 = test_connect.at(a).at(b).fromindex[1];
						int towardindex2 = test_connect.at(a).at(b).towardindex[1];
						bool judge1 = (fromindex1 == fromindex2) || (fromindex1 == towardindex2);
						bool judge2 = (towardindex1 == fromindex2) || (towardindex1 == towardindex2);

						if (!(judge1 || judge2)){
							vec3 p3 = test_connect.at(COLOR_WHITE).at(test_connect.at(a).at(b).fromindex[1]).position;
							vec3 p4 = test_connect.at(COLOR_WHITE).at(test_connect.at(a).at(b).towardindex[1]).position;

							vec3 v1 = p2 - p1;
							vec3 n1 = (p2 - p1).normalize();
							vec3 v2 = p4 - p3;
							vec3 n2 = (p4 - p3).normalize();
							
							float ap = v1 * v1; //u*u
							float bp = v1 * v2; //u*v
							float cp = v2 * v2; //v*v

							if (!((n1 ^ n2).length() < 0.001f) && !(fabs(n1 * n2) < 0.001f)){
								vec3 v3 = p1 - p3;
							
								float dp = v1 * v3; //u*w 
								float ep = v2 * v3; //v*w

								float dt = ap * cp - bp * bp;
								float sd = dt;
								float td = dt;

								float sn = bp * ep - cp * dp;
								float tn = ap * ep - bp * dp;

								if (sn < 0.0001f){
									sn = 0.0f;
									tn = ep;
									td = cp;
								}
								else if (sn > sd){
									sn = sd;
									tn = ep + bp;
									td = cp;
								}

								if (tn < 0.0001f){
									tn = 0.0f;
									if (-dp < 0.0001f){
										sn = 0.0f;
									}
									else if (-dp > ap){
										sn = sd;
									}
									else{
										sn = -dp;
										sd = -ap;
									}
								}
								else if (tn > td){
									tn = td;
									if ((-dp + bp) < 0.0001f){
										sn = 0.0f;
									}
									else if ((-dp + bp) > ap){
										sn = sd;
									}
									else{
										sn = (-dp + bp);
										sd = ap;
									}
								}

								float sc = 0.0f;
								float tc = 0.0f;

								if (fabs(sn) < 0.0001f){
									sc = 0.0f;
								}
								else{
									sc = sn / sd;
								}

								if (fabs(tn) < 0.0001f){
									tc = 0.0f;
								}
								else{
									tc = tn / td;
								}

								vec3 dist = v3 + (sc * v1) - (tc * v2);

								if (dist.length() < ERROR_THICKNESS){
									stick_stick_count += 1;
								}
							}
						}

					}
				}
			}
		}
	}
	stick_stick_count /= 2.0f;

	cout << "error => ball-to-ball : " << ball_ball_count << " ball-to-rod :  " << stick_ball_count << " rod-to-rod :  " << stick_stick_count << endl;

	if (ball_ball_count != 0 || stick_ball_count != 0 || stick_stick_count != 0){
		if (ball_ball_count != 0)
			give_up[0] += 1.0f;
		if (stick_ball_count != 0)
			give_up[1] += 1.0f;
		if (stick_stick_count != 0)
			give_up[2] += 1.0f;
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	//    findzoom();

	myObj = glmReadOBJ(model_source);
	//myObj_inner = glmCopy(myObj);

	//cout << "size : " << myObj->numtriangles << endl;
	init();

	/////*vec3 p1(174.059, 312.327, 107.917);
	////vec3 p2(112.142, 288.677, 69.6504);*/
	/////*vec3 p1(-77.0575, 430.577, 69.6504);
	////vec3 p2(-177.241, 392.31, 7.7339);*/
	/////*vec3 p1(-138.974, 321.361, -57.633);
	////vec3 p2(-138.974, 359.627, -33.983);*/
	///*vec3 p1(17.5425, 241.377, -24.9497);
	//vec3 p2(-44.374, 141.194, 13.3168);*/
	//vec3 p1(-138.974, 359.627, 13.317);
	//vec3 p2(-100.7075, 359.627, 75.233);
	//bool test = check_stick_intersect(myObj, p1, p2);
	//cout << "test : " << test << endl;

	//test();

	//fake();
	//	
	//for (int j = 0; j < 4; j += 1){
	//	for (int k = 0; k < zome_queue.at(2).at(j).size(); k += 1){
	//		zome_queue.at(2).at(j).at(k).position += delta;
	//	}
	//}

	//combine_zome_ztruc(zome_queue.at(1), zome_queue.at(2));
	//combine_zome_ztruc(zome_queue.at(1), test_connect);

	////for (int j = 0; j < 4; j += 1){
	////	cout << j << " : " << endl;
	////	if (j != 3){
	////		/*for (int k = 0; k < zome_queue.at(i).at(j).size(); k += 1){
	////		cout << "\t " << k << " : (" << zome_queue.at(i).at(j).at(k).fromindex[0] << " , " << zome_queue.at(i).at(j).at(k).fromindex[1]
	////		<< ") (" << zome_queue.at(i).at(j).at(k).towardindex[0] << " , " << zome_queue.at(i).at(j).at(k).towardindex[1] << ")" << endl;
	////		}*/
	////	}
	////	else{
	////		for (int k = 0; k < zome_queue.at(1).at(3).size(); k += 1){
	////			cout << "\t " << k << " : ";
	////			for (int a = 0; a < 62; a += 1){
	////				if (zome_queue.at(1).at(3).at(k).connect_stick[a] != vec2(-1.0f, -1.0f))
	////					cout << " (" << zome_queue.at(1).at(3).at(k).connect_stick[a][0] << " , " << zome_queue.at(1).at(3).at(k).connect_stick[a][1] << ")";
	////			}
	////			cout << endl;
	////			cout << "\t " << zome_queue.at(1).at(3).at(k).position[0] << " " << zome_queue.at(1).at(3).at(k).position[1] << " " << zome_queue.at(1).at(3).at(k).position[2] << endl;
	////		}
	////	}
	////}

	//output_zometool(zome_queue.at(1), string("test.obj"));
	//output_struc(zome_queue.at(1), string("fake.txt"));
	
	srand((unsigned)time(NULL));
	struc_parser(test_connect, string("fake.txt"));
	//struc_parser(test_connect, string("fake123.txt"));
	
	float origin_e = compute_energy(test_connect, myObj);

	///*for (int j = 0; j < 4; j += 1){
	//	cout << j << " : " << endl;
	//	if (j != 3){
	//		for (int k = 0; k < test_connect.at(j).size(); k += 1){
	//			cout << "\t " << k << " : (" << test_connect.at(j).at(k).fromindex[0] << " , " << test_connect.at(j).at(k).fromindex[1]
	//				<< ") (" << test_connect.at(j).at(k).towardindex[0] << " , " << test_connect.at(j).at(k).towardindex[1] << ")" << endl;
	//		}
	//	}
	//	else{
	//		for (int k = 0; k < test_connect.at(3).size(); k += 1){
	//			cout << "\t " << k << " : ";
	//			for (int a = 0; a < 62; a += 1){
	//				if (test_connect.at(3).at(k).connect_stick[a] != vec2(-1.0f, -1.0f))
	//					cout << a << ": (" << test_connect.at(3).at(k).connect_stick[a][0] << " , " << test_connect.at(3).at(k).connect_stick[a][1] << ") ";
	//			}
	//			cout << endl;
	//			cout << "\t " << test_connect.at(3).at(k).position[0] << " " << test_connect.at(3).at(k).position[1] << " " << test_connect.at(3).at(k).position[2] << endl;
	//		}
	//	}
	//}*/

	cout << "start" << endl;
	int collision = 0;

	vec3 give_up;
	int num_split = 0, num_merge = 0, num_bridge = 0;

	for (int i = 0; i < 1000; i += 1){
		cout << i << " :" << endl;
		int choose_op = rand() % 3;
				
		vector<vector<zomeconn>> temp_connect(4);
		temp_connect = test_connect;

		if (choose_op == 0){
			cout << "split" << endl;
			int result = rand() % test_connect.at(COLOR_WHITE).size();
			cout << result << endl;
			split(temp_connect, result, myObj);
			num_split += 1;
		}
		else if(choose_op == 1){
			cout << "merge" << endl;
			vector<vec4> can_merge;
			check_merge(can_merge, myObj);
			if (can_merge.size() > 0){
				int merge_index = rand() % can_merge.size();
				merge(temp_connect, can_merge.at(merge_index));
			}
			num_merge += 1;
		}
		else{
			cout << "bridge" << endl;
			vector<vec4> can_bridge;
			check_bridge(can_bridge, myObj);
			if (can_bridge.size() > 0){
				int bridge_index = rand() % can_bridge.size();
				bridge(temp_connect, can_bridge.at(bridge_index));
			}
			num_bridge += 1;
		}

		float temp_e = compute_energy(temp_connect, myObj);

		if (temp_e < origin_e){
			if (collision_test(temp_connect, give_up)){
				test_connect = temp_connect;
				origin_e = temp_e;
				cout << "commit" << endl;
			}
			else{
				collision += 1;
			}
		}
		cout << endl;
	}

	cout << "collision : " << collision << " " << 1000 << endl;
	cout << "split : " << num_split << " merge : " << num_merge << " bridge : " << num_bridge << endl;
	cout << "ball-to-ball : " << give_up[0] << " ball-to-rod :  " << give_up[1] << " rod-to-rod :  " << give_up[2] << endl;

	/*vector<vec4> can_bridge;
	check_bridge(can_bridge);
	bridge(test_connect, can_bridge.at(can_bridge.size() - 1));*/

	/*cout << can_bridge.size() << endl;

	for (unsigned int i = 0; i < can_bridge.size(); i += 1){
		cout << can_bridge.at(i)[0] << " " << can_bridge.at(i)[1] << " " << can_bridge.at(i)[2] << " " << can_bridge.at(i)[3] << endl;
	}*/
	
	
	output_zometool(test_connect, string("fake123.obj"));
	///*output_struc(test_connect, string("fake123.txt"));*/
		
	//glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	//glutInitWindowSize(1000,1000);

	//   glutCreateWindow("Zometool");
	//glutDisplayFunc(display);
	//glutReshapeFunc(myReshape);
	//glutMouseFunc(mouse);
	//   glutMotionFunc(mouseMotion);
	//   glutKeyboardFunc(keyboard);
	//   glutSpecialFunc(special);
	//glEnable(GL_DEPTH_TEST); /* Enable hidden--surface--removal */

	//glewInit();

	//setShaders();

	//glutMainLoop();
	//
	system("pause");
	return 0;
}
