#include "DebugPainter.h"
#include <Windows.h>
#include <string>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

//std::string string_format(const std::string fmt_str, ...) {
//	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
//	std::unique_ptr<char[]> formatted;
//	va_list ap;
//	while (1) {
//		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
//		strcpy(&formatted[0], fmt_str.c_str());
//		va_start(ap, fmt_str);
//		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
//		va_end(ap);
//		if (final_n < 0 || final_n >= n)
//			n += abs(final_n - n + 1);
//		else
//			break;
//	}
//	return std::string(formatted.get());
//}
//
//
//struct Color
//{
//	unsigned char r, g, b, a;
//};
//#define COLOR_TEXT 255, 255, 255, 255
//#define COLOR_PANEL 64, 64, 64, 192
//void drawStringAt(CanvasWrapper cw, std::string text, int x, int y, Color col = { 255, 255, 255, 255 })
//{
//	cw.SetPosition({ x, y });
//	cw.SetColor(col.r, col.g, col.b, col.a);
//	cw.DrawString(text);
//}
//
//void drawStringAt(CanvasWrapper cw, std::string text, Vector2 loc, Color col)
//{
//	drawStringAt(cw, text, loc.X, loc.Y, col);
//}
//
//template<typename t>
//string tToBinary(t thingy)
//{
//	std::string returnString = "";
//	for (int i = 0; i < sizeof(thingy) * 8; i++)
//	{
//		returnString += (thingy & (1 << i)) ? "1" : "0";
//	}
//	return returnString;
//}
//
//DWORD Error = -1;
//void drawLastInput(CanvasWrapper cw)
//{
//	int x = 20, y = 50;
//	int marginLeft = 30;
//	int marginTop = 20;
//
//	int nameSpacing = 100;
//	int vecSpacing = 80;
//	int quatSpacing = 120;
//
//	int lineSpacing = 30;
//
//	int width = 620;
//	int height = 210;
//
//	cw.SetPosition({ x, y });
//	cw.SetColor(COLOR_PANEL);
//	cw.FillBox({ width, height });
//	cw.SetColor(COLOR_TEXT);
//
//	drawStringAt(cw, "HID values", x + marginLeft, y + marginTop);
//	drawStringAt(cw, ("Counter: " + to_string(lastInput.timestamp)), x + marginLeft, y + marginTop + 20);
//	drawStringAt(cw, ("Buttons: " + tToBinary(lastInput.buttons)), x + marginLeft, y + marginTop + 20 + 20);
//	drawStringAt(cw, ("Left stick: " + to_string(lastInput.leftx) + "x" + to_string(lastInput.lefty) + " / " + tToBinary(lastInput.lefty)), x + marginLeft, y + marginTop + 20 + 20 + 20);
//
//
//
//	drawStringAt(cw, ("Error: " + to_string(Error)), x + marginLeft, y + marginTop + 20 + 20 + 20 + 20);
//	drawStringAt(cw, ("x scale: " + to_string(scale(0, 256, lastInput.leftx))), x + marginLeft, y + marginTop + 20 + 20 + 20 + 20 + 20);
//	//
//	drawStringAt(cw, ("Cross pressed: " + to_string(lastInput.buttons & BUTTON_CROSS)), x + marginLeft, y + marginTop + 20 + 20 + 20 + 20 + 20 + 20);
//
//
//}
//
//DebugPainter::DebugPainter()
//{
//}
//
//
//DebugPainter::~DebugPainter()
//{
//}
//
//void DebugPainter::Render(CanvasWrapper cw)
//{
//}
//
//void DebugPainter::initialize(std::shared_ptr<GameWrapper> gameWrapper, std::shared_ptr<CVarManagerWrapper> _cvarManager)
//{
//	cvarManager->registerNotifier("hid_list", [this](std::vector<string> params) {
//		int res = hid_init();
//		struct hid_device_info *devs, *cur_dev;
//
//		devs = hid_enumerate(0x0, 0x0);
//		cur_dev = devs;
//		while (cur_dev) {
//			cvarManager->log(string_format("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
//				cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number));
//			cvarManager->log("\n");
//			cvarManager->log(string_format("  Manufacturer: %ls\n", cur_dev->manufacturer_string));
//			cvarManager->log(string_format("  Product:      %ls\n", cur_dev->product_string));
//			cvarManager->log("\n");
//			cur_dev = cur_dev->next;
//		}
//		hid_free_enumeration(devs);
//	}, "Lists all connected HID devices", PERMISSION_ALL);
//	gameWrapper->RegisterDrawable(&drawLastInput);
//}
