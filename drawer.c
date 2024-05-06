#include "headers.h"

// Window size constants
const int WINDOW_WIDTH = 1900;
const int WINDOW_HEIGHT = 980;
int drawer_queue;

#define PI 3.14159265358979323846

// Global variables for animation
// float containerPosY = 700.0f; // Initial Y position of the container
float parachuteSize = 50.0f; // Size of the parachute
float containerSize = 50.0f; // Size of the container
float containerSpeed = 1.0f; // Falling speed of the container
float swingAmplitude = 15.0f; // Amplitude of swinging motion
float angle = 0.0f; // Angle for swinging motion
float occupationTrianglePosition = 0.0f;
int counter = 0;

GUI_Plane planes[100];
GUI_Drop drops[100];
GUI_Collector collectors[100];
GUI_Splitter splitters[100];
GUI_Distributor distributors[100];
GUI_Family families[100];

int sorter_fam_index;
int sorter_fam_rate;
int sorter_fam_bags;

int num_planes;
int num_drops;
int num_collectors;
int num_splitters;
int num_distributors;
int num_families;

int containers_in_safe_house = 0;
int kg_bags_in_safe_house = 0;
int plane_collisions = 0;
int drops_on_ground = 0;
int destroyed_drops = 0;
int killed_collectors = 0;
int killed_distributors = 0;

void setupProjectionMatrix();
void read_from_queue();

void drawContainer(float size) {
    glColor3f(0.8f, 0.3f, 0.3f); // Red color for the container
    glBegin(GL_QUADS);
    glVertex2f(-size / 2, -size / 2);
    glVertex2f(size / 2, -size / 2);
    glVertex2f(size / 2, size / 2);
    glVertex2f(-size / 2, size / 2);
    glEnd();
}

void drawParachute(float size) {
    glColor3f(0.5f, 0.5f, 0.5f); // Gray color for the parachute
    glBegin(GL_TRIANGLE_FAN); // Draw a circle as a simple parachute
    for (int i = 0; i <= 360; i++) {
        float degInRad = i * 3.14159 / 180;
        glVertex2f(cos(degInRad) * size, sin(degInRad) * size);
    }
    glEnd();
}

void drawRope() {
    glColor3f(0.3f, 0.3f, 0.3f); // Dark gray color for the rope
    glBegin(GL_LINES); // Draw a line segment
    glVertex2f(0.0f, containerSize / 2); // Starting point at the top center of the container
    glVertex2f(0.0f, parachuteSize); // Ending point at the bottom center of the parachute
    glEnd();
}

void drawFallingContainerWithParachute(float i, float width, float height, float containerPosY) {
    glPushMatrix();
    float swingX = swingAmplitude * sin(angle); // Calculate swinging motion
    glTranslatef(i + swingX, containerPosY, 0.0f); // Apply translation for swing and initial position
    glScalef(width, height, 1.0f); // Scale to the specified size

    // Draw rope
    drawRope();

    // Draw parachute above the container
    glPushMatrix();
    glTranslatef(0.0f, parachuteSize + containerSize, 0.0f);
    drawParachute(parachuteSize);
    glPopMatrix();

    // Draw container
    drawContainer(containerSize);

    glPopMatrix();
}

