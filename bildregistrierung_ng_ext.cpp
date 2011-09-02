#ifdef __CPLUSPLUS__
extern "C" {
#endif

#ifndef __GNUC__
#pragma warning(disable: 4275)
#pragma warning(disable: 4101)

#endif
#include "Python.h"
#include "blitz/array.h"
#include "compile.h"
#include "frameobject.h"
#include <complex>
#include <math.h>
#include <string>
#include "scxx/object.h"
#include "scxx/list.h"
#include "scxx/tuple.h"
#include "scxx/dict.h"
#include <iostream>
#include <stdio.h>
#include "numpy/arrayobject.h"




// global None value for use in functions.
namespace py {
object None = object(Py_None);
}

const char* find_type(PyObject* py_obj)
{
    if(py_obj == NULL) return "C NULL value";
    if(PyCallable_Check(py_obj)) return "callable";
    if(PyString_Check(py_obj)) return "string";
    if(PyInt_Check(py_obj)) return "int";
    if(PyFloat_Check(py_obj)) return "float";
    if(PyDict_Check(py_obj)) return "dict";
    if(PyList_Check(py_obj)) return "list";
    if(PyTuple_Check(py_obj)) return "tuple";
    if(PyFile_Check(py_obj)) return "file";
    if(PyModule_Check(py_obj)) return "module";

    //should probably do more intergation (and thinking) on these.
    if(PyCallable_Check(py_obj) && PyInstance_Check(py_obj)) return "callable";
    if(PyInstance_Check(py_obj)) return "instance";
    if(PyCallable_Check(py_obj)) return "callable";
    return "unkown type";
}

void throw_error(PyObject* exc, const char* msg)
{
 //printf("setting python error: %s\n",msg);
  PyErr_SetString(exc, msg);
  //printf("throwing error\n");
  throw 1;
}

void handle_bad_type(PyObject* py_obj, const char* good_type, const char* var_name)
{
    char msg[500];
    sprintf(msg,"received '%s' type instead of '%s' for variable '%s'",
            find_type(py_obj),good_type,var_name);
    throw_error(PyExc_TypeError,msg);
}

void handle_conversion_error(PyObject* py_obj, const char* good_type, const char* var_name)
{
    char msg[500];
    sprintf(msg,"Conversion Error:, received '%s' type instead of '%s' for variable '%s'",
            find_type(py_obj),good_type,var_name);
    throw_error(PyExc_TypeError,msg);
}


class int_handler
{
public:
    int convert_to_int(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyInt_Check(py_obj))
            handle_conversion_error(py_obj,"int", name);
        return (int) PyInt_AsLong(py_obj);
    }

    int py_to_int(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyInt_Check(py_obj))
            handle_bad_type(py_obj,"int", name);
        
        return (int) PyInt_AsLong(py_obj);
    }
};

int_handler x__int_handler = int_handler();
#define convert_to_int(py_obj,name) \
        x__int_handler.convert_to_int(py_obj,name)
#define py_to_int(py_obj,name) \
        x__int_handler.py_to_int(py_obj,name)


PyObject* int_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class float_handler
{
public:
    double convert_to_float(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyFloat_Check(py_obj))
            handle_conversion_error(py_obj,"float", name);
        return PyFloat_AsDouble(py_obj);
    }

    double py_to_float(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyFloat_Check(py_obj))
            handle_bad_type(py_obj,"float", name);
        
        return PyFloat_AsDouble(py_obj);
    }
};

float_handler x__float_handler = float_handler();
#define convert_to_float(py_obj,name) \
        x__float_handler.convert_to_float(py_obj,name)
#define py_to_float(py_obj,name) \
        x__float_handler.py_to_float(py_obj,name)


