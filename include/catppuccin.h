#ifndef CATPPUCCIN_H
#define CATPPUCCIN_H

#include <stdint.h>
#include <CatppuccinRGB565.h>

// Old flavor macros for compatibility with settings storage (which used 1-based indexing)
#define CATPPUCCIN_MOCHA      1
#define CATPPUCCIN_MACCHIATO  2
#define CATPPUCCIN_FRAPPE     3
#define CATPPUCCIN_LATTE      4

// We use Catppuccin::Palette from CatppuccinRGB565.h
using CatppuccinColors = Catppuccin::Palette;

const CatppuccinColors& getCatppuccinFlavor(int flavor);

#endif // CATPPUCCIN_H
