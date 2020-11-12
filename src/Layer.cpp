// Created by Modar Nasser on 12/11/2020.

#include <iostream>
#include <memory>

#include "LDtkLoader/Layer.hpp"
#include "LDtkLoader/Utils.hpp"

using namespace ldtk;

Layer::Layer(const nlohmann::json& j) :
type(getLayerTypeFromString(j["__type"].get<std::string>())),
name(j["__identifier"].get<std::string>()),
grid_size({j["__cWid"].get<unsigned int>(), j["__cHei"].get<unsigned int>()}),
cell_size(j["__gridSize"].get<unsigned int>()),
m_total_offset({j["__pxTotalOffsetX"].get<int>(), j["__pxTotalOffsetY"].get<int>()}),
m_opacity(j["__opacity"].get<float>())
{
    std::string key = "gridTiles";
    int d_offset = 0;
    if (type == LayerType::IntGrid || type == LayerType::AutoLayer) {
        key = "autoLayerTiles";
        d_offset = 1;
    }
    for (const auto& tile : j[key]) {
        auto new_tile = std::unique_ptr<Tile>(new Tile());
        new_tile->coordId = tile["d"].get<std::vector<unsigned int>>()[d_offset];
        new_tile->position.x = tile["px"].get<std::vector<unsigned int>>()[0];
        new_tile->position.y = tile["px"].get<std::vector<unsigned int>>()[1];

        new_tile->tileId = tile["d"].get<std::vector<unsigned int>>()[d_offset+1];
        new_tile->texture_position.x = tile["src"].get<std::vector<unsigned int>>()[0];
        new_tile->texture_position.y = tile["src"].get<std::vector<unsigned int>>()[1];

        auto flip = tile["f"].get<unsigned int>();
        new_tile->flipX = flip & 1u;
        new_tile->flipY = (flip>>1u) & 1u;

        updateTileVertices(*new_tile);

        m_tiles.push_back(new_tile.get());
        m_tiles_map[new_tile->coordId] = std::move(new_tile);
    }
}

auto Layer::getOffset() const -> const IntPoint& {
    return m_total_offset;
}

void Layer::setOffset(const IntPoint& offset) {
    m_total_offset = offset;
}

auto Layer::getOpacity() const -> float {
    return m_opacity;
}

void Layer::setOpacity(float opacity) {
    m_opacity = std::min(1.f, std::max(0.f, opacity));
}

auto Layer::hasTileset() const -> bool {
    return m_tileset != nullptr;
}

auto Layer::getTileset() const -> const Tileset& {
    if (m_tileset == nullptr) {
        std::cerr << "ERROR in Layer::getTileset : Layer " << name << " doesn't have a tileset." << std::endl;
        exit(-1);
    }
    return *m_tileset;
}

auto Layer::allTiles() const -> const std::vector<Tile*>& {
    return m_tiles;
}

auto Layer::getTile(unsigned int grid_x, unsigned int grid_y) const -> const Tile& {
    auto id = grid_x + grid_size.x*grid_y;
    if (m_tiles_map.count(id) > 0)
        return *m_tiles_map.at(id);
    throw std::invalid_argument("Layer "+name+" does not have a tile at position ("+std::to_string(grid_x)+", "+std::to_string(grid_y)+")");
}

void Layer::setLayerDef(const LayerDef& layer_def) {
    m_layer_def = &layer_def;
}

void Layer::setTileset(const Tileset& tileset) {
    m_tileset = &tileset;
}

void Layer::updateTileVertices(Tile& tile) const {
    auto& verts = tile.vertices;
    verts[0].pos.x = tile.position.x;           verts[0].pos.y = tile.position.y;
    verts[1].pos.x = tile.position.x+cell_size; verts[1].pos.y = tile.position.y;
    verts[2].pos.x = tile.position.x+cell_size; verts[2].pos.y = tile.position.y+cell_size;
    verts[3].pos.x = tile.position.x;           verts[3].pos.y = tile.position.y+cell_size;

    std::array<UIntPoint, 4> modif;
    if (tile.flipX) {
        modif[0].x = cell_size; modif[1].x = -cell_size;
        modif[3].x = cell_size; modif[2].x = -cell_size;
    }
    if (tile.flipY) {
        modif[0].y =  cell_size; modif[1].y =  cell_size;
        modif[3].y = -cell_size; modif[2].y = -cell_size;
    }
    for (int i = 0; i < 4; ++i) {
        verts[i].tex.x = tile.texture_position.x+modif[i].x;
        verts[i].tex.y = tile.texture_position.y+modif[i].y;
    }
}