void drawAirplane(float x, float y, float width, float height) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Scale the airplane to the specified size
    glScalef(width, height, 1.0f);

    // Draw the head (front of the plane)
    glColor3f(0.7f, 0.7f, 0.7f); // Gray color for fuselage

    glBegin(GL_POLYGON);
    for(int i = 0; i < 360; i += 10){
        glVertex2f(cos(i * 3.14159265 / 180) * 0.6 + 0.6, sin(i * 3.14159265 / 180) * 0.1);
    }
    glEnd();

    // Draw the fuselage (body of the plane)
    glColor3f(0.7f, 0.7f, 0.7f); // Gray color for fuselage
    glBegin(GL_QUADS);
    glVertex3f(-0.6f, -0.1f, 0.0f); // Rear left
    glVertex3f(0.6f, -0.1f, 0.0f);  // Front left
    glVertex3f(0.6f, 0.1f, 0.0f);   // Front right
    glVertex3f(-0.6f, 0.1f, 0.0f);  // Rear right
    glEnd();

    // Draw the right wing at the center, pointing downward and outward
    glColor3f(0.5f, 0.5f, 0.5f); // Darker shade for wings
    glBegin(GL_TRIANGLES);
    glVertex3f(0.3f, -0.1f, 0.0f); // Base at the fuselage center
    glVertex3f(-0.7f, -0.5f, 0.0f); // Tip, pointing downward and outward
    glVertex3f(0.0f, 0.2f, 0.0f); // Connection point
    glEnd();

    // Draw the left wing at the center, pointing downward and outward to the left
    glBegin(GL_TRIANGLES);
    glVertex3f(0.3f, 0.1f, 0.0f); // Base at the fuselage center
    glVertex3f(-0.7f, 0.5f, 0.0f); // Tip, pointing downward and outward
    glVertex3f(-0.0f, -0.2f, 0.0f); // Connection point
    glEnd();

    // Draw the tail (with two diagonal parts, shaped like triangles)
    glColor3f(0.5f, 0.5f, 0.5f); // Same shade as wings
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.6f, 0.1f, 0.0f); // Tail fin left
    glVertex3f(-1.0f, 0.3f, 0.0f);
    glVertex3f(-0.6f, -0.1f, 0.0f);

    glVertex3f(-0.6f, -0.1f, 0.0f); // Tail fin right
    glVertex3f(-1.0f, -0.3f, 0.0f);
    glVertex3f(-0.6f, 0.1f, 0.0f);
    glEnd();

    glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for(int ii = 0; ii < num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * ((float)ii / (float)num_segments); // Ensure proper casting
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawTriangle(float x, float y, float width, float height, int flag) {
    if(flag == 1){//upwards
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + height); // Bottom-left vertex
        glVertex2f(x + width, y + height); // Bottom-right vertex
        glVertex2f(x + width / 2, y + height * 2); // Top vertex
        glEnd();
    }

    else if(flag==0){//rightwards
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y); // Bottom-left vertex
        glVertex2f(x, y + height); // Top-left vertex
        glVertex2f(x + width, y + height / 2); // Middle-right vertex
        glEnd();
    }
    else if(flag==2){//downwards
        glBegin(GL_TRIANGLES);
        glVertex2f(x+0.025, y*1.8); // Bottom-left vertex
        glVertex2f(x+0.025-width/1.5, y*1.8 + height); // Top-left vertex
        glVertex2f(x+0.025 + width/1.5, y*1.8 + height); // Middle-right vertex
        glEnd();
    }
    else if(flag==3){//leftwards
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y); // Bottom-left vertex
        glVertex2f(x, y + height); // Top-left vertex
        glVertex2f(x - width, y + height / 2); // Middle-right vertex
        glEnd();
    }
}

