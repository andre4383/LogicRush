#include "../core/game.h"
#include "../core/screens.h"
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

// Switches
typedef struct {
    int row, col;
    bool active;
    char label;
    const char* desc;
} SwitchInfo;

// Barriers (Gates)
typedef struct {
    int row, col;
    GateType type;
    int inputs[2]; // Indices of switches in the array
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
static SwitchInfo activeSwitches[24];
static int activeNumSwitches = 0;
static BarrierInfo activeBarriers[16];
static int activeNumBarriers = 0;
static int currentLevelIdx = 0;
static char currentLevelName[128];
static char currentObjective[256];

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

// Safety check for switch placement
static bool IsCellSafeForSwitch(Coord sw, Coord barrier) {
    if (sw.r == barrier.r && sw.c == barrier.c) return false;
    if (sw.r == 1 && sw.c == 1) return false;
    if (sw.r == 15 && sw.c == 29) return false;
    if (activeMaze[sw.r][sw.c] != 0) return false;
    
    Coord queue[ROWS * COLS];
    int head = 0, tail = 0;
    
    static bool visited[ROWS][COLS];
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            visited[r][c] = false;
        }
    }
    
    queue[tail++] = (Coord){1, 1};
    visited[1][1] = true;
    
    while (head < tail) {
        Coord curr = queue[head++];
        if (curr.r == sw.r && curr.c == sw.c) {
            return true;
        }
        
        Coord neighbors[4] = {
            {curr.r-1, curr.c}, {curr.r+1, curr.c},
            {curr.r, curr.c-1}, {curr.r, curr.c+1}
        };
        
        for (int i = 0; i < 4; i++) {
            int nr = neighbors[i].r;
            int nc = neighbors[i].c;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if (nr == barrier.r && nc == barrier.c) continue;
                if ((activeMaze[nr][nc] == 0 || activeMaze[nr][nc] == 2 || activeMaze[nr][nc] == 3) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    queue[tail++] = (Coord){nr, nc};
                }
            }
        }
    }
    
    return false;
}

