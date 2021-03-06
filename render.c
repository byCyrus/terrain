#include <assimp/scene.h>
#include "maths.h"


void updateFogAndFrustum(GLfloat *clear, struct v3f camerapos, int t_size)
{
  struct v3f pos;
  float fog_end = t_size * TERRAIN_GRID_SIZE * 0.45f;
  static float fog_start = 0.0f;
  float ground = readTerrainHeightPlane(camerapos.x, camerapos.z, &pos, t_size);
  float fstart;
  float temp = camerapos.y - TERRAIN_SCALE_HEIGHT;

  if (temp < 1250.0f && camerapos.y > TERRAIN_SCALE_HEIGHT)
    fog_end -= (1 - temp / 1250.0f) * (fog_end * 0.66667f);
  if (camerapos.y < LOWER_CLOUD_HEIGHT)
    temp = fog_end * 0.3f;
  else if (camerapos.y < TERRAIN_SCALE_HEIGHT)
    temp = fog_end * 0.5f;
  else
    temp = fog_end * 0.7f;
  if (fog_start > fog_end * 0.7f)
    fog_start = fog_end * 0.7f;
  else
    fog_start += (temp - fog_start) * 0.1f;
  ground = ground < TERRAIN_WATER_LEVEL ? TERRAIN_WATER_LEVEL : ground;
  fstart = (camerapos.y - ground) * 0.1f;
  if (fstart > 10.0f)
    fstart = 10.0f;
  else if (fstart < 1.0f)
    fstart = 1.0f;
  glFogfv(GL_FOG_COLOR, clear);
  glFogf(GL_FOG_START, fog_start);
  glFogf(GL_FOG_END, fog_end);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-0.54f * fstart, 0.54f * fstart, -0.4f * fstart, 0.4f * fstart,
            0.8f * fstart, fog_end * 1.1f);
  glMatrixMode(GL_MODELVIEW);
}


void grassQuad(struct v3f pos, float rot, char type, GLuint alpha)
{
  int h;
  float u, v, vv;

  switch (type) {
  case GRASS_GRASS1:
    h = 1;
    u = 0.0f;
    v = 0.0f;
    break;
  case GRASS_GRASS2:
    h = 1;
    u = 0.0f;
    v = 0.25f;
    break;
  case GRASS_BUSH1:
    h = 1;
    u = 0.505f;
    v = 0.0f;
    break;
  case GRASS_BUSH2:
    h = 1;
    u = 0.505f;
    v = 0.25f;
    break;
  case GRASS_BAMBOO:
    h = 2;
    u = 0.0f;
    v = 0.5f;
    break;
  case GRASS_DEAD:
    h = 1;
    u = 0.505f;
    v = 0.5f;
    break;
  case GRASS_FLOWERS:
    h = 1;
    u = 0.505f;
    v = 0.75f;
    break;
  default:
    h = 1;
    u = 0.0f;
    v = 0.0f;
  }
  if (h == 1)
    vv = 0.245f;
  else
    vv = 0.495f;
  glMateriali(GL_FRONT, GL_SHININESS, 170);
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(rot, 0.0f, 1.0f, 0.0f);
  glBegin(GL_QUADS);
  glColor4ub(255, 255, 255, alpha);
  glNormal3i(0, 0, 1);
  glTexCoord2f(u, v);
  glVertex3i(0, 0, 0);
  glTexCoord2f(u + 0.495f, v);
  glVertex3i(1, 0, 0);
  glTexCoord2f(u + 0.495f, v + vv);
  glVertex3i(1, h, 0);
  glTexCoord2f(u, v + vv);
  glVertex3i(0, h, 0);
  glEnd();
  glPopMatrix();
}


