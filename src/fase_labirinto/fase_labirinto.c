#include "../core/game.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define COLS 32
#define ROWS 18
#define CELL_SIZE 40

typedef enum GateType {
    GATE_AND,
    GATE_OR,
    GATE_NOT,
    GATE_XOR
} GateType;

// Barriers (Gates)
typedef struct {
    int row, col;
    GateType type;
    int inputs[2]; // Indices of switches in the array (unused)
    bool open;
    Color color;
    const char* label;
} BarrierInfo;

#define MAX_LEVELS 5

typedef struct {
    int r, c;
} Coord;

// Active runtime state
static int activeMaze[ROWS][COLS];
static BarrierInfo activeBarriers[32];
static int activeNumBarriers = 0;
static int currentLevelIdx = 0;
static char currentLevelName[128];
static char currentObjective[256];

// New gameplay state variables
static Vector2 enemyPos;
static float enemySpeed = 100.0f;
static float enemyRadius = 12.0f;
static Coord enemyPath[ROWS * COLS];
static int enemyPathLen = 0;
static float enemySpawnDelay = 3.0f;
static int enemyPathUpdateTimer = 0;

static bool isIntroActive = true;
static bool isPropositionActive = false;
static int activeBarrierChallengeIdx = -1;
static int lastInteractedBarrierIdx = -1;
static bool gameOver = false;
static bool isGamePaused = false;
static float errorCooldownTimer = 0.0f;

typedef struct {
    char questionText[256];
    bool correctAnswer; // true = V, false = F
    int selectedOption;  // 0 = None, 1 = V, 2 = F
} PropositionQuestion;

static PropositionQuestion currentQuestion;

// DFS Maze Generator
static void GenerateMazeDFS(int r, int c) {
    Coord dirs[4] = {
        {-2, 0}, {2, 0}, {0, -2}, {0, 2}
    };
    
    // Shuffle directions
    for (int i = 0; i < 4; i++) {
        int target = GetRandomValue(0, 3);
        Coord temp = dirs[i];
        dirs[i] = dirs[target];
        dirs[target] = temp;
    }
    
    for (int i = 0; i < 4; i++) {
        int nr = r + dirs[i].r;
        int nc = c + dirs[i].c;
        
        if (nr >= 1 && nr <= 15 && nc >= 1 && nc <= 29) {
            if (activeMaze[nr][nc] == 1) {
                // Break wall
                activeMaze[r + dirs[i].r / 2][c + dirs[i].c / 2] = 0;
                activeMaze[nr][nc] = 0;
                GenerateMazeDFS(nr, nc);
            }
        }
    }
}

// Find path from (1,1) to (15,29) using BFS
static int FindMainPath(Coord path[], int maxLen) {
    Coord queue[ROWS * COLS];
    int head = 0, tail = 0;
    
    static bool visited[ROWS][COLS];
    static Coord parent[ROWS][COLS];
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            visited[r][c] = false;
            parent[r][c] = (Coord){-1, -1};
        }
    }
    
    Coord start = {1, 1};
    queue[tail++] = start;
    visited[1][1] = true;
    
    bool found = false;
    Coord goal = {15, 29};
    
    while (head < tail) {
        Coord curr = queue[head++];
        if (curr.r == goal.r && curr.c == goal.c) {
            found = true;
            break;
        }
        
        Coord neighbors[4] = {
            {curr.r-1, curr.c}, {curr.r+1, curr.c},
            {curr.r, curr.c-1}, {curr.r, curr.c+1}
        };
        
        for (int i = 0; i < 4; i++) {
            int nr = neighbors[i].r;
            int nc = neighbors[i].c;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if ((activeMaze[nr][nc] == 0 || activeMaze[nr][nc] == 3) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = curr;
                    queue[tail++] = (Coord){nr, nc};
                }
            }
        }
    }
    
    if (!found) return 0;
    
    int len = 0;
    Coord curr = goal;
    while (curr.r != -1 && curr.c != -1) {
        if (len < maxLen) {
            path[len++] = curr;
        }
        curr = parent[curr.r][curr.c];
    }
    
    // Reverse
    for (int i = 0; i < len / 2; i++) {
        Coord temp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = temp;
    }
    
    return len;
}

// Find path for the chasing enemy using BFS, treating closed barriers and walls as obstacles
static int FindEnemyPath(Coord path[], int maxLen, Coord start, Coord goal) {
    Coord queue[ROWS * COLS];
    int head = 0, tail = 0;
    
    static bool visited[ROWS][COLS];
    static Coord parent[ROWS][COLS];
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            visited[r][c] = false;
            parent[r][c] = (Coord){-1, -1};
        }
    }
    
    queue[tail++] = start;
    visited[start.r][start.c] = true;
    
    bool found = false;
    
    while (head < tail) {
        Coord curr = queue[head++];
        if (curr.r == goal.r && curr.c == goal.c) {
            found = true;
            break;
        }
        
        Coord neighbors[4] = {
            {curr.r-1, curr.c}, {curr.r+1, curr.c},
            {curr.r, curr.c-1}, {curr.r, curr.c+1}
        };
        
        for (int i = 0; i < 4; i++) {
            int nr = neighbors[i].r;
            int nc = neighbors[i].c;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                // Enemy can walk on: empty (0), spawn (2), goal (3)
                // and open barriers. It cannot walk on walls (1) or closed barriers.
                bool isWalkable = (activeMaze[nr][nc] == 0 || activeMaze[nr][nc] == 2 || activeMaze[nr][nc] == 3);
                
                // If it is a barrier, check if it's open
                if (activeMaze[nr][nc] == 5 || activeMaze[nr][nc] == 6 || activeMaze[nr][nc] == 7 || activeMaze[nr][nc] == 8) {
                    for (int b = 0; b < activeNumBarriers; b++) {
                        if (activeBarriers[b].row == nr && activeBarriers[b].col == nc && activeBarriers[b].open) {
                            isWalkable = true;
                            break;
                        }
                    }
                }
                
                if (isWalkable && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = curr;
                    queue[tail++] = (Coord){nr, nc};
                }
            }
        }
    }
    
    if (!found) return 0;
    
    int len = 0;
    Coord curr = goal;
    while (curr.r != start.r || curr.c != start.c) {
        if (len < maxLen) {
            path[len++] = curr;
        }
        curr = parent[curr.r][curr.c];
        if (curr.r == -1) break;
    }
    
    // Reverse path to go from start to goal
    for (int i = 0; i < len / 2; i++) {
        Coord temp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = temp;
    }
    
    return len;
}