static Coord FindSafeSwitchLocation(Coord barrier) {
    for (int attempts = 0; attempts < 1000; attempts++) {
        int r = GetRandomValue(1, 15);
        int c = GetRandomValue(1, 29);
        if (activeMaze[r][c] == 0) {
            Coord sw = {r, c};
            if (IsCellSafeForSwitch(sw, barrier)) {
                bool overlaps = false;
                for (int i = 0; i < activeNumSwitches; i++) {
                    if (activeSwitches[i].row == r && activeSwitches[i].col == c) {
                        overlaps = true;
                        break;
                    }
                }
                if (!overlaps) return sw;
            }
        }
    }
    for (int r = 1; r <= 15; r++) {
        for (int c = 1; c <= 29; c++) {
            if (activeMaze[r][c] == 0 && !(r == barrier.r && c == barrier.c) && !(r == 1 && c == 1) && !(r == 15 && c == 29)) {
                return (Coord){r, c};
            }
        }
    }
    return (Coord){1, 1};
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
    activeNumSwitches = 0;
    activeNumBarriers = 0;
    
    int level = currentLevelIdx + 1;
    sprintf(currentLevelName, "FASE %d", level);
    
    if (level == 1) {
        // NOT + AND
        int barIdx1 = pathLen / 3;
        int barIdx2 = (2 * pathLen) / 3;
        
        Coord barPos1 = mainPath[barIdx1];
        Coord barPos2 = mainPath[barIdx2];
        
        activeMaze[barPos1.r][barPos1.c] = 7; // NOT Barrier
        activeMaze[barPos2.r][barPos2.c] = 5; // AND Barrier
        
        Coord swPosE = FindSafeSwitchLocation(barPos1);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosE.r, swPosE.c, true, 'E', "Entrada da Porta NOT"};
        
        Coord swPosA = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosA.r, swPosA.c, false, 'A', "Entrada 1 da Porta AND"};
        Coord swPosB = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosB.r, swPosB.c, false, 'B', "Entrada 2 da Porta AND"};
        
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos1.r, barPos1.c, GATE_NOT, {0, -1}, false, (Color){ 249, 115, 22, 255 }, "PORTA NOT"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos2.r, barPos2.c, GATE_AND, {1, 2}, false, (Color){ 59, 130, 246, 255 }, "PORTA AND"
        };
        
        strcpy(currentObjective, "FASE 1: Desative E (NOT) e Ative A/B (AND) para avançar.");
    }
    else if (level == 2) {
        // NOT + AND + OR
        int barIdx1 = pathLen / 4;
        int barIdx2 = (2 * pathLen) / 4;
        int barIdx3 = (3 * pathLen) / 4;
        
        Coord barPos1 = mainPath[barIdx1];
        Coord barPos2 = mainPath[barIdx2];
        Coord barPos3 = mainPath[barIdx3];
        
        activeMaze[barPos1.r][barPos1.c] = 7; // NOT Barrier
        activeMaze[barPos2.r][barPos2.c] = 5; // AND Barrier
        activeMaze[barPos3.r][barPos3.c] = 8; // OR Barrier
        
        Coord swPosE = FindSafeSwitchLocation(barPos1);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosE.r, swPosE.c, true, 'E', "Entrada da Porta NOT"};
        
        Coord swPosA = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosA.r, swPosA.c, false, 'A', "Entrada 1 da Porta AND"};
        Coord swPosB = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosB.r, swPosB.c, false, 'B', "Entrada 2 da Porta AND"};
        
        Coord swPosC = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosC.r, swPosC.c, false, 'C', "Entrada 1 da Porta OR"};
        Coord swPosD = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosD.r, swPosD.c, false, 'D', "Entrada 2 da Porta OR"};
        
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos1.r, barPos1.c, GATE_NOT, {0, -1}, false, (Color){ 249, 115, 22, 255 }, "PORTA NOT"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos2.r, barPos2.c, GATE_AND, {1, 2}, false, (Color){ 59, 130, 246, 255 }, "PORTA AND"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos3.r, barPos3.c, GATE_OR, {3, 4}, false, (Color){ 234, 179, 8, 255 }, "PORTA OR"
        };
        
        strcpy(currentObjective, "FASE 2: Abra NOT(E), AND(A/B) e OR(C/D).");
    }
    else if (level == 3) {
        // NOT + AND + XOR + OR
        int barIdx1 = pathLen / 5;
        int barIdx2 = (2 * pathLen) / 5;
        int barIdx3 = (3 * pathLen) / 5;
        int barIdx4 = (4 * pathLen) / 5;
        
        Coord barPos1 = mainPath[barIdx1];
        Coord barPos2 = mainPath[barIdx2];
        Coord barPos3 = mainPath[barIdx3];
        Coord barPos4 = mainPath[barIdx4];
        
        activeMaze[barPos1.r][barPos1.c] = 7; // NOT
        activeMaze[barPos2.r][barPos2.c] = 5; // AND
        activeMaze[barPos3.r][barPos3.c] = 6; // XOR
        activeMaze[barPos4.r][barPos4.c] = 8; // OR
        
        Coord swPosF = FindSafeSwitchLocation(barPos1);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosF.r, swPosF.c, true, 'F', "Entrada da Porta NOT"};
        
        Coord swPosA = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosA.r, swPosA.c, false, 'A', "Entrada 1 da AND"};
        Coord swPosB = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosB.r, swPosB.c, false, 'B', "Entrada 2 da AND"};
        
        Coord swPosC = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosC.r, swPosC.c, false, 'C', "Entrada 1 da XOR"};
        Coord swPosD = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosD.r, swPosD.c, false, 'D', "Entrada 2 da XOR"};
        
        Coord swPosE = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosE.r, swPosE.c, false, 'E', "Entrada 1 da OR"};
        Coord swPosG = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosG.r, swPosG.c, false, 'G', "Entrada 2 da OR"};
        
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos1.r, barPos1.c, GATE_NOT, {0, -1}, false, (Color){ 249, 115, 22, 255 }, "PORTA NOT"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos2.r, barPos2.c, GATE_AND, {1, 2}, false, (Color){ 59, 130, 246, 255 }, "PORTA AND"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos3.r, barPos3.c, GATE_XOR, {3, 4}, false, (Color){ 168, 85, 247, 255 }, "PORTA XOR"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos4.r, barPos4.c, GATE_OR, {5, 6}, false, (Color){ 234, 179, 8, 255 }, "PORTA OR"
        };
        
        strcpy(currentObjective, "FASE 3: Abra NOT(F), AND(A/B), XOR(C/D) e OR(E/G)!");
    }
    else if (level == 4) {
        // NOT 1 + NOT 2 + AND 1 + AND 2 + XOR + OR
        int barIdx1 = pathLen / 7;
        int barIdx2 = (2 * pathLen) / 7;
        int barIdx3 = (3 * pathLen) / 7;
        int barIdx4 = (4 * pathLen) / 7;
        int barIdx5 = (5 * pathLen) / 7;
        int barIdx6 = (6 * pathLen) / 7;
        
        Coord barPos1 = mainPath[barIdx1];
        Coord barPos2 = mainPath[barIdx2];
        Coord barPos3 = mainPath[barIdx3];
        Coord barPos4 = mainPath[barIdx4];
        Coord barPos5 = mainPath[barIdx5];
        Coord barPos6 = mainPath[barIdx6];
        
        activeMaze[barPos1.r][barPos1.c] = 7; // NOT 1
        activeMaze[barPos2.r][barPos2.c] = 7; // NOT 2
        activeMaze[barPos3.r][barPos3.c] = 5; // AND 1
        activeMaze[barPos4.r][barPos4.c] = 5; // AND 2
        activeMaze[barPos5.r][barPos5.c] = 6; // XOR
        activeMaze[barPos6.r][barPos6.c] = 8; // OR
        
        Coord swPosH = FindSafeSwitchLocation(barPos1);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosH.r, swPosH.c, true, 'H', "Entrada da NOT 1"};
        
        Coord swPosI = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosI.r, swPosI.c, true, 'I', "Entrada da NOT 2"};
        
        Coord swPosA = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosA.r, swPosA.c, false, 'A', "Entrada 1 da AND 1"};
        Coord swPosB = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosB.r, swPosB.c, false, 'B', "Entrada 2 da AND 1"};
        
        Coord swPosC = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosC.r, swPosC.c, false, 'C', "Entrada 1 da AND 2"};
        Coord swPosD = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosD.r, swPosD.c, false, 'D', "Entrada 2 da AND 2"};
        
        Coord swPosE = FindSafeSwitchLocation(barPos5);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosE.r, swPosE.c, false, 'E', "Entrada 1 da XOR"};
        Coord swPosF = FindSafeSwitchLocation(barPos5);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosF.r, swPosF.c, false, 'F', "Entrada 2 da XOR"};
        
        Coord swPosG = FindSafeSwitchLocation(barPos6);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosG.r, swPosG.c, false, 'G', "Entrada 1 da OR"};
        Coord swPosJ = FindSafeSwitchLocation(barPos6);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosJ.r, swPosJ.c, false, 'J', "Entrada 2 da OR"};
        
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos1.r, barPos1.c, GATE_NOT, {0, -1}, false, (Color){ 249, 115, 22, 255 }, "PORTA NOT 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos2.r, barPos2.c, GATE_NOT, {1, -1}, false, (Color){ 244, 63, 94, 255 }, "PORTA NOT 2"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos3.r, barPos3.c, GATE_AND, {2, 3}, false, (Color){ 59, 130, 246, 255 }, "PORTA AND 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos4.r, barPos4.c, GATE_AND, {4, 5}, false, (Color){ 34, 211, 238, 255 }, "PORTA AND 2"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos5.r, barPos5.c, GATE_XOR, {6, 7}, false, (Color){ 168, 85, 247, 255 }, "PORTA XOR"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos6.r, barPos6.c, GATE_OR, {8, 9}, false, (Color){ 234, 179, 8, 255 }, "PORTA OR"
        };
        
        strcpy(currentObjective, "FASE 4: Cruze NOTs(H/I), ANDs(A/B, C/D), XOR(E/F) e OR(G/J)!");
    }
    else {
        // NOT 1 + NOT 2 + AND 1 + AND 2 + XOR 1 + XOR 2 + OR 1 + OR 2 (8 barriers, 14 switches)
        int barIdx1 = pathLen / 9;
        int barIdx2 = (2 * pathLen) / 9;
        int barIdx3 = (3 * pathLen) / 9;
        int barIdx4 = (4 * pathLen) / 9;
        int barIdx5 = (5 * pathLen) / 9;
        int barIdx6 = (6 * pathLen) / 9;
        int barIdx7 = (7 * pathLen) / 9;
        int barIdx8 = (8 * pathLen) / 9;
        
        Coord barPos1 = mainPath[barIdx1];
        Coord barPos2 = mainPath[barIdx2];
        Coord barPos3 = mainPath[barIdx3];
        Coord barPos4 = mainPath[barIdx4];
        Coord barPos5 = mainPath[barIdx5];
        Coord barPos6 = mainPath[barIdx6];
        Coord barPos7 = mainPath[barIdx7];
        Coord barPos8 = mainPath[barIdx8];
        
        activeMaze[barPos1.r][barPos1.c] = 7; // NOT 1
        activeMaze[barPos2.r][barPos2.c] = 7; // NOT 2
        activeMaze[barPos3.r][barPos3.c] = 5; // AND 1
        activeMaze[barPos4.r][barPos4.c] = 5; // AND 2
        activeMaze[barPos5.r][barPos5.c] = 6; // XOR 1
        activeMaze[barPos6.r][barPos6.c] = 6; // XOR 2
        activeMaze[barPos7.r][barPos7.c] = 8; // OR 1
        activeMaze[barPos8.r][barPos8.c] = 8; // OR 2
        
        Coord swPosK = FindSafeSwitchLocation(barPos1);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosK.r, swPosK.c, true, 'K', "NOT 1"};
        Coord swPosL = FindSafeSwitchLocation(barPos2);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosL.r, swPosL.c, true, 'L', "NOT 2"};
        
        Coord swPosA = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosA.r, swPosA.c, false, 'A', "AND 1 IN 1"};
        Coord swPosB = FindSafeSwitchLocation(barPos3);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosB.r, swPosB.c, false, 'B', "AND 1 IN 2"};
        
        Coord swPosC = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosC.r, swPosC.c, false, 'C', "AND 2 IN 1"};
        Coord swPosD = FindSafeSwitchLocation(barPos4);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosD.r, swPosD.c, false, 'D', "AND 2 IN 2"};
        
        Coord swPosE = FindSafeSwitchLocation(barPos5);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosE.r, swPosE.c, false, 'E', "XOR 1 IN 1"};
        Coord swPosF = FindSafeSwitchLocation(barPos5);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosF.r, swPosF.c, false, 'F', "XOR 1 IN 2"};
        
        Coord swPosG = FindSafeSwitchLocation(barPos6);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosG.r, swPosG.c, false, 'G', "XOR 2 IN 1"};
        Coord swPosH = FindSafeSwitchLocation(barPos6);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosH.r, swPosH.c, false, 'H', "XOR 2 IN 2"};
        
        Coord swPosI = FindSafeSwitchLocation(barPos7);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosI.r, swPosI.c, false, 'I', "OR 1 IN 1"};
        Coord swPosJ = FindSafeSwitchLocation(barPos7);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosJ.r, swPosJ.c, false, 'J', "OR 1 IN 2"};
        
        Coord swPosM = FindSafeSwitchLocation(barPos8);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosM.r, swPosM.c, false, 'M', "OR 2 IN 1"};
        Coord swPosN = FindSafeSwitchLocation(barPos8);
        activeSwitches[activeNumSwitches++] = (SwitchInfo){swPosN.r, swPosN.c, false, 'N', "OR 2 IN 2"};
        
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos1.r, barPos1.c, GATE_NOT, {0, -1}, false, (Color){ 249, 115, 22, 255 }, "PORTA NOT 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos2.r, barPos2.c, GATE_NOT, {1, -1}, false, (Color){ 244, 63, 94, 255 }, "PORTA NOT 2"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos3.r, barPos3.c, GATE_AND, {2, 3}, false, (Color){ 59, 130, 246, 255 }, "PORTA AND 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos4.r, barPos4.c, GATE_AND, {4, 5}, false, (Color){ 34, 211, 238, 255 }, "PORTA AND 2"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos5.r, barPos5.c, GATE_XOR, {6, 7}, false, (Color){ 168, 85, 247, 255 }, "PORTA XOR 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos6.r, barPos6.c, GATE_XOR, {8, 9}, false, (Color){ 192, 132, 252, 255 }, "PORTA XOR 2"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos7.r, barPos7.c, GATE_OR, {10, 11}, false, (Color){ 234, 179, 8, 255 }, "PORTA OR 1"
        };
        activeBarriers[activeNumBarriers++] = (BarrierInfo){
            barPos8.r, barPos8.c, GATE_OR, {12, 13}, false, (Color){ 253, 224, 71, 255 }, "PORTA OR 2"
        };
        
        strcpy(currentObjective, "DESAFIO FINAL: Resolva 8 portas lógicas sequenciais para escapar!");
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