void renderGrass(GLuint *textures, struct v3f camerapos, struct v3f camerarot, int t_size, float fps)
{
  int xgrid, zgrid, x, z, x1, z1, cull, density, i, a;
  static char update = 1;
  static float v_dist = VIEW_DISTANCE_HALF;
  const int size = t_size * 0.05f; /* Size of generation sector, also affects density. */
  float xpos, zpos, dist, rot, v_dist_half;
  static struct v3f sector;
  static struct v3f normal[SCENERY_SIZE_GRASS];
  static float height[SCENERY_SIZE_GRASS];
  static unsigned char type[SCENERY_SIZE_GRASS];
  GLubyte alpha;

  glMateriali(GL_FRONT, GL_SHININESS, 92);
  glBindTexture(GL_TEXTURE_2D, textures[TEX_FOLIAGE_GRASS]);
  if (distance2d(camerapos, sector) > t_size){
    sector = camerapos;
    update = 1;
  }
  x = (int) (sector.x / size);
  z = (int) (sector.z / size);
  if (fps < 21.0f && v_dist > 150.0f) {
    if (fps < 17.0f)
      v_dist -= 25.0f;
    else
      v_dist -= 2.5f;
  }
  else if (v_dist < VIEW_DISTANCE_HALF)
    v_dist++;
  v_dist_half = v_dist / 2.0f;
  for (xgrid = 0, zgrid = 0, i = 0; i < SCENERY_SIZE_GRASS && xgrid < TERRAIN_GRID_SIZE_HALF && zgrid < TERRAIN_GRID_SIZE_HALF; xgrid++) {
    xpos = (xgrid - TERRAIN_GRID_SIZE_QUARTER + x) * size;
    zpos = (zgrid - TERRAIN_GRID_SIZE_QUARTER + z) * size;
    x1 = sqrt(fabs(xpos * zpos + 83)) * 213;
    x1 = (int) (x1 + xpos) % 77;
    z1 = (int) (x1 + zpos) % 73;
    xpos += z1;
    zpos += x1;
    dist = distance3d(camerapos, mv3f(xpos, height[i], zpos));
    rot = 180 - vectorstodegree2d(camerapos, mv3f(xpos, 0, zpos));
    cull = fabs((int) (camerarot.y - rot));
    while (cull >= 360)
      cull -= 360;
    if (cull <= 85 || cull >= 275 || camerarot.x > 27 || update == 1 || dist < t_size * 2) {
      if (update) {
        height[i] = readTerrainHeightPlane(xpos, zpos, &normal[i], t_size);
        height[i] -= distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) * 0.5f;
        type[i] = readTerrainType(xpos, zpos);
      }
      x1 = x1 * x1 + z1 * z1;
      x1 = x1 % SCENERY_DENSITY_GRASS;
      switch (type[i]) {
      case T_TYPE_GRASS1:
        density = 710;
        break;
      case T_TYPE_GRASS2:
        density = 747;
        break;
      case T_TYPE_GRASS3:
        density = 701;
        break;
      case T_TYPE_VILLAGE:
        density = 880;
        break;
      case T_TYPE_FOREST1:
        density = 970;
        break;
      case T_TYPE_FOREST2:
        density = 920;
        break;
      case T_TYPE_DIRT:
        density = 376;
        break;
      case T_TYPE_DESERT:
        density = 261;
        break;
      case T_TYPE_SNOW:
        density = 0;
        break;
      default:
        density = 670;
      }
      if (dist < v_dist) {
        if (height[i] > TERRAIN_WATER_LEVEL + 50) {
          if (dist < v_dist_half)
            alpha = 255;
          else if (dist < v_dist)
            alpha = (GLubyte) (255 - ((dist - v_dist_half) / v_dist_half) * 255);
          else
            alpha = 0;
          if (x1 < density) {
            if (type[i] == T_TYPE_DIRT || type[i] == T_TYPE_DESERT)
              grassQuad(mv3f(xpos, height[i], zpos), rot, GRASS_DEAD, alpha);
            else if (type[i] == T_TYPE_GRASS1) {
              a = x1 % 6;
              if (a < 2)
                grassQuad(mv3f(xpos, height[i], zpos), rot, GRASS_GRASS1, alpha);
              else if (a < 4)
                grassQuad(mv3f(xpos, height[i], zpos), rot, GRASS_GRASS2, alpha);
              else
                grassQuad(mv3f(xpos, height[i], zpos), rot, GRASS_FLOWERS, alpha);
            }
            else
              grassQuad(mv3f(xpos, height[i], zpos), rot, x1 % 8, alpha);
          }
        }
      }
    }
    i++;
    if (xgrid >= TERRAIN_GRID_SIZE_HALF - 1) {
      zgrid++;
      xgrid = -1;
    }
  }
  update = 0;
}


void renderGroundScenery(struct aiScene *scene, GLuint *textures, struct v3f camerapos,
                         struct v3f camerarot, int t_size, float fps)
{
  int xgrid, zgrid, x, z, x1, z1, cull, density, i, a;
  static char update = 1;
  static float v_dist = VIEW_DISTANCE;
  const int size = t_size * 0.2f; /* Size of generation sector, also affects density. */
  float xpos, zpos, dist, v_dist_half;
  static struct v3f sector;
  static struct v3f normal[SCENERY_SIZE];
  static float height[SCENERY_SIZE];
  static unsigned char type[SCENERY_SIZE];
  GLubyte alpha;
  GLuint color[3];

