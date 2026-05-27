#include "../core/ranking.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Leaderboard g_leaderboard = {0};

void Ranking_Load(void) {
    g_leaderboard.count = 0;
    FILE *f = fopen(RANKING_FILE, "rb");
    if (!f) return;
    int n = 0;
    if (fread(&n, sizeof(int), 1, f) == 1 && n > 0 && n <= RANKING_MAX_ENTRIES) {
        g_leaderboard.count = n;
        for (int i = 0; i < n; i++)
            fread(&g_leaderboard.entries[i], sizeof(RankEntry), 1, f);
    }
    fclose(f);
}

void Ranking_Save(void) {
    FILE *f = fopen(RANKING_FILE, "wb");
    if (!f) return;
    fwrite(&g_leaderboard.count, sizeof(int), 1, f);
    for (int i = 0; i < g_leaderboard.count; i++)
        fwrite(&g_leaderboard.entries[i], sizeof(RankEntry), 1, f);
    fclose(f);
}

bool Ranking_NameExists(const char *name) {
    for (int i = 0; i < g_leaderboard.count; i++) {
        // Case-insensitive compare
        int eq = 1;
        for (int j = 0; name[j] || g_leaderboard.entries[i].name[j]; j++) {
            char a = name[j], b = g_leaderboard.entries[i].name[j];
            if (a >= 'a' && a <= 'z') a -= 32;
            if (b >= 'a' && b <= 'z') b -= 32;
            if (a != b) { eq = 0; break; }
        }
        if (eq) return true;
    }
    return false;
}

bool Ranking_QualifiesForTop10(int score) {
    if (g_leaderboard.count < RANKING_MAX_ENTRIES) return true;
    return score > g_leaderboard.entries[g_leaderboard.count-1].score;
}

int Ranking_Add(const char *name, int score, float time, int phase) {
    if (name == NULL || name[0] == '\0') return -1;
    if (Ranking_NameExists(name)) return -1;  // no duplicates
    if (!Ranking_QualifiesForTop10(score)) return -1;

    // Create entry
    RankEntry e = {0};
    strncpy(e.name, name, RANKING_NAME_LEN-1);
    e.name[RANKING_NAME_LEN-1] = '\0';
    // Uppercase name
    for (int i = 0; e.name[i]; i++)
        if (e.name[i] >= 'a' && e.name[i] <= 'z') e.name[i] -= 32;
    e.score = score;
    e.time  = time;
    e.phase = phase;

    // Find insertion position (sorted by score desc)
    int pos = g_leaderboard.count;
    for (int i = 0; i < g_leaderboard.count; i++) {
        if (score > g_leaderboard.entries[i].score) { pos = i; break; }
    }

    // Shift down
    int new_count = g_leaderboard.count < RANKING_MAX_ENTRIES
                    ? g_leaderboard.count + 1 : RANKING_MAX_ENTRIES;
    for (int i = new_count - 1; i > pos; i--)
        g_leaderboard.entries[i] = g_leaderboard.entries[i-1];
    g_leaderboard.entries[pos] = e;
    g_leaderboard.count = new_count;

    Ranking_Save();
    return pos;
}