// Current active switch near player (-1 if none)
static int activeSwitchIdx = -1;

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

// Evaluate logic gate states for active barriers
static void EvaluateLogicGates(void) {
    for (int i = 0; i < activeNumBarriers; i++) {
        BarrierInfo* b = &activeBarriers[i];
        if (b->type == GATE_NOT) {
            int swIdx = b->inputs[0];
            if (swIdx != -1 && swIdx < activeNumSwitches) {
                b->open = !activeSwitches[swIdx].active;
            }
        } else if (b->type == GATE_AND) {
            int swIdx1 = b->inputs[0];
            int swIdx2 = b->inputs[1];
            if (swIdx1 != -1 && swIdx1 < activeNumSwitches && swIdx2 != -1 && swIdx2 < activeNumSwitches) {
                b->open = activeSwitches[swIdx1].active && activeSwitches[swIdx2].active;
            }
        } else if (b->type == GATE_OR) {
            int swIdx1 = b->inputs[0];
            int swIdx2 = b->inputs[1];
            if (swIdx1 != -1 && swIdx1 < activeNumSwitches && swIdx2 != -1 && swIdx2 < activeNumSwitches) {
                b->open = activeSwitches[swIdx1].active || activeSwitches[swIdx2].active;
            }
        } else if (b->type == GATE_XOR) {
            int swIdx1 = b->inputs[0];
            int swIdx2 = b->inputs[1];
            if (swIdx1 != -1 && swIdx1 < activeNumSwitches && swIdx2 != -1 && swIdx2 < activeNumSwitches) {
                b->open = (activeSwitches[swIdx1].active != activeSwitches[swIdx2].active);
            }
        }
    }
}

