// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: these e-papers require 3.3V supply AND data lines!
//
// based on Demo Example from Good Display: http://www.good-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#include "GxEPD2_inkyphat.h"
#include "WaveTables.h"

GxEPD2_inkyphat::GxEPD2_inkyphat(int8_t cs, int8_t dc, int8_t rst, int8_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, LOW, 20000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  _initial = true;
  _power_is_on = false;
}

void GxEPD2_inkyphat::init(uint32_t serial_diag_bitrate)
{
  GxEPD2_EPD::init(serial_diag_bitrate);
  _initial = true;
  _power_is_on = false;
}

void GxEPD2_inkyphat::clearScreen(uint8_t value)
{
  clearScreen(value, 0xFF);
}

void GxEPD2_inkyphat::clearScreen(uint8_t black_value, uint8_t color_value)
{
  writeScreenBuffer(black_value, color_value);
  _Update_Full();
  _initial = false;
}

void GxEPD2_inkyphat::writeScreenBuffer(uint8_t value)
{
  writeScreenBuffer(value, 0xFF);
}

void GxEPD2_inkyphat::writeScreenBuffer(uint8_t black_value, uint8_t color_value)
{
  _Init_Full();
  //_writeCommand(0x11); // data entry
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x24);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(black_value);
  }
  //_writeCommand(0x11); // data entry
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x26);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(color_value);
  }
}

void GxEPD2_inkyphat::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, NULL, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_inkyphat::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  int16_t wb = (w + 7) / 8; // width bytes, bitmaps are padded
  x -= x % 8; // byte boundary
  w = wb * 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  _Init_Full();
  //_writeCommand(0x11); // data entry
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(0x24);
  for (int16_t i = 0; i < h1; i++)
  {
    for (int16_t j = 0; j < w1 / 8; j++)
    {
      uint8_t data = 0xFF;
      if (black)
      {
        // use wb, h of bitmap for index!
        int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&black[idx]);
#else
          data = black[idx];
#endif
        }
        else
        {
          data = black[idx];
        }
        if (invert) data = ~data;
      }
      _writeData(data);
    }
  }
  //_writeCommand(0x11); // data entry
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x26);
  for (int16_t i = 0; i < h1; i++)
  {
    for (int16_t j = 0; j < w1 / 8; j++)
    {
      uint8_t data = 0xFF;
      if (color)
      {
        // use wb, h of bitmap for index!
        int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&color[idx]);
#else
          data = color[idx];
#endif
        }
        else
        {
          data = color[idx];
        }
        if (invert) data = ~data;
      }
      _writeData(data);
    }
  }
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_inkyphat::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1)
  {
    writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_inkyphat::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_inkyphat::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(black, color, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_inkyphat::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeNative(data1, data2, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_inkyphat::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else _Update_Full();
}

void GxEPD2_inkyphat::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  x -= x % 8; // byte boundary
  w -= x % 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  w1 -= x1 - x;
  h1 -= y1 - y;
  _Init_Full();
  _setPartialRamArea(x1, y1, w1, h1);
  _Update_Full();
}

void GxEPD2_inkyphat::powerOff()
{
  _PowerOff();
}

void GxEPD2_inkyphat::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t ye = y + h - 1;
  x &= 0xFFF8; // byte boundary

  _writeCommand(0x44); // set ram x start end
  _writeData(x);
  _writeData(xe);
  _writeCommand(0x45); // set ram y start end
  _writeData(y);
  _writeData(y >> 8);
  _writeData(ye);
  _writeData(ye >> 8);

  _writeCommand(0x4e); // ram x counter
  _writeData(x);
  _writeCommand(0x4f); // ram y counter
  _writeData(y);
  _writeData(y >> 8);
}

void GxEPD2_inkyphat::_PowerOff()
{
  _writeCommand(0x10); // deep sleep
  _writeData(0x01);
  _power_is_on = false;
}

void GxEPD2_inkyphat::_Reset()
{
  if (_rst >= 0 )
  {
    digitalWrite(_rst, 0);
    delay(100);
    digitalWrite(_rst, 1);
    delay(100);
  }
  _power_is_on = false;
}

void GxEPD2_inkyphat::_InitDisplay()
{
  // reset required for wakeup
  if (!_power_is_on)
  {
    _Reset();
  }
  _writeCommand(0X12); // sw reset
  _waitWhileBusy("_wakeUp reset");
  _writeCommand(0X74); // set analog control block
  _writeData(0x54);
  _writeCommand(0X7E); // set digital control block
  _writeData(0x3b);
  _writeCommand(0x01); // driver output control
  _writeData (0xd3);
  _writeData (0x00);
  _writeData (0x00);
  _writeCommand(0X3a); // dummy line period
  _writeData(0x07);
  _writeCommand(0X3b); // gate line width
  _writeData(0x04);
  _writeCommand(0x11); // data entry mode
  _writeData (0x03);
  _power_is_on = true;
}

void GxEPD2_inkyphat::_Init_Full()
{
  _InitDisplay();
  _writeCommand(0x04); // source driving voltage
  _writeData (0x41);
  _writeData (0xa8);
  _writeData (0x32);
  _writeCommand(0x2c); // vcom
  _writeData (0x68);
  _writeCommand(0x3c); // border waveform
  _writeData (0x33);
  _writeCommand(0x32); // write lut register
  _writeData_nCS(GxINKYPHAT_lut_full, sizeof(GxINKYPHAT_lut_full));
}

void GxEPD2_inkyphat::_Update_Full()
{
  _writeCommand(0x22); // display update
  _writeData (0xc7);
  _writeCommand(0x20); // master activation
  _waitWhileBusy("_Update_Full", full_refresh_time);
}

