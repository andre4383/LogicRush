#ifndef RANKING_H
#define RANKING_H

#include <stdbool.h>

#define RANKING_MAX_ENTRIES 10
#define RANKING_FILE        "logicRushRanking.dat"
#define RANKING_NAME_LEN    16

typedef struct {
    char  name[RANKING_NAME_LEN];
    int   score;
    float time;
    int   phase;   // highest phase reached (1/2/3)
} RankEntry;

typedef struct {
    RankEntry entries[RANKING_MAX_ENTRIES];
    int       count;
} Leaderboard;

// Shared global leaderboard
extern Leaderboard g_leaderboard;

void Ranking_Load(void);
void Ranking_Save(void);
// Returns -1 if score is too low, otherwise returns insertion position
int  Ranking_Add(const char *name, int score, float time, int phase);
bool Ranking_NameExists(const char *name);
bool Ranking_QualifiesForTop10(int score);

#endif
