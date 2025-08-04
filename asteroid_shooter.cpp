// asteroid_shooter.cpp
// A 2D space shooter game with end-game score report

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>

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
    bool reverseThrusting;
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

// Statistics tracking
int totalAsteroidsDestroyed = 0;
int largeAsteroidsDestroyed = 0;
int mediumAsteroidsDestroyed = 0;
int smallAsteroidsDestroyed = 0;
int livesLost = 0;
time_t gameStartTime;
time_t gameEndTime;
bool showScoreReport = false;

// Game constants
const float SHIP_SIZE = 0.02f;
const float BULLET_SPEED = 0.01f;
const float BULLET_LIFE = 100.0f;
const float ASTEROID_MIN_SIZE = 0.03f;
const float ASTEROID_MAX_SIZE = 0.08f;
const int INITIAL_ASTEROIDS = 5;

bool specialKeys[256] = { false };

// Forward declarations
void createAsteroids(int count);
void displayGameReport();

// Initialize game
void initGame() {
    srand(time(0));
    gameStartTime = time(0);
    gameEndTime = 0;

    // Initialize player
    player.pos = Point(0, 0);
    player.angle = 0;
    player.velocity = Point(0, 0);
    player.thrusting = false;
    player.reverseThrusting = false;
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
    showScoreReport = false;

    // Reset session stats
    totalAsteroidsDestroyed = 0;
    largeAsteroidsDestroyed = 0;
    mediumAsteroidsDestroyed = 0;
    smallAsteroidsDestroyed = 0;
    livesLost = 0;
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

void displayGameReport() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate survival time - use gameEndTime if game is over, otherwise current time
    float survivalTime = (gameEndTime > 0) ? difftime(gameEndTime, gameStartTime) : difftime(time(0), gameStartTime);
    int rating = (score / 1000) + (totalAsteroidsDestroyed / 10) + (level * 5);

    const char* rank = "Cadet";
    if (rating >= 80) rank = "Space Admiral";
    else if (rating >= 60) rank = "Squadron Leader";
    else if (rating >= 40) rank = "Ace Pilot";
    else if (rating >= 25) rank = "Veteran";
    else if (rating >= 15) rank = "Pilot";

    glColor3f(1.0f, 1.0f, 0.0f);
    glRasterPos2f(-0.3f, 0.8f);
    const char* title = "ASTEROID SHOOTER - GAME REPORT";
    for (int i = 0; title[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, title[i]);
    }

    // Draw a line under title
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(-0.6f, 0.75f);
    glVertex2f(0.6f, 0.75f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    char buffer[100];
    float yPos = 0.6f;

    // Final Statistics
    glRasterPos2f(-0.5f, yPos);
    const char* statsTitle = "FINAL STATISTICS:";
    for (int i = 0; statsTitle[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, statsTitle[i]);
    }
    yPos -= 0.08f;

    sprintf(buffer, "Final Score: %d", score);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Level Reached: %d", level);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Asteroids Destroyed: %d", totalAsteroidsDestroyed);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Survival Time: %d seconds", (int)survivalTime);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Lives Lost: %d", livesLost);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.12f;

    // Asteroid Breakdown
    glRasterPos2f(-0.5f, yPos);
    const char* breakdownTitle = "ASTEROID BREAKDOWN:";
    for (int i = 0; breakdownTitle[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, breakdownTitle[i]);
    }
    yPos -= 0.08f;

    sprintf(buffer, "Large Asteroids: %d", largeAsteroidsDestroyed);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Medium Asteroids: %d", mediumAsteroidsDestroyed);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Small Asteroids: %d", smallAsteroidsDestroyed);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.12f;

    // Performance Rating
    glColor3f(1.0f, 1.0f, 0.0f);
    glRasterPos2f(-0.5f, yPos);
    const char* ratingTitle = "PERFORMANCE RATING:";
    for (int i = 0; ratingTitle[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ratingTitle[i]);
    }
    yPos -= 0.08f;

    glColor3f(0.0f, 1.0f, 0.0f);
    sprintf(buffer, "Rank: %s", rank);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }
    yPos -= 0.06f;

    sprintf(buffer, "Overall Rating: %d/100", rating);
    glRasterPos2f(-0.4f, yPos);
    for (int i = 0; buffer[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
    }

    // Instructions
    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(-0.3f, -0.8f);
    const char* instructions = "Press any key to return to game";
    for (int i = 0; instructions[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, instructions[i]);
    }

    glutSwapBuffers();
}

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

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(ship.size, 0);
    glVertex2f(-ship.size, ship.size * 0.7f);
    glVertex2f(-ship.size * 0.5f, 0);
    glVertex2f(-ship.size, -ship.size * 0.7f);
    glEnd();

    if (ship.thrusting) {
        glColor3f(1.0f, 0.5f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-ship.size * 0.5f, 0);
        glVertex2f(-ship.size * 1.5f, ship.size * 0.3f);
        glVertex2f(-ship.size * 1.5f, -ship.size * 0.3f);
        glEnd();
    }

    if (ship.reverseThrusting) {
        glColor3f(0.0f, 0.7f, 1.0f); // Blue flames for reverse thrust
        glBegin(GL_TRIANGLES);
        glVertex2f(ship.size, 0);
        glVertex2f(ship.size * 1.5f, ship.size * 0.3f);
        glVertex2f(ship.size * 1.5f, -ship.size * 0.3f);
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
        glRasterPos2f(-0.15f, 0.0f);
        const char* text = "GAME OVER! Press R for report, S to restart";
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

    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(-0.95f, -0.85f);
    const char* controls = "Controls: Up/Down=Thrust, Left/Right=Rotate, Space=Shoot, R=Report, S=Restart";
    for (int i = 0; controls[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, controls[i]);
    }
}

