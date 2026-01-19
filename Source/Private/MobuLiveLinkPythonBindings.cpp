// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Python bindings for MobuLiveLink using Python C API
 * 
 * This module provides Python interface for controlling MobuLiveLink,
 * such as adding/removing models from the LiveLink stream.
 * 
 * We use Python C API directly instead of Boost.Python to avoid
 * compatibility issues between VS 2017-era Boost (in Mobu SDK) and VS 2022.
 */

#include <Python.h>
#include "MobuLiveLinkDevice.h"
#include "MobuLiveLinkStreamObjects.h"

// ============================================================================
// Global Device Instance
// ============================================================================

// Pointer to the active MobuLiveLink device instance
// This is set when the device is created and cleared when destroyed
static FMobuLiveLink* g_MobuLiveLinkDevice = nullptr;

void SetMobuLiveLinkDeviceInstance(FMobuLiveLink* Device)
{
	g_MobuLiveLinkDevice = Device;
}

// ============================================================================
// Python C API Function Implementations
// ============================================================================

/**
 * Add a model to the LiveLink stream
 * Python signature: add_stream_object(model_name) -> bool
 * 
 * Args:
 *     model_name: The name of the model to add (string)
 * 
 * Returns:
 *     True if the model was added successfully, False otherwise
 */
static PyObject* livelink_add_stream_object(PyObject* self, PyObject* args)
{
	// Check if device is initialized
	if (!g_MobuLiveLinkDevice)
	{
		PyErr_SetString(PyExc_RuntimeError, "MobuLiveLink device is not initialized");
		return NULL;
	}
	
	// Parse arguments - model name as string
	const char* modelName = NULL;
	if (!PyArg_ParseTuple(args, "s", &modelName))
	{
		PyErr_SetString(PyExc_TypeError, "Argument must be a model name (string)");
		return NULL;
	}
	
	// Find the model by name
	FBModel* fbModel = FBFindModelByLabelName(modelName);
	
	if (!fbModel)
	{
		PyErr_Format(PyExc_ValueError, "Model not found: %s", modelName);
		return NULL;
	}
	
	// Check if this model is already in the stream
	for (const TPair<int32, TSharedPtr<IStreamObject>>& MapPair : g_MobuLiveLinkDevice->StreamObjects)
	{
		if (MapPair.Value.IsValid() && MapPair.Value->GetModelPointer() == fbModel)
		{
			// Model already exists in stream, skip adding
			FBTrace("Python API: Model '%s' is already in LiveLink stream, skipping\n", modelName);
			Py_RETURN_TRUE;
		}
	}
	
	// Create the appropriate StreamObject based on model type
	TSharedPtr<IStreamObject> streamObject = StreamObjectManagement::FBModelToStreamObject(fbModel);
	
	if (!streamObject.IsValid() || !streamObject->IsValid())
	{
		PyErr_Format(PyExc_RuntimeError, "Failed to create stream object for model: %s", modelName);
		return NULL;
	}
	
	// Get a unique ID and add the object to the stream
	int32 newUID = g_MobuLiveLinkDevice->GetNextUID();
	g_MobuLiveLinkDevice->AddStreamObject(newUID, streamObject);
	
	// Success message
	FBTrace("Python API: Successfully added model '%s' to LiveLink stream\n", modelName);
	PyRun_SimpleString("print 'Successfully added model to LiveLink stream!'");
	
	// Return True
	Py_RETURN_TRUE;
}

/**
 * Remove a model from the LiveLink stream
 * Python signature: remove_stream_object(model_name) -> bool
 * 
 * Args:
 *     model_name: The name of the model to remove (string)
 * 
 * Returns:
 *     True if the model was removed successfully, False otherwise
 */
