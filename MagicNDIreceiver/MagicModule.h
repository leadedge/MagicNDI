/*
 * Magic Module Development Kit (MDK) v2.3
 * Copyright (c) 2012-2020 Color & Music, LLC.  All rights reserved.
 *
 * The MDK is provided "as is" without any express or implied warranty
 * of any kind, oral or written, including warranties of merchantability,
 * fitness for any particular purpose, non-infringement, information
 * accuracy, integration, interoperability, or quiet enjoyment.  In no
 * event shall Color & Music, LLC or its suppliers be liable for any
 * damages whatsoever (including, without limitation, damages for loss
 * of profits, business interruption, loss of information, or physical
 * damage to hardware or storage media) arising out of the use of, misuse
 * of, or inability to use the MDK, your reliance on any content in the
 * MDK, or from the modification, alteration or complete discontinuance
 * of the MDK, even if Color & Music, LLC has been advised of the
 * possibility of such damages.
 */

#ifndef MAGIC_MODULE_H
#define MAGIC_MODULE_H

#define MAGIC_MDK_VERSION 2.3

#ifndef NULL
#define NULL 0
#endif

enum MagicWidgetType {
	MWT_TEXTBOX,
	MWT_TOGGLEBUTTON,
	MWT_FILECHOOSER,
	MWT_COMBOBOX,
	MWT_TRIGGERBUTTON
};

enum MagicValueType {
	MVT_FLOAT,
	MVT_INT,
	MVT_BOOL,
	MVT_STRING

	/*
	Not all types are supported by all widgets:
	- MWT_TEXTBOX must use an MVT_FLOAT, MVT_INT, or MVT_STRING value.
	- MWT_TOGGLEBUTTON and MWT_TRIGGERBUTTON must use an MVT_BOOL value.
	- MWT_FILECHOOSER must use an MVT_STRING value.
	- MWT_COMBOBOX must use an MVT_INT or MVT_STRING value.
	*/
};

struct MagicModuleParam
{
	// See further below for an example

	const char *label;
	const char *defaultValue;
	const char *minimumValue;
	const char *maximumValue;
	MagicValueType valueType;
	MagicWidgetType widgetType;
	bool isLinkable;
	const char *description;
	const char *extraInfo;

	MagicModuleParam() {}; // in case the parameters can't immediately be determined

	MagicModuleParam(
		const char *paramLabel,
		const char *defaultVal,
		const char *minVal,						// Must be NULL if no min val needed
		const char *maxVal,						// Must be NULL if no max val needed
		const MagicValueType vType,
		const MagicWidgetType wType,
		const bool linkable,
		const char *paramDescription = NULL,	// The description is what's used for the tooltip
		const char *paramInfo = NULL)			// How this is used depends on valueType and widgetType; see below
		: label(paramLabel), defaultValue(defaultVal), minimumValue(minVal), maximumValue(maxVal), valueType(vType), widgetType(wType),
		  isLinkable(linkable), description(paramDescription != NULL ? paramDescription : paramLabel),
		  extraInfo(paramInfo) {};

	/*
	Note on paramInfo (extraInfo):
	For file choosers, this is the file filter to be used in the File Open dialog box, with each extension separated by semicolon.  For example, "*.jpg;*.png;*.bmp".
	For combo boxes, this is the list of options in the drop-down box, separated by newline.  For example, "X-Axis\nY-Axis\nZ-Axis".
	For other widget types, this is not used (should be left as NULL).
	*/
};

struct MagicModuleSettings
{
	const int numModuleParams;			// Number of parameters this module uses.  See the MyModule example below
	const bool usesAudioSamples;		// Whether or not this module uses audio samples
	const bool usesSpectrumMagnitudes;	// Whether or not this module uses spectrum magnitudes
    const bool loadFromMainThread;		// Some modules, such as those using QuickTime, cannot be loaded on a background thread
	const int maxNumInputs;				// Maximum number of independent inputs for this module (generally this will be 1 in most cases)
    
	MagicModuleSettings(const int numParams, const bool usesSamples, const bool usesMagnitudes,
		const bool mustBeLoadedFromMainThread = false, const int numInputs = 1)
		:	numModuleParams(numParams), usesAudioSamples(usesSamples), usesSpectrumMagnitudes(usesMagnitudes),
			loadFromMainThread(mustBeLoadedFromMainThread), maxNumInputs(numInputs)
        {};
	