// Generate dynamic logic propositions based on the barrier's gate type
static void GeneratePropositionForGate(GateType type) {
    int op = GetRandomValue(0, 2);
    bool p = GetRandomValue(0, 1) == 1;
    bool q = GetRandomValue(0, 1) == 1;
    
    currentQuestion.selectedOption = 0;
    
    switch (type) {
        case GATE_NOT: {
            if (op == 0) {
                sprintf(currentQuestion.questionText, "Dada a proposicao P = %s.\nQual o valor de ~P (NOT P)?", p ? "V" : "F");
                currentQuestion.correctAnswer = !p;
            } else if (op == 1) {
                sprintf(currentQuestion.questionText, "Se a proposicao P e %s,\nentao a sua negacao ~P e:", p ? "Verd. (V)" : "Falsa (F)");
                currentQuestion.correctAnswer = !p;
            } else {
                sprintf(currentQuestion.questionText, "Qual o valor logico de ~(~P) se P = %s?", p ? "V" : "F");
                currentQuestion.correctAnswer = p;
            }
            break;
        }
        case GATE_AND: {
            if (op == 0) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de P ^ Q (AND)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = p && q;
            } else if (op == 1) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de P -> Q (Condicional)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = (!p || q);
            } else {
                sprintf(currentQuestion.questionText, "Para P = %s e Q = %s,\nqual o valor de ~(P ^ Q)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = !(p && q);
            }
            break;
        }
        case GATE_OR: {
            if (op == 0) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de P v Q (OR)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = p || q;
            } else if (op == 1) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de P <-> Q (Bicondicional)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = (p == q);
            } else {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de ~P v Q?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = (!p || q);
            }
            break;
        }
        case GATE_XOR: {
            if (op == 0) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de P ⊕ Q (XOR)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = (p != q);
            } else if (op == 1) {
                sprintf(currentQuestion.questionText, "Se P = %s e Q = %s,\nqual o valor de ~(P ⊕ Q)?", p ? "V" : "F", q ? "V" : "F");
                currentQuestion.correctAnswer = (p == q);
            } else {
                sprintf(currentQuestion.questionText, "Se P = %s,\nqual o valor de P -> ~P?", p ? "V" : "F");
                currentQuestion.correctAnswer = !p;
            }
            break;
        }
    }
}

static void GenerateProceduralLevel(void) {
    // Fill all with walls
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            activeMaze[r][c] = 1;
        }
    }
    
    // Set spawn and start DFS
    activeMaze[1][1] = 0;
    GenerateMazeDFS(1, 1);
    
    // Restore spawn and set goal
    activeMaze[1][1] = 2; // Spawn
    activeMaze[15][29] = 3; // Goal
    
    // Find main path
    static Coord mainPath[ROWS * COLS];
    int pathLen = FindMainPath(mainPath, ROWS * COLS);
    
    // Clear active objects
    activeNumBarriers = 0;
    
    int level = currentLevelIdx + 1;
    sprintf(currentLevelName, "FASE %d", level);
    
    if (level == 1) {
        // 5 barriers
        int num = 5;
        GateType types[5] = { GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, GATE_NOT };
        Color colors[5] = {
            (Color){ 249, 115, 22, 255 },  // NOT (Orange)
            (Color){ 59, 130, 246, 255 },  // AND (Blue)
            (Color){ 234, 179, 8, 255 },   // OR (Yellow)
            (Color){ 168, 85, 247, 255 },  // XOR (Purple)
            (Color){ 244, 63, 94, 255 }    // NOT 2 (Rose)
        };
        const char* labels[5] = { "PORTA NOT 1", "PORTA AND 1", "PORTA OR 1", "PORTA XOR 1", "PORTA NOT 2" };
        int cellTypes[5] = { 7, 5, 8, 6, 7 }; // NOT=7, AND=5, OR=8, XOR=6
        
        for (int i = 0; i < num; i++) {
            int idx = ((i + 1) * pathLen) / (num + 1);
            Coord pos = mainPath[idx];
            activeMaze[pos.r][pos.c] = cellTypes[i];
            activeBarriers[activeNumBarriers++] = (BarrierInfo){
                pos.r, pos.c, types[i], {i, -1}, false, colors[i], labels[i]
            };
        }
        strcpy(currentObjective, "FASE 1: Resolva as 5 portas lógicas para fugir do Vírus!");
    }
    else if (level == 2) {
        // 7 barriers
        int num = 7;
        GateType types[7] = { GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, GATE_NOT, GATE_AND, GATE_OR };
        Color colors[7] = {
            (Color){ 249, 115, 22, 255 },  // Orange
            (Color){ 59, 130, 246, 255 },  // Blue
            (Color){ 234, 179, 8, 255 },   // Yellow
            (Color){ 168, 85, 247, 255 },  // Purple
            (Color){ 244, 63, 94, 255 },   // Rose
            (Color){ 34, 211, 238, 255 },  // Cyan
            (Color){ 253, 224, 71, 255 }   // Light Yellow
        };
        const char* labels[7] = { "PORTA NOT 1", "PORTA AND 1", "PORTA OR 1", "PORTA XOR 1", "PORTA NOT 2", "PORTA AND 2", "PORTA OR 2" };
        int cellTypes[7] = { 7, 5, 8, 6, 7, 5, 8 };
        
        for (int i = 0; i < num; i++) {
            int idx = ((i + 1) * pathLen) / (num + 1);
            Coord pos = mainPath[idx];
            activeMaze[pos.r][pos.c] = cellTypes[i];
            activeBarriers[activeNumBarriers++] = (BarrierInfo){
                pos.r, pos.c, types[i], {i, -1}, false, colors[i], labels[i]
            };
        }
        strcpy(currentObjective, "FASE 2: Decodifique os 7 bloqueios lógicos rapidamente!");
    }
    else if (level == 3) {
        // 9 barriers
        int num = 9;
        GateType types[9] = { GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, GATE_NOT };
        Color colors[9] = {
            (Color){ 249, 115, 22, 255 },
            (Color){ 59, 130, 246, 255 },
            (Color){ 234, 179, 8, 255 },
            (Color){ 168, 85, 247, 255 },
            (Color){ 244, 63, 94, 255 },
            (Color){ 34, 211, 238, 255 },
            (Color){ 253, 224, 71, 255 },
            (Color){ 192, 132, 252, 255 }, // Light Purple
            (Color){ 236, 72, 153, 255 }   // Pink
        };
        const char* labels[9] = { 
            "PORTA NOT 1", "PORTA AND 1", "PORTA OR 1", "PORTA XOR 1", 
            "PORTA NOT 2", "PORTA AND 2", "PORTA OR 2", "PORTA XOR 2", 
            "PORTA NOT 3" 
        };
        int cellTypes[9] = { 7, 5, 8, 6, 7, 5, 8, 6, 7 };
        
        for (int i = 0; i < num; i++) {
            int idx = ((i + 1) * pathLen) / (num + 1);
            Coord pos = mainPath[idx];
            activeMaze[pos.r][pos.c] = cellTypes[i];
            activeBarriers[activeNumBarriers++] = (BarrierInfo){
                pos.r, pos.c, types[i], {i, -1}, false, colors[i], labels[i]
            };
        }
        strcpy(currentObjective, "FASE 3: Decodifique os 9 bloqueios sob crescente pressão!");
    }
    else if (level == 4) {
        // 12 barriers
        int num = 12;
        GateType types[12] = { 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR 
        };
        Color colors[12] = {
            (Color){ 249, 115, 22, 255 },
            (Color){ 59, 130, 246, 255 },
            (Color){ 234, 179, 8, 255 },
            (Color){ 168, 85, 247, 255 }, // Purple
            (Color){ 244, 63, 94, 255 },
            (Color){ 34, 211, 238, 255 },
            (Color){ 253, 224, 71, 255 },
            (Color){ 192, 132, 252, 255 },
            (Color){ 236, 72, 153, 255 },
            (Color){ 14, 165, 233, 255 },
            (Color){ 45, 212, 191, 255 }, // Teal
            (Color){ 251, 146, 60, 255 }  // Light Orange
        };
        const char* labels[12] = { 
            "PORTA NOT 1", "PORTA AND 1", "PORTA OR 1", "PORTA XOR 1", 
            "PORTA NOT 2", "PORTA AND 2", "PORTA OR 2", "PORTA XOR 2",
            "PORTA NOT 3", "PORTA AND 3", "PORTA OR 3", "PORTA XOR 3"
        };
        int cellTypes[12] = { 7, 5, 8, 6, 7, 5, 8, 6, 7, 5, 8, 6 };
        
        for (int i = 0; i < num; i++) {
            int idx = ((i + 1) * pathLen) / (num + 1);
            Coord pos = mainPath[idx];
            activeMaze[pos.r][pos.c] = cellTypes[i];
            activeBarriers[activeNumBarriers++] = (BarrierInfo){
                pos.r, pos.c, types[i], {i, -1}, false, colors[i], labels[i]
            };
        }
        strcpy(currentObjective, "FASE 4: O Vírus está muito agressivo! Resolva as 12 portas!");
    }
    else {
        // 15 barriers
        int num = 15;
        GateType types[15] = { 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR, 
            GATE_NOT, GATE_AND, GATE_OR, GATE_XOR,
            GATE_NOT, GATE_AND, GATE_OR
        };
        Color colors[15] = {
            (Color){ 249, 115, 22, 255 },
            (Color){ 59, 130, 246, 255 },
            (Color){ 234, 179, 8, 255 },
            (Color){ 168, 85, 247, 255 },
            (Color){ 244, 63, 94, 255 },
            (Color){ 34, 211, 238, 255 },
            (Color){ 253, 224, 71, 255 },
            (Color){ 192, 132, 252, 255 },
            (Color){ 236, 72, 153, 255 },
            (Color){ 14, 165, 233, 255 },
            (Color){ 45, 212, 191, 255 },
            (Color){ 251, 146, 60, 255 },
            (Color){ 248, 113, 113, 255 }, // Light Red
            (Color){ 129, 140, 248, 255 }, // Indigo
            (Color){ 52, 211, 153, 255 }   // Emerald
        };
        const char* labels[15] = { 
            "PORTA NOT 1", "PORTA AND 1", "PORTA OR 1", "PORTA XOR 1", 
            "PORTA NOT 2", "PORTA AND 2", "PORTA OR 2", "PORTA XOR 2",
            "PORTA NOT 3", "PORTA AND 3", "PORTA OR 3", "PORTA XOR 3",
            "PORTA NOT 4", "PORTA AND 4", "PORTA OR 4"
        };
        int cellTypes[15] = { 7, 5, 8, 6, 7, 5, 8, 6, 7, 5, 8, 6, 7, 5, 8 };
        
        for (int i = 0; i < num; i++) {
            int idx = ((i + 1) * pathLen) / (num + 1);
            Coord pos = mainPath[idx];
            activeMaze[pos.r][pos.c] = cellTypes[i];
            activeBarriers[activeNumBarriers++] = (BarrierInfo){
                pos.r, pos.c, types[i], {i, -1}, false, colors[i], labels[i]
            };
        }
        strcpy(currentObjective, "DESAFIO FINAL: Supere 15 bloqueios lógicos para salvar o sistema!");
    }
}

