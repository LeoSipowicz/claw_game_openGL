/*
 *  Leo Sipowicz: Final Project
 *
 *  Claw game with random toy spawns animation lighting and texture
 *
 *  Key bindings:
 * 
 *  Game controls
 *  ←→/⇵        = Toggle camera views
 *  e         = Lower Claw/Pick up claw/reset game
 *  w         = move claw back
 *  s         = move claw forward
 *  a         = move claw left
 *  d         = move claw right
 *  
 *  Developer controls
 *  o         = change objects to test
 *  m         = play/pause light (default paused for)
 *  PgUp/PgDn = Zoom
 *  l         = close claw
 */
#include "CSCIx229.h"

int mode = 1; //  Projection mode
int move = 0; //  Move light
int th = 0; //  Azimuth of view angle
int ph = 25; //  Elevation of view angle
int fov = 55; //  Field of view (for perspective)
int obj = 0; //  Scene/opbject selection
double asp = 1; //  Aspect ratio
double dim = 1.5; //  Size of world
// Light values
int light = 1; // Lighting
int one = 1; // Unit value
int distance = 5; // Light distance
int inc = 10; // Ball increment
int smooth = 1; // Smooth/Flat shading
int local = 0; // Local Viewer Model
int emission = 0; // Emission intensity (%)
int ambient = 35; // Ambient intensity (%)
int diffuse = 50; // Diffuse intensity (%)
int specular = 0; // Specular intensity (%)
int shininess = 0; // Shininess (power of two)
float shiny = 3; // Shininess (value)
int zh = 90;
float ylight = 0; // Elevation of light
typedef struct {
  float x, y, z;
}
vtx;
typedef struct {
  int A, B, C;
}
tri;
#define n 500
vtx is[n];
unsigned int texture[16]; // Tradional textures
int sky[2]; //  Sky textures
int ntex = 0;
double rep = 1;

//Claw game controls
double buttonHeight = .2;
double controlAngleX = 0.0;
double controlAngleZ = 0.0;

//Claw state
double clawZpos = 0.0;
double clawXpos = 0.0;
double clawYpos = .30;

int IsClawOut = 0;
double clawClosedAngle = 0;

//Bear state
float bearXpos = 0;
double bearYpos = .225;
float bearZpos = 0;

//Train state
float trainXpos = .25;
double trainYpos = .25;
float trainZpos = .15;

//Eobot state
float robotXpos = -.25;
double robotYpos = .18;
float robotZpos = 0;

//Keep track of wins and losses
int wins = 0;
int misses = 0;

//Camera mode
int viewMode = 1;

/* 
 *  Draw sky box with simple wall texture to simulate arcade
 *  Code borrowed from class
 */
static void Sky(double D) {
  //  Textured white box dimension (-D,+D)
  glPushMatrix();
  glScaled(D, D, D);
  glEnable(GL_TEXTURE_2D);
  glColor3f(1, 1, 1);

  //  Sides
  glBindTexture(GL_TEXTURE_2D, sky[0]);
  glBegin(GL_QUADS);
  glTexCoord2f(0.00, 0);
  glVertex3f(-1, -1, -1);
  glTexCoord2f(0.25, 0);
  glVertex3f(+1, -1, -1);
  glTexCoord2f(0.25, 1);
  glVertex3f(+1, +1, -1);
  glTexCoord2f(0.00, 1);
  glVertex3f(-1, +1, -1);

  glTexCoord2f(0.25, 0);
  glVertex3f(+1, -1, -1);
  glTexCoord2f(0.50, 0);
  glVertex3f(+1, -1, +1);
  glTexCoord2f(0.50, 1);
  glVertex3f(+1, +1, +1);
  glTexCoord2f(0.25, 1);
  glVertex3f(+1, +1, -1);

  glTexCoord2f(0.50, 0);
  glVertex3f(+1, -1, +1);
  glTexCoord2f(0.75, 0);
  glVertex3f(-1, -1, +1);
  glTexCoord2f(0.75, 1);
  glVertex3f(-1, +1, +1);
  glTexCoord2f(0.50, 1);
  glVertex3f(+1, +1, +1);

  glTexCoord2f(0.75, 0);
  glVertex3f(-1, -1, +1);
  glTexCoord2f(1.00, 0);
  glVertex3f(-1, -1, -1);
  glTexCoord2f(1.00, 1);
  glVertex3f(-1, +1, -1);
  glTexCoord2f(0.75, 1);
  glVertex3f(-1, +1, +1);
  glEnd();

  //  Top and bottom
  glBindTexture(GL_TEXTURE_2D, sky[1]);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0);
  glVertex3f(+1, +1, -1);
  glTexCoord2f(0.5, 0);
  glVertex3f(+1, +1, +1);
  glTexCoord2f(0.5, 1);
  glVertex3f(-1, +1, +1);
  glTexCoord2f(0.0, 1);
  glVertex3f(-1, +1, -1);

  glTexCoord2f(1.0, 1);
  glVertex3f(-1, -1, +1);
  glTexCoord2f(0.5, 1);
  glVertex3f(+1, -1, +1);
  glTexCoord2f(0.5, 0);
  glVertex3f(+1, -1, -1);
  glTexCoord2f(1.0, 0);
  glVertex3f(-1, -1, -1);
  glEnd();

  //  Undo
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