  glMateriali(GL_FRONT, GL_SHININESS, 92);
  if (distance2d(camerapos, sector) > SCENERY_STEP_SIZE * t_size){
    sector = camerapos;
    update = 1;
  }
  x = (int) (sector.x / size);
  z = (int) (sector.z / size);
  if (fps < 18.0f && v_dist > 300.0f) {
    if (fps < 15.0f)
      v_dist -= 25.0f;
    else
      v_dist -= 2.5f;
  }
  else if (v_dist < VIEW_DISTANCE)
    v_dist++;
  v_dist_half = v_dist / 2.0f;
  for (xgrid = 0, zgrid = 0, i = 0; i < SCENERY_SIZE && xgrid < TERRAIN_GRID_SIZE && zgrid < TERRAIN_GRID_SIZE; xgrid++) {
    xpos = (xgrid - TERRAIN_GRID_SIZE_HALF + x) * size;
    zpos = (zgrid - TERRAIN_GRID_SIZE_HALF + z) * size;
    x1 = sqrt(fabs(xpos * zpos + 107)) * 123;
    x1 = (int) (x1 + xpos) % 88;
    z1 = (int) (x1 + zpos) % 85;
    xpos += z1;
    zpos += x1;
    dist = distance3d(camerapos, mv3f(xpos, height[i], zpos));
    cull = fabs((int) (camerarot.y - 180 - vectorstodegree2d(camerapos, mv3f(xpos, 0, zpos))));
    while (cull >= 360)
      cull -= 360;
    if (cull <= 85 || cull >= 275 || camerarot.x > 27 || update == 1 || dist < t_size * 2) {
      if (update) {
        height[i] = readTerrainHeightPlane(xpos, zpos, &normal[i], t_size);
        type[i] = readTerrainType(xpos, zpos);
      }
      x1 = x1 * x1 + z1 * z1;
      x1 = x1 % SCENERY_DENSITY;
      switch (type[i]) {
      case T_TYPE_GRASS1:
        density = 210;
        break;
      case T_TYPE_GRASS2:
        density = 327;
        break;
      case T_TYPE_GRASS3:
        density = 201;
        break;
      case T_TYPE_VILLAGE:
        density = 720;
        break;
      case T_TYPE_FOREST1:
        density = 470;
        break;
      case T_TYPE_FOREST2:
        density = 420;
        break;
      case T_TYPE_DIRT:
        density = 76;
        break;
      case T_TYPE_DESERT:
        density = 52;
        break;
      case T_TYPE_SNOW:
        density = 103;
        break;
      default:
        density = 170;
      }
      if (dist < v_dist) {
        if (height[i] > TERRAIN_WATER_LEVEL + 50) {
          if (dist < v_dist_half)
            alpha = 255;
          else if (dist < v_dist)
            alpha = (GLubyte) (255 - ((dist - v_dist_half) / v_dist_half) * 255);
          else
            alpha = 0;
          if (x1 < density) {
            glBindTexture(GL_TEXTURE_2D, textures[TEX_FOLIAGE]);
            if (type[i] == T_TYPE_GRASS1 || type[i] == T_TYPE_GRASS2) {
              a = x1 % 15;
              if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.1f && height[i] > 250)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_SPARSE], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
              else if (a < 5)
                drawModel((const struct aiScene *) &scene[MODEL_TREE_POPLAR], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
              else if (a == 6) {
                color[0] = 95; color[1] = 95; color[2] = 95;
                glBindTexture(GL_TEXTURE_2D, textures[TEX_TERRAIN]);
                drawModel2((const struct aiScene *) &scene[MODEL_ROCK1], mv3f(xpos, height[i], zpos), mv3f(x1, z1, 0),
                  1.8f + (z1 % 10) * 0.23f, color, alpha);
              }
              else if (x1 % 15 == 7 && z1 % 7 == 0) {
                if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.3f) {
                  glBindTexture(GL_TEXTURE_2D, textures[TEX_BUILDING]);
                  drawModel((const struct aiScene *) &scene[MODEL_BUILDING_HOUSE1], mv3f(xpos, height[i], zpos), mv3f(0, x1 % 90, 0), 0.35f, alpha);
                }
              }
              else
                drawModel((const struct aiScene *) &scene[x1 % 6], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.33f, alpha);
            }
            else if (type[i] == T_TYPE_FOREST1) {
              if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.1f)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_SPARSE], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.34f, alpha);
              else if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.14f)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_BIG], mv3f(xpos, height[i] - 1, zpos), mv3f(0, x1, 0), 0.34f, alpha);
              else
                drawModel((const struct aiScene *) &scene[x1 % 6], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.34f, alpha);
            }
            else if (type[i] == T_TYPE_FOREST2) {
              if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.1f)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_FIR], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.34f, alpha);
              else if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.14f)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_BIG], mv3f(xpos, height[i] - 1, zpos), mv3f(0, x1, 0), 0.34f, alpha);
              else
                drawModel((const struct aiScene *) &scene[MODEL_TREE_FIR], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.35f, alpha);
            }
            else if (type[i] == T_TYPE_DIRT || type[i] == T_TYPE_DESERT) {
              a = x1 % 3;
              if (a == 0)
                drawModel((const struct aiScene *) &scene[MODEL_TREE_STUMP], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
              else if (a == 1 && height[i] < 3700) {
                color[0] = 255; color[1] = 230; color[2] = 240;
                drawModel2((const struct aiScene *) &scene[MODEL_TREE_BUSH], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.32f, color, alpha);
              }
              else {
                color[0] = 103; color[1] = 111; color[2] = 63;
                glBindTexture(GL_TEXTURE_2D, textures[TEX_TERRAIN]);
                drawModel2((const struct aiScene *) &scene[MODEL_ROCK1], mv3f(xpos, height[i], zpos), mv3f(x1, z1, 0),
                  2.3f + (z1 % 10) * 0.23f, color, alpha);
              }
            }
            else if (type[i] == T_TYPE_VILLAGE) {
              a = (x1 + z1) % 15;
              if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.1f && a < 2)
                drawModel((const struct aiScene *) &scene[MODEL_MTREE_SPARSE], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
              else if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.3f) {
                glBindTexture(GL_TEXTURE_2D, textures[TEX_BUILDING]);
                if (a < 5)
                  drawModel((const struct aiScene *) &scene[MODEL_BUILDING_HOUSE2], mv3f(xpos, height[i], zpos), mv3f(0, x1 % 90, 0), 0.4f, alpha);
                else if (a < 10)
                  drawModel((const struct aiScene *) &scene[MODEL_BUILDING_HOUSE1], mv3f(xpos, height[i], zpos), mv3f(0, x1 % 90, 0), 0.35f, alpha);
                else {
                  glBindTexture(GL_TEXTURE_2D, textures[TEX_FOLIAGE]);
                  drawModel((const struct aiScene *) &scene[MODEL_TREE_POPLAR], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
                }
              }
              else
                drawModel((const struct aiScene *) &scene[x1 % 6], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
            }
            else if (type[i] == T_TYPE_SNOW) {
              if (distance3d(mv3f(0, 1, 0), normalize3d(normal[i])) < 0.45f) {
                color[0] = 155; color[1] = 155; color[2] = 155;
                glBindTexture(GL_TEXTURE_2D, textures[TEX_TERRAIN]);
                drawModel2((const struct aiScene *) &scene[MODEL_ROCK1], mv3f(xpos, height[i], zpos), mv3f(x1, z1, 0),
                           2.35f + (z1 % 10) * 0.71f, color, alpha);
              }
            }
            else if (height[i] < 3100)
              drawModel((const struct aiScene *) &scene[x1 % 6], mv3f(xpos, height[i], zpos), mv3f(0, x1, 0), 0.333f, alpha);
          }
        }
      }
    }
    i++;
    if (xgrid >= TERRAIN_GRID_SIZE - 1) {
      zgrid++;
      xgrid = -1;
    }
  }
  update = 0;
}


