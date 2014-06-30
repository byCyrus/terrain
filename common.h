#include <GL/glfw.h>

#define PI                     3.14159265358979323846f
#define PIx180                 180.0f*PI
#define VIEW_DISTANCE                       5000
#define VIEW_DISTANCE_HALF                  2500
#define TERRAIN_GRID_SIZE                   80
#define TERRAIN_GRID_SIZE_HALF              40
#define TERRAIN_SQUARE_SIZE                 100
#define TERRAIN_CENTRE_DISTANCE             (TERRAIN_SQUARE_SIZE*TERRAIN_GRID_SIZE*0.4f)
#define TERRAIN_STEP_SIZE                   8
#define TERRAIN_WATER_LEVEL                 -500
#define T_TYPE_WATER                        1
#define T_TYPE_CRATER                       2
#define T_TYPE_GRASS1                       3
#define T_TYPE_GRASS2                       4
#define T_TYPE_GRASS3                       5
#define T_TYPE_DIRT                         6
#define T_TYPE_ROCK                         7
#define T_TYPE_WATER_EDGE                   9
#define KEY_NONE                            0
#define KEY_LMB                             5
#define KEY_LMB_RELEASE                     6
#define KEY_MMB                             7
#define KEY_RMB                             9
#define INPUT_NONE                          0
#define INPUT_UP                            1
#define INPUT_DOWN                          2
#define INPUT_LEFT                          3
#define INPUT_RIGHT                         4
#define INPUT_UP_RIGHT                      5
#define INPUT_UP_LEFT                       6
#define INPUT_DOWN_RIGHT                    7
#define INPUT_DOWN_LEFT                     8
#define INPUT_VERT_UP                       9
#define INPUT_VERT_DOWN                     10


struct v2i {
  int x;
  int y;
};
struct v2f {
  float x;
  float y;
};
struct v3i {
  int x;
  int y;
  int z;
};
struct v3f {
  float x;
  float y;
  float z;
};

struct model {
  GLenum type;
  GLuint num_indices;
  GLfloat (*verts)[3];
  GLfloat (*norms)[3];
  GLuint *indices;
  GLuint *tex_indices;
  GLfloat (*tex_coords)[2];
};
/*struct model {
  GLenum type;
  GLuint num_indices;
  GLfloat verts[5000][3];
  GLfloat norms[5000][3];
  GLuint indices[5000];
  GLuint tex_indices[5000];
  GLfloat tex_coords[5000][2];
};*/

float algorithmicTerrainHeight(float x, float z);
float readTerrainHeight(int x, int y);
void moveTerrain(struct v3f camerapos, struct v3f camerarot, struct v2f *sector, int *swapb, int squaresize);
void drawTerrain(struct v3f camerapos, struct v3f camerarot, struct v2f *sector, float camheight, int *swapb, int *squaresize);
void loadFromOBJFile(char *name, struct model *model);
void drawModel(struct model model, struct v3f pos, struct v3f rot, GLfloat size, GLuint alpha);