/*
 *  Draw vertex in polar coordinates with normal
 *  Code borrowed from class
 */
static void Vertex(double th, double ph) {
  double x = Sin(th) * Cos(ph);
  double y = Cos(th) * Cos(ph);
  double z = Sin(ph);
  //  For a sphere at the origin, the position
  //  and normal vectors are the same
  glNormal3d(x, y, z);
  glTexCoord2f(th / 50, ph / 50);
  glVertex3d(x, y, z);
}

/*
 *  Draw a Box
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     Rotated around y and z axis
 *     takes int for texture value
 */
static void Box(double x, double y, double z,
  double dx, double dy, double dz,
  double yh, double zh, int text) {
  //  Set specular color to white
  float white[] = {
    1,
    1,
    1,
    1
  };
  float black[] = {
    0,
    0,
    0,
    1
  };
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
  //  Save transformation
  glPushMatrix();
  //  Offset, scale and rotate
  glTranslated(x, y, z);
  glRotated(yh, 1, 0, 0);
  glRotated(zh, 0, 0, 1);
  glScaled(dx, dy, dz);
  //  Cube
  glBindTexture(GL_TEXTURE_2D, texture[text]);

  glBegin(GL_QUADS);
  // Front Face
  glNormal3f(0.0, 0.0, one);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-1.0, -1.0, 1.0);
  glTexCoord2f(rep, 0.0);
  glVertex3f(1.0, -1.0, 1.0);
  glTexCoord2f(rep, rep);
  glVertex3f(1.0, 1.0, 1.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(-1.0, 1.0, 1.0);
  // Back Face
  glNormal3f(0.0, 0.0, -one);
  glTexCoord2f(rep, 0.0);
  glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(rep, rep);
  glVertex3f(-1.0, 1.0, -1.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(1.0, 1.0, -1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(1.0, -1.0, -1.0);
  // Top Face
  glNormal3f(0.0, one, 0.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(-1.0, 1.0, -1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-1.0, 1.0, 1.0);
  glTexCoord2f(rep, 0.0);
  glVertex3f(1.0, 1.0, 1.0);
  glTexCoord2f(rep, rep);
  glVertex3f(1.0, 1.0, -1.0);
  // Bottom Face
  glNormal3f(0.0, -one, 0.0);
  glTexCoord2f(rep, rep);
  glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(1.0, -1.0, -1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(1.0, -1.0, 1.0);
  glTexCoord2f(rep, 0.0);
  glVertex3f(-1.0, -1.0, 1.0);
  // Right Face
  glNormal3f(one, 0.0, 0.0);
  glTexCoord2f(rep, 0.0);
  glVertex3f(1.0, -1.0, -1.0);
  glTexCoord2f(rep, rep);
  glVertex3f(1.0, 1.0, -1.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(1.0, 1.0, 1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(1.0, -1.0, 1.0);
  // Left Face
  glNormal3f(-one, 0.0, 0.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(rep, 0.0);
  glVertex3f(-1.0, -1.0, 1.0);
  glTexCoord2f(rep, rep);
  glVertex3f(-1.0, 1.0, 1.0);
  glTexCoord2f(0.0, rep);
  glVertex3f(-1.0, 1.0, -1.0);
  glEnd();
  //  Undo transofrmations
  glPopMatrix();
}

/*
 *  Draw a Cylinder
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 *     takes int for texture
 *     Draws with 72 quad strips to save polygons
 */
static void Cylinder(double x, double y, double z, double dx, double dy, double dz, int text) {
  glPushMatrix();
  glTranslated(x, y, z);

  glScaled(dx, dy, dz);

  glBindTexture(GL_TEXTURE_2D, texture[text]);

  glBegin(GL_QUAD_STRIP);
  for (int a = 0; a <= 360; a = a + 5) {
    //𝑥=𝑟sin𝜃
    //𝑦=𝑟cos𝜃
    double u = a / (double) 360;
    double x = dx * cos(2 * M_PI * u);
    double y = dy;
    double z = dz * sin(2 * M_PI * u);
    glTexCoord2d(u, 1.0);
    glVertex3f(x, 0, z);
    glTexCoord2d(u, 0.0);
    glVertex3f(x, 2 * y, z);
    glNormal3f(x, 0, z);
  }
  glEnd();

  glPopMatrix();
}

/*
 *  Draw a Cylinder with cap
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y and axis
 *     takes int for texture
 *     has cap on one side
 *     uses 24 triangle fans and 24 quad strips
 */
static void CappedCylinder(double x, double y, double z, double dx, double dy, double dz, double yr, double zr, int text) {
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotated(yr, 0, 1, 0);
  glRotated(zr, 1, 0, 0);
  glScaled(dx, dy, dz);
  //  Top
  glBindTexture(GL_TEXTURE_2D, texture[text]);
  glBegin(GL_TRIANGLE_FAN);
  glNormal3f(0, 0, 1);
  glVertex3f(0, 0, 1);
  for (int th = 0; th <= 360; th += 15)
    glVertex3f(Cos(th), Sin(th), 1);
  glEnd();
  //  Sides
  glBegin(GL_QUAD_STRIP);
  for (int th = 0; th <= 360; th += 15) {
    glNormal3f(Cos(th), Sin(th), 0);
    glVertex3f(Cos(th), Sin(th), 1);
    glVertex3f(Cos(th), Sin(th), 0);
  }
  glEnd();
  glPopMatrix();
}

/*
 *  Draw a rectangle for floor
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void Floor(double x, double y, double z,
  double dx, double dy, double dz,
  double th, int text) {
  rep = rep * 4;
  //  Save transformation
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);
  glBindTexture(GL_TEXTURE_2D, texture[text]);

  glBegin(GL_QUADS);
  glNormal3f(0, +1, 0);
  glTexCoord2f(0, 0);
  glVertex3f(-1, 0, +1);
  glTexCoord2f(rep, 0);
  glVertex3f(+1, 0, +1);
  glTexCoord2f(rep, rep);
  glVertex3f(+1, 0, -1);
  glTexCoord2f(0, rep);
  glVertex3f(-1, 0, -1);
  glEnd();
  //  Undo transformations
  glPopMatrix();
  rep = 1;
}

/*
 *  Draw a sphere ()
 *     at (x,y,z)
 *     radius (r)
 *     texture (text)
 *     inspiration for texture mapping onto sphere drawn from
 *     https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluSphere.xml
 */
static void sphere(double x, double y, double z, double r, int text) {

  glPushMatrix();
  glTranslated(x, y, z);
  glScaled(r, r, r);
  glRotatef(90.0, 1.0, 0.0, 0.0);
  glBindTexture(GL_TEXTURE_2D, texture[text]);

  //create new quad
  GLUquadric * quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_FILL);
  gluQuadricNormals(quadric, GLU_SMOOTH);
  gluQuadricTexture(quadric, GL_TRUE);
  //use new quad to defind normals and texture
  gluSphere(quadric, 1.0, 10, 10);
  //draw sphere quth quad
  gluDeleteQuadric(quadric);
  //delete and pop
  glPopMatrix();
}

/*
 *  Draw a ball of light
 *     at (x,y,z)
 *     radius (r)
 *     borrowed from class
 */
static void lightBall(double x, double y, double z, double r) {
  //  Save transformation
  glPushMatrix();
  //  Offset, scale and rotate
  glTranslated(x, y, z);
  glScaled(r, r, r);
  //  White ball with yellow specular
  float yellow[] = {
    1.0,
    1.0,
    0.0,
    1.0
  };
  float Emission[] = {
    0.0,
    0.0,
    0.01 * emission,
    1.0
  };
  glColor3f(1, 1, 1);
  glMaterialf(GL_FRONT, GL_SHININESS, shiny);
  glMaterialfv(GL_FRONT, GL_SPECULAR, yellow);
  glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
  //  Bands of latitude
  for (int ph = -90; ph < 90; ph += inc) {
    glBegin(GL_QUAD_STRIP);
    for (int th = 0; th <= 360; th += 2 * inc) {
      Vertex(th, ph);
      Vertex(th, ph + inc);
    }
    glEnd();
  }
  //  Undo transofrmations
  glPopMatrix();
}

/*
 *  Draw a Control stick (sphere + cylinder)
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated on z and x axis
 *     rotated together when moved
 */
static void ControlStick(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(controlAngleZ, 1, 0, 0);
  glRotated(controlAngleX, 0, 0, 1);
  glScaled(dx, dy, dz);

  Cylinder(0, .2, 0, .10, .25, .10, 1);
  sphere(0, .3, 0, .05, 7);

  glPopMatrix();
}

/*
 *  Draw a Tedy part (cylinder + sphere + cylinder)
 *     at (x,y,z) 
 *     dimensions (dx,dy,dz)
 *     rotated together when moved
 *     rotated around x,y and z axis for easy building
 */
static void teddyPart(double x, double y, double z, double dx, double dy, double dz, double ry, double rx, double rz) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(rx, 1, 0, 0);
  glRotated(ry, 1, 0, 0);
  glRotated(rz, 0, 0, 1);
  glScaled(dx, dy, dz);

  Cylinder(0, -.5, 0, .5, .7, .5, 9);
  sphere(0, .5, 0, .26, 9);
  sphere(0, -.5, 0, .26, 9);

  glPopMatrix();
}

/*
 *  Draw a train
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated around y axis
 *     made of Boxes() CappedCylinder() and sphere()
 */
static void train(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);

  //TrainCar 1
  Box(.25, 0, 0, .25, .15, .15, 0, 0, 7);

  //TrainCar 1 Z+ wheels
  CappedCylinder(.10, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);
  CappedCylinder(.40, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);

  //TrainCar 1 Z- wheels
  CappedCylinder(.10, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);
  CappedCylinder(.40, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);

  //TrainCar 1 Connector
  Box(0, 0, 0, .10, .05, .05, 0, 0, 13);

  //TrainCar 2 
  Box(-.35, 0, 0, .25, .15, .15, 0, 0, 12);

  //TrainCar 2 Z+ wheels
  CappedCylinder(-.50, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);
  CappedCylinder(-.20, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);

  //TrainCar 2 Z- wheels
  CappedCylinder(-.50, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);
  CappedCylinder(-.20, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);

  //TrainCar 2 Connector
  Box(-.60, 0, 0, .10, .05, .05, 0, 0, 13);

  //Caboose
  Box(-1.10, .1, 0, .4, .25, .15, 0, 0, 7);

  //Caboose 2 Z+ wheels
  CappedCylinder(-1.4, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);
  CappedCylinder(-1.15, -.15, (.25 / 2), .10, .10, .10, 0, 0, 14);
  CappedCylinder(-.85, -.12, (.25 / 2), .14, .14, .10, 0, 0, 14);

  //Caboose 2 Z- wheels
  CappedCylinder(-1.4, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);
  CappedCylinder(-1.15, -.15, -(.25 / 2), .10, .10, .10, 180, 0, 14);
  CappedCylinder(-.85, -.12, -(.25 / 2), .14, .14, .10, 180, 0, 14);

  //front light
  sphere(-1.5, .2, 0, .13, 8);

  //Smoke stack
  CappedCylinder(-1.4, .60, 0, .08, .08, .25, 0, 90, 12);
  CappedCylinder(-1.4, .60, 0, .12, .12, .03, 0, -90, 6);

  //ConductorBox
  Box(-.95, .50, 0, .25, .15, .15, 0, 0, 6);
  Box(-.95, .65, 0, .30, .03, .20, 0, 0, 12);

  Box(-.85, .50, 0, .08, .08, .18, 0, 0, 11);
  Box(-1.05, .50, 0, .08, .08, .18, 0, 0, 11);

  glPopMatrix();
}

/*
 *  Draw a Tedybear
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated together around y axis
 *     made of teddyPart() and sphere()
 */
static void teddyBear(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);

  //Teddy legs
  teddyPart(-.5, 0, .33, 1.5, 1, 1, 90, 0, 0);
  teddyPart(.5, 0, .33, 1.5, 1, 1, 90, 0, 0);

  //Teddy Body
  sphere(0, .65, -.75, 1, 9);

  //Teddy arms
  teddyPart(-.8, .9, 0, 1, 1, 1, 90, 0, 0);
  teddyPart(.8, .9, 0, 1, 1, 1, 90, 0, 0);

  //Teddy head
  sphere(0, 1.9, -.65, .75, 9);

  //Teddy Ears
  sphere(-.75, 2.25, -.65, .35, 9);
  sphere(.75, 2.25, -.65, .35, 9);

  //Teddy Ears
  sphere(-.75, 2.25, -.65, .35, 9);
  sphere(.75, 2.25, -.65, .35, 9);

  //Teddy Muzzle
  sphere(0, 1.8, -.20, .40, 10);
  sphere(0, 1.85, 0.05, .17, 11);

  //Teddy Eyes
  sphere(-.35, 2.2, -.20, .17, 11);
  sphere(.35, 2.2, -.20, .17, 11);

  glPopMatrix();
}

/*
 *  Draw a robot toy
 *    at (x,y,z)
 *    dimensions(dx,dy,dz)
 *    rotated around y axis
 *    made of cappedCylinder() and Box()
 */
static void robotToy(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);

  //Head
  Box(0, 1, 0, .10, .10, .10, 0, 0, 1);
  //Eyes
  CappedCylinder(-.045, 1.04, 0, .035, .035, .11, 0, 0, 11);
  CappedCylinder(.045, 1.04, 0, .035, .035, .11, 0, 0, 11);
  //Mouth
  Box(0, .95, .05, .08, .02, .08, 0, 0, 15);
  //Siren
  CappedCylinder(0, 1.05, 0, .035, .035, .11, 0, -90, 6);
  //Ears
  CappedCylinder(0, 1, 0, .05, .05, .13, 90, 0, 11);
  CappedCylinder(0, 1, 0, .05, .05, .13, -90, 0, 11);

  //Neck
  CappedCylinder(0, .90, 0, .09, .09, .13, 0, 90, 11);

  //Body
  Box(0, .69, 0, .15, .18, .12, 0, 0, 1);
  CappedCylinder(0, .60, 0, .15, .12, .13, 0, 90, 11);

  //Legs
  Box(-.09, .30, 0, .06, .18, .06, 0, 0, 1);
  Box(.09, .30, 0, .06, .18, .06, 0, 0, 1);

  //Feet
  Box(-.09, .15, 0, .08, .05, .10, 0, 0, 12);
  Box(.09, .15, 0, .08, .05, .10, 0, 0, 12);

  //Shoulders
  CappedCylinder(0, .78, 0, .08, .08, .18, 90, 0, 11);
  CappedCylinder(0, .78, 0, .08, .08, .18, -90, 0, 11);

  //Arms
  Box(-.23, .66, .07, .05, .18, .05, -30, 0, 1);
  Box(.23, .65, 0, .05, .18, .05, 0, 0, 1);

  glPopMatrix();
}

/*
 *  Draw simple button with global for pushed or not
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 */
static void Button(double x, double y, double z, double dx, double dy, double dz) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(0, 1, 0, 0);
  glRotated(0, 0, 0, 1);
  glScaled(dx, dy, dz);

  sphere(0, buttonHeight, 0, .05, 6);

  glPopMatrix();
}

/*
 *  Draw a claw
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     takes global (clawYpos) for angle that claw is open
 *    
 */
static void Claw(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glScaled(dx, dy, dz);

  //Verticle tube
  Cylinder(0, (2 * (.30 - clawYpos)), 0, .15, clawYpos, .15, 11);

  //flat x direction arms
  Box(-.05, (2 * (.30 - clawYpos)), 0, .05, .01, .01, 0, 0, 8);
  Box(.05, (2 * (.30 - clawYpos)), 0, .05, .01, .01, 0, 0, 8);

  //verticle x direction arms
  Box(.10 - (clawClosedAngle * .0015), -.10 + (2 * (.30 - clawYpos)), 0, .01, .1, .01, 0, -clawClosedAngle, 8);
  Box(-.10 + (clawClosedAngle * .0015), -.10 + (2 * (.30 - clawYpos)), 0, .01, .1, .01, 0, clawClosedAngle, 8);

  //flat z direction arms
  Box(0, (2 * (.30 - clawYpos)), -.05, .01, .01, .05, 0, 0, 8);
  Box(0, (2 * (.30 - clawYpos)), .05, .01, .01, .05, 0, 0, 8);

  //verticle z direction arms
  Box(0, -.10 + (2 * (.30 - clawYpos)), .10 - (clawClosedAngle * .0015), .01, .1, .01, clawClosedAngle, 0, 8);
  Box(0, -.10 + (2 * (.30 - clawYpos)), -.10 + (clawClosedAngle * .0015), .01, .1, .01, -clawClosedAngle, 0, 8);

  glPopMatrix();
}

/*
 *  Draw arcades front pannel
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated around y axis
 *     made with button*() and ControlStick() and Box()
 */
static void frontPannel(double x, double y, double z, double dx, double dy, double dz, double th) {
  glPushMatrix();
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);

  Box(0, 0, 0, .2, .2, .10, 0, 0, 2);
  ControlStick(-.10, 0, 0, 1, 1, 1, 0);
  Button(.10, 0, 0, 1, 1, 1);

  glPopMatrix();
}

