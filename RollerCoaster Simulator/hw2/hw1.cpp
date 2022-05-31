/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: ninghaoh@usc.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "imageIO.h"
#include <math.h>
#include "glutHeader.h"
#include "openGLMatrix.h"
#include <vector>
#include "basicPipelineProgram.h"
#include "texturePipelineProgram.h"
#include <fstream>
#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif
using namespace std;
ImageIO * heightmapImage;
int photonumber = 0;
bool startRecord = false;
struct Point 
{
  double x;
  double y;
  double z;
};

struct Point_floating
{
  float x;
  float y;
  float z;
};
int mousePos[2]; // x,y coordinate of the mouse position
vector<Point_floating> startLocation;
Point_floating tangent;
Point_floating normal;
Point_floating binormal;
Point_floating tangent1;
Point_floating normal1;
Point_floating binormal1;
Point_floating binormal_prev;
Point_floating binormal_prev_r;
vector<Point_floating> tangent_vector;
vector<Point_floating> normal_vector;
vector<Point_floating> binormal_vector_prev;
vector<Point_floating> binormal_vector;
int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not
double startLocationx = 0;
double startLocationy = 0;
double startLocationz = 0;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;
float s = 0.5;
float constant_M[4][4] = {
    {-s, 2-s, s-2, s},
    {2*s, s-3, 3-2*s, -s},
    {-s, 0, s, 0},
    {0, 1, 0, 0}
  };

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";
vector<float> line_buffer;
vector<float> tan_buffer;
vector<float> line_buffer_right;
vector<float> line_color;
vector<float> tan_color;
vector<float> line_color_left;
vector<float> line_color_right;
GLuint texHandle;
vector<Point_floating> pointBuffer;
vector<Point_floating> pointBuffer_right;
vector<Point_floating> tangent_vector_r;
vector<Point_floating> normal_vector_r;
vector<Point_floating> binormal_vector_r;


vector<float> railBuffer;
vector<float> railBufferNormal;
vector<float> railBufferColor;
GLuint lineBuffer, lineColor;
GLuint skybuffer, skyColor;
GLuint lineBuffer_left;
GLuint vbo_tex,vao_tex;
GLuint vbo_sky,vao_sky;
vector<float> tex_buffer;
vector<float> tex_buffer_sky;
vector<float> tex_buffer_uv;
vector<float> tex_buffer_uv_sky;
//GLuint texBuffer;


GLuint vao_line_left, vao_line_right;
GLuint vao_point, vao_line;
GLuint loc, colorloc;
GLuint program;
GLuint tex_program;
float frame_temp = 1;
int frame2 = 0;
OpenGLMatrix matrix;
BasicPipelineProgram pipelineProgram;
texturePipelineProgram texPipelineProgram;
GLint h_modelViewMatrix, h_projectionMatrix;
GLint t_modelViewMatrix, t_projectionMatrix;

void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}



void matrixMul(float p[][4], float constant[][4], float result[1][4]){
  for(int i = 0; i < 1;i++){
    for(int j = 0; j<4;j++){
      result[i][j] = 0;
      for(int k = 0; k < 4;k++){
        result[i][j] += p[i][k] * constant[k][j];
      }
      // cout << result[i][j] << "\t";
    }
    // cout << endl;
  }
};
void matrixMul2(float p[1][4], float coordinate[4][3], float result[1][3]){
  for(int i = 0; i < 1;i++){
    for(int j = 0; j < 3;j++){
      result[i][j] = 0;
      for(int k = 0; k < 4;k++){
        result[i][j] += p[i][k] * coordinate[k][j];
      }
      // cout << result[i][j] << "\t";
    }
    // cout << endl;
  }
};


Point_floating calculate_normal(Point_floating points){
  Point_floating point;
  point.x = -((points.z*-1.0f));
  point.y = 0;
  point.z = points.x*-1.0f;
  float magnitude = std::sqrt (std::pow (point.x, 2) + std::pow (point.y, 2) + std::pow (point.z, 2));
  if(magnitude!=0){
    point.x = point.x/magnitude;
    point.y = point.y/magnitude;
    point.z = point.z/magnitude;
  }
  return point;
}

