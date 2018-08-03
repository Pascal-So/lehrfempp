/**
 * @file
 * @brief Declares the VtkWriter which can write vtk files that can be read
 *        by ParaView
 * @author Raffael Casagrande
 * @date   2018-07-14 07:13:49
 * @copyright MIT License
 */

#ifndef __3e48c7b32a034cb3be3dbca884ff4f6c
#define __3e48c7b32a034cb3be3dbca884ff4f6c

#include <lf/mesh/mesh.h>
#include <Eigen/Eigen>
#include <boost/variant/variant.hpp>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace lf::io {

/**
 * @brief Representation of a VTK file (only relevant) features (advanced usage)
 *
 * This class can be written to a *.vtk file through the method WriteToFile().
 * The VtkWriter uses this class internally, it is not intended for direct
 * usage by the normal user but it can be useful if a special kind of
 * VtkFile should be written.
 */
class VtkFile {
 public:
  using size_type = unsigned int;

  /// Nested types
  //////////////////////////////////////////////////////////////////////////////
  enum class Format { ASCII, BINARY };

  enum class CellType {
    VTK_VERTEX = 1,
    VTK_POLY_VERTEX = 2,
    VTK_LINE = 3,
    VTK_POLY_LINE = 4,
    VTK_TRIANGLE = 5,
    VTK_TRIANGLE_STRIP = 6,
    VTK_POLYGON = 7,
    VTK_PIXEL = 8,
    VTK_QUAD = 9,
    VTK_TETRA = 10,
    VTK_VOXEL = 11,
    VTK_HEXAHEDRON = 12,
    VTK_WEDGE = 13,
    VTK_PYRAMID = 14,
    VTK_QUADRATIC_EDGE = 21,
    VTK_QUADRATIC_TRIANGLE = 22,
    VTK_QUADRATIC_QUAD = 23,
    VTK_QUADRATIC_TETRA = 24,
    VTK_QUADRATIC_HEXAHEDRON = 25
  };

  class UnstructuredGrid {
   public:
    std::vector<Eigen::Vector3f> points;
    std::vector<std::vector<size_type>> cells;
    std::vector<CellType> cell_types;
  };

  template <class T>
  class FieldDataArray {
   public:
    std::string name;
    std::vector<T> data;

    FieldDataArray() = default;

    explicit FieldDataArray(std::string name, std::vector<T> data)
        : name(std::move(name)), data(std::move(data)) {}
  };

  /// Represents one set of attribute data (can be attached to points or cells)
  template <class T>
  class ScalarData {
   public:
    std::string name;
    std::vector<T> data;
    std::string lookup_table = "default";

    ScalarData() = default;

    explicit ScalarData(std::string data_name, std::vector<T> data,
                        std::string lookup_table = "default")
        : name(std::move(data_name)),
          data(std::move(data)),
          lookup_table(std::move(lookup_table)) {}
  };

  template <class T>
  class VectorData {
   public:
    std::string name;
    std::vector<Eigen::Matrix<T, 3, 1>> data;

    VectorData() = default;

    explicit VectorData(std::string name,
                        std::vector<Eigen::Matrix<T, 3, 1>> data)
        : name(std::move(name)), data(std::move(data)) {}
  };

  // WARNING: the type long is interepreted differently depending on the
  // platorm. I suggest to not use it at all.
  using Attributes = std::vector<boost::variant<
      ScalarData<char>, ScalarData<unsigned char>, ScalarData<short>,
      ScalarData<unsigned short>, ScalarData<int>, ScalarData<unsigned int>,
      ScalarData<long>, ScalarData<unsigned long>, ScalarData<float>,
      ScalarData<double>, VectorData<float>, VectorData<double>>>;

  using FieldData =
      std::vector<boost::variant<FieldDataArray<int>, FieldDataArray<float>,
                                 FieldDataArray<double>>>;

  // Actual members
  //////////////////////////////////////////////////////////////////////////////

  /// The Vtk Header, at most 256 characters, no new lines characters!
  std::string header;
  /// The format of the file.
  Format format = Format::ASCII;

  /// Describes the nodes + cells
  UnstructuredGrid unstructured_grid;

  FieldData field_data;

  // Data that is attached to points
  Attributes point_data;

  // Data that is attached to the cells of the mesh
  Attributes cell_data;
};

void WriteToFile(const VtkFile& vtk_file, const std::string& filename);

class VtkWriter {
 public:
  using dim_t = base::dim_t;

  VtkWriter(const VtkWriter&) = delete;
  VtkWriter(VtkWriter&&) = delete;
  VtkWriter& operator=(const VtkWriter&) = delete;
  VtkWriter& operator=(VtkWriter&&) = delete;

  /**
   * @brief Construct a new VtkWriter.
   * @param mesh The underlying mesh that should be written into the VtkFile.
   * @param filename The filename of the Vtk File
   * @param codim (Optional) the codimension of the cells, default is 0, i.e.
   *              the codim=0 entities of the mesh are written into the vtk
   *              files and data can be attached to these entities.
   *              If you set `codim=1`, the sekelton (only codim=1 entities)
   *              will be written into the vtk file and you can visualize data
   *              on the skeleton
   */
  VtkWriter(std::shared_ptr<mesh::Mesh> mesh, std::string filename,
            dim_t codim = 0);

  /**
   * @brief Determines whether the Vtk file is written in binary or ASCII mode
   *        (default).
   * @param binary  true if binary mode should be used, otherwise false
   */
  void setBinary(bool binary) {
    if (binary) {
      vtk_file_.format = VtkFile::Format::BINARY;
    } else {
      vtk_file_.format = VtkFile::Format::ASCII;
    }
  }

