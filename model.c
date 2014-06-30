#include <stdio.h>
#include <stdlib.h>
#include "common.h"


void loadFromOBJFile(char *name, struct model *model)
{
  FILE *fp;
  long int fpos;
  char temp, i;
  GLuint indice, tex_indice;
  GLint num_verts = 0;
  GLint num_norms = 0;
  GLint num_tex_coords = 0;

  model->num_indices = 10000;
  model->verts = (void *) malloc(sizeof(GLfloat) * model->num_indices);
  model->norms = (void *) malloc(sizeof(GLfloat) * model->num_indices);
  model->indices = (GLuint *) malloc(sizeof(GLuint) * model->num_indices);
  model->tex_indices = (GLuint *) malloc(sizeof(GLuint) * model->num_indices);
  model->tex_coords = (void *) malloc(sizeof(GLfloat) * model->num_indices);
  model->num_indices = 0;
  model->type = GL_TRIANGLES;
  if ((fp = fopen(name, "r"))) {
    while (fscanf(fp, "%c", &temp) != EOF) {
      if (temp == '\n') {
        fscanf(fp, "%c", &temp);
        if (temp == 'v') {
          fpos = ftell(fp);
          fscanf(fp, "%c", &temp);
          if (temp != 't' && temp != 'n') {
            fpos = fseek(fp, fpos, SEEK_SET);
            fscanf(fp, "%f %f %f", &model->verts[num_verts][0], &model->verts[num_verts][1], &model->verts[num_verts][2]);
            num_verts++;
          }
          if (temp == 't') {
            fscanf(fp, "%f %f", &model->tex_coords[num_tex_coords][0], &model->tex_coords[num_tex_coords][1]);
            num_tex_coords++;
          }
          if (temp == 'n') {
            fscanf(fp, "%f %f %f", &model->norms[num_norms][0], &model->norms[num_norms][1], &model->norms[num_norms][2]);
            num_norms++;
          }
        }
        if (temp == 'f') {
          for (i = 0; i < 3; i++) {
            fscanf(fp, "%d/%d/%d", &indice, &tex_indice, &indice);
            model->indices[model->num_indices] = indice - 1;
            model->tex_indices[model->num_indices] = tex_indice - 1;
            model->num_indices++;
          }
        }
      }
    }
    model->verts = (void *) realloc(model->verts, sizeof(GLfloat) * model->num_indices);
    model->norms = (void *) realloc(model->norms, sizeof(GLfloat) * model->num_indices);
    model->indices = (GLuint *) realloc(model->indices, sizeof(GLuint) * model->num_indices);
    model->tex_indices = (GLuint *) realloc(model->tex_indices, sizeof(GLuint) * model->num_indices);
    model->tex_coords = (void *) realloc(model->tex_coords, sizeof(GLfloat) * model->num_indices);
    fclose(fp);
  }
}


void drawModel(struct model model, struct v3f pos, struct v3f rot, GLfloat size, GLuint alpha)
{
  GLuint k;
  GLint j, h;

  glPushMatrix();
  glTranslatef((GLfloat) (-pos.x), (GLfloat) (pos.y), (GLfloat) (-pos.z));
  glRotatef((GLfloat) (rot.y), 0.0f, 1.0f, 0.0f);
  glRotatef((GLfloat) (rot.x), 1.0f, 0.0f, 0.0f);
  glRotatef((GLfloat) (rot.z), 0.0f, 0.0f, 1.0f);
  glScalef(size, size, size);
  glColor4ub(125, 125, 125, alpha);
  glTexCoordPointer(2, GL_FLOAT, 0, model.tex_coords);
  glVertexPointer(3, GL_FLOAT, 0, model.verts);
  glNormalPointer(GL_FLOAT, 0, model.norms);
  glBegin(model.type);
  for (k = 0; k < model.num_indices; k++) {
    j = model.indices[k];
    h = model.tex_indices[k];
    glTexCoord2f(model.tex_coords[h][0], model.tex_coords[h][1]);
    glArrayElement(j);
  }
  glEnd();
  glPopMatrix();
}