Point_floating calculate_normal2(Point_floating tangent, Point_floating binormal){
  Point_floating point;

  point.x = tangent.y * binormal.z - tangent.z* binormal.y;;
  point.y = tangent.z * binormal.x - tangent.x* binormal.z;
  point.z = tangent.x * binormal.y - tangent.y* binormal.x;
  float magnitude = std::sqrt (std::pow (point.x, 2) + std::pow (point.y, 2) + std::pow (point.z, 2));
  if(magnitude!=0){
    point.x = point.x/magnitude;
    point.y = point.y/magnitude;
    point.z = point.z/magnitude;
  }
  return point;
}

Point_floating calculate_binormal(Point_floating Tan, Point_floating Nor){
  // B = T x N
  Point_floating point;
  point.x = Tan.y * Nor.z - Tan.z* Nor.y;
  point.y = Tan.z * Nor.x - Tan.x* Nor.z;
  point.z = Tan.x * Nor.y - Tan.y* Nor.x;
  float magnitude = std::sqrt (std::pow (point.x, 2) + std::pow (point.y, 2) + std::pow (point.z, 2));
  point.x = point.x/magnitude;
  point.y = point.y/magnitude;
  point.z = point.z/magnitude;
  return point;
}

Point_floating calculate_binormal2(Point_floating Tan, Point_floating Nor){
  // B = T x N
  Point_floating point;
  point.x = Tan.y * Nor.z - Tan.z* Nor.y;
  point.y = Tan.z * Nor.x - Tan.x* Nor.z;
  point.z = Tan.x * Nor.y - Tan.y* Nor.x;
  return point;
}

void setTextureUnit(GLint unit)
{
  glActiveTexture(unit); // select texture unit affected by subsequent texture calls
// get a handle to the “textureImage” shader variable
  GLint h_textureImage = glGetUniformLocation(texPipelineProgram.GetProgramHandle(), "textureImage");
// deem the shader variable “textureImage” to read from texture unit “unit”
  glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}
