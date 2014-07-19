#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeImage.h>
#include "maths.h"


static void keyInputGLFW(GLFWwindow* window, int key, int scancode, int action, int mods);


static void errorGLFW(int error, const char* err)
{
  printf("GLFW: %s\n", err);
}


static void errorFreeImage(FREE_IMAGE_FORMAT fif, const char *err)
{
  printf("FreeImage: %s\n", err);
}


void loadTexture2D(const char *file)
{
  FIBITMAP *img = FreeImage_Load(FreeImage_GetFileType(file, 0), file, 0);
  img = FreeImage_ConvertTo32Bits(img);
  GLsizei width = FreeImage_GetWidth(img);
  GLsizei height = FreeImage_GetHeight(img);
  GLubyte *bits = (GLubyte*) FreeImage_GetBits(img);
  int i;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *) bits);
  for (i = 1; i < 10; i++){
    width /= 2;
    height /= 2;
    bits = (GLubyte*) FreeImage_GetBits(FreeImage_Rescale(img, width, height, FILTER_BICUBIC));
    glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *) bits);
    if (width == 1 || height == 1)
      break;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i);
  FreeImage_Unload(img);
}


long fileLength(const char *file)
{
  FILE *fp = NULL;
  long len = 0;

  if ((fp = fopen(file, "r"))) {
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fclose(fp);
    return len;
  }
  return len;
}


void loadGLSL(GLchar *src, long len, const char *file)
{
  FILE *fp = NULL;

  if ((fp = fopen(file, "r"))) {
    fread(src, 1, len, fp);
    src[len++] = '\0';
    fclose(fp);
  }
}


