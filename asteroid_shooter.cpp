// asteroid_shooter.cpp
// A simple 2D space shooter game using OpenGL and GLUT

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

// Game structures
struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Ship {
    Point pos;
    float angle;
    Point velocity;
    bool thrusting;
    float size;
};

struct Bullet {
    Point pos;
    Point velocity;
    float life;
};

struct Asteroid {
    Point pos;
    Point velocity;
    float size;
    float rotation;
    int sides;
};

struct Star {
    Point pos;
    float brightness;
    float twinkle;
};

// Game variables
Ship player;
std::vector<Bullet> bullets;
std::vector<Asteroid> asteroids;
std::vector<Star> stars;

int score = 0;
int lives = 3;
bool gameOver = false;
bool gameWon = false;
int level = 1;
float respawnTimer = 0;

// Game constants
const float SHIP_SIZE = 0.02f;
const float BULLET_SPEED = 0.01f;
const float BULLET_LIFE = 100.0f;
const float ASTEROID_MIN_SIZE = 0.03f;
const float ASTEROID_MAX_SIZE = 0.08f;
const int INITIAL_ASTEROIDS = 5;

// FIXED: Properly initialize special keys array
bool specialKeys[256] = { false }; // Track special key states

// Forward declaration
void createAsteroids(int count);

// Initialize game
void initGame() {
    srand(time(0));

    // Initialize player
    player.pos = Point(0, 0);
    player.angle = 0;
    player.velocity = Point(0, 0);
    player.thrusting = false;
    player.size = SHIP_SIZE;

    // Clear vectors
    bullets.clear();
    asteroids.clear();
    stars.clear();

    // Create stars
    for (int i = 0; i < 100; i++) {
        Star star;
        star.pos.x = ((rand() % 2000) - 1000) / 1000.0f;
        star.pos.y = ((rand() % 2000) - 1000) / 1000.0f;
        star.brightness = 0.3f + (rand() % 70) / 100.0f;
        star.twinkle = 0;
        stars.push_back(star);
    }

    // Create asteroids
    createAsteroids(INITIAL_ASTEROIDS + level - 1);

    score = 0;
    lives = 3;
    gameOver = false;
    gameWon = false;
    respawnTimer = 0;
}

void createAsteroids(int count) {
    for (int i = 0; i < count; i++) {
        Asteroid asteroid;

        // Spawn away from player
        do {
            asteroid.pos.x = ((rand() % 2000) - 1000) / 1000.0f;
            asteroid.pos.y = ((rand() % 2000) - 1000) / 1000.0f;
        } while (sqrt(asteroid.pos.x * asteroid.pos.x + asteroid.pos.y * asteroid.pos.y) < 0.3f);

        asteroid.velocity.x = ((rand() % 100) - 50) / 10000.0f;
        asteroid.velocity.y = ((rand() % 100) - 50) / 10000.0f;
        asteroid.size = ASTEROID_MIN_SIZE + (rand() % 100) / 100.0f * (ASTEROID_MAX_SIZE - ASTEROID_MIN_SIZE);
        asteroid.rotation = 0;
        asteroid.sides = 6 + rand() % 4;

        asteroids.push_back(asteroid);
    }
}

// Draw functions
void drawStar(const Star& star) {
    glColor3f(star.brightness, star.brightness, star.brightness);
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    glVertex2f(star.pos.x, star.pos.y);
    glEnd();
}

void drawShip(const Ship& ship) {
    glPushMatrix();
    glTranslatef(ship.pos.x, ship.pos.y, 0);
    glRotatef(ship.angle * 180.0f / M_PI, 0, 0, 1);

    // Ship body
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(ship.size, 0);
    glVertex2f(-ship.size, ship.size * 0.7f);
    glVertex2f(-ship.size * 0.5f, 0);
    glVertex2f(-ship.size, -ship.size * 0.7f);
    glEnd();

    // Thruster flame
    if (ship.thrusting) {
        glColor3f(1.0f, 0.5f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-ship.size * 0.5f, 0);
        glVertex2f(-ship.size * 1.5f, ship.size * 0.3f);
        glVertex2f(-ship.size * 1.5f, -ship.size * 0.3f);
        glEnd();
    }

    glPopMatrix();
}