void Phong(){

  float view[16];
  matrix.GetMatrix(view); // read the view matrix
// get a handle to the program
  GLuint program = pipelineProgram.GetProgramHandle();
// get a handle to the viewLightDirection shader variable
  GLint h_viewLightDirection = glGetUniformLocation(program, "viewLightDirection"); 
  float lightDirection[3] = { 0, 1, 0 }; 
  float viewLightDirection[3];
  glUniform3fv(h_viewLightDirection, 1, viewLightDirection);
  Point_floating pointing_point;
  pointing_point.x = view[0] * lightDirection[0] + view[4] * lightDirection[1] + view[8]  * lightDirection[2];
  pointing_point.y = view[1] * lightDirection[0] + view[5] * lightDirection[1] + view[9]  * lightDirection[2];
  pointing_point.z = view[2] * lightDirection[0] + view[6] * lightDirection[1] + view[10] * lightDirection[2];

  viewLightDirection[0] = pointing_point.x;
  viewLightDirection[1] = pointing_point.y;
  viewLightDirection[2] = pointing_point.z;
  glUniform3fv(h_viewLightDirection, 1, viewLightDirection);
    float ka[4] = {  0.4f,  0.4f,  0.4f, 1.0f };
    float La[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float Ld[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float kd[4] = {  0.4f,  0.4f,  0.4f, 1.0f};
    float Ls[4] = { 0.9f, 0.9f, 0.9f, 0.9f };
    float ks[4] = {  0.4f,  0.4f, 0.4f , 1.0f };
    h_viewLightDirection = glGetUniformLocation(program, "La");
    glUniform4fv(h_viewLightDirection, 1, La);

    h_viewLightDirection = glGetUniformLocation(program, "ka");
    glUniform4fv(h_viewLightDirection, 1, ka);

    h_viewLightDirection = glGetUniformLocation(program, "Ld");
    glUniform4fv(h_viewLightDirection, 1, Ld);

    h_viewLightDirection = glGetUniformLocation(program, "kd");
    glUniform4fv(h_viewLightDirection, 1, kd);

    h_viewLightDirection = glGetUniformLocation(program, "Ls");
    glUniform4fv(h_viewLightDirection, 1, Ls);

    h_viewLightDirection = glGetUniformLocation(program, "ks");
    glUniform4fv(h_viewLightDirection, 1, ks);

    h_viewLightDirection = glGetUniformLocation(program, "alpha");
    glUniform1f(h_viewLightDirection, 1.0f);
    h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");

    // Set up model, perspective and normal matrix
    float m[16]; 
    matrix.GetMatrix(m);
    glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
    h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    float p[16]; 
    matrix.GetMatrix(p);
    glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);



  GLint h_normalMatrix = glGetUniformLocation(program, "normalMatrix"); 
  float n[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetNormalMatrix(n); 
  GLboolean isRowMajor = GL_FALSE;
  glUniformMatrix4fv(h_normalMatrix, 1, isRowMajor, n);
}
void displayFunc()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  //matrix.LookAt(-10.0f,20.0f,10.0f, 10.0f, 0.0f, -10.0f, 0.0f, 0.0f, -1.0f);
  float height = 0.8;
  matrix.LookAt(pointBuffer[frame2].x + (normal_vector[frame2].x * height), 
                pointBuffer[frame2].y + (normal_vector[frame2].y * height), 
                pointBuffer[frame2].z + (normal_vector[frame2].z * height), 
                pointBuffer[frame2].x + tangent_vector[frame2].x,
                pointBuffer[frame2].y + tangent_vector[frame2].y, 
                pointBuffer[frame2].z + tangent_vector[frame2].z, 
                normal_vector[frame2].x, 
                normal_vector[frame2].y, 
                normal_vector[frame2].z);
  Phong();
  frame2 = frame2 + 1;
  if(frame2 >= pointBuffer.size()){
    frame2 = 0;
  }
  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);


  texPipelineProgram.Bind();
  texPipelineProgram.SetModelViewMatrix(m);
  texPipelineProgram.SetProjectionMatrix(p);
  setTextureUnit(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texHandle);
  glBindVertexArray(vao_tex);
  glDrawArrays(GL_TRIANGLES, 0, tex_buffer.size()/3);

  // glBindVertexArray(0); 
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);

  glBindVertexArray(vao_line_left); 
  // draw 
  GLsizei count1 = railBuffer.size()/3;
  glDrawArrays(GL_TRIANGLES, 0, count1);

  glBindVertexArray(0); 

  glutSwapBuffers();
}

