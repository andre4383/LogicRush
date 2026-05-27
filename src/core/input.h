#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"

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

void Input_DrawCursor(void);

void Input_NotifyDamage(void);
void Input_NotifyPhaseWin(void);
void Input_NotifyComboLife(void);

#endif // INPUT_H