bool checkCollision(Point p1, float r1, Point p2, float r2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < (r1 + r2);
}

void wrapPosition(Point& pos) {
    if (pos.x > 1.0f) pos.x = -1.0f;
    if (pos.x < -1.0f) pos.x = 1.0f;
    if (pos.y > 1.0f) pos.y = -1.0f;
    if (pos.y < -1.0f) pos.y = 1.0f;
}

void updateGame() {
    if (gameOver || gameWon) return;

    const float ROTATION_SPEED = 0.15f;
    const float THRUST = 0.0005f;
    const float FRICTION = 0.99f;

    if (specialKeys[GLUT_KEY_UP]) {
        player.thrusting = true;
        player.reverseThrusting = false;
        player.velocity.x += cos(player.angle) * THRUST;
        player.velocity.y += sin(player.angle) * THRUST;
    }
    else {
        player.thrusting = false;
    }

    if (specialKeys[GLUT_KEY_LEFT]) {
        player.angle += ROTATION_SPEED;
    }

    if (specialKeys[GLUT_KEY_RIGHT]) {
        player.angle -= ROTATION_SPEED;
    }

    if (specialKeys[GLUT_KEY_DOWN]) {
        // Reverse thrust - move backward
        player.reverseThrusting = true;
        player.thrusting = false;
        player.velocity.x -= cos(player.angle) * THRUST;
        player.velocity.y -= sin(player.angle) * THRUST;
    }
    else if (!specialKeys[GLUT_KEY_UP]) {
        player.reverseThrusting = false;
    }

    player.velocity.x *= FRICTION;
    player.velocity.y *= FRICTION;

    player.pos.x += player.velocity.x;
    player.pos.y += player.velocity.y;

    wrapPosition(player.pos);

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

    for (auto& asteroid : asteroids) {
        asteroid.pos.x += asteroid.velocity.x;
        asteroid.pos.y += asteroid.velocity.y;
        asteroid.rotation += 1.0f;

        wrapPosition(asteroid.pos);
    }

    for (auto& star : stars) {
        star.twinkle += 0.1f;
        star.brightness = 0.3f + 0.4f * (0.5f + 0.5f * sin(star.twinkle));
    }

    // Check bullet-asteroid collisions
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        bool bulletHit = false;

        for (auto asteroidIt = asteroids.begin(); asteroidIt != asteroids.end();) {
            if (checkCollision(bulletIt->pos, 0.01f, asteroidIt->pos, asteroidIt->size)) {
                totalAsteroidsDestroyed++;

                // Track asteroid sizes and award points
                if (asteroidIt->size > ASTEROID_MAX_SIZE * 0.8f) {
                    largeAsteroidsDestroyed++;
                    score += 20;
                }
                else if (asteroidIt->size > ASTEROID_MIN_SIZE * 1.5f) {
                    mediumAsteroidsDestroyed++;
                    score += 15;
                }
                else {
                    smallAsteroidsDestroyed++;
                    score += 10;
                }

                // Split large asteroids
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

    // Check ship-asteroid collisions
    if (respawnTimer <= 0) {
        for (const auto& asteroid : asteroids) {
            if (checkCollision(player.pos, player.size, asteroid.pos, asteroid.size)) {
                lives--;
                livesLost++;
                respawnTimer = 120;

                if (lives <= 0) {
                    gameOver = true;
                    gameEndTime = time(0);
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

void display() {
    if (showScoreReport) {
        displayGameReport();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& star : stars) {
        drawStar(star);
    }

    for (const auto& asteroid : asteroids) {
        drawAsteroid(asteroid);
    }

    for (const auto& bullet : bullets) {
        drawBullet(bullet);
    }

    if (respawnTimer <= 0 || (((int)respawnTimer) % 10) < 5) {
        drawShip(player);
    }

    drawHUD();

    glutSwapBuffers();
}

bool keys[256] = { false };

void keyDown(unsigned char key, int x, int y) {
    keys[key] = true;

    if (showScoreReport) {
        // Any key press while showing report returns to game
        showScoreReport = false;
        return;
    }

    if (key == ' ') {
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
            showScoreReport = true;
        }
    }

    if (key == 's' || key == 'S') {
        level = 1;
        initGame();
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

void specialKeyDown(int key, int x, int y) {
    if (key < 256) {
        specialKeys[key] = true;
    }
}

void specialKeyUp(int key, int x, int y) {
    if (key < 256) {
        specialKeys[key] = false;
    }
}

void timer(int value) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initGame();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Asteroid Shooter - With Game Report");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(16, timer, 0);

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                🚀 ASTEROID SHOOTER GAME 🌌                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("🎮 CONTROLS:\n");
    printf("   ⬆️  UP Arrow    - Thrust forward\n");
    printf("   ⬇️  DOWN Arrow  - Reverse thrust (go backward)\n");
    printf("   ⬅️  LEFT Arrow  - Rotate left\n");
    printf("   ➡️  RIGHT Arrow - Rotate right\n");
    printf("   🚀 SPACEBAR    - Shoot\n");
    printf("   📊 R Key       - Show game report (after game over)\n");
    printf("   🔄 S Key       - Start new game\n");
    printf("   ➡️  N Key       - Next level (when available)\n\n");
    printf("🎯 OBJECTIVES:\n");
    printf("   💥 Destroy all asteroids to advance levels\n");
    printf("   🌟 Survive as long as possible\n");
    printf("   📈 Get the highest score\n\n");
    printf("Press any key to start!\n");
    getchar();

    glutMainLoop();
    return 0;
}