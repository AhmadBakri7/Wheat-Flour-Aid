#include "headers.h"

// Constants for window size
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 980;

// Function to draw an airplane with properly positioned wings
void drawAirplane() {
    glPushMatrix();

    // Apply a rotation to achieve the diagonal orientation
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f); // Rotate by 45 degrees for a diagonal look

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

    glPopMatrix(); // Restore matrix state
}

// Display function to render the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
    glLoadIdentity(); // Reset transformations

    drawAirplane(); // Draw the airplane

    glutSwapBuffers(); // Swap buffers for double buffering
}

// Initialization function for OpenGL settings
void initialize() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for 3D effects
}

// Main function to initialize GLUT and start OpenGL
int main(int argc, char **argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Double buffering
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); // Set window size
    glutCreateWindow("OpenGL Airplane with Correct Wing Positioning"); // Create GLUT window

    initialize(); // Initialize OpenGL settings
    glutDisplayFunc(display); // Set display function

    glutMainLoop(); // Enter the GLUT event processing loop

    return 0;
}