	MagicModuleSettings(const int numParams) :
        numModuleParams(numParams), usesAudioSamples(false), usesSpectrumMagnitudes(false),
			loadFromMainThread(false), maxNumInputs(1)
        {};
	
	MagicModuleSettings() :
        numModuleParams(0), usesAudioSamples(false), usesSpectrumMagnitudes(false),
			loadFromMainThread(false), maxNumInputs(1)
        {};
};

struct OpenGLState
{
	// Various aspects of the current OpenGL state.
	// These are provided so that you don't have to use glGet to query for them,
	// which would slow down the OpenGL pipeline.

	// You can modify the non-const variables as necessary in drawBefore() (see class MagicModule, below),
	// but be sure to return them to their previous values in drawAfter().
	// Otherwise, you'll throw off everything else in your scenes.

	// Magic's current overall graphics resolution in pixels
	int overallWidth, overallHeight;

	// Resolution of current OpenGL viewport in pixels
	int viewportWidth, viewportHeight;

	// Currently bound Framebuffer Object.
	// This value is the same as what would be retrieved by glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFramebuffer)
	unsigned int currentFramebuffer;

	// Input textures (if your module uses more than 1).  These will always be the same size as the viewport
	const unsigned int *inputTextureIDs;
	const unsigned int numInputTextureIDs;

	// Current modelview matrix
	float modelview[16];

	// Current color
	float r, g, b, a;

	// Depth testing enabled
	bool depthTest;

	// Blending enabled
	bool blend;

	// Source and destination factors for blending
	int sfactor, dfactor;
};

struct MagicUserData
{
	// Stuff you might want to use during drawing:

	// The current audio samples
	const float *samples; // ring buffer (default)
	const float *samplesFifo; // fifo buffer
	
	// The length of the audio samples
	const int numSamples;

	// The current audio spectrum magnitudes
	const float *magnitudes;

	// The length of the audio spectrum magnitudes
	const int numMagnitudes;
	
	// Current time in seconds
	// Note: your module shouldn't assume 60 fps.  Always use the difference in time between the current frame and the previous frame.
	const double currentTime;

	// Instantaneous values of the mapped params your module uses
	const float *paramValues;

	// Whether or not this module was drawn last frame.  Useful for indicating that the scene has changed.
	const bool drawnLastFrame;

	// Whether or not to draw any of the inputs to this module.  Default is always true for all.  Length of the array is numInputs (below).
	bool *inputsToDraw;

	// Number of modules connected as inputs to this module
	const int numInputs;

	// See above
	OpenGLState *glState;

	// The unique path of modules leading to this one
	const char *uniquePathID;
};

/*
Subclass the MagicModule class (see below this comment block) to define your module.

getSettings() should return your module's overall configuration.
getParams() should return each parameter's configuration.

Example:

#define numParams 3

class MyModule : public MagicModule
{
	static const MagicModuleSettings settings;
	static const MagicModuleParam params[numParams];
	
	virtual const MagicModuleSettings *getSettings() { return &settings; }
	virtual const MagicModuleParam *getParams() { return params; }
};

const MagicModuleSettings MyModule::settings = MagicModuleSettings(numParams, true, false);

const MagicModuleParam MyModule::params[numParams] = {
	MagicModuleParam("myParam",			".5",	"0",	"1",	MVT_FLOAT,	MWT_TEXTBOX,		true,	"what a great parameter"),
	MagicModuleParam("anotherParam",	"3",	"0",	NULL,	MVT_INT,	MWT_TEXTBOX,		true,	"an even better parameter!"),
	MagicModuleParam("yetAnother",		"0",	NULL,	NULL,	MVT_BOOL,	MWT_TOGGLEBUTTON,	false),
	MagicModuleParam("andAnother",		".5",	NULL,	NULL,	MVT_STRING, MWT_FILECHOOSER,	true,	"no description for the previous parameter")
};

Also note that the name of your module as used and displayed in Magic will be its filename without extension.
*/

class MagicModule
{
public:

	// getSettings() and getParams() are called whenever a new instance of the module is created.
	// getSettings() is required!
	// See above for more info.
	virtual const MagicModuleSettings *getSettings() = 0;
	virtual const MagicModuleParam *getParams() { return NULL; }