GLFWwindow *startGraphics(GLFWwindow *window, GLuint *textures, GLuint *shaders)
{
  int lightpos[]  = {1, 0, 500, 0};
  int width, height;
  // float lightdir[]  = {0.0f, -1.0f, 0.0f};
  float lightspec[] = {0.15f, 0.18f, 0.17f, 1.0f};
  float lightamb[]  = {0.1f, 0.07f, 0.08f, 1.0f};
  float lightdiff[] = {0.82f, 0.77f, 0.75f, 1.0f};
  GLuint fragShader, vertShader;
  GLchar *fragSrc = NULL, *vertSrc = NULL;
  long filelen;
  GLsizei logSize;
  GLchar log[5000];

  glfwSetErrorCallback(errorGLFW);
  if (glfwInit() == GL_FALSE)
    return NULL;
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  glfwWindowHint(GLFW_ACCUM_RED_BITS, 8);
  glfwWindowHint(GLFW_ACCUM_GREEN_BITS, 8);
  glfwWindowHint(GLFW_ACCUM_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
  width = 1366;
  height = 768;
  window = glfwCreateWindow(width, height, "test", glfwGetPrimaryMonitor(), NULL);
  if (window == NULL)
    return NULL;
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyInputGLFW);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearDepth(1.0f);
  glDepthRange(0.0f, 1.0f);
  glDepthFunc(GL_LESS);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LIGHTING);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0f);
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glHint(GL_FOG_HINT, GL_NICEST);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glShadeModel(GL_SMOOTH);
  glMatrixMode(GL_PROJECTION);
  glFrustum(-0.54f, 0.54f, -0.4f, 0.4f, 0.8f, 50000.0f);
  glMatrixMode(GL_MODELVIEW);
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  FreeImage_Initialise(GL_FALSE);
  FreeImage_SetOutputMessage(errorFreeImage);
  glEnable(GL_TEXTURE_2D);
  glGenTextures(5, textures);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  loadTexture2D("data/textures/terrain.tga");
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  loadTexture2D("data/textures/foliage.tga");
  glBindTexture(GL_TEXTURE_2D, textures[2]);
  loadTexture2D("data/textures/cloud_alpha.tga");
  glBindTexture(GL_TEXTURE_2D, textures[3]);
  loadTexture2D("data/textures/fighter.png");
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glEnable(GL_LIGHT0);
  glLightiv(GL_LIGHT0, GL_POSITION, lightpos);
  //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 77.0f);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 10.0f);
  //glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 100.0f);
  //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightdir);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightspec);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdiff);
  glEnable(GL_LIGHT1);
  lightpos[1]  = 5000;
  glLightiv(GL_LIGHT1, GL_POSITION, lightpos);
  glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 10.0f);
  lightspec[0] = 0.12f;
  lightspec[1] = 0.1f;
  lightspec[2] = 0.11f;
  glLightfv(GL_LIGHT1, GL_SPECULAR, lightspec);
  lightamb[0] = 0.0f;
  lightamb[1] = 0.0f;
  lightamb[2] = 0.0f;
  glLightfv(GL_LIGHT1, GL_AMBIENT, lightamb);
  lightdiff[0] = 0.15f;
  lightdiff[1] = 0.17f;
  lightdiff[2] = 0.19f;
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightdiff);

  printf("%s\n", glGetString(GL_VERSION));
  printf("%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  glActiveTexureARB = (PFNGLACTIVETEXTUREARBPROC) glfwGetProcAddress("glActiveTexureARB");
  glCreateProgramARB = (PFNGLCREATEPROGRAMPROC) glfwGetProcAddress("glCreateProgram");
  glCreateShaderARB = (PFNGLCREATESHADERPROC) glfwGetProcAddress("glCreateShader");
  glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) glfwGetProcAddress("glShaderSourceARB");
  glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC) glfwGetProcAddress("glGetShaderSourceARB");
  glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC) glfwGetProcAddress("glCompileShaderARB");
  glGetShaderInfoLogARB = (PFNGLGETSHADERINFOLOGPROC) glfwGetProcAddress("glGetShaderInfoLog");
  glGetProgramInfoLogARB = (PFNGLGETPROGRAMINFOLOGPROC) glfwGetProcAddress("glGetProgramInfoLog");
  glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC) glfwGetProcAddress("glGetProgramivARB");
  glAttachShaderARB = (PFNGLATTACHSHADERPROC) glfwGetProcAddress("glAttachShader");
  glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) glfwGetProcAddress("glLinkProgramARB");
  glUseProgramARB = (PFNGLUSEPROGRAMPROC) glfwGetProcAddress("glUseProgram");
  glUniform1iARB = (PFNGLUNIFORM1IARBPROC) glfwGetProcAddress("glUniform1iARB");
  glUniform1fARB = (PFNGLUNIFORM1FARBPROC) glfwGetProcAddress("glUniform1fARB");
  glUniform2fvARB = (PFNGLUNIFORM3FVARBPROC) glfwGetProcAddress("glUniform1fARB");
  glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC) glfwGetProcAddress("glUniformMatrix4fvARB");
  glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC) glfwGetProcAddress("glGetUniformLocationARB");
  glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC) glfwGetProcAddress("glGetAttribLocationARB");
  glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC) glfwGetProcAddress("glBindAttribLocationARB");
  glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC) glfwGetProcAddress("glVertexAttrib3fvARB");

  shaders[0] = glCreateProgramARB();
  vertShader = glCreateShaderARB(GL_VERTEX_SHADER);
  filelen = fileLength("data/shaders/1.vsh");
  vertSrc = (GLchar *) malloc(sizeof(GLchar) * filelen);
  loadGLSL(vertSrc, filelen, "data/shaders/1.vsh");
  printf("%s\n", vertSrc);
  glShaderSourceARB(vertShader, 1, (const GLchar **) &vertSrc, NULL);
  free(vertSrc);
  glCompileShaderARB(vertShader);
  glGetShaderInfoLogARB(vertShader, 500, &logSize, log);
  if (logSize)
    printf("GLSL vertex: %s\n", log);
  glAttachShaderARB(shaders[0], vertShader);
  fragShader = glCreateShaderARB(GL_FRAGMENT_SHADER);
  filelen = fileLength("data/shaders/2.fsh");
  fragSrc = (GLchar *) malloc(sizeof(GLchar) * filelen);
  loadGLSL(fragSrc, filelen, "data/shaders/2.fsh");
  printf("%s\n", fragSrc);
  glShaderSourceARB(fragShader, 1, (const GLchar **) &fragSrc, NULL);
  free(fragSrc);
  glCompileShaderARB(fragShader);
  glGetShaderInfoLogARB(fragShader, 500, &logSize, log);
  if (logSize)
    printf("GLSL fragment: %s\n", log);
  glAttachShaderARB(shaders[0], fragShader);
  glLinkProgramARB(shaders[0]);
  glGetProgramInfoLogARB(shaders[0], 500, &logSize, log);
  if (logSize)
    printf("%s\n", log);

  return window;
}


