// Created by Modar Nasser on 11/11/2020.

#include "LDtkLoader/Tileset.hpp"

using namespace ldtk;

Tileset::Tileset(const nlohmann::json& j) :
name(j["identifier"].get<std::string>()),
uid(j["uid"].get<unsigned int>()),
path(j["relPath"].get<std::string>()),
image_size({j["pxWid"].get<unsigned int>(), j["pxHei"].get<unsigned int>()}),
tile_size(j["tileGridSize"].get<unsigned int>()),
spacing(j["spacing"].get<unsigned int>()),
padding(j["padding"].get<unsigned int>())
{}