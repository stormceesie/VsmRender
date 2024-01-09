// Function to define build conditions
#pragma once

// ######################################################################################################################
// ############################################## Multi Sample Anti Aliasing ############################################
// ######################################################################################################################
//
// Multi sampling for smoother graphics (more expensive)
// 
#define MSAA_GRAPHICS


// ######################################################################################################################
// ############################################## Swap chain computing mode #############################################
// ######################################################################################################################
//
// Advantage over OpenGL, in vulkan there is more control over settings
// Code will try to configure your prefered mode but when this mode is not supported by your device VSYNC is the standard.
// #define MAILBOX_MODE   // (High response on user interaction
// #define IMMEDIATE_MODE // Highest possible FPS consumes more power
#define VSYNC_MODE        // Efficient mode


// ######################################################################################################################
// ############################################## Validation Layers #####################################################
// ######################################################################################################################
//
// More usefull information for debugging (takes performance)
// Console can be disabled
#define CONSOLE_OUTPUT
#define VALIDATION_LAYERS

// ######################################################################################################################
// ############################################ Select GPU ##############################################################
// ######################################################################################################################
//
// You have the ability to choose the GPU in your machine this application only uses 1 GPU and possibly also the integrated graphics
// multiple GPU's is a bit hard to effectively divide the computing tasks considered their possible different memory and speed
//#define SELECTED_GPU 1