PyObject* float_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class complex_handler
{
public:
    std::complex<double> convert_to_complex(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyComplex_Check(py_obj))
            handle_conversion_error(py_obj,"complex", name);
        return std::complex<double>(PyComplex_RealAsDouble(py_obj),PyComplex_ImagAsDouble(py_obj));
    }

    std::complex<double> py_to_complex(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyComplex_Check(py_obj))
            handle_bad_type(py_obj,"complex", name);
        
        return std::complex<double>(PyComplex_RealAsDouble(py_obj),PyComplex_ImagAsDouble(py_obj));
    }
};

complex_handler x__complex_handler = complex_handler();
#define convert_to_complex(py_obj,name) \
        x__complex_handler.convert_to_complex(py_obj,name)
#define py_to_complex(py_obj,name) \
        x__complex_handler.py_to_complex(py_obj,name)


PyObject* complex_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class unicode_handler
{
public:
    Py_UNICODE* convert_to_unicode(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        Py_XINCREF(py_obj);
        if (!py_obj || !PyUnicode_Check(py_obj))
            handle_conversion_error(py_obj,"unicode", name);
        return PyUnicode_AS_UNICODE(py_obj);
    }

    Py_UNICODE* py_to_unicode(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyUnicode_Check(py_obj))
            handle_bad_type(py_obj,"unicode", name);
        Py_XINCREF(py_obj);
        return PyUnicode_AS_UNICODE(py_obj);
    }
};

unicode_handler x__unicode_handler = unicode_handler();
#define convert_to_unicode(py_obj,name) \
        x__unicode_handler.convert_to_unicode(py_obj,name)
#define py_to_unicode(py_obj,name) \
        x__unicode_handler.py_to_unicode(py_obj,name)


PyObject* unicode_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class string_handler
{
public:
    std::string convert_to_string(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        Py_XINCREF(py_obj);
        if (!py_obj || !PyString_Check(py_obj))
            handle_conversion_error(py_obj,"string", name);
        return std::string(PyString_AsString(py_obj));
    }

    std::string py_to_string(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyString_Check(py_obj))
            handle_bad_type(py_obj,"string", name);
        Py_XINCREF(py_obj);
        return std::string(PyString_AsString(py_obj));
    }
};

string_handler x__string_handler = string_handler();
#define convert_to_string(py_obj,name) \
        x__string_handler.convert_to_string(py_obj,name)
#define py_to_string(py_obj,name) \
        x__string_handler.py_to_string(py_obj,name)


               PyObject* string_to_py(std::string s)
               {
                   return PyString_FromString(s.c_str());
               }
               
class list_handler
{
public:
    py::list convert_to_list(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyList_Check(py_obj))
            handle_conversion_error(py_obj,"list", name);
        return py::list(py_obj);
    }

    py::list py_to_list(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyList_Check(py_obj))
            handle_bad_type(py_obj,"list", name);
        
        return py::list(py_obj);
    }
};

list_handler x__list_handler = list_handler();
#define convert_to_list(py_obj,name) \
        x__list_handler.convert_to_list(py_obj,name)
#define py_to_list(py_obj,name) \
        x__list_handler.py_to_list(py_obj,name)


PyObject* list_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class dict_handler
{
public:
    py::dict convert_to_dict(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyDict_Check(py_obj))
            handle_conversion_error(py_obj,"dict", name);
        return py::dict(py_obj);
    }

    py::dict py_to_dict(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyDict_Check(py_obj))
            handle_bad_type(py_obj,"dict", name);
        
        return py::dict(py_obj);
    }
};

dict_handler x__dict_handler = dict_handler();
#define convert_to_dict(py_obj,name) \
        x__dict_handler.convert_to_dict(py_obj,name)
#define py_to_dict(py_obj,name) \
        x__dict_handler.py_to_dict(py_obj,name)


PyObject* dict_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class tuple_handler
{
public:
    py::tuple convert_to_tuple(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyTuple_Check(py_obj))
            handle_conversion_error(py_obj,"tuple", name);
        return py::tuple(py_obj);
    }

    py::tuple py_to_tuple(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyTuple_Check(py_obj))
            handle_bad_type(py_obj,"tuple", name);
        
        return py::tuple(py_obj);
    }
};

