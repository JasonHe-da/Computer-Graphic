/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <ninghaoh@usc.edu 4247495840>
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"
#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;
GLint h_modelViewMatrix, h_projectionMatrix;
int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not
int photonumber = 0;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";
bool startRecord = false;
ImageIO * heightmapImage;
vector<float> point_buffer;
vector<float> point_color;
vector<float> line_buffer;
vector<float> line_color;
vector<float> line_colorE;
vector<float> triangle_buffer;
vector<float> striangle_buffer;
vector<float> triangle_color;
vector<float> striangle_color;
vector<float> left_p;
vector<float> right_p;
vector<float> top_p;
vector<float> under_p;

int move_image =0;
GLuint pointBuffer;
GLuint lineBufferE;
GLuint lineBuffer, lineColor;
GLuint triBuffer, triColor;
GLuint triBufferE;
GLuint striBuffer, striColor;
GLuint vao_point, vao_line;
GLuint vboLeft, vboRight, vboTop, vboUnder;
GLuint vao_tri;
int sizeTri;
GLuint loc, colorloc;
GLuint tri_loc, tri_colorloc;
GLuint vao_soomth;
GLuint vao_lineE;
GLuint vao_triE;
OpenGLMatrix matrix;
BasicPipelineProgram pipelineProgram;
GLuint soomth_mode;
GLuint program;
int totalpixal;
int mode_temp = 1;

// write a screenshot to the specified filename
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

// set up modelview and draw points
void renderPoints(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f,500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_point); 
  // draw 
  GLsizei count = point_buffer.size()/3;
  glDrawArrays(GL_POINTS, 0, count);


  glBindVertexArray(0); 
  glutSwapBuffers();
}
// set up modelview and draw lines
void renderLines(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f, 500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_line); 
  // draw 
  GLsizei count = line_buffer.size()/3;
  glDrawArrays(GL_LINES, 0, count);


  glBindVertexArray(0); 
  glutSwapBuffers();
}


// set up modelview and draw triangles
void renderTriangles(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f, 500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);
  //matrix.LookAt(64.0f, 300.0f, 0, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_tri); 
  // draw 
  GLsizei count = triangle_buffer.size()/3;
  glDrawArrays(GL_TRIANGLES, 0, count);

  glBindVertexArray(0); 
  glutSwapBuffers();
}

// set up modelview and draw smooth triangles
void renderSTriangles(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f, 500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);
  //matrix.LookAt(64.0f, 300.0f, 0, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_soomth); 
  // draw 
  GLsizei count = striangle_buffer.size()/3;
  
  glDrawArrays(GL_TRIANGLES, 0, count);

  glBindVertexArray(0); 
  glutSwapBuffers();
}

// set up offset and draw lines first then draw the triangles (extra)
void renderExtra(){
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonOffset(-1.0, -1.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f, 500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
  float m[16];
  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  float p[16];
  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_lineE); 
  // draw 
  GLsizei count = line_buffer.size()/3;
  glDrawArrays(GL_LINES, 0, count);


  glBindVertexArray(0); 

  glDisable(GL_POLYGON_OFFSET_LINE);   
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
  matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
  matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.LookAt(64.0f, 500.0f, -64.0f, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);
  //matrix.LookAt(64.0f, 300.0f, 0, 64.0f, 0.0f, -64.0f, 0.0f, 0.0f, -1.0f);

  h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");

  matrix.GetMatrix(m);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

  h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::Projection);

  matrix.GetMatrix(p);
  pipelineProgram.Bind();
  glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
  glBindVertexArray(vao_triE); 
  // draw 
  count = triangle_buffer.size()/3;
  // glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
  glDrawArrays(GL_TRIANGLES, 0, count);

  glBindVertexArray(0); 
  glutSwapBuffers();
}
void displayFunc()
{
  GLint loc = glGetUniformLocation(pipelineProgram.GetProgramHandle(), "mode");
  // render some stuff...
  // depending which key is pressed, here we will decide which function should be calles
  if (mode_temp==1){
    glUniform1i(loc,0);
    renderPoints();
  }else if(mode_temp == 2){
    glUniform1i(loc,0);
    renderLines();
  }else if(mode_temp == 3){
    glUniform1i(loc,0);
    renderTriangles();
  }else if(mode_temp == 4){
    glUniform1i(loc,1);
    renderSTriangles();
  }else if(mode_temp == 5){
    glUniform1i(loc,0);
    renderExtra();
  }
}