static PyObject* livelink_remove_stream_object(PyObject* self, PyObject* args)
{
	// Check if device is initialized
	if (!g_MobuLiveLinkDevice)
	{
		PyErr_SetString(PyExc_RuntimeError, "MobuLiveLink device is not initialized");
		return NULL;
	}
	
	// Parse arguments - model name as string
	const char* modelName = NULL;
	if (!PyArg_ParseTuple(args, "s", &modelName))
	{
		PyErr_SetString(PyExc_TypeError, "Argument must be a model name (string)");
		return NULL;
	}
	
	// Find the model by name
	FBModel* fbModel = FBFindModelByLabelName(modelName);
	
	if (!fbModel)
	{
		PyErr_Format(PyExc_ValueError, "Model not found: %s", modelName);
		return NULL;
	}
	
	// Search for the stream object associated with this model
	bool bFound = false;
	int32 foundKey = -1;
	TSharedPtr<IStreamObject> foundObject = nullptr;
	
	for (TPair<int32, TSharedPtr<IStreamObject>>& MapPair : g_MobuLiveLinkDevice->StreamObjects)
	{
		if (MapPair.Value.IsValid() && MapPair.Value->GetModelPointer() == fbModel)
		{
			foundKey = MapPair.Key;
			foundObject = MapPair.Value;
			bFound = true;
			break;
		}
	}
	
	if (!bFound)
	{
		PyErr_Format(PyExc_ValueError, "Model '%s' is not in the LiveLink stream", modelName);
		return NULL;
	}
	
	// Remove the object from the stream
	g_MobuLiveLinkDevice->RemoveStreamObject(foundKey, foundObject);
	
	// Success message
	FBTrace("Python API: Successfully removed model '%s' from LiveLink stream\n", modelName);
	PyRun_SimpleString("print 'Successfully removed model from LiveLink stream!'");
	
	// Return True
	Py_RETURN_TRUE;
}

// ============================================================================
// Module Method Table
// ============================================================================

static PyMethodDef LiveLinkMethods[] = {
	{
		"add_stream_object",                       // Python function name
		livelink_add_stream_object,                // C function pointer
		METH_VARARGS,                              // Takes arguments
		"Add a model to the LiveLink stream.\n\n"
		"Args:\n"
		"    model_name (str): The name of the model to add\n\n"
		"Returns:\n"
		"    bool: True if successful\n\n"
		"Raises:\n"
		"    RuntimeError: If MobuLiveLink device is not initialized\n"
		"    ValueError: If model not found\n\n"
		"Example:\n"
		"    import livelink\n"
		"    livelink.add_stream_object('MyCharacter')\n"
		"    livelink.add_stream_object('Camera001')"
	},
	{
		"remove_stream_object",                    // Python function name
		livelink_remove_stream_object,             // C function pointer
		METH_VARARGS,                              // Takes arguments
		"Remove a model from the LiveLink stream.\n\n"
		"Args:\n"
		"    model_name (str): The name of the model to remove\n\n"
		"Returns:\n"
		"    bool: True if successful\n\n"
		"Raises:\n"
		"    RuntimeError: If MobuLiveLink device is not initialized\n"
		"    ValueError: If model not found or not in stream\n\n"
		"Example:\n"
		"    import livelink\n"
		"    livelink.remove_stream_object('MyCharacter')\n"
		"    livelink.remove_stream_object('Camera001')"
	},
	
	// Null terminator for method array
	{ NULL, NULL, 0, NULL }
};

// ============================================================================
// Module Initialization Function (Python 2.7 style)
// ============================================================================

/**
 * Module initialization function for Python 2.7
 * Note: Python 2 uses initXXX() and returns void, not PyObject*
 * This is different from Python 3's PyInit_XXX()
 */
static void initlivelink(void)
{
	PyObject* module = Py_InitModule3(
		"livelink",                                // Module name
		LiveLinkMethods,                           // Method table
		"MobuLiveLink Python API for controlling LiveLink stream.\n\n"
		"This module allows you to add/remove models from the LiveLink stream.\n\n"
		"Example:\n"
		"    import livelink\n"
		"    livelink.add_stream_object('MyCharacter')"
	);
	
	if (module == NULL)
	{
		FBTrace("ERROR: Failed to create livelink module\n");
		return;
	}
	
	FBTrace("MobuLiveLink Python module 'livelink' created successfully\n");
}

// ============================================================================
// Plugin Initialization - Register Module with Python
// ============================================================================

/**
 * Initialize the Python module and register it with the Python interpreter
 * This gets called when the plugin DLL loads
 */
void InitMobuLiveLinkPythonBindings()
{
	FBTrace("Initializing MobuLiveLink Python bindings...\n");
	
	// Register the module initialization function
	// Python 2.7 uses initXXX function name and returns void
	// This makes the module available for import in Python
	if (PyImport_AppendInittab("livelink", &initlivelink) == -1)
	{
		FBTrace("ERROR: Failed to register livelink module\n");
		return;
	}
	
	FBTrace("MobuLiveLink Python bindings registered successfully\n");
	FBTrace("You can now use: import livelink\n");
}
