#include "Chip8.h"

Chip8::Chip8(uint16_t height, uint16_t width) :
	_height{height}, _width{ width }
{
	Initialise();
}

Chip8::~Chip8() {
	DestroyPixelGrid();
}

void Chip8::LoadRom(char* rom, uint16_t length) {
	uint16_t writeIndex = 0x200;
	for (int i = 0; i < length; i++) {
		_ram[writeIndex] = rom[i];
		writeIndex++;
	}
}

void Chip8::ProcessorTick() {
	uint16_t instruction = Fetch();
	Execute(instruction);
}

void Chip8::DisplayTick() {
	DecrementTimers();
}

// Initialisation

void Chip8::Initialise() {
	InitialiseFont();
	InitialisePixelGrid();
	InitialiseVariableRegisters();
	InitialisePointers();
	InitialiseInput();
}

void Chip8::InitialiseFont() {
	constexpr uint8_t fonts[] = 
	{ 
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	for (size_t i = 0; i < sizeof(fonts) / sizeof(fonts[0]); i++)
	{
		_ram[i + FONT_OFFSET] = fonts[i];
	}
}

void Chip8::InitialisePixelGrid() {
	_pixelGrid = new bool* [_height];
	for (size_t i = 0; i < _height; i++)
	{
		_pixelGrid[i] = new bool[_width];
	}
	for (size_t i = 0; i < _height; i++)
	{
		for (size_t j = 0; j < _width; j++)
		{
			_pixelGrid[i][j] = false;
		}
	}
}

void Chip8::InitialiseVariableRegisters() {
	for (size_t i = 0; i < sizeof(_vars) / sizeof(_vars[0]); i++)
	{
		_vars[i] = 0;
	}
}

void Chip8::InitialisePointers() {
	_pc = 0x200;
	_I = 0x0;
}

void Chip8::InitialiseInput() {
	for (int i = 0; i < sizeof(_input) / sizeof(_input[0]); i++) {
		_input[i] = 0;
	}
}

// Cleanup

void Chip8::DestroyPixelGrid() {
	for (size_t i = 0; i < _height; i++)
	{
		delete[] _pixelGrid[i];
		_pixelGrid[i] = NULL;
	}
	delete[] _pixelGrid;
	_pixelGrid = NULL;
}


// Ops

// op: 2NNN
void Chip8::SubroutineCall(int16_t address) {
	if (_stack.size() > 32) {
		throw std::overflow_error("stack overflow");
	}

	_stack.push(_pc);
	_pc = address;
}


// op: 00EE
void Chip8::SubroutineReturn() {
	uint16_t result = _stack.top();
	_stack.pop();
	_pc = result;
}

// op: 00E0
void Chip8::ClearScreen() {
	for (size_t i = 0; i < _height; i++)
	{
		for (size_t j = 0; j < _width; j++)
		{
			_pixelGrid[i][j] = false;
		}
	}
	_displayChanged = true;
}

// op: 1NNN
void Chip8::Jump(uint16_t nnn) {
	_pc = nnn;
}

// op: 6XNN
void Chip8::SetRegister(uint8_t x, uint8_t nn) {
	_vars[x] = nn;
}

// op: 7XNN
void Chip8::AddValueToRegister(uint8_t x, uint8_t nn) {
	_vars[x] += nn;
}

// op: DXYN
void Chip8::DrawDisplay(uint8_t x, uint8_t y, uint8_t value) {
	uint16_t vx = _vars[x] % _width;
	uint16_t vy = _vars[y] % _height;
	uint8_t n = 0;
	_vars[0xF] = 0;

	for (size_t yOffset = 0; yOffset < value; yOffset++)
	{
		if (vy + yOffset >= _height) {
			break;
		}

		int8_t sprite = _ram[_I + yOffset];

		for (size_t xOffset = 0; xOffset < 8; xOffset++)
		{
			if (vx + xOffset >= _width) break;

			bool spriteBit = (sprite >> (7 - xOffset)) & 1;

			if (spriteBit) {
				_pixelGrid[vy + yOffset][vx + xOffset] = !_pixelGrid[vy + yOffset][vx + xOffset];

				if (_pixelGrid[vy + yOffset][vx + xOffset] == false) {
					_vars[0xF] = 1;
				}
			}
		}
	}

	_displayChanged = true;
}

// op: ANNN
void Chip8::SetI(int16_t nnn) {
	_I = nnn;
}

// op: 3XNN 4XNN
void Chip8::SkipConditionX(uint8_t x, uint8_t nn, bool expected) {
	uint8_t vx = _vars[x];
	if ((vx == nn) == expected) // CHECK: does unsigned type matter? is gV supposed to be uint? are we comparing value or bits?
	{
		_pc += 2;
	}
}

// op: 5XY0 9XY0
void Chip8::SkipConditionXY(uint8_t x, uint8_t y, bool expected) {
	uint8_t vx = _vars[x];
	uint8_t vy = _vars[y];
	if ((vx == vy) == expected)
	{
		_pc += 2;
	}
}

// op: BXNN
void Chip8::JumpWithOffset(uint8_t x, uint16_t nnn) {
	if (COSMAC_VIP_JUMP_BEHAVIOUR) {
		_pc = _vars[0] + nnn;
	}
	else {
		_pc = _vars[x] + nnn;
	}
}

// op: CXNN
void Chip8::Random(uint8_t x, uint8_t nn) {
	_vars[x] = rand() & nn;
}

// op: EX9E EXA1
void Chip8::InputSkipInstruction(uint8_t x, bool pressed) {
	uint8_t key = _vars[x] & 0xF;
	if (_vars[x] != key) {
		printf("Invalid input key checked: %X", _vars[x]);
	}

	if ((bool)_input[key] == pressed) {
		_pc += 2;
	}
}

// Logical

// op: 8XY0
void Chip8::SetXY(uint8_t x, uint8_t y) {
	_vars[x] = _vars[y];
}

// op: 8XY1
void Chip8::BinaryOr(uint8_t x, uint8_t y) {
	_vars[x] = _vars[x] | _vars[y];
}

// op: 8XY2
void Chip8::BinaryAnd(uint8_t x, uint8_t y) {
	_vars[x] = _vars[x] & _vars[y];
}

// op: 8XY3
void Chip8::LogicalXor(uint8_t x, uint8_t y) {
	_vars[x] = _vars[x] ^ _vars[y];
}

// op: 8XY4
void Chip8::AddXY(uint8_t x, uint8_t y) {
	_vars[0xF] = (255 - _vars[x] < _vars[y]); // carry flag
	_vars[x] = _vars[x] + _vars[y];
}

// op: 8XY5 8XY7
void Chip8::SubtractXY(uint8_t x, uint8_t y, uint8_t set) {
	_vars[0xF] = _vars[x] > _vars[y];
	_vars[set] = _vars[x] - _vars[y];
}

// op: 8XYE
void Chip8::LeftShift(uint8_t x, uint8_t y) {
	if (COSMAC_VIP_SHIFT_BEHAVIOUR) {
		_vars[x] = _vars[y];
	}

	_vars[0xF] = (_vars[x] >> 7) & 1; // bit that's shifted out
	_vars[x] = _vars[x] << 1;
}

// op: 8XY6
void Chip8::RightShift(uint8_t x, uint8_t y) {
	if (COSMAC_VIP_SHIFT_BEHAVIOUR) {
		_vars[x] = _vars[y];
	}

	_vars[0xF] = _vars[x] & 1; // bit that's shifted out
	_vars[x] = _vars[x] >> 1;
}

// System

// op: FX07
void Chip8::TimerSet(uint8_t x) {
	_vars[x] = _delayTimer;
}

// op: FX15
void Chip8::DelaySet(uint8_t x) {
	_delayTimer = _vars[x];
}

// op: FX18
void Chip8::SoundSet(uint8_t x) {
	_soundTimer = _vars[x];
}

// op: FX1E
void Chip8::IndexAdd(uint8_t x) {
	if (AMIGA_INDEX_BEHAVIOUR) {
		if (0xFFF - _vars[x] < _I) {
			_vars[0xF] = 1;
		}
	}

	_I += _vars[x];
}

// op: FX0A
void Chip8::GetKey(uint8_t x) {
	for (uint8_t i = 0; i < sizeof(_input) / sizeof(_input[0]); i++)
	{
		if (_input[i] == 1) {
			_vars[x] = i;
			return;
		}
	}

	// wait on this instruction until input is found
	_pc -= 2;
}

// op: FX29
void Chip8::FontCharacter(uint8_t x) {
	uint8_t ch = _vars[x] & 0xF;
	_I = FONT_OFFSET + ch * 5;
}

// op: FX33
void Chip8::BinaryDecimalConversion(uint8_t x) {
	uint8_t vx = _vars[x];

	_ram[_I] = (vx / 100) % 10;
	_ram[_I + 1] = (vx / 10) % 10;
	_ram[_I + 2] = vx % 10;
}

// op: FX55
void Chip8::StoreMemory(uint8_t x) {
	for (int i = 0; i <= x; i++) {
		_ram[_I + i] = _vars[i];
	}

	if (COSMAC_VIP_MEMORY_BEHAVIOUR) {
		_I = _I + x + 1;
	}
}

// op: FX65
void Chip8::LoadMemory(uint8_t x) {
	for (int i = 0; i <= x; i++) {
		_vars[i] = _ram[_I + i];
	}

	if (COSMAC_VIP_MEMORY_BEHAVIOUR) {
		_I = _I + x + 1;
	}
}

void Test1(uint16_t ins) {
	// t1;
}

void Test2(uint16_t ins) {
	// t2;
}

// Execution

void Chip8::DecrementTimers() {
	if (_delayTimer > 0) _delayTimer--;
	if (_soundTimer > 0) _soundTimer--;
}

int16_t Chip8::Fetch() {
	uint16_t instruction = _ram[_pc] << 8 | _ram[_pc + 1];
	_pc += 2;
	return instruction;
}

// TODO: Can this be a bit cleaner by directly building a vtable?
void Chip8::Execute(uint16_t instruction) {
	uint8_t op = (instruction >> 12) & 0xF;
	uint8_t x = (instruction >> 8) & 0xF;
	uint8_t y = (instruction >> 4) & 0xF;
	uint8_t n = (instruction >> 0) & 0xF;
	uint8_t nn = (instruction >> 0) & 0xFF;
	uint16_t nnn = (instruction >> 0) & 0xFFF;

	switch (op) {
	case 0x0: // Clear screen
		if (y == 0xE && n == 0x0) {
			ClearScreen();
		}
		else if (y == 0xE && n == 0xE) {
			SubroutineReturn();
		}
		break;
	case 0x1:
		Jump(nnn);
		break;
	case 0x2:
		SubroutineCall(nnn);
		break;
	case 0x3:
		SkipConditionX(x, nn, true);
		break;
	case 0x4:
		SkipConditionX(x, nn, false);
		break;
	case 0x5:
		SkipConditionXY(x, y, true);
		break;
	case 0x6:
		SetRegister(x, nn);
		break;
	case 0x7:
		AddValueToRegister(x, nn);
		break;
	case 0x8:
		ExecuteLogical(instruction, op, x, y, n, nn, nnn);
		break;
	case 0x9:
		SkipConditionXY(x, y, false);
		break;
	case 0xA:
		SetI(nnn);
		break;
	case 0xB:
		JumpWithOffset(x, nnn);
		break;
	case 0xC:
		Random(x, nn);
		break;
	case 0xD:
		DrawDisplay(x, y, n);
		break;
	case 0xE:
		if (nn == 0x9E) {
			InputSkipInstruction(x, true);
		}
		else if (nn == 0xA1) {
			InputSkipInstruction(x, false);
		}
		break;
	case 0xF:
		ExecuteSystem(instruction, op, x, y, n, nn, nnn);
		break;
	default:
		printf("opcode not implemented: 0x%X\n", instruction);
	}
}

void Chip8::ExecuteLogical(uint16_t instruction, uint8_t op, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn) {
	switch (n) {
	case 0x0:
		SetXY(x, y);
		break;
	case 0x1:
		BinaryOr(x, y);
		break;
	case 0x2:
		BinaryAnd(x, y);
		break;
	case 0x3:
		LogicalXor(x, y);
		break;
	case 0x4:
		AddXY(x, y);
		break;
	case 0x5:
		SubtractXY(x, y, x);
		break;
	case 0x6:
		RightShift(x, y);
		break;
	case 0x7:
		SubtractXY(y, x, x);
		break;
	case 0xE:
		LeftShift(x, y);
		break;
	default:
		printf("Logical opcode not implemented: 0x%X\n", instruction);
	}
}

void Chip8::ExecuteSystem(uint16_t instruction, uint8_t op, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn) {
	switch (nn) {
	case 0x07:
		TimerSet(x);
		break;
	case 0x15:
		DelaySet(x);
		break;
	case 0x18:
		SoundSet(x);
		break;
	case 0x1E:
		IndexAdd(x);
		break;
	case 0x0A:
		GetKey(x);
		break;
	case 0x29:
		FontCharacter(x);
		break;
	case 0x33:
		BinaryDecimalConversion(x);
		break;
	case 0x55:
		StoreMemory(x);
		break;
	case 0x65:
		LoadMemory(x);
		break;
	default:
		printf("System opcode not implemented: 0x % X\n", instruction);
	}
}