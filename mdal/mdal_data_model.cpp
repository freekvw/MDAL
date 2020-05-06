/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <algorithm>
#include "mdal_utils.hpp"

MDAL::Dataset::~Dataset() = default;

MDAL::Dataset::Dataset( MDAL::DatasetGroup *parent )
  : mParent( parent )
{
  assert( mParent );
}

size_t MDAL::Dataset::valuesCount() const
{
  const MDAL_DataLocation location = group()->dataLocation();

  switch ( location )
  {
    case MDAL_DataLocation::DataOnVertices: return mesh()->verticesCount();
    case MDAL_DataLocation::DataOnFaces: return mesh()->facesCount();
    case MDAL_DataLocation::DataOnVolumes: return volumesCount();
    case MDAL_DataLocation::DataOnEdges: return mesh()->edgesCount();
    default: return 0;
  }
}

size_t MDAL::Dataset::activeData( size_t, size_t, int * )
{
  assert( !supportsActiveFlag() );
  return 0;
}

MDAL::Statistics MDAL::Dataset::statistics() const
{
  return mStatistics;
}

void MDAL::Dataset::setStatistics( const MDAL::Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DatasetGroup *MDAL::Dataset::group() const
{
  return mParent;
}

MDAL::Mesh *MDAL::Dataset::mesh() const
{
  return mParent->mesh();
}

double MDAL::Dataset::time( RelativeTimestamp::Unit unit ) const
{
  return mTime.value( unit );
}

void MDAL::Dataset::setTime( double time, RelativeTimestamp::Unit unit )
{
  mTime = RelativeTimestamp( time, unit );
}

void MDAL::Dataset::setTime( const MDAL::RelativeTimestamp &time )
{
  mTime = time;
}

bool MDAL::Dataset::supportsActiveFlag() const
{
  return mSupportsActiveFlag;
}

void MDAL::Dataset::setSupportsActiveFlag( bool value )
{
  mSupportsActiveFlag = value;
}

bool MDAL::Dataset::isValid() const
{
  return mIsValid;
}

MDAL::Dataset2D::Dataset2D( MDAL::DatasetGroup *parent )
  : Dataset( parent )
{
}

MDAL::Dataset2D::~Dataset2D() = default;

size_t MDAL::Dataset2D::volumesCount() const { return 0; }

size_t MDAL::Dataset2D::maximumVerticalLevelsCount() const { return 0; }

size_t MDAL::Dataset2D::verticalLevelCountData( size_t, size_t, int * ) { return 0; }

size_t MDAL::Dataset2D::verticalLevelData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset2D::faceToVolumeData( size_t, size_t, int * ) { return 0; }

size_t MDAL::Dataset2D::scalarVolumesData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset2D::vectorVolumesData( size_t, size_t, double * ) { return 0; }

MDAL::Dataset3D::Dataset3D( MDAL::DatasetGroup *parent, size_t volumes, size_t maxVerticalLevelCount )
  : Dataset( parent )
  , mVolumesCount( volumes )
  , mMaximumVerticalLevelsCount( maxVerticalLevelCount )
{
}

MDAL::Dataset3D::~Dataset3D() = default;

size_t MDAL::Dataset3D::volumesCount() const
{
  return mVolumesCount;
}

size_t MDAL::Dataset3D::maximumVerticalLevelsCount() const
{
  return mMaximumVerticalLevelsCount;
}

size_t MDAL::Dataset3D::scalarData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset3D::vectorData( size_t, size_t, double * ) { return 0; }

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri,
                                  const std::string &name )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
  setName( name );
}

std::string MDAL::DatasetGroup::driverName() const
{
  return mDriverName;
}

MDAL::DatasetGroup::~DatasetGroup() = default;

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
}

std::string MDAL::DatasetGroup::getMetadata( const std::string &key )
{
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      return pair.second;
    }
  }
  return std::string();
}

void MDAL::DatasetGroup::setMetadata( const std::string &key, const std::string &val )
{
  bool found = false;
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      found = true;
      pair.second = val;
    }
  }
  if ( !found )
    metadata.push_back( std::make_pair( key, val ) );
}

void MDAL::DatasetGroup::setMetadata( const MDAL::Metadata &metadata )
{
  for ( const auto &meta : metadata )
    setMetadata( meta.first, meta.second );
}