void idleFunc()
{

  // do some stuff... 
    // for example, here, you can save the screenshots to disk (to make the animation)
    // make 300 screenshots and name it from 0.jpg to 299 jpg
   if(startRecord == true){
     if(photonumber <= 300){
      unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
      glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

      ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);
      string s = to_string(photonumber-1);
      string news1 = s+".jpg";
      char const *num_char = news1.c_str();
      if (screenshotImg.save(num_char, ImageIO::FORMAT_JPEG) == ImageIO::OK)
        cout << "File " << photonumber << " saved successfully." << endl;
      else cout << "Failed to save file " << photonumber << '.' << endl;

      delete [] screenshotData;
      photonumber++;
    }
   } 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 100.0f);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      frame2 = frame2 + 1;
      cout << "pos: "<< pointBuffer[frame2].x << " " << pointBuffer[frame2].y << " "  << pointBuffer[frame2].z <<endl;
    break;

    case 'x':
      saveScreenshot("screenshot.jpg");
    break;
    case 'k':
      startRecord = true;
    break;

    case 'c':
      startRecord = false;
    break;
  }
}
// represents one control point along the spline 
// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline 
{
  int numControlPoints;
  Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;
int iLength;
int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) 
  {
    printf ("can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);
  printf("%d",numSplines);
  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) 
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) 
    {
      printf ("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &splines[j].points[i].x, 
	   &splines[j].points[i].y, 
	   &splines[j].points[i].z) != EOF) 
    {
      i++;
    }
    
  }

  free(cName);

  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) 
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }
  
  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) 
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) 
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}
void create_rail( vector<Point_floating> Tan , vector<Point_floating> Nor, vector<Point_floating> Pos, vector<Point_floating> Bin){
  for(int i = 0; i < Pos.size()-1; i = i + 1){
    // cout << i << endl;
    Point_floating v0;
    float alpha = 0.1;
    Point_floating temp;
    temp.x = alpha * (-Nor[i].x+ Bin[i].x);
    temp.y = alpha * (-Nor[i].y+ Bin[i].y);
    temp.z = alpha * (-Nor[i].z+ Bin[i].z);
    v0.x = Pos[i].x + temp.x;
    v0.y = Pos[i].y + temp.y;
    v0.z = Pos[i].z + temp.z;
    float rail_current0[3] = {v0.x,v0.y,v0.z};
    Point_floating v1;
    temp.x = alpha * (Nor[i].x+ Bin[i].x);
    temp.y = alpha * (Nor[i].y+ Bin[i].y);
    temp.z = alpha * (Nor[i].z+ Bin[i].z);
    v1.x = Pos[i].x + temp.x;
    v1.y = Pos[i].y + temp.y;
    v1.z = Pos[i].z + temp.z;
    float rail_current1[3] = {v1.x,v1.y,v1.z};
    Point_floating v2;
    temp.x = alpha * (Nor[i].x - Bin[i].x);
    temp.y = alpha * (Nor[i].y - Bin[i].y);
    temp.z = alpha * (Nor[i].z - Bin[i].z);
    v2.x = Pos[i].x + temp.x;
    v2.y = Pos[i].y + temp.y;
    v2.z = Pos[i].z + temp.z;
    float rail_current2[3] = {v2.x,v2.y,v2.z};
    Point_floating v3;
    temp.x = alpha * (-Nor[i].x - Bin[i].x);
    temp.y = alpha * (-Nor[i].y - Bin[i].y);
    temp.z = alpha * (-Nor[i].z - Bin[i].z);
    v3.x = Pos[i].x + temp.x;
    v3.y = Pos[i].y + temp.y;
    v3.z = Pos[i].z + temp.z;
    float rail_current3[3] = {v3.x,v3.y,v3.z};

    // Point_floating temp;
    int i0 = i+1;
    Point_floating v4;
    temp.x = alpha * (-Nor[i0].x+ Bin[i0].x);
    temp.y = alpha * (-Nor[i0].y+ Bin[i0].y);
    temp.z = alpha * (-Nor[i0].z+ Bin[i0].z);
    v4.x = Pos[i0].x + temp.x;
    v4.y = Pos[i0].y + temp.y;
    v4.z = Pos[i0].z + temp.z;
    float rail_current4[3] = {v4.x,v4.y,v4.z};



    Point_floating v5;
    temp.x = alpha * (Nor[i0].x+ Bin[i0].x);
    temp.y = alpha * (Nor[i0].y+ Bin[i0].y);
    temp.z = alpha * (Nor[i0].z+ Bin[i0].z);
    v5.x = Pos[i0].x + temp.x;
    v5.y = Pos[i0].y + temp.y;
    v5.z = Pos[i0].z + temp.z;
    float rail_current5[3] = {v5.x,v5.y,v5.z};



    Point_floating v6;
    temp.x = alpha * (Nor[i0].x - Bin[i0].x);
    temp.y = alpha * (Nor[i0].y - Bin[i0].y);
    temp.z = alpha * (Nor[i0].z - Bin[i0].z);
    v6.x = Pos[i0].x + temp.x;
    v6.y = Pos[i0].y + temp.y;
    v6.z = Pos[i0].z + temp.z;
    float rail_current6[3] = {v6.x,v6.y,v6.z};


    Point_floating v7;
    temp.x = alpha * (-Nor[i0].x - Bin[i0].x);
    temp.y = alpha * (-Nor[i0].y - Bin[i0].y);
    temp.z = alpha * (-Nor[i0].z - Bin[i0].z);
    v7.x = Pos[i0].x + temp.x;
    v7.y = Pos[i0].y + temp.y;
    v7.z = Pos[i0].z + temp.z;
    float rail_current7[3] = {v7.x,v7.y,v7.z};


    // right face tube right normal is +b
    railBuffer.insert(railBuffer.end(), rail_current1, rail_current1 + 3);
    railBuffer.insert(railBuffer.end(), rail_current4, rail_current4 + 3);
    railBuffer.insert(railBuffer.end(), rail_current5, rail_current5 + 3);

    railBuffer.insert(railBuffer.end(), rail_current1, rail_current1 + 3);
    railBuffer.insert(railBuffer.end(), rail_current4, rail_current4 + 3);
    railBuffer.insert(railBuffer.end(), rail_current0, rail_current0 + 3);

    float rail_current_color3[3] = {Bin[i].x ,Bin[i].y ,Bin[i].z};
    float rail_current_color7[3] = {Bin[i0].x ,Bin[i0].y ,Bin[i0].z};
    railBufferColor.insert(railBufferColor.end(), rail_current_color3, rail_current_color3 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color7, rail_current_color7 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color7, rail_current_color7 + 3);

    railBufferColor.insert(railBufferColor.end(), rail_current_color3, rail_current_color3 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color7, rail_current_color7 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color3, rail_current_color3 + 3);

    // top face tube top normal is +n 
    railBuffer.insert(railBuffer.end(), rail_current1, rail_current1 + 3);
    railBuffer.insert(railBuffer.end(), rail_current2, rail_current2 + 3);
    railBuffer.insert(railBuffer.end(), rail_current5, rail_current5 + 3);

    railBuffer.insert(railBuffer.end(), rail_current2, rail_current2 + 3);
    railBuffer.insert(railBuffer.end(), rail_current5, rail_current5 + 3);
    railBuffer.insert(railBuffer.end(), rail_current6, rail_current6 + 3);

    float rail_current_color1[3] = {Nor[i].x ,Nor[i].y ,Nor[i].z};
    float rail_current_color2[3] = {Nor[i0].x ,Nor[i0].y ,Nor[i0].z};
    //float rail_current_color1[4] = {255.0f ,255.0f ,255.0f ,1.0f};
    railBufferColor.insert(railBufferColor.end(), rail_current_color1, rail_current_color1 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color1, rail_current_color1 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color2, rail_current_color2 + 3);

    railBufferColor.insert(railBufferColor.end(), rail_current_color1, rail_current_color1 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color2, rail_current_color2 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color2, rail_current_color2 + 3);


    //left face tube left normal is -b
    railBuffer.insert(railBuffer.end(), rail_current3, rail_current3 + 3);
    railBuffer.insert(railBuffer.end(), rail_current2, rail_current2 + 3);
    railBuffer.insert(railBuffer.end(), rail_current6, rail_current6 + 3);

    railBuffer.insert(railBuffer.end(), rail_current6, rail_current6 + 3);
    railBuffer.insert(railBuffer.end(), rail_current7, rail_current7 + 3);
    railBuffer.insert(railBuffer.end(), rail_current3, rail_current3 + 3);

    float rail_current_color5[3] = {Bin[i].x * -1.0f,Bin[i].y * -1.0f,Bin[i].z * -1.0f};
    railBufferColor.insert(railBufferColor.end(), rail_current_color5, rail_current_color5 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color5, rail_current_color5 + 3);
    float rail_current_color6[3] = {Bin[i0].x * -1.0f,Bin[i0].y * -1.0f,Bin[i0].z * -1.0f};
    railBufferColor.insert(railBufferColor.end(), rail_current_color6, rail_current_color6 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color6, rail_current_color6 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color6, rail_current_color6 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color5, rail_current_color5 + 3);



    //bottom face tube bottom normal is -n
    railBuffer.insert(railBuffer.end(), rail_current3, rail_current3 + 3);
    railBuffer.insert(railBuffer.end(), rail_current0, rail_current0 + 3);
    railBuffer.insert(railBuffer.end(), rail_current4, rail_current4 + 3);

    railBuffer.insert(railBuffer.end(), rail_current4, rail_current4 + 3);
    railBuffer.insert(railBuffer.end(), rail_current7, rail_current7 + 3);
    railBuffer.insert(railBuffer.end(), rail_current3, rail_current3 + 3);    


    float rail_current_color4[3] = {Nor[i].x * -1.0f,Nor[i].y * -1.0f,Nor[i].z * -1.0f};
    float rail_current_color8[3] = {Nor[i0].x * -1.0f,Nor[i0].y * -1.0f,Nor[i0].z * -1.0f};
    // cout << rail_current_color8[0] << " " << rail_current_color8[1] << " " << rail_current_color8[2] << endl;
    railBufferColor.insert(railBufferColor.end(), rail_current_color4, rail_current_color4 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color4, rail_current_color4 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color8, rail_current_color8 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color8, rail_current_color8 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color8, rail_current_color8 + 3);
    railBufferColor.insert(railBufferColor.end(), rail_current_color4, rail_current_color4 + 3);


  }
}
void populate_texture_vbo(){

    tex_buffer.push_back(1200.0f);  tex_buffer.push_back(-1200.0f); tex_buffer.push_back(10.0f);
    tex_buffer.push_back(1200.0f);  tex_buffer.push_back(1200.0f);  tex_buffer.push_back(10.0f);
    tex_buffer.push_back(-1200.0f); tex_buffer.push_back(1200.0f);  tex_buffer.push_back(10.0f);

    tex_buffer.push_back(-1200.0f); tex_buffer.push_back(1200.0f);  tex_buffer.push_back(10.0f);
    tex_buffer.push_back(-1200.0f); tex_buffer.push_back(-1200.0f); tex_buffer.push_back(10.0f);
    tex_buffer.push_back(1200.0f);  tex_buffer.push_back(-1200.0f); tex_buffer.push_back(10.0f);

    tex_buffer_uv.push_back(16.0f); tex_buffer_uv.push_back(16.0f);
    tex_buffer_uv.push_back(16.0f); tex_buffer_uv.push_back(0.0f);
    tex_buffer_uv.push_back(0.0f);  tex_buffer_uv.push_back(0.0f);

    tex_buffer_uv.push_back(0.0f);  tex_buffer_uv.push_back(0.0f);
    tex_buffer_uv.push_back(0.0f);  tex_buffer_uv.push_back(16.0f);
    tex_buffer_uv.push_back(16.0f); tex_buffer_uv.push_back(16.0f);
}