void renderSky(struct v3f camerapos, struct v3f camerarot, GLfloat *clear, int t_size)
{
  float fog_end = t_size * TERRAIN_GRID_SIZE * 0.45f;

  glPushMatrix();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glTranslatef(camerapos.x, camerapos.y, camerapos.z);
  glRotatef(-camerarot.y, 0.0f, 1.0f, 0.0f);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glBegin(GL_QUADS);
  glColor3fv(clear);
  glVertex3f(7000.0f, 3000.0f, 50000.0f);
  glVertex3f(-7000.0f, 3000.0f, 50000.0f);
  glVertex3f(-7000.0f, 3000.0f, -fog_end);
  glVertex3f(7000.0f, 3000.0f, -fog_end);
  glVertex3f(7000.0f, 3000.0f, -fog_end);
  glVertex3f(-7000.0f, 3000.0f, -fog_end);
  glVertex3f(-7000.0f, -5000.0f, -fog_end);
  glVertex3f(7000.0f, -5000.0f, -fog_end);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3f(7000.0f, 3000.0f, 50000.0f);
  glVertex3f(7000.0f, 3000.0f, -fog_end);
  glVertex3f(7000.0f, -5000.0f, -fog_end);
  glVertex3f(-7000.0f, 3000.0f, -fog_end);
  glVertex3f(-7000.0f, 3000.0f, 50000.0f);
  glVertex3f(-7000.0f, -5000.0f, -fog_end);
  glEnd();
  glPopMatrix();
}