void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawRectangleLine(float x, float y, float width, float height) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawPlayer(float x, float y) {
    // Set player color
    glColor3f(0.0f, 0.0f, 1.0f); // Green
    
    glLineWidth(1.5);
    // Draw head
    drawCircle(x, y + 0.08f, 0.03f, 20);

    glColor3f(1.0f, 1.0f, 0.0f); // Set color to orange
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(0.0f, 0.0f, 1.0f); // Green
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    
    glVertex2f(x - 0.05f, y + 0.05f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y + 0.05f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

void drawPlayerTriangleHat(float x, float y) {
    // Set player color
    glColor3f(1.0f, 0.5f, 0.0f); // Set color to orange

    // Draw head
    glLineWidth(1.5);
    drawCircle(x, y + 0.08f, 0.03f, 20);

    //draw hat
    drawRectangle(x-0.045, y+0.115, 0.1, 0.005);
    drawTriangle(x-0.024, y+0.07, 0.05, 0.05, 1);
    glColor3f(0.0f, 0.0f, 0.0f); // Set color to white
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(1.0f, 0.5f, 0.0f); // Set color to orange
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    
    glVertex2f(x - 0.05f, y + 0.05f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y + 0.05f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

void drawPlayerRectangleHat(float x, float y) {
    // Set player color
    glColor3f(0.0f, 0.0f, 0.0f); // Set color to green

    glLineWidth(1.5);
    // Draw head
    drawCircle(x, y + 0.08f, 0.03f, 20);

    //draw hat
    drawRectangle(x-0.045, y+0.115, 0.1, 0.005);
    drawRectangle(x-0.024, y+0.115, 0.05, 0.05);
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(0.0f, 0.0f, 0.0f); // Set color to green
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    
    glVertex2f(x - 0.05f, y + 0.05f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y + 0.05f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

void drawOccupation(float x, float y) {
    // Set player color
    glColor3f(0.0f, 1.0f, 1.0f);

    // Draw head
    glLineWidth(1.5);
    drawCircle(x, y + 0.08f, 0.03f, 20);

    glColor3f(1.0f, 0.0f, 0.0f); // Set color to orange
    //draw hat
    drawTriangle(x-0.024, y+0.07+occupationTrianglePosition, 0.05, 0.05, 2);

    glColor3f(1.0f, 0.0f, 0.0f); // Set color to white
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(0.0f, 1.0f, 0.0f); // Set color to orange
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    
    glVertex2f(x - 0.05f, y + 0.05f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y + 0.05f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

void drawSorter(float x, float y) {
    // Set player color
    glColor3f(0.0f, 1.0f, 0.0f); // Set color to green

    // Draw head
    glLineWidth(1.5);
    drawCircle(x, y + 0.08f, 0.03f, 20);

    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(0.0f, 1.0f, 0.0f); // Set color to orange
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glColor3f(0.0f, 0.0f, 0.0f); // Set color to orange
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    
    glVertex2f(x - 0.05f, y + 0.05f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y + 0.05f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

void drawText(float x, float y, const char *string) {
    glRasterPos2f(x, y);

    // Loop through each character of the string
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *string);
        string++;
    }
}

void drawHouse(float x, float y, float width, float height) {
    glColor3f(1, 1, 0); // Set color to yellow for house body
    drawRectangle(x, y, width, height); // Draw house body

    glColor3f(1, 0, 0); // Set color to red for roof
    drawTriangle(x, y, width, height, 1); // Draw roof

    glColor3f(0, 1, 1); // Set color to cyan for smoke area
    drawRectangle(x, y + height, width - width / 1.1, height - height / 1.1 + 0.15); // Draw smoke area

    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    drawRectangle(x + width / 20, y + height - (0.2 * height), width - width / 1.1, height - height / 1.1); // Draw window area
    drawRectangle(x + width / 1.2, y + height - (0.2 * height), width - width / 1.1, height - height / 1.1); // Draw window area

    // Draw door
    glColor3f(0.5f, 0.35f, 0.05f); // Set color to brown for door
    drawRectangle(x + width / 2 - width / 10, y, width / 5, height * 0.6); // Draw door

    // Draw door handle
    glColor3f(0.5f, 0.5f, 0.5f); // Set color to gray for door handle
    drawCircle(x + width / 2 + width / 20, y + height * 0.3, height * 0.025, 20); // Draw door handle
}

void drawFlag(float x, float y, float width, float height) {
    // Calculate stripe height and triangle width based on total flag height
    float stripeHeight = height / 3;
    float triangleWidth = width / 3;

    // Draw stripes
    glColor3f(0.0, 1.0, 0.0); // Green stripe
    drawRectangle(x + triangleWidth, y, width - triangleWidth, stripeHeight);
    glColor3f(1.0, 1.0, 1.0); // White stripe
    drawRectangle(x + triangleWidth, y + stripeHeight, width - triangleWidth, stripeHeight);
    glColor3f(0.0, 0.0, 0.0); // Black stripe
    drawRectangle(x + triangleWidth, y + 2 * stripeHeight, width - triangleWidth, stripeHeight);


    glColor3f(0.0, 0.0, 0.0);
    drawRectangle(x+width/3,y - height/1.2,0.2,100);

    // Draw red triangle
    glColor3f(1.0, 0.0, 0.0); // Red
    //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawTriangle(x+triangleWidth, y, triangleWidth, height, 0);
}

void drawTent(GLfloat x, GLfloat y, GLfloat width, GLfloat height) {
    glBegin(GL_POLYGON);
    glColor3f(0,.7,.7);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width/3, y+height);
    glVertex2f(x, y);
    glVertex2f(x-width/2, y+height);
    glVertex2f(x-width/2, y+height);
    glVertex2f(x + width/3, y+height);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.7,0.7,0.1);
    glVertex2f(x+width, y);
    glVertex2f(x+width/0.97, y+height/10.5);
    glVertex2f(x+width/0.97, y+height/10.5);
    glVertex2f(x+width, y+height/10);
    glVertex2f(x+width, y+height/10);
    glVertex2f(x+width/1.015, y+height/40);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.7,0.7,0.1);
    glVertex2f(x, y);
    glVertex2f(x + width/50, y+height/10);
    glVertex2f(x + width/50, y+height/10);
    glVertex2f(x + width/20, y+height/10.5);
    glVertex2f(x + width/20, y+height/10.5);
    glVertex2f(x + width/30, y);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.5,0.5,0.5);
    glVertex2f(x-width/2, y+height);
    glVertex2f(x-width, y);
    glVertex2f(x-width, y);
    glVertex2f(x, y);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.7,0.7,0.1);
    glVertex2f(x-width, y);
    glVertex2f(x-width/0.97, y+height/10);
    glVertex2f(x-width/0.97, y+height/10);
    glVertex2f(x-width/1.01, y+height/10.5);
    glVertex2f(x-width/1.01, y+height/10.5);
    glVertex2f(x-width/1.015, y+height/40);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.7,0.7,0.1);
    glVertex2f(x+width/8, y+height/2.5);
    glVertex2f(x+width/2, y+height/2.5);
    glVertex2f(x+width/8, y+height/2.5);
    glVertex2f(x-width/15, y+height/1.5);
    glVertex2f(x+width/2, y+height/2.5);
    glVertex2f(x+width/3.3, y+height/1.5);
    glVertex2f(x+width/3.3, y+height/1.5);
    glVertex2f(x-width/15, y+height/1.5);

    glEnd();
}

void drawMissile(float x, float y, float width, float height){
    glColor3f(0,0,0);
    drawRectangle(x+width,y+height,width*1.1,height/5);
    drawTriangle(x+width*2.05,y+height/1.169,width/2,height/2,3);
    drawTriangle(x+width/1.01,y+height/1.001,width/3,height/5,3);
}

void drawSniper(float x, float y, float width, float height){
    glColor3f(0,0,0);
    drawRectangle(x+width,y+height,width/2,height/15);
    glColor3f(0.6f, 0.3f, 0.1f); // Approximate values for brown
    drawRectangle(x+width*1.5,y+height/1.035,width/2,height/10);
     glColor3f(0,0,0);
    drawRectangle(x+width*1.85,y+height/1.3,width/10,height/1.9);
    drawRectangle(x+width*1.867,y+height*1.29,width/15,height/8);
     glColor3f(0.5,0.5,0.5);
    drawRectangle(x+width*1.75,y+height*1.29,width/3.5,height/15);
    drawRectangle(x+width*2,y+height*1.275,width/10,height/10);
    drawRectangle(x+width*1.71,y+height*1.275,width/10,height/10);
    glColor3f(0.6f, 0.3f, 0.1f); // Approximate values for brown
    drawRectangle(x+width*2,y+height/1.035,width/2,height/20);
    
    glBegin(GL_LINES);
    glVertex2f(x+width*1.8, y+height);
    glVertex2f(x+width*1.8, y+height/1.1);
    glVertex2f(x+width*1.8, y+height/1.1);
    glVertex2f(x+width*1.85, y+height/1.1);
    glVertex2f(x+width*1.8, y+height/1.01);
    glVertex2f(x+width*1.85, y+height/1.1);
    glEnd();
    drawRectangle(x+width*2.5,y+height/1.31,width/10,height/4);

    drawRectangle(x+width*2,y+height/1.035,width/2,height/20);
    
    glBegin(GL_POLYGON);
    glVertex2f(x+width*2, y+height-6.5);
    glVertex2f(x+width*2.09, y+height/1.1);
    glVertex2f(x+width*2.09, y+height/1.1);
    glVertex2f(x+width*2.09, y+height/1.3);
    glVertex2f(x+width*2.09, y+height/1.3);
    glVertex2f(x+width*2.5, y+height/1.2);
    glVertex2f(x+width*2, y+height-6.5);
    glVertex2f(x+width*2.5, y+height-6.5);
    glVertex2f(x+width*2.5, y+height-6.5);
    glVertex2f(x+width*2.5, y+height/1.2);
    glEnd();

}

void drawMountain(float x, float y, float width, float height){
    glBegin(GL_POLYGON);
    glColor3f(0.5f, 0.35f, 0.05f);
    glVertex2f(x, y-height);
    glVertex2f(x-width+100, y+height-200);
    glVertex2f(x-width+100, y+height-200);
    glVertex2f(x-width-10, y-height+90);
    glVertex2f(x-width-10, y-height+90);
    glVertex2f(x-width-10, y-height-180);
    glVertex2f(x-width-10, y-height-180);
    glVertex2f(x, y-height-180);

    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.5f, 0.35f, 0.05f);
    glVertex2f(x-width*1.8, y-height-180);
    glVertex2f(x-width*1.45, y+height-50);

    glVertex2f(x-width*1.45, y+height-50);
    glVertex2f(x-width-10, y-height+90);

    glVertex2f(x-width-10, y-height+90);
    glVertex2f(x-width-10, y-height-180);

    glVertex2f(x-width-10, y-height-180);
    glVertex2f(x-width*1.8, y-height-180);
    
    glEnd();

}

void drawCane() {
    glBegin(GL_POLYGON);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(0.2, -0.8);
        glVertex2f(0.2, 0.2);

        glVertex2f(0.1, -0.8);
        glVertex2f(0.1, 0.2);

        glVertex2f(0.1, -0.8);
        glVertex2f(0.2, -0.8);

        glVertex2f(0.2, 0.2);
        glVertex2f(0.1, 0.2);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(0.25, -0.8);
        glVertex2f(0.05, -0.8);

        glVertex2f(0.25, -0.8);
        glVertex2f(0.25, -0.9);

        glVertex2f(0.05, -0.8);
        glVertex2f(0.05, -0.9);

        glVertex2f(0.25, -0.9);
        glVertex2f(0.05, -0.9);


    glEnd();

    glBegin(GL_POLYGON);
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(-0.29, 0.1);
        glVertex2f(-0.42, 0.1);

        glVertex2f(-0.29 + 0.005, 0.25);
        glVertex2f(-0.42 + 0.005, 0.25);

        glVertex2f(-0.29, 0.1);
        glVertex2f(-0.29 + 0.005, 0.25);

        glVertex2f(-0.42, 0.1);
        glVertex2f(-0.42 + 0.005, 0.25);
    glEnd();

    float x_initial_one;
    float y_initial_one;
    float x_initial_two;
    float y_initial_two;

    glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 180; i++) {
            float angle = PI * i / 180;  // Convert angle to radians
            float x = -0.1 + 0.2 * cos(angle);  // Calculate x coordinate
            float y = 0.2 + 0.2 * sin(angle);  // Calculate y coordinate
            if (i==0){
                x_initial_one = x;
                y_initial_one = y;
            }
            glVertex2f(x, y);
        }
    glEnd();
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 180; i++) {
            float angle = PI * i / 180;  // Convert angle to radians
            float x = -0.1 + 0.3 * cos(angle);  // Calculate x coordinate
            float y = 0.2 + 0.3 * sin(angle);  // Calculate y coordinate
            if (i==0){
                x_initial_two = x;
                y_initial_two = y;
            }
            glVertex2f(x, y);
        }
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(x_initial_one, y_initial_one);
        glVertex2f(x_initial_two, y_initial_two);
    glEnd();

}