tuple_handler x__tuple_handler = tuple_handler();
#define convert_to_tuple(py_obj,name) \
        x__tuple_handler.convert_to_tuple(py_obj,name)
#define py_to_tuple(py_obj,name) \
        x__tuple_handler.py_to_tuple(py_obj,name)


PyObject* tuple_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class file_handler
{
public:
    FILE* convert_to_file(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        Py_XINCREF(py_obj);
        if (!py_obj || !PyFile_Check(py_obj))
            handle_conversion_error(py_obj,"file", name);
        return PyFile_AsFile(py_obj);
    }

    FILE* py_to_file(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyFile_Check(py_obj))
            handle_bad_type(py_obj,"file", name);
        Py_XINCREF(py_obj);
        return PyFile_AsFile(py_obj);
    }
};

file_handler x__file_handler = file_handler();
#define convert_to_file(py_obj,name) \
        x__file_handler.convert_to_file(py_obj,name)
#define py_to_file(py_obj,name) \
        x__file_handler.py_to_file(py_obj,name)


               PyObject* file_to_py(FILE* file, const char* name,
                                    const char* mode)
               {
                   return (PyObject*) PyFile_FromFile(file,
                     const_cast<char*>(name),
                     const_cast<char*>(mode), fclose);
               }
               
class instance_handler
{
public:
    py::object convert_to_instance(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !PyInstance_Check(py_obj))
            handle_conversion_error(py_obj,"instance", name);
        return py::object(py_obj);
    }

    py::object py_to_instance(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyInstance_Check(py_obj))
            handle_bad_type(py_obj,"instance", name);
        
        return py::object(py_obj);
    }
};

instance_handler x__instance_handler = instance_handler();
#define convert_to_instance(py_obj,name) \
        x__instance_handler.convert_to_instance(py_obj,name)
#define py_to_instance(py_obj,name) \
        x__instance_handler.py_to_instance(py_obj,name)


PyObject* instance_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class numpy_size_handler
{
public:
    void conversion_numpy_check_size(PyArrayObject* arr_obj, int Ndims,
                                     const char* name)
    {
        if (arr_obj->nd != Ndims)
        {
            char msg[500];
            sprintf(msg,"Conversion Error: received '%d' dimensional array instead of '%d' dimensional array for variable '%s'",
                    arr_obj->nd,Ndims,name);
            throw_error(PyExc_TypeError,msg);
        }
    }

    void numpy_check_size(PyArrayObject* arr_obj, int Ndims, const char* name)
    {
        if (arr_obj->nd != Ndims)
        {
            char msg[500];
            sprintf(msg,"received '%d' dimensional array instead of '%d' dimensional array for variable '%s'",
                    arr_obj->nd,Ndims,name);
            throw_error(PyExc_TypeError,msg);
        }
    }
};

numpy_size_handler x__numpy_size_handler = numpy_size_handler();
#define conversion_numpy_check_size x__numpy_size_handler.conversion_numpy_check_size
#define numpy_check_size x__numpy_size_handler.numpy_check_size


class numpy_type_handler
{
public:
    void conversion_numpy_check_type(PyArrayObject* arr_obj, int numeric_type,
                                     const char* name)
    {
        // Make sure input has correct numeric type.
        int arr_type = arr_obj->descr->type_num;
        if (PyTypeNum_ISEXTENDED(numeric_type))
        {
        char msg[80];
        sprintf(msg, "Conversion Error: extended types not supported for variable '%s'",
                name);
        throw_error(PyExc_TypeError, msg);
        }
        if (!PyArray_EquivTypenums(arr_type, numeric_type))
        {

        const char* type_names[23] = {"bool", "byte", "ubyte","short", "ushort",
                                "int", "uint", "long", "ulong", "longlong", "ulonglong",
                                "float", "double", "longdouble", "cfloat", "cdouble",
                                "clongdouble", "object", "string", "unicode", "void", "ntype",
                                "unknown"};
        char msg[500];
        sprintf(msg,"Conversion Error: received '%s' typed array instead of '%s' typed array for variable '%s'",
                type_names[arr_type],type_names[numeric_type],name);
        throw_error(PyExc_TypeError,msg);
        }
    }