void renderWater(struct v3f camerapos, struct v3f camerarot, GLfloat color[4], int t_size)
{
  int xshift, zshift, xgrid, zgrid, size = t_size * 8;
  float xpos, zpos;
  const float scale = 0.0001f;

  glMateriali(GL_FRONT, GL_SHININESS, 97);
  glDisable(GL_CULL_FACE);
  glPushMatrix();
  glTranslatef(camerapos.x, 0.0f, camerapos.z);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glScalef(scale, scale, scale);
  glTranslatef(camerapos.x, camerapos.z, 0.0f);
  glBegin(GL_QUADS);
  color[3] = 0.7f;
  glColor4fv(color);
  glNormal3i(0, -1, 0);
  for (xgrid = 0, zgrid = 0; xgrid < TERRAIN_GRID_SIZE_HALF && zgrid < TERRAIN_GRID_SIZE_HALF; xgrid++) {
    xshift = zshift = size;
    xpos = (-TERRAIN_GRID_SIZE_QUARTER + xgrid) * xshift;
    zpos = (-TERRAIN_GRID_SIZE_QUARTER + zgrid) * zshift;
    xshift = zshift = size / 2;
    glTexCoord2f(xpos + xshift, zpos + zshift);
    glVertex3f(xpos + xshift, TERRAIN_WATER_LEVEL, zpos + zshift);
    glTexCoord2f(xpos - xshift, zpos + zshift);
    glVertex3f(xpos - xshift, TERRAIN_WATER_LEVEL, zpos + zshift);
    glTexCoord2f(xpos - xshift, zpos - zshift);
    glVertex3f(xpos - xshift, TERRAIN_WATER_LEVEL, zpos - zshift);
    glTexCoord2f(xpos + xshift, zpos - zshift);
    glVertex3f(xpos + xshift, TERRAIN_WATER_LEVEL, zpos - zshift);
    if (xgrid >= TERRAIN_GRID_SIZE_HALF - 1) {
      zgrid++;
      xgrid = -1;
    }
  }
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_CULL_FACE);
}


void renderCloud(struct v3f camerapos, struct v3f camerarot, GLubyte alpha, float height, float scale, int t_size)
{
  int i, size;
  float rot, x, z;
  int temp;

  size = TERRAIN_GRID_SIZE * t_size * 0.5f;
  temp = fabs(height - camerapos.y) * 0.1f;
  temp = temp > alpha ? alpha : temp;
  glMateriali(GL_FRONT, GL_SHININESS, 31);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glPushMatrix();
  glTranslatef(camerapos.x, 0.0f, camerapos.z);
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glScalef(scale, scale, scale);
  glTranslatef(camerapos.x, camerapos.z, 0.0f);
  glColor4ub(128, 128, 128, (GLubyte) temp);
  glNormal3i(0, -1, 0);
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(0.0f, height, 0.0f);
  glColor4ub(128, 128, 128, 0);
  for (i = 0; i <= 360; i += 30) {
    rot = i / PIx180;
    x = -size * sinf(rot);
    z = size * cosf(rot);
    glTexCoord2f(x, z);
    glVertex3f(x, height, z);
  }
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_CULL_FACE);
}


void renderSun(struct v3f camerapos, GLint sunpos[4], float size)
{
  int i;
  float r, x, z;

  glPushMatrix();
  glTranslatef(camerapos.x + sunpos[0], camerapos.y + sunpos[1], camerapos.z + sunpos[2]);
  r = atan2(sunpos[0], sunpos[1]) * -180 / PI;
  glRotatef(r, 0.0f, 0.0f, 1.0f);
  glColor4ub(235, 215, 30, 10 + (GLubyte) fabs(sunpos[1] * 0.244f));
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glColor4ub(235, 200, 60, 3);
  for (i = 0; i <= 360; i += 15) {
    r = i / PIx180;
    x = -size * sinf(r);
    z = size * cosf(r);
    glVertex3f(x, 0.0f, z);
  }
  glEnd();
  glPopMatrix();
}