void drawPlayerWithCane(float x, float y) {
    // Set player color
    glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

    // Draw head
    drawCircle(x, y + 0.08f, 0.03f, 20);

    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    //draw eyes
    drawCircle(x-0.015, y + 0.09f, 0.006f, 10);//left eye
    drawCircle(x+0.015, y + 0.09f, 0.006f, 10);//right eye
    //draw mouth
    drawRectangle(x-0.021, y+0.065, 0.04, 0.003);

    glColor3f(1.0f, 0.0f, 0.0f); // Set color to green
    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.05); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x - 0.05f, y - 0.03f); // Left arm's lower point
    
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y + 0.01f); // Arms' upper point
    glVertex2f(x + 0.05f, y - 0.03f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to set up the projection matrix
void setupProjectionMatrix() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Function to initialize the OpenGL environment
void initializeOpenGL() {
    glClearColor(1.0, 1.0, 1.0, 1.0); // Set background color to white
    glEnable(GL_DEPTH_TEST); // Enable depth test for 3D rendering
    glDepthFunc(GL_LEQUAL); // Specify the depth test function
    setupProjectionMatrix();
}

// Display function to render the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    read_from_queue();

    for(int i = 0; i<num_planes; i++){// Drawing the airplane

        if (!planes[i].destroyed) {

            drawAirplane((WINDOW_WIDTH / 25)+(i*150), WINDOW_HEIGHT - 100, 70, 70);

            glColor3f(0.8f, 0.3f, 0.3f); // Red color for the container

            char buff[1000];
            sprintf(buff, "Amp: %d", planes[i].amplitude);
            drawText((WINDOW_WIDTH / 25)+(i*150) - 40, WINDOW_HEIGHT - 150, buff);

            sprintf(buff, "num_con: %d", planes[i].num_containers);
            drawText((WINDOW_WIDTH / 25)+(i*150) - 40, WINDOW_HEIGHT - 170, buff);

            sprintf(buff, "refill: %d", planes[i].refilling);
            drawText((WINDOW_WIDTH / 25)+(i*150) - 40, WINDOW_HEIGHT - 190, buff);

            sprintf(buff, "number: %d", planes[i].plane_number);
            drawText((WINDOW_WIDTH / 25)+(i*150) - 40, WINDOW_HEIGHT - 210, buff);
        }
    }

    for (int i = 0; i < num_drops; i++) {
        drawFallingContainerWithParachute( ((i+1) * 80) % 400 + 50, 0.8, 0.8, drops[i].amplitude + 50 );

        char buff[1000];
        glColor3f(0, 0, 0);

        sprintf(buff, "%d", drops[i].amplitude);
        drawText(((i+1) * 80) % 400 + 30, (drops[i].amplitude + 70) + 50, buff);

        sprintf(buff, "%d.KG", drops[i].weight);
        drawText(((i+1) * 80) % 400 + 35, (drops[i].amplitude - 10) + 50, buff);
    }
    
    char buff[1000];

    glPushMatrix();
    glTranslatef(800, 295, 0); //safe house
    glScalef(300, 300, 1.0f); 
    drawHouse(-0.85f, -0.9, 0.9f, 0.9f);
    glPopMatrix();

    glColor3f(0, 0, 0);

    sprintf(buff, "containersNum: %d", containers_in_safe_house);
    drawText(800, 300, buff);

    sprintf(buff, "KG_bags: %d", kg_bags_in_safe_house);
    drawText(800, 330, buff);

    // statistics text
    glColor3f(0, 0, 0);

    sprintf(buff, "Destroyed Drops: %d", destroyed_drops);
    drawText(600, 800, buff);

    sprintf(buff, "Destroyed Planes: %d", plane_collisions);
    drawText(600, 770, buff);

    sprintf(buff, "Killed Collectors: %d", killed_collectors);
    drawText(600, 740, buff);

    sprintf(buff, "Killed Distributors: %d", killed_distributors);
    drawText(600, 710, buff);


    for (int i = 0; i < num_splitters; i++) {
        
        if (splitters[i].swapped)
            continue;

        glPushMatrix();
        glTranslatef(580 + (i*80), 115, 0); // splitter
        glScalef(600, 600, 1.0f);
        drawPlayerRectangleHat(0,0);
        glPopMatrix();

        glColor3f(0, 0, 0);

        char buff[1000];
        sprintf(buff, "%d", splitters[i].pid);
        drawText(565+(i*80) - 20, 10, buff);

        sprintf(buff, "E: %d", splitters[i].energy);
        drawText(580+(i*80) - 20, 80, buff);

        sprintf(buff, "KG: %d", splitters[i].weight);
        drawText(580+(i*80) - 20, 110, buff);
    }
    
    for(int i=0; i<num_collectors; i++){

        if (!collectors[i].killed) {
            glPushMatrix();
            glTranslatef(80+(i*100), 130, 0); // collector
            glScalef(600, 600, 1.0f); 
            drawPlayerTriangleHat(0,0);
            // draw energy bar
            glPopMatrix();

            glColor3f(0, 0, 0);

            char buff[1000];
            sprintf(buff, "%d", collectors[i].pid);
            drawText(70+(i*100) - 20, 10, buff);

            sprintf(buff, "E: %d", collectors[i].energy);
            drawText(80+(i*100) - 20, 90, buff);

            sprintf(buff, "Con: %d", collectors[i].containers);
            drawText(80+(i*100) - 20, 110, buff);
        }
    }

    for (int i = 0; i < num_distributors; i++) {

        if (!distributors[i].killed) {
            glPushMatrix();//distributor
            glTranslatef(900 + (i*80), 115, 0); 
            glScalef(600, 600, 1.0f); 
            drawPlayer(0,0); 
            glPopMatrix();

            glColor3f(0, 0, 0);

            char buff[1000];
            sprintf(buff, "%d", distributors[i].pid);
            drawText(890+(i*80) - 20, 10, buff);

            sprintf(buff, "E: %d", distributors[i].energy);
            drawText(900+(i*80) - 20, 90, buff);

            sprintf(buff, "Bags: %d", distributors[i].bags);
            drawText(900+(i*80) - 20, 110, buff);
        }
    }

    glPushMatrix();//flag
    drawFlag(1433, 270, 100, 60); 
    glPopMatrix();

    glPushMatrix();//tent
    drawTent(1400, 20, 200, 200); 
    glPopMatrix();

    glPushMatrix();//missile
    drawMissile(1420, 500, 100, 100); 
    glPopMatrix();

    glPushMatrix();//mountain
    drawMountain(1900, 350, 150, 150); 
    glPopMatrix();

    glPushMatrix();//sniper
    drawSniper(1370, 410, 100, 100); 
    glPopMatrix();

    glPushMatrix();//occupation
    glTranslatef(1683, 510, 0);  
    glScalef(600, 600, 1.0f);
    drawOccupation(0,0); 
    glPopMatrix();

    glPushMatrix();//sorter
    glTranslatef(1330, 310, 0);
    glScalef(600, 600, 1.0f);
    drawSorter(0,0); 
    glPopMatrix();

    drawText(1300, 380, "Sorter");

    sprintf(buff, "index: %d", sorter_fam_index);
    drawText(1300, 280, buff);

    sprintf(buff, "starve_rate: %d", sorter_fam_rate);
    drawText(1300, 250, buff);

    sprintf(buff, "Bags: %d", sorter_fam_bags);
    drawText(1300, 220, buff);

    GLfloat family_x;
    GLfloat family_y;
    GLfloat cane_x;
    GLfloat cane_y;


    for (int i = 0; i < num_families; i++) {

        if (!families[i].alive)
            continue;

        family_x = 1230 + (i*100);
        family_y = 110;
        cane_x = family_x + 50;
        cane_y = family_y - 27;
        
        glPushMatrix();
        glTranslatef(family_x, family_y, 0);  
        glScalef(600, 600, 1.0f);
        drawPlayerWithCane(0,0); 
        glPopMatrix();

        glPushMatrix();//cane for family
        glTranslatef(cane_x, cane_y, 0);  
        glScalef(70, 70, 1.0f);
        drawCane(); 
        glPopMatrix();

        glColor3f(0, 0, 0);

        char buff[1000];
        sprintf(buff, "Num: %d", families[i].number);
        drawText(family_x - 20, 50, buff);

        sprintf(buff, "Strv: %d", families[i].starvation_rate);
        drawText(family_x - 20, 70, buff);
    }

    if (counter %10 == 0){
        if (occupationTrianglePosition == 0.0f){
        occupationTrianglePosition += 0.01f;
        }
        else{
            occupationTrianglePosition -= 0.01f;
        }
    }
    
    counter ++;

    glutSwapBuffers();
}