// Reset level state and load active level data
static void ResetLevel(void) {
    gameTimer = 0.0f;
    gameWon = false;
    trailCount = 0;
    activeSwitchIdx = -1;
    
    // Dynamically generate the maze and level elements!
    GenerateProceduralLevel();
    
    // Player spawn point is always (1,1) in the generator
    playerPos.x = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    playerPos.y = 1 * CELL_SIZE + CELL_SIZE / 2.0f;
    
    // Reset trail
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
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
        }
        return;
    }
    
    // Update timer
    gameTimer += GetFrameTime();
    
    // Pulsate target goal and elements
    globalPulse += 0.03f * globalPulseDir;
    if (globalPulse >= 1.0f) {
        globalPulse = 1.0f;
        globalPulseDir = -1;
    } else if (globalPulse <= 0.0f) {
        globalPulse = 0.0f;
        globalPulseDir = 1;
    }
    
    // Evaluate logic gate states for barriers
    EvaluateLogicGates();
    
    // Movement inputs
    float dt = GetFrameTime();
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
    
    // Check switch collision/proximity (interact with E)
    activeSwitchIdx = -1;
    for (int i = 0; i < activeNumSwitches; i++) {
        float swX = activeSwitches[i].col * CELL_SIZE + CELL_SIZE / 2.0f;
        float swY = activeSwitches[i].row * CELL_SIZE + CELL_SIZE / 2.0f;
        float dist = sqrtf((playerPos.x - swX) * (playerPos.x - swX) + (playerPos.y - swY) * (playerPos.y - swY));
        
        if (dist < 32.0f) {
            activeSwitchIdx = i;
            if (IsKeyPressed(KEY_E)) {
                activeSwitches[i].active = !activeSwitches[i].active;
            }
            break;
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
    
    // Quick exit to menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentScreen = SCREEN_TITLE;
    }
}