void renderMoon(struct v3f camerapos, GLint pos[4], float size)
{
  int i;
  float r, x, z;

  glPushMatrix();
  glTranslatef(camerapos.x + pos[0], camerapos.y + pos[1], camerapos.z + pos[2]);
  r = atan2(pos[0], pos[1]) * -180 / PI;
  glRotatef(r, 0.0f, 0.0f, 1.0f);
  glColor3ub(210, 210, 212);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(size*0.5f, 0.0f, size*0.4f);
  glColor4ub(203, 203, 206, 200);
  for (i = 0; i <= 360; i += 15) {
    r = i / PIx180;
    x = -size * sinf(r);
    z = size * cosf(r);
    glVertex3f(x, 0.0f, z);
  }
  glEnd();
  glPopMatrix();
}


void renderExhaust(struct v3f pos, struct v3f rot, float scale, float size)
{
  int i;
  float r, x, y;

  glMateriali(GL_FRONT, GL_SHININESS, 110);
  /* glMateriali(GL_FRONT, GL_EMISSION, 78); */
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);
  glScalef(scale, scale, scale);
  glRotatef(-rot.y, 0.0f, 1.0f, 0.0f);
  glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
  glRotatef(rot.z, 0.0f, 0.0f, 1.0f);
  glBegin(GL_TRIANGLE_FAN);
  glColor4ub(195, 110, 30, 185);
  glVertex3f(1.4286f, 1.4286f, -powf(size, 1.5f) * 5.0f - 14.857f * size);
  glColor4ub(255, 225, 90, 30);
  for (i = 0; i <= 360; i += 60) {
    r = i / PIx180;
    x = -size * sinf(r);
    y = size * cosf(r);
    glVertex3f(x + 1.4286f, y + 1.428f, -4.857f);
  }
  glEnd();
  glBegin(GL_POLYGON);
  glColor4ub(255, 225, 90, 70);
  for (i = 360; i >= 0; i -= 60) {
    r = i / PIx180;
    x = -size * sinf(r);
    y = size * cosf(r);
    glVertex3f(x + 1.428f, y + 1.428f, -4.857f);
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  glColor4ub(195, 110, 30, 185);
  glVertex3f(-1.4286f, 1.4286f, -powf(size, 1.5f) * 5.0f - 14.857f * size);
  glColor4ub(255, 225, 90, 30);
  for (i = 0; i <= 360; i += 60) {
    r = i / PIx180;
    x = -size * sinf(r);
    y = size * cosf(r);
    glVertex3f(x - 1.4286f, y + 1.428f, -4.857f);
  }
  glEnd();
  glBegin(GL_POLYGON);
  glColor4ub(255, 225, 90, 70);
  for (i = 360; i >= 0; i -= 60) {
    r = i / PIx180;
    x = -size * sinf(r);
    y = size * cosf(r);
    glVertex3f(x - 1.428f, y + 1.428f, -4.857f);
  }
  glEnd();
  glPopMatrix();
  /* glMateriali(GL_FRONT, GL_EMISSION, 0); */
}


void renderNumber(int num, struct aiScene *textquads, struct v2f pos)
{
  int i = 0, j = num;

  num = fabs(num);
  drawModel((const struct aiScene *) &textquads[num % 10], mv3f(pos.x, pos.y, 0), mv3f(0, 0, 0), 250, 255);
  while (num > 9) {
    num -= num % 10;
    num /= 10;
    drawModel((const struct aiScene *) &textquads[num % 10], mv3f(pos.x - 50 * ++i, pos.y, 0), mv3f(0, 0, 0), 250, 255);
  }
  if (j < 0)
    drawModel((const struct aiScene *) &textquads[10], mv3f(pos.x - 50 * ++i, pos.y, 0), mv3f(0, 0, 0), 250, 255);
}


void renderFX(void)
{
  int xgrid, zgrid;

  glColor3ub(255, 255, 255);
  for (xgrid = 0, zgrid = 0; xgrid < 10 && zgrid < 10; xgrid++) {
    glBegin(GL_POINTS);
    glVertex3f(xgrid * 50, 5000, zgrid * 50);
    glEnd();
    if (xgrid >= 9) {
      zgrid++;
      xgrid = -1;
    }
  }
}