void drawBullet(const Bullet& bullet) {
    glColor3f(1.0f, 1.0f, 0.0f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(bullet.pos.x, bullet.pos.y);
    glEnd();
}

void drawAsteroid(const Asteroid& asteroid) {
    glPushMatrix();
    glTranslatef(asteroid.pos.x, asteroid.pos.y, 0);
    glRotatef(asteroid.rotation, 0, 0, 1);

    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < asteroid.sides; i++) {
        float angle = 2.0f * M_PI * i / asteroid.sides;
        float radius = asteroid.size * (0.8f + 0.4f * sin(angle * 3));
        glVertex2f(radius * cos(angle), radius * sin(angle));
    }

    glEnd();
    glPopMatrix();
}

void drawHUD() {
    glColor3f(1.0f, 1.0f, 1.0f);

    char buffer[100];
    sprintf(buffer, "Score: %d", score);
    glRasterPos2f(-0.95f, 0.9f);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    sprintf(buffer, "Lives: %d", lives);
    glRasterPos2f(-0.95f, 0.85f);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    sprintf(buffer, "Level: %d", level);
    glRasterPos2f(-0.95f, 0.8f);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
    }

    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2f(-0.1f, 0.0f);
        const char* text = "GAME OVER! Press R to restart";
        for (int i = 0; text[i]; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        }
    }

    if (gameWon) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glRasterPos2f(-0.1f, 0.0f);
        const char* text = "LEVEL COMPLETE! Press N for next level";
        for (int i = 0; text[i]; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        }
    }

    // Controls
    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(-0.95f, -0.85f);
    const char* controls = "Controls: Arrow Keys = Move, Space = Shoot, R = Restart";
    for (int i = 0; controls[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, controls[i]);
    }
}

// Collision detection
bool checkCollision(Point p1, float r1, Point p2, float r2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < (r1 + r2);
}

// Wrap around screen
void wrapPosition(Point& pos) {
    if (pos.x > 1.0f) pos.x = -1.0f;
    if (pos.x < -1.0f) pos.x = 1.0f;
    if (pos.y > 1.0f) pos.y = -1.0f;
    if (pos.y < -1.0f) pos.y = 1.0f;
}

// Update game logic
void updateGame() {
    if (gameOver || gameWon) return;

    const float ROTATION_SPEED = 0.15f;
    const float THRUST = 0.0005f;
    const float FRICTION = 0.99f;

    // FIXED: Handle thrust
    if (specialKeys[GLUT_KEY_UP]) {
        player.thrusting = true;
        player.velocity.x += cos(player.angle) * THRUST;
        player.velocity.y += sin(player.angle) * THRUST;
    }
    else {
        player.thrusting = false;
    }

    // FIXED: Handle rotation
    if (specialKeys[GLUT_KEY_LEFT]) {
        player.angle += ROTATION_SPEED;
    }

    if (specialKeys[GLUT_KEY_RIGHT]) {
        player.angle -= ROTATION_SPEED;
    }

    // FIXED: Handle braking/reverse thrust
    if (specialKeys[GLUT_KEY_DOWN]) {
        player.velocity.x *= 0.9f;
        player.velocity.y *= 0.9f;
    }

    // Apply friction
    player.velocity.x *= FRICTION;
    player.velocity.y *= FRICTION;

    // Update position
    player.pos.x += player.velocity.x;
    player.pos.y += player.velocity.y;

    wrapPosition(player.pos);

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->pos.x += it->velocity.x;
        it->pos.y += it->velocity.y;
        it->life--;

        wrapPosition(it->pos);

        if (it->life <= 0) {
            it = bullets.erase(it);
        }
        else {
            ++it;
        }
    }

    // Update asteroids
    for (auto& asteroid : asteroids) {
        asteroid.pos.x += asteroid.velocity.x;
        asteroid.pos.y += asteroid.velocity.y;
        asteroid.rotation += 1.0f;

        wrapPosition(asteroid.pos);
    }

    // Update stars
    for (auto& star : stars) {
        star.twinkle += 0.1f;
        star.brightness = 0.3f + 0.4f * (0.5f + 0.5f * sin(star.twinkle));
    }

    // Check bullet-asteroid collisions
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        bool bulletHit = false;

        for (auto asteroidIt = asteroids.begin(); asteroidIt != asteroids.end();) {
            if (checkCollision(bulletIt->pos, 0.01f, asteroidIt->pos, asteroidIt->size)) {
                score += 10;

                // Break asteroid into smaller pieces
                if (asteroidIt->size > ASTEROID_MIN_SIZE * 1.5f) {
                    for (int i = 0; i < 2; i++) {
                        Asteroid newAsteroid;
                        newAsteroid.pos = asteroidIt->pos;
                        newAsteroid.velocity.x = ((rand() % 100) - 50) / 8000.0f;
                        newAsteroid.velocity.y = ((rand() % 100) - 50) / 8000.0f;
                        newAsteroid.size = asteroidIt->size * 0.6f;
                        newAsteroid.rotation = 0;
                        newAsteroid.sides = 6 + rand() % 4;
                        asteroids.push_back(newAsteroid);
                    }
                }

                asteroidIt = asteroids.erase(asteroidIt);
                bulletHit = true;
                break;
            }
            else {
                ++asteroidIt;
            }
        }

        if (bulletHit) {
            bulletIt = bullets.erase(bulletIt);
        }
        else {
            ++bulletIt;
        }
    }

    // Check player-asteroid collisions
    if (respawnTimer <= 0) {
        for (const auto& asteroid : asteroids) {
            if (checkCollision(player.pos, player.size, asteroid.pos, asteroid.size)) {
                lives--;
                respawnTimer = 120; // 2 seconds of invincibility

                if (lives <= 0) {
                    gameOver = true;
                }
                break;
            }
        }
    }
    else {
        respawnTimer--;
    }

    // Check for level completion
    if (asteroids.empty() && !gameWon) {
        gameWon = true;
    }
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw stars
    for (const auto& star : stars) {
        drawStar(star);
    }

    // Draw asteroids
    for (const auto& asteroid : asteroids) {
        drawAsteroid(asteroid);
    }

    // Draw bullets
    for (const auto& bullet : bullets) {
        drawBullet(bullet);
    }

    // Draw player (with respawn flashing)
    if (respawnTimer <= 0 || (((int)respawnTimer) % 10) < 5) {
        drawShip(player);
    }

    // Draw HUD
    drawHUD();

    glutSwapBuffers();
}