// Draw circuit trace with right angles and moving electrons
static void DrawCircuitWire(Vector2 start, Vector2 end, bool active) {
    Color col = active ? (Color){ 34, 197, 94, 255 } : (Color){ 71, 85, 105, 255 }; // active green, inactive slate
    Color glowCol = active ? ColorAlpha((Color){ 34, 197, 94, 255 }, 0.25f) : ColorAlpha((Color){ 71, 85, 105, 255 }, 0.05f);
    Vector2 mid = { end.x, start.y };
    
    // Glow line (thicker)
    DrawLineEx(start, mid, 6.0f, glowCol);
    DrawLineEx(mid, end, 6.0f, glowCol);
    
    // Core line (thinner)
    DrawLineEx(start, mid, 2.5f, col);
    DrawLineEx(mid, end, 2.5f, col);
    
    // Draw animated electrons flowing along the path if active
    if (active) {
        float time = (float)GetTime();
        float speed = 120.0f; // pixels per second
        
        float segment1 = fabsf(mid.x - start.x);
        float segment2 = fabsf(end.y - mid.y);
        float totalLen = segment1 + segment2;
        
        if (totalLen > 0) {
            for (int e = 0; e < 2; e++) {
                float progress = fmodf(time * speed + e * (totalLen / 2.0f), totalLen);
                Vector2 electronPos;
                
                if (progress < segment1) {
                    float t = progress / segment1;
                    electronPos.x = start.x + (mid.x - start.x) * t;
                    electronPos.y = start.y;
                } else {
                    float t = (progress - segment1) / segment2;
                    electronPos.x = mid.x;
                    electronPos.y = mid.y + (end.y - mid.y) * t;
                }
                
                DrawCircleV(electronPos, 3.5f, (Color){ 241, 245, 249, 255 });
                DrawCircleV(electronPos, 6.0f, ColorAlpha((Color){ 74, 222, 128, 255 }, 0.6f));
            }
        }
    }
}

