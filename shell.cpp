#include <algorithm>
#include <vector>
#include "glm.h"
#include <cmath>
#include <vector>

void recount_normal(GLMmodel *myObj,std::vector<int> *point_tri)
{
    glmFacetNormals(myObj);

    point_tri = new std::vector<int>[myObj->numvertices + 1];

    for(int i = 0 ; i < myObj->numtriangles ; i += 1)
    {
        for(int j = 0 ; j < 3 ; j += 1)
        {
            bool add = true;
            for(int k = 0 ; k < point_tri[myObj->triangles[i].vindices[j]].size() ; k += 1)
            {
                GLfloat *temp = &myObj->facetnorms[3 * point_tri[myObj->triangles[i].vindices[j]][k]];
                if(*temp == myObj->facetnorms[3 * (i + 1) + 0] && *(temp + 1) == myObj->facetnorms[3 * (i + 1) + 1] && *(temp + 2) == myObj->facetnorms[3 * (i + 1) + 2])
                {
                    add = false;
                    break;
                }
            }
            if(add)
                point_tri[myObj->triangles[i].vindices[j]].push_back(i + 1);
        }
    }

    for(int i = 1 ; i <= myObj->numvertices ; i += 1)
    {
        sort(point_tri[i].begin(),point_tri[i].begin() + point_tri[i].size());
    }

    myObj->numnormals = myObj->numvertices;
    myObj->normals = new GLfloat[3 * (myObj->numnormals + 1)];
    for(int i = 1 ; i <= myObj->numnormals ; i += 1)
    {
        GLfloat temp[3] = {0.0 , 0.0 , 0.0};
        for(int j = 0 ; j < point_tri[i].size() ; j += 1)
        {
            temp[0] += myObj->facetnorms[3 * point_tri[i][j] + 0];
            temp[1] += myObj->facetnorms[3 * point_tri[i][j] + 1];
            temp[2] += myObj->facetnorms[3 * point_tri[i][j] + 2];
        }
        GLfloat normal_length = sqrt(pow(temp[0],2) + pow(temp[1],2) + pow(temp[2],2));
        temp[0] /= normal_length;
        temp[1] /= normal_length;
        temp[2] /= normal_length;

        myObj->normals[3 * i + 0] = temp[0];
        myObj->normals[3 * i + 1] = temp[1];
        myObj->normals[3 * i + 2] = temp[2];
    }
}

void process_inner(GLMmodel *myObj,GLMmodel *myObj_inner)
{
    myObj_inner->numnormals = myObj_inner->numvertices;
    myObj_inner->normals = new GLfloat[3 * (myObj_inner->numnormals + 1)];
    for(int i = 1 ; i <= myObj_inner->numvertices ; i += 1)
    {
        myObj_inner->vertices[3 * i + 0] = myObj->vertices[3 * i + 0] - 0.05 * myObj->normals[3 * i + 0];
        myObj_inner->normals[3 * i + 0] =  -1 * myObj->normals[3 * i + 0];

        myObj_inner->vertices[3 * i + 1] = myObj->vertices[3 * i + 1] - 0.05 * myObj->normals[3 * i + 1];
        myObj_inner->normals[3 * i + 1] =  -1 * myObj->normals[3 * i + 1];

        myObj_inner->vertices[3 * i + 2] = myObj->vertices[3 * i + 2] - 0.05 * myObj->normals[3 * i + 2];
        myObj_inner->normals[3 * i + 2] =  -1 * myObj->normals[3 * i + 2];
    }

    myObj_inner->numfacetnorms = myObj->numfacetnorms;
//    myObj_inner->facetnorms = new GLfloat[3 * (myObj_inner->numfacetnorms + 1)];
    std::vector<GLfloat> temp_facenormal(3 * (myObj_inner->numfacetnorms + 1));
    myObj_inner->facetnorms = temp_facenormal;

    for(int i = 1 ; i <= myObj->numfacetnorms ; i += 1)
    {
        myObj_inner->facetnorms[3 * i + 0] =  -1 * myObj->facetnorms[3 * i + 0];

        myObj_inner->facetnorms[3 * i + 1] =  -1 * myObj->facetnorms[3 * i + 1];

        myObj_inner->facetnorms[3 * i + 2] =  -1 * myObj->facetnorms[3 * i + 2];
    }
}