// Player and game variables
static Vector2 playerPos;
static float playerRadius = 12.0f;
static float playerSpeed = 220.0f;

// Trail system
#define MAX_TRAIL 15
static Vector2 playerTrail[MAX_TRAIL];
static int trailCount = 0;

// Game states
static float gameTimer = 0.0f;
static bool gameWon = false;
static float globalPulse = 0.0f;
static int globalPulseDir = 1;

// Helper function to clamp floats
static float ClampFloat(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Check collision with walls and barriers
static bool CheckWallCollision(float px, float py, float radius) {
    int minCol = (int)((px - radius) / CELL_SIZE);
    int maxCol = (int)((px + radius) / CELL_SIZE);
    int minRow = (int)((py - radius) / CELL_SIZE);
    int maxRow = (int)((py + radius) / CELL_SIZE);
    
    for (int r = minRow; r <= maxRow; r++) {
        for (int c = minCol; c <= maxCol; c++) {
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                int cellType = activeMaze[r][c];
                bool isSolid = false;
                
                if (cellType == 1) {
                    isSolid = true; // Wall
                } else if (cellType == 5 || cellType == 6 || cellType == 7 || cellType == 8) {
                    // Check if this barrier is closed in activeBarriers
                    for (int b = 0; b < activeNumBarriers; b++) {
                        if (activeBarriers[b].row == r && activeBarriers[b].col == c && !activeBarriers[b].open) {
                            isSolid = true;
                            break;
                        }
                    }
                }
                
                if (isSolid) {
                    float cellX = c * CELL_SIZE;
                    float cellY = r * CELL_SIZE;
                    
                    float closestX = ClampFloat(px, cellX, cellX + CELL_SIZE);
                    float closestY = ClampFloat(py, cellY, cellY + CELL_SIZE);
                    
                    float distX = px - closestX;
                    float distY = py - closestY;
                    float distance = sqrtf(distX * distX + distY * distY);
                    
                    if (distance < radius) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Reset level state and load active level data
static void ResetLevel(void) {
    gameTimer = 0.0f;
    gameWon = false;
    gameOver = false;
    isPropositionActive = false;
    isGamePaused = false;
    errorCooldownTimer = 0.0f;
    activeBarrierChallengeIdx = -1;
    lastInteractedBarrierIdx = -1;
    enemySpawnDelay = 3.0f;
    enemyPathLen = 0;
    enemyPathUpdateTimer = 0;
    
    // Show intro popup on every level
    isIntroActive = true;
    
    // Dynamically generate the maze and level elements!
    GenerateProceduralLevel();
    
    // Player spawn point is always (1,1) in the generator
    playerPos.x = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    playerPos.y = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    
    // Enemy spawn point is also (1,1)
    enemyPos.x = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    enemyPos.y = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    enemySpeed = 110.0f + currentLevelIdx * 25.0f;
    
    // Reset trail
    trailCount = 0;
    for (int i = 0; i < MAX_TRAIL; i++) {
        playerTrail[i] = playerPos;
    }
}

void InitGameplayScreen(void) {
    currentLevelIdx = 0;
    ResetLevel();
    globalPulse = 0.0f;
    globalPulseDir = 1;
}

static void UpdateEnemy(float dt) {
    if (enemySpawnDelay > 0.0f) {
        enemySpawnDelay -= dt;
    } else {
        enemyPathUpdateTimer++;
        if (enemyPathUpdateTimer >= 10 || enemyPathLen == 0) {
            enemyPathUpdateTimer = 0;
            Coord eGrid = { (int)(enemyPos.y / CELL_SIZE), (int)(enemyPos.x / CELL_SIZE) };
            Coord pGrid = { (int)(playerPos.y / CELL_SIZE), (int)(playerPos.x / CELL_SIZE) };
            enemyPathLen = FindEnemyPath(enemyPath, ROWS * COLS, eGrid, pGrid);
        }
        
        if (enemyPathLen > 0) {
            Coord targetCell = enemyPath[0];
            Vector2 targetPos = { targetCell.c * CELL_SIZE + CELL_SIZE / 2.0f, targetCell.r * CELL_SIZE + CELL_SIZE / 2.0f };
            
            // Move towards targetPos
            Vector2 dir = { targetPos.x - enemyPos.x, targetPos.y - enemyPos.y };
            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
            float step = enemySpeed * dt;
            
            if (dist <= step) {
                enemyPos = targetPos;
                for (int i = 0; i < enemyPathLen - 1; i++) {
                    enemyPath[i] = enemyPath[i + 1];
                }
                enemyPathLen--;
            } else {
                enemyPos.x += (dir.x / dist) * step;
                enemyPos.y += (dir.y / dist) * step;
            }
        }
        
        // Check collision between enemy and player
        float distToPlayer = sqrtf((playerPos.x - enemyPos.x) * (playerPos.x - enemyPos.x) + (playerPos.y - enemyPos.y) * (playerPos.y - enemyPos.y));
        if (distToPlayer < (enemyRadius + playerRadius)) {
            gameOver = true;
        }
    }
}

void UpdateGameplayScreen(void) {
    if (gameWon) {
        if (IsKeyPressed(KEY_SPACE)) {
            if (currentLevelIdx < MAX_LEVELS - 1) {
                currentLevelIdx++;
                ResetLevel();
            } else {
                currentLevelIdx = 0;
                ResetLevel();
            }
            return;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
            return;
        }
        
        // Mouse click detection
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        Rectangle btnNext = { SCREEN_WIDTH / 2.0f - 180, card.y + 185, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 185, 160, 45 };
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, btnNext)) {
                if (currentLevelIdx < MAX_LEVELS - 1) {
                    currentLevelIdx++;
                    ResetLevel();
                } else {
                    currentLevelIdx = 0;
                    ResetLevel();
                }
                return;
            }
            if (CheckCollisionPointRec(mousePos, btnMenu)) {
                currentScreen = SCREEN_TITLE;
                return;
            }
        }
        return;
    }
    
    if (gameOver) {
        if (IsKeyPressed(KEY_SPACE)) {
            ResetLevel();
            return;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
            return;
        }
        
        // Mouse interaction for Game Over
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 180, card.y + 180, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 180, 160, 45 };
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, btnRestart)) {
                ResetLevel();
                return;
            }
            if (CheckCollisionPointRec(mousePos, btnMenu)) {
                currentScreen = SCREEN_TITLE;
                return;
            }
        }
        return;
    }
    
    if (isIntroActive) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            isIntroActive = false;
        }
        
        // Check for glass panel button click
        Rectangle btnRect = { SCREEN_WIDTH / 2.0f - 100, SCREEN_HEIGHT / 2.0f + 165, 200, 45 };
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, btnRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isIntroActive = false;
        }
        
        // Quick exit to menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
        }
        return;
    }
    
    if (isPropositionActive) {
        if (errorCooldownTimer > 0.0f) {
            float dt = GetFrameTime();
            errorCooldownTimer -= dt;
            
            // The virus keeps updating and can capture the player!
            UpdateEnemy(dt);
            
            if (gameOver) {
                isPropositionActive = false;
                activeBarrierChallengeIdx = -1;
                lastInteractedBarrierIdx = -1;
                errorCooldownTimer = 0.0f;
                return;
            }
            
            if (errorCooldownTimer <= 0.0f) {
                errorCooldownTimer = 0.0f;
                // Incorrect answer!
                // Push player back
                if (activeBarrierChallengeIdx >= 0 && activeBarrierChallengeIdx < activeNumBarriers) {
                    BarrierInfo b = activeBarriers[activeBarrierChallengeIdx];
                    Vector2 barCenter = { b.col * CELL_SIZE + CELL_SIZE / 2.0f, b.row * CELL_SIZE + CELL_SIZE / 2.0f };
                    Vector2 dir = { playerPos.x - barCenter.x, playerPos.y - barCenter.y };
                    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    if (len > 0.0f) {
                        dir.x /= len;
                        dir.y /= len;
                    } else {
                        dir.x = -1.0f; dir.y = 0.0f; // Default push direction
                    }
                    
                    for (float d = 40.0f; d >= 15.0f; d -= 5.0f) {
                        Vector2 newPos = { barCenter.x + dir.x * d, barCenter.y + dir.y * d };
                        if (!CheckWallCollision(newPos.x, newPos.y, playerRadius)) {
                            playerPos = newPos;
                            break;
                        }
                    }
                    // Generate new question for next try
                    GeneratePropositionForGate(b.type);
                }
                isPropositionActive = false;
                activeBarrierChallengeIdx = -1;
            }
            
            // Quick exit to menu
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_TITLE;
                errorCooldownTimer = 0.0f;
                lastInteractedBarrierIdx = -1;
            }
            return;
        }

        // Option buttons in Proposition Overlay
        Rectangle challengeRect = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 160, 500, 300 };
        Rectangle btnV = { SCREEN_WIDTH / 2.0f - 140, challengeRect.y + 180, 120, 50 };
        Rectangle btnF = { SCREEN_WIDTH / 2.0f + 20, challengeRect.y + 180, 120, 50 };
        
        Vector2 mousePos = GetMousePosition();
        bool hoveredV = CheckCollisionPointRec(mousePos, btnV);
        bool hoveredF = CheckCollisionPointRec(mousePos, btnF);
        
        bool choseTrue = false;
        bool choseFalse = false;
        
        if (IsKeyPressed(KEY_V)) choseTrue = true;
        if (IsKeyPressed(KEY_F)) choseFalse = true;
        
        if (hoveredV && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) choseTrue = true;
        if (hoveredF && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) choseFalse = true;
        
        if (choseTrue || choseFalse) {
            bool answer = choseTrue;
            if (answer == currentQuestion.correctAnswer) {
                // Correct answer! Open barrier and unfreeze
                if (activeBarrierChallengeIdx >= 0 && activeBarrierChallengeIdx < activeNumBarriers) {
                    activeBarriers[activeBarrierChallengeIdx].open = true;
                }
                isPropositionActive = false;
                activeBarrierChallengeIdx = -1;
                lastInteractedBarrierIdx = -1;
            } else {
                // Incorrect answer! Start 5 second cooldown
                errorCooldownTimer = 5.0f;
            }
        }
        
        // Quick exit to menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
            errorCooldownTimer = 0.0f;
            lastInteractedBarrierIdx = -1;
        }
        return;
    }
    
    if (isGamePaused) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            isGamePaused = false;
            return;
        }
        
        // Mouse interaction for Pause Screen
        Rectangle card = { SCREEN_WIDTH / 2.0f - 200, SCREEN_HEIGHT / 2.0f - 120, 400, 240 };
        Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 150, card.y + 100, 300, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f - 150, card.y + 160, 300, 45 };
        
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, btnResume)) {
                isGamePaused = false;
                return;
            }
            if (CheckCollisionPointRec(mousePos, btnMenu)) {
                currentScreen = SCREEN_TITLE;
                return;
            }
        }
        return;
    }
    
    // Normal Gameplay loop
    float dt = GetFrameTime();
    
    // Update timer
    gameTimer += dt;
    
    // Pulsate target goal and elements
    globalPulse += 0.03f * globalPulseDir;
    if (globalPulse >= 1.0f) {
        globalPulse = 1.0f;
        globalPulseDir = -1;
    } else if (globalPulse <= 0.0f) {
        globalPulse = 0.0f;
        globalPulseDir = 1;
    }
    
    // Movement inputs
    float dx = 0.0f;
    float dy = 0.0f;
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dy -= playerSpeed * dt;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy += playerSpeed * dt;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx -= playerSpeed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += playerSpeed * dt;
    
    // Slide along X-axis
    float nextX = playerPos.x + dx;
    if (!CheckWallCollision(nextX, playerPos.y, playerRadius)) {
        playerPos.x = nextX;
    }
    
    // Slide along Y-axis
    float nextY = playerPos.y + dy;
    if (!CheckWallCollision(playerPos.x, nextY, playerRadius)) {
        playerPos.y = nextY;
    }
    
    // Update Trail
    for (int i = MAX_TRAIL - 1; i > 0; i--) {
        playerTrail[i] = playerTrail[i - 1];
    }
    playerTrail[0] = playerPos;
    if (trailCount < MAX_TRAIL) trailCount++;
    
    // Update Enemy (Virus)
    UpdateEnemy(dt);
    
    // Reset lastInteractedBarrierIdx if player moves away from it
    if (lastInteractedBarrierIdx != -1) {
        BarrierInfo* b = &activeBarriers[lastInteractedBarrierIdx];
        float bX = b->col * CELL_SIZE + CELL_SIZE / 2.0f;
        float bY = b->row * CELL_SIZE + CELL_SIZE / 2.0f;
        float dist = sqrtf((playerPos.x - bX) * (playerPos.x - bX) + (playerPos.y - bY) * (playerPos.y - bY));
        if (dist >= 45.0f) {
            lastInteractedBarrierIdx = -1;
        }
    }
    
    // Check player proximity to closed barriers to trigger logic challenges
    for (int i = 0; i < activeNumBarriers; i++) {
        BarrierInfo* b = &activeBarriers[i];
        if (!b->open && i != lastInteractedBarrierIdx) {
            float bX = b->col * CELL_SIZE + CELL_SIZE / 2.0f;
            float bY = b->row * CELL_SIZE + CELL_SIZE / 2.0f;
            float dist = sqrtf((playerPos.x - bX) * (playerPos.x - bX) + (playerPos.y - bY) * (playerPos.y - bY));
            if (dist < 39.0f) {
                isPropositionActive = true;
                activeBarrierChallengeIdx = i;
                lastInteractedBarrierIdx = i;
                GeneratePropositionForGate(b->type);
                break;
            }
        }
    }
    
    // Check if player reached the Goal (cell value 3)
    int pCol = (int)(playerPos.x / CELL_SIZE);
    int pRow = (int)(playerPos.y / CELL_SIZE);
    if (pRow >= 0 && pRow < ROWS && pCol >= 0 && pCol < COLS) {
        if (activeMaze[pRow][pCol] == 3) {
            gameWon = true;
        }
    }
    
    // Pause the game when ESC is pressed
    if (IsKeyPressed(KEY_ESCAPE)) {
        isGamePaused = true;
    }
}