/*
 *  Draw a Crane Cabnet
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 *     calls smaller parts to create entire cabnet (without toys)
 */
static void CraneCabnet(double x, double y, double z,
  double dx, double dy, double dz,
  double th) {
  //  Set specular color to white
  float white[] = {
    1,
    1,
    1,
    1
  };
  float black[] = {
    0,
    0,
    0,
    1
  };
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
  //  Save transformation
  glPushMatrix();
  //  Offset, scale and rotate
  glTranslated(x, y, z);
  glRotated(th, 0, 1, 0);
  glScaled(dx, dy, dz);

  //bottom
  Box(0, 0, 0, .5, .5, .5, 0, 0, 1);

  //top
  Box(0, 1.45, 0, .5, .10, .5, 0, 0, 1);

  //leftside
  Box(-.5, .5, 0, .05, 1, .49, 0, 0, 1);

  //backside
  Box(0, .5, -.5, .49, 1, .05, 0, 0, 1);

  //Front pannel
  frontPannel(0, .35, .55, 1, 1, 1, 0);

  //Claw
  Claw(clawXpos, 1.25, clawZpos, 1, 1, 1, 0);

  //  Undo transofrmations
  glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display() {
  //  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  Enable Z-buffering in OpenGL
  glEnable(GL_DEPTH_TEST);
  //  Undo previous transformations
  glLoadIdentity();
  //  Perspective - set eye position
  if (mode) {
    double Ex = -2 * dim * Sin(th) * Cos(ph);
    double Ey = +2 * dim * Sin(ph);
    double Ez = +2 * dim * Cos(th) * Cos(ph);
    gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
  }
  //  Orthogonal - set world orientation
  else {
    glRotatef(ph, 1, 0, 0);
    glRotatef(th, 0, 1, 0);
  }

  //  Flat or smooth shading
  glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

  //  Translate intensity to color vectors
  float Ambient[] = {
    0.01 * ambient,
    0.01 * ambient,
    0.01 * ambient,
    1.0
  };
  float Diffuse[] = {
    0.01 * diffuse,
    0.01 * diffuse,
    0.01 * diffuse,
    1.0
  };
  float Specular[] = {
    0.01 * specular,
    0.01 * specular,
    0.01 * specular,
    1.0
  };
  //  Light position
  float Position[] = {
    distance * Cos(zh),
    ylight,
    distance * Sin(zh),
    1.0
  };
  //  Draw light position as ball (still no lighting here)
  glColor3f(1, 1, 1);
  lightBall(Position[0], Position[1], Position[2], 0.1);
  //  OpenGL should normalize normal vectors
  glEnable(GL_NORMALIZE);
  //  Enable lighting
  glEnable(GL_LIGHTING);
  //  Location of viewer for specular calculations
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, local);
  //  glColor sets ambient and diffuse color materials
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  //  Enable light 0
  glEnable(GL_LIGHT0);

  //  Set ambient, diffuse, specular components and position of light 0
  glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
  glLightfv(GL_LIGHT0, GL_POSITION, Position);

  // Enable textures
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  //FrontPannel
  if (obj == 0) {
    // Three modes for viewing while in game mode

    Sky(3.5 * dim);

    glEnable(GL_TEXTURE_2D);

    Floor(0, -.75, 0, 5, 5, 5, 0, 3);

    CraneCabnet(0, -.30, 0, 1, 1, 1, 0);

    train(trainXpos + .16, trainYpos, trainZpos - .10, .2, .2, .2, 30);
    teddyBear(bearXpos, bearYpos, bearZpos + .05, .1, .1, .1, 0);
    robotToy(robotXpos, robotYpos, robotZpos, .32, .32, .32, -30);

    glDisable(GL_TEXTURE_2D);
  }
  // Cabnet
  else if (obj == 1) {
    glEnable(GL_TEXTURE_2D);
    CraneCabnet(0, 0, 0, .75, .75, .75, 0);
    glDisable(GL_TEXTURE_2D);
  }
  //Box
  else if (obj == 2) {
    glEnable(GL_TEXTURE_2D);
    robotToy(0, 0, 0, 1, 1, 1, 0);
    glDisable(GL_TEXTURE_2D);
  } else if (obj == 3) {
    glEnable(GL_TEXTURE_2D);
    train(1, 0, 0, 1, 1, 1, 1);
    glDisable(GL_TEXTURE_2D);
  }
  // Teddy Bear
  else if (obj == 4) {
    glEnable(GL_TEXTURE_2D);
    teddyBear(0, -1.2, 0, 1, 1, 1, 0);
    glDisable(GL_TEXTURE_2D);
  }
  //Sphere
  else if (obj == 5) {
    glEnable(GL_TEXTURE_2D);
    sphere(0, 0, 0, 1, 6);
    glDisable(GL_TEXTURE_2D);
  }

  //  Draw axes - no lighting from here on
  glColor3f(1, 1, 1);
  if (obj != 0) {
    const double len = 2.0; //  Length of axes
    glBegin(GL_LINES);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(len, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, len, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, len);
    glEnd();
  }

  //  Display parameters
  if (obj != 0) {
    glWindowPos2i(5, 5);
    Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Move=%d",
      th, ph, dim, fov, mode ? "Perpective" : "Orthogonal", move);
  }

  // Display score if in game mode
  else if (obj == 0) {
    glWindowPos2i(5, 5);
    //Print("TimeElasped =  %d Claw X = %f claw Z = %f    train X = %f train Z = %f",TimeElaspedMili,clawXpos,clawZpos,trainXpos,trainZpos);
    Print("WINS=%d    MISSES=%d ", wins, misses);
  }

  //  Render the scene and make it visible
  ErrCheck("display");
  glFlush();
  glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle() {
  //  Elapsed time in seconds
  double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  zh = fmod(90 * t, 360.0);
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key, int x, int y) {
  //  Right arrow key - increase angle unless in game mode
  if (key == GLUT_KEY_RIGHT) {

    th -= 5;
  }

  //  Left arrow key - decrease angle unless in game mode
  else if (key == GLUT_KEY_LEFT) {
    th += 5;
  }
  //  Up arrow key - increase elevation by 5 degrees unless in game mode
  else if (key == GLUT_KEY_UP) {
    ph += 5;
  }
  //  Down arrow key - decrease elevation by 5 degrees unless in game mode
  else if (key == GLUT_KEY_DOWN) {
    ph -= 5;
  }
  //  PageUp key - increase dim
  else if (key == GLUT_KEY_PAGE_DOWN)
    dim += 0.1;
  //  PageDown key - decrease dim
  else if (key == GLUT_KEY_PAGE_UP && dim > 1)
    dim -= 0.1;
  //  Keep angles to +/-360 degrees

  th %= 360;
  ph %= 360;
  //  Update projection
  Project(mode ? fov : 0, asp, dim);
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}

