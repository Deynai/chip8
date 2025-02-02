#pragma once

#include <stack>
#include <stdexcept>
#include "config.h"

class Chip8
{
	static const int FONT_OFFSET = 0x50;

public:
	Chip8(uint16_t height, uint16_t width);
	~Chip8();

	bool const *const * GetPixelGrid() { return _pixelGrid; } // read-only access
	bool DisplayChanged() { return _displayChanged; }
	void ResetDisplayChanged() { _displayChanged = false; }
	void SetInput(uint8_t key, bool value) { _input[key] = value; };

	void LoadRom(char* rom, uint16_t length);
	void ProcessorTick();
	void DisplayTick();

private:
	bool** _pixelGrid = NULL;
	uint8_t _ram[4096];
	uint16_t _pc = 0;
	uint16_t _I = 0;
	std::stack<uint16_t> _stack;
	uint8_t _delayTimer = 0;
	uint8_t _soundTimer = 0;
	uint8_t _vars[16];
	uint8_t _input[16];
	bool _displayChanged = true;
	uint16_t _width;
	uint16_t _height;

	// Initialisation
	void Initialise();
	void InitialiseFont();
	void InitialisePixelGrid();
	void InitialiseVariableRegisters();
	void InitialisePointers();
	void InitialiseInput();

	// Cleanup
	void DestroyPixelGrid();

	// TODO: inline where it makes sense
	// Ops
	void SubroutineCall(int16_t address);
	void SubroutineReturn();
	void ClearScreen();
	void Jump(uint16_t nnn);
	void SetRegister(uint8_t x, uint8_t nn);
	void AddValueToRegister(uint8_t x, uint8_t nn);
	void DrawDisplay(uint8_t x, uint8_t y, uint8_t value);
	void SetI(int16_t nnn);
	void SkipConditionX(uint8_t x, uint8_t nn, bool expected);
	void SkipConditionXY(uint8_t x, uint8_t y, bool expected);
	void JumpWithOffset(uint8_t x, uint16_t nnn);
	void Random(uint8_t x, uint8_t nn);
	void InputSkipInstruction(uint8_t x, bool pressed);

	// Logical Ops
	void SetXY(uint8_t x, uint8_t y);
	void BinaryOr(uint8_t x, uint8_t y);
	void BinaryAnd(uint8_t x, uint8_t y);
	void LogicalXor(uint8_t x, uint8_t y);
	void AddXY(uint8_t x, uint8_t y);
	void SubtractXY(uint8_t x, uint8_t y, uint8_t set);
	void LeftShift(uint8_t x, uint8_t y);
	void RightShift(uint8_t x, uint8_t y);

	// System Ops
	void TimerSet(uint8_t x);
	void DelaySet(uint8_t x);
	void SoundSet(uint8_t x);
	void IndexAdd(uint8_t x);
	void GetKey(uint8_t x);
	void FontCharacter(uint8_t x);
	void BinaryDecimalConversion(uint8_t x);
	void StoreMemory(uint8_t x);
	void LoadMemory(uint8_t x);

	// Execute
	int16_t Fetch();
	void DecrementTimers();
	void Execute(uint16_t instruction);
	void ExecuteLogical(uint16_t instruction, uint8_t op, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn);
	void ExecuteSystem(uint16_t instruction, uint8_t op, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn);
};