void DrawGameplayScreen(void) {
    // Cyber Cadet dark blue/black background
    ClearBackground(COLOR_BG_DARK); 
    
    // Draw background grid lines (cyber style)
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    
    // Floating tech particles in the background (ambient depth)
    float timeVal = (float)GetTime();
    for (int i = 0; i < 24; i++) {
        float phaseX = sinf(timeVal * 0.4f + i * 2.3f);
        float phaseY = cosf(timeVal * 0.3f + i * 1.7f);
        float px = (i * 81 + (int)(phaseX * 140.0f)) % SCREEN_WIDTH;
        float py = (i * 53 + (int)(phaseY * 110.0f)) % (SCREEN_HEIGHT - CELL_SIZE) + CELL_SIZE;
        float size = 1.0f + fabsf(sinf(timeVal + i)) * 1.5f;
        DrawCircle(px, py, size, ColorAlpha(COLOR_TEXT_CYBER, 0.12f));
        DrawCircleLines(px, py, size + 2.0f, ColorAlpha(COLOR_TEXT_CYBER, 0.04f));
    }
    
    // Draw Maze Cells, Walls, and Barriers
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cellX = c * CELL_SIZE;
            float cellY = r * CELL_SIZE;
            int cellType = activeMaze[r][c];
            
            if (cellType == 1) {
                // Wall - Cybertech Panel look
                DrawRectangle(cellX, cellY, CELL_SIZE, CELL_SIZE, COLOR_PANEL_BG); 
                DrawRectangleLines(cellX, cellY, CELL_SIZE, CELL_SIZE, COLOR_PANEL_BORDER);
                
                // Sci-fi corners
                DrawRectangle(cellX + 1, cellY + 1, 3, 3, (Color){ 30, 58, 138, 255 });
                DrawRectangle(cellX + CELL_SIZE - 4, cellY + 1, 3, 3, (Color){ 30, 58, 138, 255 });
                DrawRectangle(cellX + 1, cellY + CELL_SIZE - 4, 3, 3, (Color){ 30, 58, 138, 255 });
                DrawRectangle(cellX + CELL_SIZE - 4, cellY + CELL_SIZE - 4, 3, 3, (Color){ 30, 58, 138, 255 });
            } 
            else if (cellType == 3) {
                // Goal - Beautiful rotating portal ring
                float cx = cellX + CELL_SIZE / 2.0f;
                float cy = cellY + CELL_SIZE / 2.0f;
                float portalTime = (float)GetTime();
                
                DrawCircle(cx, cy, 18.0f + globalPulse * 4.0f, ColorAlpha(COLOR_NEON_MAGENTA, 0.20f)); // Aura
                DrawCircleLines(cx, cy, 14.0f + sinf(portalTime * 6.0f) * 2.0f, COLOR_NEON_MAGENTA); // Rotating ring 1
                DrawCircleLines(cx, cy, 10.0f - sinf(portalTime * 6.0f) * 2.0f, COLOR_NEON_PURPLE); // Rotating ring 2
                
                // Cyber spark lines radiating outward
                for (int i = 0; i < 4; i++) {
                    float angle = portalTime * 3.0f + i * (PI / 2.0f);
                    Vector2 start = { cx + cosf(angle) * 4.0f, cy + sinf(angle) * 4.0f };
                    Vector2 end = { cx + cosf(angle) * 12.0f, cy + sinf(angle) * 12.0f };
                    DrawLineEx(start, end, 1.5f, ColorAlpha(COLOR_NEON_MAGENTA, 0.6f));
                }
                
                DrawCircle(cx, cy, 5.0f, WHITE); // Core center
            }
            else if (cellType == 5 || cellType == 6 || cellType == 7 || cellType == 8) {
                // Barriers
                int bIdx = -1;
                for (int b = 0; b < activeNumBarriers; b++) {
                    if (activeBarriers[b].row == r && activeBarriers[b].col == c) {
                        bIdx = b;
                        break;
                    }
                }
                
                if (bIdx != -1) {
                    BarrierInfo b = activeBarriers[bIdx];
                    if (!b.open) {
                        // Closed barrier: Glowing energy field with animated plasma ripple
                        DrawRectangle(cellX + 2, cellY + 2, CELL_SIZE - 4, CELL_SIZE - 4, ColorAlpha(b.color, 0.15f));
                        DrawRectangleLinesEx((Rectangle){cellX + 1, cellY + 1, CELL_SIZE - 2, CELL_SIZE - 2}, 1.5f, ColorAlpha(b.color, 0.4f));
                        DrawRectangleLines(cellX + 2, cellY + 2, CELL_SIZE - 4, CELL_SIZE - 4, b.color);
                        
                        // Animated rippling energy waves
                        float plasmaTime = (float)GetTime();
                        for (int offset = 0; offset < CELL_SIZE - 4; offset += 6) {
                            float wave = sinf(plasmaTime * 8.0f + offset * 0.3f) * 3.0f;
                            DrawLine(cellX + 2 + offset, cellY + 2 + wave + CELL_SIZE/2 - 2, cellX + 2 + offset, cellY + 8 + wave + CELL_SIZE/2 - 2, ColorAlpha(b.color, 0.7f));
                        }
                        
                        // Center text label
                        int textW = MeasureText(b.label, 9);
                        DrawText(b.label, cellX + CELL_SIZE/2 - textW/2, cellY + 5, 9, b.color);
                    } else {
                        // Open barrier: Translucent fading outline
                        DrawRectangleLines(cellX + 4, cellY + 4, CELL_SIZE - 8, CELL_SIZE - 8, ColorAlpha(b.color, 0.15f));
                    }
                }
            }
        }
    }
    
    // Draw Player Trail (glowing cyber cyan trail)
    for (int i = trailCount - 1; i >= 0; i--) {
        float factor = (float)(MAX_TRAIL - i) / MAX_TRAIL;
        Color trailColor = ColorAlpha(COLOR_NEON_CYAN, factor * 0.40f); 
        DrawCircleV(playerTrail[i], playerRadius * factor, trailColor);
    }
    
    // Draw Player
    float playerTime = (float)GetTime();
    float glowRadius = playerRadius + 4.0f + sinf(playerTime * 12.0f) * 2.0f;
    DrawCircle(playerPos.x, playerPos.y, glowRadius, ColorAlpha(COLOR_NEON_GREEN, 0.20f)); 
    DrawCircleV(playerPos, playerRadius, COLOR_NEON_GREEN); 
    DrawCircleV(playerPos, playerRadius - 4.0f, (Color){ 187, 247, 208, 255 }); 
    DrawCircleLines(playerPos.x, playerPos.y, playerRadius + 2.0f, ColorAlpha(COLOR_TEXT_MAIN, 0.4f));
    
    // Draw Enemy (Virus)
    if (enemySpawnDelay > 0.0f) {
        // Spawning effect: flashing hazard circle
        float pulse = sinf(GetTime() * 15.0f) * 0.5f + 0.5f;
        DrawCircleLines(enemyPos.x, enemyPos.y, enemyRadius + 8.0f + pulse * 6.0f, ColorAlpha(COLOR_NEON_RED, 0.8f));
        DrawCircle(enemyPos.x, enemyPos.y, enemyRadius + 4.0f, ColorAlpha(COLOR_NEON_RED, 0.2f * pulse));
        DrawText("WARN", enemyPos.x - 14, enemyPos.y - 4, 10, COLOR_NEON_RED);
    } else {
        // Active Enemy: Glowing red glitchy core
        float virusTime = (float)GetTime();
        float glowRadius = enemyRadius + 5.0f + sinf(virusTime * 20.0f) * 3.0f;
        
        // Aura
        DrawCircle(enemyPos.x, enemyPos.y, glowRadius, ColorAlpha(COLOR_NEON_RED, 0.25f));
        
        // Inner spiked glitch triangles
        for (int i = 0; i < 4; i++) {
            float angle = virusTime * 5.0f + i * (PI / 2.0f);
            float gl = 4.0f + sinf(virusTime * 30.0f + i) * 3.0f;
            Vector2 p1 = { enemyPos.x + cosf(angle) * (enemyRadius + gl), enemyPos.y + sinf(angle) * (enemyRadius + gl) };
            Vector2 p2 = { enemyPos.x + cosf(angle + 0.3f) * (enemyRadius - 4.0f), enemyPos.y + sinf(angle + 0.3f) * (enemyRadius - 4.0f) };
            Vector2 p3 = { enemyPos.x + cosf(angle - 0.3f) * (enemyRadius - 4.0f), enemyPos.y + sinf(angle - 0.3f) * (enemyRadius - 4.0f) };
            DrawTriangle(p1, p2, p3, COLOR_NEON_RED);
        }
        
        // Core center
        DrawCircleV(enemyPos, enemyRadius - 2.0f, COLOR_NEON_RED);
        DrawCircleV(enemyPos, enemyRadius - 6.0f, BLACK);
        DrawCircleV(enemyPos, 3.0f, WHITE);
    }
    
    // Ambient vignette corners (vignette overlay for cinematic feel)
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Floating Glassmorphic HUD overlay top panel
    DrawThemeGlassPanel((Rectangle){ 15, 10, SCREEN_WIDTH - 30, CELL_SIZE }, 0.20f, ColorAlpha(COLOR_GRID_LINE, 0.25f));
    
    char hudTitle[128];
    sprintf(hudTitle, "LOGIC RUSH: %s", currentLevelName);
    DrawText(hudTitle, 35, 20, 20, COLOR_TEXT_CYBER);
    
    char timerText[32];
    sprintf(timerText, "Tempo: %.1f s", gameTimer);
    DrawText(timerText, SCREEN_WIDTH - 220, 20, 20, COLOR_TEXT_MAIN);
    
    DrawText("ESC: Voltar ao Menu | WASD/Setas: Mover", SCREEN_WIDTH / 2 - 180, 23, 14, COLOR_TEXT_MUTED);
    
    // Draw level objective at the bottom center
    const char* objText = currentObjective;
    int textW = MeasureText(objText, 14);
    Rectangle tooltip = { SCREEN_WIDTH / 2.0f - textW / 2.0f - 20, SCREEN_HEIGHT - 45, textW + 40, 30 };
    DrawThemeGlassPanel(tooltip, 0.25f, ColorAlpha(COLOR_TEXT_MUTED, 0.4f));
    DrawText(objText, tooltip.x + 20, tooltip.y + 8, 14, COLOR_TEXT_MAIN);
    
    // Draw Intro Pop-up Overlay
    if (isIntroActive) {
        // Draw overlay background (darken the screen)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.75f));
        
        // Main modal container
        Rectangle popupRect = { SCREEN_WIDTH / 2.0f - 300, SCREEN_HEIGHT / 2.0f - 230, 600, 460 };
        DrawThemeGlassPanel(popupRect, 0.10f, COLOR_GRID_LINE);
        
        // Header
        const char* title = "SISTEMA COMPROMETIDO: INVASAO DE VIRUS";
        int titleW = MeasureText(title, 20);
        DrawText(title, SCREEN_WIDTH / 2 - titleW / 2, popupRect.y + 25, 20, COLOR_NEON_RED);
        
        // Underline glow
        DrawLineEx((Vector2){ popupRect.x + 40, popupRect.y + 55 }, (Vector2){ popupRect.x + 560, popupRect.y + 55 }, 2.0f, COLOR_NEON_RED);
        
        // Subtitle / Intro text
        DrawText("Um virus de logica esta comprometendo a CPU central!", popupRect.x + 40, popupRect.y + 70, 15, COLOR_TEXT_MAIN);
        DrawText("Para escapar, avance pelo labirinto resolvendo proposicoes logicas", popupRect.x + 40, popupRect.y + 90, 13, COLOR_TEXT_MUTED);
        DrawText("nas portas bloqueadas antes que o virus te alcance.", popupRect.x + 40, popupRect.y + 108, 13, COLOR_TEXT_MUTED);
        
        // Logic explanation
        DrawText("CONECTIVOS LOGICOS (PORTAS):", popupRect.x + 40, popupRect.y + 138, 14, COLOR_TEXT_CYBER);
        
        DrawText("~P (NOT)", popupRect.x + 50, popupRect.y + 163, 13, COLOR_NEON_CYAN);
        DrawText("Inverte a verdade. (~V = F, ~F = V)", popupRect.x + 180, popupRect.y + 163, 13, COLOR_TEXT_MUTED);
        
        DrawText("P ^ Q (AND)", popupRect.x + 50, popupRect.y + 185, 13, COLOR_NEON_CYAN);
        DrawText("Verdadeiro apenas se ambos forem Verdadeiros.", popupRect.x + 180, popupRect.y + 185, 13, COLOR_TEXT_MUTED);
        
        DrawText("P v Q (OR)", popupRect.x + 50, popupRect.y + 207, 13, COLOR_NEON_CYAN);
        DrawText("Verdadeiro se pelo menos um for Verdadeiro.", popupRect.x + 180, popupRect.y + 207, 13, COLOR_TEXT_MUTED);
        
        DrawText("P [+] Q (XOR)", popupRect.x + 50, popupRect.y + 229, 13, COLOR_NEON_CYAN);
        DrawText("Verdadeiro se os valores forem Diferentes.", popupRect.x + 180, popupRect.y + 229, 13, COLOR_TEXT_MUTED);
        
        DrawText("P -> Q (COND)", popupRect.x + 50, popupRect.y + 251, 13, COLOR_NEON_CYAN);
        DrawText("Condicional: Falso apenas na implicacao V -> F.", popupRect.x + 180, popupRect.y + 251, 13, COLOR_TEXT_MUTED);
        
        DrawText("P <-> Q (BICOND)", popupRect.x + 50, popupRect.y + 273, 13, COLOR_NEON_CYAN);
        DrawText("Bicondicional: Verdadeiro se ambos forem iguais.", popupRect.x + 180, popupRect.y + 273, 13, COLOR_TEXT_MUTED);
        
        // Controls / Warnings
        DrawText("CONTROLES DE RESPOSTA:", popupRect.x + 40, popupRect.y + 305, 14, COLOR_NEON_GREEN);
        DrawText("Pressione [V] para VERDADEIRO ou [F] para FALSO.", popupRect.x + 40, popupRect.y + 327, 13, COLOR_TEXT_MAIN);
        DrawText("(Ou use o mouse para clicar na opcao desejada na tela)", popupRect.x + 40, popupRect.y + 347, 12, COLOR_TEXT_MUTED);
        
        // Button
        Rectangle btnRect = { SCREEN_WIDTH / 2.0f - 100, popupRect.y + 390, 200, 45 };
        Vector2 mousePos = GetMousePosition();
        bool hovered = CheckCollisionPointRec(mousePos, btnRect);
        
        DrawThemeButton(btnRect, "ENTENDIDO", 16, hovered, COLOR_NEON_GREEN);
    }
    
    // Draw Proposition Overlay
    if (isPropositionActive) {
        // Darken background
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
        
        // Glass container for proposition challenge
        Rectangle challengeRect = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 160, 500, 300 };
        
        // Color theme based on the gate type
        Color gateColor = COLOR_NEON_CYAN;
        if (activeBarrierChallengeIdx >= 0 && activeBarrierChallengeIdx < activeNumBarriers) {
            gateColor = activeBarriers[activeBarrierChallengeIdx].color;
        }
        
        DrawThemeGlassPanel(challengeRect, 0.15f, gateColor);
        
        // Challenge title
        const char* challengeTitle = "DECODIFICADOR DE BARREIRA";
        if (activeBarrierChallengeIdx >= 0 && activeBarrierChallengeIdx < activeNumBarriers) {
            challengeTitle = activeBarriers[activeBarrierChallengeIdx].label;
        }
        int titleW = MeasureText(challengeTitle, 20);
        DrawText(challengeTitle, SCREEN_WIDTH / 2 - titleW / 2, challengeRect.y + 25, 20, gateColor);
        
        // Underline
        DrawLineEx((Vector2){ challengeRect.x + 30, challengeRect.y + 55 }, (Vector2){ challengeRect.x + 470, challengeRect.y + 55 }, 1.5f, ColorAlpha(gateColor, 0.5f));
        
        // Question text - split into lines if contains newlines
        char questionBuffer[256];
        strcpy(questionBuffer, currentQuestion.questionText);
        
        char* line1 = strtok(questionBuffer, "\n");
        char* line2 = strtok(NULL, "\n");
        
        if (line1) {
            int line1W = MeasureText(line1, 16);
            DrawText(line1, SCREEN_WIDTH / 2 - line1W / 2, challengeRect.y + 80, 16, COLOR_TEXT_MAIN);
        }
        if (line2) {
            int line2W = MeasureText(line2, 16);
            DrawText(line2, SCREEN_WIDTH / 2 - line2W / 2, challengeRect.y + 110, 16, COLOR_NEON_CYAN);
        }
        
        if (errorCooldownTimer > 0.0f) {
            // Draw a security lockout panel instead of option buttons
            char penaltyText[128];
            sprintf(penaltyText, "BLOQUEIO: AGUARDE %.1fs", errorCooldownTimer);
            int penaltyW = MeasureText(penaltyText, 16);
            DrawText(penaltyText, SCREEN_WIDTH / 2 - penaltyW / 2, challengeRect.y + 190, 16, COLOR_NEON_RED);
            
            const char* descText = "Tentativa incorreta detectada. Sistema temporariamente travado.";
            int descTextW = MeasureText(descText, 12);
            DrawText(descText, SCREEN_WIDTH / 2 - descTextW / 2, challengeRect.y + 225, 12, COLOR_TEXT_MUTED);
        } else {
            // Option buttons V (Verdadeiro) and F (Falso)
            Rectangle btnV = { SCREEN_WIDTH / 2.0f - 140, challengeRect.y + 180, 120, 50 };
            Rectangle btnF = { SCREEN_WIDTH / 2.0f + 20, challengeRect.y + 180, 120, 50 };
            
            Vector2 mousePos = GetMousePosition();
            bool hoveredV = CheckCollisionPointRec(mousePos, btnV);
            bool hoveredF = CheckCollisionPointRec(mousePos, btnF);
            
            DrawThemeButton(btnV, "VERDADEIRO (V)", 14, hoveredV, COLOR_NEON_GREEN);
            DrawThemeButton(btnF, "FALSO (F)", 14, hoveredF, COLOR_NEON_RED);
            
            // Small note at the bottom
            const char* noteText = "Decida se a proposicao acima e V ou F para liberar a porta.";
            int noteW = MeasureText(noteText, 11);
            DrawText(noteText, SCREEN_WIDTH / 2 - noteW / 2, challengeRect.y + 255, 11, COLOR_TEXT_MUTED);
        }
    }
    
    // Draw Game Over Overlay
    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 15, 10, 10, 255 }, 0.90f));
        
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        DrawThemeGlassPanel(card, 0.15f, COLOR_NEON_RED);
        
        const char* failText = "SISTEMA COMPROMETIDO!";
        int failTextW = MeasureText(failText, 30);
        DrawText(failText, SCREEN_WIDTH / 2 - failTextW / 2, card.y + 40, 30, COLOR_NEON_RED);
        
        const char* descText = "O Virus infectou a CPU e parou a execucao.";
        int descTextW = MeasureText(descText, 16);
        DrawText(descText, SCREEN_WIDTH / 2 - descTextW / 2, card.y + 90, 16, COLOR_TEXT_MUTED);
        
        char statsText[64];
        sprintf(statsText, "Tempo de Sobrevivencia: %.1f s", gameTimer);
        int statsTextW = MeasureText(statsText, 16);
        DrawText(statsText, SCREEN_WIDTH / 2 - statsTextW / 2, card.y + 130, 16, COLOR_TEXT_MAIN);
        
        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 180, card.y + 180, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 180, 160, 45 };
        
        Vector2 mousePos = GetMousePosition();
        bool hoveredRestart = CheckCollisionPointRec(mousePos, btnRestart);
        bool hoveredMenu = CheckCollisionPointRec(mousePos, btnMenu);
        
        DrawThemeButton(btnRestart, "REINICIAR (ESP)", 12, hoveredRestart, COLOR_NEON_GREEN);
        DrawThemeButton(btnMenu, "MENU (ESC)", 12, hoveredMenu, COLOR_TEXT_MUTED);
    }
    
    // Draw Win Screen Overlay
    if (gameWon) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 9, 13, 22, 255 }, 0.85f));
        
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        bool isLastLevel = (currentLevelIdx == MAX_LEVELS - 1);
        Color borderCol = isLastLevel ? COLOR_NEON_MAGENTA : COLOR_NEON_GREEN;
        
        DrawThemeGlassPanel(card, 0.15f, borderCol);
        
        const char* winText = isLastLevel ? "SISTEMA RESTAURADO!" : "FASE COMPLETADA!";
        int winTextW = MeasureText(winText, 30);
        DrawText(winText, SCREEN_WIDTH / 2 - winTextW / 2, card.y + 45, 30, borderCol);
        
        char statsText[64];
        sprintf(statsText, "Tempo de Execucao: %.2f segundos", gameTimer);
        int statsTextW = MeasureText(statsText, 16);
        DrawText(statsText, SCREEN_WIDTH / 2 - statsTextW / 2, card.y + 110, 16, COLOR_TEXT_MAIN);
        
        Rectangle btnNext = { SCREEN_WIDTH / 2.0f - 180, card.y + 185, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 185, 160, 45 };
        
        Vector2 mousePos = GetMousePosition();
        bool hoveredNext = CheckCollisionPointRec(mousePos, btnNext);
        bool hoveredMenu = CheckCollisionPointRec(mousePos, btnMenu);
        
        const char* btnNextLabel = isLastLevel ? "REINICIAR (ESP)" : "AVANCAR (ESP)";
        DrawThemeButton(btnNext, btnNextLabel, 12, hoveredNext, COLOR_NEON_GREEN);
        DrawThemeButton(btnMenu, "MENU (ESC)", 12, hoveredMenu, COLOR_TEXT_MUTED);
    }
    
    // Draw Pause Overlay
    if (isGamePaused) {
        // Darken background
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.75f));
        
        // Card container
        Rectangle card = { SCREEN_WIDTH / 2.0f - 200, SCREEN_HEIGHT / 2.0f - 120, 400, 240 };
        DrawThemeGlassPanel(card, 0.15f, COLOR_NEON_CYAN);
        
        // Header
        const char* pauseTitle = "SISTEMA PAUSADO";
        int titleW = MeasureText(pauseTitle, 24);
        DrawText(pauseTitle, SCREEN_WIDTH / 2 - titleW / 2, card.y + 30, 24, COLOR_NEON_CYAN);
        
        // Subtitle
        const char* pauseDesc = "A execucao dos processos foi suspensa.";
        int descW = MeasureText(pauseDesc, 14);
        DrawText(pauseDesc, SCREEN_WIDTH / 2 - descW / 2, card.y + 65, 14, COLOR_TEXT_MUTED);
        
        // Buttons
        Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 150, card.y + 100, 300, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f - 150, card.y + 160, 300, 45 };
        
        Vector2 mousePos = GetMousePosition();
        bool hoveredResume = CheckCollisionPointRec(mousePos, btnResume);
        bool hoveredMenu = CheckCollisionPointRec(mousePos, btnMenu);
        
        DrawThemeButton(btnResume, "RETOMAR (ESC)", 14, hoveredResume, COLOR_NEON_GREEN);
        DrawThemeButton(btnMenu, "MENU (SAIR)", 14, hoveredMenu, COLOR_TEXT_MUTED);
    }
}

void UnloadGameplayScreen(void) {
    // Clean up
}