std::string MDAL::DatasetGroup::name()
{
  return getMetadata( "name" );
}

void MDAL::DatasetGroup::setName( const std::string &name )
{
  setMetadata( "name", name );
}

std::string MDAL::DatasetGroup::uri() const
{
  return mUri;
}

MDAL::Statistics MDAL::DatasetGroup::statistics() const
{
  return mStatistics;
}

void MDAL::DatasetGroup::setStatistics( const Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DateTime MDAL::DatasetGroup::referenceTime() const
{
  return mReferenceTime;
}

void MDAL::DatasetGroup::setReferenceTime( const DateTime &referenceTime )
{
  mReferenceTime = referenceTime;
}

MDAL::Mesh *MDAL::DatasetGroup::mesh() const
{
  return mParent;
}

size_t MDAL::DatasetGroup::maximumVerticalLevelsCount() const
{
  size_t maxLevels = 0;
  for ( const std::shared_ptr<Dataset> &ds : datasets )
  {
    const size_t maxDsLevels = ds->maximumVerticalLevelsCount();
    if ( maxDsLevels > maxLevels )
      return maxLevels = maxDsLevels;
  }
  return maxLevels;
}

bool MDAL::DatasetGroup::isInEditMode() const
{
  return mInEditMode;
}

void MDAL::DatasetGroup::startEditing()
{
  mInEditMode = true;
}

void MDAL::DatasetGroup::stopEditing()
{
  mInEditMode = false;
}

MDAL_DataLocation MDAL::DatasetGroup::dataLocation() const
{
  return mDataLocation;
}

void MDAL::DatasetGroup::setDataLocation( MDAL_DataLocation dataLocation )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mDataLocation = dataLocation;
}

bool MDAL::DatasetGroup::isScalar() const
{
  return mIsScalar;
}

void MDAL::DatasetGroup::setIsScalar( bool isScalar )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mIsScalar = isScalar;
}

MDAL::Mesh::Mesh( const std::string &driverName,
                  size_t verticesCount,
                  size_t edgesCount,
                  size_t facesCount,
                  size_t faceVerticesMaximumCount,
                  MDAL::BBox extent,
                  const std::string &uri )
  : mDriverName( driverName )
  , mVerticesCount( verticesCount )
  , mEdgesCount( edgesCount )
  , mFacesCount( facesCount )
  , mFaceVerticesMaximumCount( faceVerticesMaximumCount )
  , mExtent( extent )
  , mUri( uri )
{
}

std::string MDAL::Mesh::driverName() const
{
  return mDriverName;
}

MDAL::Mesh::~Mesh() = default;

std::shared_ptr<MDAL::DatasetGroup> MDAL::Mesh::group( const std::string &name )
{
  for ( auto grp : datasetGroups )
  {
    if ( grp->name() == name )
      return grp;
  }
  return std::shared_ptr<MDAL::DatasetGroup>();
}

void MDAL::Mesh::setSourceCrs( const std::string &str )
{
  mCrs = MDAL::trim( str );
}

void MDAL::Mesh::setSourceCrsFromWKT( const std::string &wkt )
{
  setSourceCrs( wkt );
}

void MDAL::Mesh::setSourceCrsFromEPSG( int code )
{
  setSourceCrs( std::string( "EPSG:" ) + std::to_string( code ) );
}

void MDAL::Mesh::setSourceCrsFromPrjFile( const std::string &filename )
{
  const std::string proj = MDAL::readFileToString( filename );
  setSourceCrs( proj );
}

size_t MDAL::Mesh::verticesCount() const
{
  return mVerticesCount;
}

size_t MDAL::Mesh::edgesCount() const
{
  return mEdgesCount;
}

size_t MDAL::Mesh::facesCount() const
{
  return mFacesCount;
}

std::string MDAL::Mesh::uri() const
{
  return mUri;
}

MDAL::BBox MDAL::Mesh::extent() const
{
  return mExtent;
}

std::string MDAL::Mesh::crs() const
{
  return mCrs;
}

size_t MDAL::Mesh::faceVerticesMaximumCount() const
{
  return mFaceVerticesMaximumCount;
}

MDAL::MeshVertexIterator::~MeshVertexIterator() = default;

MDAL::MeshFaceIterator::~MeshFaceIterator() = default;

MDAL::MeshEdgeIterator::~MeshEdgeIterator() = default;
