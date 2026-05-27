#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"
#include <stdbool.h>

void Input_Init(void);
void Input_Update(float dt);

Vector2 Input_GetPointer(void);
bool Input_PointerPressed(void);
bool Input_PointerDown(void);

Vector2 Input_GetMove(void);

bool Input_PressedConfirm(void);
bool Input_PressedCancel(void);
bool Input_PressedGateTrue(void);
bool Input_PressedGateFalse(void);

bool Input_UsingGamepad(void);
bool Input_IsConnected(void);
const char *Input_GetGamepadName(void);

void Input_FocusReset(int *focus, int count);
void Input_FocusUpdate(int *focus, int count, int cols);
typedef bool (*InputFocusSkipFn)(int index, void *user);
void Input_FocusUpdateSkip(int *focus, int count, int cols, InputFocusSkipFn skip, void *user);

bool Input_ItemHot(int index, Rectangle rect, int *focus, int count, int cols);
bool Input_ItemPressed(int index, Rectangle rect, int *focus, int count, int cols);

void Input_DrawCursor(void);
void Input_DrawDebug(void);

void Input_NotifyDamage(void);
void Input_NotifyPhaseWin(void);
void Input_NotifyComboLife(void);

#endif // INPUT_H