// Timer function to animate
void timer(int value) {
    glutPostRedisplay(); // Redraw the scene
    glutTimerFunc(16, timer, 0); // Re-register timer callback
}

int get_zero_amp() {

    for (int i = 0; i < num_drops; i++) {
        if (drops[i].amplitude == 0)
            return i;
    }
    return -1;
}

int find_drop_by_id(int id) {

    for (int i = 0; i < num_drops; i++) {
        if (drops[i].number == id)
            return i;
    }
    return -1;
}

int find_dead_collector() {

    for (int i = 0; i < num_collectors; i++) {
        if (collectors[i].killed)
            return i;
    }
    return -1;
}

int find_collector(pid_t pid) {

    for (int i = 0; i < num_collectors; i++) {
        if (collectors[i].pid == pid)
            return i;
    }
    return -1;
}

int find_splitter(pid_t pid) {

    for (int i = 0; i < num_splitters; i++) {
        if (splitters[i].pid == pid)
            return i;
    }
    return -1;
}

void read_from_queue() {

    struct msqid_ds buf;
    msgctl(drawer_queue, IPC_STAT, &buf);

    for (int i = 0; i < buf.msg_qnum; i++) {

        MESSAGE msg;

        if (msgrcv(drawer_queue, &msg, sizeof(msg), 0, IPC_NOWAIT) == -1) {
            perror("msg error");
            exit(-1);
        }

        int index = 0;

        switch (msg.type)
        {
        case PLANE:
            planes[ msg.data.planes.plane_number ].num_containers = msg.data.planes.num_containers;
            planes[ msg.data.planes.plane_number ].amplitude = msg.data.planes.amplitude;
            planes[ msg.data.planes.plane_number ].refilling = msg.data.planes.refilling;
            planes[ msg.data.planes.plane_number ].plane_number = msg.data.planes.plane_number;
            planes[ msg.data.planes.plane_number ].destroyed = msg.data.planes.destroyed;

            if (msg.data.planes.destroyed)
                plane_collisions++;

            break;
        
        case COLLECTOR:

            collectors[ msg.data.collector.number ].pid = msg.data.collector.pid;
            collectors[ msg.data.collector.number ].number = msg.data.collector.number;
            collectors[ msg.data.collector.number ].energy = msg.data.collector.energy;
            collectors[ msg.data.collector.number ].containers = msg.data.collector.containers;
            collectors[ msg.data.collector.number ].killed = msg.data.collector.killed;

            if (msg.data.collector.killed)
                killed_collectors++;

            if (msg.operation == 2)
                containers_in_safe_house++;
            else if (msg.operation == 1) {
                int drop_on_ground = get_zero_amp();

                // swap...
                drops[drop_on_ground].number = drops[num_drops-1].number;
                drops[drop_on_ground].amplitude = drops[num_drops-1].amplitude;
                drops[drop_on_ground].weight = drops[num_drops-1].weight;

                num_drops--; /* in the sky */
            }
            
            break;

        case SPLITTER:
            index = msg.data.splitter.number;

            if (msg.operation == 1)
                kg_bags_in_safe_house++;

            else if (msg.operation == 2)
                containers_in_safe_house--;

            else if (msg.operation == 3) {
                int dead_collector = msg.data.splitter.number;
                index = find_splitter(msg.data.splitter.pid);

                printf("(DRAWER)---------------------------------- Dead Collector: %d. splitterNum: %d\n", dead_collector, index);
                splitters[ index ].swapped = true;

                printf("Splitter is swapping--------------------------\n");

                if (dead_collector != -1) {
                    // change the pid
                    collectors[dead_collector].pid = splitters[ index ].pid;
                    collectors[dead_collector].energy = splitters[ index ].energy;
                    collectors[dead_collector].killed = false;
                    printf("(DRAWER)----------------------------------\n");

                    for (int i = 0; i < num_splitters; i++) {
                        printf("-----(SSS)------pid: %d, number: %d, swapped: %d\n", splitters[i].pid, splitters[i].number, splitters[i].swapped);
                    }
                    for (int i = 0; i < num_collectors; i++) {
                        printf("-----(CCC)------pid: %d, number: %d, killed: %d\n", collectors[i].pid, collectors[i].number, collectors[i].killed);
                    }
                }
            }
            else if (msg.operation == 4) {
                int dead_distributor = msg.data.splitter.number;
                index = find_splitter(msg.data.splitter.pid);

                printf("(DRAWER)---------------------------------- Dead Distributor: %d, splitterNum: %d\n", dead_distributor, index);
                splitters[ index ].swapped = true;

                if (dead_distributor != -1) {
                    // change the pid
                    distributors[dead_distributor].pid = splitters[ index ].pid;
                    distributors[dead_distributor].energy = splitters[ index ].energy;
                    distributors[dead_distributor].bags = 0;
                    distributors[dead_distributor].killed = false;
                    printf("(DRAWER)----------------------------------\n");

                    for (int i = 0; i < num_splitters; i++) {
                        printf("-----(SSS)------pid: %d, number: %d, swapped: %d\n", splitters[i].pid, splitters[i].number, splitters[i].swapped);
                    }
                    for (int i = 0; i < num_distributors; i++) {
                        printf("-----(DDD)------pid: %d, number: %d, killed: %d\n", distributors[i].pid, distributors[i].number, distributors[i].killed);
                    }
                }
            }
            splitters[ index ].pid = msg.data.splitter.pid;
            splitters[ index ].number = msg.data.splitter.number;
            splitters[ index ].energy = msg.data.splitter.energy;
            splitters[ index ].weight = msg.data.splitter.weight;

            break;

        case DISTRIBUTOR:
            distributors[ msg.data.distributor.number ].pid = msg.data.distributor.pid;
            distributors[ msg.data.distributor.number ].number = msg.data.distributor.number;
            distributors[ msg.data.distributor.number ].energy = msg.data.distributor.energy;
            distributors[ msg.data.distributor.number ].bags = msg.data.distributor.bags;
            distributors[ msg.data.distributor.number ].killed = msg.data.distributor.killed;

            if (msg.data.distributor.killed)
                killed_distributors++;

            if (msg.operation == 1 && kg_bags_in_safe_house > 0)
                kg_bags_in_safe_house--;

            break;

        case FAMILY:
            families[ msg.data.families.number ].number = msg.data.families.number + 1;
            families[ msg.data.families.number ].starvation_rate = msg.data.families.starvation_rate;
            families[ msg.data.families.number ].alive = msg.data.families.alive;

            break;

        case SKY:

            if (msg.operation == 1) {
                index = num_drops;
                num_drops++;
            } else {
                index = find_drop_by_id(msg.data.sky.drop_number);
            }

            if (msg.operation == 2) {
                printf("Drops is totally lost -----------%d--------%d\n", num_drops, drops[index].number);
                drops[index].number = drops[num_drops-1].number;
                drops[index].amplitude = drops[num_drops-1].amplitude;
                drops[index].weight = drops[num_drops-1].weight;
                num_drops--;
                destroyed_drops++;
            } else {
                drops[ index ].amplitude = msg.data.sky.amplitude;
                drops[ index ].weight = msg.data.sky.weight;
                drops[ index ].number = msg.data.sky.drop_number;
            }
            break;

        case SORTER:

            sorter_fam_bags = msg.data.sorter.bags_required;
            sorter_fam_index = msg.data.sorter.worst_fam_index;
            sorter_fam_rate = msg.data.sorter.worst_fam_starve_rate;

            break;
        }
    }
}

// Main function
int main(int argc, char** argv) {

    if (argc < 7) {
        perror("Not enough args (DRAWER)");
        exit(-1);
    }
    drawer_queue = atoi(argv[1]);

    num_planes = atoi(argv[2]);
    num_collectors = atoi(argv[3]);
    num_distributors = atoi(argv[4]);
    num_splitters = atoi(argv[5]);
    num_families = atoi(argv[6]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Interactive OpenGL Scene");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
