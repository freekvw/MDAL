/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_H
#define MDAL_H

/**********************************************************************/
/**********************************************************************/
/* API is considered EXPERIMENTAL and can be changed without a notice */
/**********************************************************************/
/**********************************************************************/

#ifdef MDAL_STATIC
#  define MDAL_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef mdal_EXPORTS
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllexport))
#      else
#        define MDAL_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    else
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllimport))
#      else
#        define MDAL_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define MDAL_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define MDAL_EXPORT
#    endif
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Statuses */
enum MDAL_Status
{
  None,
  // Errors
  Err_NotEnoughMemory,
  Err_FileNotFound,
  Err_UnknownFormat,
  Err_IncompatibleMesh,
  Err_InvalidData,
  Err_IncompatibleDataset,
  Err_IncompatibleDatasetGroup,
  Err_MissingDriver,
  // Warnings
  Warn_UnsupportedElement,
  Warn_InvalidElements,
  Warn_ElementWithInvalidNode,
  Warn_ElementNotUnique,
  Warn_NodeNotUnique
};

//! Mesh
typedef void *MeshH;
typedef void *DatasetGroupH;
typedef void *DatasetH;

//! Returns MDAL version
MDAL_EXPORT const char *MDAL_Version();

//! Returns last status message
MDAL_EXPORT MDAL_Status MDAL_LastStatus();

///////////////////////////////////////////////////////////////////////////////////////
/// MESH
///////////////////////////////////////////////////////////////////////////////////////

//! Loads mesh file. On error see MDAL_LastStatus for error type This effectively loads whole mesh in-memory
MDAL_EXPORT MeshH MDAL_LoadMesh( const char *meshFile );
//! Closes mesh, frees the memory
MDAL_EXPORT void MDAL_CloseMesh( MeshH mesh );
//! Returns mesh projection
//! not thread-safe and valid only till next call
MDAL_EXPORT const char *MDAL_M_projection( MeshH mesh );
//! Returns vertex count for the mesh
MDAL_EXPORT int MDAL_M_vertexCount( MeshH mesh );
//! Returns vertex X coord for the mesh
MDAL_EXPORT double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index );
//! Returns vertex Y coord for the mesh
MDAL_EXPORT double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index );
//! Returns vertex Z coord for the mesh
MDAL_EXPORT double MDAL_M_vertexZCoordinatesAt( MeshH mesh, int index );
//! Returns face count for the mesh
MDAL_EXPORT int MDAL_M_faceCount( MeshH mesh );
//! Returns number of vertices face consist of, e.g. 3 for triangle
MDAL_EXPORT int MDAL_M_faceVerticesCountAt( MeshH mesh, int index );
//! Returns vertex index for face
MDAL_EXPORT int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index );

//! Loads dataset file. On error see MDAL_LastStatus for error type.
//! This may effectively load whole dataset in-memory for some providers
//! Datasets will be closed automatically on mesh destruction or memory
//! can be freed manually with MDAL_CloseDataset if needed
MDAL_EXPORT void MDAL_M_LoadDatasets( MeshH mesh, const char *datasetFile );

//! Returns dataset groups count
MDAL_EXPORT int MDAL_M_datasetGroupCount( MeshH mesh );

//! Returns dataset group handle
MDAL_EXPORT DatasetGroupH MDAL_M_datasetGroup( MeshH mesh, int index );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASET GROUPS
///////////////////////////////////////////////////////////////////////////////////////

//! Returns dataset parent mesh
MDAL_EXPORT MeshH MDAL_G_mesh( DatasetGroupH group );

//! Returns dataset count in group
MDAL_EXPORT int MDAL_G_datasetCount( DatasetGroupH group );

//! Returns dataset handle
MDAL_EXPORT DatasetH MDAL_G_dataset( DatasetGroupH group, int index );

//! Returns number of metadata values
MDAL_EXPORT int MDAL_G_metadataCount( DatasetGroupH group );

//! Returns dataset metadata key
//! not thread-safe and valid only till next call
MDAL_EXPORT const char *MDAL_G_metadataKey( DatasetGroupH group, int index );

//! Returns dataset metadata value
//! not thread-safe and valid only till next call
MDAL_EXPORT const char *MDAL_G_metadataValue( DatasetGroupH group, int index );

//! Returns dataset group name
//! not thread-safe and valid only till next call
MDAL_EXPORT const char *MDAL_G_name( DatasetGroupH group );

//! Whether dataset has scalar data associated
MDAL_EXPORT bool MDAL_G_hasScalarData( DatasetGroupH group );

//! Whether dataset is on vertices
MDAL_EXPORT bool MDAL_G_isOnVertices( DatasetGroupH group );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASETS
///////////////////////////////////////////////////////////////////////////////////////

//! Returns dataset parent group
MDAL_EXPORT DatasetGroupH MDAL_D_group( DatasetH dataset );

//! Returns dataset time
MDAL_EXPORT double MDAL_D_time( DatasetH dataset );

//! Returns number of values
MDAL_EXPORT int MDAL_D_valueCount( DatasetH dataset );

//! Returns whether dataset is valid
MDAL_EXPORT bool MDAL_D_isValid( DatasetH dataset );

//! Data type to be returned by MDAL_D_data
enum MDAL_DataType
{
  SCALAR_DOUBLE, //!< double value for scalar datasets
  VECTOR_2D_DOUBLE, //!< double, double value for vector datasets
  ACTIVE_BOOL //!< active flag for datasets. Some formats support switching off the element for particular timestep.
};

//! Populates buffer with values from the dataset
//! for nodata, returned is numeric_limits<double>:quiet_NaN
//!
//! \param dataset handle to dataset
//! \param indexStart index of face/vertex to start reading of values to the buffer
//! \param count number of values to be written to the buffer
//! \param dataType type of values to be written to the buffer
//! \param buffer output array to be populated with the values. must be already allocated
//!               For SCALAR_DOUBLE, the minimum size must be valuesCount * size_of(double)
//!               For VECTOR_2D_DOUBLE, the minimum size must be valuesCount * 2 * size_of(double).
//!                                     Values are returned as x1, y1, x2, y2, ..., xN, yN
//!               For ACTIVE_BOOL, the minimum size must be valuesCount * size_of(char)
//! \returns number of values written to buffer. If return value != count requested, see MDAL_LastStatus() for error type
MDAL_EXPORT int MDAL_D_data( DatasetH dataset, int indexStart, int count, MDAL_DataType dataType, void *buffer );

#ifdef __cplusplus
}
#endif

#endif //MDAL_H