/*
 *  Returns random number between two floats
 *    Used for random bear pos
 *    Function borrowed from https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats/5289624
 */
float RandomFloat(float min, float max) {
  float random = ((float) rand()) / (float) RAND_MAX;
  float range = max - min;
  return (random * range) + min;
}

/*  Called when e is pressed and claw must be moved
 *    Checks state of claw then acts accordingly
 *    Moves done with loops so they can be animated easily
 *    Keeps track of wins and misses
 */
void lowerClaw() {
  //IsClawOut==0 means claw is up so lower on 'e' press
  if (IsClawOut == 0) {
    int x = 0;
    buttonHeight = .18;
    while (x < 75) {
      x = x + 1;
      clawYpos = clawYpos + .003;
      usleep(10000);
      display();
    }
    x = 0;
    while (x < 60) {
      x = x + 1;
      clawClosedAngle = clawClosedAngle + .5;
      usleep(10000);
      display();
    }
    IsClawOut = 1;
  }
  //Claw is out so check if toy is in range
  else if (IsClawOut == 1) {
    buttonHeight = .2;
    int x = 0;
    //If bear is in range pull up
    if ((clawXpos >= bearXpos - .06 && clawXpos <= bearXpos + .06) && (clawZpos >= bearZpos - .06 && clawZpos <= bearZpos + .06)) {
      while (x < 75) {
        x = x + 1;
        clawYpos = clawYpos - .003;
        bearYpos = bearYpos + .006;
        usleep(10000);
        display();
      }
      IsClawOut = 3;
      x = 0;
    }
    //Else if train is in range pull up
    else if ((clawXpos >= trainXpos - .09 && clawXpos <= trainXpos + .09) && (clawZpos >= trainZpos - .09 && clawZpos <= trainZpos + .09)) {
      while (x < 75) {
        x = x + 1;
        clawYpos = clawYpos - .003;
        trainYpos = trainYpos + .006;
        usleep(10000);
        display();
      }
      IsClawOut = 3;
      x = 0;
    }
    //Else if robot is in range pull up
    else if ((clawXpos >= robotXpos - .07 && clawXpos <= robotXpos + .07) && (clawZpos >= robotZpos - .07 && clawZpos <= robotZpos + .07)) {
      while (x < 75) {
        x = x + 1;
        clawYpos = clawYpos - .003;
        robotYpos = robotYpos + .006;
        usleep(10000);
        display();
      }
      IsClawOut = 3;
      x = 0;
    } else {
      while (x < 75) {
        x = x + 1;
        clawYpos = clawYpos - .003;
        usleep(10000);
        display();
      }
      x = 0;
      while (x < 60) {
        x = x + 1;
        clawClosedAngle = clawClosedAngle - .5;
        usleep(10000);
        display();
      }
      misses = misses + 1;
      IsClawOut = 0;
    }
  }
  //If has been gotten randomly generate new positions for toys and reset claw
  else if (IsClawOut == 3) {
    // Reset states after win
    wins = wins + 1;
    clawZpos = 0.0;
    clawXpos = 0.0;
    clawYpos = .30;

    clawClosedAngle = 0;

    //makes sure no overlap between objects
    bearXpos = RandomFloat(.05, .35);
    bearYpos = .225;
    bearZpos = RandomFloat(0, .35);

    trainXpos = RandomFloat(-.25, -.05);
    trainYpos = .25;
    trainZpos = RandomFloat(0, .30);

    robotXpos = RandomFloat(-.30, .30);
    robotYpos = .18;
    robotZpos = RandomFloat(-.25, -.05);

    IsClawOut = 0;
  }
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch, int x, int y) {
  //  Exit on ESC
  if (ch == 27)
    exit(0);
  //  Switch projection mode
  else if (ch == 'p' || ch == 'P')
    mode = 1 - mode;
  //  Toggle light movement
  else if (ch == 'm' || ch == 'M')
    move = 1 - move;
  //  Change field of view angle
  else if (ch == '-' && ch > 1)
    fov--;
  else if (ch == '+' && ch < 179)
    fov++;
  //  Switch scene/object
  else if (ch == 'o') {
    obj = obj + 1;
    if (obj == 6) {
      obj = 0;
    }
  }
  //controls for claw game
  else if (ch == 'w' && IsClawOut != 1) {
    controlAngleZ = 0;
    controlAngleX = 0;
    if (clawZpos >= -.30) {
      clawZpos += -.03;
    }
    if (controlAngleZ >= -15) {
      controlAngleZ += -10;
    }
  } else if (ch == 's' && IsClawOut != 1) {
    controlAngleZ = 0;
    controlAngleX = 0;
    if (clawZpos <= .35) {
      clawZpos += .03;
    }
    if (controlAngleZ <= 15) {
      controlAngleZ += 10;
    }
  } else if (ch == 'd' && IsClawOut != 1) {
    controlAngleZ = 0;
    controlAngleX = 0;
    if (clawXpos <= .35) {
      clawXpos += .03;
    }
    if (controlAngleX >= -15) {
      controlAngleX += -10;
    }
  } else if (ch == 'a' && IsClawOut != 1) {
    controlAngleZ = 0;
    controlAngleX = 0;
    if (clawXpos >= -.30) {
      clawXpos += -.03;
    }
    if (controlAngleX <= 15) {
      controlAngleX += 10;
    }
  } else if (ch == 'e') {

    controlAngleZ = 0;
    controlAngleX = 0;
    lowerClaw();
  } else if (ch == 'l') {
    if (clawClosedAngle < 30) {
      clawClosedAngle = clawClosedAngle + 1;
    }
  }

  //  Translate shininess power to value (-1 => 0)
  shiny = shininess < 0 ? 0 : pow(2.0, shininess);
  //  Reproject
  Project(mode ? fov : 0, asp, dim);
  //  Animate if requested
  glutIdleFunc(move ? idle : NULL);
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 *    Borrowed from class
 */
void reshape(int width, int height) {
  //  Ratio of the width to the height of the window
  asp = (height > 0) ? (double) width / height : 1;
  //  Set the viewport to the entire window
  glViewport(0, 0, RES * width, RES * height);
  //  Set projection
  Project(mode ? fov : 0, asp, dim);
}

/*
 *  Start up GLUT and tell it what to do
 *    Some of this function is borrowed from class
 */
int main(int argc, char * argv[]) {
  //  Initialize GLUT
  glutInit( & argc, argv);
  //  Request double buffered, true color window with Z buffering at 600x600
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Leo Sipowicz-Project");
  #ifdef USEGLEW
  //  Initialize GLEW
  if (glewInit() != GLEW_OK)
    Fatal("Error initializing GLEW\n");
  #endif
  //  Set callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(special);
  glutKeyboardFunc(key);

  sky[0] = LoadTexBMP("celing.bmp");
  sky[1] = LoadTexBMP("celing.bmp");

  texture[0] = LoadTexBMP("block.bmp");
  texture[1] = LoadTexBMP("metal.bmp");
  texture[2] = LoadTexBMP("colorful.bmp");
  texture[3] = LoadTexBMP("carpet.bmp");
  texture[4] = LoadTexBMP("bark.bmp");
  texture[5] = LoadTexBMP("leaves.bmp");
  texture[6] = LoadTexBMP("redPlastic.bmp");
  texture[7] = LoadTexBMP("bluePlastic.bmp");
  texture[8] = LoadTexBMP("yellowPlastic.bmp");
  texture[9] = LoadTexBMP("fur.bmp");
  texture[10] = LoadTexBMP("lightFur.bmp");
  texture[11] = LoadTexBMP("blackPlastic.bmp");
  texture[12] = LoadTexBMP("lightBluePlastic.bmp");
  texture[13] = LoadTexBMP("greenPlastic.bmp");
  texture[14] = LoadTexBMP("brownPlastic.bmp");
  texture[15] = LoadTexBMP("whitePlastic.bmp");

  glutIdleFunc(idle);
  //  Pass control to GLUT so it can interact with the user
  ErrCheck("init");
  glutMainLoop();
  return 0;
}