void axisMultiplier(float *quat1_in, float *quat2_in, float *quat3_in)
{
  quat1_in[0] = quat2_in[0] * quat3_in[0] - quat2_in[0] * quat3_in[1] - quat2_in[2] * quat3_in[2] - quat2_in[3] * quat3_in[3];
  quat1_in[1] = quat2_in[0] * quat3_in[1] + quat2_in[1] * quat3_in[0] + quat2_in[2] * quat3_in[3] - quat2_in[3] * quat3_in[2];
  quat1_in[2] = quat2_in[0] * quat3_in[2] + quat2_in[2] * quat3_in[0] + quat2_in[3] * quat3_in[1] - quat2_in[1] * quat3_in[3];
  quat1_in[3] = quat2_in[0] * quat3_in[3] + quat2_in[3] * quat3_in[0] + quat2_in[1] * quat3_in[2] - quat2_in[2] * quat3_in[1];
}


void createQuats(float *quat, char axis, float degrees)
{
  float radians = ((degrees / 180.0f) * PI);
  float result = sinf(radians / 2.0f);

  quat[3] = 0.0f;
  if (axis == 'x') {
    quat[0] = cosf(radians / 2.0f);
    quat[1] = result;
    quat[2] = 0.0f;
  }
  else /*if (axis == "y")*/ {
    quat[0] = cosf(radians / 2.0f);
    quat[2] = result;
    quat[1] = 0.0f;
  }
}


void createMatrix(float *quat, float *viewmatrix)
{
  viewmatrix[0] = 1.0f - 2.0f * (quat[2] * quat[2] + quat[3] * quat[3]);
  viewmatrix[1] = 2.0f * (quat[1] * quat[2] + quat[3] * quat[0]);
  viewmatrix[2] = 2.0f * (quat[1] * quat[3] - quat[2] * quat[0]);
  viewmatrix[3] = 0.0f;
  viewmatrix[4] = 2.0f * (quat[1]* quat[2] - quat[3] * quat[0]);
  viewmatrix[5] = 1.0f - 2.0f * (quat[1] * quat[1] + quat[3] * quat[3]);
  viewmatrix[6] = 2.0f * (quat[3] * quat[2] + quat[1] * quat[0]);
  viewmatrix[7] = 0.0f;
  viewmatrix[8] = 2.0f * (quat[1] * quat[3] + quat[2] * quat[0]);
  viewmatrix[9] = 2.0f * (quat[2]* quat[3] - quat[1] * quat[0]);
  viewmatrix[10] = 1.0f - 2.0f * (quat[1] * quat[1] + quat[2] * quat[2]);
  viewmatrix[11] = 0.0f;
  viewmatrix[12] = 0;
  viewmatrix[13] = 0;
  viewmatrix[14] = 0;
  viewmatrix[15] = 1.0f;
}


void updateCamera(struct v3f cameraRot)
{
  static float q1[4], q2[4], q3[4];
  static float viewmatrix[16];

  glLoadIdentity();
  createQuats(q3, 'x', cameraRot.x);
  axisMultiplier(q1, q2, q3);
  q2[0] = q3[0];
  q2[1] = q3[1];
  createQuats(q3, 'y', cameraRot.y);
  axisMultiplier(q1, q2, q3);
  createMatrix(q1, viewmatrix);
  glMultMatrixf(viewmatrix);
}


float cameraHeight(struct v3f camerapos)
{
  static float height;
  float ground;

  ground = fabs((int) (-camerapos.y - readTerrainHeight(-camerapos.x, -camerapos.z)));
  ground = ground < TERRAIN_WATER_LEVEL ? TERRAIN_WATER_LEVEL : ground;
  height -= (height - ground) * 0.11f;

  return height;
}