void DrawGameplayScreen(void) {
    // Cyber Cadet dark blue/black background
    ClearBackground((Color){ 10, 15, 26, 255 }); 
    
    // Draw background grid lines (cyber style)
    for (int x = 0; x < SCREEN_WIDTH; x += CELL_SIZE) {
        Color col = (x % (CELL_SIZE * 4) == 0) ? ColorAlpha((Color){ 59, 130, 246, 255 }, 0.08f) : ColorAlpha((Color){ 59, 130, 246, 255 }, 0.03f);
        DrawLine(x, CELL_SIZE, x, SCREEN_HEIGHT, col);
    }
    for (int y = CELL_SIZE; y < SCREEN_HEIGHT; y += CELL_SIZE) {
        Color col = (y % (CELL_SIZE * 4) == 0) ? ColorAlpha((Color){ 59, 130, 246, 255 }, 0.08f) : ColorAlpha((Color){ 59, 130, 246, 255 }, 0.03f);
        DrawLine(0, y, SCREEN_WIDTH, y, col);
    }
    
    // Floating tech particles in the background (ambient depth)
    float timeVal = (float)GetTime();
    for (int i = 0; i < 24; i++) {
        float phaseX = sinf(timeVal * 0.4f + i * 2.3f);
        float phaseY = cosf(timeVal * 0.3f + i * 1.7f);
        float px = (i * 81 + (int)(phaseX * 140.0f)) % SCREEN_WIDTH;
        float py = (i * 53 + (int)(phaseY * 110.0f)) % (SCREEN_HEIGHT - CELL_SIZE) + CELL_SIZE;
        float size = 1.0f + fabsf(sinf(timeVal + i)) * 1.5f;
        DrawCircle(px, py, size, ColorAlpha((Color){ 96, 165, 250, 255 }, 0.12f));
        DrawCircleLines(px, py, size + 2.0f, ColorAlpha((Color){ 96, 165, 250, 255 }, 0.04f));
    }
    
    // Draw circuit wires on the floor under everything
    for (int i = 0; i < activeNumBarriers; i++) {
        BarrierInfo b = activeBarriers[i];
        Vector2 endPos = { b.col * CELL_SIZE + CELL_SIZE / 2.0f, b.row * CELL_SIZE + CELL_SIZE / 2.0f };
        
        for (int j = 0; j < 2; j++) {
            int swIdx = b.inputs[j];
            if (swIdx != -1 && swIdx < activeNumSwitches) {
                SwitchInfo sw = activeSwitches[swIdx];
                Vector2 startPos = { sw.col * CELL_SIZE + CELL_SIZE / 2.0f, sw.row * CELL_SIZE + CELL_SIZE / 2.0f };
                DrawCircuitWire(startPos, endPos, sw.active);
            }
        }
    }
    
    // Draw Maze Cells, Walls, and Barriers
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float cellX = c * CELL_SIZE;
            float cellY = r * CELL_SIZE;
            int cellType = activeMaze[r][c];
            
            if (cellType == 1) {
                // Wall - Cybertech Panel look
                DrawRectangle(cellX, cellY, CELL_SIZE, CELL_SIZE, (Color){ 13, 20, 35, 255 }); 
                DrawRectangleLines(cellX, cellY, CELL_SIZE, CELL_SIZE, (Color){ 20, 30, 50, 255 });
                
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
                
                DrawCircle(cx, cy, 18.0f + globalPulse * 4.0f, ColorAlpha((Color){ 217, 70, 239, 255 }, 0.20f)); // Aura
                DrawCircleLines(cx, cy, 14.0f + sinf(portalTime * 6.0f) * 2.0f, (Color){ 236, 72, 153, 255 }); // Rotating ring 1
                DrawCircleLines(cx, cy, 10.0f - sinf(portalTime * 6.0f) * 2.0f, (Color){ 168, 85, 247, 255 }); // Rotating ring 2
                
                // Cyber spark lines radiating outward
                for (int i = 0; i < 4; i++) {
                    float angle = portalTime * 3.0f + i * (PI / 2.0f);
                    Vector2 start = { cx + cosf(angle) * 4.0f, cy + sinf(angle) * 4.0f };
                    Vector2 end = { cx + cosf(angle) * 12.0f, cy + sinf(angle) * 12.0f };
                    DrawLineEx(start, end, 1.5f, ColorAlpha((Color){ 217, 70, 239, 255 }, 0.6f));
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
    
    // Draw Switches on the floor
    for (int i = 0; i < activeNumSwitches; i++) {
        SwitchInfo sw = activeSwitches[i];
        float cx = sw.col * CELL_SIZE + CELL_SIZE / 2.0f;
        float cy = sw.row * CELL_SIZE + CELL_SIZE / 2.0f;
        
        Color lightCol = sw.active ? (Color){ 34, 197, 94, 255 } : (Color){ 239, 68, 68, 255 }; 
        
        // tech terminal plate
        DrawRectangle(sw.col * CELL_SIZE + 4, sw.row * CELL_SIZE + 4, CELL_SIZE - 8, CELL_SIZE - 8, (Color){ 20, 27, 45, 255 });
        DrawRectangleRoundedLines((Rectangle){(float)(sw.col * CELL_SIZE + 4), (float)(sw.row * CELL_SIZE + 4), (float)(CELL_SIZE - 8), (float)(CELL_SIZE - 8)}, 0.15f, 4, ColorAlpha(lightCol, 0.45f));
        
        // Outer light ring
        DrawCircleLines(cx, cy, 13.0f, ColorAlpha(lightCol, 0.35f));
        
        // Inner button core
        DrawCircle(cx, cy, 6.0f, (Color){ 10, 15, 26, 255 });
        DrawCircle(cx, cy, 4.0f, lightCol);
        DrawCircle(cx, cy, 1.5f, WHITE);
        
        // Proximity radar pulse
        if (activeSwitchIdx == i) {
            float pulseScale = 14.0f + sinf(GetTime() * 10.0f) * 4.0f;
            DrawCircleLines(cx, cy, pulseScale, ColorAlpha(lightCol, 0.7f));
        }
        
        // Label
        char labelStr[2] = { sw.label, '\0' };
        DrawText(labelStr, cx - 4, cy - 20, 10, (Color){ 148, 163, 184, 255 });
    }
    
    // Draw Player Trail (glowing cyber cyan trail)
    for (int i = trailCount - 1; i >= 0; i--) {
        float factor = (float)(MAX_TRAIL - i) / MAX_TRAIL;
        Color trailColor = ColorAlpha((Color){ 34, 211, 238, 255 }, factor * 0.40f); 
        DrawCircleV(playerTrail[i], playerRadius * factor, trailColor);
    }
    
    // Draw Player
    float playerTime = (float)GetTime();
    float glowRadius = playerRadius + 4.0f + sinf(playerTime * 12.0f) * 2.0f;
    DrawCircle(playerPos.x, playerPos.y, glowRadius, ColorAlpha((Color){ 34, 197, 94, 255 }, 0.20f)); 
    DrawCircleV(playerPos, playerRadius, (Color){ 34, 197, 94, 255 }); 
    DrawCircleV(playerPos, playerRadius - 4.0f, (Color){ 187, 247, 208, 255 }); 
    DrawCircleLines(playerPos.x, playerPos.y, playerRadius + 2.0f, ColorAlpha((Color){ 241, 245, 249, 255 }, 0.4f));
    
    // Draw Glowing Laser Beam connecting player and switch when nearby
    if (activeSwitchIdx != -1) {
        SwitchInfo sw = activeSwitches[activeSwitchIdx];
        Vector2 swPos = { sw.col * CELL_SIZE + CELL_SIZE / 2.0f, sw.row * CELL_SIZE + CELL_SIZE / 2.0f };
        float beamTime = (float)GetTime();
        Color beamColor = ColorAlpha((Color){ 34, 211, 238, 255 }, 0.45f + sinf(beamTime * 15.0f) * 0.15f);
        DrawLineEx(playerPos, swPos, 2.0f, beamColor);
        DrawCircleV(swPos, 16.0f + sinf(beamTime * 12.0f) * 3.0f, ColorAlpha((Color){ 34, 211, 238, 255 }, 0.18f));
    }
    
    // Ambient vignette corners (vignette overlay for cinematic feel)
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, 40, ColorAlpha(BLACK, 0.45f), ColorAlpha(BLACK, 0.0f));
    DrawRectangleGradientV(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, ColorAlpha(BLACK, 0.0f), ColorAlpha(BLACK, 0.45f));
    DrawRectangleGradientH(0, 0, 40, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.45f), ColorAlpha(BLACK, 0.0f));
    DrawRectangleGradientH(SCREEN_WIDTH - 40, 0, 40, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.0f), ColorAlpha(BLACK, 0.45f));
    
    // Floating Glassmorphic HUD overlay top panel
    DrawRectangleRounded((Rectangle){ 15, 10, SCREEN_WIDTH - 30, CELL_SIZE }, 0.20f, 4, ColorAlpha((Color){ 13, 20, 35, 255 }, 0.85f));
    DrawRectangleRoundedLines((Rectangle){ 15, 10, SCREEN_WIDTH - 30, CELL_SIZE }, 0.20f, 4, ColorAlpha((Color){ 59, 130, 246, 255 }, 0.25f));
    
    char hudTitle[128];
    sprintf(hudTitle, "LOGIC RUSH: %s", currentLevelName);
    DrawText(hudTitle, 35, 20, 20, (Color){ 96, 165, 250, 255 });
    
    char timerText[32];
    sprintf(timerText, "Tempo: %.1f s", gameTimer);
    DrawText(timerText, SCREEN_WIDTH - 220, 20, 20, (Color){ 241, 245, 249, 255 });
    
    DrawText("ESC: Voltar ao Menu | WASD / Setas: Mover", SCREEN_WIDTH / 2 - 180, 23, 14, (Color){ 148, 163, 184, 255 });
    
    // Draw Prompt to Toggle Switch
    if (activeSwitchIdx != -1) {
        SwitchInfo sw = activeSwitches[activeSwitchIdx];
        char promptText[128];
        sprintf(promptText, "Pressione [E] para alternar o interruptor %c (%s) -> %s", 
                sw.label, sw.desc, sw.active ? "DESLIGAR" : "LIGAR");
        
        int textW = MeasureText(promptText, 16);
        
        // Translucent background card for tooltip
        Rectangle tooltip = { SCREEN_WIDTH / 2.0f - textW / 2.0f - 20, SCREEN_HEIGHT - 65, textW + 40, 40 };
        DrawRectangleRounded(tooltip, 0.25f, 4, ColorAlpha((Color){ 15, 23, 42, 255 }, 0.85f));
        DrawRectangleRoundedLines(tooltip, 0.25f, 4, (Color){ 59, 130, 246, 255 });
        
        DrawText(promptText, tooltip.x + 20, tooltip.y + 12, 16, (Color){ 241, 245, 249, 255 });
    } else {
        // Draw level objective at the bottom center
        const char* objText = currentObjective;
        int textW = MeasureText(objText, 14);
        Rectangle tooltip = { SCREEN_WIDTH / 2.0f - textW / 2.0f - 20, SCREEN_HEIGHT - 45, textW + 40, 30 };
        DrawRectangleRounded(tooltip, 0.25f, 4, ColorAlpha((Color){ 15, 23, 42, 255 }, 0.85f));
        DrawRectangleRoundedLines(tooltip, 0.25f, 4, (Color){ 71, 85, 105, 255 });
        DrawText(objText, tooltip.x + 20, tooltip.y + 8, 14, (Color){ 226, 232, 240, 255 });
    }
    
    // Draw Win Screen Overlay
    if (gameWon) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 9, 13, 22, 255 }, 0.85f));
        
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        DrawRectangleRounded(card, 0.15f, 4, (Color){ 17, 24, 39, 255 });
        
        bool isLastLevel = (currentLevelIdx == MAX_LEVELS - 1);
        Color borderCol = isLastLevel ? (Color){ 217, 70, 239, 255 } : (Color){ 16, 185, 129, 255 }; // Purple for final win, green for stage win
        DrawRectangleRoundedLines(card, 0.15f, 4, borderCol);
        
        const char* winText = isLastLevel ? "JOGO CONCLUÍDO!" : "FASE COMPLETADA!";
        int winTextW = MeasureText(winText, 32);
        DrawText(winText, SCREEN_WIDTH / 2 - winTextW / 2, card.y + 45, 32, isLastLevel ? (Color){ 217, 70, 239, 255 } : (Color){ 34, 197, 94, 255 });
        
        char statsText[64];
        sprintf(statsText, "Tempo Final: %.2f segundos", gameTimer);
        int statsTextW = MeasureText(statsText, 18);
        DrawText(statsText, SCREEN_WIDTH / 2 - statsTextW / 2, card.y + 110, 18, (Color){ 226, 232, 240, 255 });
        
        const char* playAgainText = isLastLevel ? "Pressione ESPAÇO para reiniciar o jogo" : "Pressione ESPAÇO para ir para a próxima fase";
        int playAgainTextW = MeasureText(playAgainText, 16);
        DrawText(playAgainText, SCREEN_WIDTH / 2 - playAgainTextW / 2, card.y + 180, 16, (Color){ 16, 185, 129, 255 });
        
        const char* exitText = "Pressione ESC para voltar ao Menu Principal";
        int exitTextW = MeasureText(exitText, 16);
        DrawText(exitText, SCREEN_WIDTH / 2 - exitTextW / 2, card.y + 220, 16, (Color){ 148, 163, 184, 255 });
    }
}

void UnloadGameplayScreen(void) {
    // Clean up
}
