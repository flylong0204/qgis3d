#ifndef FLATTERRAINTILE_H
#define FLATTERRAINTILE_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DExtras/QTextureMaterial>

struct QuadTreeNode;
class FlatTerrainTileMesh;
class Map3D;
class QgsRectangle;

//! base class for all kinds of terrain tiles
class TerrainTile : public Qt3DCore::QEntity
{
public:
  TerrainTile(QuadTreeNode* node, Map3D& map, Qt3DCore::QNode *parent = nullptr);

protected:
  Qt3DExtras::QTextureMaterial* material;
  Qt3DCore::QTransform* transform;
};

//! just a simple quad with a map texture
class FlatTerrainTile : public TerrainTile
{
public:
  FlatTerrainTile(Qt3DExtras::QPlaneGeometry* tileGeometry, QuadTreeNode* node, Map3D& map, Qt3DCore::QNode *parent = nullptr);

private:
  FlatTerrainTileMesh* mesh;
};



//! tile made from DEM
class DemTerrainTile : public TerrainTile
{
public:
  DemTerrainTile(QuadTreeNode* node, Map3D& map, Qt3DCore::QNode *parent = nullptr);

private:
  //Qt3DRender::QGeometryRenderer* mesh;
};


class QuantizedMeshTerrainTile : public TerrainTile
{
public:
  QuantizedMeshTerrainTile(QuadTreeNode* node, Map3D& map, Qt3DCore::QNode *parent = nullptr);

  static void tileExtentXYZ(QuadTreeNode* node, Map3D& map, int& tx, int& ty, int& tz);
};

#endif // FLATTERRAINTILE_H