void renderAircraft(struct aiScene *scene, GLuint *textures, struct v3f camerapos,
                    struct airunit *units)
{
  int i, texture, model;

  for (i = 0; i < 15; i++) {
    switch (units[i].type) {
    case UNIT_AIR_FIGHTER1:
      texture = TEX_AIR_FIGHTER1;
      model = MODEL_AIR_FIGHTER1;
      break;
    default:
      texture = TEX_AIR_FIGHTER1;
      model = MODEL_AIR_FIGHTER1;
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, textures[texture]);
    drawModel((const struct aiScene *) &scene[model], units[i].pos, units[i].rot, 0.7f, 255);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    renderExhaust(units[i].pos, units[i].rot, 0.7f, units[i].thrust * 1.5f);
  }
}


void sceneQuad(void)
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glColor3ub(255, 255, 255);
  glBegin(GL_QUADS);
  glMultiTexCoord2i(4, 0, 0);
  glVertex3i(0, 0, 0);
  glMultiTexCoord2i(4, 1, 0);
  glVertex3i(RESX, 0, 0);
  glMultiTexCoord2i(4, 1, 1);
  glVertex3i(RESX, RESY, 0);
  glMultiTexCoord2i(4, 0, 1);
  glVertex3i(0, RESY, 0);
  glEnd();
}