void initScene()
{
  
  int row = 4;
  int col = 3;
  float temp[4][3];
  int control = 0;
  // grab 4 points for tangent and matrix multiplation
  for(int i = 0; i < iLength - 3 ;i++){
    for(int tempRow = 0; tempRow < row; tempRow++){
      for(int tempCol = 0; tempCol < col; tempCol++){
        if(tempCol == 0){
          temp[tempRow][tempCol] = splines[0].points[i+tempRow].x;
        }else if(tempCol == 1){
          temp[tempRow][tempCol] = splines[0].points[i+tempRow].y;
        }else{
          temp[tempRow][tempCol] = splines[0].points[i+tempRow].z;
        }
      }
    }
    float line_current[3];
    float line_next[3];
    float line_left[3];
    float line_right[3];
    float line_color_c[4];
    float line_color_l[4];
    float line_color_r[4];
    float line_color_n[4];
    for(float u = 0.000f; u <= 1.000f; u = u + 0.01f){
      float parameter[1][4] = {{u*u*u, u*u, u, 1}};
      float result[1][4];
      Point_floating point;
      matrixMul(parameter,constant_M, result);
      float final[1][3];
      matrixMul2(result, temp, final);
      vector<Point_floating> temp2;

      point.x = final[0][0];
      point.y = final[0][1];
      point.z = final[0][2];
      pointBuffer.push_back(point);
      line_current[0] = point.x;
      line_current[1] = point.y;
      line_current[2] = point.z;
      line_buffer.insert(line_buffer.end(), line_current, line_current + 3);

      Point_floating tangent2;
      float parameter1[1][4] = {{3*u*u, 2*u, 1.0f, 0.0f}};
      float result1[1][4];
      matrixMul(parameter1,constant_M, result1);
      float final1[1][3];
      matrixMul2(result1, temp, final1);
      tangent2.x = final1[0][0];
      tangent2.y = final1[0][1];
      tangent2.z = final1[0][2];
      float magnitude = std::sqrt (std::pow (tangent2.x, 2) + std::pow (tangent2.y, 2) + std::pow (tangent2.z, 2));
      tangent2.x = tangent2.x/magnitude;
      tangent2.y = tangent2.y/magnitude;
      tangent2.z = tangent2.z/magnitude;
      tangent_vector.push_back(tangent2);
      if(u == 0 && i == 0){
        normal1 = calculate_normal(tangent2);
        // cout << "my tangent: "<<normal1.x << " " << normal1.y << " " << normal1.z << endl;
      }else{
        normal1 = calculate_normal2( binormal_prev, tangent2);
      }
      binormal1 = calculate_binormal(tangent2, normal1);
      binormal_prev.x = binormal1.x;
      binormal_prev.y = binormal1.y;
      binormal_prev.z = binormal1.z;
      normal_vector.push_back(normal1);
      binormal_vector.push_back(binormal1);



      // Point_floating binormal2;
      // binormal2 = calculate_binormal2(tangent2, normal1);
      // Point_floating newPos;
      // newPos = calculate_binormal2(point,binormal2);
      // newPos.x = point.x + (1/100)*newPos.x;
      // newPos.y = point.y + (1/100)*newPos.y;
      // newPos.z = point.z + (1/100)*newPos.z;
      // pointBuffer_right.push_back(newPos);
      // Point_floating tangent3;
      // float parameter2[1][4] = {{3*u*u, 2*u, 1.0f, 0.0f}};
      // float result2[1][4];
      // matrixMul(parameter2,constant_M, result2);
      // float final2[1][3];
      // matrixMul2(result2, temp, final2);
      // tangent3.x = final2[0][0];
      // tangent3.y = final2[0][1];
      // tangent3.z = final2[0][2];
      // float magnitude2 = std::sqrt (std::pow (tangent3.x, 2) + std::pow (tangent3.y, 2) + std::pow (tangent3.z, 2));
      // tangent3.x = tangent3.x/magnitude2;
      // tangent3.y = tangent3.y/magnitude2;
      // tangent3.z = tangent3.z/magnitude2;
      // tangent_vector_r.push_back(tangent3);
      // Point_floating normal2;
      // Point_floating binormal3;
      // if(u == 0 && i == 0){
      //   normal2 = calculate_normal(tangent3);
      //   // cout << "my tangent: "<<normal1.x << " " << normal1.y << " " << normal1.z << endl;
      // }else{
      //   normal2 = calculate_normal2( binormal_prev_r, tangent3);
      // }
      // binormal3 = calculate_binormal(tangent3, normal2);
      // binormal_prev_r.x = binormal3.x;
      // binormal_prev_r.y = binormal3.y;
      // binormal_prev_r.z = binormal3.z;
      // normal_vector_r.push_back(normal2);
      // binormal_vector_r.push_back(binormal3);

      // line_color_c[0] = 255.0f;
      // line_color_c[1] = 128.0f;
      // line_color_c[2] = 0.0f;
      // line_color_c[3] = 1.0f;

      // line_color.insert(line_color.end(), line_color_c, line_color_c + 4);
    }
  }
  create_rail(tangent_vector, normal_vector, pointBuffer, binormal_vector);
  //create_rail(tangent_vector_r, normal_vector_r, pointBuffer_right, binormal_vector_r);

  // myfile << "Writing this to a file.\n";
  //  for(int i=0; i < pointBuffer.size(); i++){
  //     myfile << "POS: " << pointBuffer[i].x << ' ' << pointBuffer[i].y << ' ' << pointBuffer[i].z<<endl;
  //     myfile << "TAN: " << tangent_vector[i].x << ' ' << tangent_vector[i].y << ' ' << tangent_vector[i].z<<endl;
  //     myfile << "NOR: " << normal_vector[i].x << ' ' << normal_vector[i].y << " " << normal_vector[i].z << endl;
  //     myfile << "BIN: " << binormal_vector[i].x << ' ' << binormal_vector[i].y << " " << binormal_vector[i].z << endl;
  //  }

  pipelineProgram.Init("../openGLHelper-starterCode");
  program = pipelineProgram.GetProgramHandle();


  glGenVertexArrays(1, &vao_line_left);
  glBindVertexArray(vao_line_left);
  //init VBO
  glGenBuffers(1, &lineBuffer_left);
  glBindBuffer(GL_ARRAY_BUFFER, lineBuffer_left);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * railBuffer.size()) + (sizeof(float) *railBufferColor.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo               
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * railBuffer.size()),&railBuffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * railBuffer.size()), (sizeof(float) *railBufferColor.size()), &railBufferColor[0]);

  glBindBuffer(GL_ARRAY_BUFFER, lineBuffer_left);
  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  colorloc = glGetAttribLocation(program, "normal");
  glEnableVertexAttribArray(colorloc);
  offset = (const void*) (sizeof(float) * railBuffer.size());
  glVertexAttribPointer(colorloc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);


  // glGenVertexArrays(1, &vao_sky);
  // glBindVertexArray(vao_sky);
  // //init VBO
  // glGenBuffers(1, &vbo_sky);
  // glBindBuffer(GL_ARRAY_BUFFER, vbo_sky);
  // // create space for vbo
  // glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * tex_buffer_sky.size()) + (sizeof(float) *tex_buffer_uv_sky.size()), NULL,
  //              GL_STATIC_DRAW);
  // // load data into vbo               
  // glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * tex_buffer_sky.size()),&tex_buffer_sky[0]);
  // glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * tex_buffer_sky.size()), (sizeof(float) *tex_buffer_uv_sky.size()), &tex_buffer_uv_sky[0]);

  // glBindBuffer(GL_ARRAY_BUFFER, vbo_sky);
  // loc = glGetAttribLocation(program, "position");
  // glEnableVertexAttribArray(loc);
  // const void * offset = (const void*) 0;
  // glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  // //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  // colorloc = glGetAttribLocation(program, "normal");
  // glEnableVertexAttribArray(colorloc);
  // offset = (const void*) (sizeof(float) * railBuffer.size());
  // glVertexAttribPointer(colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  // glBindVertexArray(0);

  populate_texture_vbo();
  texPipelineProgram.Init("../openGLHelper-starterCode");
  tex_program = texPipelineProgram.GetProgramHandle();
  // t_modelViewMatrix = glGetUniformLocation(tex_program, "modelViewMatrix");
  // t_projectionMatrix = glGetUniformLocation(tex_program, "projectionMatrix");
  glGenTextures(1,&texHandle);
  initTexture("space.jpg", texHandle);

  glGenVertexArrays(1, &vao_tex);
  glBindVertexArray(vao_tex);
  //init VBO
  glGenBuffers(1, &vbo_tex);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * tex_buffer.size() + sizeof(float)*tex_buffer_uv.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo               
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * tex_buffer.size()),&tex_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * tex_buffer.size()), (sizeof(float) * tex_buffer_uv.size()),&tex_buffer_uv[0]);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
  loc = glGetAttribLocation(tex_program, "position");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  loc = glGetAttribLocation(tex_program, "texCoord");
  glEnableVertexAttribArray(loc);
  offset = (const void*) (sizeof(float) * tex_buffer.size());
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);

}
// Note: You should combine this file
// with the solution of homework 1.

// Note for Windows/MS Visual Studio:
// You should set argv[1] to track.txt.
// To do this, on the "Solution Explorer",
// right click your project, choose "Properties",
// go to "Configuration Properties", click "Debug",
// then type your track file name for the "Command Arguments".
// You can also repeat this process for the "Release" configuration.

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  // load the splines from the provided filename
  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif
  loadSplines(argv[1]);

  printf("Loaded %d spline(s).\n", numSplines);

  for(int i=0; i<numSplines; i++){
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);
    initScene();
  }
    
    // initScene();
  glutMainLoop();
  return 0;
}
