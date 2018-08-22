#ifndef FLIGHTGOGGLESCLIENT_H
#define FLIGHTGOGGLESCLIENT_H
/**
 * @file   FlightGogglesClient.hpp
 * @author Winter Guerra
 * @date   Feb 22, 2018
 * @brief  Library class that abstracts interactions with FlightGoggles.
 */

#include <fstream>
#include <chrono>

// Include ZMQ bindings for comms with Unity.
#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

// Include JSON message type definitions.
#include "jsonMessageSpec.hpp"
#include "json.hpp"
using json = nlohmann::json;

// For image operations
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// For converting ROS/LCM coordinates to Unity coordinates
#include "transforms.hpp"

class FlightGogglesClient
{
  public:

    //////////////////
    // VARIABLES
    //////////////////

    // Base status object (which holds camera settings, env settings, etc)
    unity_outgoing::StateMessage_t state;

    // ZMQ connection parameters
    std::string client_address = "tcp://*";
    int upload_port = 10253;
    int download_port = 10254;
    // Socket variables
    zmqpp::context context;
    zmqpp::socket upload_socket {
        context, 
        zmqpp::socket_type::publish};
    zmqpp::socket download_socket {
        context, 
        zmqpp::socket_type::subscribe};

    // Temporary buffer for reshaping and casting of received images
    std::vector<uint8_t> _castedInputBuffer;

    // Keep track of time of last sent/received messages
    int64_t last_uploaded_utime = 0;
    int64_t last_downloaded_utime = 0;
    int64_t last_upload_debug_utime = 0;
    int64_t last_download_debug_utime = 0;
    int64_t u_packet_latency = 0;
    int64_t num_frames = 0;

    ////////////////////////////////
    // FLIGHTGOGGLES SETUP FUNCTIONS
    ////////////////////////////////

    // Constructor.
    FlightGogglesClient();
    FlightGogglesClient(int instance_num);

    // Connects to FlightGoggles.
    void initializeConnections();

    
    //////////////////////////////////
    // FLIGHTGOGGLES OUTPUT FUNCTIONS
    //////////////////////////////////

    // Set camera pose using ROS coordinates.
    void setCameraPoseUsingROSCoordinates(Eigen::Affine3d ros_pose, int cam_index);
    void setCameraPoseUsingNEDCoordinates(Transform3 NED_pose, int cam_index);

    // Send render request to Unity
    bool requestRender();
    bool requestRender(bool throttleRequest);

    ///////////////////////////////////////////
    // FLIGHTGOGGLES INCOMING MESSAGE HANDLERS
    ///////////////////////////////////////////

    // Ensure that input buffer can handle the incoming message.
    inline void ensureBufferIsAllocated(unity_incoming::RenderMetadata_t renderMetadata){
        // Check that buffer size is correct
        uint64_t requested_buffer_size = renderMetadata.camWidth * renderMetadata.camHeight * 3;
        // Resize if necessary
        if (_castedInputBuffer.size() != requested_buffer_size)
        {
            _castedInputBuffer.resize(requested_buffer_size);
        }
    };

    // Blocking call. Returns rendered images and render metadata whenever 
    // it becomes available.
    unity_incoming::RenderOutput_t handleImageResponse();

    ///////////////////
    // HELPER FUNCTIONS
    ///////////////////
    static inline int64_t getTimestamp(){
        int64_t time = std::chrono::high_resolution_clock::now().time_since_epoch() /
                    std::chrono::microseconds(1);
        return time;
    };
};

#endif