// Keyboard input
bool keys[256] = { false };

void keyDown(unsigned char key, int x, int y) {
    keys[key] = true;

    if (key == ' ') {
        // Shoot bullet
        if (bullets.size() < 10) {
            Bullet bullet;
            bullet.pos = player.pos;
            bullet.velocity.x = cos(player.angle) * BULLET_SPEED;
            bullet.velocity.y = sin(player.angle) * BULLET_SPEED;
            bullet.life = BULLET_LIFE;
            bullets.push_back(bullet);
        }
    }

    if (key == 'r' || key == 'R') {
        if (gameOver) {
            level = 1;
            initGame();
        }
    }

    if (key == 'n' || key == 'N') {
        if (gameWon) {
            level++;
            asteroids.clear();
            bullets.clear();
            createAsteroids(INITIAL_ASTEROIDS + level - 1);
            gameWon = false;
            player.pos = Point(0, 0);
            player.velocity = Point(0, 0);
        }
    }
}

void keyUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

// FIXED: Proper special key handling
void specialKeyDown(int key, int x, int y) {
    if (key < 256) {  // Safety check
        specialKeys[key] = true;
    }
}

void specialKeyUp(int key, int x, int y) {
    if (key < 256) {  // Safety check
        specialKeys[key] = false;
    }
}

// Timer function
void timer(int value) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // 60 FPS
}

// Initialize OpenGL
void init() {
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initGame();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Asteroid Shooter - Classic Space Game!");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(16, timer, 0);

    printf("=== ASTEROID SHOOTER ===\n");
    printf("Controls:\n");
    printf("  Arrow Keys - Steer and thrust\n");
    printf("  UP - Thrust forward\n");
    printf("  DOWN - Brake/slow down\n");
    printf("  LEFT/RIGHT - Rotate\n");
    printf("  Spacebar - Shoot\n");
    printf("  R - Restart game\n");
    printf("  N - Next level (when level complete)\n");
    printf("\nObjective: Destroy all asteroids!\n");
    printf("Large asteroids break into smaller ones.\n");
    printf("Avoid colliding with asteroids!\n\n");

    glutMainLoop();
    return 0;
}