void cameraTrailMovement(struct v3f *camerapos, struct v3f *camerarot, struct v3f modelpos, struct v3f modelrot)
{
  struct v3f temppos = modelpos;
  float temp = 0.0f, ground = 0.0f;
  const int offset = TERRAIN_SQUARE_SIZE / 4;

  degreestovector3d(&temppos, modelrot, mv3f(0, 180, 0), 150);
  camerapos->x += (temppos.x - camerapos->x) * 0.27f;
  camerapos->y += (-temppos.y - camerapos->y) * 0.27f;
  camerapos->z += (temppos.z - camerapos->z) * 0.27f;
  camerarot->x = modelrot.x;
  camerarot->y = vectorstodegree2d(mv3f(modelpos.x, 0, modelpos.z), *camerapos) - 180;
  ground = -readTerrainHeight(-camerapos->x, -camerapos->z);
  temp = -readTerrainHeight(-camerapos->x + offset, -camerapos->z + offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x + offset, -camerapos->z - offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x - offset, -camerapos->z + offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x - offset, -camerapos->z - offset);
  ground = ground < temp ? ground : temp;
  ground += -TERRAIN_SQUARE_SIZE * 0.07f;
  ground = ground > TERRAIN_WATER_LEVEL - 70 ? TERRAIN_WATER_LEVEL - 70 : ground;
  camerapos->y = camerapos->y > ground ? ground : camerapos->y;
}


void mouseLook(GLFWwindow *window, struct v3f *camerarot)
{
  const float mouse_sensitivity = 0.1f;
  struct v2d mouse_pos;

  glfwGetCursorPos (window, &mouse_pos.x, &mouse_pos.y);
  if (camerarot->x > 90.0f)
    glfwSetCursorPos (window, mouse_pos.x, (int) (90 / mouse_sensitivity));
  if (camerarot->x < -90.0f)
    glfwSetCursorPos (window, mouse_pos.x, (int) (-90 / mouse_sensitivity));
  while (camerarot->y >= 360.0f)
    camerarot->y -= 360.0f;
  while (camerarot->y < 0.0f)
    camerarot->y += 360.0f;
  // mouse x, y / view x, y swapped here...
  camerarot->y = (float) (mouse_pos.x * mouse_sensitivity);
  camerarot->x = (float) (mouse_pos.y * mouse_sensitivity);
}


static void keyInputGLFW(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}


void keyboardInput(GLFWwindow *window, char *direction)
{
  char x = 0, z = 0;

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, 'W') == GLFW_PRESS)
    z++;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, 'S') == GLFW_PRESS)
    z--;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, 'D') == GLFW_PRESS)
    x++;
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, 'A') == GLFW_PRESS)
    x--;
  if (x == 0 && z == 0)
    *direction = INPUT_NONE;
  else if (x == 0 && z == 1)
    *direction = INPUT_UP;
  else if (x == 0 && z == -1)
    *direction = INPUT_DOWN;
  else if (x == 1 && z == 0)
    *direction = INPUT_RIGHT;
  else if (x == -1 && z == 0)
    *direction = INPUT_LEFT;
  else if (x == 1 && z == 1)
    *direction = INPUT_UP_RIGHT;
  else if (x == -1 && z == 1)
    *direction = INPUT_UP_LEFT;
  else if (x == 1 && z == -1)
    *direction = INPUT_DOWN_RIGHT;
  else if (x == -1 && z == -1)
    *direction = INPUT_DOWN_LEFT;
}


void movement(struct v3f *camerapos, struct v3f camerarot, char direction, float speed)
{
  float ground, temp;
  const int offset = TERRAIN_SQUARE_SIZE / 4;

  switch (direction) {
  case INPUT_UP:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, 0.0f, 0.0f), speed);
    break;
  case INPUT_DOWN:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, 0.0f, 0.0f), -speed);
    break;
  case INPUT_LEFT:
    degreestovector3d(camerapos, mv3f(0.0f, camerarot.y, 0.0f),
                                   mv3f(0.0f, 270.0f, 0.0f), speed);
    break;
  case INPUT_RIGHT:
    degreestovector3d(camerapos, mv3f(0.0f, camerarot.y, 0.0f),
                                   mv3f(0.0f, 90.0f, 0.0f), speed);
    break;
  case INPUT_UP_RIGHT:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, 45.0f, 0.0f), speed);
    break;
  case INPUT_UP_LEFT:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, -45.0f, 0.0f), speed);
    break;
  case INPUT_DOWN_RIGHT:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, -45.0f, 0.0f), -speed);
    break;
  case INPUT_DOWN_LEFT:
    degreestovector3d(camerapos, camerarot, mv3f(0.0f, 45.0f, 0.0f), -speed);
    break;
  default: break;
  }
  ground = -readTerrainHeight(-camerapos->x, -camerapos->z);
  temp = -readTerrainHeight(-camerapos->x + offset, -camerapos->z + offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x + offset, -camerapos->z - offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x - offset, -camerapos->z + offset);
  ground = ground < temp ? ground : temp;
  temp = -readTerrainHeight(-camerapos->x - offset, -camerapos->z - offset);
  ground = ground < temp ? ground : temp;
  ground += -TERRAIN_SQUARE_SIZE * 0.02f;
  ground = ground > TERRAIN_WATER_LEVEL - 70 ? TERRAIN_WATER_LEVEL - 70 : ground;
  camerapos->y = camerapos->y > ground ? ground : camerapos->y;
  //camerapos->y = ground;
}