void render(GLFWwindow *window, struct aiScene *scene, struct aiScene *textquads, GLuint *textures,
            GLuint *shaders, struct v3f camerapos, struct v3f camerarot, struct v2f *sector,
            int *t_size, float *fps, struct airunit *airunits)
{
  GLfloat color[4], temp;
  GLint lpos[4], mpos[4];
  static double time = 0;
  static float fps2 = 0;
  static char swapb = 1;

  *fps = 1 / (glfwGetTime() - time);
  fps2 += (*fps - fps2) * 0.05f;
  time = glfwGetTime();
  glEnable(GL_LIGHT0); /* sun */
  glEnable(GL_LIGHT1); /* moon */
  temp = time * 0.003f;
  lpos[0] = -1000 * sinf(temp);
  lpos[1] = 1000 * cosf(temp);
  lpos[2] = 0;
  lpos[3] = 0;
  glLightiv(GL_LIGHT0, GL_POSITION, lpos);
  mpos[0] = 400;
  mpos[1] = 800;
  mpos[2] = 300;
  mpos[3] = 0;
  glLightiv(GL_LIGHT1, GL_POSITION, mpos);
  if (lpos[1] > 300)
    temp = 0.3f * (lpos[1] - 300) / 700.0f;
  else
    temp = 0.0f;
  color[0] = temp;
  color[1] = temp;
  color[2] = temp;
  color[3] = 1.0f;
  glLightfv(GL_LIGHT0, GL_SPECULAR, color);
  color[0] = 0.1f;
  color[1] = 0.1f;
  color[2] = 0.12f;
  glLightfv(GL_LIGHT1, GL_SPECULAR, color);
  if (lpos[1] < -300)
    temp = 0.35f * (1 - (-300 - lpos[1]) / 700.0f);
  else
    temp = 0.35f;
  color[0] = temp;
  color[1] = temp;
  color[2] = temp;
  glLightfv(GL_LIGHT0, GL_AMBIENT, color);
  color[0] = 0.03f;
  color[1] = 0.03f;
  color[2] = 0.04f;
  glLightfv(GL_LIGHT1, GL_AMBIENT, color);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
  if (lpos[1] > -600)
    temp = (lpos[1] + 600) / 1600.0f;
  else
    temp = 0.0f;
  color[0] = temp;
  color[1] = lpos[1] > -400 ? (lpos[1] + 400) / 1400.0f : 0;
  color[2] = lpos[1] > -200 ? (lpos[1] + 200) / 1200.0f : 0;
  glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
  color[0] = 0.2f;
  color[1] = 0.2f;
  color[2] = 0.22f;
  glLightfv(GL_LIGHT1, GL_DIFFUSE, color);
  color[0] = 0.8f;
  color[1] = 0.8f;
  color[2] = 0.8f;
  glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
  glMaterialfv(GL_FRONT, GL_AMBIENT, color);
  glMaterialfv(GL_FRONT, GL_SPECULAR, color);
  /* glMaterialfv(GL_FRONT, GL_EMISSION, color); */
  color[0] = 0.4588235294117647f * temp;
  color[0] += lpos[0] > 800 ? (lpos[0] - 800) * 0.005f * 0.1f : 0;
  color[1] = 0.5176470588235295f * (lpos[1] > -400 ? (lpos[1] + 400) / 1400.0f : 0);
  color[2] = 0.8431372549019608f * (lpos[1] > -200 ? 0.05f + (lpos[1] + 200) / 1200.0f : 0.05f);
  updateFogAndFrustum(color, camerapos, *t_size);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);
  renderSky(camerapos, camerarot, color, *t_size);
  renderSun(camerapos, lpos, 160);
  renderMoon(camerapos, mpos, 60);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_FOG);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glBindTexture(GL_TEXTURE_2D, textures[TEX_TERRAIN]);
  drawTerrain(camerapos, camerarot, sector, t_size, &swapb);
  if (camerapos.y < TERRAIN_SCALE_HEIGHT)
    renderGroundScenery(scene, textures, camerapos, camerarot, *t_size, fps2);
  if (swapb) {
    glBindTexture(GL_TEXTURE_2D, textures[TEX_CLOUD]);
    renderWater(camerapos, camerarot, color, *t_size);
    renderAircraft(scene, textures, camerapos, airunits);
    /* glEnable(GL_POINT_SPRITE); */
    /* glEnable(GL_PROGRAM_POINT_SIZE); */
    /* glUseProgramARB(shaders[2]); */
    /* glUniform1iARB(glGetUniformLocationARB(shaders[2], "texture"), 1); */
    /* glUniform1fARB(glGetUniformLocationARB(shaders[2], "size"), 10.0f); */
    /* glUniform2fARB(glGetUniformLocationARB(shaders[2], "screensize"), 1366, 768); */
    /* renderFX(); */
    /* glUseProgramARB(0); */
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_CLOUD]);
    if (camerapos.y < TERRAIN_SCALE_HEIGHT) {
      renderCloud(camerapos, camerarot, 180, TERRAIN_SCALE_HEIGHT, 0.00005f, *t_size);
      renderCloud(camerapos, camerarot, 100, LOWER_CLOUD_HEIGHT, 0.00001f, *t_size);
      renderGrass(textures, camerapos, camerarot, *t_size, fps2);
    }
    else {
      renderCloud(camerapos, camerarot, 65, LOWER_CLOUD_HEIGHT, 0.00001f, *t_size);
      renderCloud(camerapos, camerarot, 120, TERRAIN_SCALE_HEIGHT, 0.00005f, *t_size);
    }
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, RESX, 0, RESY, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, textures[TEX_FONT]);
    renderNumber(camerapos.x, textquads, mv2f(250, 120));
    renderNumber(camerapos.z, textquads, mv2f(250, 70));
    renderNumber(fps2, textquads, mv2f(250, 20));
    renderNumber(airunits[0].height, textquads, mv2f(RESX - 100, 120));
    renderNumber(airunits[0].speed * 10.0f, textquads, mv2f(RESX - 100, 70));
    renderNumber(airunits[0].thrust * 100, textquads, mv2f(RESX - 100, 20));
    if (USE_SHADERS) {
      glReadBuffer(GL_BACK);
      glBindTexture(GL_TEXTURE_2D, textures[TEX_RENDER]);
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, RESX, RESY, 0);
      glUseProgramARB(shaders[0]);
      glUniform1iARB(glGetUniformLocationARB(shaders[0], "scene"), 4);
      glUniform1fARB(glGetUniformLocationARB(shaders[0], "gamma"), 0.6f);
      glUniform1fARB(glGetUniformLocationARB(shaders[0], "numcolors"), 64.0f);
      glUniform4fvARB(glGetUniformLocationARB(shaders[0], "clear"), 0, color);
      sceneQuad();
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, RESX, RESY, 0);
      glUseProgramARB(shaders[1]);
      glUniform1iARB(glGetUniformLocationARB(shaders[1], "scene"), 4);
      glUniform2fARB(glGetUniformLocationARB(shaders[1], "steps"), RESX, RESY);
      glUniform4fvARB(glGetUniformLocationARB(shaders[1], "clear"), 0, color);
      sceneQuad();
    }
  }
  glUseProgramARB(0);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  if (swapb)
    glfwSwapBuffers(window);
  swapb = 1;
  glfwPollEvents();
}
