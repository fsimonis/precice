#ifndef PRECICE_NO_PYTHON

#include "PythonAction.hpp"
#include <Eigen/Core>
#include <Python.h>
#include <boost/filesystem/operations.hpp>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <pthread.h>
#include <string>
#include <utility>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "logging/LogMacros.hpp"
#include "mesh/Data.hpp"
#include "mesh/Mesh.hpp"
#include "utils/String.hpp"
#include "utils/assertion.hpp"

namespace precice::action {

namespace {
std::string python_error_as_string()
{
  PyObject *ptype, *pvalue, *ptraceback;
  PyErr_Fetch(&ptype, &pvalue, &ptraceback);
  if (ptype == nullptr) {
    return "<no error available>";
  } else {
    // pvalue and ptraceback may be NULL
    // We don't need the type or the traceback, so we dereference them straight away
    Py_DECREF(ptype);
    Py_XDECREF(ptraceback); // may be NULL

    if (pvalue == nullptr) {
      return "<no error message available>";
    }
    wchar_t *wmessage = PyUnicode_AsWideCharString(pvalue, nullptr);
    Py_DECREF(pvalue);

    if (wmessage) {
      auto message = utils::truncate_wstring_to_string(wmessage);
      PyMem_Free(wmessage);
      return message;
    } else {
      return "<fetching error message failed>";
    }
  }
}
} // namespace

PythonAction::PythonAction(
    Timing               timing,
    std::string          modulePath,
    std::string          moduleName,
    const mesh::PtrMesh &mesh,
    int                  targetDataID,
    int                  sourceDataID)
    : Action(timing, mesh),
      _modulePath(std::move(modulePath)),
      _moduleName(std::move(moduleName))
{
  PRECICE_CHECK(boost::filesystem::is_directory(_modulePath),
                "The module path of the python action \"{}\" does not exist. The configured path is \"{}\".",
                _moduleName, _modulePath);
  if (targetDataID != -1) {
    _targetData = getMesh()->data(targetDataID);
    _numberArguments++;
  }
  if (sourceDataID != -1) {
    _sourceData = getMesh()->data(sourceDataID);
    _numberArguments++;
  }
}

PythonAction::~PythonAction()
{
  if (_module != nullptr) {
    PRECICE_ASSERT(_moduleNameObject != nullptr);
    PRECICE_ASSERT(_module != nullptr);
    Py_DECREF(_moduleNameObject);
    Py_DECREF(_module);
    Py_Finalize();
  }
}

void PythonAction::performAction()
{
  PRECICE_TRACE();

  if (not _isInitialized) {
    initialize();
  }

  PyObject *dataArgs = PyTuple_New(_numberArguments);

  int i = 0;
  for (auto &targetStample : _targetData->timeStepsStorage().stamples()) { // iterate over _targetData, because it must always exist
    PRECICE_ASSERT(_targetData);                                           // _targetData is mandatory, cannot call setSampleAtTime, if target data is not provided!

    PyObject *pythonTime = PyFloat_FromDouble(targetStample.timestamp);
    PyTuple_SetItem(dataArgs, 0, pythonTime);

    if (_sourceData) {                                                       // _sourceData is optional
      auto &sourceStample = _sourceData->timeStepsStorage().stamples()[i++]; // simultaneously iterate over _targetData->stamples()
      PRECICE_CHECK(math::equals(sourceStample.timestamp, targetStample.timestamp), "Trying to perform python action on samples with different timestamps: {} for source data and {} for target data. Time mesh of source data and target data must agree.", sourceStample.timestamp, targetStample.timestamp);

      auto &   sourceValues = sourceStample.sample.values; // put data into temporary buffer
      npy_intp sourceDim[]  = {sourceValues.size()};
      double * sourceData   = sourceValues.data();
      _sourceValues         = PyArray_SimpleNewFromData(1, sourceDim, NPY_DOUBLE, sourceData);
      PRECICE_CHECK(_sourceValues != nullptr, "Creating python source values failed. Please check that the source data name is used by the mesh in action:python.");
      PyTuple_SetItem(dataArgs, 1, _sourceValues);
    }

    auto &   sample       = targetStample.sample;
    npy_intp targetDim[]  = {sample.values.size()};
    double * targetValues = sample.values.data();

    _targetValues = PyArray_SimpleNewFromData(1, targetDim, NPY_DOUBLE, targetValues);
    PRECICE_CHECK(_targetValues != nullptr, "Creating python target values failed. Please check that the target data name is used by the mesh in action:python.");
    int argumentIndex = _sourceData ? 2 : 1;
    PyTuple_SetItem(dataArgs, argumentIndex, _targetValues);

    PyObject_CallObject(_performAction, dataArgs);
    PRECICE_CHECK(!PyErr_Occurred(),
                  "Error occurred during call of function performAction() in python module \"{}\". "
                  "The error message is: {}",
                  _moduleName, python_error_as_string());
  }
  Py_DECREF(dataArgs);
  _targetData->updateSample();
}

void PythonAction::initialize()
{
  PRECICE_ASSERT(not _isInitialized);
  // Initialize Python
  Py_Initialize();
  makeNumPyArraysAvailable();
  // Append execution path to find module to import
  PyRun_SimpleString("import sys");
  std::string appendPathCommand("sys.path.append('" + _modulePath + "')");
  PyRun_SimpleString(appendPathCommand.c_str());
  _moduleNameObject = PyUnicode_FromString(_moduleName.c_str());
  _module           = PyImport_Import(_moduleNameObject);
  if (_module == nullptr) {
    PRECICE_ERROR("An error occurred while loading python module \"{}\": {}", _moduleName, python_error_as_string());
  }

  // Construct method performAction
  _performAction = PyObject_GetAttrString(_module, "performAction");
  if (PyErr_Occurred()) {
    PyErr_Clear();
    PRECICE_WARN("Python module \"{}\" does not define function performAction().", _moduleName);
    _performAction = nullptr;
  }
}

int PythonAction::makeNumPyArraysAvailable()
{
  static bool importedAlready = false;
  if (importedAlready)
    return 0;
  import_array1(-1); // this macro is defined be NumPy and must be included
  importedAlready = true;
  return 1;
}

} // namespace precice::action

#endif