void flyMovement(struct airunit *unit, char input)
{
  struct v3f temppos;
  float ground, speed, temp1, temp2;
  const int offset = TERRAIN_SQUARE_SIZE / 4;

  switch (input) {
  case INPUT_UP:
    unit->thrust = unit->thrust + 0.05f;
    break;
  case INPUT_DOWN:
    unit->thrust = unit->thrust - 0.05f;
    break;
  case INPUT_LEFT:
    unit->rot.z -= 5.0f;
    break;
  case INPUT_RIGHT:
    unit->rot.z += 5.0f;
    break;
  case INPUT_UP_RIGHT:
    unit->thrust = unit->thrust + 0.05f;
    unit->rot.z += 5.0f;
    break;
  case INPUT_UP_LEFT:
    unit->thrust = unit->thrust + 0.05f;
    unit->rot.z -= 5.0f;
    break;
  case INPUT_DOWN_RIGHT:
    unit->thrust = unit->thrust - 0.05f;
    unit->rot.z += 5.0f;
    break;
  case INPUT_DOWN_LEFT:
    unit->thrust = unit->thrust - 0.05f;
    unit->rot.z -= 5.0f;
    break;
  default: break;
  }
  ground = readTerrainHeight(-unit->pos.x, -unit->pos.z);
  temp1 = readTerrainHeight(-unit->pos.x + offset, -unit->pos.z + offset);
  ground = ground > temp1 ? ground : temp1;
  temp1 = readTerrainHeight(-unit->pos.x + offset, -unit->pos.z - offset);
  ground = ground > temp1 ? ground : temp1;
  temp1 = readTerrainHeight(-unit->pos.x - offset, -unit->pos.z + offset);
  ground = ground > temp1 ? ground : temp1;
  temp1 = readTerrainHeight(-unit->pos.x - offset, -unit->pos.z - offset);
  ground = ground > temp1 ? ground : temp1;
  ground += TERRAIN_SQUARE_SIZE * 0.02f;
  ground = ground < TERRAIN_WATER_LEVEL + 20 ? TERRAIN_WATER_LEVEL + 20 : ground;
  temp1 = 1 - fabs(unit->pos.y - 9700) / 19000.0f;
  temp1 = temp1 < 0.0f ? 0.0f : temp1;
  temp2 = 1 - unit->pos.y / 17000.0f;
  temp2 = temp2 < 0.0f ? 0.0f : temp2;
  temppos = unit->pos;
  unit->thrust = unit->thrust > unit->maxthrust ? unit->maxthrust : unit->thrust < 0 ? 0 : unit->thrust;
  degreestovector3d(&temppos, unit->rot, mv3f(180.0f, 180.0f, 0.0f), unit->thrust * temp1);
  unit->vec.x += temppos.x - unit->pos.x;
  unit->vec.y += temppos.y - unit->pos.y;
  unit->vec.z += temppos.z - unit->pos.z;
  speed = distance2d(mv3f(0, 0, 0), unit->vec) * 0.5f;
  speed = speed < 300 ? speed : 300;
  temppos = mv3f(0, 0, 0);
  degreestovector3d(&temppos, unit->rot, mv3f(180.0f, 180.0f, 0.0f), speed * temp2 * 0.7f);
  temp1 = (vectorstodegree2d(temppos, mv3f(0, 0, 0)) - vectorstodegree2d(unit->vec, mv3f(0, 0, 0))) * 0.05f;
  if (vectorstodegree2d(temppos, mv3f(0, 0, 0)) > vectorstodegree2d(unit->vec, mv3f(0, 0, 0)) + 0.8f)
    unit->rot.z += temp1 > 21 ? 0 : temp1;
  else if (vectorstodegree2d(unit->vec, mv3f(0, 0, 0)) > vectorstodegree2d(temppos, mv3f(0, 0, 0)) + 0.8f)
    unit->rot.z += temp1 < -21 ? 0 : temp1;
  else
    unit->rot.z -= unit->rot.z * 0.07f;
  unit->rot.z = unit->rot.z > 70 ? 70 : unit->rot.z < -70 ? -70 : unit->rot.z;
  unit->pos.x += temppos.x + unit->vec.x;
  unit->pos.y += temppos.y + unit->vec.y;
  unit->pos.z += temppos.z + unit->vec.z;
  unit->vec.y += speed * 0.017f * temp2 - WORLD_GRAVITY;
  temp1 = (1.5f - unit->vec.y) * 0.1f;
  temp1 = temp1 < 50 ? temp1 - 3 > 0 ? temp1 - 3 : 0 : 50;
  degreestovector3d(&unit->vec, unit->rot, mv3f(180.0f, 180.0f, 0.0f), temp1 * temp2 > 0 ? temp1 * temp2 : 0);
  unit->vec.x -= unit->vec.x * 0.002f * speed * temp2;
  unit->vec.y -= unit->vec.y * 0.002f * speed * temp2;
  unit->vec.z -= unit->vec.z * 0.002f * speed * temp2;
  temp2 = fabs(unit->pos.y - ground);
  if (temp2 < 500)
    unit->vec.y += 0.2f * (500 - temp2) / 500.0f;
  if (temp2 < 1250)
    unit->vec.y += 0.5f * (1250 - temp2) / 1250.0f;
  if (unit->pos.y < ground) {
    unit->vec.y = 0.0f;
    unit->pos.y = ground;
    unit->vec.x -= unit->vec.x * 0.4f;
    unit->vec.z -= unit->vec.z * 0.4f;
    unit->thrust -= unit->thrust * 0.08f;
  }
}