    void numpy_check_type(PyArrayObject* arr_obj, int numeric_type, const char* name)
    {
        // Make sure input has correct numeric type.
        int arr_type = arr_obj->descr->type_num;
        if (PyTypeNum_ISEXTENDED(numeric_type))
        {
        char msg[80];
        sprintf(msg, "Conversion Error: extended types not supported for variable '%s'",
                name);
        throw_error(PyExc_TypeError, msg);
        }
        if (!PyArray_EquivTypenums(arr_type, numeric_type))
        {
            const char* type_names[23] = {"bool", "byte", "ubyte","short", "ushort",
                                    "int", "uint", "long", "ulong", "longlong", "ulonglong",
                                    "float", "double", "longdouble", "cfloat", "cdouble",
                                    "clongdouble", "object", "string", "unicode", "void", "ntype",
                                    "unknown"};
            char msg[500];
            sprintf(msg,"received '%s' typed array instead of '%s' typed array for variable '%s'",
                    type_names[arr_type],type_names[numeric_type],name);
            throw_error(PyExc_TypeError,msg);
        }
    }
};

numpy_type_handler x__numpy_type_handler = numpy_type_handler();
#define conversion_numpy_check_type x__numpy_type_handler.conversion_numpy_check_type
#define numpy_check_type x__numpy_type_handler.numpy_check_type


class numpy_handler
{
public:
    PyArrayObject* convert_to_numpy(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        Py_XINCREF(py_obj);
        if (!py_obj || !PyArray_Check(py_obj))
            handle_conversion_error(py_obj,"numpy", name);
        return (PyArrayObject*) py_obj;
    }

    PyArrayObject* py_to_numpy(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !PyArray_Check(py_obj))
            handle_bad_type(py_obj,"numpy", name);
        Py_XINCREF(py_obj);
        return (PyArrayObject*) py_obj;
    }
};

numpy_handler x__numpy_handler = numpy_handler();
#define convert_to_numpy(py_obj,name) \
        x__numpy_handler.convert_to_numpy(py_obj,name)
#define py_to_numpy(py_obj,name) \
        x__numpy_handler.py_to_numpy(py_obj,name)


PyObject* numpy_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}


class catchall_handler
{
public:
    py::object convert_to_catchall(PyObject* py_obj, const char* name)
    {
        // Incref occurs even if conversion fails so that
        // the decref in cleanup_code has a matching incref.
        
        if (!py_obj || !(py_obj))
            handle_conversion_error(py_obj,"catchall", name);
        return py::object(py_obj);
    }

    py::object py_to_catchall(PyObject* py_obj, const char* name)
    {
        // !! Pretty sure INCREF should only be called on success since
        // !! py_to_xxx is used by the user -- not the code generator.
        if (!py_obj || !(py_obj))
            handle_bad_type(py_obj,"catchall", name);
        
        return py::object(py_obj);
    }
};

catchall_handler x__catchall_handler = catchall_handler();
#define convert_to_catchall(py_obj,name) \
        x__catchall_handler.convert_to_catchall(py_obj,name)
#define py_to_catchall(py_obj,name) \
        x__catchall_handler.py_to_catchall(py_obj,name)


PyObject* catchall_to_py(PyObject* obj)
{
    return (PyObject*) obj;
}



// This should be declared only if they are used by some function
// to keep from generating needless warnings. for now, we'll always
// declare them.

int _beg = blitz::fromStart;
int _end = blitz::toEnd;
blitz::Range _all = blitz::Range::all();

