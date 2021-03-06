#include "quantizedmeshterraingenerator.h"

#include "map3d.h"
#include "quadtree.h"
#include "quantizedmeshgeometry.h"
#include "terrain.h"

#include "qgsmapsettings.h"

#include <Qt3DRender/QGeometryRenderer>

class QuantizedMeshTerrainTile : public TerrainTileEntity
{
public:
  QuantizedMeshTerrainTile(Terrain* terrain, QuadTreeNode* node, Qt3DCore::QNode *parent = nullptr);

};

QuantizedMeshTerrainTile::QuantizedMeshTerrainTile(Terrain* terrain, QuadTreeNode *node, Qt3DCore::QNode *parent)
  : TerrainTileEntity(terrain, node, parent)
{
  const Map3D& map = terrain->map3D();
  QuantizedMeshTerrainGenerator* generator = static_cast<QuantizedMeshTerrainGenerator*>(map.terrainGenerator.get());

  int tx, ty, tz;
  generator->quadTreeTileToBaseTile(node->x, node->y, node->level, tx, ty, tz);

  QgsRectangle tileRect = map.terrainGenerator->terrainTilingScheme.tileToExtent(tx, ty, tz);

  // we need map settings here for access to mapToPixel
  QgsMapSettings mapSettings;
  mapSettings.setLayers(map.layers());
  mapSettings.setOutputSize(QSize(map.tileTextureSize,map.tileTextureSize));
  mapSettings.setDestinationCrs(map.crs);
  mapSettings.setExtent(terrain->terrainToMapTransform().transformBoundingBox(tileRect));

  QuantizedMeshGeometry::downloadTileIfMissing(tx, ty, tz);
  QuantizedMeshTile* qmt = QuantizedMeshGeometry::readTile(tx, ty, tz, tileRect);
  Q_ASSERT(qmt);
  Qt3DRender::QGeometryRenderer* mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry(new QuantizedMeshGeometry(qmt, map, mapSettings.mapToPixel(), terrain->terrainToMapTransform(), mesh));
  addComponent(mesh);

  transform->setScale3D(QVector3D(1.f, map.zExaggeration, 1.f));

  QgsRectangle mapExtent = mapSettings.extent();
  float x0 = mapExtent.xMinimum() - map.originX;
  float y0 = mapExtent.yMinimum() - map.originY;
  float x1 = mapExtent.xMaximum() - map.originX;
  float y1 = mapExtent.yMaximum() - map.originY;
  float z0 = qmt->header.MinimumHeight, z1 = qmt->header.MaximumHeight;
  bbox = AABB(x0, z0*map.zExaggeration, -y0, x1, z1*map.zExaggeration, -y1);
  epsilon = mapExtent.width() / map.tileTextureSize;
}


// ---------------



QuantizedMeshTerrainGenerator::QuantizedMeshTerrainGenerator()
{
  terrainBaseX = terrainBaseY = terrainBaseZ = 0;
  terrainTilingScheme = TilingScheme(QgsRectangle(-180,-90,0,90), QgsCoordinateReferenceSystem("EPSG:4326"));
}

void QuantizedMeshTerrainGenerator::setBaseTileFromExtent(const QgsRectangle &extentInTerrainCrs)
{
  terrainTilingScheme.extentToTile(extentInTerrainCrs, terrainBaseX, terrainBaseY, terrainBaseZ);
}

void QuantizedMeshTerrainGenerator::quadTreeTileToBaseTile(int x, int y, int z, int &tx, int &ty, int &tz) const
{
  // true tile coords (using the base tile pos)
  int multiplier = pow(2, z);
  tx = terrainBaseX * multiplier + x;
  ty = terrainBaseY * multiplier + y;
  tz = terrainBaseZ + z;
}

TerrainGenerator::Type QuantizedMeshTerrainGenerator::type() const
{
  return TerrainGenerator::QuantizedMesh;
}

QgsRectangle QuantizedMeshTerrainGenerator::extent() const
{
  return terrainTilingScheme.tileToExtent(terrainBaseX, terrainBaseY, terrainBaseZ);
}

TerrainTileEntity *QuantizedMeshTerrainGenerator::createTile(Terrain* terrain, QuadTreeNode *n, Qt3DCore::QNode *parent) const
{
  return new QuantizedMeshTerrainTile(terrain, n, parent);
}

void QuantizedMeshTerrainGenerator::writeXml(QDomElement &elem) const
{
  elem.setAttribute("base-x", terrainBaseX);
  elem.setAttribute("base-y", terrainBaseY);
  elem.setAttribute("base-z", terrainBaseZ);
}

void QuantizedMeshTerrainGenerator::readXml(const QDomElement &elem)
{
  terrainBaseX = elem.attribute("base-x").toInt();
  terrainBaseY = elem.attribute("base-y").toInt();
  terrainBaseZ = elem.attribute("base-z").toInt();
  // TODO: update tiling scheme
}
