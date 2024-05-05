#include <GL/glut.h>
#include <math.h>

// Window size constants
const int WINDOW_WIDTH = 1900;
const int WINDOW_HEIGHT = 980;

// Global variables for animation
float containerPosY = 700.0f; // Initial Y position of the container
float parachuteSize = 50.0f; // Size of the parachute
float containerSize = 50.0f; // Size of the container
float containerSpeed = 1.0f; // Falling speed of the container
float swingAmplitude = 15.0f; // Amplitude of swinging motion
float angle = 0.0f; // Angle for swinging motion
float occupationTrianglePosition = 0.0f;
int counter = 0;

void setupProjectionMatrix();

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

void drawFallingContainerWithParachute(float i, float width, float height) {
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
    glColor3f(0.0f, 1.0f, 0.0f); // Set color to orange

    // Draw head
    glLineWidth(1.5);
    drawCircle(x, y + 0.08f, 0.03f, 20);

    glColor3f(1.0f, 0.0f, 0.0f); // Set color to orange
    //draw hat
    drawTriangle(x-0.024, y+0.07+occupationTrianglePosition, 0.05, 0.05, 2);

    glColor3f(0.0f, 0.0f, 0.0f); // Set color to white
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
    glColor3f(0,0,1);
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

    // Drawing the container with parachute
    for (float i = 100; i < 400; i += 100) {
        drawFallingContainerWithParachute(i, 0.9, 0.9);
    }

    // Drawing the airplane
    drawAirplane(WINDOW_WIDTH / 20, WINDOW_HEIGHT - 100, 100, 100);

    glPushMatrix();
    glTranslatef(800, 295, 0); //safe house
    glScalef(300, 300, 1.0f); 
    drawHouse(-0.85f, -0.9, 0.9f, 0.9f); 
    glPopMatrix();

    glPushMatrix();
    glTranslatef(750, 115, 0); // splitter
    glScalef(600, 600, 1.0f); 
    drawPlayerRectangleHat(0,0); 
    glPopMatrix();
    
    for(int i=0; i<3; i++){
        glPushMatrix();
        glTranslatef(80+(i*100), 115, 0); // collector
        glScalef(600, 600, 1.0f); 
        drawPlayerTriangleHat(0,0);
        glPopMatrix(); 
    }
    
    glPushMatrix();//flag
    drawFlag(1333, 270, 100, 60); 
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

    glPushMatrix();//distributor
    glTranslatef(1000, 115, 0); 
    glScalef(600, 600, 1.0f); 
    drawPlayer(0,0); 
    glPopMatrix();

    glPushMatrix();//occupation
    glTranslatef(1683, 510, 0);  
    glScalef(600, 600, 1.0f);
    drawOccupation(0,0); 
    glPopMatrix();

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

    // Update animation state
    if (containerPosY >= 50) {
        angle += 0.05f;
        containerPosY -= containerSpeed;
    }

}

// Main function
int main(int argc, char** argv) {
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
