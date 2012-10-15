#include <Python.h>

// hawkey
#include "src/selector.h"

// pyhawkey
#include "exception-py.h"
#include "sack-py.h"
#include "selector-py.h"

typedef struct {
    PyObject_HEAD
    HySelector sltr;
    PyObject *sack;
} _SelectorObject;

int
selector_converter(PyObject *o, HySelector *sltr_ptr)
{
    if (!PyType_IsSubtype(o->ob_type, &selector_Type)) {
	PyErr_SetString(PyExc_TypeError, "Expected a Selector object.");
	return 0;
    }
    *sltr_ptr = ((_SelectorObject *)o)->sltr;

    return 1;
}

static PyObject *
selector_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    _SelectorObject *self = (_SelectorObject*)type->tp_alloc(type, 0);
    if (self) {
	self->sltr = NULL;
	self->sack = NULL;
    }
    return (PyObject*)self;
}

static int
selector_init(_SelectorObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *sack;
    HySack csack;

    if (!PyArg_ParseTuple(args, "O!", &sack_Type, &sack))
	return -1;
    csack = sackFromPyObject(sack);
    if (csack == NULL)
	return -1;
    self->sack = sack;
    Py_INCREF(self->sack); // sack has to kept around until we are
    self->sltr = hy_selector_create(csack);
    return 0;
}

static void
selector_dealloc(_SelectorObject *self)
{
    if (self->sltr)
	hy_selector_free(self->sltr);

    Py_XDECREF(self->sack);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
set(_SelectorObject *self, PyObject *args)
{
    key_t keyname;
    int cmp_type;
    const char *cmatch;

    if (!PyArg_ParseTuple(args, "iis", &keyname, &cmp_type, &cmatch))
	return NULL;
    if (ret2e(hy_selector_set(self->sltr, keyname, cmp_type, cmatch),
	      "Invalid Selector spec." ))
	return NULL;
    Py_RETURN_NONE;
}

static struct PyMethodDef selector_methods[] = {
    {"set", (PyCFunction)set, METH_VARARGS,
     NULL},
    {NULL}                      /* sentinel */
};

PyTypeObject selector_Type = {
    PyObject_HEAD_INIT(NULL)
    0,				/*ob_size*/
    "_hawkey.Selector",		/*tp_name*/
    sizeof(_SelectorObject),	/*tp_basicsize*/
    0,				/*tp_itemsize*/
    (destructor) selector_dealloc, /*tp_dealloc*/
    0,				/*tp_print*/
    0,				/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    "Selector object",		/* tp_doc */
    0,				/* tp_traverse */
    0,				/* tp_clear */
    0,				/* tp_richcompare */
    0,				/* tp_weaklistoffset */
    PyObject_SelfIter,		/* tp_iter */
    0,                         	/* tp_iternext */
    selector_methods,		/* tp_methods */
    0,				/* tp_members */
    0,				/* tp_getset */
    0,				/* tp_base */
    0,				/* tp_dict */
    0,				/* tp_descr_get */
    0,				/* tp_descr_set */
    0,				/* tp_dictoffset */
    (initproc)selector_init,	/* tp_init */
    0,				/* tp_alloc */
    selector_new,		/* tp_new */
    0,				/* tp_free */
    0,				/* tp_is_gc */
};