template<class T, int N>
static blitz::Array<T,N> convert_to_blitz(PyArrayObject* arr_obj,const char* name)
{
    blitz::TinyVector<int,N> shape(0);
    blitz::TinyVector<int,N> strides(0);
    //for (int i = N-1; i >=0; i--)
    for (int i = 0; i < N; i++)
    {
        shape[i] = arr_obj->dimensions[i];
        strides[i] = arr_obj->strides[i]/sizeof(T);
    }
    //return blitz::Array<T,N>((T*) arr_obj->data,shape,
    return blitz::Array<T,N>((T*) arr_obj->data,shape,strides,
                             blitz::neverDeleteData);
}

template<class T, int N>
static blitz::Array<T,N> py_to_blitz(PyArrayObject* arr_obj,const char* name)
{

    blitz::TinyVector<int,N> shape(0);
    blitz::TinyVector<int,N> strides(0);
    //for (int i = N-1; i >=0; i--)
    for (int i = 0; i < N; i++)
    {
        shape[i] = arr_obj->dimensions[i];
        strides[i] = arr_obj->strides[i]/sizeof(T);
    }
    //return blitz::Array<T,N>((T*) arr_obj->data,shape,
    return blitz::Array<T,N>((T*) arr_obj->data,shape,strides,
                             blitz::neverDeleteData);
}
#include <cmath> 