void updateAirPositions(struct airunit *airunits)
{

}


int main(int argc, char *argv[])
{
  GLuint textures[5], shaders[5];
  GLFWwindow *window = NULL;
  int swapb = 1, squaresize = 0;
  char direction;
  float camheight = 0.0f, fogend = 20.0f;
  struct v2f sector    = {0.0f, 0.0f};
  struct v3f camerarot = {0.0f, 0.0f, 0.0f};
  struct v3f camerapos = {0.0f, 0.0f, 0.0f};
  struct model *models = malloc(sizeof(struct model) * 8);
  struct airunit *airunits = malloc(sizeof(struct airunit) * 16);

  if ((window = startGraphics(window, textures, shaders)) != NULL) {
    glfwSwapBuffers(window);
    loadFromOBJFile("data/models/tree1.obj", &models[0]);
    loadFromOBJFile("data/models/tree2.obj", &models[1]);
    loadFromOBJFile("data/models/tree3.obj", &models[2]);
    loadFromOBJFile("data/models/tree4.obj", &models[3]);
    loadFromOBJFile("data/models/mtree1.obj", &models[4]);
    loadFromOBJFile("data/models/stump1.obj", &models[5]);
    loadFromOBJFile("data/models/fighter1.obj", &models[6]);
    loadFromOBJFile("data/models/fighter2.obj", &models[7]);
    airunits[0].maxthrust = 10.0f;
    while (!glfwWindowShouldClose(window)) {
      camheight = cameraHeight(camerapos);
      keyboardInput(window, &direction);
      //mouseLook(window, &camerarot);
      mouseLook(window, &airunits[0].rot);
      render(window, models, textures, shaders, &swapb, camerapos, camerarot, &sector, camheight, &squaresize, &fogend, airunits[0].pos, airunits[0].rot);
      //movement(&camerapos, camerarot, direction, 100);
      flyMovement(&airunits[0], direction);
      updateAirPositions(airunits);
      cameraTrailMovement(&camerapos, &camerarot, airunits[0].pos, airunits[0].rot);
      updateCamera(camerarot);
      glTranslatef(camerapos.x, camerapos.y, camerapos.z);
    }
    free(models);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
  }
  else
    return EXIT_FAILURE;
}