  /**
   * @brief Add a new `unsigned char` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<unsigned char>& mds,
                      unsigned char undefined_value = 0);

  /**
   * @brief Add a new `char` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name, const mesh::MeshDataSet<char>& mds,
                      char undefined_value = 0);

  /**
   * @brief Add a new `unsigned int` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<unsigned int>& mds,
                      unsigned undefined_value = 0);

  /**
   * @brief Add a new `unsigned int` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name, const mesh::MeshDataSet<int>& mds,
                      int undefined_value = 0);

  /**
   * @brief Add a new `float` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name, const mesh::MeshDataSet<float>& mds,
                      float undefined_value = 0.f);

  /**
   * @brief Add a new `double` attribute dataset that attaches data to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name, const mesh::MeshDataSet<double>& mds,
                      double undefined_value = 0.);

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<Eigen::Vector2d>& mds,
                      Eigen::Vector2d undefined_value = {0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<Eigen::Vector2f>& mds,
                      Eigen::Vector2f undefined_value = {0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<Eigen::Vector3d>& mds,
                      Eigen::Vector3d undefined_value = {0, 0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the points/nodes of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the points of
   *            the mesh.
   * @param undefined_value The value that should be written for a point to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WritePointData(std::string name,
                      const mesh::MeshDataSet<Eigen::Vector3f>& mds,
                      Eigen::Vector3f undefined_value = {0, 0, 0});

  /**
   * @brief Add a new `unsigned char` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<unsigned char>& mds,
                     unsigned char undefined_value = 0);

  /**
   * @brief Add a new `char` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name, const mesh::MeshDataSet<char>& mds,
                     char undefined_value = 0);

  /**
   * @brief Add a new `unsigned int` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<unsigned int>& mds,
                     unsigned int undefined_value = 0);

  /**
   * @brief Add a new `int` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name, const mesh::MeshDataSet<int>& mds,
                     int undefined_value = 0);

  /**
   * @brief Add a new `float` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name, const mesh::MeshDataSet<float>& mds,
                     float undefined_value = 0);

  /**
   * @brief Add a new `double` attribute dataset that attaches data to
   * the `cells` of the mesh (i.e. to entities with codim = "codim that was
   * specified in constructor of VtkWriter")
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name, const mesh::MeshDataSet<double>& mds,
                     double undefined_value = 0);

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the cells of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells of
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<Eigen::Vector2d>& mds,
                     Eigen::Vector2d undefined_value = {0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the cells of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells of
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<Eigen::Vector2f>& mds,
                     Eigen::Vector2f undefined_value = {0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the cells of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells of
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<Eigen::Vector3d>& mds,
                     Eigen::Vector3d undefined_value = {0, 0, 0});

  /**
   * @brief Add a new vector attribute dataset that attaches vectors to
   * the cells of the mesh.
   * @param name The name of the attribute set.
   * @param mds The mesh dataset that that attaches the data to the cells of
   *            the mesh.
   * @param undefined_value The value that should be written for a cell to
   * which `mds` does not attach data (i.e. if `mds.DefinedOn() == false`)
   */
  void WriteCellData(std::string name,
                     const mesh::MeshDataSet<Eigen::Vector3f>& mds,
                     Eigen::Vector3f undefined_value = {0, 0, 0});

  /**
   * @brief Write global data into the vtk file that is not related to the mesh
   *        at all.
   * @param name Name of the global dataset.
   * @param data The data series that should be written.
   *
   * This function writes so-called "Field Data" into the vtk file which can
   * be read and visualized by paraview. It is e.g. a convenient way to export
   * the simulation time or the global mesh size.
   */
  void WriteGlobalData(std::string name, std::vector<int> data);

  /**
   * @brief Write global data into the vtk file that is not related to the mesh
   *        at all.
   * @param name Name of the global dataset.
   * @param data The data series that should be written.
   *
   * This function writes so-called "Field Data" into the vtk file which can
   * be read and visualized by paraview. It is e.g. a convenient way to export
   * the simulation time or the global mesh size.
   */
  void WriteGlobalData(std::string name, std::vector<float> data);

  /**
   * @brief Write global data into the vtk file that is not related to the mesh
   *        at all.
   * @param name Name of the global dataset.
   * @param data The data series that should be written.
   *
   * This function writes so-called "Field Data" into the vtk file which can
   * be read and visualized by paraview. It is e.g. a convenient way to export
   * the simulation time or the global mesh size.
   */
  void WriteGlobalData(std::string name, std::vector<double> data);

  /**
   * @brief Destructor, writes everything into the file and closes it.
   */
  ~VtkWriter() { WriteToFile(vtk_file_, filename_); }

 private:
  std::shared_ptr<mesh::Mesh> mesh_;
  VtkFile vtk_file_;
  std::string filename_;
  dim_t codim_;

  template <class T>
  void WriteScalarPointData(std::string name, const mesh::MeshDataSet<T>&,
                            T undefined_value);

  template <int ROWS, class T>
  void WriteVectorPointData(
      std::string, const mesh::MeshDataSet<Eigen::Matrix<T, ROWS, 1>>& mds,
      Eigen::Matrix<T, ROWS, 1> undefined_value);

  template <class T>
  void WriteScalarCellData(std::string name, const mesh::MeshDataSet<T>& mds,
                           T undefined_value);

  template <int ROWS, class T>
  void WriteVectorCellData(
      std::string name, const mesh::MeshDataSet<Eigen::Matrix<T, ROWS, 1>>& mds,
      Eigen::Matrix<T, ROWS, 1> undefined_value);

  template <class T>
  void WriteFieldData(std::string name, std::vector<T> data);
};

}  // namespace lf::io

#endif  // __3e48c7b32a034cb3be3dbca884ff4f6c