static PyObject* correlation(PyObject*self, PyObject* args, PyObject* kywds)
{
    py::object return_val;
    int exception_occured = 0;
    PyObject *py_local_dict = NULL;
    static const char *kwlist[] = {"I","T","local_dict", NULL};
    PyObject *py_I, *py_T;
    int I_used, T_used;
    py_I = py_T = NULL;
    I_used= T_used = 0;
    
    if(!PyArg_ParseTupleAndKeywords(args,kywds,"OO|O:correlation",const_cast<char**>(kwlist),&py_I, &py_T, &py_local_dict))
       return NULL;
    try                              
    {                                
        py_I = py_I;
        PyArrayObject* I_array = convert_to_numpy(py_I,"I");
        conversion_numpy_check_type(I_array,PyArray_DOUBLE,"I");
        conversion_numpy_check_size(I_array,2,"I");
        blitz::Array<double,2> I = convert_to_blitz<double,2>(I_array,"I");
        blitz::TinyVector<int,2> NI = I.shape();
        I_used = 1;
        py_T = py_T;
        PyArrayObject* T_array = convert_to_numpy(py_T,"T");
        conversion_numpy_check_type(T_array,PyArray_DOUBLE,"T");
        conversion_numpy_check_size(T_array,2,"T");
        blitz::Array<double,2> T = convert_to_blitz<double,2>(T_array,"T");
        blitz::TinyVector<int,2> NT = T.shape();
        T_used = 1;
        /*<function call here>*/     
         
            double I_mean = mean(I);
            double T_mean = mean(T);
            return_val = mean((I-I_mean) * (T-T_mean)) / (sqrt(mean(pow2(I-I_mean))) * sqrt(mean(pow2(T-T_mean))));
        if(py_local_dict)                                  
        {                                                  
            py::dict local_dict = py::dict(py_local_dict); 
        }                                                  
    
    }                                
    catch(...)                       
    {                                
        return_val =  py::object();      
        exception_occured = 1;       
    }                                
    /*cleanup code*/                     
    if(I_used)
    {
        Py_XDECREF(py_I);
    }
    if(T_used)
    {
        Py_XDECREF(py_T);
    }
    if(!(PyObject*)return_val && !exception_occured)
    {
                                  
        return_val = Py_None;            
    }
                                  
    return return_val.disown();           
}                                
static PyObject* transform_part(PyObject*self, PyObject* args, PyObject* kywds)
{
    py::object return_val;
    int exception_occured = 0;
    PyObject *py_local_dict = NULL;
    static const char *kwlist[] = {"data","transformation","result","left","top","local_dict", NULL};
    PyObject *py_data, *py_transformation, *py_result, *py_left, *py_top;
    int data_used, transformation_used, result_used, left_used, top_used;
    py_data = py_transformation = py_result = py_left = py_top = NULL;
    data_used= transformation_used= result_used= left_used= top_used = 0;
    
    if(!PyArg_ParseTupleAndKeywords(args,kywds,"OOOOO|O:transform_part",const_cast<char**>(kwlist),&py_data, &py_transformation, &py_result, &py_left, &py_top, &py_local_dict))
       return NULL;
    try                              
    {                                
        py_data = py_data;
        PyArrayObject* data_array = convert_to_numpy(py_data,"data");
        conversion_numpy_check_type(data_array,PyArray_DOUBLE,"data");
        conversion_numpy_check_size(data_array,2,"data");
        blitz::Array<double,2> data = convert_to_blitz<double,2>(data_array,"data");
        blitz::TinyVector<int,2> Ndata = data.shape();
        data_used = 1;
        py_transformation = py_transformation;
        PyArrayObject* transformation_array = convert_to_numpy(py_transformation,"transformation");
        conversion_numpy_check_type(transformation_array,PyArray_DOUBLE,"transformation");
        conversion_numpy_check_size(transformation_array,1,"transformation");
        blitz::Array<double,1> transformation = convert_to_blitz<double,1>(transformation_array,"transformation");
        blitz::TinyVector<int,1> Ntransformation = transformation.shape();
        transformation_used = 1;
        py_result = py_result;
        PyArrayObject* result_array = convert_to_numpy(py_result,"result");
        conversion_numpy_check_type(result_array,PyArray_DOUBLE,"result");
        conversion_numpy_check_size(result_array,2,"result");
        blitz::Array<double,2> result = convert_to_blitz<double,2>(result_array,"result");
        blitz::TinyVector<int,2> Nresult = result.shape();
        result_used = 1;
        py_left = py_left;
        int left = convert_to_int(py_left,"left");
        left_used = 1;
        py_top = py_top;
        int top = convert_to_int(py_top,"top");
        top_used = 1;
        /*<function call here>*/     
        
            # line 10047 "bildregistrierung-ng.py"
            // transform backwards, ->   / scale
            double dx = transformation(0);
            double dy = transformation(1);
            double alpha = transformation(2);
            double scale = transformation(3);
        
            float c = cos(alpha);
            float s = sin(alpha);
        
            int w  = data.extent(1);
            int h  = data.extent(0);
            int rw = result.extent(1);
            int rh = result.extent(0);
        
            for (int ry = 0; ry < rh; ry++){
                for(int rx = 0; rx < rw; rx++){
        
                    int x = rx + left;
                    int y = ry + top;
        
                    // transform backwards
                    int tx = (int) w/2 + c * (x-w/2) + s * (y-h/2) - dx;
                    int ty = (int) h/2 - s * (x-w/2) + c * (y-h/2) - dy;
                    result(ry, rx) = (tx >= 0 && tx < w && ty >= 0 && ty < h ?
                                    data(ty, tx): -1);
                }
            }
        if(py_local_dict)                                  
        {                                                  
            py::dict local_dict = py::dict(py_local_dict); 
        }                                                  
    
    }                                
    catch(...)                       
    {                                
        return_val =  py::object();      
        exception_occured = 1;       
    }                                
    /*cleanup code*/                     
    if(data_used)
    {
        Py_XDECREF(py_data);
    }
    if(transformation_used)
    {
        Py_XDECREF(py_transformation);
    }
    if(result_used)
    {
        Py_XDECREF(py_result);
    }
    if(!(PyObject*)return_val && !exception_occured)
    {
                                  
        return_val = Py_None;            
    }
                                  
    return return_val.disown();           
}                                
static PyObject* find_best_for_angle(PyObject*self, PyObject* args, PyObject* kywds)
{
    py::object return_val;
    int exception_occured = 0;
    PyObject *py_local_dict = NULL;
    static const char *kwlist[] = {"xr","yr","dx","dy","kwerte","maxdelta","alpha","scale","translation_stepsize","result","local_dict", NULL};
    PyObject *py_xr, *py_yr, *py_dx, *py_dy, *py_kwerte, *py_maxdelta, *py_alpha, *py_scale, *py_translation_stepsize, *py_result;
    int xr_used, yr_used, dx_used, dy_used, kwerte_used, maxdelta_used, alpha_used, scale_used, translation_stepsize_used, result_used;
    py_xr = py_yr = py_dx = py_dy = py_kwerte = py_maxdelta = py_alpha = py_scale = py_translation_stepsize = py_result = NULL;
    xr_used= yr_used= dx_used= dy_used= kwerte_used= maxdelta_used= alpha_used= scale_used= translation_stepsize_used= result_used = 0;
    
    if(!PyArg_ParseTupleAndKeywords(args,kywds,"OOOOOOOOOO|O:find_best_for_angle",const_cast<char**>(kwlist),&py_xr, &py_yr, &py_dx, &py_dy, &py_kwerte, &py_maxdelta, &py_alpha, &py_scale, &py_translation_stepsize, &py_result, &py_local_dict))
       return NULL;
    try                              
    {                                
        py_xr = py_xr;
        PyArrayObject* xr_array = convert_to_numpy(py_xr,"xr");
        conversion_numpy_check_type(xr_array,PyArray_LONG,"xr");
        conversion_numpy_check_size(xr_array,1,"xr");
        blitz::Array<long,1> xr = convert_to_blitz<long,1>(xr_array,"xr");
        blitz::TinyVector<int,1> Nxr = xr.shape();
        xr_used = 1;
        py_yr = py_yr;
        PyArrayObject* yr_array = convert_to_numpy(py_yr,"yr");
        conversion_numpy_check_type(yr_array,PyArray_LONG,"yr");
        conversion_numpy_check_size(yr_array,1,"yr");
        blitz::Array<long,1> yr = convert_to_blitz<long,1>(yr_array,"yr");
        blitz::TinyVector<int,1> Nyr = yr.shape();
        yr_used = 1;
        py_dx = py_dx;
        double dx = convert_to_float(py_dx,"dx");
        dx_used = 1;
        py_dy = py_dy;
        double dy = convert_to_float(py_dy,"dy");
        dy_used = 1;
        py_kwerte = py_kwerte;
        PyArrayObject* kwerte_array = convert_to_numpy(py_kwerte,"kwerte");
        conversion_numpy_check_type(kwerte_array,PyArray_DOUBLE,"kwerte");
        conversion_numpy_check_size(kwerte_array,3,"kwerte");
        blitz::Array<double,3> kwerte = convert_to_blitz<double,3>(kwerte_array,"kwerte");
        blitz::TinyVector<int,3> Nkwerte = kwerte.shape();
        kwerte_used = 1;
        py_maxdelta = py_maxdelta;
        int maxdelta = convert_to_int(py_maxdelta,"maxdelta");
        maxdelta_used = 1;
        py_alpha = py_alpha;
        double alpha = convert_to_float(py_alpha,"alpha");
        alpha_used = 1;
        py_scale = py_scale;
        double scale = convert_to_float(py_scale,"scale");
        scale_used = 1;
        py_translation_stepsize = py_translation_stepsize;
        double translation_stepsize = convert_to_float(py_translation_stepsize,"translation_stepsize");
        translation_stepsize_used = 1;
        py_result = py_result;
        PyArrayObject* result_array = convert_to_numpy(py_result,"result");
        conversion_numpy_check_type(result_array,PyArray_DOUBLE,"result");
        conversion_numpy_check_size(result_array,1,"result");
        blitz::Array<double,1> result = convert_to_blitz<double,1>(result_array,"result");
        blitz::TinyVector<int,1> Nresult = result.shape();
        result_used = 1;
        /*<function call here>*/     
        
            #line 100100 "bildregistrierung-ng.py"
            using namespace blitz;
        
            float c = cos(alpha) * scale;
            float s = sin(alpha) * scale;
        
            // verschiebung der einzelnen Punkte durch rotation um das zentrum
            Array<double,1> rdx ((c-1) * xr  - s     * yr);
            Array<double,1> rdy (s     * xr  + (c-1) * yr);
        
            // maximale verschiebung in beide richtungen
        
            double max_rot_delta_x = max(abs(rdx));
            double max_rot_delta_y = max(abs(rdy));
        
            const int kwerte_size = kwerte.extent(1);
        
            double xstart = max(-kwerte_size/2 + max_rot_delta_x + 1, dx - maxdelta);
            double ystart = max(-kwerte_size/2 + max_rot_delta_y + 1, dy - maxdelta);
            double xend   = min(+kwerte_size/2 - max_rot_delta_x - 1, dx + maxdelta);
            double yend   = min(+kwerte_size/2 - max_rot_delta_x - 1, dx + maxdelta);
        
            //printf("minrdx = %f maxrdx = %f minrdy = %f maxrdy = %f, x: %f:%f y: %f:%f\n",
            //       (float)minrdx, (float) maxrdx, (float) minrdy, (float) maxrdy, 
            //       (float) xstart, (float)xend, (float)ystart, (float)yend);
            //printf("x: %lf:%lf, y: %lf:%lf\n", xstart, xend, ystart, yend);
        
            for (double dx = xstart; dx < xend; dx+= translation_stepsize){
                for (double dy = ystart; dy < yend; dy+= translation_stepsize){
                    double tmp = 1;
        
        
                    int size = kwerte.extent(1);
                    for(int i = 0; i < rdx.extent(0); i++){
                        //           rotation  verschiebung  runden  relativ zur mitte der kwerte
                        int xi = (int) (rdx(i) + dx          +  0.5 + kwerte.extent(1)/2);
                        int yi = (int) (rdy(i) + dy          +  0.5 + kwerte.extent(2)/2);
                        if (xi < 0 || xi >= size || xi < 0 || yi >= size){
                            //printf("dx, dy = %lf %lf\n",dx, dy);
                        }
                        tmp *= kwerte(i, yi, xi);
                    }
        
                    if (tmp > result(0)){
                        result = tmp, dx, dy, alpha, scale;
                        //printf("---- %lf %lf %i %i\n",(double) tmp, (double)result(0), (int)dx, (int)dy);
                    }
                }
            }
        
        if(py_local_dict)                                  
        {                                                  
            py::dict local_dict = py::dict(py_local_dict); 
        }                                                  
    
    }                                
    catch(...)                       
    {                                
        return_val =  py::object();      
        exception_occured = 1;       
    }                                
    /*cleanup code*/                     
    if(xr_used)
    {
        Py_XDECREF(py_xr);
    }
    if(yr_used)
    {
        Py_XDECREF(py_yr);
    }
    if(kwerte_used)
    {
        Py_XDECREF(py_kwerte);
    }
    if(result_used)
    {
        Py_XDECREF(py_result);
    }
    if(!(PyObject*)return_val && !exception_occured)
    {
                                  
        return_val = Py_None;            
    }
                                  
    return return_val.disown();           
}                                


static PyMethodDef compiled_methods[] = 
{
    {"correlation",(PyCFunction)correlation , METH_VARARGS|METH_KEYWORDS},
    {"transform_part",(PyCFunction)transform_part , METH_VARARGS|METH_KEYWORDS},
    {"find_best_for_angle",(PyCFunction)find_best_for_angle , METH_VARARGS|METH_KEYWORDS},
    {NULL,      NULL}        /* Sentinel */
};

PyMODINIT_FUNC initbildregistrierung_ng_ext(void)
{
    
    Py_Initialize();
    import_array();
    PyImport_ImportModule("numpy");
    (void) Py_InitModule("bildregistrierung_ng_ext", compiled_methods);
}

#ifdef __CPLUSCPLUS__
}
#endif
