#ifndef __GLM_H__
#define __GLM_H__ 1

/*
      glm.h
      Nate Robins, 1997, 2000
      nate@pobox.com, http://www.pobox.com/~nate

      Wavefront OBJ model file format reader/writer/manipulator.

      Includes routines for generating smooth normals with
      preservation of edges, welding redundant vertices & texture
      coordinate generation (spheremap and planar projections) + more.

 */


#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <vector>
#include "algebra3.h"

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#define GLM_NONE     (0)            /* render with only vertices */
#define GLM_FLAT     (1 << 0)       /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)       /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)       /* render with texture coords */
#define GLM_COLOR    (1 << 3)       /* render with colors */
#define GLM_MATERIAL (1 << 4)       /* render with materials */


/* GLMmaterial: Structure that defines a material in a model.
 */
typedef struct _GLMmaterial
{
  char*   name;                 /* name of material */
  GLfloat diffuse[4];           /* diffuse component */
  GLfloat ambient[4];           /* ambient component */
  GLfloat specular[4];          /* specular component */
  GLfloat emmissive[4];         /* emmissive component */
  GLfloat shininess;            /* specular exponent */
} GLMmaterial;

class simple_material{
public:
	std::string name;
	float diffuse[3];
};

void output_material(std::vector<simple_material> materials, std::string &filename);

/* GLMtriangle: Structure that defines a triangle in a model.
 */
class GLMtriangle {
public:
  GLuint vindices[3];           /* array of triangle vertex indices */
  GLuint nindices[3];           /* array of triangle normal indices */
  GLuint tindices[3];           /* array of triangle texcoord indices*/
  GLuint findex;                /* index of triangle facet normal */

  int edge_index[3];
  std::vector<unsigned int> split_plane_id;
  bool split_by_process;
  int material_id;
  int material_id_energy;
  int material_id_graph_cut;
  int material_id_saliency;
  int near_tri[3];

  int near_node;
  float near_dist;
  float energy_d;
  
  float saliency = 0.0f;

  int tag;
};

/* GLMgroup: Structure that defines a group in a model.
 */
typedef struct _GLMgroup {
  char*             name;           /* name of this group */
  GLuint            numtriangles;   /* number of triangles in this group */
  GLuint*           triangles;      /* array of triangle indices */
  GLuint            material;       /* index to material for group */
  struct _GLMgroup* next;           /* pointer to next group in model */
} GLMgroup;

class vertex {
public:
    std::vector<unsigned int> connect_edge;
    std::vector<unsigned int> align_plane;
	std::vector<unsigned int> tri_use;
	std::vector<unsigned int> material;
	int tag;
	int material_id_saliency;
};

class Loop{
public:
    vec3 plane_normal;
    std::vector<int> *loop_line;
	std::vector<vec3> *three_d_point;
    std::vector<vec2> *two_d_point;
    bool travel_reverse;
    std::vector<int> *sign;
    std::vector<vec3> *tri;
    float sign_flip;
    int num_concave;
};

/* GLMmodel: Structure that defines a model.
 */
class GLMmodel {
public:
  char*    pathname;            /* path to this model */
  char*    mtllibname;          /* name of the material library */

  GLuint   numvertices;         /* number of vertices in model */
  std::vector<GLfloat> *vertices;            /* array of vertices  */
  std::vector<vertex> *cut_loop;
  std::vector<unsigned int> *multi_vertex;

  GLuint   numnormals;          /* number of normals in model */
  GLfloat* normals;             /* array of normals */

  GLuint   numtexcoords;        /* number of texcoords in model */
  GLfloat* texcoords;           /* array of texture coordinates */

  GLuint   numfacetnorms;       /* number of facetnorms in model */
  std::vector<GLfloat> *facetnorms;          /* array of facetnorms */

  GLuint       numtriangles;    /* number of triangles in model */
  std::vector<GLMtriangle> *triangles;       /* vector of triangles */

  GLuint       nummaterials;    /* number of materials in model */
  GLMmaterial* materials;       /* array of materials */

  GLuint       numgroups;       /* number of groups in model */
  GLMgroup*    groups;          /* linked list of groups */

  GLfloat position[3];          /* position of the model */

  std::vector<Loop> *loop = NULL;

  vec3 obb_max;
  vec3 obb_min;
  vec3 obb_size;
  vec3 obb_center;
};

void empty_model(GLMmodel *model);

void output_nearest_point(GLMmodel *model, std::string &filename);

void nearest_point_parser(GLMmodel *model, std::string &filename);

vec2
tri_find_neigbor(GLMtriangle t1, GLMtriangle t2);

vec3
glmCentroid(GLMmodel *model);

GLMmodel* glmCopy(GLMmodel* m1);

/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure
 */
GLfloat
glmUnitize(GLMmodel* model);

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions);

/* glmScale: Scales a model by a given amount.
 *
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale);

GLvoid
glmScale_x(GLMmodel* model, GLfloat scale_x);

GLvoid
glmScale_y(GLMmodel* model, GLfloat scale_y);

GLvoid
glmScale_z(GLMmodel* model, GLfloat scale_z);

GLvoid
glmRT(GLMmodel* model, vec3 &rotate, vec3 &translate);

GLvoid
glmR(GLMmodel* model, vec3 &axis, GLfloat angle);

GLvoid
glmR(GLMmodel* model, vec3 &rotate);

GLvoid
glmT(GLMmodel* model, vec3 &translate);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 *
 * model - properly initialized GLMmodel structure
 */
GLvoid
glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */

 /*one face normals*/
GLvoid
glmOneFacetNormals(GLMmodel* model, int tri_index);

GLvoid
glmVertexNormals(GLMmodel* model, GLfloat angle);

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid
glmSpheremapTexture(GLMmodel* model);

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid
glmDelete(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
GLMmodel*
glmReadOBJ(char* filename);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE    -  write only vertices
 *            GLM_FLAT    -  write facet normals
 *            GLM_SMOOTH  -  write vertex normals
 *            GLM_TEXTURE -  write texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode);

GLvoid
glmWriteOBJ_EXP(GLMmodel* model, char* filename, std::vector<simple_material> materials, std::string &materials_filename, int mode);

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid
glmDraw(GLMmodel* model, GLuint mode);

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLuint
glmList(GLMmodel* model, GLuint mode);

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon);

/* glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
 * that should look something like:
 *
 *    P6
 *    # comment
 *    width height max_value
 *    rgbrgbrgb...
 *
 * where "P6" is the magic cookie which identifies the file type and
 * should be the only characters on the first line followed by a
 * carriage return.  Any line starting with a # mark will be treated
 * as a comment and discarded.   After the magic cookie, three integer
 * values are expected: width, height of the image and the maximum
 * value for a pixel (max_value must be < 256 for PPM raw files).  The
 * data section consists of width*height rgb triplets (one byte each)
 * in binary format (i.e., such as that written with fwrite() or
 * equivalent).
 *
 * The rgb data is returned as an array of unsigned chars (packed
 * rgb).  The malloc()'d memory should be free()'d by the caller.  If
 * an error occurs, an error message is sent to stderr and NULL is
 * returned.
 *
 * filename   - name of the .ppm file.
 * width      - will contain the width of the image on return.
 * height     - will contain the height of the image on return.
 *
 */
GLubyte*
glmReadPPM(char* filename, int* width, int* height);

GLvoid
glmCombine(GLMmodel *model, GLMmodel *source);

char *my_strdup(const char *str);

#endif