	// getHelpText() is called when the user selects "Help" from the module menu.
	virtual const char *getHelpText() { return NULL; }

	// glInit() is called the first time the OpenGL context is available after the module is created.
	// glClose() is called the last time the OpenGL context is available before the module will be deleted.
	// Use these functions to load/delete textures, etc.
	// Note that the context is always recreated from scratch when entering or exiting fullscreen mode,
	// so make sure to allocate and deallocate all graphics memory properly.
	virtual void glInit(MagicUserData *userData) {};
	virtual void glClose() {};

	// drawBefore() is called every frame, before any children (anything connected to the input pins) are drawn.
	// drawAfter() is called every frame, after any children are drawn.
	virtual void drawBefore(MagicUserData *userData) {};
	virtual void drawAfter(MagicUserData *userData) {};

	// fixedParamValueChanged() is called when a user edits a value in the module GUI, or when a project is loaded.
	// It should return false if newValue is invalid somehow (can't load file, value out of range, etc.).
	// Note that this function is called from the GUI thread, not the OpenGL thread, so use mutexes where appropriate.
	virtual bool fixedParamValueChanged(const int whichParam, const char* newValue) { return true; }

	// ADVANCED FUNCTIONS

	// exportingStatusChanged() is called immediately before a movie export begins (exporting = true),
	// or after the export ends (exporting = false).
	virtual void exportingStatusChanged(const bool exporting) {};

	// paramCurrentlyEnabled() is called when the module needs to determine what parameters are being used.
	// It can return false if a specific parameter temporarily needs to be displayed as greyed out in the module GUI.
	virtual bool paramCurrentlyEnabled(const int whichParam) { return true; }
    
	// paramNeedsUpdating() is called every frame.
	// It can return true if a parameter's value needs to be updated in the module GUI.
	// Currently it is used only for updating combo box options.
    virtual bool paramNeedsUpdating(const int whichParam) { return false; }

	// paramAffectsOtherModules() is called when the module needs to determine if all other modules of the same type
	// are affected by a parameter change in this one.
	// The return value is a different parameter to test for equivalence, if applicable.
	virtual int paramAffectsOtherModules(const int whichParam) { return -1; }

	// paramAffectsOtherParams() is called when the module needs to determine if this parameter should automatically set any other ones.
	virtual void paramAffectsOtherParam(const int whichParam, int *whichOthersOut, int *numOthersOut) { }
		
	// paramBlocks() is called when the module needs to determine if setting this parameter should block the rest of the Magic GUI.
	virtual bool paramBlocks(const int whichParam, const char* newValue) { return false; }

	// getDefaultForParam() is called when the module needs the current default value for the parameter.
	// This can be different than the default provided by the MagicModuleParam. 
	virtual const char *getDefaultForParam(const int whichParam) { return NULL; }

	virtual const char *getParamValue(const int whichParam) { return NULL; }

	virtual void setMainWindowID(void *) { }

	virtual ~MagicModule() {};
};

#ifdef _WIN32 // Windows
#define MAGIC_MODULE_API __declspec(dllexport)
#else // OS X
#define MAGIC_MODULE_API
#endif

/*
Somewhere in your code, you must define the CreateInstance function to return a new instance of your class.
It should look exactly like this, replacing MyModule with the name of your class:
MagicModule *CreateInstance() { return new MyModule(); }
*/
extern "C" MAGIC_MODULE_API MagicModule* CreateInstance();

/*
If you don't want your module to have an input pin, you must define the HasInputPin function somewhere in your code.
It should look exactly like this:
const bool HasInputPin() { return false; }
This function isn't required, so if you don't define it, Magic will assume that you want an input pin.
*/
extern "C" MAGIC_MODULE_API const bool HasInputPin();

/*
The GetMDKVersion function tells Magic what MDK version your module was compiled with.
If you need to include this header in more than one place,
you can #define DISABLE_GET_MDK_VERSION before the #include statement,
or you can move the function definition to one of your .cpp files.
*/
#ifndef DISABLE_GET_MDK_VERSION
extern "C" MAGIC_MODULE_API const double GetMDKVersion() { return MAGIC_MDK_VERSION; }
#endif

#endif // MAGIC_MODULE_H