void idleFunc()
{
  // some simple animation will be shown, move top left then bottom right
  if(move_image < 100){
    matrix.Translate(1.0f, 1.0f, 0);
    //matrix.Rotate(0.2f, 0.0f, 1.0f, 0.0f);
    move_image += 1;
  }else if(move_image < 200){
    matrix.Translate(-1.0f, -1.0f, 0);
    //matrix.Rotate(0.2f, 0.0f, 1.0f, 0.0f);
    move_image += 1;
  }else if(move_image < 300){
    matrix.Translate(-1.0f, -1.0f, 0);
    //matrix.Rotate(0.2f, 0.0f, 1.0f, 0.0f);
    move_image += 1;
  }else if(move_image < 400){
    matrix.Translate(1.0f, 1.0f, 0);
    //matrix.Rotate(0.2f, 0.0f, 1.0f, 0.0f);
    move_image += 1;
  }else{
    move_image = 0;
  }
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


  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 1000.0f);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
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
        landTranslate[0] += mousePosDelta[0] * 0.1f;
        landTranslate[1] -= mousePosDelta[1] * 0.1f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.1f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1]* 0.1f;
        landRotate[1] += mousePosDelta[0]* 0.1f;
        //matrix.Rotate(0.5, landRotate[0],landRotate[1],landRotate[2]);
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
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.003f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.003f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.003f;
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
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
    case '1':
      mode_temp = 1;
    break;

    case '2':
      mode_temp = 2;
    break;

    case '3':

      mode_temp = 3;

    break;
    case '4':

      mode_temp = 4;
      
    break;
    
    case '5':
      mode_temp = 5;
    break;

    case 't':
      controlState = TRANSLATE;
    break;

    case 'T':
      controlState = TRANSLATE;
    break;

    case 'k':
      startRecord = true;
    break;

    case 'c':
      startRecord = false;
    break;
  }
}
void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  // get height and width according to pixels
  int height = heightmapImage->getHeight();
  int width = heightmapImage->getHeight();



  // insert points from bottom left.
  // and color with respect to their pixel
  // color set up : range(0-1,0-1,0-1, 1)
  float point_current[3];
  float point_c_color[4];
  for(int i=0; i < height - 1; i++){
    for(int j=0; j < width - 1;j++){
      point_current[0] = (float) i;
      // scale down the height
      // otherwise it will be too large to see
      point_current[1] = heightmapImage->getPixel(i, j, 0) *0.2;
      point_current[2] = -(float) j;

      point_c_color[0] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      point_c_color[1] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      point_c_color[2] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      point_c_color[3] = 1.0f;

      point_buffer.insert(point_buffer.end(),point_current, point_current+3);
      point_color.insert(point_color.end(),point_c_color, point_c_color+4);
    }
  }
  float line_current[3];
  float line_below[3];
  float line_right[3];
  float line_color_c[4];
  float line_color_b[4];
  float line_color_r[4];


  // put them in order so we are able to draw lines
  // current -> right
  //    |
  //  below
  // color set up : range(0-1,0-1,0-1, 1)
  for(int i = 0;i < height - 1;i++){
    for(int j=0; j< width - 1;j++){
      line_current[0] = (float) i;
      line_current[1] = heightmapImage->getPixel(i, j, 0) *0.2;
      line_current[2] = -(float) j;
      line_color_c[0] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_c[1] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_c[2] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_c[3] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;

      line_below[0] = (float) i+1;
      // scale down the height
      // otherwise it will be too large to see
      line_below[1] = heightmapImage->getPixel(i+1, j, 0) *0.2;
      line_below[2] = -(float) j;
      line_color_b[0] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_b[1] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_b[2] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_b[3] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;

      line_right[0] = (float) i;
      line_right[1] = heightmapImage->getPixel(i, j+1, 0) *0.2;
      line_right[2] = -(float) (j+1);
      line_color_r[0] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_r[1] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_r[2] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;
      line_color_r[3] = float(heightmapImage->getPixel(i, j, 0))*1/155.0f;

      line_buffer.insert (line_buffer.end(), line_current, line_current + 3);
      line_buffer.insert (line_buffer.end(), line_below, line_below + 3);
      line_buffer.insert (line_buffer.end(), line_current, line_current + 3);
      line_buffer.insert (line_buffer.end(), line_right, line_right + 3);

      line_color.insert (line_color.end(), line_color_c, line_color_c + 4);
      line_color.insert (line_color.end(), line_color_b, line_color_b + 4);
      line_color.insert (line_color.end(), line_color_c, line_color_c + 4);
      line_color.insert (line_color.end(), line_color_r, line_color_r + 4);

      
      // extra part with red color on top of the triangles mode
      float temp[4] = {0.5f,0,0,1.0f};
      line_colorE.insert (line_colorE.end(), temp, temp + 4);
      line_colorE.insert (line_colorE.end(), temp, temp + 4);
      line_colorE.insert (line_colorE.end(), temp, temp + 4);
      line_colorE.insert (line_colorE.end(), temp, temp + 4);
    }
  }


  float tri_current[3];
  float tri_below[3];
  float tri_right[3];
  float tri_temp[3];
  float c_left[3];  
  float c_top[3]; 
  float c_under[3];  
  float c_right[3]; 
  float b_left[3];  
  float b_top[3]; 
  float b_under[3];  
  float b_right[3]; 
  float t_left[3];  
  float t_top[3]; 
  float t_under[3];  
  float t_right[3]; 
  float r_left[3];  
  float r_top[3]; 
  float r_under[3];  
  float r_right[3]; 
  float color_current[4];
  float color_below[4];
  float color_right[4];
  float color_temp[4];

  // triangle points insert in order
  // basically draw two triangles
  // so in a square there are two triangles will be shown
  // color set up : range(0-1,0-1,0-1, 1)
  for(int i = 0; i < height - 1; i++){
    for(int j= 0; j < width - 1; j++){
      //range(0-1,0-1,0-1, 1)
      // have to scale down otherwise there are many obvious white color pixels on the edge
      float color_current_num = float(heightmapImage->getPixel(i, j, 0)-7)/155.0f;
      float color_below_num = (float(heightmapImage->getPixel(i, j, 0)-7))/155.0f;
      float color_right_num = (float(heightmapImage->getPixel(i, j, 0)-7))/155.0f;
      float color_temp_num = (float(heightmapImage->getPixel(i, j, 0)-7))/155.0f;

      tri_current[0] = (float) i;
      tri_current[1] = heightmapImage->getPixel(i, j, 0)*0.2;
      tri_current[2] = -(float) j;
      color_current[0] = color_current_num;
      color_current[1] = color_current_num;
      color_current[2] = color_current_num;
      color_current[3] = 1.0;

      tri_below[0] = (float) i;
      // scale down the height
      // otherwise it will be too large to see
      tri_below[1] = heightmapImage->getPixel(i, j+1, 0)*0.2;
      tri_below[2] = -(float) (j+1);
      color_below[0] = color_below_num;
      color_below[1] = color_below_num;
      color_below[2] = color_below_num;
      color_below[3] = 1.0;

      tri_temp[0] = (float) i+1;
      tri_temp[1] = heightmapImage->getPixel(i+1, j+1, 0)*0.2;
      tri_temp[2] = -(float) (j+1);
      color_temp[0] = color_temp_num;
      color_temp[1] = color_temp_num;
      color_temp[2] = color_temp_num;
      color_temp[3] = 1.0;

      tri_right[0] = (float) i+1;
      tri_right[1] = heightmapImage->getPixel(i+1, j, 0)*0.2;
      tri_right[2] = -(float) (j);
      color_right[0] = color_right_num;
      color_right[1] = color_right_num;
      color_right[2] = color_right_num;
      color_right[3] = 1.0;
      triangle_buffer.insert (triangle_buffer.end(), tri_below, tri_below + 3);
      triangle_buffer.insert (triangle_buffer.end(), tri_temp, tri_temp + 3);
      triangle_buffer.insert (triangle_buffer.end(), tri_current, tri_current + 3);
      
      triangle_buffer.insert (triangle_buffer.end(), tri_current, tri_current + 3);
      triangle_buffer.insert (triangle_buffer.end(), tri_temp, tri_temp + 3);
      triangle_buffer.insert (triangle_buffer.end(), tri_right, tri_right + 3);

      triangle_color.insert (triangle_color.end(), color_below, color_below + 4);
      triangle_color.insert (triangle_color.end(), color_temp, color_temp + 4);
      triangle_color.insert (triangle_color.end(), color_current, color_current + 4);
      triangle_color.insert (triangle_color.end(), color_current, color_current + 4);
      triangle_color.insert (triangle_color.end(), color_temp, color_temp + 4);
      triangle_color.insert (triangle_color.end(), color_right, color_right + 4);
      

      // generate neighbor points for each vertex 
      // corner case: for soomth triangles
      // left boundary will make left point become right
      // and so on
      if(i == 0){
        c_left[0] = (float) i+1;
        c_left[1] = heightmapImage->getPixel(i + 1, j, 0)*0.2;
        c_left[2] = -(float) j;
        b_left[0] = (float) i+1;
        b_left[1] = heightmapImage->getPixel(i + 1, j + 1, 0)*0.2;
        b_left[2] = -(float) j+1;
        t_left[0] = (float) i+2;
        t_left[1] = heightmapImage->getPixel(i+2, j + 1, 0)*0.2;
        t_left[2] = -(float) j+1;
        r_left[0] = (float) i+2;
        r_left[1] = heightmapImage->getPixel(i + 2, j, 0)*0.2;
        r_left[2] = -(float) j;
      }else{
        c_left[0] = (float) i-1;
        c_left[1] = heightmapImage->getPixel(i - 1, j, 0)*0.2;
        c_left[2] = -(float) j;
        b_left[0] = (float) i-1;
        b_left[1] = heightmapImage->getPixel(i - 1, j + 1, 0)*0.2;
        b_left[2] = -(float) j+1;
        t_left[0] = (float) i;
        t_left[1] = heightmapImage->getPixel(i, j + 1, 0)*0.2;
        t_left[2] = -(float) j+1;
        r_left[0] = (float) i;
        r_left[1] = heightmapImage->getPixel(i, j, 0)*0.2;
        r_left[2] = -(float) j;
      }
      left_p.insert(left_p.end(), b_left,b_left+3);
      left_p.insert(left_p.end(), t_left,t_left+3);
      left_p.insert(left_p.end(), c_left,c_left+3);
      left_p.insert(left_p.end(), c_left,c_left+3);
      left_p.insert(left_p.end(), t_left,t_left+3);
      left_p.insert(left_p.end(), r_left,r_left+3);
      if(i==width-2){
        c_right[0] = (float) i-1;
        c_right[1] = heightmapImage->getPixel(i - 1, j, 0)*0.2;
        c_right[2] = -(float) j;
        b_right[0] = (float) i-1;
        b_right[1] = heightmapImage->getPixel(i - 1, j + 1, 0)*0.2;
        b_right[2] = -(float) j+1;
        t_right[0] = (float) i;
        t_right[1] = heightmapImage->getPixel(i, j + 1, 0)*0.2;
        t_right[2] = -(float) j+1;
        r_right[0] = (float) i;
        r_right[1] = heightmapImage->getPixel(i, j, 0)*0.2;
        r_right[2] = -(float) j;
      }else{
        c_right[0] = (float) i+1;
        c_right[1] = heightmapImage->getPixel(i + 1, j, 0)*0.2;
        c_right[2] = -(float) j;
        b_right[0] = (float) i+1;
        b_right[1] = heightmapImage->getPixel(i + 1, j+1, 0)*0.2;
        b_right[2] = -(float) j+1;
        t_right[0] = (float) i+2;
        t_right[1] = heightmapImage->getPixel(i + 2, j+1, 0)*0.2;
        t_right[2] = -(float) j+1;
        r_right[0] = (float) i+2;
        r_right[1] = heightmapImage->getPixel(i + 2, j, 0)*0.2;
        r_right[2] = -(float) j;
      }
      
      right_p.insert(right_p.end(),b_right,b_right+3);
      right_p.insert(right_p.end(),t_right,t_right+3);
      right_p.insert(right_p.end(),c_right,c_right+3);
      right_p.insert(right_p.end(),c_right,c_right+3);
      right_p.insert(right_p.end(),t_right,t_right+3);
      right_p.insert(right_p.end(),r_right,r_right+3);
      if(j==height-2){
        c_top[0] = (float) i;
        c_top[1] = heightmapImage->getPixel(i, j - 1, 0)*0.2;
        c_top[2] = -(float) j - 1;
        b_top[0] = (float) i;
        b_top[1] = heightmapImage->getPixel(i, j, 0)*0.2;
        b_top[2] = -(float) j;
        t_top[0] = (float) i+1;
        t_top[1] = heightmapImage->getPixel(i+1, j, 0)*0.2;
        t_top[2] = -(float) j;
        r_top[0] = (float) i+1;
        r_top[1] = heightmapImage->getPixel(i+1, j - 1, 0)*0.2;
        r_top[2] = -(float) j - 1;
      }else{
        c_top[0] = (float) i;
        c_top[1] = heightmapImage->getPixel(i, j + 1, 0)*0.2;
        c_top[2] = -(float) j + 1;
        b_top[0] = (float) i;
        b_top[1] = heightmapImage->getPixel(i, j + 2, 0)*0.2;
        b_top[2] = -(float) j + 2;
        t_top[0] = (float) i+1;
        t_top[1] = heightmapImage->getPixel(i+1, j + 2, 0)*0.2;
        t_top[2] = -(float) j + 2;
        r_top[0] = (float) i+1;
        r_top[1] = heightmapImage->getPixel(i+1, j + 1, 0)*0.2;
        r_top[2] = -(float) j + 1;
      }
      
      top_p.insert(top_p.end(),b_top,b_top+3);
      top_p.insert(top_p.end(),t_top,t_top+3);
      top_p.insert(top_p.end(),c_top,c_top+3);
      top_p.insert(top_p.end(),c_top,c_top+3);
      top_p.insert(top_p.end(),t_top,t_top+3);
      top_p.insert(top_p.end(),r_top,r_top+3);
      if(j==0){
        c_under[0] = (float) i;
        c_under[1] = heightmapImage->getPixel(i, j + 1, 0)*0.2;
        c_under[2] = -(float) j + 1;
        b_under[0] = (float) i;
        b_under[1] = heightmapImage->getPixel(i, j + 2, 0)*0.2;
        b_under[2] = -(float) j + 2;
        t_under[0] = (float) i+1;
        t_under[1] = heightmapImage->getPixel(i+1, j + 2, 0)*0.2;
        t_under[2] = -(float) j + 2;
        r_under[0] = (float) i+1;
        r_under[1] = heightmapImage->getPixel(i+1, j + 1, 0)*0.2;
        r_under[2] = -(float) j + 1;
      }else{
        c_under[0] = (float) i;
        c_under[1] = heightmapImage->getPixel(i, j - 1, 0)*0.2;
        c_under[2] = -(float) j - 1;
        b_under[0] = (float) i;
        b_under[1] = heightmapImage->getPixel(i, j, 0)*0.2;
        b_under[2] = -(float) j;
        t_under[0] = (float) i+1;
        t_under[1] = heightmapImage->getPixel(i+1, j, 0)*0.2;
        t_under[2] = -(float) j;
        r_under[0] = (float) i+1;
        r_under[1] = heightmapImage->getPixel(i+1, j - 1, 0)*0.2;
        r_under[2] = -(float) j - 1;
      }
      under_p.insert(under_p.end(),b_under,b_under+3);
      under_p.insert(under_p.end(),t_under,t_under+3);
      under_p.insert(under_p.end(),c_under,c_under+3);
      under_p.insert(under_p.end(),c_under,c_under+3);
      under_p.insert(under_p.end(),t_under,t_under+3);
      under_p.insert(under_p.end(),r_under,r_under+3);
    }
  }

  striangle_buffer = triangle_buffer;
  striangle_color = triangle_color;

  // modify the following code accordingly



  // set up pipeline
  pipelineProgram.Init("../openGLHelper-starterCode");
  program = pipelineProgram.GetProgramHandle();

  // set up vao and vbo for points
  glGenVertexArrays(1, &vao_point);
  glBindVertexArray(vao_point);
  //init VBO
  glGenBuffers(1, &pointBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * point_buffer.size()) + (sizeof(float) * point_color.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo             
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * point_buffer.size()), &point_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * point_buffer.size()), (sizeof(float) * point_color.size()), &point_color[0]);

  glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  const void * offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(colorloc);
  offset = (const void*) (sizeof(float) * point_buffer.size());
  glVertexAttribPointer(colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);
  
  // set up vao and vbo for lines
  glGenVertexArrays(1, &vao_line);
  glBindVertexArray(vao_line);
  //init VBO
  glGenBuffers(1, &lineBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * line_buffer.size()) + (sizeof(float) *line_color.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo               
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * line_buffer.size()),&line_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * line_buffer.size()), (sizeof(float) *line_color.size()), &line_color[0]);

  glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(colorloc);
  offset = (const void*) (sizeof(float) * line_buffer.size());
  glVertexAttribPointer(colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);



  // set up vao and vbo for triangle
  glGenVertexArrays(1, &vao_tri);
  glBindVertexArray(vao_tri);
  //init VBO
  glGenBuffers(1, &triBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triBuffer);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * triangle_buffer.size()) + (sizeof(float) *triangle_color.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * triangle_buffer.size()),&triangle_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * triangle_buffer.size()), (sizeof(float) *triangle_color.size()), &triangle_color[0]);

  glBindBuffer(GL_ARRAY_BUFFER, triBuffer);
  tri_loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(tri_loc);
  offset = (const void*) 0;
  glVertexAttribPointer(tri_loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  tri_colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(tri_colorloc);
  // set up offset correctly, otherwise it will make 1/4 of triangles with no color
  offset = (const void*) (sizeof(float) * triangle_buffer.size());
  glVertexAttribPointer(tri_colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);



  // separate VAO and VBO for extra credits
  glGenVertexArrays(1, &vao_triE);
  glBindVertexArray(vao_triE);
  //init VBO
  glGenBuffers(1, &triBufferE);
  glBindBuffer(GL_ARRAY_BUFFER, triBufferE);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * triangle_buffer.size()) + (sizeof(float) *triangle_color.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * triangle_buffer.size()),&triangle_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * triangle_buffer.size()), (sizeof(float) *triangle_color.size()), &triangle_color[0]);

  glBindBuffer(GL_ARRAY_BUFFER, triBufferE);
  tri_loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(tri_loc);
  offset = (const void*) 0;
  glVertexAttribPointer(tri_loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  tri_colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(tri_colorloc);
  offset = (const void*) (sizeof(float) * triangle_buffer.size());
  glVertexAttribPointer(tri_colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);

  glGenVertexArrays(1, &vao_lineE);
  glBindVertexArray(vao_lineE);
  //init VBO
  glGenBuffers(1, &lineBufferE);
  glBindBuffer(GL_ARRAY_BUFFER, lineBufferE);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * line_buffer.size()) + (sizeof(float) *line_colorE.size()), NULL,
               GL_STATIC_DRAW);
  // load data into vbo             
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * line_buffer.size()),&line_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * line_buffer.size()), (sizeof(float) *line_colorE.size()), &line_colorE[0]);

  glBindBuffer(GL_ARRAY_BUFFER, lineBufferE);
  loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(colorloc);
  offset = (const void*) (sizeof(float) * line_buffer.size());
  glVertexAttribPointer(colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);

  glGenVertexArrays(1, &vao_soomth);
  glBindVertexArray(vao_soomth);

  // set up 4 vbos for each left right top under points for soomth triangles
  glGenBuffers(1, &vboLeft);
  glBindBuffer(GL_ARRAY_BUFFER, vboLeft);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * left_p.size()), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * left_p.size()),&left_p[0]);
  loc = glGetAttribLocation(program, "left_p");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);


  glGenBuffers(1, &vboRight);
  glBindBuffer(GL_ARRAY_BUFFER, vboRight);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * right_p.size()), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * right_p.size()),&right_p[0]);
  loc = glGetAttribLocation(program, "right_p");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);


  glGenBuffers(1, &vboTop);
  glBindBuffer(GL_ARRAY_BUFFER, vboTop);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * top_p.size()) , NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * top_p.size()),&top_p[0]);
  loc = glGetAttribLocation(program, "top_p");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

  glGenBuffers(1, &vboUnder);
  glBindBuffer(GL_ARRAY_BUFFER, vboUnder);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * under_p.size()), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * under_p.size()),&under_p[0]);
  loc = glGetAttribLocation(program, "under_p");
  glEnableVertexAttribArray(loc);
  offset = (const void*) 0;
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);  

  glGenBuffers(1, &striBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, striBuffer);
  // create space for vbo
  glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * striangle_buffer.size()) + (sizeof(float) *striangle_color.size()), NULL,
               GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0, (sizeof(float) * striangle_buffer.size()),&striangle_buffer[0]);
  glBufferSubData(GL_ARRAY_BUFFER,(sizeof(float) * striangle_buffer.size()), (sizeof(float) *striangle_color.size()), &striangle_color[0]);

  glBindBuffer(GL_ARRAY_BUFFER, striBuffer);
  tri_loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(tri_loc);
  offset = (const void*) 0;
  glVertexAttribPointer(tri_loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
  //glBindBuffer(GL_ARRAY_BUFFER, pointColor); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WHY
  tri_colorloc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(tri_colorloc);
  offset = (const void*) (sizeof(float) * striangle_buffer.size());
  glVertexAttribPointer(tri_colorloc, 4, GL_FLOAT, GL_FALSE, 0, offset);
  glBindVertexArray(0);  
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

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

  // do initialization
  initScene(argc, argv);
  // sink forever into the glut loop
  glutMainLoop();
}


